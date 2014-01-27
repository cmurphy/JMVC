#include <cstdio>
#include <math.h>
#include "MVCBStreamExtractor.h"
#include "Extractor.h"
#include "MVCScalableModifyCode.h"
#include "MVCScalableTestCode.h"





Extractor::Extractor()
: m_pcReadBitstream       ( 0 )
, m_pcWriteBitstream      ( 0 )
, m_pcExtractorParameter  ( 0 )
{
}



Extractor::~Extractor()
{
}



ErrVal
Extractor::create( Extractor*& rpcExtractor )
{
  rpcExtractor = new Extractor;
  ROT( NULL == rpcExtractor );
  return Err::m_nOK;
}



ErrVal
Extractor::init( ExtractorParameter *pcExtractorParameter )
{
  ROT( NULL == pcExtractorParameter );

  m_pcExtractorParameter  = pcExtractorParameter;

  ReadBitstreamFile*  pcReadBitstreamFile;
  RNOKS( ReadBitstreamFile::create( pcReadBitstreamFile ) );
  RNOKS( pcReadBitstreamFile->init( m_pcExtractorParameter->getInFile() ) );
  m_pcReadBitstream = (ReadBitstreamIf*)pcReadBitstreamFile;

  if( !m_pcExtractorParameter->getAnalysisOnly() )
  {
    WriteBitstreamToFile*  pcWriteBitstreamFile;
    RNOKS( WriteBitstreamToFile::create( pcWriteBitstreamFile ) );
    RNOKS( pcWriteBitstreamFile->init( m_pcExtractorParameter->getOutFile() ) );
    m_pcWriteBitstream = (WriteBitstreamIf*)pcWriteBitstreamFile;
  }
  else
  {
    m_pcWriteBitstream = NULL;
  }

  RNOK( h264::H264AVCPacketAnalyzer::create( m_pcH264AVCPacketAnalyzer ) );

  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset();
  m_cBinDataStartCode.set( m_aucStartCodeBuffer, 4 );

  return Err::m_nOK;
}



ErrVal
Extractor::destroy()
{
  m_cBinDataStartCode.reset();

  if( NULL != m_pcH264AVCPacketAnalyzer )
  {
    RNOK( m_pcH264AVCPacketAnalyzer->destroy() );
  }

  if( NULL != m_pcReadBitstream )
  {
    RNOK( m_pcReadBitstream->uninit() );
    RNOK( m_pcReadBitstream->destroy() );
  }

  delete this;

  return Err::m_nOK;
}

ErrVal
Extractor::go()
{

  RNOK( xDisplayOperationPoints() );

  ROTRS( m_pcExtractorParameter->getAnalysisOnly(), Err::m_nOK );

  RNOK( xExtractOperationPoints() );
 
  return Err::m_nOK;
}


ErrVal 
Extractor::xWriteViewScalSEIToBuffer(h264::SEI::ViewScalabilityInfoSei *pcViewScalSei, BinData *pcBinData)
{
	const UInt uiSEILength = 1000;
	UChar		pulStreamPacket[uiSEILength];
	pcBinData->reset();
	pcBinData->set( new UChar[uiSEILength], uiSEILength );

	UChar *m_pucBuffer = pcBinData->data();

	MVCScalableModifyCode cMVCScalableModifyCode;
	MVCScalableTestCode cMVCScalableTestCode;
	RNOK( cMVCScalableTestCode.init() );
	RNOK( cMVCScalableModifyCode.init( (ULong*) pulStreamPacket ) );
	RNOK( cMVCScalableTestCode.SEICode( pcViewScalSei, &cMVCScalableTestCode ) );
	UInt uiBits = cMVCScalableTestCode.getNumberOfWrittenBits();
	UInt uiSize = (uiBits+7)/8;
	RNOK( cMVCScalableModifyCode.WriteFlag( 0 ) );
	RNOK( cMVCScalableModifyCode.WriteCode( 0 ,2 ) );
	RNOK( cMVCScalableModifyCode.WriteCode( NAL_UNIT_SEI, 5 ) );
	RNOK( cMVCScalableModifyCode.WritePayloadHeader( pcViewScalSei->getMessageType(), uiSize ) );
	RNOK( cMVCScalableModifyCode.SEICode( pcViewScalSei, &cMVCScalableModifyCode ) );
	uiBits = cMVCScalableModifyCode.getNumberOfWrittenBits();
	uiSize = (uiBits+7)/8;
	UInt uiAlignedBits = 8 - (uiBits&7);
	if( uiAlignedBits != 0 && uiAlignedBits != 8 )
	{
		RNOK( cMVCScalableModifyCode.WriteCode( 1 << (uiAlignedBits-1), uiAlignedBits ) );
	}
	RNOK ( cMVCScalableModifyCode.WriteTrailingBits() );
	RNOK ( cMVCScalableModifyCode.flushBuffer() );
	uiBits = cMVCScalableModifyCode.getNumberOfWrittenBits();
	uiBits              = ( uiBits >> 3 ) + ( 0 != ( uiBits & 0x07 ) );
	uiSize = uiBits;
	RNOK( cMVCScalableModifyCode.ConvertRBSPToPayload( m_pucBuffer, pulStreamPacket, uiSize, 2 ) );
	pcBinData->decreaseEndPos( uiSEILength - uiSize );
	return Err::m_nOK;
}


ErrVal
Extractor::xChangeViewScalSEIMessage( BinData *pcBinData, h264::SEI::SEIMessage* pcSEIMessage, UInt uiOpId, UInt*& uiViewId, UInt* uiNewNumViews )
{
	if(pcSEIMessage->getMessageType() == h264::SEI::VIEW_SCALABILITY_INFO_SEI)
	{

	h264::SEI::ViewScalabilityInfoSei* pcNewViewScalSei;
	RNOK( h264::SEI::ViewScalabilityInfoSei::create(pcNewViewScalSei) );

	h264::SEI::ViewScalabilityInfoSei* pcOldViewScalSei = ( h264::SEI::ViewScalabilityInfoSei*) pcSEIMessage;
	UInt   uiOperationPointId[MAX_OPERATION_POINTS];
	UInt   uiNumOps = 0;
	UInt   uiNumViews = 0;
	UInt uiOldOpToNewOp[MAX_OPERATION_POINTS];
	::memset( uiOperationPointId, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( uiOldOpToNewOp, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );

//now operation_point_id[i] = i
//get the operation point ids that need to be preserved
	UInt m_uiOpId = uiOpId;
	UInt uiOps = uiNumOps;
	uiOperationPointId[uiNumOps] = m_uiOpId;
	uiNumOps++;
	Bool bMoreOps = true;
	while( bMoreOps )
	{
	  if( pcOldViewScalSei->getViewDependencyInfoPresentFlag( m_uiOpId ) )//SEI JJ
	  {
		  UInt uiNumOpDep = pcOldViewScalSei->getNumDirectlyDependentViews( m_uiOpId );//SEI JJ
	      for( UInt i = 0; i < uiNumOpDep; i++ )
		  {
		    UInt OpId = m_uiOpId - 1 - pcOldViewScalSei->getDirectlyDependentViewId( m_uiOpId, i );//SEI JJ
		    if( OpId >= 0 && OpId <= pcOldViewScalSei->getNumOperationPointsMinus1() )
		    {
		      uiOperationPointId[uiNumOps] = OpId;
			    uiNumOps ++;
		    }
		  }
     uiOps++;
     m_uiOpId = uiOperationPointId[uiOps];
	  }
	  else
	    bMoreOps = false;
	}
//get the views that should be preserved
	UInt TotNumView=0;	
	for( UInt i = 0; i < uiNumOps; i++ )
	{	 
	  m_uiOpId = uiOperationPointId[i];
	  TotNumView += pcOldViewScalSei->getNumTargetOutputViewsMinus1( m_uiOpId ) + 1;//SEI JJ  	 
	}
	TotNumView *= 3; // to be conservative
	 uiViewId = new UInt[TotNumView];
	  ::memset( uiViewId, 0x00, TotNumView*sizeof(UInt) );

	uiNumViews = 0;
	for( UInt i = 0; i < uiNumOps; i++ )
	{
	  m_uiOpId = uiOperationPointId[i];	  
	  UInt NumView = pcOldViewScalSei->getNumTargetOutputViewsMinus1( m_uiOpId ) + 1;//SEI JJ

	  for( UInt j = 0; j < NumView; j++ )
	  {
		UInt uiId = pcOldViewScalSei->getViewId( m_uiOpId, j );
		UInt curr;
		for( curr=0; curr < uiNumViews; curr++ )
			if( uiViewId[curr] == uiId )
				break;
		if( curr == uiNumViews )
		{
		  uiViewId[uiNumViews] = uiId;
		  uiNumViews++;
		}
	  }
	}

	*uiNewNumViews = uiNumViews;
//set new view scalability info sei message
	UInt uiNumOperationPointsMinus1 = pcOldViewScalSei->getNumOperationPointsMinus1(); 
	UInt uiNumNewOps = 0;
	for( UInt i = 0; i <= uiNumOperationPointsMinus1; i++ )
	{
	  UInt uiOpPointId = pcOldViewScalSei->getOperationPointId( i );
	  UInt j;
	  for( j = 0; j< uiNumOps; j++ )
		  if( uiOpPointId == uiOperationPointId[j] )
			  break;
	  if( j == uiNumOps )
		  continue;

	  pcNewViewScalSei->setNumOperationPointsMinus1( uiNumNewOps );
	  uiOldOpToNewOp[ i ] = uiNumNewOps;
	  pcNewViewScalSei->setOperationPointId( uiNumNewOps, uiNumNewOps );
	  pcNewViewScalSei->setPriorityId( uiNumNewOps, pcOldViewScalSei->getPriorityId( i ) );
	  pcNewViewScalSei->setTemporalId( uiNumNewOps, pcOldViewScalSei->getTemporalId( i ) );
	  pcNewViewScalSei->setNumTargetOutputViewsMinus1( uiNumNewOps, pcOldViewScalSei->getNumTargetOutputViewsMinus1( i ) );//SEI JJ
	  for( UInt view = 0; view <= pcOldViewScalSei->getNumTargetOutputViewsMinus1(i); view++ )//SEI JJ
		  pcNewViewScalSei->setViewId( uiNumNewOps, view, pcOldViewScalSei->getViewId( i, view ) );

	  pcNewViewScalSei->setProfileLevelInfoPresentFlag( uiNumNewOps, pcOldViewScalSei->getProfileLevelInfoPresentFlag( i ) );
	  pcNewViewScalSei->setBitRateInfoPresentFlag( uiNumNewOps, pcOldViewScalSei->getBitRateInfoPresentFlag( i ) );
	  pcNewViewScalSei->setFrmRateInfoPresentFlag( uiNumNewOps, pcOldViewScalSei->getFrmRateInfoPresentFlag( i ) );
	  if(!(pcOldViewScalSei->getNumTargetOutputViewsMinus1( i ))) //SEI JJ Nov 15
		pcNewViewScalSei->setViewDependencyInfoPresentFlag( uiNumNewOps, pcOldViewScalSei->getViewDependencyInfoPresentFlag( i ) );//SEI JJ
	  pcNewViewScalSei->setParameterSetsInfoPresentFlag( uiNumNewOps, pcOldViewScalSei->getParameterSetsInfoPresentFlag( i ) );//SEI JJ
	  pcNewViewScalSei->setBitstreamRestrictionInfoPresentFlag(uiNumNewOps, pcOldViewScalSei->getBitstreamRestrictionInfoPresentFlag(i));//SEI JJ

	  if( pcOldViewScalSei->getProfileLevelInfoPresentFlag( i ) )
	  {
		pcNewViewScalSei->setOpProfileLevelIdc( uiNumNewOps, pcOldViewScalSei->getOpProfileLevelIdc( i ) );//SEI JJ
		pcNewViewScalSei->setOpConstraintSet0Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet0Flag( i ) );
		pcNewViewScalSei->setOpConstraintSet1Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet1Flag( i ) );
		pcNewViewScalSei->setOpConstraintSet2Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet2Flag( i ) );
		pcNewViewScalSei->setOpConstraintSet3Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet3Flag( i ) );
		//bug_fix_chenlulu{
		pcNewViewScalSei->setOpConstraintSet4Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet4Flag( i ) );
		pcNewViewScalSei->setOpConstraintSet5Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet5Flag( i ) );
		//bug_fix_chenlulu}
		pcNewViewScalSei->setOpLevelIdc( uiNumNewOps, pcOldViewScalSei->getOpLevelIdc( i ) );
	  }
	  else if( pcOldViewScalSei->getProfileLevelInfoSrcOpIdDelta( i ) ) 
	  {
		UInt OldOpId = pcOldViewScalSei->getOperationPointId( i ) - pcOldViewScalSei->getProfileLevelInfoSrcOpIdDelta( i );
	    for( j = 0; j< uiNumOps; j++ )
		  if( OldOpId == uiOperationPointId[j] )
		    break;

		if( j == uiNumOps )
		{
		  pcNewViewScalSei->setProfileLevelInfoPresentFlag( uiNumNewOps, true );
		  pcNewViewScalSei->setOpProfileLevelIdc( uiNumNewOps, pcOldViewScalSei->getOpProfileLevelIdc( OldOpId ) );//SEI JJ
		  pcNewViewScalSei->setOpConstraintSet0Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet0Flag( OldOpId ) );
		  pcNewViewScalSei->setOpConstraintSet1Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet1Flag( OldOpId ) );
		  pcNewViewScalSei->setOpConstraintSet2Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet2Flag( OldOpId ) );
		  pcNewViewScalSei->setOpConstraintSet3Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet3Flag( OldOpId ) );
		  //bug_fix_chenlulu{
		  pcNewViewScalSei->setOpConstraintSet4Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet4Flag( OldOpId ) );
		  pcNewViewScalSei->setOpConstraintSet5Flag( uiNumNewOps, pcOldViewScalSei->getOpConstraintSet5Flag( OldOpId ) );
		  //bug_fix_chenlulu}
		  pcNewViewScalSei->setOpLevelIdc( uiNumNewOps, pcOldViewScalSei->getOpLevelIdc( OldOpId ) );
		}
	  }

	  if( pcOldViewScalSei->getBitRateInfoPresentFlag( i ) )
	  {
		pcNewViewScalSei->setAvgBitrate( uiNumNewOps, pcOldViewScalSei->getAvgBitrate( i ) );
		pcNewViewScalSei->setMaxBitrate( uiNumNewOps, pcOldViewScalSei->getMaxBitrate( i ) );
		pcNewViewScalSei->setMaxBitrateCalcWindow( uiNumNewOps, pcOldViewScalSei->getMaxBitrateCalcWindow( i ) );
	  }

	  if( pcOldViewScalSei->getFrmRateInfoPresentFlag( i ) )
	  {
		pcNewViewScalSei->setConstantFrmRateIdc( uiNumNewOps, pcOldViewScalSei->getConstantFrmRateIdc( i ) );
		pcNewViewScalSei->setAvgFrmRate( uiNumNewOps, pcOldViewScalSei->getAvgFrmRate( i ) );
	  }
	  else if( pcOldViewScalSei->getFrmRateInfoSrcOpIdDela( i ) )
	  {
		UInt OldOpId = pcOldViewScalSei->getOperationPointId( i ) - pcOldViewScalSei->getFrmRateInfoSrcOpIdDela( i );
	    for( j = 0; j< uiNumOps; j++ )
		  if( OldOpId == uiOperationPointId[j] )
		    break;

		if( j == uiNumOps )
		{
		  pcNewViewScalSei->setFrmRateInfoPresentFlag( uiNumNewOps, true );
		  pcNewViewScalSei->setConstantFrmRateIdc( uiNumNewOps, pcOldViewScalSei->getConstantFrmRateIdc( OldOpId ) );
		  pcNewViewScalSei->setAvgFrmRate( uiNumNewOps, pcOldViewScalSei->getAvgFrmRate( OldOpId ) );
		}
	  }

	  if( pcOldViewScalSei->getViewDependencyInfoPresentFlag( i ) )//SEI JJ
	  {
		  pcNewViewScalSei->setNumDirectlyDependentViews( uiNumNewOps, pcOldViewScalSei->getNumDirectlyDependentViews( i ) );//SEI JJ 
		  for( UInt ui = 0; ui < pcOldViewScalSei->getNumDirectlyDependentViews( i ); ui++ )//SEI JJ
		  {
			  UInt OldOpId = pcOldViewScalSei->getOperationPointId( i ) - pcOldViewScalSei->getDirectlyDependentViewId( i, ui ) - 1;//SEI JJ
			  for( j = 0; j< uiNumOps; j++ )
				  if( OldOpId == uiOperationPointId[j] )
					  break;
			  pcNewViewScalSei->setDirectlyDependentViewId( uiNumNewOps, ui, uiNumNewOps - uiOldOpToNewOp[OldOpId] - 1 );//SEI JJ 
		  }
	  }
	  else if( pcOldViewScalSei->getViewDependencyInfoSrcOpId( i ) )//SEI JJ 
	  {
		  UInt OldOpId = pcOldViewScalSei->getOperationPointId( i ) - pcOldViewScalSei->getViewDependencyInfoSrcOpId( i );//SEI JJ 
		  for( j = 0; j< uiNumOps; j++ )
			  if( OldOpId == uiOperationPointId[j] )
				  break;

		  if( j == uiNumOps )
		  {
			  pcNewViewScalSei->setNumDirectlyDependentViews( uiNumNewOps, pcOldViewScalSei->getNumDirectlyDependentViews( OldOpId ) );//SEI JJ 
			  for( UInt ui = 0; ui < pcOldViewScalSei->getNumDirectlyDependentViews( i ); ui++ )//SEI JJ 
			  {
				  UInt OldOpId1 = pcOldViewScalSei->getOperationPointId( i ) - pcOldViewScalSei->getDirectlyDependentViewId( OldOpId, ui ) - 1;//SEI JJ 
				  for( j = 0; j< uiNumOps; j++ )
					  if( OldOpId1 == uiOperationPointId[j] )
						  break;
				  pcNewViewScalSei->setDirectlyDependentViewId( uiNumNewOps, ui, uiNumNewOps - uiOldOpToNewOp[OldOpId1] - 1 );//SEI JJ 
			  }
		  }
		  else
			  pcNewViewScalSei->setViewDependencyInfoSrcOpId( uiNumNewOps, uiNumNewOps - uiOldOpToNewOp[OldOpId] ); //SEI JJ
	  }
	  else
		  pcNewViewScalSei->setViewDependencyInfoSrcOpId( uiNumNewOps, 0 );//SEI JJ 

	  if( pcOldViewScalSei->getParameterSetsInfoPresentFlag( i ) )//SEI JJ
	  {
		  pcNewViewScalSei->setNumSeqParameterSetMinus1( uiNumNewOps, pcOldViewScalSei->getNumSeqParameterSetMinus1( i ) );//SEI JJ 
		  for( j = 0; j <= pcOldViewScalSei->getNumSeqParameterSetMinus1(i); j++)//SEI JJ 
			  pcNewViewScalSei->setSeqParameterSetIdDelta( uiNumNewOps, j, pcOldViewScalSei->getSeqParameterSetIdDelta( i, j ) );//SEI JJ 
		  pcNewViewScalSei->setNumSubsetSeqParameterSetMinus1(uiNumNewOps,pcOldViewScalSei->getNumSubsetSeqParameterSetMinus1(i));
		  for( j=0; j<pcOldViewScalSei->getNumSubsetSeqParameterSetMinus1( i );j++)
			  pcNewViewScalSei->setSubsetSeqParameterSetIdDelta(uiNumNewOps,j,pcOldViewScalSei->getSubsetSeqParameterSetIdDelta(i,j));
		  pcNewViewScalSei->setNumPicParameterSetMinus1( uiNumNewOps, pcOldViewScalSei->getNumPicParameterSetMinus1( i ) );//SEI JJ 
		  for( j = 0; j <= pcOldViewScalSei->getNumPicParameterSetMinus1(i); j++)//SEI JJ
			  pcNewViewScalSei->setPicParameterSetIdDelta( uiNumNewOps, j, pcOldViewScalSei->getPicParameterSetIdDelta( i, j ) );//SEI JJ 
	  }
	  else if( pcOldViewScalSei->getParameterSetsInfoSrcOpId( i ) )//SEI JJ
	  {
		  UInt OldOpId = pcOldViewScalSei->getOperationPointId( i ) - pcOldViewScalSei->getParameterSetsInfoSrcOpId( i );//SEI JJ 
		  for( j = 0; j< uiNumOps; j++ )
			  if( OldOpId == uiOperationPointId[j] )
				  break;

		  if( j == uiNumOps )
		  {
			  pcNewViewScalSei->setNumSeqParameterSetMinus1( uiNumNewOps, pcOldViewScalSei->getNumSeqParameterSetMinus1( OldOpId ) );//SEI JJ 
			  for( j = 0; j <= pcOldViewScalSei->getNumSeqParameterSetMinus1(OldOpId); j++)//SEI JJ
				  pcNewViewScalSei->setSeqParameterSetIdDelta( uiNumNewOps, j, pcOldViewScalSei->getSeqParameterSetIdDelta( OldOpId, j ) );//SEI JJ 

			  pcNewViewScalSei->setNumSubsetSeqParameterSetMinus1(uiNumNewOps,pcOldViewScalSei->getNumSubsetSeqParameterSetMinus1(OldOpId));
			  for( j=0; j<pcOldViewScalSei->getNumSubsetSeqParameterSetMinus1( OldOpId );j++)
				  pcNewViewScalSei->setSubsetSeqParameterSetIdDelta(uiNumNewOps,j,pcOldViewScalSei->getSubsetSeqParameterSetIdDelta(OldOpId,j));
			  pcNewViewScalSei->setNumPicParameterSetMinus1( uiNumNewOps, pcOldViewScalSei->getNumPicParameterSetMinus1( OldOpId ) );//SEI JJ 
			  for( j = 0; j <= pcOldViewScalSei->getNumPicParameterSetMinus1(OldOpId); j++)//SEI JJ 
				  pcNewViewScalSei->setPicParameterSetIdDelta( uiNumNewOps, j, pcOldViewScalSei->getPicParameterSetIdDelta( OldOpId, j ) );//SEI JJ 
		  }
		  else
			  pcNewViewScalSei->setParameterSetsInfoSrcOpId( uiNumNewOps, uiNumNewOps - uiOldOpToNewOp[OldOpId] );//SEI JJ

	  }
	  else
		  pcNewViewScalSei->setParameterSetsInfoSrcOpId( uiNumNewOps, 0 );//SEI JJ 
	  if( pcOldViewScalSei->getBitstreamRestrictionInfoPresentFlag(i) )
	  {
		  pcNewViewScalSei->setMotionVectorsOverPicBoundariesFlag(uiNumNewOps,pcOldViewScalSei->getMotionVectorsOverPicBoundariesFlag(i));
		  pcNewViewScalSei->setMaxBytesPerPicDenom(uiNumNewOps,pcOldViewScalSei->getMaxBytesPerPicDenom(i));
		  pcNewViewScalSei->setMaxBitsPerMbDenom(uiNumNewOps,pcOldViewScalSei->getMaxBitsPerMbDenom(i));
		  pcNewViewScalSei->setLog2MaxMvLengthHorizontal(uiNumNewOps,pcOldViewScalSei->getLog2MaxMvLengthHorizontal(i));
		  pcNewViewScalSei->setLog2MaxMvLengthVertical(uiNumNewOps,pcOldViewScalSei->getLog2MaxMvLengthVertical(i));
		  pcNewViewScalSei->setNumReorderFrames(uiNumNewOps,pcOldViewScalSei->getNumReorderFrames(i));
		  pcNewViewScalSei->setMaxDecFrameBuffering(uiNumNewOps,pcOldViewScalSei->getMaxDecFrameBuffering(i));

	  }

	  uiNumNewOps++;
	}
	
	RNOK( xWriteViewScalSEIToBuffer( pcNewViewScalSei, pcBinData ) );
  }

  return Err::m_nOK;

}

ErrVal
Extractor::xExtractOperationPoints()
{
	UInt  uiOpId	    = m_pcExtractorParameter->getOpId();
    Bool  bApplyToNext  = false;
	Bool  bKeep			= true;
    Bool  bEOS          = false;
	UInt  uiNewNumViews = 0;

	UInt *uiViewId=NULL;



	RNOK( m_pcH264AVCPacketAnalyzer->init() );

	

	while( ! bEOS )
	{
	  //========== get packet ==============
	  BinData * pcBinData;
	  Bool bWriteBinData = true;

	 

	  RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
	  if( bEOS )
	  {
	    RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
        pcBinData = NULL;
	    continue;
	  }
	  //===== get packet description ======
	  h264::SEI::SEIMessage*  pcSEIMessage = 0;
	  h264::PacketDescription cPacketDescription;
	  RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcSEIMessage ) );
	  if( pcSEIMessage )
	  {
	    if( pcSEIMessage->getMessageType() != h264::SEI::VIEW_SCALABILITY_INFO_SEI )
	       bWriteBinData = false;
   	
		RNOK( xChangeViewScalSEIMessage( pcBinData, pcSEIMessage, uiOpId, uiViewId, &uiNewNumViews ) );
	  }
  	  delete pcSEIMessage;

	  if( bWriteBinData ) 			
	  {
		//============ get packet size ===========
		while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
		{
			RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
		}

		//============ set parameters ===========

		bApplyToNext = cPacketDescription.ApplyToNext;
		if( !cPacketDescription.ParameterSet && cPacketDescription.NalUnitType != NAL_UNIT_SEI)
		{
		  UInt uiView; 
		  for( uiView = 0; uiView < uiNewNumViews; uiView++ )
			if( uiViewId[uiView] == cPacketDescription.ViewId )
				break;
		  if( uiView < uiNewNumViews )
			bKeep = true;
		  else
			bKeep = false;
		}
    
        //============ write and release packet ============
        if( bKeep )
        {
          RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
          RNOK( m_pcWriteBitstream->writePacket( pcBinData ) );
        }
      }

	  RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
      pcBinData = NULL;
	}
	
	RNOK( m_pcH264AVCPacketAnalyzer->uninit() );

	if (uiViewId!=NULL)
		delete [] uiViewId;

	return Err::m_nOK;
}


ErrVal
Extractor::xDisplayOperationPoints()
{
    Bool  bEOS = false;
	h264::SEI::ViewScalabilityInfoSei*  pcViewScalInfoSei = 0;

	RNOK( m_pcH264AVCPacketAnalyzer->init() );
    BinData * pcBinData;
	RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
	if( bEOS )
	{
	  RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
      pcBinData = NULL;
	  return Err::m_nERR;
	}
	//========== initialize (View Scal Info SEI message shall be the first packet of the stream) ===========
	h264::SEI::SEIMessage*  pcSEIMessage = 0;
	h264::PacketDescription cPacketDescription;
	RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcSEIMessage ) );
    if( pcSEIMessage->getMessageType() != h264::SEI::VIEW_SCALABILITY_INFO_SEI )
	{
 	  printf("No View Scalability Info SEI Message!\n\nExtractor exit.\n\n");
	  exit(0);
	}

	pcViewScalInfoSei = (h264::SEI::ViewScalabilityInfoSei* )pcSEIMessage;
	
	UInt uiNumOperationPoints = pcViewScalInfoSei->getNumOperationPointsMinus1() + 1;
	printf("\nTotal number of operation points: %d\n", uiNumOperationPoints);
	for( UInt i = 0; i < uiNumOperationPoints; i++ )
	{
	  printf("\nNO.%d of opertion points: \n", i);
	
	  UInt uiOperationPointId = pcViewScalInfoSei->getOperationPointId(i);
	  printf("Operation Point Id: %d\n", uiOperationPointId);

	  UInt uiPriorityId = pcViewScalInfoSei->getPriorityId(i);
	  printf("Priority Id: %d\n", uiPriorityId);

	  UInt uiTemporalId = pcViewScalInfoSei->getTemporalId(i);
	  printf("Temporal Id: %d\n", uiTemporalId);

	  UInt uiNumTargetOutputViewsMinus1 = pcViewScalInfoSei->getNumTargetOutputViewsMinus1(i);//SEI JJ
	  printf("Number of active views: %d\n", uiNumTargetOutputViewsMinus1+1);//SEI JJ
	  for( UInt j = 0; j <= uiNumTargetOutputViewsMinus1; j++ )//SEI JJ
	  {
	    UInt viewid = pcViewScalInfoSei->getViewId( i, j );
		printf("View_Id[%d]: %d\n", j, viewid);
	  }
	  
	  if( pcViewScalInfoSei->getProfileLevelInfoPresentFlag(i) )
	  {
		printf( "Op Profile Idc: %d\n", pcViewScalInfoSei->getOpProfileLevelIdc(i) );//SEI JJ
		printf( "Op Constraint Set0 Flag(0:false; 1:true): %d\n", (UInt)pcViewScalInfoSei->getOpConstraintSet0Flag(i) );
		printf( "Op Constraint Set1 Flag(0:false; 1:true): %d\n", (UInt)pcViewScalInfoSei->getOpConstraintSet1Flag(i) );
		printf( "Op Constraint Set2 Flag(0:false; 1:true): %d\n", (UInt)pcViewScalInfoSei->getOpConstraintSet2Flag(i) );
		printf( "Op Constraint Set3 Flag(0:false; 1:true): %d\n", (UInt)pcViewScalInfoSei->getOpConstraintSet3Flag(i) );
		//bug_fix_chenlulu{
		printf( "Op Constraint Set4 Flag(0:false; 1:true): %d\n", (UInt)pcViewScalInfoSei->getOpConstraintSet4Flag(i) );
		printf( "Op Constraint Set5 Flag(0:false; 1:true): %d\n", (UInt)pcViewScalInfoSei->getOpConstraintSet5Flag(i) );
		//bug_fix_chenlulu}
		printf( "Op Level Idc: %d\n", pcViewScalInfoSei->getOpLevelIdc(i) );
	  }
	  else if( pcViewScalInfoSei->getProfileLevelInfoSrcOpIdDelta(i) )
	  {
	    UInt uiRefOpId = uiOperationPointId - pcViewScalInfoSei->getProfileLevelInfoSrcOpIdDelta(i);
		printf( "The profile and level information is the same as that of the operation point id %d\n", uiRefOpId );
	  }
	  else
	    printf("No profile and level information.\n");

	  if( pcViewScalInfoSei->getBitRateInfoPresentFlag(i) )
	  {
		printf( "Avg Bitrate: %d\n", pcViewScalInfoSei->getAvgBitrate(i) );
		printf( "Max Bitrate: %d\n", pcViewScalInfoSei->getMaxBitrate(i) );
		printf( "Max Bitrate Calc Window: %d\n", pcViewScalInfoSei->getMaxBitrateCalcWindow(i) );
      }
	  else
	    printf("No bitrate information.\n");

	  if( pcViewScalInfoSei->getFrmRateInfoPresentFlag(i) )
	  {
		printf( "Constant Frame rate Idc: %d\n", pcViewScalInfoSei->getConstantFrmRateIdc(i) );
		printf( "Avg Frame rate: %d\n", pcViewScalInfoSei->getAvgFrmRate(i) );
	  }
	  else if( pcViewScalInfoSei->getFrmRateInfoSrcOpIdDela(i) )
	  {
	    UInt uiRefOpId = uiOperationPointId - pcViewScalInfoSei->getFrmRateInfoSrcOpIdDela(i);
		printf( "The frame rate information is the same as that of the operation point id %d\n", uiRefOpId );
	  }
	  else
	    printf("No frame rate information.\n");

	  if( pcViewScalInfoSei->getViewDependencyInfoPresentFlag(i) )//SEI JJ
	  {
		  UInt uiNumDirectlyDependentViews = pcViewScalInfoSei->getNumDirectlyDependentViews(i);//SEI JJ
		  printf("Number of Directly Dependent Operation points: %d\n", uiNumDirectlyDependentViews);//SEI JJ
		  for( UInt j = 0; j < uiNumDirectlyDependentViews; j++ )//SEI JJ
		  {
			  UInt uiOpId = uiOperationPointId - pcViewScalInfoSei->getDirectlyDependentViewId(i, j) - 1;//SEI JJ
			  printf("NO%d of the dependent operation points(operation point id): %d\n", j+1, uiOpId);
		  }
	  }
	  else if( pcViewScalInfoSei->getViewDependencyInfoSrcOpId(i) )//SEI JJ 
	  {
		  UInt uiRefOpId = uiOperationPointId - pcViewScalInfoSei->getViewDependencyInfoSrcOpId(i);//SEI JJ 
		  printf( "The OpDependency information is the same as that of the operation point id %d\n", uiRefOpId );
	  }
	  else
		  printf("No OpDependency information.\n");

	  if( pcViewScalInfoSei->getParameterSetsInfoPresentFlag(i) )//SEI JJ 
	  {
		  UInt uiNumSeqParameterSetMinus1 = pcViewScalInfoSei->getNumSeqParameterSetMinus1(i);//SEI JJ 
		  printf("Number of SPS required: %d\n", uiNumSeqParameterSetMinus1+1 );//SEI JJ
		  for( UInt j = 0; j <= uiNumSeqParameterSetMinus1; j++ )//SEI JJ
			  printf("NO%d of the SPS required: %d\n", j, pcViewScalInfoSei->getSeqParameterSetIdDelta(i,j));//SEI JJ

		  UInt uiNumPicParameterSetMinus1 = pcViewScalInfoSei->getNumPicParameterSetMinus1(i);//SEI JJ 
		  printf("Number of PPS required: %d\n", uiNumPicParameterSetMinus1+1 );//SEI JJ
		  for( UInt j = 0; j <= uiNumPicParameterSetMinus1; j++ )//SEI JJ
			  printf("NO%d of the PPS required: %d\n", j, pcViewScalInfoSei->getPicParameterSetIdDelta(i,j));//SEI JJ 
	  }
	  else if( pcViewScalInfoSei->getParameterSetsInfoSrcOpId(i) )//SEI JJ 
	  {
		  UInt uiRefOpId = uiOperationPointId - pcViewScalInfoSei->getParameterSetsInfoSrcOpId(i);//SEI JJ
		  printf( "The SPS/PPS required information is the same as that of the operation point id %d\n", uiRefOpId );
	  }

	  else
	    printf("No SPS/PPS required information.\n\n");
    
	}
	delete pcSEIMessage;

	RNOK( m_pcH264AVCPacketAnalyzer->uninit() );

	//----- reset input file -----
	RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->uninit() );
    RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->init  ( m_pcExtractorParameter->getInFile() ) );

    return Err::m_nOK;
}


#include <cstdio>
#include <math.h>
#include "AssemblerParameter.h"
#include "Assembler.h"
#include "ReadBitstreamFile.h"
#include "MVCScalableModifyCode.h"
#include "MVCScalableTestCode.h"



Assembler::Assembler()
: m_ppcReadBitstream      (  0   )
, pcTmpViewScalInfoSei	  (  0   ) //SEI LSJ
, m_pcWriteBitstream      (  0   )
, m_pcAssemblerParameter  (  0   )
, m_pcTraceFile           (  0   )
, m_pcAssemblerTraceFile  (  0   )
, m_uiNumViews            (  0   )
, m_uiTempViewDecOrder    (  0   )
, m_bSuffix               ( true )
{
}



Assembler::~Assembler()
{
}



ErrVal
Assembler::create( Assembler*& rpcAssembler )
{
  rpcAssembler = new Assembler;
  ROT( NULL == rpcAssembler );
  return Err::m_nOK;
}

ErrVal
Assembler::init( AssemblerParameter *pcAssemblerParameter )
{
  
  ROT( NULL == pcAssemblerParameter );

  m_pcAssemblerParameter  = pcAssemblerParameter;
  m_pcAssemblerParameter->setResult( -1 );

  m_uiNumViews        = m_pcAssemblerParameter->getNumViews(); 
  m_ppcReadBitstream  = new ReadBitstreamIf * [m_uiNumViews]; 
  pcTmpViewScalInfoSei = new h264::SEI::ViewScalabilityInfoSei * [m_uiNumViews]; //SEI 
//  m_bSuffix           = m_pcAssemblerParameter->getSuffix() > 0; 
  m_bSuffix           = true; // it is mandatory to be true;
  for( UInt uiV=0; uiV <m_uiNumViews ; uiV++)
  {
    ReadBitstreamFile*  pcReadBitstreamFile;
    RNOKS( ReadBitstreamFile::create( pcReadBitstreamFile ) );
    RNOKS( pcReadBitstreamFile->init( m_pcAssemblerParameter->getInFile(uiV) ) );
    m_ppcReadBitstream [uiV] = (ReadBitstreamIf*)pcReadBitstreamFile;
  }

  WriteBitstreamToFile*  pcWriteBitstreamFile;
  RNOKS( WriteBitstreamToFile::create( pcWriteBitstreamFile ) );
  RNOKS( pcWriteBitstreamFile->init( m_pcAssemblerParameter->getOutFile() ) );
  m_pcWriteBitstream = (WriteBitstreamIf*)pcWriteBitstreamFile;
  RNOK( h264::H264AVCPacketAnalyzer::create( m_pcH264AVCPacketAnalyzer ) );      //SEI 
  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset();
  m_cBinDataStartCode.set( m_aucStartCodeBuffer, 4 );

  return Err::m_nOK;
}

ErrVal
Assembler::destroy()
{
  m_cBinDataStartCode.reset();


  if( NULL != m_ppcReadBitstream )
  {
    for( UInt uiV=0; uiV <m_uiNumViews ; uiV++)
    {
      RNOK( m_ppcReadBitstream[uiV]->uninit() );
      RNOK( m_ppcReadBitstream[uiV]->destroy() );
    }
    m_ppcReadBitstream = NULL;
  }

  if( NULL != m_pcH264AVCPacketAnalyzer )
  {
    RNOK( m_pcH264AVCPacketAnalyzer->destroy() );
  } //SEI 


  delete this;

  return Err::m_nOK;
}

//SEI {
ErrVal 
Assembler::xWriteViewScalSEIToBuffer(h264::SEI::ViewScalabilityInfoSei *pcViewScalSei, BinData *pcBinData)
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
//SEI }

ErrVal
Assembler::xAnalyse()
{
//  Bool                    bNewPicture   = false;
//Bool                    bApplyToNext  = false;
  Bool                    bEOS          = false;
  Bool                    bNewAUStart   = true;
  BinData*                pcBinData     = 0;

  m_uiTempViewDecOrder                  = 0; 
  UInt                    uiFrames      = 0; 
  UInt                    uiNumVCLUnits = 0;
  UInt                    uiNumOtherNalU= 0;

//SEI { 
  h264::SEI::ViewScalabilityInfoSei*  pcViewScalInfoSei = 0;
  Bool					  bViewScalSei = false;
  Bool					  bFinalViewScalSei = false;
  Bool					  bScanViewScalSei = false;
  h264::SEI::ViewScalabilityInfoSei*  pcFinalViewScalSei;
  RNOK( h264::SEI::ViewScalabilityInfoSei::create(pcFinalViewScalSei) );
//SEI }
  Int64                   i64Start;
//  UInt                    uiEnd;
// Dec. 1
  while( ! bEOS )
  {
    Bool bKeep       = false;

    UInt uiProcessingView = m_uiTempViewDecOrder;
//SEI { 
   uiProcessingView = 0;
   while(uiProcessingView < m_uiNumViews && !bScanViewScalSei)
   {
     RNOK( m_pcH264AVCPacketAnalyzer->init() );
     //--- get first packet ---
	 Int iPos;
	 m_ppcReadBitstream[uiProcessingView]->getPosition( iPos );
     RNOK( m_ppcReadBitstream[uiProcessingView]->extractPacket( pcBinData, bEOS ) );
   
     //--- analyse packet ---
     RNOK( m_pcH264AVCPacketAnalyzer ->processSEIAndMVC( pcBinData, pcViewScalInfoSei) );
	 
	 if( uiProcessingView == m_uiNumViews-1)
		bScanViewScalSei = true;
	 
	 if( !pcViewScalInfoSei )
	 {
	   m_ppcReadBitstream[uiProcessingView]->setPosition( iPos );
	   uiProcessingView++;
	   continue;
     }

	pcTmpViewScalInfoSei[uiProcessingView]    = pcViewScalInfoSei;
	bViewScalSei = true;
	
	UInt NumOp;
	if( uiProcessingView )
		NumOp = pcFinalViewScalSei->getNumOperationPointsMinus1() + pcTmpViewScalInfoSei[uiProcessingView]->getNumOperationPointsMinus1() + 1;
	else
		NumOp = pcTmpViewScalInfoSei[uiProcessingView]->getNumOperationPointsMinus1();
	
	pcFinalViewScalSei->setNumOperationPointsMinus1( NumOp );

    if(pcBinData)
    { 
      RNOK( m_ppcReadBitstream[uiProcessingView]->releasePacket( pcBinData ) );
      pcBinData = NULL;
    }
    uiProcessingView++;
   }

   if( bViewScalSei )
   {
     UInt OpId = 0;
	 bViewScalSei = false;
	 bFinalViewScalSei = true;
	 UInt j;

	 for( uiProcessingView = 0; uiProcessingView < m_uiNumViews; uiProcessingView++ )
	 {
	   UInt NumOp = pcTmpViewScalInfoSei[uiProcessingView]->getNumOperationPointsMinus1();
	   for( UInt i = 0; i <= NumOp; i++ )
	   {
		 pcFinalViewScalSei->setOperationPointId( OpId, OpId );
		 pcFinalViewScalSei->setPriorityId( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getPriorityId(i) );
		 pcFinalViewScalSei->setTemporalId( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getTemporalId(i) );

		 pcFinalViewScalSei->setNumTargetOutputViewsMinus1( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getNumTargetOutputViewsMinus1(i) );//SEI JJ
		 for(  j = 0; j <= pcTmpViewScalInfoSei[uiProcessingView]->getNumTargetOutputViewsMinus1(i); j++ )//SEI JJ
		   pcFinalViewScalSei->setViewId( OpId, j, pcTmpViewScalInfoSei[uiProcessingView]->getViewId(i,j) );

		 pcFinalViewScalSei->setProfileLevelInfoPresentFlag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getProfileLevelInfoPresentFlag(i) );
		 pcFinalViewScalSei->setBitRateInfoPresentFlag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getBitRateInfoPresentFlag(i) );
		 pcFinalViewScalSei->setFrmRateInfoPresentFlag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getFrmRateInfoPresentFlag(i) );
		 if(!(pcFinalViewScalSei->getNumTargetOutputViewsMinus1(OpId)))//SEI JJ
			pcFinalViewScalSei->setViewDependencyInfoPresentFlag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getViewDependencyInfoPresentFlag(i) );//SEI JJ 
		 pcFinalViewScalSei->setParameterSetsInfoPresentFlag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getParameterSetsInfoPresentFlag(i) );//SEI JJ 
		 pcFinalViewScalSei->setBitstreamRestrictionInfoPresentFlag(OpId, pcTmpViewScalInfoSei[uiProcessingView]->getBitstreamRestrictionInfoPresentFlag(i));//SEI JJ

		 if( pcFinalViewScalSei->getProfileLevelInfoPresentFlag(OpId) )
		 {
		   pcFinalViewScalSei->setOpProfileLevelIdc( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getOpProfileLevelIdc(i) );//SEI JJ
		   pcFinalViewScalSei->setOpConstraintSet0Flag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getOpConstraintSet0Flag(i) );
		   pcFinalViewScalSei->setOpConstraintSet1Flag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getOpConstraintSet1Flag(i) );
		   pcFinalViewScalSei->setOpConstraintSet2Flag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getOpConstraintSet2Flag(i) );
		   pcFinalViewScalSei->setOpConstraintSet3Flag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getOpConstraintSet3Flag(i) );
		   //bug_fix_chenlulu{
		   pcFinalViewScalSei->setOpConstraintSet4Flag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getOpConstraintSet4Flag(i) );
		   pcFinalViewScalSei->setOpConstraintSet5Flag( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getOpConstraintSet5Flag(i) );
		   //bug_fix_chenlulu}
		   pcFinalViewScalSei->setOpLevelIdc( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getOpLevelIdc(i) );
		 }
		 if( pcFinalViewScalSei->getBitRateInfoPresentFlag(OpId) )
		 {
		   pcFinalViewScalSei->setAvgBitrate( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getAvgBitrate(i) );
		   pcFinalViewScalSei->setMaxBitrate( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getMaxBitrate(i) );
		   pcFinalViewScalSei->setMaxBitrateCalcWindow( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getMaxBitrateCalcWindow(i) );
		 }

		 if( pcFinalViewScalSei->getFrmRateInfoPresentFlag(OpId) )
		 {
		   pcFinalViewScalSei->setConstantFrmRateIdc( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getConstantFrmRateIdc(i) );
		   pcFinalViewScalSei->setAvgFrmRate( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getAvgFrmRate(i) );
   		 }

		 if( pcFinalViewScalSei->getViewDependencyInfoPresentFlag(OpId) )//SEI JJ
		 {
			 pcFinalViewScalSei->setNumDirectlyDependentViews( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getNumDirectlyDependentViews(i) );//SEI JJ 
			 for( j = 0; j < pcFinalViewScalSei->getNumDirectlyDependentViews( OpId ); j++ )//SEI JJ 
				 pcFinalViewScalSei->setDirectlyDependentViewId( OpId, j, pcTmpViewScalInfoSei[uiProcessingView]->getDirectlyDependentViewId(i,j) );//SEI JJ 
		 }
		 else
			 pcFinalViewScalSei->setViewDependencyInfoSrcOpId( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getViewDependencyInfoSrcOpId(i) );//SEI JJ 


		 if( pcFinalViewScalSei->getParameterSetsInfoPresentFlag(OpId) )//SEI JJ 
		 {
			 pcFinalViewScalSei->setNumSeqParameterSetMinus1( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getNumSeqParameterSetMinus1(i) );//SEI JJ
			 for( j = 0; j < pcFinalViewScalSei->getNumSeqParameterSetMinus1( OpId ); j++ )//SEI JJ
				 pcFinalViewScalSei->setSeqParameterSetIdDelta( OpId, j, pcTmpViewScalInfoSei[uiProcessingView]->getSeqParameterSetIdDelta(i,j) );//SEI JJ
		     
			 pcFinalViewScalSei->setNumSubsetSeqParameterSetMinus1(OpId,pcTmpViewScalInfoSei[uiProcessingView]->getNumSubsetSeqParameterSetMinus1(i));//SEI JJ
			 for( j=0; j<pcFinalViewScalSei->getNumSubsetSeqParameterSetMinus1(OpId);j++)//SEI JJ
				 pcFinalViewScalSei->setSubsetSeqParameterSetIdDelta(OpId,j,pcTmpViewScalInfoSei[uiProcessingView]->getSubsetSeqParameterSetIdDelta(i,j));//SEI JJ

			 pcFinalViewScalSei->setNumPicParameterSetMinus1( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getNumPicParameterSetMinus1(i) );//SEI JJ
			 for( j = 0; j < pcFinalViewScalSei->getNumPicParameterSetMinus1( OpId ); j++ )//SEI JJ
				 pcFinalViewScalSei->setPicParameterSetIdDelta( OpId, j, pcTmpViewScalInfoSei[uiProcessingView]->getPicParameterSetIdDelta(i,j) );//SEI JJ
		 }
		 else
			 pcFinalViewScalSei->setParameterSetsInfoSrcOpId( OpId, pcTmpViewScalInfoSei[uiProcessingView]->getParameterSetsInfoSrcOpId(i) );//SEI JJ 
		 //{{SEI JJ
		 if( pcFinalViewScalSei->getBitstreamRestrictionInfoPresentFlag(OpId) )
		 {
			 pcFinalViewScalSei->setMotionVectorsOverPicBoundariesFlag(OpId,pcTmpViewScalInfoSei[uiProcessingView]->getMotionVectorsOverPicBoundariesFlag(i));
			 pcFinalViewScalSei->setMaxBytesPerPicDenom(OpId,pcTmpViewScalInfoSei[uiProcessingView]->getMaxBytesPerPicDenom(i));
			 pcFinalViewScalSei->setMaxBitsPerMbDenom(OpId,pcTmpViewScalInfoSei[uiProcessingView]->getMaxBitsPerMbDenom(i));
			 pcFinalViewScalSei->setLog2MaxMvLengthHorizontal(OpId,pcTmpViewScalInfoSei[uiProcessingView]->getLog2MaxMvLengthHorizontal(i));
			 pcFinalViewScalSei->setLog2MaxMvLengthVertical(OpId,pcTmpViewScalInfoSei[uiProcessingView]->getLog2MaxMvLengthVertical(i));
			 pcFinalViewScalSei->setNumReorderFrames(OpId,pcTmpViewScalInfoSei[uiProcessingView]->getNumReorderFrames(i));
			 pcFinalViewScalSei->setMaxDecFrameBuffering(OpId,pcTmpViewScalInfoSei[uiProcessingView]->getMaxDecFrameBuffering(i));

		 }//}}SEI JJ

	   
	     OpId++;
	   }
	 }
   }
   uiProcessingView = m_uiTempViewDecOrder;
//SEI }
    //===== get packet =====
    RNOK( m_ppcReadBitstream[uiProcessingView]->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      RNOK( m_ppcReadBitstream[uiProcessingView]->releasePacket( pcBinData ) );
      pcBinData = NULL;
      m_uiTempViewDecOrder++;
      continue;
    }
    uiNumVCLUnits++;

//JVT-W080, BUG_FIX    //===== set packet length =====
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }

//~JVT-W080
    //===== get NAL Unit type only =====
    //----------------------------assembling ----------------------------------------------
    UChar       ucByte        = (pcBinData->data())[0];
    NalUnitType eNalUnitType  = NalUnitType ( ucByte  & 0x1F );
    if(NAL_UNIT_SPS == eNalUnitType || NAL_UNIT_SUBSET_SPS == eNalUnitType || NAL_UNIT_PPS == eNalUnitType )
    {
      Bool isMVCProfile = false;
      uiNumVCLUnits--;
      if(bNewAUStart)
	  { 
		  bKeep = true; 
		  uiNumOtherNalU++;
	  }
    
    
	  if (NAL_UNIT_SPS == eNalUnitType || NAL_UNIT_SUBSET_SPS == eNalUnitType ) 
		RNOK( m_pcH264AVCPacketAnalyzer ->isMVCProfile ( pcBinData, isMVCProfile) );
		
	  if(isMVCProfile && bFinalViewScalSei && pcFinalViewScalSei )
	  {
		RNOK( m_pcH264AVCPacketAnalyzer ->processSEIAndMVC( pcBinData, pcFinalViewScalSei ) );
		BinData* pcBinData1 = 0;
		pcBinData1 = new BinData;
		xWriteViewScalSEIToBuffer( pcFinalViewScalSei, pcBinData1 );
	      
		UChar ucTemp[100];
		UInt  uiRet;
		i64Start = m_pcWriteBitstream->getFile().tell();
		m_pcWriteBitstream->getFile().close();
		m_pcWriteBitstream->getFile().open(m_pcAssemblerParameter->getOutFile(),LargeFile::OM_READONLY);
		m_pcWriteBitstream->getFile().read(ucTemp,(UInt) i64Start,uiRet);
		m_pcWriteBitstream->getFile().close();
		m_pcWriteBitstream->getFile().open(m_pcAssemblerParameter->getOutFile(),LargeFile::OM_WRITEONLY);
	      
		RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
		RNOK( m_pcWriteBitstream->writePacket( pcBinData1 ) );
	      
		m_pcWriteBitstream->getFile().write(ucTemp,(UInt) i64Start);
	     
		pcFinalViewScalSei->destroy();
		pcFinalViewScalSei = NULL;
		pcBinData1->deleteData();
		delete pcBinData1;
	  }

    }
//JVT-W080
	else if ( NAL_UNIT_SEI == eNalUnitType )
	{
	  uiNumVCLUnits--;
      bKeep = true; 
	  uiNumOtherNalU++;
    }
//~JVT-W080
    else if ( NAL_UNIT_CODED_SLICE_PREFIX == eNalUnitType )
    {
      bKeep        =  true;
      bNewAUStart  =  false;
      // for trace only 
      printf("frames %d \n", uiFrames++);
    }
    else if ( NAL_UNIT_CODED_SLICE == eNalUnitType || NAL_UNIT_CODED_SLICE_IDR == eNalUnitType || NAL_UNIT_CODED_SLICE_SCALABLE == eNalUnitType || NAL_UNIT_CODED_SLICE_IDR_SCALABLE  == eNalUnitType )
      // JVT-W035
    {
      bKeep          =  true;

      m_uiTempViewDecOrder++;
      
      if( m_uiTempViewDecOrder==m_uiNumViews ) 
      {
        bNewAUStart          = true;
        m_uiTempViewDecOrder = 0;
      }
      else bNewAUStart = false;
    }
    else 
    {
      uiNumOtherNalU++;
      bKeep =true;
    }
    
    if( bKeep )
    {
      RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
      RNOK( m_pcWriteBitstream->writePacket( pcBinData ) );
      printf("%d\n", uiProcessingView);
    }
    //----------------------------assembling ----------------------------------------------
    
   if(pcBinData)
   {
    RNOK( m_ppcReadBitstream[uiProcessingView]->releasePacket( pcBinData ) );
    pcBinData = NULL;
   }
  }
  printf("**************************************************\n");
  printf("%d views x %d frames processed by the assembler\n number of total VCL NAL Units: %d\n", m_uiNumViews, uiFrames, uiNumVCLUnits);
  printf("Number of Other written NAL Units: %d \n", uiNumOtherNalU);
  return Err::m_nOK;
}

ErrVal
Assembler::go()
{
  RNOK ( xAnalyse() );


  /* trace 
  if( m_pcExtractionTraceFile ) 
  {
    RNOK( xExtractTrace() ); 
  }
  */

  return Err::m_nOK;
}


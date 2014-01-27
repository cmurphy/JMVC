#include <cstdio>
#include "H264AVCEncoderLib.h"
#include "H264AVCEncoder.h"

//#include "GOPEncoder.h"
#include "CreaterH264AVCEncoder.h"
#include "ControlMngH264AVCEncoder.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/FrameMng.h"

#include <math.h>




H264AVC_NAMESPACE_BEGIN


H264AVCEncoder::H264AVCEncoder():
  m_pcParameterSetMng ( NULL ),
  m_pcPocCalculator   ( NULL ),
  m_pcNalUnitEncoder  ( NULL ),
  m_pcControlMng      ( NULL ),
  m_pcCodingParameter ( NULL ),
  m_pcFrameMng        ( NULL ),
  m_bVeryFirstCall    ( true ),
  m_bScalableSeiMessage( false ),
  m_bInitDone         ( false ),
  m_bTraceEnable      ( false )  
{
  ::memset( m_dFinalFramerate, 0x00,MAX_LAYERS*MAX_DSTAGES*MAX_QUALITY_LEVELS*sizeof(Double) );
	::memset( m_dFinalBitrate,	0x00, MAX_LAYERS*MAX_DSTAGES*MAX_QUALITY_LEVELS*sizeof(Double) );
	for( UInt ui = 0; ui < MAX_LAYERS; ui++ )
	for( UInt uj = 0; uj < MAX_TEMP_LEVELS; uj++ )
		for( UInt uk = 0; uk < MAX_QUALITY_LEVELS; uk++ ){

		  m_aaauidSeqBits[ui][uj][uk] = 0;                   
		  m_aaadSingleLayerBitrate[ui][uj][uk] = 0;            // BUG_FIX Shenqiu (06-04-08)
		  m_aaauiScalableLayerId[ui][uj][uk] = MSYS_UINT_MAX;  // BUG_FIX Shenqiu (06-04-08)
		}

}

H264AVCEncoder::~H264AVCEncoder()
{
}


ErrVal
H264AVCEncoder::create( H264AVCEncoder*& rpcH264AVCEncoder )
{
  rpcH264AVCEncoder = new H264AVCEncoder;

  ROT( NULL == rpcH264AVCEncoder );

  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal
H264AVCEncoder::init( 
                      ParameterSetMng*  pcParameterSetMng,
                      PocCalculator*    pcPocCalculator,
                      NalUnitEncoder*   pcNalUnitEncoder,
                      ControlMngIf*     pcControlMng,
                      CodingParameter*  pcCodingParameter,
                      FrameMng*         pcFrameMng)
{
  ROT( NULL == pcFrameMng );
  ROT( NULL == pcParameterSetMng );
  ROT( NULL == pcPocCalculator );
  ROT( NULL == pcNalUnitEncoder );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcCodingParameter );

  m_pcFrameMng        = pcFrameMng;
  m_pcParameterSetMng = pcParameterSetMng;
  m_pcPocCalculator   = pcPocCalculator;
  m_pcNalUnitEncoder  = pcNalUnitEncoder;
  m_pcControlMng      = pcControlMng;
  m_pcCodingParameter = pcCodingParameter;

  m_cAccessUnitList.clear();

  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::uninit()
{
  m_cUnWrittenSPS.clear();
  m_cUnWrittenPPS.clear();
  m_pcParameterSetMng           = NULL;
  m_pcPocCalculator             = NULL;
  m_pcNalUnitEncoder            = NULL;
  m_pcControlMng                = NULL;
  m_pcCodingParameter           = NULL;
  m_pcFrameMng                  = NULL;
  m_bInitDone                   = false;
  m_bVeryFirstCall              = true;
  m_bScalableSeiMessage         = true;
  m_bTraceEnable                = false;  

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_acOrgPicBufferList[uiLayer]   .clear();
    m_acRecPicBufferList[uiLayer]   .clear();
  }

  m_cAccessUnitList.clear();

  return Err::m_nOK;
}



//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
ErrVal H264AVCEncoder::writeQualityLevelInfosSEI(ExtBinDataAccessor* pcExtBinDataAccessor, UInt* uiaQualityLevel, UInt *uiaDelta, UInt uiNumLevels, UInt uiLayer ) 
{
	//===== create message =====
  SEI::QualityLevelSEI* pcQualityLevelSEI;
  RNOK( SEI::QualityLevelSEI::create( pcQualityLevelSEI ) );

  //===== set message =====
  pcQualityLevelSEI->setNumLevel(uiNumLevels);
  pcQualityLevelSEI->setDependencyId(uiLayer);

  UInt ui;
  for(ui= 0; ui < uiNumLevels; ui++)
  {
	  pcQualityLevelSEI->setQualityLevel(ui,uiaQualityLevel[ui]);
	  pcQualityLevelSEI->setDeltaBytesRateOfLevel(ui,uiaDelta[ui]);
  }
  
  //===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcQualityLevelSEI );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

  return Err::m_nOK;
}
//}}Quality level estimation and modified truncation- JVTO044 and m12007

//SEI {
ErrVal H264AVCEncoder::writeNestingSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor, Double* dSeqBits) 
{
	SEI::ScalableNestingSei* pcScalableNestingSei;
	RNOK( SEI::ScalableNestingSei::create(pcScalableNestingSei) );

	//===== set message =====
	//may be changed here
	Bool bAllPicturesInAuFlag = false;
   	pcScalableNestingSei->setAllPicturesInAuFlag( bAllPicturesInAuFlag );
	if( bAllPicturesInAuFlag  == 0 )
	{
		UInt uiNumPicturesMinus1;

		// assign value, may be changed here
		uiNumPicturesMinus1 = 1;
		UInt *uiPicId = new UInt[uiNumPicturesMinus1+1];
        uiPicId[0] = 0;
		uiPicId[1] = 1;
        
		pcScalableNestingSei->setNumPicturesMinus1( uiNumPicturesMinus1 );
		for( UInt uiIndex = 0; uiIndex <= uiNumPicturesMinus1; uiIndex++ )
		{
			pcScalableNestingSei->setPicId( uiIndex, uiPicId[uiIndex] );
		}
		pcScalableNestingSei->setTemporalId( 0 );
		delete uiPicId;
	}
	//deal with the following SEI message in nesting SEI message
	//may be changed here, here take snap_shot_sei as an example
	SEI::FullframeSnapshotSei *pcSnapShotSEI;
	RNOK( SEI::FullframeSnapshotSei::create( pcSnapShotSEI ) );
	pcSnapShotSEI->setSnapShotId(0);

	UInt              uiBits = 0;
	SEI::MessageList  cSEIMessageList;
	cSEIMessageList.push_back                 ( pcScalableNestingSei );
	cSEIMessageList.push_back                 ( pcSnapShotSEI );
	RNOK ( m_pcNalUnitEncoder->initNalUnit    ( pcExtBinDataAccessor ) );
	RNOK ( m_pcNalUnitEncoder->writeNesting   ( cSEIMessageList ) );
	RNOK ( m_pcNalUnitEncoder->closeNalUnit   ( uiBits ) );
	dSeqBits[0] += uiBits+4*8;
//	RNOK( m_apcMCTFEncoder[0]->addParameterSetBits ( uiBits+4*8 ) );

	RNOK( pcSnapShotSEI->destroy() );
	RNOK( pcScalableNestingSei->destroy() );
	return Err::m_nOK;
}

ErrVal H264AVCEncoder::writeMultiviewSceneInfoSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor, Double* dSeqBits ) // SEI JVT-W060
{
	SEI::MultiviewSceneInfoSei* pcMultiviewSceneInfoSei;
	RNOK( SEI::MultiviewSceneInfoSei::create(pcMultiviewSceneInfoSei) );

	//===== set message =====
	pcMultiviewSceneInfoSei->setMaxDisparity(m_pcCodingParameter->getMaxDisparity());

	
	UInt              uiBits = 0;
	SEI::MessageList  cSEIMessageList;
	cSEIMessageList.push_back                 ( pcMultiviewSceneInfoSei );
	RNOK ( m_pcNalUnitEncoder->initNalUnit    ( pcExtBinDataAccessor ) );
	RNOK ( m_pcNalUnitEncoder->write	      ( cSEIMessageList ) );
	RNOK ( m_pcNalUnitEncoder->closeNalUnit   ( uiBits ) );
	dSeqBits[0] += uiBits+4*8;
	return Err::m_nOK;
}

ErrVal H264AVCEncoder::writeMultiviewAcquisitionInfoSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor, Double* dSeqBits ) // SEI JVT-W060, JVT-Z038
{
	SEI::MultiviewAcquisitionInfoSei* pcMultiviewAcquisitionInfoSei;
	RNOK( SEI::MultiviewAcquisitionInfoSei::create(pcMultiviewAcquisitionInfoSei) );
	UInt uiIndex;
	int i,j;

	Bool IntParaFlag=false;
	Bool ExtParaFlag=false;

	UInt NumViewsMinus1 = m_pcCodingParameter->getNumViewMinus1();
	pcMultiviewAcquisitionInfoSei->initialize_memory(NumViewsMinus1+1);
	//===== set message =====
	pcMultiviewAcquisitionInfoSei->setIntrinsicParamFlag(IntParaFlag=m_pcCodingParameter->getIntrinsicParamFlag());
	if (IntParaFlag)
	{
		pcMultiviewAcquisitionInfoSei->setIntrinsicParamsEqual(m_pcCodingParameter->getIntrinsicParamsEqual());
		pcMultiviewAcquisitionInfoSei->setPrecFocalLength(m_pcCodingParameter->getPrecFocalLength());
		pcMultiviewAcquisitionInfoSei->setPrecPrincipalPoint(m_pcCodingParameter->getPrecPrincipalPoint());
		pcMultiviewAcquisitionInfoSei->setPrecRadialDistortion(m_pcCodingParameter->getPrecRadialDistortion());
		

		for (uiIndex=0; uiIndex<= NumViewsMinus1; uiIndex++)
		{
			pcMultiviewAcquisitionInfoSei->setSignFocalLengthX(uiIndex,m_pcCodingParameter->getSignFocalLengthX(uiIndex));
			pcMultiviewAcquisitionInfoSei->setSignFocalLengthY(uiIndex,m_pcCodingParameter->getSignFocalLengthY(uiIndex));
			pcMultiviewAcquisitionInfoSei->setSignPrincipalPointX(uiIndex,m_pcCodingParameter->getSignPrincipalPointX(uiIndex));
			pcMultiviewAcquisitionInfoSei->setSignPrincipalPointY(uiIndex,m_pcCodingParameter->getSignPrincipalPointY(uiIndex));
			pcMultiviewAcquisitionInfoSei->setSignRadialDistortion(uiIndex,m_pcCodingParameter->getSignRadialDistortion(uiIndex));

			pcMultiviewAcquisitionInfoSei->setExponentFocalLengthX(uiIndex,m_pcCodingParameter->getExponentFocalLengthX(uiIndex));
			pcMultiviewAcquisitionInfoSei->setExponentFocalLengthY(uiIndex,m_pcCodingParameter->getExponentFocalLengthY(uiIndex));
			pcMultiviewAcquisitionInfoSei->setExponentPrincipalPointX(uiIndex,m_pcCodingParameter->getExponentPrincipalPointX(uiIndex));
			pcMultiviewAcquisitionInfoSei->setExponentPrincipalPointY(uiIndex,m_pcCodingParameter->getExponentPrincipalPointY(uiIndex));
			pcMultiviewAcquisitionInfoSei->setExponentRadialDistortion(uiIndex,m_pcCodingParameter->getExponentRadialDistortion(uiIndex));
		
			pcMultiviewAcquisitionInfoSei->setMantissaFocalLengthX(uiIndex,m_pcCodingParameter->getMantissaFocalLengthX(uiIndex));
			pcMultiviewAcquisitionInfoSei->setMantissaFocalLengthY(uiIndex,m_pcCodingParameter->getMantissaFocalLengthY(uiIndex));
			pcMultiviewAcquisitionInfoSei->setMantissaPrincipalPointX(uiIndex,m_pcCodingParameter->getMantissaPrincipalPointX(uiIndex));
			pcMultiviewAcquisitionInfoSei->setMantissaPrincipalPointY(uiIndex,m_pcCodingParameter->getMantissaPrincipalPointY(uiIndex));
			pcMultiviewAcquisitionInfoSei->setMantissaRadialDistortion(uiIndex,m_pcCodingParameter->getMantissaRadialDistortion(uiIndex));
		
		}
	} 

	pcMultiviewAcquisitionInfoSei->setExtrinsicParamFlag(ExtParaFlag=m_pcCodingParameter->getExtrinsicParamFlag());
	if (ExtParaFlag)
	{
		pcMultiviewAcquisitionInfoSei->setPrecRotationParam(m_pcCodingParameter->getPrecRotationParam());
		pcMultiviewAcquisitionInfoSei->setPrecTranslationParam(m_pcCodingParameter->getPrecTranslationParam());
		
		for (uiIndex=0; uiIndex<= NumViewsMinus1; uiIndex++)
		{
			for (i=0; i<3; i++)
			{
				pcMultiviewAcquisitionInfoSei->setSignTranslationParam(uiIndex,i,m_pcCodingParameter->getSignTranslationParam(uiIndex,i));
				pcMultiviewAcquisitionInfoSei->setExponentTranslationParam(uiIndex,i,m_pcCodingParameter->getExponentTranslationParam(uiIndex,i));
				pcMultiviewAcquisitionInfoSei->setMantissaTranslationParam(uiIndex,i,m_pcCodingParameter->getMantissaTranslationParam(uiIndex,i));
				for (j=0; j<3; j++) {
					pcMultiviewAcquisitionInfoSei->setSignRotationParam(uiIndex,i,j,m_pcCodingParameter->getSignRotationParam(uiIndex,i,j));
					pcMultiviewAcquisitionInfoSei->setExponentRotationParam(uiIndex,i,j,m_pcCodingParameter->getExponentRotationParam(uiIndex,i,j));
					pcMultiviewAcquisitionInfoSei->setMantissaRotationParam(uiIndex,i,j,m_pcCodingParameter->getMantissaRotationParam(uiIndex,i,j));
				}
			}
				
		}
	}

	
	UInt              uiBits = 0;
	SEI::MessageList  cSEIMessageList;
	cSEIMessageList.push_back                 ( pcMultiviewAcquisitionInfoSei );
	RNOK ( m_pcNalUnitEncoder->initNalUnit    ( pcExtBinDataAccessor ) );
	RNOK ( m_pcNalUnitEncoder->write	      ( cSEIMessageList ) );
	RNOK ( m_pcNalUnitEncoder->closeNalUnit   ( uiBits ) );
	dSeqBits[0] += uiBits+4*8;
  	m_pcCodingParameter->release_memory();
	return Err::m_nOK;
}

ErrVal H264AVCEncoder::writeViewScalInfoSEIMessage(ExtBinDataAccessor *pcExtBinDataAccessor, 
												   Double* dBitRate, 
												   Double* dFrameRate,
												   Double  dMaxRate)
{
  SEI::ViewScalabilityInfoSei* pcViewScalInfoSei;
  RNOK( SEI::ViewScalabilityInfoSei::create(pcViewScalInfoSei) );

  //==set message==
  UInt i, j;
 // UInt uiNumViews = m_pcCodingParameter->getSpsMVC()->getNumViewMinus1() + 1 ; 
  UInt uiNumOperationPointsMinus1 = 0;
  UInt uiCurrViewId = m_pcCodingParameter->getCurentViewId();
  pcViewScalInfoSei->setNumOperationPointsMinus1( uiNumOperationPointsMinus1 );
  for( i = 0; i <= uiNumOperationPointsMinus1; i++ )
  {
    UInt uiOperationPointId, uiPriorityId, uiTemporalId, uiNumTargetOutputViewsMinus1;//SEI JJ

	uiOperationPointId = i;
	uiTemporalId = m_pcCodingParameter->getDecompositionStages();
	uiPriorityId = uiCurrViewId == 0 ? uiTemporalId : ( uiTemporalId+uiCurrViewId%2+1 );
	uiNumTargetOutputViewsMinus1 = 0;//SEI JJ

	pcViewScalInfoSei->setOperationPointId( i, uiOperationPointId );
	pcViewScalInfoSei->setPriorityId( i, uiPriorityId );
	pcViewScalInfoSei->setTemporalId( i, uiTemporalId );
	pcViewScalInfoSei->setNumTargetOutputViewsMinus1( i, uiNumTargetOutputViewsMinus1 );//SEI JJ 
    
	for( j = 0; j <= uiNumTargetOutputViewsMinus1; j++)//SEI JJ
	{
	  UInt uiViewId = uiCurrViewId;
	  pcViewScalInfoSei->setViewId( i, j, uiViewId );
	}

	Bool bProfileLevelInfoPresentFlag, bBitRateInfoPresentFlag, bFrmRateInfoPresentFlag;
	Bool bOpDependencyInfoPresentFlag, bInitParameterSetsInfoPresentFlag,bBitstreamRestrictionInfoPresentFlag;

	bProfileLevelInfoPresentFlag = false;
	bBitRateInfoPresentFlag = true;
	bFrmRateInfoPresentFlag = true;//may be changed
	bOpDependencyInfoPresentFlag = false;
	bInitParameterSetsInfoPresentFlag = false;
	bBitstreamRestrictionInfoPresentFlag=false;
	pcViewScalInfoSei->setProfileLevelInfoPresentFlag( i, bProfileLevelInfoPresentFlag );
	pcViewScalInfoSei->setBitRateInfoPresentFlag( i, bBitRateInfoPresentFlag );
	pcViewScalInfoSei->setFrmRateInfoPresentFlag( i, bFrmRateInfoPresentFlag );
	
	pcViewScalInfoSei->setViewDependencyInfoPresentFlag( i, bOpDependencyInfoPresentFlag );//SEI JJ
	pcViewScalInfoSei->setParameterSetsInfoPresentFlag( i, bInitParameterSetsInfoPresentFlag );//SEI JJ 
	pcViewScalInfoSei->setBitstreamRestrictionInfoPresentFlag(i, bBitstreamRestrictionInfoPresentFlag);//SEI JJ

	if( bProfileLevelInfoPresentFlag )
	{
	  UInt uiOpProfileIdc, uiOpLevelIdc;
	  Bool uiOpConstraintSet0Flag, uiOpConstraintSet1Flag, 
		  uiOpConstraintSet2Flag, uiOpConstraintSet3Flag;
	  Bool uiOpConstraintSet4Flag, uiOpConstraintSet5Flag;//bug_fix_chenlulu

	  uiOpProfileIdc = 0;		// may be changed
	  uiOpLevelIdc = 0;			// may be changed
	  uiOpConstraintSet0Flag = false;		// may be changed
	  uiOpConstraintSet1Flag = false;		// may be changed
	  uiOpConstraintSet2Flag = false;		// may be changed
	  uiOpConstraintSet3Flag = false;		// may be changed
//bug_fix_chenlulu{
	  uiOpConstraintSet4Flag = false;		// may be changed
	  uiOpConstraintSet5Flag = false;		// may be changed
//bug_fix_chenlulu}

	  pcViewScalInfoSei->setOpProfileLevelIdc( i, uiOpProfileIdc );//SEI JJ
	  pcViewScalInfoSei->setOpConstraintSet0Flag( i, uiOpConstraintSet0Flag );
	  pcViewScalInfoSei->setOpConstraintSet1Flag( i, uiOpConstraintSet1Flag );
	  pcViewScalInfoSei->setOpConstraintSet2Flag( i, uiOpConstraintSet2Flag );
	  pcViewScalInfoSei->setOpConstraintSet3Flag( i, uiOpConstraintSet3Flag );
//bug_fix_chenlulu{
	  pcViewScalInfoSei->setOpConstraintSet4Flag( i, uiOpConstraintSet4Flag );
	  pcViewScalInfoSei->setOpConstraintSet5Flag( i, uiOpConstraintSet5Flag );
//bug_fix_chenlulu}

	  pcViewScalInfoSei->setOpLevelIdc( i, uiOpLevelIdc );
	}

	if( bBitRateInfoPresentFlag )
	{
	  UInt uiAvgBitrate, uiMaxBitrate, uiMaxBitrateCalcWindow;

	  uiAvgBitrate = (UInt)dBitRate[uiTemporalId];		
	  uiMaxBitrate = (UInt)dMaxRate;		//may be changed
	  uiMaxBitrateCalcWindow = 100;		//should be changed

	  pcViewScalInfoSei->setAvgBitrate( i, uiAvgBitrate );
	  pcViewScalInfoSei->setMaxBitrate( i, uiMaxBitrate );
	  pcViewScalInfoSei->setMaxBitrateCalcWindow( i, uiMaxBitrateCalcWindow );
	}

	if( bFrmRateInfoPresentFlag )
	{
	  UInt uiConstantFrmRateIdc, uiAvgFrmRate;

	  uiConstantFrmRateIdc = 0;		
	  uiAvgFrmRate =(UInt) dFrameRate[uiTemporalId];	

	  pcViewScalInfoSei->setConstantFrmRateIdc( i, uiConstantFrmRateIdc );
	  pcViewScalInfoSei->setAvgFrmRate( i, uiAvgFrmRate );
	}

	if( bOpDependencyInfoPresentFlag )
	{
		
	  UInt uiNumDirectlyDependentViews =0 ;//SEI JJ

	  pcViewScalInfoSei->setNumDirectlyDependentViews( i, uiNumDirectlyDependentViews );//SEI JJ 

	  for( j = 0; j <= uiNumDirectlyDependentViews; j++ )//SEI JJ
	  {
	    UInt uiDirectlyDependentViewId = 0;//SEI JJ

		pcViewScalInfoSei->setDirectlyDependentViewId( i, j, uiDirectlyDependentViewId );//SEI JJ
	  }
	}
	else
	{
	  UInt uiViewDependencyInfoSrcOpId = 0; //should be changed SEI JJ

	  pcViewScalInfoSei->setViewDependencyInfoSrcOpId( i, uiViewDependencyInfoSrcOpId );//SEI JJ 
	}

	if( bInitParameterSetsInfoPresentFlag )
	{
	  //the parameters may should be changed
	  UInt uiNumSeqParameterSetMinus1, uiNumPicParameterSetMinus1,uiNumSubsetSeqParameterSetMinus1;//SEI JJ

	  uiNumSeqParameterSetMinus1 = 0;//SEI JJ
	  uiNumPicParameterSetMinus1 = 0;//SEI JJ
	  uiNumSubsetSeqParameterSetMinus1=0;//

	  pcViewScalInfoSei->setNumSeqParameterSetMinus1( i, uiNumSeqParameterSetMinus1 );//SEI JJ 

	  for( j = 0; j <= uiNumSeqParameterSetMinus1; j++ )//SEI JJ
	  {
	    UInt uiSeqParameterSetIdDelta = 0;//SEI JJ

		pcViewScalInfoSei->setSeqParameterSetIdDelta( i, j, uiSeqParameterSetIdDelta );//SEI JJ 
	  }
	  
	  pcViewScalInfoSei->setNumSubsetSeqParameterSetMinus1( i, uiNumSubsetSeqParameterSetMinus1 );
	  for( j = 0; j <= uiNumSubsetSeqParameterSetMinus1; j++ )
	  {
		  UInt uiSubsetSeqParameterSetIdDelta = 0;//SEI JJ

		  pcViewScalInfoSei->setSubsetSeqParameterSetIdDelta( i, j, uiSubsetSeqParameterSetIdDelta );//SEI JJ 
	  }
      
	  pcViewScalInfoSei->setNumPicParameterSetMinus1( i, uiNumPicParameterSetMinus1 );//SEI JJ 

	  for( j = 0; j <= uiNumPicParameterSetMinus1; j++ )//SEI JJ
	  {
	    UInt uiPicParameterSetIdDelta = 0;//SEI JJ

		pcViewScalInfoSei->setPicParameterSetIdDelta( i, j, uiPicParameterSetIdDelta );//SEI JJ 
	  }

	}
	else
	{
	  UInt uiParameterSetsInfoSrcOpId = 0; //may should be changed SEI JJ

	  pcViewScalInfoSei->setParameterSetsInfoSrcOpId( i, uiParameterSetsInfoSrcOpId );//SEI JJ 
	}
	if( bBitstreamRestrictionInfoPresentFlag )
	{
	  pcViewScalInfoSei->setMotionVectorsOverPicBoundariesFlag(i,true);
	  pcViewScalInfoSei->setMaxBytesPerPicDenom(i,0);
      pcViewScalInfoSei->setMaxBitsPerMbDenom(i,0);
	  pcViewScalInfoSei->setLog2MaxMvLengthHorizontal(i,0);
	  pcViewScalInfoSei->setLog2MaxMvLengthVertical(i,0);
	  pcViewScalInfoSei->setNumReorderFrames(i,0);
	  pcViewScalInfoSei->setMaxDecFrameBuffering(i,0);
	}
  }

  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcViewScalInfoSei );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

  return Err::m_nOK;
}
//SEI }
ErrVal
H264AVCEncoder::xWriteScalableSEI( ExtBinDataAccessor* pcExtBinDataAccessor )
{
	//===== create message =====
	SEI::ScalableSei* pcScalableSEI;
	RNOK(SEI::ScalableSei::create(pcScalableSEI) );


	//===== set message =====
	UInt j; //JVT-S036 lsj 
	UInt uiInputLayers = m_pcCodingParameter->getNumberOfLayers ();
	UInt uiLayerNum = 0;	//total scalable layer numbers
	for ( UInt i = 0; i < uiInputLayers; i++ )	//calculate total scalable layer numbers
	{
		Bool bH264AVCCompatible = ( i == 0 && m_pcCodingParameter->getBaseLayerMode() > 0 );
		Bool bSubSeq            = ( i == 0 && m_pcCodingParameter->getBaseLayerMode() > 1 );

		LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( i );
		UInt uiTotalTempLevel = rcLayer.getDecompositionStages () - rcLayer.getNotCodedMCTFStages();
// *LMH(20060203): Fix Bug due to underflow (Replace)
		//UInt uiMinTempLevel   = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max( 0, uiTotalTempLevel - 1 );
		UInt uiMinTempLevel   = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max( 0, (Int)uiTotalTempLevel - 1 );
		UInt uiActTempLevel   = uiTotalTempLevel - uiMinTempLevel + 1;
		UInt uiTotalFGSLevel  = (UInt)rcLayer.getNumFGSLayers () + 1;
		uiLayerNum += uiActTempLevel * uiTotalFGSLevel;

		pcScalableSEI->setROINum ( i, rcLayer.getNumROI() );
		pcScalableSEI->setROIID  ( i, rcLayer.getROIID() );
		pcScalableSEI->setSGID  ( i, rcLayer.getSGID() );
		pcScalableSEI->setSLID  ( i, rcLayer.getSLID() );
	}
	UInt uiTotalScalableLayer = 0;

	//===== get framerate information ===
	Double *dFramerate = dGetFramerate();
  
	UInt uiNumLayersMinus1 = uiLayerNum - 1;

	pcScalableSEI->setNumLayersMinus1 ( uiNumLayersMinus1 );


	UInt uiNumScalableLayer = 0;
	for ( UInt uiCurrLayer = 0; uiCurrLayer < uiInputLayers; uiCurrLayer++)
	{
		LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( uiCurrLayer );
		UInt uiTotalTempLevel = rcLayer.getDecompositionStages () - rcLayer.getNotCodedMCTFStages() + 1;
		UInt uiTotalFGSLevel = (UInt)rcLayer.getNumFGSLayers () + 1;
		//Bool bFGSLayerFlag = uiTotalFGSLevel > 1; //JVT-S036 
		Bool bH264AVCCompatible = ( uiCurrLayer == 0 && m_pcCodingParameter->getBaseLayerMode() > 0 );
		Bool bSubSeq            = ( uiCurrLayer == 0 && m_pcCodingParameter->getBaseLayerMode() > 1 );
// *LMH(20060203): Fix Bug due to underflow (Replace)
		//UInt uiMinTempLevel     = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max(0,uiTotalTempLevel - 2);
		UInt uiMinTempLevel     = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max(0, (Int)uiTotalTempLevel - 2);

		for ( UInt uiCurrTempLevel = 0; uiCurrTempLevel < uiTotalTempLevel; uiCurrTempLevel++ )
		{
			for ( UInt uiCurrFGSLevel = 0; uiCurrFGSLevel < uiTotalFGSLevel; uiCurrFGSLevel++ )
			{
				if( uiCurrTempLevel >= uiMinTempLevel )
				{
				  //Bool bSubPicLayerFlag = false;
				  Bool bSubRegionLayerFlag = false;
				  Bool bProfileLevelInfoPresentFlag = false;
				  Bool bInitParameterSetsInfoPresentFlag = false;		//may be changed  //JVT-S036 
				  if( uiNumScalableLayer == 0 )
				 {//JVT-S036 
					 bSubRegionLayerFlag = true;
					 bProfileLevelInfoPresentFlag = true;
					 bInitParameterSetsInfoPresentFlag = true;		
				 }
				  Bool bBitrateInfoPresentFlag = true;
				  Bool bFrmRateInfoPresentFlag = true;//rcLayer.getInputFrameRate () > 0;
				  Bool bFrmSizeInfoPresentFlag = true;
// BUG_FIX liuhui{
				  Bool bLayerDependencyInfoPresentFlag = true;			//may be changed
// BUG_FIX liuhui}
				  //Bool bInitParameterSetsInfoPresentFlag = false;		//may be changed //JVT-S036 
				  Bool bExactInterayerPredFlag = true;			//JVT-S036  may be changed
          // JVT-S054 (REMOVE)
  				//Bool bIroiSliceDivisionFlag = false;  //JVT-S036 
				  pcScalableSEI->setLayerId(uiNumScalableLayer, uiNumScalableLayer);
	//JVT-S036  start
				  //pcScalableSEI->setFGSlayerFlag(uiNumScalableLayer, bFGSLayerFlag); 
				  //pcScalableSEI->setSubPicLayerFlag(uiNumScalableLayer,0);				  
					UInt uiTempLevel = uiCurrTempLevel; //BUG_FIX_FT_01_2006
					UInt uiDependencyID = uiCurrLayer;
					UInt uiQualityLevel = uiCurrFGSLevel;
	// BUG_FIX liuhui{
					m_aaauiScalableLayerId[uiCurrLayer][uiCurrTempLevel][uiCurrFGSLevel] = uiNumScalableLayer;
	// BUG_FIX liuhui}					
					UInt uiSimplePriorityId = 0;
					Bool bDiscardableFlag  = false;
					if( uiCurrFGSLevel > rcLayer.getNumFGSLayers() )
						bDiscardableFlag = true;
					pcScalableSEI->setSimplePriorityId(uiNumScalableLayer, uiSimplePriorityId);
					pcScalableSEI->setDiscardableFlag(uiNumScalableLayer, bDiscardableFlag);
					pcScalableSEI->setTemporalLevel(uiNumScalableLayer, uiTempLevel);
					pcScalableSEI->setDependencyId(uiNumScalableLayer, uiDependencyID);
					pcScalableSEI->setQualityLevel(uiNumScalableLayer, uiQualityLevel);				
	 //JVT-S036  end
				  pcScalableSEI->setSubRegionLayerFlag(uiNumScalableLayer, bSubRegionLayerFlag);
          // JVT-S054 (REPLACE)
				  //pcScalableSEI->setIroiSliceDivisionInfoPresentFlag(uiNumScalableLayer, bIroiSliceDivisionFlag); //JVT-S036 
				  pcScalableSEI->setIroiSliceDivisionInfoPresentFlag(uiNumScalableLayer, rcLayer.m_bSliceDivisionFlag);
				  pcScalableSEI->setProfileLevelInfoPresentFlag(uiNumScalableLayer, bProfileLevelInfoPresentFlag);
				  pcScalableSEI->setBitrateInfoPresentFlag(uiNumScalableLayer, bBitrateInfoPresentFlag);
				  pcScalableSEI->setFrmRateInfoPresentFlag(uiNumScalableLayer, bFrmRateInfoPresentFlag);
				  pcScalableSEI->setFrmSizeInfoPresentFlag(uiNumScalableLayer, bFrmSizeInfoPresentFlag);
				  pcScalableSEI->setLayerDependencyInfoPresentFlag(uiNumScalableLayer, bLayerDependencyInfoPresentFlag);
				  pcScalableSEI->setInitParameterSetsInfoPresentFlag(uiNumScalableLayer, bInitParameterSetsInfoPresentFlag);

				  pcScalableSEI->setExactInterlayerPredFlag(uiNumScalableLayer, bExactInterayerPredFlag);//JVT-S036 

				  if(pcScalableSEI->getProfileLevelInfoPresentFlag(uiNumScalableLayer))
				  {
					  UInt uilayerProfileIdc = 0;	//may be changed
					  Bool bLayerConstraintSet0Flag = false;	//may be changed
					  Bool bH264AVCCompatibleTmp  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiCurrLayer == 0;
					  Bool bLayerConstraintSet1Flag = ( bH264AVCCompatibleTmp ? 1 : 0 );	//may be changed
					  Bool bLayerConstraintSet2Flag = false;	//may be changed
					  Bool bLayerConstraintSet3Flag = false;	//may be changed
					  UInt uiLayerLevelIdc = 0;		//may be changed

					  pcScalableSEI->setLayerProfileIdc(uiNumScalableLayer, uilayerProfileIdc);
					  pcScalableSEI->setLayerConstraintSet0Flag(uiNumScalableLayer, bLayerConstraintSet0Flag);
					  pcScalableSEI->setLayerConstraintSet1Flag(uiNumScalableLayer, bLayerConstraintSet1Flag);
					  pcScalableSEI->setLayerConstraintSet2Flag(uiNumScalableLayer, bLayerConstraintSet2Flag);
					  pcScalableSEI->setLayerConstraintSet3Flag(uiNumScalableLayer, bLayerConstraintSet3Flag);
					  pcScalableSEI->setLayerLevelIdc(uiNumScalableLayer, uiLayerLevelIdc);
				  }
				  else
				  {//JVT-S036 
					  UInt bProfileLevelInfoSrcLayerIdDelta = 0;  //may be changed

					  pcScalableSEI->setProfileLevelInfoSrcLayerIdDelta(uiNumScalableLayer, bProfileLevelInfoSrcLayerIdDelta);
				  }


	/*			  if(pcScalableSEI->getDecodingDependencyInfoPresentFlag(uiNumScalableLayer))
				  {
					  //UInt uiTempLevel = uiCurrTempLevel - uiMinTempLevel;
					  UInt uiTempLevel = uiCurrTempLevel; //BUG_FIX_FT_01_2006
					  UInt uiDependencyID = uiCurrLayer;
					  UInt uiQualityLevel = uiCurrFGSLevel;
// BUG_FIX liuhui{
					  m_aaauiScalableLayerId[uiCurrLayer][uiCurrTempLevel][uiCurrFGSLevel] = uiNumScalableLayer;
// BUG_FIX liuhui}
					 
					  UInt uiSimplePriorityId = 0;
					  Bool uiDiscardableFlag  = false;

					  pcScalableSEI->setSimplePriorityId(uiNumScalableLayer, uiSimplePriorityId);
					  pcScalableSEI->setDiscardableFlag(uiNumScalableLayer, uiDiscardableFlag);
			
					  pcScalableSEI->setTemporalLevel(uiNumScalableLayer, uiTempLevel);
					  pcScalableSEI->setDependencyId(uiNumScalableLayer, uiDependencyID);
					  pcScalableSEI->setQualityLevel(uiNumScalableLayer, uiQualityLevel);
				  }
 JVT-S036  */
				  if(pcScalableSEI->getBitrateInfoPresentFlag(uiNumScalableLayer))
				  {
// BUG_FIX liuhui{
					  UInt uiAvgBitrate = (UInt)( m_aaadSingleLayerBitrate[uiCurrLayer][uiCurrTempLevel][uiCurrFGSLevel]+0.5 );
// BUG_FIX liuhui}
					//JVT-S036  start
					  UInt uiMaxBitrateLayer = 0;	//should be changed
					  UInt uiMaxBitrateDecodedPicture = 0;	//should be changed
					  UInt uiMaxBitrateCalcWindow = 0; //should be changed

					  pcScalableSEI->setAvgBitrate(uiNumScalableLayer, uiAvgBitrate);
					  pcScalableSEI->setMaxBitrateLayer(uiNumScalableLayer, uiMaxBitrateLayer);
					  pcScalableSEI->setMaxBitrateDecodedPicture(uiNumScalableLayer, uiMaxBitrateDecodedPicture);
					  pcScalableSEI->setMaxBitrateCalcWindow(uiNumScalableLayer, uiMaxBitrateCalcWindow);
				    //JVT-S036  end
				  }

				  if(pcScalableSEI->getFrmRateInfoPresentFlag(uiNumScalableLayer))
				  {
					  UInt uiConstantFrmRateIdc = 0;
					  UInt uiAvgFrmRate = (UInt)( 256*dFramerate[uiTotalScalableLayer] + 0.5 );

					  pcScalableSEI->setConstantFrmRateIdc(uiNumScalableLayer, uiConstantFrmRateIdc);
					  pcScalableSEI->setAvgFrmRate(uiNumScalableLayer, uiAvgFrmRate);
				  }
				  else
				  {//JVT-S036 
					  UInt  bFrmRateInfoSrcLayerIdDelta = 0;  //may be changed

					  pcScalableSEI->setFrmRateInfoSrcLayerIdDelta(uiNumScalableLayer, bFrmRateInfoSrcLayerIdDelta);
				  }

				  if(pcScalableSEI->getFrmSizeInfoPresentFlag(uiNumScalableLayer))
				  {
					  UInt uiFrmWidthInMbsMinus1 = rcLayer.getFrameWidth()/16 - 1;
					  UInt uiFrmHeightInMbsMinus1 = rcLayer.getFrameHeight()/16 - 1;

					  pcScalableSEI->setFrmWidthInMbsMinus1(uiNumScalableLayer, uiFrmWidthInMbsMinus1);
					  pcScalableSEI->setFrmHeightInMbsMinus1(uiNumScalableLayer, uiFrmHeightInMbsMinus1);
				  }
				  else
				  {//JVT-S036 
					  UInt  bFrmSizeInfoSrcLayerIdDelta = 0;  //may be changed

					  pcScalableSEI->setFrmSizeInfoSrcLayerIdDelta(uiNumScalableLayer, bFrmSizeInfoSrcLayerIdDelta);
				  }

				  if(pcScalableSEI->getSubRegionLayerFlag(uiNumScalableLayer))
				  {
					  UInt uiBaseRegionLayerId = 0;
					  Bool bDynamicRectFlag = false;

					  pcScalableSEI->setBaseRegionLayerId(uiNumScalableLayer, uiBaseRegionLayerId);
					  pcScalableSEI->setDynamicRectFlag(uiNumScalableLayer, bDynamicRectFlag);
					  if(pcScalableSEI->getDynamicRectFlag(uiNumScalableLayer))
					  {
						  UInt uiHorizontalOffset = 0;
						  UInt uiVerticalOffset = 0;
						  UInt uiRegionWidth = 0;
						  UInt uiRegionHeight = 0;
						  pcScalableSEI->setHorizontalOffset(uiNumScalableLayer, uiHorizontalOffset);
						  pcScalableSEI->setVerticalOffset(uiNumScalableLayer, uiVerticalOffset);
						  pcScalableSEI->setRegionWidth(uiNumScalableLayer, uiRegionWidth);
						  pcScalableSEI->setRegionHeight(uiNumScalableLayer, uiRegionHeight);
					  }
				  }
				 else
				  {//JVT-S036 
					  UInt  bSubRegionInfoSrcLayerIdDelta = 0; //may be changed

					  pcScalableSEI->setSubRegionInfoSrcLayerIdDelta(uiNumScalableLayer, bSubRegionInfoSrcLayerIdDelta);
				  }

			  //JVT-S036  start
				  if( pcScalableSEI->getSubPicLayerFlag( uiNumScalableLayer ) )
				  {
					  UInt RoiId = 1;//should be changed
					  pcScalableSEI->setRoiId( uiNumScalableLayer, RoiId );
				  }
				  if( pcScalableSEI->getIroiSliceDivisionInfoPresentFlag( uiNumScalableLayer ) )
				  {
            // JVT-S054 (REPLACE) ->
            /*
					  UInt bIroiSliceDivisionType = 0; //may be changed
					  UInt bNumSliceMinus1 = 1;
					  pcScalableSEI->setIroiSliceDivisionType( uiNumScalableLayer, bIroiSliceDivisionType );
					  if( bIroiSliceDivisionType == 0 )
					  {
						  UInt bGridSliceWidthInMbsMinus1 = 0; //should be changed
						  UInt bGridSliceHeightInMbsMinus1 = 0; //should be changed
						  pcScalableSEI->setGridSliceWidthInMbsMinus1( uiNumScalableLayer, bGridSliceWidthInMbsMinus1 );
						  pcScalableSEI->setGridSliceHeightInMbsMinus1( uiNumScalableLayer, bGridSliceHeightInMbsMinus1 );
					  }
					  else if( bIroiSliceDivisionType == 1 )
					  {
						  bNumSliceMinus1 = 1; //should be changed
						  pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, bNumSliceMinus1);
						  for ( j = 0; j <= bNumSliceMinus1; j++ )
						  {
							  UInt bFirstMbInSlice = 1;//should be changed
							  UInt bSliceWidthInMbsMinus1 = 1;//should be changed
							  UInt bSliceHeightInMbsMinus1 = 1;//should be changed
							  pcScalableSEI->setFirstMbInSlice( uiNumScalableLayer, j, bFirstMbInSlice );
							  pcScalableSEI->setSliceWidthInMbsMinus1( uiNumScalableLayer, j, bSliceWidthInMbsMinus1 );
							  pcScalableSEI->setSliceHeightInMbsMinus1( uiNumScalableLayer, j, bSliceHeightInMbsMinus1 );
						  }
					  }
					  else if( bIroiSliceDivisionType == 2 )
					  {
						  pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, bNumSliceMinus1 );
						  UInt uiFrameHeightInMb = pcScalableSEI->getFrmHeightInMbsMinus1( uiNumScalableLayer ) + 1;
						  UInt uiFrameWidthInMb  = pcScalableSEI->getFrmWidthInMbsMinus1(uiNumScalableLayer ) + 1;
						  UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
						  for ( j = 0; j < uiPicSizeInMbs; j++)
						  {
							  UInt bSliceId = 1; //should be changed
							  pcScalableSEI->setSliceId( uiNumScalableLayer, j, bSliceId );
						  }
					  }
            */
					  pcScalableSEI->setIroiSliceDivisionType( uiNumScalableLayer, rcLayer.m_uiSliceDivisionType );
            if (rcLayer.m_uiSliceDivisionType == 0)
					  {
						  pcScalableSEI->setGridSliceWidthInMbsMinus1( uiNumScalableLayer, rcLayer.m_puiGridSliceWidthInMbsMinus1[0] );
						  pcScalableSEI->setGridSliceHeightInMbsMinus1( uiNumScalableLayer, rcLayer.m_puiGridSliceHeightInMbsMinus1[0] );
					  }
            else if (rcLayer.m_uiSliceDivisionType == 1)
					  {
						  pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, rcLayer.m_uiNumSliceMinus1 );
						  for ( j = 0; j <= rcLayer.m_uiNumSliceMinus1; j++ )
						  {
							  pcScalableSEI->setFirstMbInSlice( uiNumScalableLayer, j, rcLayer.m_puiFirstMbInSlice[j] );
							  pcScalableSEI->setSliceWidthInMbsMinus1( uiNumScalableLayer, j, rcLayer.m_puiGridSliceWidthInMbsMinus1[j] );
							  pcScalableSEI->setSliceHeightInMbsMinus1( uiNumScalableLayer, j, rcLayer.m_puiGridSliceHeightInMbsMinus1[j] );
						  }
					  }
            else if (rcLayer.m_uiSliceDivisionType == 2)
					  {
						  pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, rcLayer.m_uiNumSliceMinus1 );
						  UInt uiFrameHeightInMb = pcScalableSEI->getFrmHeightInMbsMinus1( uiNumScalableLayer ) + 1;
						  UInt uiFrameWidthInMb  = pcScalableSEI->getFrmWidthInMbsMinus1(uiNumScalableLayer ) + 1;
						  UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
						  for ( j = 0; j < uiPicSizeInMbs; j++)
						  {
							  pcScalableSEI->setSliceId( uiNumScalableLayer, j, rcLayer.m_puiSliceId[j] );
						  }
					  }
            // JVT-S054 (REPLACE) <-
				  }
			  //JVT-S036  end

				  if(pcScalableSEI->getLayerDependencyInfoPresentFlag(uiNumScalableLayer))
				  {
// BUG_FIX liuhui{
					{
					  UInt uiDelta;
					  if( uiCurrFGSLevel ) // FGS layer, Q != 0
					  {
					    uiDelta = uiNumScalableLayer - getScalableLayerId( uiCurrLayer, uiCurrTempLevel, uiCurrFGSLevel-1 );
						pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta );//JVT-S036 
						pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 1 );
						if( uiCurrTempLevel- uiMinTempLevel ) // T != 0
						{
						  uiDelta = uiNumScalableLayer - getScalableLayerId( uiCurrLayer, uiCurrTempLevel-1, uiCurrFGSLevel );
						  pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta );//JVT-S036 
						  pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 2 );
						}
					  }
					  else if( ( uiCurrTempLevel- uiMinTempLevel ) ) // Q = 0, T != 0					    
					  {
					    uiDelta = uiNumScalableLayer - getScalableLayerId( uiCurrLayer, uiCurrTempLevel-1, uiCurrFGSLevel );
						pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta ); //JVT-S036 
						pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 1 );
						if( uiCurrLayer ) // D != 0, T != 0, Q = 0
						{
						  UInt uiBaseLayerId = rcLayer.getBaseLayerId();
						  LayerParameters& rcBaseLayer = m_pcCodingParameter->getLayerParameters ( uiBaseLayerId );
						  UInt uiBaseFGSLayers = (UInt)( rcBaseLayer.getNumFGSLayers() );
						  UInt uiBaseQualityLevel = rcLayer.getBaseQualityLevel();
						  uiBaseQualityLevel = min( uiBaseQualityLevel, uiBaseFGSLayers );
						  if( uiBaseLayerId == 0 && m_pcCodingParameter->getBaseLayerMode() == 1 ) // AVC-COMPATIBLE
						  {
						    UInt uiBaseTempLevel = max( 0, rcBaseLayer.getDecompositionStages() - rcBaseLayer.getNotCodedMCTFStages() - 1 );
							if( uiCurrTempLevel-uiMinTempLevel >= uiBaseTempLevel )
							{
							  if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel ) )
							  {
							  uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel );
							   pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta );//JVT-S036 
							  pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
							  }
							}
							else 
							{
							  if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerId, uiBaseTempLevel, uiBaseQualityLevel ) )
							  { //this should always be true
							    uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, uiBaseTempLevel, uiBaseQualityLevel );
							    pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta ); //JVT-S036 
							    pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
							  }
							}
						  }
						  else //non-AVC mode
						  {
						    if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel ) )
							{
						      uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel );
						      pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta ); //JVT-S036 
						      pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
							}
						  }
						}
					  }
					  else if ( uiCurrLayer ) // D != 0,T = 0, Q = 0
					  {
						UInt uiBaseLayerId = rcLayer.getBaseLayerId();
						LayerParameters& rcBaseLayer = m_pcCodingParameter->getLayerParameters ( uiBaseLayerId );
						UInt uiBaseFGSLayers = (UInt)( rcBaseLayer.getNumFGSLayers() );
						UInt uiBaseQualityLevel = rcLayer.getBaseQualityLevel();
						uiBaseQualityLevel = min( uiBaseQualityLevel, uiBaseFGSLayers );
						if( uiBaseLayerId == 0 && m_pcCodingParameter->getBaseLayerMode() == 1 ) //AVC-COMPATIBLE
						{
						  Int iBaseTempLevel = max( 0, (Int)( rcBaseLayer.getDecompositionStages() - rcBaseLayer.getNotCodedMCTFStages() ) - 1 );
						  uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, (UInt)iBaseTempLevel, (UInt)uiBaseQualityLevel );
						}
						else
						  uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel );
						pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta ); //JVT-S036 
						pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 1 );
					  }
				      else // base layer, no dependency layers
					  {
						pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 0 );
					  }
					}
// BUG_FIX liuhui}
				  }
				  else
				  {//JVT-S036 
					  UInt uiLayerDependencyInfoSrcLayerIdDelta = 0; //may be changed

					  pcScalableSEI->setLayerDependencyInfoSrcLayerIdDelta( uiNumScalableLayer, uiLayerDependencyInfoSrcLayerIdDelta);
				  }

				  if(pcScalableSEI->getInitParameterSetsInfoPresentFlag(uiNumScalableLayer))
				  {
        	  UInt uiNumInitSPSMinus1 = 0;	//should be changed
					  UInt uiNumInitPPSMinus1 = 0;	//should be changed
					  pcScalableSEI->setNumInitSeqParameterSetMinus1(uiNumScalableLayer, uiNumInitSPSMinus1);
					  pcScalableSEI->setNumInitPicParameterSetMinus1(uiNumScalableLayer, uiNumInitPPSMinus1);
					  for( j = 0; j <= pcScalableSEI->getNumInitSPSMinus1(uiNumScalableLayer); j++)
					  {
						  UInt uiDelta = 0; //should be changed
						  pcScalableSEI->setInitSeqParameterSetIdDelta( uiNumScalableLayer, j, uiDelta );
					  }
					  for( j = 0; j <= pcScalableSEI->getNumInitPPSMinus1(uiNumScalableLayer); j++)
					  {
						  UInt uiDelta = 0; //should be changed
						  pcScalableSEI->setInitPicParameterSetIdDelta( uiNumScalableLayer, j, uiDelta );
					  }
				  }
				  else
				  {//JVT-S036 
					  UInt bInitParameterSetsInfoSrcLayerIdDelta = 0;  //may be changed

					  pcScalableSEI->setInitParameterSetsInfoSrcLayerIdDelta( uiNumScalableLayer, bInitParameterSetsInfoSrcLayerIdDelta );
				  }

				  uiNumScalableLayer++;
				}
				uiTotalScalableLayer++;
			}
		}

	}

	UInt              uiBits = 0;
	SEI::MessageList  cSEIMessageList;
	cSEIMessageList.push_back                       ( pcScalableSEI );
	RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
	RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
	RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

	return Err::m_nOK;

}

//JVT-T054{
ErrVal
H264AVCEncoder::xWriteScalableSEICGSSNR( ExtBinDataAccessor* pcExtBinDataAccessor )
{

	//===== create message =====
	SEI::ScalableSei* pcScalableSEI;
	RNOK(SEI::ScalableSei::create(pcScalableSEI) );


	//===== set message =====
	UInt j; //JVT-S036 
	UInt uiInputLayers = m_pcCodingParameter->getNumberOfLayers ();
	UInt uiLayerNum = 0;	//total scalable layer numbers
	for ( UInt i = 0; i < uiInputLayers; i++ )	//calculate total scalable layer numbers
	{
		Bool bH264AVCCompatible = ( i == 0 && m_pcCodingParameter->getBaseLayerMode() > 0 );
		Bool bSubSeq            = ( i == 0 && m_pcCodingParameter->getBaseLayerMode() > 1 );

		LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( i );
		UInt uiTotalTempLevel = rcLayer.getDecompositionStages () - rcLayer.getNotCodedMCTFStages();
		UInt uiMinTempLevel   = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max( 0, (Int)uiTotalTempLevel - 1 );
		UInt uiActTempLevel   = uiTotalTempLevel - uiMinTempLevel + 1;
		//UInt uiTotalFGSLevel  = (UInt)rcLayer.getNumFGSLayers () + 1; //mwi. variable not used.
		uiLayerNum += uiActTempLevel;

		pcScalableSEI->setROINum ( i, rcLayer.getNumROI() );
		pcScalableSEI->setROIID  ( i, rcLayer.getROIID() );
		pcScalableSEI->setSGID  ( i, rcLayer.getSGID() );
		pcScalableSEI->setSLID  ( i, rcLayer.getSLID() );
	}
	UInt uiTotalScalableLayer = 0;

	//===== get framerate information ===
	Double *dFramerate = dGetFramerate();
  
	UInt uiNumLayersMinus1 = uiLayerNum - 1;

	pcScalableSEI->setNumLayersMinus1 ( uiNumLayersMinus1 );


	UInt uiNumScalableLayer = 0;
	for ( UInt uiCurrLayer = 0; uiCurrLayer < uiInputLayers; uiCurrLayer++)
	{
		LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( uiCurrLayer );
		UInt uiTotalTempLevel = rcLayer.getDecompositionStages () - rcLayer.getNotCodedMCTFStages() + 1;
		//UInt uiTotalFGSLevel = (UInt)rcLayer.getNumFGSLayers () + 1;//mwi. variable not used.
		Bool bH264AVCCompatible = ( uiCurrLayer == 0 && m_pcCodingParameter->getBaseLayerMode() > 0 );
		Bool bSubSeq            = ( uiCurrLayer == 0 && m_pcCodingParameter->getBaseLayerMode() > 1 );
		UInt uiMinTempLevel     = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max(0, (Int)uiTotalTempLevel - 2);

		for ( UInt uiCurrTempLevel = 0; uiCurrTempLevel < uiTotalTempLevel; uiCurrTempLevel++ )
		{
			//for ( UInt uiCurrFGSLevel = 0; uiCurrFGSLevel < uiTotalFGSLevel; uiCurrFGSLevel++ )
			{
				if( uiCurrTempLevel >= uiMinTempLevel )
				{
				  //Bool bSubPicLayerFlag = false;
				  Bool bSubRegionLayerFlag = false;
				  Bool bProfileLevelInfoPresentFlag = false;
				  Bool bInitParameterSetsInfoPresentFlag = false;		//may be changed  //JVT-S036 
				  if( uiNumScalableLayer == 0 )
				 {//JVT-S036 
					 bSubRegionLayerFlag = true;
					 bProfileLevelInfoPresentFlag = true;
					 bInitParameterSetsInfoPresentFlag = true;		
				 }
				  Bool bBitrateInfoPresentFlag = true;
				  Bool bFrmRateInfoPresentFlag = true;//rcLayer.getInputFrameRate () > 0;
				  Bool bFrmSizeInfoPresentFlag = true;
// BUG_FIX liuhui{
				  Bool bLayerDependencyInfoPresentFlag = true;			//may be changed
// BUG_FIX liuhui}
				  Bool bExactInterayerPredFlag = true;			//JVT-S036  may be changed
          // JVT-S054 (REMOVE)
				  pcScalableSEI->setLayerId(uiNumScalableLayer, uiNumScalableLayer);
	//JVT-S036  start
					UInt uiTempLevel = uiCurrTempLevel; //BUG_FIX_FT_01_2006
          UInt uiDependencyID = rcLayer.getLayerCGSSNR();//uiCurrLayer;
          UInt uiQualityLevel = rcLayer.getQualityLevelCGSSNR();//uiCurrFGSLevel;
	// BUG_FIX liuhui{
					m_aaauiScalableLayerId[uiDependencyID][uiCurrTempLevel][uiQualityLevel] = uiNumScalableLayer;
	// BUG_FIX liuhui}					
					UInt uiSimplePriorityId = 0;
					Bool bDiscardableFlag  = false;
					//if( uiCurrFGSLevel > rcLayer.getNumFGSLayers() )
					//	bDiscardableFlag = true;
          if(rcLayer.isDiscardable())
            bDiscardableFlag = true;
					pcScalableSEI->setSimplePriorityId(uiNumScalableLayer, uiSimplePriorityId);
					pcScalableSEI->setDiscardableFlag(uiNumScalableLayer, bDiscardableFlag);
					pcScalableSEI->setTemporalLevel(uiNumScalableLayer, uiTempLevel);
					pcScalableSEI->setDependencyId(uiNumScalableLayer, uiDependencyID);
					pcScalableSEI->setQualityLevel(uiNumScalableLayer, uiQualityLevel);				
	 //JVT-S036  end
				  pcScalableSEI->setSubRegionLayerFlag(uiNumScalableLayer, bSubRegionLayerFlag);
          // JVT-S054 (REPLACE)
				  pcScalableSEI->setIroiSliceDivisionInfoPresentFlag(uiNumScalableLayer, rcLayer.m_bSliceDivisionFlag);
				  pcScalableSEI->setProfileLevelInfoPresentFlag(uiNumScalableLayer, bProfileLevelInfoPresentFlag);
				  pcScalableSEI->setBitrateInfoPresentFlag(uiNumScalableLayer, bBitrateInfoPresentFlag);
				  pcScalableSEI->setFrmRateInfoPresentFlag(uiNumScalableLayer, bFrmRateInfoPresentFlag);
				  pcScalableSEI->setFrmSizeInfoPresentFlag(uiNumScalableLayer, bFrmSizeInfoPresentFlag);
				  pcScalableSEI->setLayerDependencyInfoPresentFlag(uiNumScalableLayer, bLayerDependencyInfoPresentFlag);
				  pcScalableSEI->setInitParameterSetsInfoPresentFlag(uiNumScalableLayer, bInitParameterSetsInfoPresentFlag);

				  pcScalableSEI->setExactInterlayerPredFlag(uiNumScalableLayer, bExactInterayerPredFlag);//JVT-S036 

				  if(pcScalableSEI->getProfileLevelInfoPresentFlag(uiNumScalableLayer))
				  {
					  UInt uilayerProfileIdc = 0;	//may be changed
					  Bool bLayerConstraintSet0Flag = false;	//may be changed
					  Bool bH264AVCCompatibleTmp  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiCurrLayer == 0;
					  Bool bLayerConstraintSet1Flag = ( bH264AVCCompatibleTmp ? 1 : 0 );	//may be changed
					  Bool bLayerConstraintSet2Flag = false;	//may be changed
					  Bool bLayerConstraintSet3Flag = false;	//may be changed
					  UInt uiLayerLevelIdc = 0;		//may be changed

					  pcScalableSEI->setLayerProfileIdc(uiNumScalableLayer, uilayerProfileIdc);
					  pcScalableSEI->setLayerConstraintSet0Flag(uiNumScalableLayer, bLayerConstraintSet0Flag);
					  pcScalableSEI->setLayerConstraintSet1Flag(uiNumScalableLayer, bLayerConstraintSet1Flag);
					  pcScalableSEI->setLayerConstraintSet2Flag(uiNumScalableLayer, bLayerConstraintSet2Flag);
					  pcScalableSEI->setLayerConstraintSet3Flag(uiNumScalableLayer, bLayerConstraintSet3Flag);
					  pcScalableSEI->setLayerLevelIdc(uiNumScalableLayer, uiLayerLevelIdc);
				  }
				  else
				  {//JVT-S036 
					  UInt bProfileLevelInfoSrcLayerIdDelta = 0;  //may be changed

					  pcScalableSEI->setProfileLevelInfoSrcLayerIdDelta(uiNumScalableLayer, bProfileLevelInfoSrcLayerIdDelta);
				  }


				  if(pcScalableSEI->getBitrateInfoPresentFlag(uiNumScalableLayer))
				  {
// BUG_FIX liuhui{
					  UInt uiAvgBitrate = (UInt)( m_aaadSingleLayerBitrate[uiCurrLayer][uiCurrTempLevel][0]+0.5 );
// BUG_FIX liuhui}
					//JVT-S036  start
					  UInt uiMaxBitrateLayer = 0;	//should be changed
					  UInt uiMaxBitrateDecodedPicture = 0;	//should be changed
					  UInt uiMaxBitrateCalcWindow = 0; //should be changed

					  pcScalableSEI->setAvgBitrate(uiNumScalableLayer, uiAvgBitrate);
					  pcScalableSEI->setMaxBitrateLayer(uiNumScalableLayer, uiMaxBitrateLayer);
					  pcScalableSEI->setMaxBitrateDecodedPicture(uiNumScalableLayer, uiMaxBitrateDecodedPicture);
					  pcScalableSEI->setMaxBitrateCalcWindow(uiNumScalableLayer, uiMaxBitrateCalcWindow);
				    //JVT-S036  end
				  }

				  if(pcScalableSEI->getFrmRateInfoPresentFlag(uiNumScalableLayer))
				  {
					  UInt uiConstantFrmRateIdc = 0;
					  UInt uiAvgFrmRate = (UInt)( 256*dFramerate[uiTotalScalableLayer] + 0.5 );

					  pcScalableSEI->setConstantFrmRateIdc(uiNumScalableLayer, uiConstantFrmRateIdc);
					  pcScalableSEI->setAvgFrmRate(uiNumScalableLayer, uiAvgFrmRate);
				  }
				  else
				  {//JVT-S036 
					  UInt  bFrmRateInfoSrcLayerIdDelta = 0;  //may be changed

					  pcScalableSEI->setFrmRateInfoSrcLayerIdDelta(uiNumScalableLayer, bFrmRateInfoSrcLayerIdDelta);
				  }

				  if(pcScalableSEI->getFrmSizeInfoPresentFlag(uiNumScalableLayer))
				  {
					  UInt uiFrmWidthInMbsMinus1 = rcLayer.getFrameWidth()/16 - 1;
					  UInt uiFrmHeightInMbsMinus1 = rcLayer.getFrameHeight()/16 - 1;

					  pcScalableSEI->setFrmWidthInMbsMinus1(uiNumScalableLayer, uiFrmWidthInMbsMinus1);
					  pcScalableSEI->setFrmHeightInMbsMinus1(uiNumScalableLayer, uiFrmHeightInMbsMinus1);
				  }
				  else
				  {//JVT-S036 
					  UInt  bFrmSizeInfoSrcLayerIdDelta = 0;  //may be changed

					  pcScalableSEI->setFrmSizeInfoSrcLayerIdDelta(uiNumScalableLayer, bFrmSizeInfoSrcLayerIdDelta);
				  }

				  if(pcScalableSEI->getSubRegionLayerFlag(uiNumScalableLayer))
				  {
					  UInt uiBaseRegionLayerId = 0;
					  Bool bDynamicRectFlag = false;

					  pcScalableSEI->setBaseRegionLayerId(uiNumScalableLayer, uiBaseRegionLayerId);
					  pcScalableSEI->setDynamicRectFlag(uiNumScalableLayer, bDynamicRectFlag);
					  if(pcScalableSEI->getDynamicRectFlag(uiNumScalableLayer))
					  {
						  UInt uiHorizontalOffset = 0;
						  UInt uiVerticalOffset = 0;
						  UInt uiRegionWidth = 0;
						  UInt uiRegionHeight = 0;
						  pcScalableSEI->setHorizontalOffset(uiNumScalableLayer, uiHorizontalOffset);
						  pcScalableSEI->setVerticalOffset(uiNumScalableLayer, uiVerticalOffset);
						  pcScalableSEI->setRegionWidth(uiNumScalableLayer, uiRegionWidth);
						  pcScalableSEI->setRegionHeight(uiNumScalableLayer, uiRegionHeight);
					  }
				  }
				 else
				  {//JVT-S036 
					  UInt  bSubRegionInfoSrcLayerIdDelta = 0; //may be changed

					  pcScalableSEI->setSubRegionInfoSrcLayerIdDelta(uiNumScalableLayer, bSubRegionInfoSrcLayerIdDelta);
				  }

			  //JVT-S036  start
				  if( pcScalableSEI->getSubPicLayerFlag( uiNumScalableLayer ) )
				  {
					  UInt RoiId = 1;//should be changed
					  pcScalableSEI->setRoiId( uiNumScalableLayer, RoiId );
				  }
				  if( pcScalableSEI->getIroiSliceDivisionInfoPresentFlag( uiNumScalableLayer ) )
				  {
					  pcScalableSEI->setIroiSliceDivisionType( uiNumScalableLayer, rcLayer.m_uiSliceDivisionType );
            if (rcLayer.m_uiSliceDivisionType == 0)
					  {
						  pcScalableSEI->setGridSliceWidthInMbsMinus1( uiNumScalableLayer, rcLayer.m_puiGridSliceWidthInMbsMinus1[0] );
						  pcScalableSEI->setGridSliceHeightInMbsMinus1( uiNumScalableLayer, rcLayer.m_puiGridSliceHeightInMbsMinus1[0] );
					  }
            else if (rcLayer.m_uiSliceDivisionType == 1)
					  {
						  pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, rcLayer.m_uiNumSliceMinus1 );
						  for ( j = 0; j <= rcLayer.m_uiNumSliceMinus1; j++ )
						  {
							  pcScalableSEI->setFirstMbInSlice( uiNumScalableLayer, j, rcLayer.m_puiFirstMbInSlice[j] );
							  pcScalableSEI->setSliceWidthInMbsMinus1( uiNumScalableLayer, j, rcLayer.m_puiGridSliceWidthInMbsMinus1[j] );
							  pcScalableSEI->setSliceHeightInMbsMinus1( uiNumScalableLayer, j, rcLayer.m_puiGridSliceHeightInMbsMinus1[j] );
						  }
					  }
            else if (rcLayer.m_uiSliceDivisionType == 2)
					  {
						  pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, rcLayer.m_uiNumSliceMinus1 );
						  UInt uiFrameHeightInMb = pcScalableSEI->getFrmHeightInMbsMinus1( uiNumScalableLayer ) + 1;
						  UInt uiFrameWidthInMb  = pcScalableSEI->getFrmWidthInMbsMinus1(uiNumScalableLayer ) + 1;
						  UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
						  for ( j = 0; j < uiPicSizeInMbs; j++)
						  {
							  pcScalableSEI->setSliceId( uiNumScalableLayer, j, rcLayer.m_puiSliceId[j] );
						  }
					  }
            // JVT-S054 (REPLACE) <-
				  }
			  //JVT-S036  end

				  if(pcScalableSEI->getLayerDependencyInfoPresentFlag(uiNumScalableLayer))
				  {
// BUG_FIX liuhui{
					{
					  UInt uiDelta;
            if( rcLayer.getQualityLevelCGSSNR() ) // FGS layer, Q != 0
					  {
              if( rcLayer.getLayerCGSSNR() == 0 && m_pcCodingParameter->getBaseLayerMode() == 1 ) // AVC-COMPATIBLE
						  {
                LayerParameters& rcBaseLayer = m_pcCodingParameter->getLayerParameters ( 0 );
						    UInt uiBaseTempLevel = max( 0, rcBaseLayer.getDecompositionStages() - rcBaseLayer.getNotCodedMCTFStages() - 1 );
							if( uiCurrTempLevel-uiMinTempLevel >= uiBaseTempLevel )
							{
                uiDelta = uiNumScalableLayer - getScalableLayerId( rcLayer.getLayerCGSSNR(), uiCurrTempLevel, rcLayer.getQualityLevelCGSSNR()-1 );
						    pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta );//JVT-S036 
						    pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 1 );
              }
              else
              {
                uiDelta = uiNumScalableLayer - getScalableLayerId( rcLayer.getLayerCGSSNR(), uiBaseTempLevel, rcLayer.getQualityLevelCGSSNR()-1 );
						    pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta );//JVT-S036 
						    pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 1 );
              }
              }
              else
              {
                uiDelta = uiNumScalableLayer - getScalableLayerId( rcLayer.getLayerCGSSNR(), uiCurrTempLevel, rcLayer.getQualityLevelCGSSNR()-1 );
						    pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta );//JVT-S036 
						    pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 1 );
              }
              
						if( uiCurrTempLevel- uiMinTempLevel ) // T != 0
						{
						  uiDelta = uiNumScalableLayer - getScalableLayerId( rcLayer.getLayerCGSSNR(), uiCurrTempLevel-1, rcLayer.getQualityLevelCGSSNR() );
						  pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta );//JVT-S036 
						  pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 2 );
						}
					  }
					  else if( ( uiCurrTempLevel- uiMinTempLevel ) ) // Q = 0, T != 0					    
					  {
					    uiDelta = uiNumScalableLayer - getScalableLayerId( rcLayer.getLayerCGSSNR(), uiCurrTempLevel-1, rcLayer.getQualityLevelCGSSNR() );
						pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta ); //JVT-S036 
						pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 1 );
						if( rcLayer.getLayerCGSSNR() ) // D != 0, T != 0, Q = 0
						{
						  UInt uiBaseLayerId = rcLayer.getBaseLayerId();
              UInt uiBaseLayerCGSSNR = rcLayer.getBaseLayerCGSSNR();
						  LayerParameters& rcBaseLayer = m_pcCodingParameter->getLayerParameters ( uiBaseLayerId );
						  //UInt uiBaseFGSLayers = (UInt)( rcBaseLayer.getNumFGSLayers() );
						  UInt uiBaseQualityLevel = rcLayer.getBaseQualityLevelCGSSNR();
						  //uiBaseQualityLevel = min( uiBaseQualityLevel, uiBaseFGSLayers );
						  if( uiBaseLayerCGSSNR == 0 && m_pcCodingParameter->getBaseLayerMode() == 1 ) // AVC-COMPATIBLE
						  {
						    UInt uiBaseTempLevel = max( 0, rcBaseLayer.getDecompositionStages() - rcBaseLayer.getNotCodedMCTFStages() - 1 );
							if( uiCurrTempLevel-uiMinTempLevel >= uiBaseTempLevel )
							{
							  if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerCGSSNR, uiCurrTempLevel, uiBaseQualityLevel ) )
							  {
							  uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerCGSSNR, uiCurrTempLevel, uiBaseQualityLevel );
							   pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta );//JVT-S036 
							  pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
							  }
							}
							else 
							{
							  if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerCGSSNR, uiBaseTempLevel, uiBaseQualityLevel ) )
							  { //this should always be true
							    uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerCGSSNR, uiBaseTempLevel, uiBaseQualityLevel );
							    pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta ); //JVT-S036 
							    pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
							  }
							}
						  }
						  else //non-AVC mode
						  {
						    if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel ) )
							{
						      uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel );
						      pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta ); //JVT-S036 
						      pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
							}
						  }
						}
					  }
            else if ( rcLayer.getLayerCGSSNR() ) // D != 0,T = 0, Q = 0
					  {
						UInt uiBaseLayerId = rcLayer.getBaseLayerId();
						UInt uiBaseLayerCGSSNR = rcLayer.getBaseLayerCGSSNR();
						LayerParameters& rcBaseLayer = m_pcCodingParameter->getLayerParameters ( uiBaseLayerId );
						//UInt uiBaseFGSLayers = (UInt)( rcBaseLayer.getNumFGSLayers() );
						UInt uiBaseQualityLevel = rcLayer.getBaseQualityLevelCGSSNR();
						//uiBaseQualityLevel = min( uiBaseQualityLevel, uiBaseFGSLayers );
						if( uiBaseLayerCGSSNR == 0 && m_pcCodingParameter->getBaseLayerMode() == 1 ) //AVC-COMPATIBLE
						{
						  Int iBaseTempLevel = max( 0, (Int)( rcBaseLayer.getDecompositionStages() - rcBaseLayer.getNotCodedMCTFStages() ) - 1 );
						  uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerCGSSNR, (UInt)iBaseTempLevel, (UInt)uiBaseQualityLevel );
						}
						else
						  uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerCGSSNR, uiCurrTempLevel, uiBaseQualityLevel );
						pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta ); //JVT-S036 
						pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 1 );
					  }
				      else // base layer, no dependency layers
					  {
						pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 0 );
					  }
					}
// BUG_FIX liuhui}
				  }
				  else
				  {//JVT-S036 
					  UInt uiLayerDependencyInfoSrcLayerIdDelta = 0; //may be changed

					  pcScalableSEI->setLayerDependencyInfoSrcLayerIdDelta( uiNumScalableLayer, uiLayerDependencyInfoSrcLayerIdDelta);
				  }

				  if(pcScalableSEI->getInitParameterSetsInfoPresentFlag(uiNumScalableLayer))
				  {
        	  UInt uiNumInitSPSMinus1 = 0;	//should be changed
					  UInt uiNumInitPPSMinus1 = 0;	//should be changed
					  pcScalableSEI->setNumInitSeqParameterSetMinus1(uiNumScalableLayer, uiNumInitSPSMinus1);
					  pcScalableSEI->setNumInitPicParameterSetMinus1(uiNumScalableLayer, uiNumInitPPSMinus1);
					  for( j = 0; j <= pcScalableSEI->getNumInitSPSMinus1(uiNumScalableLayer); j++)
					  {
						  UInt uiDelta = 0; //should be changed
						  pcScalableSEI->setInitSeqParameterSetIdDelta( uiNumScalableLayer, j, uiDelta );
					  }
					  for( j = 0; j <= pcScalableSEI->getNumInitPPSMinus1(uiNumScalableLayer); j++)
					  {
						  UInt uiDelta = 0; //should be changed
						  pcScalableSEI->setInitPicParameterSetIdDelta( uiNumScalableLayer, j, uiDelta );
					  }
				  }
				  else
				  {//JVT-S036 
					  UInt bInitParameterSetsInfoSrcLayerIdDelta = 0;  //may be changed

					  pcScalableSEI->setInitParameterSetsInfoSrcLayerIdDelta( uiNumScalableLayer, bInitParameterSetsInfoSrcLayerIdDelta );
				  }

				  uiNumScalableLayer++;
				}
				uiTotalScalableLayer++;
			}
		}

	}

	UInt              uiBits = 0;
	SEI::MessageList  cSEIMessageList;
	cSEIMessageList.push_back                       ( pcScalableSEI );
	RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
	RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
	RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

	return Err::m_nOK;
}
//JVT-T054}

// JVT-S080 LMI {
ErrVal
H264AVCEncoder::xWriteScalableSEILayersNotPresent( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiNumLayers, UInt* uiLayerId)
{
	UInt i, uiBits = 0;
	SEI::MessageList  cSEIMessageList;
	SEI::ScalableSeiLayersNotPresent* pcScalableSeiLayersNotPresent;
	RNOK(SEI::ScalableSeiLayersNotPresent::create(pcScalableSeiLayersNotPresent) );
	pcScalableSeiLayersNotPresent->setNumLayers( uiNumLayers );
	for (i=0; i < uiNumLayers; i++)
	pcScalableSeiLayersNotPresent->setLayerId( i, uiLayerId[i] );
	cSEIMessageList.push_back                       ( pcScalableSeiLayersNotPresent );
	RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
	RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
	RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

	return Err::m_nOK;
}

ErrVal
H264AVCEncoder::xWriteScalableSEIDependencyChange( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiNumLayers, UInt* uiLayerId, Bool* pbLayerDependencyInfoPresentFlag, 
												  UInt* uiNumDirectDependentLayers, UInt** puiDirectDependentLayerIdDeltaMinus1, UInt* puiLayerDependencyInfoSrcLayerIdDeltaMinus1)
{
	UInt uiBits = 0;
	SEI::MessageList  cSEIMessageList;
	SEI::ScalableSeiDependencyChange* pcScalableSeiDependencyChange;
	RNOK(SEI::ScalableSeiDependencyChange::create(pcScalableSeiDependencyChange) );
	pcScalableSeiDependencyChange->setNumLayersMinus1(uiNumLayers-1);
    UInt uiLayer, uiDirectLayer;

	for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
	{
		pcScalableSeiDependencyChange->setLayerId( uiLayer, uiLayerId[uiLayer]);
		pcScalableSeiDependencyChange->setLayerDependencyInfoPresentFlag( uiLayer, pbLayerDependencyInfoPresentFlag[uiLayer] );
		if ( pcScalableSeiDependencyChange->getLayerDependencyInfoPresentFlag( uiLayer ) )
		{
          pcScalableSeiDependencyChange->setNumDirectDependentLayers( uiLayer, uiNumDirectDependentLayers[uiLayer] );
		  for ( uiDirectLayer = 0; uiDirectLayer < pcScalableSeiDependencyChange->getNumDirectDependentLayers( uiLayer ); uiDirectLayer++)
              pcScalableSeiDependencyChange->setDirectDependentLayerIdDeltaMinus1( uiLayer, uiDirectLayer,  puiDirectDependentLayerIdDeltaMinus1[uiLayer][uiDirectLayer] );
		}
		else
            pcScalableSeiDependencyChange->setLayerDependencyInfoSrcLayerIdDeltaMinus1( uiLayer, puiLayerDependencyInfoSrcLayerIdDeltaMinus1[uiLayer] );
	}


	cSEIMessageList.push_back                       ( pcScalableSeiDependencyChange );
	RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
	RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
	RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

	return Err::m_nOK;
}
//  JVT-S080 LMI }

ErrVal
H264AVCEncoder::xWriteSubPicSEI ( ExtBinDataAccessor* pcExtBinDataAccessor )
{
	SEI::SubPicSei* pcSubPicSEI;
	RNOK( SEI::SubPicSei::create( pcSubPicSEI ) );

  //===== set message =====
	UInt uiScalableLayerId = 0;	//should be changed
	pcSubPicSEI->setLayerId( uiScalableLayerId );
  
	//===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcSubPicSEI );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

	return Err::m_nOK;
}


ErrVal
H264AVCEncoder::xWriteSubPicSEI ( ExtBinDataAccessor* pcExtBinDataAccessor, UInt layer_id )
{
	SEI::SubPicSei* pcSubPicSEI;
	RNOK( SEI::SubPicSei::create( pcSubPicSEI ) );

  //===== set message =====
	pcSubPicSEI->setLayerId( layer_id );
  
	//===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcSubPicSEI );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

	return Err::m_nOK;
}

// Scalable SEI for ROI ICU/ETRI
ErrVal
H264AVCEncoder::xWriteMotionSEI( ExtBinDataAccessor* pcExtBinDataAccessor, UInt sg_id ) 
{
  //===== create message =====
  SEI::MotionSEI* pcMotionSEI;
  RNOK( SEI::MotionSEI::create( pcMotionSEI) );

  pcMotionSEI->setSliceGroupId(sg_id);


  //===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcMotionSEI);
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

  return Err::m_nOK;
}

//JVT-W080
ErrVal
H264AVCEncoder::writePDSSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor
																  , const UInt uiSPSId
																	, const UInt uiNumView
																  , UInt* num_refs_list0_anc
																	, UInt* num_refs_list1_anc 
																  , UInt* num_refs_list0_nonanc
																	, UInt* num_refs_list1_nonanc  
																	, UInt  PDSInitialDelay_anc 
																	, UInt  PDSInitialDelay_nonanc
																	)
{
  RNOK( xWritePDSSEI( pcExtBinDataAccessor
		                , uiSPSId
		                , uiNumView
										, num_refs_list0_anc
										, num_refs_list1_anc
										, num_refs_list0_nonanc
										, num_refs_list1_nonanc
										, PDSInitialDelay_anc
										, PDSInitialDelay_nonanc
										)
			);
	return Err::m_nOK;
}

ErrVal
H264AVCEncoder::xWritePDSSEI(  ExtBinDataAccessor* pcExtBinDataAccessor
														 , const UInt uiSPSId
														 , const UInt uiNumView
														 , UInt* num_refs_list0_anc
														 , UInt* num_refs_list1_anc
														 , UInt* num_refs_list0_nonanc
														 , UInt* num_refs_list1_nonanc
														 , UInt  PDSInitialDelay_anc
														 , UInt  PDSInitialDelay_nonanc
														)
{
  //===== create message =====
	SEI::ParallelDecodingSEI* pcPdSEI;
	RNOK( SEI::ParallelDecodingSEI::create( pcPdSEI ) );

	//===== message initialization ===========  
	RNOK( pcPdSEI->init( uiSPSId, 
		uiNumView, 
		num_refs_list0_anc,
		num_refs_list1_anc, 
		num_refs_list0_nonanc, 
		num_refs_list1_nonanc,
		PDSInitialDelay_anc,
		PDSInitialDelay_nonanc ) );

	//change PDIInitialDelay here
	
	//save needed parameter to CodingParameter
	m_pcCodingParameter->savePDSParameters( pcPdSEI->getNumView()
		                                    , pcPdSEI->getNumRefAnchorFramesL0() 
																				, pcPdSEI->getNumRefAnchorFramesL1() 
		                                    , pcPdSEI->getNumNonRefAnchorFramesL0() 
																				, pcPdSEI->getNumNonRefAnchorFramesL1() 	
																				, pcPdSEI->getPDIInitDelayMinus2L0Anc()
																				, pcPdSEI->getPDIInitDelayMinus2L1Anc()
																				, pcPdSEI->getPDIInitDelayMinus2L0NonAnc()
																				, pcPdSEI->getPDIInitDelayMinus2L1NonAnc()
																				);

  //===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcPdSEI );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

  return Err::m_nOK;
}
//~JVT-W080

Bool    m_bWrteROISEI = true;
UInt    m_loop_roi_sei=0;


ErrVal
H264AVCEncoder::writeParameterSets( ExtBinDataAccessor* pcExtBinDataAccessor, Bool &rbMoreSets )
{
  if( m_bVeryFirstCall )
  {
    m_bVeryFirstCall = false;

    RNOK( xInitParameterSets() );
    if( m_bScalableSeiMessage )
    if(m_pcCodingParameter->getCGSSNRRefinement() )
    { 
      RNOK( xWriteScalableSEICGSSNR( pcExtBinDataAccessor ) ); //JVT-T054
    }
    else
    {
      RNOK( xWriteScalableSEI( pcExtBinDataAccessor ) );
    }

	LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( 0 );
	if (0 < rcLayer.getNumROI())
		m_bWrteROISEI = true;
	else
		m_bWrteROISEI = false;
    m_loop_roi_sei=0;

    return Err::m_nOK;
  }
  else
    m_bScalableSeiMessage = true;

  UInt uiNumLayer = m_pcCodingParameter->getNumberOfLayers();
	
  if(m_bWrteROISEI)
  {
	LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( m_loop_roi_sei/2 );
	{
	  if(((m_loop_roi_sei+1)/2) >= uiNumLayer )
	  {
		m_bWrteROISEI = false;
	  }

	  if(m_loop_roi_sei%2)
	  {			
		RNOK( xWriteMotionSEI( pcExtBinDataAccessor,rcLayer.getSGID()[0] ) );    m_loop_roi_sei++; return Err::m_nOK;
	  }
	  else
	  {
	    RNOK( xWriteSubPicSEI( pcExtBinDataAccessor, rcLayer.getSLID()[0] ) );    m_loop_roi_sei++; return Err::m_nOK;
	  }
    }		
  }
    
  UInt uiBits;

  if( ! m_cUnWrittenSPS.empty() )
  {
    RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
    SequenceParameterSet& rcSPS = *m_cUnWrittenSPS.front();

	   if( rcSPS.getMbAdaptiveFrameFieldFlag() )
    {
      rcSPS.setFrameMbsOnlyFlag( false );
    }

    RNOK( m_pcNalUnitEncoder->write( rcSPS ) );
    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

    if( m_pcCodingParameter->getNumberOfLayers() )
    {

    }
    
    m_cUnWrittenSPS.pop_front();
  }
  else
  {
    if( ! m_cUnWrittenPPS.empty() )
    {
      RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
      PictureParameterSet& rcPPS = *m_cUnWrittenPPS.front();
      RNOK( m_pcNalUnitEncoder->write( rcPPS ) )
      RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

      if( m_pcCodingParameter->getNumberOfLayers() )
      {
        UInt  uiSPSId = rcPPS.getSeqParameterSetId();
        SequenceParameterSet* pcSPS;
        RNOK( m_pcParameterSetMng->get( pcSPS, uiSPSId ) );


      }
      
      m_cUnWrittenPPS.pop_front();
    }
    else
    {
      AF();
      rbMoreSets = false;
      return Err::m_nERR;
    }
  }

  rbMoreSets = !(m_cUnWrittenSPS.empty() && m_cUnWrittenPPS.empty());
  return Err::m_nOK;
}



ErrVal
H264AVCEncoder::finish( ExtBinDataAccessorList&  rcExtBinDataAccessorList,
                        PicBufferList*           apcPicBufferOutputList,
                        PicBufferList*           apcPicBufferUnusedList,
                        UInt&                    ruiNumCodedFrames,
                        Double&                  rdHighestLayerOutputRate )
{
 
  m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList );

  printf("\n");

  return Err::m_nOK;
}




ErrVal
H264AVCEncoder::xInitParameterSets()
{
  UInt uiSPSId = 0;
  UInt uiPPSId = 0;
  UInt uiIndex;


  //===== determine values for POC calculation =====
  UInt uiMaxResolutionStages  = m_pcCodingParameter->getDecompositionStages();  
  UInt uiRequiredPocBits      = max( 4, 1 + (Int)ceil( log10( 1.0 + ( 1 << uiMaxResolutionStages ) ) / log10( 2.0 ) ) );


  //===== loop over layers =====
  for( uiIndex = 0; uiIndex < m_pcCodingParameter->getNumberOfLayers(); uiIndex++ )
  {
    //===== get configuration parameters =====
    LayerParameters&  rcLayerParameters   = m_pcCodingParameter->getLayerParameters( uiIndex );
    Bool              bH264AVCCompatible  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiIndex == 0;
    UInt              uiMbY               = rcLayerParameters.getFrameHeight() / 16;
    UInt              uiMbX               = rcLayerParameters.getFrameWidth () / 16;
    UInt              uiOutFreq           = (UInt)ceil( rcLayerParameters.getOutputFrameRate() );
    //UInt              uiMvRange           = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange() / 4;
	UInt              uiMvRange           = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange()*4 ;    
    UInt              uiDPBSize           = ( 1 << max( 1, rcLayerParameters.getDecompositionStages() ) );
    UInt              uiNumRefPic         = uiDPBSize; 
	UInt              uiLevelIdc          = SequenceParameterSet::getLevelIdc( uiMbY, uiMbX, uiOutFreq, uiMvRange, uiDPBSize, 1 );
    UInt              uiCropLeft          = 0;
    UInt              uiCropTop           = 0;
    UInt              uiCropRight         = rcLayerParameters.getHorPadding() / 2;                                            // chroma_format_idc is always equal to 1
    UInt              uiCropBottom        = rcLayerParameters.getVerPadding() / ( m_pcCodingParameter->isInterlaced() ? 4 : 2 ); // chroma_format_idc is always equal to 1
    
    ROT( bH264AVCCompatible && uiDPBSize > 16 );
    ROT( uiLevelIdc == MSYS_UINT_MAX );

    
    //===== create parameter sets, set Id's, and store =====
    SequenceParameterSet* pcSPS;
    PictureParameterSet*  pcPPSLP;
    PictureParameterSet*  pcPPSHP;
    
    RNOK( SequenceParameterSet::create( pcSPS   ) );
    RNOK( PictureParameterSet ::create( pcPPSHP ) );
    pcPPSHP->setPicParameterSetId( uiPPSId++ );
    pcPPSHP->setSeqParameterSetId( uiSPSId   );
    RNOK( m_pcParameterSetMng->store( pcPPSHP ) );
    if( rcLayerParameters.getContrainedIntraForLP() )
    {
      pcPPSLP = pcPPSHP;
    }
    else
    {
      RNOK( PictureParameterSet ::create( pcPPSLP ) );
      pcPPSLP->setPicParameterSetId( uiPPSId++ );
      pcPPSLP->setSeqParameterSetId( uiSPSId   );
      RNOK( m_pcParameterSetMng->store( pcPPSLP ) );
    }
    pcSPS->setSeqParameterSetId( uiSPSId++ );
    RNOK( m_pcParameterSetMng->store( pcSPS   ) );

    pcSPS->setMbAdaptiveFrameFieldFlag( (m_pcCodingParameter->getMbAff()?true:false) ); //th test
    if( pcSPS->getMbAdaptiveFrameFieldFlag() && uiMbY % 2)
    {
        printf(" mbaff ignored ");
    }
    pcSPS->setFrameMbsOnlyFlag( ! (m_pcCodingParameter->getMbAff() != 0 || m_pcCodingParameter->getPAff() != 0 ));


    //===== set sequence parameter set parameters =====
    pcSPS->setNalUnitType                         ( NAL_UNIT_SPS );
    pcSPS->setLayerId                             ( rcLayerParameters.getLayerId() );
    pcSPS->setProfileIdc                          ( bH264AVCCompatible ? ( rcLayerParameters.getAdaptiveTransform() > 0 ? HIGH_PROFILE : MAIN_PROFILE ) : SCALABLE_PROFILE );
    pcSPS->setConstrainedSet0Flag                 ( false );
    pcSPS->setConstrainedSet1Flag                 ( bH264AVCCompatible ? 1 : 0 );
    pcSPS->setConstrainedSet2Flag                 ( false );
    pcSPS->setConstrainedSet3Flag                 ( false );
    pcSPS->setLevelIdc                            ( uiLevelIdc );
    pcSPS->setSeqScalingMatrixPresentFlag         ( rcLayerParameters.getAdaptiveTransform() > 1 );
    pcSPS->setLog2MaxFrameNum                     ( MAX_FRAME_NUM_LOG2 );
    pcSPS->setLog2MaxPicOrderCntLsb               ( min( 15, uiRequiredPocBits + 2 ) );  // HS: decoder robustness -> value increased by 2
    pcSPS->setNumRefFrames                        ( uiNumRefPic );
    pcSPS->setRequiredFrameNumUpdateBehaviourFlag ( true );
    pcSPS->setFrameWidthInMbs                     ( uiMbX );
    pcSPS->setFrameHeightInMbs                    ( uiMbY );
    pcSPS->setDirect8x8InferenceFlag              ( true  );
    // TMM_ESS 
    pcSPS->setResizeParameters                    (rcLayerParameters.getResizeParameters());
    pcSPS->setCropOffset(uiCropLeft, uiCropRight, uiCropTop, uiCropBottom);

    if(rcLayerParameters.getFGSCodingMode() == 0)
    {
      pcSPS->setFGSCodingMode                     ( false );
    }
    else
    {
      pcSPS->setFGSCodingMode                     ( true );
    }
    pcSPS->setGroupingSize                        ( rcLayerParameters.getGroupingSize() );
    for( UInt ui = 0; ui < 16; ui++ )
    {
      pcSPS->setPosVect                           ( ui, rcLayerParameters.getPosVect(ui) );
    }

    //===== set picture parameter set parameters =====
    pcPPSHP->setNalUnitType                           ( NAL_UNIT_PPS );
    pcPPSHP->setLayerId                               ( rcLayerParameters.getLayerId() );
    pcPPSHP->setEntropyCodingModeFlag                 ( rcLayerParameters.getEntropyCodingModeFlag() );
    pcPPSHP->setPicOrderPresentFlag                   ( true );
    pcPPSHP->setNumRefIdxActive( LIST_0               , m_pcCodingParameter->getNumRefFrames() );
    pcPPSHP->setNumRefIdxActive( LIST_1               , m_pcCodingParameter->getNumRefFrames() );
    // heiko.schwarz@hhi.fhg.de: ensures that the PPS QP will be in the valid range (specified QP can be outside that range to force smaller/higher lambdas)
    //pcPPSHP->setPicInitQp                             ( (Int)rcLayerParameters.getBaseQpResidual() );
    pcPPSHP->setPicInitQp                             ( min( 51, max( 0, (Int)rcLayerParameters.getBaseQpResidual() ) ) );
    pcPPSHP->setChomaQpIndexOffset                    ( 0 );
    pcPPSHP->setDeblockingFilterParametersPresentFlag ( ! m_pcCodingParameter->getLoopFilterParams().isDefault() );
    pcPPSHP->setConstrainedIntraPredFlag              ( true );
    pcPPSHP->setRedundantPicCntPresentFlag            ( rcLayerParameters.getUseRedundantSliceFlag() ); // JVT-Q054 Red. Picture
    pcPPSHP->setTransform8x8ModeFlag                  ( rcLayerParameters.getAdaptiveTransform() > 0 );
    pcPPSHP->setPicScalingMatrixPresentFlag           ( false );
    pcPPSHP->set2ndChromaQpIndexOffset                ( 0 );

    pcPPSHP->setWeightedPredFlag                      ( WEIGHTED_PRED_FLAG );
    pcPPSHP->setWeightedBiPredIdc                     ( WEIGHTED_BIPRED_IDC );
//TMM_WP
    pcPPSHP->setWeightedPredFlag                   (m_pcCodingParameter->getIPMode()!=0);
    pcPPSHP->setWeightedBiPredIdc                  (m_pcCodingParameter->getBMode());  
//TMM_WP

	  //--ICU/ETRI FMO Implementation : FMO stuff start
	  pcPPSHP->setNumSliceGroupsMinus1                  (rcLayerParameters.getNumSliceGroupsMinus1());
	  pcPPSHP->setSliceGroupMapType                     (rcLayerParameters.getSliceGroupMapType());
	  pcPPSHP->setArrayRunLengthMinus1					      (rcLayerParameters.getArrayRunLengthMinus1());
	  pcPPSHP->setArrayTopLeft								  (rcLayerParameters.getArrayTopLeft());
	  pcPPSHP->setArrayBottomRight							  (rcLayerParameters.getArrayBottomRight());
	  pcPPSHP->setSliceGroupChangeDirection_flag		  (rcLayerParameters.getSliceGroupChangeDirection_flag());
	  pcPPSHP->setSliceGroupChangeRateMinus1			  (rcLayerParameters.getSliceGroupChangeRateMinus1());
	  pcPPSHP->setNumSliceGroupMapUnitsMinus1			  (rcLayerParameters.getNumSliceGroupMapUnitsMinus1());
	  pcPPSHP->setArraySliceGroupId						  (rcLayerParameters.getArraySliceGroupId());
	  //--ICU/ETRI FMO Implementation : FMO stuff end

    if( ! rcLayerParameters.getContrainedIntraForLP() )
    {
      pcPPSLP->setNalUnitType                           ( pcPPSHP->getNalUnitType                           ()  );
      pcPPSLP->setLayerId                               ( pcPPSHP->getLayerId                               ()  );
      pcPPSLP->setEntropyCodingModeFlag                 ( pcPPSHP->getEntropyCodingModeFlag                 ()  );
      pcPPSLP->setPicOrderPresentFlag                   ( pcPPSHP->getPicOrderPresentFlag                   ()  );
      pcPPSLP->setNumRefIdxActive( LIST_0               , pcPPSHP->getNumRefIdxActive               ( LIST_0 )  );
      pcPPSLP->setNumRefIdxActive( LIST_1               , pcPPSHP->getNumRefIdxActive               ( LIST_1 )  );
      pcPPSLP->setPicInitQp                             ( pcPPSHP->getPicInitQp                             ()  );
      pcPPSLP->setChomaQpIndexOffset                    ( pcPPSHP->getChomaQpIndexOffset                    ()  );
      pcPPSLP->setDeblockingFilterParametersPresentFlag ( pcPPSHP->getDeblockingFilterParametersPresentFlag ()  );
      pcPPSLP->setConstrainedIntraPredFlag              ( false                                                 );
      pcPPSLP->setRedundantPicCntPresentFlag            ( pcPPSHP->getRedundantPicCntPresentFlag            ()  );  //JVT-Q054 Red. Picture
      pcPPSLP->setTransform8x8ModeFlag                  ( pcPPSHP->getTransform8x8ModeFlag                  ()  );
      pcPPSLP->setPicScalingMatrixPresentFlag           ( pcPPSHP->getPicScalingMatrixPresentFlag           ()  );
      pcPPSLP->set2ndChromaQpIndexOffset                ( pcPPSHP->get2ndChromaQpIndexOffset                ()  );
      pcPPSLP->setWeightedPredFlag                      ( pcPPSHP->getWeightedPredFlag                      ()  );
      pcPPSLP->setWeightedBiPredIdc                     ( pcPPSHP->getWeightedBiPredIdc                     ()  );
    }

  	//--ICU/ETRI FMO Implementation : FMO stuff start
	  pcPPSLP->setNumSliceGroupsMinus1                  (rcLayerParameters.getNumSliceGroupsMinus1());
	  pcPPSLP->setSliceGroupMapType                     (rcLayerParameters.getSliceGroupMapType());
	  pcPPSLP->setArrayRunLengthMinus1					      (rcLayerParameters.getArrayRunLengthMinus1());
	  pcPPSLP->setArrayTopLeft								  (rcLayerParameters.getArrayTopLeft());
	  pcPPSLP->setArrayBottomRight							  (rcLayerParameters.getArrayBottomRight());
	  pcPPSLP->setSliceGroupChangeDirection_flag		  (rcLayerParameters.getSliceGroupChangeDirection_flag());
	  pcPPSLP->setSliceGroupChangeRateMinus1			  (rcLayerParameters.getSliceGroupChangeRateMinus1());
	  pcPPSLP->setNumSliceGroupMapUnitsMinus1			  (rcLayerParameters.getNumSliceGroupMapUnitsMinus1());
	  pcPPSLP->setArraySliceGroupId						  (rcLayerParameters.getArraySliceGroupId());
	  //--ICU/ETRI FMO Implementation : FMO stuff end

    //===== initialization using parameter sets =====
    RNOK( m_pcControlMng->initParameterSets( *pcSPS, *pcPPSLP, *pcPPSHP ) );
  }


  uiIndex = 0;
  LayerParameters&  rcLayerParameters   = m_pcCodingParameter->getLayerParameters( uiIndex );
  Bool              bH264AVCCompatible  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiIndex == 0;
  if(bH264AVCCompatible && m_pcCodingParameter->getNumberOfLayers() == 1 && rcLayerParameters.getNumFGSLayers() > 0)
  {
    UInt              uiMbY               = rcLayerParameters.getFrameHeight() / 16;
    UInt              uiMbX               = rcLayerParameters.getFrameWidth () / 16;
    UInt              uiOutFreq           = (UInt)ceil( rcLayerParameters.getOutputFrameRate() );
    //UInt              uiMvRange           = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange() / 4;
	UInt              uiMvRange           = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange()*4 ;    
    UInt              uiDPBSize           = ( 1 << max( 1, rcLayerParameters.getDecompositionStages() ) );
    UInt              uiNumRefPic         = uiDPBSize; 
	UInt              uiLevelIdc          = SequenceParameterSet::getLevelIdc( uiMbY, uiMbX, uiOutFreq, uiMvRange, uiDPBSize, 1 );
    
    ROT( bH264AVCCompatible && uiDPBSize > 16 );
    ROT( uiLevelIdc == MSYS_UINT_MAX );

    
    //===== create parameter sets, set Id's, and store =====
    SequenceParameterSet* pcSPS;
    PictureParameterSet*  pcPPSLP;
    PictureParameterSet*  pcPPSHP;
    
    RNOK( SequenceParameterSet::create( pcSPS   ) );
    RNOK( PictureParameterSet ::create( pcPPSHP ) );
    pcPPSHP->setPicParameterSetId( uiPPSId++ );
    pcPPSHP->setSeqParameterSetId( uiSPSId   );
    RNOK( m_pcParameterSetMng->store( pcPPSHP ) );
    if( rcLayerParameters.getContrainedIntraForLP() )
    {
      pcPPSLP = pcPPSHP;
    }
    else
    {
      RNOK( PictureParameterSet ::create( pcPPSLP ) );
      pcPPSLP->setPicParameterSetId( uiPPSId++ );
      pcPPSLP->setSeqParameterSetId( uiSPSId   );
      RNOK( m_pcParameterSetMng->store( pcPPSLP ) );
    }
    pcSPS->setSeqParameterSetId( uiSPSId++ );
    RNOK( m_pcParameterSetMng->store( pcSPS   ) );


    //===== set sequence parameter set parameters =====
    pcSPS->setNalUnitType                         ( NAL_UNIT_SPS );
    pcSPS->setLayerId                             ( rcLayerParameters.getLayerId() );
    pcSPS->setProfileIdc                          ( SCALABLE_PROFILE );
    pcSPS->setConstrainedSet0Flag                 ( false );
    pcSPS->setConstrainedSet1Flag                 ( bH264AVCCompatible ? 1 : 0 );
    pcSPS->setConstrainedSet2Flag                 ( false );
    pcSPS->setConstrainedSet3Flag                 ( false );
    pcSPS->setLevelIdc                            ( uiLevelIdc );
    pcSPS->setSeqScalingMatrixPresentFlag         ( rcLayerParameters.getAdaptiveTransform() > 1 );
    pcSPS->setLog2MaxFrameNum                     ( MAX_FRAME_NUM_LOG2 );
    pcSPS->setLog2MaxPicOrderCntLsb               ( min( 15, uiRequiredPocBits + 2 ) );  // HS: decoder robustness -> value increased by 2
    pcSPS->setNumRefFrames                        ( uiNumRefPic );
    pcSPS->setRequiredFrameNumUpdateBehaviourFlag ( true );
    pcSPS->setFrameWidthInMbs                     ( uiMbX );
    pcSPS->setFrameHeightInMbs                    ( uiMbY );
    pcSPS->setDirect8x8InferenceFlag              ( true  );
    // TMM_ESS 
    pcSPS->setResizeParameters                    (rcLayerParameters.getResizeParameters());

    if(rcLayerParameters.getFGSCodingMode() == 0)
    {
      pcSPS->setFGSCodingMode                     ( false );
    }
    else
    {
      pcSPS->setFGSCodingMode                     ( true );
    }
    pcSPS->setGroupingSize                        ( rcLayerParameters.getGroupingSize() );
    for( UInt ui = 0; ui < 16; ui++ )
    {
      pcSPS->setPosVect                           ( ui, rcLayerParameters.getPosVect(ui) );
    }

    //===== set picture parameter set parameters =====
    pcPPSHP->setNalUnitType                           ( NAL_UNIT_PPS );
    pcPPSHP->setLayerId                               ( rcLayerParameters.getLayerId() );
    pcPPSHP->setEntropyCodingModeFlag                 ( rcLayerParameters.getEntropyCodingModeFlag() );
    pcPPSHP->setPicOrderPresentFlag                   ( true );
    pcPPSHP->setNumRefIdxActive( LIST_0               , m_pcCodingParameter->getNumRefFrames() );
    pcPPSHP->setNumRefIdxActive( LIST_1               , m_pcCodingParameter->getNumRefFrames() );
    // heiko.schwarz@hhi.fhg.de: ensures that the PPS QP will be in the valid range (specified QP can be outside that range to force smaller/higher lambdas)
    //pcPPSHP->setPicInitQp                             ( (Int)rcLayerParameters.getBaseQpResidual() );
    pcPPSHP->setPicInitQp                             ( min( 51, max( 0, (Int)rcLayerParameters.getBaseQpResidual() ) ) );
    pcPPSHP->setChomaQpIndexOffset                    ( 0 );
    pcPPSHP->setDeblockingFilterParametersPresentFlag ( ! m_pcCodingParameter->getLoopFilterParams().isDefault() );
    pcPPSHP->setConstrainedIntraPredFlag              ( true );
    pcPPSHP->setRedundantPicCntPresentFlag            ( rcLayerParameters.getUseRedundantSliceFlag() ); // JVT-Q054 Red. Picture
    pcPPSHP->setTransform8x8ModeFlag                  ( rcLayerParameters.getAdaptiveTransform() > 0 );
    pcPPSHP->setPicScalingMatrixPresentFlag           ( false );
    pcPPSHP->set2ndChromaQpIndexOffset                ( 0 );

    pcPPSHP->setWeightedPredFlag                      ( WEIGHTED_PRED_FLAG );
    pcPPSHP->setWeightedBiPredIdc                     ( WEIGHTED_BIPRED_IDC );
//TMM_WP
    pcPPSHP->setWeightedPredFlag                   (m_pcCodingParameter->getIPMode()!=0);
    pcPPSHP->setWeightedBiPredIdc                  (m_pcCodingParameter->getBMode());  
//TMM_WP

	  //--ICU/ETRI FMO Implementation : FMO stuff start
	  pcPPSHP->setNumSliceGroupsMinus1                  (rcLayerParameters.getNumSliceGroupsMinus1());
	  pcPPSHP->setSliceGroupMapType                     (rcLayerParameters.getSliceGroupMapType());
	  pcPPSHP->setArrayRunLengthMinus1					      (rcLayerParameters.getArrayRunLengthMinus1());
	  pcPPSHP->setArrayTopLeft								  (rcLayerParameters.getArrayTopLeft());
	  pcPPSHP->setArrayBottomRight							  (rcLayerParameters.getArrayBottomRight());
	  pcPPSHP->setSliceGroupChangeDirection_flag		  (rcLayerParameters.getSliceGroupChangeDirection_flag());
	  pcPPSHP->setSliceGroupChangeRateMinus1			  (rcLayerParameters.getSliceGroupChangeRateMinus1());
	  pcPPSHP->setNumSliceGroupMapUnitsMinus1			  (rcLayerParameters.getNumSliceGroupMapUnitsMinus1());
	  pcPPSHP->setArraySliceGroupId						  (rcLayerParameters.getArraySliceGroupId());
	  //--ICU/ETRI FMO Implementation : FMO stuff end

    if( ! rcLayerParameters.getContrainedIntraForLP() )
    {
      pcPPSLP->setNalUnitType                           ( pcPPSHP->getNalUnitType                           ()  );
      pcPPSLP->setLayerId                               ( pcPPSHP->getLayerId                               ()  );
      pcPPSLP->setEntropyCodingModeFlag                 ( pcPPSHP->getEntropyCodingModeFlag                 ()  );
      pcPPSLP->setPicOrderPresentFlag                   ( pcPPSHP->getPicOrderPresentFlag                   ()  );
      pcPPSLP->setNumRefIdxActive( LIST_0               , pcPPSHP->getNumRefIdxActive               ( LIST_0 )  );
      pcPPSLP->setNumRefIdxActive( LIST_1               , pcPPSHP->getNumRefIdxActive               ( LIST_1 )  );
      pcPPSLP->setPicInitQp                             ( pcPPSHP->getPicInitQp                             ()  );
      pcPPSLP->setChomaQpIndexOffset                    ( pcPPSHP->getChomaQpIndexOffset                    ()  );
      pcPPSLP->setDeblockingFilterParametersPresentFlag ( pcPPSHP->getDeblockingFilterParametersPresentFlag ()  );
      pcPPSLP->setConstrainedIntraPredFlag              ( false                                                 );
      pcPPSLP->setRedundantPicCntPresentFlag            ( pcPPSHP->getRedundantPicCntPresentFlag            ()  );  //JVT-Q054 Red. Picture
      pcPPSLP->setTransform8x8ModeFlag                  ( pcPPSHP->getTransform8x8ModeFlag                  ()  );
      pcPPSLP->setPicScalingMatrixPresentFlag           ( pcPPSHP->getPicScalingMatrixPresentFlag           ()  );
      pcPPSLP->set2ndChromaQpIndexOffset                ( pcPPSHP->get2ndChromaQpIndexOffset                ()  );
      pcPPSLP->setWeightedPredFlag                      ( pcPPSHP->getWeightedPredFlag                      ()  );
      pcPPSLP->setWeightedBiPredIdc                     ( pcPPSHP->getWeightedBiPredIdc                     ()  );
    }

  	//--ICU/ETRI FMO Implementation : FMO stuff start
	  pcPPSLP->setNumSliceGroupsMinus1                  (rcLayerParameters.getNumSliceGroupsMinus1());
	  pcPPSLP->setSliceGroupMapType                     (rcLayerParameters.getSliceGroupMapType());
	  pcPPSLP->setArrayRunLengthMinus1					      (rcLayerParameters.getArrayRunLengthMinus1());
	  pcPPSLP->setArrayTopLeft								  (rcLayerParameters.getArrayTopLeft());
	  pcPPSLP->setArrayBottomRight							  (rcLayerParameters.getArrayBottomRight());
	  pcPPSLP->setSliceGroupChangeDirection_flag		  (rcLayerParameters.getSliceGroupChangeDirection_flag());
	  pcPPSLP->setSliceGroupChangeRateMinus1			  (rcLayerParameters.getSliceGroupChangeRateMinus1());
	  pcPPSLP->setNumSliceGroupMapUnitsMinus1			  (rcLayerParameters.getNumSliceGroupMapUnitsMinus1());
	  pcPPSLP->setArraySliceGroupId						  (rcLayerParameters.getArraySliceGroupId());
	  //--ICU/ETRI FMO Implementation : FMO stuff end
   
  }

  //===== set unwritten parameter lists =====
  RNOK( m_pcParameterSetMng->setParamterSetList( m_cUnWrittenSPS, m_cUnWrittenPPS ) );

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END

#include <cstdio>
#include "H264AVCDecoderLib.h"
#include "H264AVCDecoder.h"

#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/TraceFile.h"

#include "SliceReader.h"
#include "SliceDecoder.h"

#include "CreaterH264AVCDecoder.h"
#include "ControlMngH264AVCDecoder.h"
//#include "FGSSubbandDecoder.h"

//#include "GOPDecoder.h"

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN


H264AVCDecoder::H264AVCDecoder()
: m_pcSliceReader                 ( NULL  )
, m_pcSliceDecoder                ( NULL  )
, m_pcFrameMng                    ( NULL  )
, m_pcNalUnitParser               ( NULL  )
, m_pcControlMng                  ( NULL  )
, m_pcLoopFilter                  ( NULL  )
, m_pcHeaderSymbolReadIf          ( NULL  )
, m_pcParameterSetMng             ( NULL  )
, m_pcPocCalculator               ( NULL  )
, m_pcSliceHeader                 ( NULL  )
, m_pcPrevSliceHeader             ( NULL  )
, m_pcSliceHeader_backup          ( NULL  ) // JVT-Q054 Red. Picture
, m_bFirstSliceHeaderBackup       ( true  ) // JVT-Q054 Red. Picture
, m_bRedundantPic                 ( false ) // JVT-Q054 Red. Picture
, m_bInitDone                     ( false )
, m_bLastFrame                    ( false )
, m_bFrameDone                    ( true  )
, m_bEnhancementLayer             ( false )
, m_bBaseLayerIsAVCCompatible     ( false )
, m_bNewSPS                       ( false )
, m_bReconstruct                  ( false )
, m_uiRecLayerId                  ( 0 )
, m_uiLastLayerId                 ( MSYS_UINT_MAX )
, m_pcVeryFirstSPS                ( NULL )
, m_bCheckNextSlice               ( false )
, m_iFirstLayerIdx                ( 0 )
, m_iLastLayerIdx                 ( 0 )
, m_iLastPocChecked               (-1 )
, m_iFirstSlicePoc                ( 0 )
, m_bBaseLayerAvcCompliant        ( false )
, m_bDependencyInitialized        ( false )
, m_uiQualityLevelForPrediction   ( 3 )
//SEI {
, m_uiScalableNestingSeiFlag      ( 0 )
, m_uiScalableNestingSeiRead      ( 0 )
, m_uiSnapshotSeiFlag			  ( 0 )
, m_bAllPicturesInAuFlag		  ( false )
, m_uiActiveViewId                ( NULL )
, m_uiNumPicturesMinus1			  ( 0 )
, m_uiTemporalId				  ( 0 )
, m_uiSnapShotId				  ( 0 )
, m_uiPoc						  ( -1 )
, m_bOpPresentFlag				  ( false )
, m_uiOperationPointId			  ( 0 )
, m_uiNumActiveViews   		      ( 0 )
, m_uiNumDecodeViews			  ( 0 )
, m_uiNumOpMinus1				  ( 0 ) 
//SEI  }
, m_pcNonRequiredSei			  ( NULL )
, m_uiNonRequiredSeiReadFlag	  ( 0 )
, m_uiNonRequiredSeiRead    	  ( 0 )
, m_uiPrevPicLayer				  ( 0 )
, m_uiCurrPicLayer				  ( 0 )
//JVT-P031
, m_uiFirstFragmentPPSId          ( 0 )
, m_uiFirstFragmentNumMbsInSlice  ( 0 )
, m_bFirstFragmentFGSCompSep      ( false )
, m_uiLastFragOrder               ( 0 )
, m_uiNumberOfSPS                 ( 0 )
, m_uiDecodedLayer                ( 0 )
, m_uiNumOfNALInAU                ( 0 )
, m_iPrevPoc                      ( 0 )
//~JVT-P031
, m_bFGSCodingMode                ( false )
, m_uiGroupingSize                ( 1 )
, m_pcBaseLayerCtrlEL						  ( 0 )		// ICU/ETRI FGS_MOT_USE
, m_iCurNalSpatialLayer		(-1)
, m_iNextNalSpatialLayer	(-1)			   
, m_iCurNalPOC				(-1)
, m_iNextNalPOC				(-1)
, m_bCurNalIsEndOfPic		(false)
, m_iCurNalFirstMb			(-1)
, m_bFirstFGS				(true)
//JVT-T054{
, m_bLastNalInAU                  ( false )
, m_bFGSRefInAU                   ( false )
, m_bAVCBased                     ( false )
, m_bCGSSNRInAU                   ( false )
, m_bOnlyAVCAtLayer               ( false )
//JVT-T054}
//JVT-AB025 {{
, m_uiTargetViewId        ( 0 )    
, m_uiOPNotPresentSeiFlag (0)
//JVT-AB025 }}
, m_puiViewOrder                  ( NULL ) //Dec. 1
{
  //::memset( m_apcMCTFDecoder, 0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_uiPosVect,      0x00, 16         * sizeof( UInt  ) ); 
  m_pcVeryFirstSliceHeader = NULL;
  //JVT-P031
  UInt uiLayer;
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
      m_uiNumberOfFragment[uiLayer] = 0;
      m_uiSPSId[uiLayer] = 0;
  }
//SEI {
  UInt uiNumPic;
  for(uiNumPic = 0; uiNumPic < MAX_PICTURES_IN_ACCESS_UNIT; uiNumPic++)
  {
      m_uiPicId[uiNumPic] = 0;
  }

  UInt uiOp;
  for(uiOp = 0; uiOp < MAX_OPERATION_POINTS; uiOp++)
  {
      m_OpViewId[uiOp] = NULL;
	  m_uiNumViews[uiOp] = 0;
  }
//SEI }
  //~JVT-P031
//	TMM	EC {{
	m_uiNextFrameNum	=	0;
	m_uiNextLayerId		=	0;
	m_uiNextPoc				=	0;
	m_uiNumLayers			=	1;
	m_uiMaxGopSize		=	16;
	m_uiMaxDecompositionStages	=	4;
	m_uiMaxLayerId	=	0;
  UInt ui;
	for ( ui=0; ui<MAX_LAYERS; ui++)
	{
		m_pauiPocInGOP         [ui]  =	NULL;
		m_pauiFrameNumInGOP    [ui]  =	NULL;
		m_pauiTempLevelInGOP   [ui]  =	NULL;
		m_uiDecompositionStages[ui]	 =	4;
		m_uiFrameIdx           [ui]  =	0;
		m_uiGopSize            [ui] 	 =	16;
	}
  
  m_eErrorConceal  =	EC_NONE;
	m_bNotSupport	=	false;
  
  if(m_eErrorConceal==EC_RECONSTRUCTION_UPSAMPLE)
    m_eErrorConceal=EC_FRAME_COPY;
//  TMM_EC }}
}



H264AVCDecoder::~H264AVCDecoder()
{
}



ErrVal H264AVCDecoder::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal H264AVCDecoder::init( 
                             SliceReader*        pcSliceReader,
                             SliceDecoder*       pcSliceDecoder,
                             FrameMng*           pcFrameMng,
                             NalUnitParser*      pcNalUnitParser,
                             ControlMngIf*       pcControlMng,
                             LoopFilter*         pcLoopFilter,
                             HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                             ParameterSetMng*    pcParameterSetMng,
                             PocCalculator*      pcPocCalculator,
                             MotionCompensation* pcMotionCompensation )
{

  ROT( NULL == pcSliceReader );
  ROT( NULL == pcSliceDecoder );
  ROT( NULL == pcFrameMng );
  ROT( NULL == pcNalUnitParser );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcLoopFilter );
  ROT( NULL == pcHeaderSymbolReadIf );
  ROT( NULL == pcParameterSetMng );
  ROT( NULL == pcPocCalculator );


  m_pcSliceReader             = pcSliceReader;
  m_pcSliceDecoder            = pcSliceDecoder;
  m_pcFrameMng                = pcFrameMng;
  m_pcNalUnitParser           = pcNalUnitParser;
  m_pcControlMng              = pcControlMng;
  m_pcLoopFilter              = pcLoopFilter;
  m_pcHeaderSymbolReadIf      = pcHeaderSymbolReadIf;
  m_pcParameterSetMng         = pcParameterSetMng;
  m_pcPocCalculator           = pcPocCalculator;
  m_pcFGSPicBuffer            = 0;
  m_bEnhancementLayer         = false;
  m_bBaseLayerIsAVCCompatible = false;
	m_bNewSPS                   = false;
  m_uiRecLayerId              = 0;
  m_uiLastLayerId             = MSYS_UINT_MAX;
  m_pcVeryFirstSPS            = 0;
  m_pcMotionCompensation      = pcMotionCompensation;

  m_bActive = false;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
  
    m_uiNumberOfFragment[uiLayer] = 0;//JVT-P031
  }

  RNOK( m_acLastPredWeightTable[LIST_0].init( 64 ) );
  RNOK( m_acLastPredWeightTable[LIST_1].init( 64 ) );

  m_bInitDone = true;

  m_pcBaseLayerCtrlEL               = 0;

  return Err::m_nOK;
}



ErrVal H264AVCDecoder::uninit()
{
  RNOK( m_acLastPredWeightTable[LIST_0].uninit() );
  RNOK( m_acLastPredWeightTable[LIST_1].uninit() );

  m_pcSliceReader         = NULL;
  m_pcSliceDecoder        = NULL;
  m_pcFrameMng            = NULL;
  m_pcNalUnitParser       = NULL;
  m_pcControlMng          = NULL;
  m_pcLoopFilter          = NULL;
  m_pcHeaderSymbolReadIf  = NULL;
  m_pcParameterSetMng     = NULL;
  m_pcPocCalculator       = NULL;
  m_bInitDone             = false;
  m_bLastFrame            = false;
  m_bFrameDone            = true;
  m_pcMotionCompensation  = NULL;

  delete m_pcSliceHeader;
  delete m_pcPrevSliceHeader;
  delete m_pcSliceHeader_backup;  // JVT-Q054 Red. Pic
  m_pcSliceHeader         = NULL;
  m_pcPrevSliceHeader     = NULL;
  m_pcSliceHeader_backup  = NULL; // JVT-Q054 Red. Pic

  m_pcSliceReader         = NULL;
  m_pcSliceDecoder        = NULL;
  m_pcFrameMng            = NULL;
  m_pcNalUnitParser       = NULL;
  m_pcControlMng          = NULL;
  m_pcLoopFilter          = NULL;
  m_pcHeaderSymbolReadIf  = NULL;
  m_pcParameterSetMng     = NULL;
  m_pcPocCalculator       = NULL;

	m_bNewSPS               = false;
	if( m_pcVeryFirstSliceHeader )
  delete m_pcVeryFirstSliceHeader;
  m_pcVeryFirstSliceHeader = NULL;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    //m_apcMCTFDecoder[uiLayer] = NULL;
  }

  for( UInt uiOp = 0; uiOp <= m_uiNumOpMinus1; uiOp++ )
		if (m_OpViewId[uiOp])
			delete [] m_OpViewId[uiOp] ;
	
	
  if (m_uiActiveViewId)
		delete [] m_uiActiveViewId;

   if (m_puiViewOrder_SubStream)
		delete [] m_puiViewOrder_SubStream;

  m_bInitDone = false;
  
  return Err::m_nOK;
}



ErrVal H264AVCDecoder::create( H264AVCDecoder*& rpcH264AVCDecoder )
{
  rpcH264AVCDecoder = new H264AVCDecoder;
  ROT( NULL == rpcH264AVCDecoder );
  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::calculatePoc( NalUnitType   eNalUnitType,
                              SliceHeader&  rcSliceHeader,
                              Int&          slicePoc )
{
  PocCalculator *pcLocalPocCalculator;

  if( eNalUnitType == NAL_UNIT_CODED_SLICE ||  eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    m_pcPocCalculator->copy( pcLocalPocCalculator );
  }
  else
  {
    //m_apcMCTFDecoder[m_iFirstLayerIdx]->getPocCalculator()->copy( pcLocalPocCalculator );
  }

  pcLocalPocCalculator->calculatePoc( rcSliceHeader );

  slicePoc = rcSliceHeader.getPoc();

  pcLocalPocCalculator->destroy();

  return Err::m_nOK;
}


// not tested with multiple slices
ErrVal
H264AVCDecoder::checkSliceLayerDependency( BinDataAccessor*  pcBinDataAccessor,
                                           Bool&             bFinishChecking
										    ,Bool&		     UnitAVCFlag     ///JVT-S036  lsj
										 )
{
  Bool bEos;
  NalUnitType eNalUnitType;
  SliceHeader* pcSliceHeader = NULL;
  Int slicePoc;

  bFinishChecking = false;
  ROT( NULL == pcBinDataAccessor );

  bEos = ( NULL == pcBinDataAccessor->data() ) || ( 0 == pcBinDataAccessor->size() );
  slicePoc = 0;

  if(bEos)
  {
	if (-1 == m_iNextNalSpatialLayer)
		m_bCurNalIsEndOfPic = true;
    m_bDependencyInitialized = true; //JVT-T054
    bFinishChecking = true;
    return Err::m_nOK;
  }

  if (m_bDependencyInitialized)
  {
	  UInt uiNumBytesTemp; 

	  RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, NULL, uiNumBytesTemp ) );

	  eNalUnitType = m_pcNalUnitParser->getNalUnitType();

	  // if SLICE
	  if( eNalUnitType == NAL_UNIT_CODED_SLICE        ||
        eNalUnitType == NAL_UNIT_CODED_SLICE_IDR      || 
        eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE || 
        eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
	  {
		  // if checked first and second Spatial Layer
		if (-1 != m_iCurNalSpatialLayer && -1 != m_iNextNalSpatialLayer)
		{
			bFinishChecking = true;
			return Err::m_nOK;		
		}
	  }

	  else	// not SLICE
	  {
			//bFinishChecking = true;
			return Err::m_nOK;	
	  }
  }	// end 

  if( ! bEos )
  {
    //m_uiNumOfNALInAU++;//JVT-P031
    m_pcNalUnitParser->setCheckAllNALUs(true);//JVT-P031
    UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
    RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, NULL, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC
    m_pcNalUnitParser->setCheckAllNALUs(false);//JVT-P031

    eNalUnitType = m_pcNalUnitParser->getNalUnitType();
//JVT-T054{
    if((m_uiNumOfNALInAU == 0 && eNalUnitType != NAL_UNIT_SEI) || 
      eNalUnitType == NAL_UNIT_CODED_SLICE        ||
        eNalUnitType == NAL_UNIT_CODED_SLICE_IDR      || 
        eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE || 
        eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
    {
      if(!m_bDependencyInitialized)
      m_uiNumOfNALInAU++;//JVT-P031
    }
//JVT-T054}

	if( eNalUnitType == NAL_UNIT_CODED_SLICE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
	{//JVT-S036 
		UnitAVCFlag = true;
	}

    if( eNalUnitType != NAL_UNIT_CODED_SLICE        &&
        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR      && 
        eNalUnitType != NAL_UNIT_CODED_SLICE_SCALABLE && 
        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
    {
      // NAL units other than slices are ignored
      if(! m_bCheckNextSlice )
        // skipped, without looking further
        bFinishChecking = true;

      return Err::m_nOK;
    }
    else
    {
      // read the slice header
      //JVT-P031								  
      RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser->getNalUnitType   (),
                                              m_pcNalUnitParser->getSvcMvcFlag(),
                                              m_pcNalUnitParser->getNonIDRFlag(), //JVT-W035 
                                              m_pcNalUnitParser->getAnchorPicFlag(),
                                              m_pcNalUnitParser->getViewId(),											
  											  m_pcNalUnitParser->getInterViewFlag(),  //JVT-W056  Samsung
											  m_pcNalUnitParser->getNalRefIdc     (),
                                              m_pcNalUnitParser->getLayerId       (),
 											  m_pcNalUnitParser->getTemporalLevel (),
                                              m_pcNalUnitParser->getQualityLevel  (),
                                              pcSliceHeader,
                                              m_uiFirstFragmentPPSId,
                                              m_uiFirstFragmentNumMbsInSlice,
                                              m_bFirstFragmentFGSCompSep
											  ,UnitAVCFlag                //JVT-S036 
                                              ) );
		if (pcSliceHeader==NULL)
		{
			bFinishChecking = true;
			return Err::m_nOK;    
		}


      
      //To detect if fragments have been lost or previously extracted
      if(pcSliceHeader->getFragmentedFlag())
      {
          m_uiNumberOfFragment[pcSliceHeader->getLayerId()]++;
      }
      if(pcSliceHeader->getFragmentOrder() == 0)
      {
          m_uiFirstFragmentPPSId = pcSliceHeader->getPicParameterSetId();
          m_uiFirstFragmentNumMbsInSlice = pcSliceHeader->getNumMbsInSlice();
          m_bFirstFragmentFGSCompSep = pcSliceHeader->getFgsComponentSep();
          //activate corresponding SPS
          PictureParameterSet * rcPPS;
          m_pcParameterSetMng->get(rcPPS,m_uiFirstFragmentPPSId);
          UInt uiSPSId = rcPPS->getSeqParameterSetId();
          Bool bFound = false;
          for(UInt ui = 0; ui < m_uiNumberOfSPS; ui++)
          {
              if(m_uiSPSId[ui] == uiSPSId)
              {
                  bFound = true;
                  break;
              }
          } 
          if(!bFound)
          {
              m_uiSPSId[m_uiNumberOfSPS] = uiSPSId;
              m_uiNumberOfSPS++;
//JVT-T054{
              SequenceParameterSet * rcSPS;
              m_pcParameterSetMng->get(rcSPS,uiSPSId);
              rcSPS->setLayerId(pcSliceHeader->getLayerId());
//JVT-T054}
          }          
      }
      else
      {
        if( pcSliceHeader != NULL )
            delete pcSliceHeader;
          
        return Err::m_nOK;      
      }
      //~JVT-P031

      calculatePoc( eNalUnitType, *pcSliceHeader, slicePoc );

	  // <-- ROI DECODE ICU/ETRI
	  // first NAL check
	  if (-1 == m_iCurNalSpatialLayer)
	  {
		  m_iCurNalSpatialLayer 	= m_pcNalUnitParser->getLayerId();
		  m_iCurNalPOC  			= slicePoc;
		  m_iCurNalFirstMb			= pcSliceHeader->getFirstMbInSlice();
		  
	  }
	  
	  // second NAL check
	  else if (-1 == m_iNextNalSpatialLayer)
	  {
		  m_iNextNalSpatialLayer	= m_pcNalUnitParser->getLayerId();
		  m_iNextNalPOC			= slicePoc;
		  
		  if (m_iCurNalSpatialLayer != m_iNextNalSpatialLayer)
			  m_bCurNalIsEndOfPic = true;
		  
		  if (m_iCurNalPOC != m_iNextNalPOC)
			  m_bCurNalIsEndOfPic = true;
		  
		  if( pcSliceHeader->getSliceType() == F_SLICE )
			  m_bCurNalIsEndOfPic = true;

		  Bool bNewFrame =false;
		  RNOK( pcSliceHeader->compareRedPic ( m_pcSliceHeader_backup, bNewFrame ) );
		  if(m_iCurNalFirstMb ==pcSliceHeader->getFirstMbInSlice())
			  m_bCurNalIsEndOfPic = true;	  
	  }
	  // --> ROI DECODE ICU/ETRI

      // F slices are also ignored
      if( pcSliceHeader->getSliceType() == F_SLICE )
      {
//JVT-T054{
        m_bFGSRefInAU = true;
//JVT-T054}
        if(! m_bCheckNextSlice )
          // skipped, without looking further
          bFinishChecking = true;

        //manu.mathew@samsung : memory leak fix
        if( pcSliceHeader )
          delete pcSliceHeader;
        //--
        return Err::m_nOK;
      }
//JVT-T054{
      if( pcSliceHeader->getQualityLevel() != 0)
      {
        m_bCGSSNRInAU = true;
      }
//JVT-T054}
      if( slicePoc == m_iLastPocChecked)
      {
		// ROI DECODE ICU/ETRI
		if (-1 != m_iCurNalSpatialLayer && -1 != m_iNextNalSpatialLayer)
		  bFinishChecking = true;

        // for the same picture with all its layers, we only check the dependency once		
        if( pcSliceHeader != NULL )
          delete pcSliceHeader;

        return Err::m_nOK;
      }

      if( ! m_bCheckNextSlice )
      {
        m_iFirstLayerIdx = m_pcNalUnitParser->getLayerId();

        m_iFirstSlicePoc = slicePoc;

        if( eNalUnitType == NAL_UNIT_CODED_SLICE ||  eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
          m_bBaseLayerAvcCompliant = true;
        else
          m_bBaseLayerAvcCompliant = false;
      }

      if( slicePoc == m_iFirstSlicePoc )
      {
        m_iLastLayerIdx = m_pcNalUnitParser->getLayerId();

        if (m_iLastLayerIdx == 0)
        {
          m_auiBaseLayerId[m_iLastLayerIdx]      = MSYS_UINT_MAX;
          m_auiBaseQualityLevel[m_iLastLayerIdx] = 0;
        }
        else
        {
          m_auiBaseLayerId[m_iLastLayerIdx]      = pcSliceHeader->getBaseLayerId();
          m_auiBaseQualityLevel[m_iLastLayerIdx] = pcSliceHeader->getBaseQualityLevel();
        }
      }

      m_bCheckNextSlice = true;
    }
  }

  if( bEos || ( m_bCheckNextSlice && slicePoc != m_iPrevPoc ) ) //JVT-T054
  {
	// ROI DECODE ICU/ETRI
	if (-1 != m_iCurNalSpatialLayer && -1 != m_iNextNalSpatialLayer)
		bFinishChecking   = true;
    // setup the state information for the previous slices	
    m_iPrevPoc = slicePoc; //JVT-P031
   // if(bEos) //JVT-P031
    m_bCheckNextSlice = false;
    m_iLastPocChecked = m_iFirstSlicePoc;
//JVT-T054{
    if(!bEos)
      decreaseNumOfNALInAU();
//JVT-T054}
    //m_apcMCTFDecoder[m_iLastLayerIdx]->setQualityLevelForPrediction( 3 );
    if( m_iFirstLayerIdx < m_iLastLayerIdx )
    {
      // set the base layer dependency
      if( m_iFirstLayerIdx == 0 )
      {
        if( m_bBaseLayerAvcCompliant )
        {
          setQualityLevelForPrediction( m_auiBaseQualityLevel[1] );
        }
        else
        {
          //m_apcMCTFDecoder[0]->setQualityLevelForPrediction( m_auiBaseQualityLevel[1] );
        }
      }

      for( Int iLayer = (m_iFirstLayerIdx == 0) ? 1 : m_iFirstLayerIdx; 
        iLayer <= m_iLastLayerIdx - 1; iLayer ++ )
      {
        //m_apcMCTFDecoder[iLayer]->setQualityLevelForPrediction( m_auiBaseQualityLevel[iLayer + 1] );
      }
    }

    m_bDependencyInitialized = true;
  }

  if( pcSliceHeader != NULL )
    delete pcSliceHeader;

  return Err::m_nOK;
}

//JVT-P031
Void H264AVCDecoder::initNumberOfFragment()
{
    UInt uiLayer;
    for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
    {
        m_uiNumberOfFragment[uiLayer] = 0;
    }
}
Void H264AVCDecoder::getDecodedResolution(UInt &uiLayerId)
{
 UInt uiSPSId = 0;
 UInt uiX     = 0;
 UInt uiY     = 0;
 UInt uiMBX   = 0;
 UInt uiMBY   = 0;
 SequenceParameterSet *rcSPS;

 uiSPSId = 0;
 uiMBX = 0;
 uiMBY = 0;
 UInt uiSPS = 0;
 while(uiSPS < m_uiNumberOfSPS)
 {
   if(m_pcParameterSetMng->isValidSPS(uiSPSId))
   {
    m_pcParameterSetMng->get(rcSPS,uiSPSId);
    uiX = rcSPS->getFrameWidthInMbs();
    uiY = rcSPS->getFrameHeightInMbs();
    if(uiX >= uiMBX && uiY >= uiMBY) //FIX_FRAG_CAVLC
    {
       uiMBX = uiX;
       uiMBY = uiY;
       uiLayerId = rcSPS->getLayerId();
    }
    uiSPS++;
   }
   uiSPSId++;
 }

}
//~JVT-P031
//TMM_EC{{
Bool
H264AVCDecoder::checkSEIForErrorConceal()
{
	Bool	ret	=	true;
	UInt	i	=	0;
	if ( m_bNotSupport)
	{
		return	false;
	}
	if ( m_uiNumLayers > 2 || m_uiNumLayers == 0 )
	{
		return	false;
	}
	for ( i=0; i<m_uiNumLayers; i++)
	{
		if ( m_uiDecompositionStages[i] == 0)
		{
			return	false;
		}
	}
	return	ret;
}

ErrVal
H264AVCDecoder::checkSliceGap( BinDataAccessor*  pcBinDataAccessor,
                               MyList<BinData*>& cVirtualSliceList 
							   ,Bool&			 UnitAVCFlag	//JVT-S036  
							 )
{
  Bool bEos;
  NalUnitType eNalUnitType;
  SliceHeader* pcSliceHeader = NULL;
  Int slicePoc;

  ROT( NULL == pcBinDataAccessor );

  bEos = ( NULL == pcBinDataAccessor->data() ) || ( 0 == pcBinDataAccessor->size() );
  slicePoc = 0;

	UInt	frame_num;
	UInt	uiLayerId;
	UInt	uiPocLsb;
	UInt	uiMaxFrameNum;
	UInt	uiMaxPocLsb;

  if( ! bEos )
  {
    UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
    RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, NULL, uiNumBytesRemoved, true, false, true ) );

    if ( m_pcNalUnitParser->getQualityLevel() != 0)
		{

			BinData	*pcBinData = new BinData;
			pcBinData->set( new UChar[pcBinDataAccessor->size()], pcBinDataAccessor->size());
			memcpy( pcBinData->data(), pcBinDataAccessor->data(), pcBinDataAccessor->size());
			cVirtualSliceList.pushBack( pcBinData);
//*///xk
			return  Err::m_nERR;
		}

		if ( !checkSEIForErrorConceal())
		{
			return	Err::m_nInvalidParameter;
		}
    eNalUnitType = m_pcNalUnitParser->getNalUnitType();

	if( eNalUnitType == NAL_UNIT_CODED_SLICE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
	{//JVT-S036 
		UnitAVCFlag = true;
	}

    if( eNalUnitType != NAL_UNIT_CODED_SLICE        &&
        eNalUnitType != NAL_UNIT_CODED_SLICE_SCALABLE && 
        eNalUnitType != NAL_UNIT_END_OF_STREAM &&
		1)
    {
			if(eNalUnitType==NAL_UNIT_CODED_SLICE_IDR||eNalUnitType==NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
      {

        RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser->getNalUnitType   (),
                                                m_pcNalUnitParser->getSvcMvcFlag(),	
                                                m_pcNalUnitParser->getNonIDRFlag(),   //JVT-W035  
                                                m_pcNalUnitParser->getAnchorPicFlag(),
                                                m_pcNalUnitParser->getViewId(),
												m_pcNalUnitParser->getInterViewFlag(), //JVT-W056  Samsung
                                                m_pcNalUnitParser->getNalRefIdc     (),
                                                m_pcNalUnitParser->getLayerId       (),
												m_pcNalUnitParser->getTemporalLevel (),
                                                m_pcNalUnitParser->getQualityLevel  (),
                                                pcSliceHeader,
                                                m_uiFirstFragmentPPSId,
                                                m_uiFirstFragmentNumMbsInSlice,
                                                m_bFirstFragmentFGSCompSep 
                                                ,UnitAVCFlag		//JVT-S036 
                ) );


        if(pcSliceHeader->getFrameNum()==0)
					m_uiMaxLayerId=pcSliceHeader->getLayerId()>m_uiMaxLayerId ? pcSliceHeader->getLayerId(): m_uiMaxLayerId;
      }
      return Err::m_nOK;
    }
    else
    {
      if ( eNalUnitType != NAL_UNIT_END_OF_STREAM)
	  {
			// read the slice header	
            RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser->getNalUnitType   (),
                                                    m_pcNalUnitParser->getSvcMvcFlag(),	
                                                    m_pcNalUnitParser->getNonIDRFlag(),   //JVT-W035 
                                                    m_pcNalUnitParser->getAnchorPicFlag(),
                                                    m_pcNalUnitParser->getViewId(),
												m_pcNalUnitParser->getInterViewFlag(), //JVT-W056  Samsung											
                                              m_pcNalUnitParser->getNalRefIdc     (),
                                              m_pcNalUnitParser->getLayerId       (),
                                              m_pcNalUnitParser->getTemporalLevel (),
                                              m_pcNalUnitParser->getQualityLevel  (),
                                              pcSliceHeader,
                                              m_uiFirstFragmentPPSId,
                                              m_uiFirstFragmentNumMbsInSlice,
                                              m_bFirstFragmentFGSCompSep 
											  ,UnitAVCFlag		//JVT-S036 
											  ) );

				if(pcSliceHeader->getFrameNum()==0)
					m_uiMaxLayerId=pcSliceHeader->getLayerId()>m_uiMaxLayerId ? pcSliceHeader->getLayerId(): m_uiMaxLayerId;

	      calculatePoc( eNalUnitType, *pcSliceHeader, slicePoc );

       	if ( pcSliceHeader->getFrameNum() == 1 && pcSliceHeader->getLayerId() == m_uiNumLayers-1)
				{
					if ( pcSliceHeader->getPicOrderCntLsb() != m_uiMaxGopSize)
					{
						m_bNotSupport	=	true;
					}
					if ( pcSliceHeader->getFirstMbInSlice() != 0)
					{
						m_bNotSupport	=	true;
					}
					if ( pcSliceHeader->getFrameNum() == 1)
					{
						UInt	i=pcSliceHeader->getPicOrderCntLsb();
						while ( i % 2 == 0)
						{
							i /=	2;
						}
						if ( i!= 1)
						{
							m_bNotSupport	=	true;
						}
					}
				}
				// F slices are also ignored
				if( pcSliceHeader->getSliceType() == F_SLICE || (pcSliceHeader->getFrameNum() == 1 && pcSliceHeader->getLayerId() < m_uiMaxLayerId))
//				if( pcSliceHeader->getSliceType() == F_SLICE)
				{
					if( pcSliceHeader != NULL )
						delete pcSliceHeader;

					return Err::m_nOK;
				}
//	detection the gap of slice
				frame_num	=	pcSliceHeader->getFrameNum();
				uiLayerId	=	pcSliceHeader->getLayerId();
				uiPocLsb	=	pcSliceHeader->getPicOrderCntLsb();
//				uiGopSize	=	pcSliceHeader->getSPS().getNumRefFrames() + 1;	//??
				UInt uiGopSize	=	m_uiGopSize[uiLayerId];
				if ( frame_num == 1 &&m_uiMaxLayerId==uiLayerId&& uiPocLsb > uiGopSize)
				{
					m_bNotSupport = true;
					if( pcSliceHeader != NULL )
						delete pcSliceHeader;
					return	Err::m_nOK;
				}
				uiMaxFrameNum	=	1 << pcSliceHeader->getSPS().getLog2MaxFrameNum();
				uiMaxPocLsb		=	1 << pcSliceHeader->getSPS().getLog2MaxPicOrderCntLsb();
			}
			else
			{
				uiMaxFrameNum	=	1 << m_pcVeryFirstSliceHeader->getSPS().getLog2MaxFrameNum();
				uiMaxPocLsb		=	1 << m_pcVeryFirstSliceHeader->getSPS().getLog2MaxPicOrderCntLsb();
				uiLayerId	=	m_uiNextLayerId;
				UInt uiGopSize	=	1 << m_uiDecompositionStages[uiLayerId];
//				uiGopSize	=	m_uiGopSize[uiLayerId];
				if ( m_uiFrameIdx[uiLayerId] % uiGopSize != 1)
				{
					frame_num	=	m_pauiFrameNumInGOP[uiLayerId][m_uiGopSize[uiLayerId]-1] % uiMaxFrameNum;
					uiPocLsb	=	m_pauiPocInGOP[uiLayerId][m_uiGopSize[uiLayerId]-1] % uiMaxPocLsb;
				}
				else
				{
					frame_num	=	m_pauiFrameNumInGOP[uiLayerId][0] % uiMaxFrameNum;
					uiPocLsb	=	m_pauiPocInGOP[uiLayerId][0] % uiMaxPocLsb;
				}
//				if ( m_uiFrameIdx[uiLayerId] % uiGopSize == 1)
				{
					frame_num	-=	(uiGopSize >> 1);
					uiPocLsb	-=	m_uiMaxGopSize;
				}
			}
		}

		if ( frame_num != m_uiNextFrameNum || uiLayerId != m_uiNextLayerId || uiPocLsb != (m_uiNextPoc % uiMaxPocLsb))
		{
//	judge if the uncompleted GOP
			UInt	uiFrameIdx	=	0;
			UInt	uiGopSize	=	1 << m_uiDecompositionStages[uiLayerId];
//			UInt	uiGopSize	=	m_uiGopSize[uiLayerId];

			for ( ;uiFrameIdx < uiGopSize; uiFrameIdx++)
			{
				if ( m_pauiPocInGOP[uiLayerId][uiFrameIdx]	% uiMaxPocLsb ==	uiPocLsb)
					break;
			}
			if ( ( m_pauiFrameNumInGOP[uiLayerId][uiFrameIdx] % uiMaxFrameNum ) == frame_num || (uiPocLsb-1) / m_uiMaxGopSize != ((m_uiNextPoc-1) % uiMaxPocLsb) / m_uiMaxGopSize)
			{
				if ( m_uiNextLayerId == 0)
				{
					BinData	*pcBinData = new BinData;
					pcBinData->set( new UChar[11], 11);
//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_BASELAYER;
					*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
					*(Int*)(pcBinData->data()+1)	=	0xdeadface;
					*(Short*)(pcBinData->data()+5)	=	(Short) m_uiNextFrameNum;
					*(Short*)(pcBinData->data()+7)	=	(Short) m_uiNextPoc;
					*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
					*(pcBinData->data()+pcBinData->size()-1)	=	0;
					cVirtualSliceList.pushFront( pcBinData);
				}
				else
				{
					BinData	*pcBinData = new BinData;
					pcBinData->set( new UChar[11], 11);
//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_ENHANCELAYER;
					*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
					*(Int*)(pcBinData->data()+1)	=	0xdeadface;
					*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
					*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
					*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
					*(pcBinData->data()+pcBinData->size()-1)	=	0;
					cVirtualSliceList.pushFront( pcBinData);
				}
			}
			else
			{
//	uncomplete GOP structure
//	calculate the gop size of the layer current packet belongs to
				if ( uiPocLsb % ( 2<<(m_uiMaxDecompositionStages-m_uiDecompositionStages[uiLayerId])) != 0)
				{
					m_uiGopSize[uiLayerId]	=	(((frame_num-1) % (uiGopSize>>1))<< 1) + 1;
				}
				else
				{
					UInt	uiFrameDiff	=	( m_pauiFrameNumInGOP[uiLayerId][uiFrameIdx] % uiMaxFrameNum)  - frame_num;
					UInt	uiTL	=	m_uiDecompositionStages[uiLayerId];
					UInt	uiTmp	=	uiPocLsb >> (m_uiMaxDecompositionStages - m_uiDecompositionStages[uiLayerId]);
					while( uiTmp % 2 == 0)
					{
						uiTmp	>>=1;
						uiTL--;
					}
          UInt ui = 0;
					for ( ui=m_uiGopSize[uiLayerId]&(-2); ui>0; ui-=2)
					{
						UInt	uiTempLevel	=	m_uiDecompositionStages[uiLayerId];
						uiTmp	=	ui;
						while( uiTmp % 2 == 0)
						{
							uiTmp	>>=1;
							uiTempLevel--;
						}
						if ( uiTL <= uiTempLevel)
						{
							continue;
						}
						uiFrameDiff--;
						if ( uiFrameDiff == 0)
						{
							break;
						}
					}
					m_uiGopSize[uiLayerId]	=	ui - 1;
				}
        //	calculate the gop size of other layers
        UInt  ui=0; 
				for ( ui=0; ui<uiLayerId; ui++)
				{
					m_uiGopSize[ui]	=	m_uiGopSize[uiLayerId] >> (m_uiDecompositionStages[uiLayerId] - m_uiDecompositionStages[ui]);
				}
				for ( ui=uiLayerId+1; ui<m_uiNumLayers; ui++)
				{
					m_uiGopSize[ui]	=	( m_uiGopSize[uiLayerId] << (m_uiDecompositionStages[ui] - m_uiDecompositionStages[uiLayerId])) + ((1<<(m_uiDecompositionStages[ui] - m_uiDecompositionStages[uiLayerId])) -1);
				}
//	calculate the correct frame_num and poc of every packet in this uncompleted gop each layer
				m_uiFrameIdx[m_uiNextLayerId]--;
				for ( ui=0; ui<m_uiNumLayers; ui++)
				{
					uiFrameIdx      	=	0;
					UInt	uiFrameNum	=	m_pauiFrameNumInGOP[ui][0];
					UInt	uiPoc	=	( m_uiNextPoc - 1 ) & -(1<<m_uiMaxDecompositionStages);
					if ( ui == 0 || m_uiFrameIdx[ui] % (1<<m_uiDecompositionStages[ui]) != 0)
						uiFrameNum	-=	( 1 << ( m_uiDecompositionStages[ui] - 1 ) );
					UInt	uiDecompositionStagesSub	=	m_uiMaxDecompositionStages - m_uiDecompositionStages[ui];
					for( UInt uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages[ui]; uiTemporalLevel++ )
					{
						UInt      uiStep    = ( 1 << ( m_uiDecompositionStages[ui] - uiTemporalLevel ) );
						for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGopSize[ui]; uiFrameId += ( uiStep << 1 ) )
						{
							m_pauiPocInGOP[ui][uiFrameIdx]	=	(uiFrameId << uiDecompositionStagesSub ) + uiPoc;
							m_pauiFrameNumInGOP[ui][uiFrameIdx]	=	uiFrameNum;
							m_pauiTempLevelInGOP[ui][uiFrameIdx]	=	uiTemporalLevel;
							uiFrameIdx++;
							if ( uiFrameId % 2 == 0)
								uiFrameNum++;
						}
					}
					for ( uiFrameIdx=0; uiFrameIdx<m_uiFrameIdx[ui] % (1<<m_uiDecompositionStages[ui]); uiFrameIdx++)
					{
						m_pauiPocInGOP[ui][uiFrameIdx]	+=	m_uiMaxGopSize;
						m_pauiFrameNumInGOP[ui][uiFrameIdx]	+=	(1<<(m_uiDecompositionStages[ui]-1));
					}
				}
			  do
			  {
				  if (m_uiFrameIdx[m_uiNextLayerId] % m_uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
					  break;
				  m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
			  }
				while( true);
				if (m_uiFrameIdx[m_uiNextLayerId] % m_uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
				{
					uiMaxFrameNum				=	1 << pcSliceHeader->getSPS().getLog2MaxFrameNum();
					m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
					m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
					m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
					m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1 << ( m_uiDecompositionStages[m_uiNextLayerId] - 1);
					m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	m_uiMaxGopSize;
					m_uiFrameIdx[m_uiNextLayerId]++;
				}
				if ( frame_num != m_uiNextFrameNum || uiLayerId != m_uiNextLayerId || uiPocLsb != (m_uiNextPoc % uiMaxPocLsb))
				{
					if ( m_uiNextLayerId == 0)
					{
						BinData	*pcBinData = new BinData;
						pcBinData->set( new UChar[11], 11);
//						*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_BASELAYER;
						*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
						*(Int*)(pcBinData->data()+1)	=	0xdeadface;
						*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
						*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
						*(pcBinData->data()+9)	=	(UChar)m_uiNextTempLevel;
						*(pcBinData->data()+pcBinData->size()-1)	=	0;
						cVirtualSliceList.pushFront( pcBinData);
					}
					else
					{
						BinData	*pcBinData = new BinData;
						pcBinData->set( new UChar[11], 11);
//						*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_ENHANCELAYER;
						*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
						*(Int*)(pcBinData->data()+1)	=	0xdeadface;
						*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
						*(Short*)(pcBinData->data()+7)	=	 (Short)m_uiNextPoc;
						*(pcBinData->data()+9)	=	(UChar)m_uiNextTempLevel;
						*(pcBinData->data()+pcBinData->size()-1)	=	0;
						cVirtualSliceList.pushFront( pcBinData);
					}
				}
			}
		}
	}

  if( pcSliceHeader != NULL )
    delete pcSliceHeader;

  return Err::m_nOK;
}
//TMM_EC }}
ErrVal
H264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor,
													  UInt&             ruiNalUnitType,
														UInt&             ruiMbX,
														UInt&             ruiMbY,
														UInt&             ruiSize
														//,UInt&             ruiNonRequiredPic  //NonRequired JVT-Q066
														//JVT-P031
														, Bool            bPreParseHeader //FRAG_FIX
														, Bool			      bConcatenated //FRAG_FIX_3
														, Bool&           rbStartDecoding,
														UInt&             ruiStartPos,
														UInt&             ruiEndPos,
														Bool&             bFragmented,
														Bool&             bDiscardable
                            //~JVT-P031
														 ,Bool&			  UnitAVCFlag    //JVT-S036 
														 ,UInt NumOfViewsInTheStream
                                                         ,Bool& bSkip)
{
  ROF( m_bInitDone );
  UInt uiLayerId;

  ROT( NULL == pcBinDataAccessor );
  if ( NULL == pcBinDataAccessor->data() || 0 == pcBinDataAccessor->size() )
  {
    // switch 
    SliceHeader* pTmp = m_pcSliceHeader;
    m_pcSliceHeader   = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader = pTmp;


    m_bLastFrame = true;
    delete m_pcSliceHeader;
    m_pcSliceHeader = 0;
    rbStartDecoding = true; //JVT-P031
    return Err::m_nOK;
  }

  Bool KeyPicFlag = false;
  static Bool bSuffixUnit = false;  //JVT-S036 
  //JVT-P031
  Bool bLastFragment;
  UInt uiHeaderBits;
  getDecodedResolution(m_uiDecodedLayer);
  m_pcNalUnitParser->setDecodedLayer(m_uiDecodedLayer);
  ruiStartPos = m_pcNalUnitParser->getNalHeaderSize(pcBinDataAccessor);
  ruiStartPos = 0; //FRAG_FIX
  //~JVT-P031
  UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
  RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, &KeyPicFlag,uiNumBytesRemoved, bPreParseHeader , bConcatenated) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC
  UInt uiBitsLeft = m_pcNalUnitParser->getBitsLeft(); //JVT-P031

  ruiNalUnitType = m_pcNalUnitParser->getNalUnitType();
  
  //TMM_EC {{
  if(!bPreParseHeader && ruiNalUnitType==NAL_UNIT_END_OF_STREAM)
  {
	  SliceHeader* pTmp = m_pcSliceHeader;
    m_pcSliceHeader   = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader = pTmp;

		for ( UInt i=0; i< m_uiNumLayers; i++)
		{
			if (m_pauiPocInGOP[i])       delete	[] m_pauiPocInGOP[i];
      if (m_pauiFrameNumInGOP[i])  delete	[] m_pauiFrameNumInGOP[i];
			if (m_pauiTempLevelInGOP[i]) delete	[] m_pauiTempLevelInGOP[i];
		}
    m_bLastFrame = true;
    delete m_pcSliceHeader;
    m_pcSliceHeader = 0;
    return Err::m_nOK;
  }
//TMM_EC }}
  switch ( m_pcNalUnitParser->getNalUnitType() )
  {
  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
		{
	UnitAVCFlag = true;  //JVT-S036 
    //JVT-P031
    RNOK( xStartSlice(bPreParseHeader,bLastFragment, bDiscardable, UnitAVCFlag) ); //FRAG_FIX //TMM_EC //JVT-S036 
    ruiEndPos = pcBinDataAccessor->size();
    bDiscardable = false;
    uiHeaderBits = uiBitsLeft - m_pcNalUnitParser->getBitsLeft();
    ruiStartPos += (uiHeaderBits+7)>>3; 
  	ruiStartPos = 0; //FRAG_FIX
    //~JVT-P031
//    RNOK( m_pcControlMng      ->initSlice0(m_pcSliceHeader) );
    RNOK( m_pcControlMng      ->initSlice0(m_pcSliceHeader,NumOfViewsInTheStream) );
    m_pcSliceHeader->setKeyPictureFlag (KeyPicFlag);
    m_bActive = true;
    rbStartDecoding = true; //JVT-P031
	bSuffixUnit = true; //JVT-S036 

//TMM_EC {{
		if (!m_bNotSupport && !bPreParseHeader)
		{
				UInt	uiMaxGopSize	=	m_uiMaxGopSize;
			  UInt	uiGopSize;
			  m_uiNextLayerId		=	m_pcSliceHeader->getLayerId();
			  do
			  {
				  m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
				  uiGopSize	=	1 << m_uiDecompositionStages[m_uiNextLayerId];

				  if (m_uiFrameIdx[m_uiNextLayerId] % uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
					  break;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  while( true);

			  if ( m_pcNalUnitParser->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR || m_uiNextLayerId == 0)
			  {
				  UInt	uiMaxFrameNum	=	1 << m_pcSliceHeader->getSPS().getLog2MaxFrameNum();
				  m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
				  m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				  m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				  m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiGopSize >> 1;
				  m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiMaxGopSize;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  else
			  {
				  m_uiNextFrameNum	=	0;
				  m_uiNextPoc			=	0;
			  }
		}
//TMM_EC }}    
    m_pcSliceHeader->setFGSCodingMode( m_bFGSCodingMode );
    m_pcSliceHeader->setGroupingSize ( m_uiGroupingSize );
    UInt ui;
    for( ui = 0; ui < 16; ui++ )
    {
      m_pcSliceHeader->setPosVect( ui, m_uiPosVect[ui] );
    }
	}
	break;
  case NAL_UNIT_SPS:
  case NAL_UNIT_SUBSET_SPS:
    {
      SequenceParameterSet* pcSPS = NULL;
      RNOK( SequenceParameterSet::create  ( pcSPS   ) );
	  pcSPS->SpsMVC = new SpsMvcExtension;	
      RNOK( pcSPS               ->read    ( m_pcHeaderSymbolReadIf,
                                            m_pcNalUnitParser->getNalUnitType() ) );
      // It is assumed that layer 0 and layer 1 use the first two SPSs, respectively.
      if( NULL == m_pcVeryFirstSPS )
      {
        setVeryFirstSPS( pcSPS );
        RNOK( m_pcPocCalculator->initSPS( *pcSPS ) );
      }
      //  SPS fix
      else if( m_pcVeryFirstSPS->getProfileIdc() != MULTI_VIEW_PROFILE && m_pcVeryFirstSPS->getProfileIdc() != STEREO_HIGH_PROFILE)
      {
        m_pcVeryFirstSPS->SpsMVC = new SpsMvcExtension;	 // Nov. 30
        m_pcVeryFirstSPS->SpsMVC->setNumViewsMinus1 ( pcSPS->getSpsMVC()->getNumViewMinus1() );      
        m_puiViewOrder = pcSPS->getSpsMVC()->getViewCodingOrder(); // Dec. 1
		m_pcVeryFirstSPS->SpsMVC->m_uiViewCodingOrder = pcSPS->getSpsMVC()->getViewCodingOrder();
		
		m_puiViewOrder_SubStream = new UInt[pcSPS->getSpsMVC()->getNumViewMinus1()+1];
		UInt j,cnt=0;
		if (m_uiNumActiveViews > 0)
			for (int i=cnt; i< pcSPS->getSpsMVC()->getNumViewMinus1()+1; i++) {				
				for (j=0; j < m_uiNumActiveViews; j++)
					if (m_puiViewOrder[i] ==  m_uiActiveViewId[j])
						break;
				if (j< m_uiNumActiveViews)
					m_puiViewOrder_SubStream[cnt++]= m_puiViewOrder[i];
			}
		else // when ViewScalSEI not present
		{
			m_uiNumActiveViews = pcSPS->getSpsMVC()->getNumViewMinus1()+1;
			for (UInt i=0; i< m_uiNumActiveViews; i++)
				m_puiViewOrder_SubStream[i]= m_puiViewOrder[i];

		}

		

      }

		for ( UInt i=0; i<m_uiNumLayers; i++)
		{
			UInt	uiDecompositionStagesSub	=	m_uiMaxDecompositionStages - m_uiDecompositionStages[i];
			UInt	uiGopSize	=	m_uiGopSize[i];
			m_pauiPocInGOP[i]				=	new	UInt[uiGopSize];
			m_pauiFrameNumInGOP[i]		=	new	UInt[uiGopSize];
			m_pauiTempLevelInGOP[i]	=	new	UInt[uiGopSize];
			UInt	uiFrameIdx	=	0;
			UInt	uiFrameNum	=	1;
			for( UInt uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages[i]; uiTemporalLevel++ )
			{
				UInt      uiStep    = ( 1 << ( m_uiDecompositionStages[i] - uiTemporalLevel ) );
				for( UInt uiFrameId = uiStep; uiFrameId <= uiGopSize; uiFrameId += ( uiStep << 1 ) )
				{
					m_pauiPocInGOP[i][uiFrameIdx]	=	uiFrameId << uiDecompositionStagesSub;
					m_pauiFrameNumInGOP[i][uiFrameIdx]	=	uiFrameNum;
					m_pauiTempLevelInGOP[i][uiFrameIdx]	=	uiTemporalLevel;
					uiFrameIdx++;
					if ( uiFrameId % 2 == 0)
						uiFrameNum++;
				}
			}
		}

	  if ( pcSPS->getProfileIdc()==SCALABLE_PROFILE )
      {

			if ( pcSPS->getSeqParameterSetId() == 0)
			{
				m_bNotSupport	=	true;
			}

			m_bEnhancementLayer = true;
      }
	
	  m_bNewSPS = true;
      RNOK( m_pcParameterSetMng ->store   ( pcSPS   ) );

      // Copy simple priority ID mapping from SPS to NAL unit parser

      ruiMbX  = pcSPS->getFrameWidthInMbs ();
      ruiMbY  = pcSPS->getFrameHeightInMbs();
      ruiSize = max( ruiSize, ( (ruiMbX << 3 ) + YUV_X_MARGIN ) * ( ( ruiMbY << 3 ) + YUV_Y_MARGIN ) * 6 );
			m_pcControlMng->initSPS( *pcSPS, m_uiRecLayerId );
      ruiEndPos = pcBinDataAccessor->size(); //JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
	  m_uiCropOffset[0]=pcSPS->getCropOffset(0)*2;
	  m_uiCropOffset[1]=pcSPS->getCropOffset(1)*2;
	  m_uiCropOffset[2]=pcSPS->getCropOffset(2)*( pcSPS->getFrameMbsOnlyFlag()?2:4);
	  m_uiCropOffset[3]=pcSPS->getCropOffset(3)*( pcSPS->getFrameMbsOnlyFlag()?2:4);
      m_bFGSCodingMode = pcSPS->getFGSCodingMode();
      m_uiGroupingSize = pcSPS->getGroupingSize ();
      UInt ui;
      for(ui = 0; ui < 16; ui++)
      {
        m_uiPosVect[ui] = pcSPS->getPosVect(ui);
      }
    }
    break;

  case NAL_UNIT_PPS:
    {
      PictureParameterSet* pcPPS = NULL;
      RNOK( PictureParameterSet::create( pcPPS  ) );
      RNOK( pcPPS->read( m_pcHeaderSymbolReadIf,
                         m_pcNalUnitParser->getNalUnitType() ) );
      RNOK( m_pcParameterSetMng->store( pcPPS   ) );
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;

  case NAL_UNIT_CODED_SLICE_PREFIX: // JVT-W035
  {
    rbStartDecoding = true;
    initPacketPrefix( pcBinDataAccessor,
                      ruiNalUnitType,
                      bPreParseHeader, //FRAG_FIX
                      bConcatenated,//FRAG_FIX_3
                      rbStartDecoding,
                      m_pcSliceHeader
                      ,m_pcNalUnitParser
    );
    ruiEndPos = pcBinDataAccessor->size(); 
  }
  break;		
  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:

    {

      if(!m_pcNalUnitParser->getSvcMvcFlag())
      {
        //for now MVC silce data are AVC compatible only high level syntax is different
        UnitAVCFlag = true;

        RNOK( xStartSlice(bPreParseHeader,bLastFragment, bDiscardable, UnitAVCFlag) ); //FRAG_FIX //TMM_EC //JVT-S036 
        ruiEndPos = pcBinDataAccessor->size();
        bDiscardable = false;
        uiHeaderBits = uiBitsLeft - m_pcNalUnitParser->getBitsLeft();
        ruiStartPos += (uiHeaderBits+7)>>3; 
        ruiStartPos = 0; //FRAG_FIX
        //~JVT-P031
//        RNOK( m_pcControlMng->initSlice0(m_pcSliceHeader) );
        RNOK( m_pcControlMng->initSlice0(m_pcSliceHeader,NumOfViewsInTheStream) );
        // m_pcSliceHeader->setKeyPictureFlag (KeyPicFlag);
        m_bActive = true;
        rbStartDecoding = true; //JVT-P031

//TMM_EC {{
        if (!m_bNotSupport && !bPreParseHeader)
        {
          UInt	uiMaxGopSize	=	m_uiMaxGopSize;
          UInt	uiGopSize;
          m_uiNextLayerId		=	m_pcSliceHeader->getLayerId();
          do
          {
            m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
            uiGopSize	=	1 << m_uiDecompositionStages[m_uiNextLayerId];

            if (m_uiFrameIdx[m_uiNextLayerId] % uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
              break;
            m_uiFrameIdx[m_uiNextLayerId]++;
          }
          while( true);

          if ( m_pcNalUnitParser->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR || m_uiNextLayerId == 0)
          {
            UInt	uiMaxFrameNum	=	1 << m_pcSliceHeader->getSPS().getLog2MaxFrameNum();
            m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
            m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
            m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
            m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiGopSize >> 1;
            m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiMaxGopSize;
            m_uiFrameIdx[m_uiNextLayerId]++;
          }
          else
          {
            m_uiNextFrameNum	=	0;
            m_uiNextPoc			=	0;
          }
        }
//TMM_EC }}    

        m_pcSliceHeader->setFGSCodingMode( m_bFGSCodingMode );
        m_pcSliceHeader->setGroupingSize ( m_uiGroupingSize );
                        
        UInt ui;
        for( ui = 0; ui < 16; ui++ )
        {
          m_pcSliceHeader->setPosVect( ui, m_uiPosVect[ui] );
        }
      }
      else
      {

        //JVT-P031
        getDecodedResolution(m_uiDecodedLayer);
        if(m_pcNalUnitParser->getLayerId() < m_uiDecodedLayer && m_pcNalUnitParser->getDiscardableFlag())
          bDiscardable = true;
        else
          bDiscardable = false;

        RNOK( xStartSlice(bPreParseHeader,bLastFragment, bDiscardable, UnitAVCFlag) ); //FRAG_FIX //TMM_EC //JVT-S036 
        if(bDiscardable)
          ruiEndPos = 0;
        else
          ruiEndPos = pcBinDataAccessor->size();

        uiHeaderBits = uiBitsLeft - m_pcNalUnitParser->getBitsLeft();
        if( (bDiscardable) || !bLastFragment) //FRAG_FIX
          ruiStartPos = 0;
        else
        {
          //JVT-S036  start
          //  if(m_pcNalUnitParser->getExtensionFlag())
          //{
          // ruiStartPos += (uiHeaderBits+3*8+7)>>3;//(uiHeaderBits+7)>>3; //BUG_FIX_FT_01_2006_2
          //3*8 is used to take into account the nal header which has already been read
          //uiHeaderBits only contains the remaining bits of the slice header read
          //}
          // else
          // {
          ruiStartPos += (uiHeaderBits+3*8+7)>>3;//BUG_FIX_FT_01_2006_2
          // }
          //JVT-S036  end
        }
        if(m_pcSliceHeader && (m_pcSliceHeader->getFragmentedFlag() && !bLastFragment ))
        { //FIX_FRAG_CAVLC
          if(bPreParseHeader)
          {
            ruiEndPos -= uiNumBytesRemoved;
          }//~FIX_FRAG_CAVLC
          ruiEndPos -= 2;

        }//FIX_FRAG_CAVLC
        if(!bDiscardable)
          //~JVT-P031
//          RNOK( m_pcControlMng      ->initSlice0(m_pcSliceHeader) );
          RNOK( m_pcControlMng      ->initSlice0(m_pcSliceHeader,NumOfViewsInTheStream) );
        if(m_pcSliceHeader) //JVT-P031
          m_pcSliceHeader->setKeyPictureFlag (KeyPicFlag);
        //JVT-P031
        bFragmented = (!m_pcSliceHeader ? false : m_pcSliceHeader->getFragmentedFlag());
        if( (bLastFragment) || (!bFragmented) || 
            ( bFragmented && m_pcSliceHeader->getFragmentOrder()+1 == m_uiNumberOfFragment[m_pcSliceHeader->getLayerId() ])) 
       
          rbStartDecoding = true;
        //~JVT-P031
//TMM_EC {{
        if ( !m_bNotSupport && !bPreParseHeader && m_pcNalUnitParser->getQualityLevel() == 0)
        {
          UInt	uiMaxGopSize	=	m_uiMaxGopSize;
          UInt	uiGopSize;
          m_uiNextLayerId		=	m_pcSliceHeader->getLayerId();
          do
          {
            m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
            uiGopSize	=	1 << m_uiDecompositionStages[m_uiNextLayerId];
            if (m_uiFrameIdx[m_uiNextLayerId] % uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
              break;
            m_uiFrameIdx[m_uiNextLayerId]++;
          }
          while( true);

    	  if ( m_pcNalUnitParser->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR || m_uiNextLayerId == 0)
          {
            UInt	uiMaxFrameNum	=	1 << m_pcSliceHeader->getSPS().getLog2MaxFrameNum();
            m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
            m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
            m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
            m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiGopSize >> 1;
            m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiMaxGopSize;
            m_uiFrameIdx[m_uiNextLayerId]++;
          }
          else
          {
            m_uiNextFrameNum	=	0;
            m_uiNextPoc			=	0;
          }
        }
//TMM_EC }}
        if(m_pcSliceHeader)
        {
          m_pcSliceHeader->setFGSCodingMode( m_bFGSCodingMode );
          m_pcSliceHeader->setGroupingSize ( m_uiGroupingSize );
          UInt ui;
          for(ui = 0; ui < 16; ui++)
          {
            m_pcSliceHeader->setPosVect( ui, m_uiPosVect[ui] );
          }
        }


      }
    }
    break;

  case NAL_UNIT_SEI:
    {
      //===== just for trace file =====
      SEI::MessageList  cMessageList;
	  UInt	i;
       RNOK( SEI::read( m_pcHeaderSymbolReadIf, cMessageList  ) );
	  
      while( ! cMessageList.empty() )
      {
        SEI::SEIMessage*  pcSEIMessage = cMessageList.popBack();
		  if(pcSEIMessage->getMessageType() == SEI::NON_REQUIRED_SEI)
		  {
				m_pcNonRequiredSei = (SEI::NonRequiredSei*) pcSEIMessage;
			  m_uiNonRequiredSeiReadFlag = 1;
		  }
//JVT-W080
		  else if( pcSEIMessage->getMessageType() == SEI::PARALLEL_DEC_SEI )
		  {
		    printf("\n Parallel SEI message received. \tParallel Decoding Enable.\n\n");
			  delete pcSEIMessage;
		  }
//~JVT-W080	
			else
		  {
				  if ( pcSEIMessage->getMessageType() == SEI::SCALABLE_SEI)
				  {
//	trick
            m_uiNumLayers						=	((SEI::ScalableSei*)pcSEIMessage)->getDependencyId( ((SEI::ScalableSei*)pcSEIMessage)->getNumLayersMinus1()) + 1;
					  for ( uiLayerId=0; uiLayerId<((SEI::ScalableSei*)pcSEIMessage)->getNumLayersMinus1()+1; uiLayerId++)
            {
              if ( ((SEI::ScalableSei*)pcSEIMessage)->getDependencyId( uiLayerId) != 0)
                break;
            }
            uiLayerId--;
            m_uiDecompositionStages[0]	=	((SEI::ScalableSei*)pcSEIMessage)->getTemporalLevel( uiLayerId);
            m_uiDecompositionStages[m_uiNumLayers-1]	=	((SEI::ScalableSei*)pcSEIMessage)->getTemporalLevel( ((SEI::ScalableSei*)pcSEIMessage)->getNumLayersMinus1());
					  m_uiMaxDecompositionStages = m_uiDecompositionStages[m_uiNumLayers-1];
					  m_uiMaxGopSize	=	1 << m_uiMaxDecompositionStages;
						for ( i=0; i< m_uiNumLayers; i++)
						  m_uiGopSize[i]	=	1 << m_uiDecompositionStages[i];
				  }
//SEI {
				  else if( pcSEIMessage->getMessageType() == SEI::SCALABLE_NESTING_SEI )
				  {
					  m_uiScalableNestingSeiFlag = 1;
					  m_bAllPicturesInAuFlag = ((SEI::ScalableNestingSei*)pcSEIMessage)->getAllPicturesInAuFlag();
					  if( m_bAllPicturesInAuFlag == 0 )
					  {
						  m_uiNumPicturesMinus1 = ((SEI::ScalableNestingSei*)pcSEIMessage)->getNumPicturesMinus1();
						  for( UInt index = 0; index <= m_uiNumPicturesMinus1; index++ )
							  m_uiPicId[index] = ((SEI::ScalableNestingSei*)pcSEIMessage)->getPicId(index);
						  m_uiTemporalId = ((SEI::ScalableNestingSei*)pcSEIMessage)->getTemporalId();
					  }
					  SEI::SEIMessage*  pcSEIMessage1 = ((SEI::ScalableNestingSei*)pcSEIMessage)->getSEIMessage();
					  if( pcSEIMessage1->getMessageType() == SEI::FULLFRAME_SNAPSHOT_SEI )
					  {
						  //for snap shot sei
						  m_uiSnapShotId = ((SEI::FullframeSnapshotSei*)pcSEIMessage1)->getSnapShotId();
						  m_uiSnapshotSeiFlag = 1;
					  }
				  }
				  else if( pcSEIMessage->getMessageType() == SEI::VIEW_SCALABILITY_INFO_SEI )
				  {
					m_uiNumOpMinus1 = ((SEI::ViewScalabilityInfoSei*)pcSEIMessage)->getNumOperationPointsMinus1();
					m_uiNumActiveViews=0;
					UInt TotNumView=0;	
					for( UInt k = 0; k <= m_uiNumOpMinus1; k++ )
					  TotNumView += ((SEI::ViewScalabilityInfoSei*)pcSEIMessage)->getNumTargetOutputViewsMinus1(k)+1;
					TotNumView *=3;
					
					m_uiActiveViewId = new UInt[TotNumView];
					for( UInt uiOp = 0; uiOp <= m_uiNumOpMinus1; uiOp++ )
					{
					  m_uiNumViews[uiOp] = ((SEI::ViewScalabilityInfoSei*)pcSEIMessage)->getNumTargetOutputViewsMinus1(uiOp)+1;//SEI JJ
					  m_OpViewId[uiOp] = (UInt*)malloc(m_uiNumViews[uiOp]*sizeof(UInt));
					  for(UInt uiView = 0; uiView < m_uiNumViews[uiOp]; uiView++ )
						  m_OpViewId[uiOp][uiView] = ((SEI::ViewScalabilityInfoSei*)pcSEIMessage)->getViewId(uiOp, uiView);
					  UInt curr;
					  for(UInt uiView = 0; uiView < m_uiNumViews[uiOp]; uiView++ ) {
						  for( curr = 0; curr < m_uiNumActiveViews; curr++ )
							  if (m_uiActiveViewId[curr] == m_OpViewId[uiOp][uiView])
								  break;
						  if (curr == m_uiNumActiveViews)
						  {
							m_uiActiveViewId[m_uiNumActiveViews] = m_OpViewId[uiOp][uiView];
							m_uiNumActiveViews++;
						  }
					  }
					 
					}
					printf("\n View Scalability Info SEI message received. \n");
			
				  }
//SEI }
				else if( pcSEIMessage->getMessageType() == SEI::MULTIVIEW_SCENE_INFO_SEI ) // SEI JVT-W060
				  {
					m_uiMultiviewSceneInfoSeiFlag = 1;
					m_uiMaxDisparity  = ((SEI::MultiviewSceneInfoSei*)pcSEIMessage)->getMaxDisparity();
					printf("\n Multiview Scene Info SEI message received. \tMaxDisparity=%d\n",m_uiMaxDisparity);
				  }
				else if ( pcSEIMessage->getMessageType() == SEI::MULTIVIEW_ACQUISITION_INFO_SEI) // SEI JVT-W060
				{
					printf("\n Multiview Acquisition Info SEI message received\n");
					m_uiMultiviewAcquisitionInfoSeiFlag = 1;					
					UInt uiIndex;
					int j = 0; //fix for MERL, ying

					UInt NumViewMinus1 = (UInt) ((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getNumViewMinus1();
					Bool m_bIntrinsicParamFlag=(Bool)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getIntrinsicParamFlag();
					printf("NumViewMunis1= %d\n", NumViewMinus1);                    
					printf("IntrinsicParamFlag= %d\n", m_bIntrinsicParamFlag);
						
					if (m_bIntrinsicParamFlag) 
					{
						Bool m_bIntrinsicParamsEqual=(Bool)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getIntrinsicParamsEqual();
						printf("IntrinsicParamsEqual = %d\n",m_bIntrinsicParamsEqual );
						UInt	m_uiPrecFocalLength= (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getPrecFocalLength();
						printf("PrecFocalLength = %d\n",m_uiPrecFocalLength );						
						UInt	m_uiPrecPrincipalPoint= (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getPrecPrincipalPoint();
						printf("PrecPrincipalPoint = %d\n",m_uiPrecPrincipalPoint );						
						UInt	m_uiPrecRadialDistortion= (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getPrecRadialDistortion();
						printf("PrecRadialDistortion = %d\n",m_uiPrecRadialDistortion );
						
						UInt sign_dec,exp_dec,mant_dec,var_length;
						double reconst=0.0;

						for( uiIndex = 0; uiIndex <= NumViewMinus1; uiIndex++ )	
						{
							printf("SignFocalLengthX[%d]=%d\n",uiIndex,sign_dec=(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getSignFocalLengthX(uiIndex));
							printf("ExponentFocalLengthX[%d]=%d\n",uiIndex,exp_dec=(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getExponentFocalLengthX(uiIndex));
							printf("MantissaFocalLengthX[%d]=%d\n",uiIndex,mant_dec=(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getMantissaFocalLengthX(uiIndex));
							var_length = (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getVarLength(exp_dec,m_uiPrecFocalLength);
							if (!(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getReconstSceneAcqSEI(exp_dec, sign_dec, mant_dec, var_length, &reconst))
								printf("Reconstructed FocalLengthX[%d]=%f\n",uiIndex,reconst);
							else
								printf("Invalid FocalLengthX[%d]\n",uiIndex);
							
							printf("SignFocalLengthY[%d]=%d\n",uiIndex,sign_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getSignFocalLengthY(uiIndex));
							printf("ExponentFocalLengthY[%d]=%d\n",uiIndex,exp_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getExponentFocalLengthY(uiIndex));
							printf("MantissaFocalLengthY[%d]=%d\n",uiIndex,mant_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getMantissaFocalLengthY(uiIndex));
							var_length = (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getVarLength(exp_dec,m_uiPrecFocalLength);							
							if (!(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getReconstSceneAcqSEI(exp_dec, sign_dec, mant_dec, var_length, &reconst))
								printf("Reconstructed FocalLengthY[%d]=%f\n",uiIndex,reconst);
							else
								printf("Invalid FocalLengthY[%d]\n",uiIndex);

							printf("SignPrincipalPointX[%d]=%d\n",uiIndex,sign_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getSignPrincipalPointX(uiIndex));
							printf("ExponentPrincipalPointX[%d]=%d\n",uiIndex,exp_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getExponentPrincipalPointX(uiIndex));
							printf("MantissaPrincipalPointX[%d]=%d\n",uiIndex,mant_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getMantissaPrincipalPointX(uiIndex));
							var_length = (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getVarLength(exp_dec,m_uiPrecPrincipalPoint);
							if (!(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getReconstSceneAcqSEI(exp_dec, sign_dec, mant_dec, var_length, &reconst))
								printf("Reconstructed PrincipalPointX[%d]=%f\n",uiIndex,reconst);
							else
								printf("Invalid PrincipalPointX[%d]\n",uiIndex);

							printf("SignPrincipalPointY[%d]=%d\n",uiIndex,sign_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getSignPrincipalPointY(uiIndex));
							printf("ExponentPrincipalPointY[%d]=%d\n",uiIndex,exp_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getExponentPrincipalPointY(uiIndex));
							printf("MantissaPrincipalPointY[%d]=%d\n",uiIndex,mant_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getMantissaPrincipalPointY(uiIndex));
							var_length = (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getVarLength(exp_dec,m_uiPrecPrincipalPoint);							
							if (!(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getReconstSceneAcqSEI(exp_dec, sign_dec, mant_dec, var_length, &reconst))
								printf("Reconstructed PrincipalPointY[%d]=%f\n",uiIndex,reconst);
							else
								printf("Invalid PrincipalPointY[%d]\n",uiIndex);

							printf("SignRadialDistortion[%d]=%d\n",uiIndex,sign_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getSignRadialDistortion(uiIndex));
							printf("ExponentRadialDistortion[%d]=%d\n",uiIndex,exp_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getExponentRadialDistortion(uiIndex));
							printf("MantissaRadialDistortion[%d]=%d\n",uiIndex,mant_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getMantissaRadialDistortion(uiIndex));
							var_length = (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getVarLength(exp_dec,m_uiPrecRadialDistortion);
							if (!(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getReconstSceneAcqSEI(exp_dec, sign_dec, mant_dec, var_length, &reconst))
								printf("Reconstructed RadialDistortion[%d]=%f\n",uiIndex,reconst);
							else
								printf("Invalid RadialDistortion[%d]\n",uiIndex);

						}
					}
					Bool m_bExtrinsicParamFlag=(Bool)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getExtrinsicParamFlag();
					
					if (m_bExtrinsicParamFlag) {
						UInt sign_dec,exp_dec,mant_dec,var_length;
						double reconst=0.0;

						UInt	m_uiPrecRotationParam= (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getPrecRotationParam();
						printf("PrecRotationParam = %d\n",m_uiPrecRotationParam );						
						UInt	m_uiPrecTranslationParam= (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getPrecTranslationParam();
						printf("PrecTranslationParam = %d\n",m_uiPrecTranslationParam );						
						
						for( uiIndex = 0; uiIndex <= NumViewMinus1; uiIndex++ )
						{
							for (i=0;i<3;i++) {
								printf("SignTranslationParam[%d][%d]=%d\n",uiIndex,i,sign_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getSignTranslationParam(uiIndex,i));
								printf("ExponentTranslationParam[%d][%d]=%d\n",uiIndex,i,exp_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getExponentTranslationParam(uiIndex,i));
								printf("MantissaTranslationParam[%d][%d]=%d\n",uiIndex,i,mant_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getMantissaTranslationParam(uiIndex,i));
								var_length = (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getVarLength(exp_dec,m_uiPrecTranslationParam);
								if (!(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getReconstSceneAcqSEI(exp_dec, sign_dec, mant_dec, var_length, &reconst))
									printf("Reconstructed TranslationParam[%d][%d]=%f\n",uiIndex,i,reconst);
								else
									printf("Invalid TranslationParam[%d][%d]\n",uiIndex,i);

								for (j=0;j<3;j++) {
									printf("SignRotationParam[%d][%d][%d]=%d\n",uiIndex,i,j,sign_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getSignRotationParam(uiIndex,i,j));
									printf("ExponentRotationParam[%d][%d][%d]=%d\n",uiIndex,i,j,exp_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getExponentRotationParam(uiIndex,i,j));
									printf("MantissaRotationParam[%d][%d][%d]=%d\n",uiIndex,i,j,mant_dec=((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getMantissaRotationParam(uiIndex,i,j));
									var_length = (UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getVarLength(exp_dec,m_uiPrecRotationParam);
									if (!(UInt)((SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage)->getReconstSceneAcqSEI(exp_dec, sign_dec, mant_dec, var_length, &reconst))
										printf("Reconstructed RotationParam[%d][%d][%d]=%f\n",uiIndex,i,j,reconst);
									else
										printf("Invalid RotationParam[%d][%d][%d]\n",uiIndex,i,j);
								}

							}							
						}	
					}
				}
        //JVT-AB025 {{

        else if (pcSEIMessage->getMessageType() == SEI::NON_REQ_VIEW_INFO_SEI)
        {
          m_uiNumTargetViewMinus1 = ((SEI::NonReqViewInfoSei*)pcSEIMessage)->getNumTargetViewMinus1();
		  m_auiViewOrderIndex = new UInt[m_uiNumTargetViewMinus1];
		  m_auiNumNonReqViewCop = new UInt[m_uiNumTargetViewMinus1];
		  m_aauiIndexDelta = new UInt*[m_uiNumTargetViewMinus1];
		  m_aauiNonReqViewOrderIndex = new UInt*[m_uiNumTargetViewMinus1];

          for ( i =0; i<= m_uiNumTargetViewMinus1; i++)
          {
			m_aauiIndexDelta[i] = new UInt[m_uiNumTargetViewMinus1];
 		    m_aauiNonReqViewOrderIndex[i] = new UInt[m_uiNumTargetViewMinus1];

            m_auiViewOrderIndex[i] = ((SEI::NonReqViewInfoSei*)pcSEIMessage)->getTargetViewOrderIndex()[i];
            m_auiNumNonReqViewCop[i] = ((SEI::NonReqViewInfoSei*)pcSEIMessage)->getNumNonReqViewCopMinus1()[i] + 1;
            for (UInt j = 0; j < m_auiNumNonReqViewCop[i]; j++)
            {
              m_aauiIndexDelta[i][j] = ((SEI::NonReqViewInfoSei*)pcSEIMessage)->getindexDeltaMinus1()[i][j];
              m_aauiNonReqViewOrderIndex[i][j] = m_auiViewOrderIndex[i] - m_aauiIndexDelta[i][j] - 1;
            }
			
			delete [] m_aauiIndexDelta[i];
 		    delete [] m_aauiNonReqViewOrderIndex[i];
          }
		  delete [] m_aauiIndexDelta;
 		  delete [] m_aauiNonReqViewOrderIndex;	
        }
		else if( pcSEIMessage->getMessageType() == SEI::VIEW_DEPENDENCY_STRUCTURE_SEI )
		{
			printf("\n View dependency structure SEI message received. \n");
		}
        else if (pcSEIMessage->getMessageType() == SEI::OP_NOT_PRESENT_SEI)
        {
          m_uiOPNotPresentSeiFlag = 1;
          m_uiNumNotPresentOP = ((SEI::OPNotPresentSei*)pcSEIMessage)->getNumNotPresentOP();
          for ( i = 0; i< m_uiNumNotPresentOP; i++)
          {
            m_auiNotPresentOPID[i] = ((SEI::OPNotPresentSei*)pcSEIMessage)->getNotPresentOPId()[i];
          }
        }
        //JVT-AB025 }}
			    delete pcSEIMessage;
		}
      }
      ruiEndPos = (uiBitsLeft+7)/8; //FRAG_FIX
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;
  case NAL_UNIT_ACCESS_UNIT_DELIMITER:
    {
      RNOK ( m_pcNalUnitParser->readAUDelimiter());
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;
  case NAL_UNIT_END_OF_SEQUENCE:
    {
      RNOK ( m_pcNalUnitParser->readEndOfSeqence());
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
  case NAL_UNIT_END_OF_STREAM:
    {
      RNOK ( m_pcNalUnitParser->readEndOfStream());
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;
  default:
    bSkip = true;
    return Err::m_nOK;
    break;
  }

  m_uiNonRequiredPic = 0; //NonRequired JVT-Q066
  //ruiNonRequiredPic = 0;

  if(m_pcSliceHeader)
  {
//SEI {
	  if( m_uiScalableNestingSeiRead == 0 && m_uiScalableNestingSeiFlag == 1 )
	  {
		  m_uiScalableNestingSeiRead = 1;
		  if( m_uiSnapshotSeiFlag == 1 )
			  m_uiPoc = m_pcSliceHeader->getPicOrderCntLsb();
	  }
	  else if( m_uiScalableNestingSeiRead == 1 )
	  {
		  if( m_pcSliceHeader->getPicOrderCntLsb() == m_uiPoc )
		  {
			  if( m_bAllPicturesInAuFlag == 1 )
				  m_pcSliceHeader->setSnapShotId( m_uiSnapShotId );
			  else
			  {
				  for( UInt index = 0; index <= m_uiNumPicturesMinus1; index++ )
					  if( m_uiPicId[index] == m_pcSliceHeader->getViewId() )
						  m_pcSliceHeader->setSnapShotId( m_uiSnapShotId );
			  }
		  }
	  }
//SEI }
	  m_uiCurrPicLayer = (m_pcSliceHeader->getLayerId() << 4) + m_pcSliceHeader->getQualityLevel();
	  if(m_uiCurrPicLayer == 0 || m_uiCurrPicLayer <= m_uiPrevPicLayer)
	  {
		  if(m_uiNonRequiredSeiReadFlag == 0 && m_pcNonRequiredSei)
		  {
			  m_pcNonRequiredSei->destroy();
			  m_pcNonRequiredSei = NULL;
		  }
		  m_uiNonRequiredSeiRead = m_uiNonRequiredSeiReadFlag;
		  m_uiNonRequiredSeiReadFlag = 0;
	  }
	  m_uiPrevPicLayer = m_uiCurrPicLayer;

	  if(m_uiNonRequiredSeiRead == 1)
	  {
		  for(UInt i = 0; i <= m_pcNonRequiredSei->getNumInfoEntriesMinus1(); i++)
		  {
			  if(m_pcNonRequiredSei->getEntryDependencyId(i))  // it should be changed to if(DenpendencyId == LayerId of the shown picture) 
			  {
				  for(UInt j = 0; j <= m_pcNonRequiredSei->getNumNonRequiredPicsMinus1(i); j++)
				  {
					  if(m_pcSliceHeader->getLayerId() == m_pcNonRequiredSei->getNonRequiredPicDependencyId(i,j) &&
						  m_pcSliceHeader->getQualityLevel() == m_pcNonRequiredSei->getNonRequiredPicQulityLevel(i,j))  // it should be add something about FragmentFlag
					  {
						  m_uiNonRequiredPic = 1;  //NonRequired JVT-Q066
						//  ruiNonRequiredPic = 1;
						  //ROTRS( m_apcMCTFDecoder[m_pcSliceHeader->getLayerId()]->getWaitForIdr() && !m_pcSliceHeader->isIdrNalUnit(), Err::m_nOK );
						  //m_apcMCTFDecoder[m_pcSliceHeader->getLayerId()]->setWaitForIdr(false);
						  return Err::m_nOK;
					  }
				  }
			  }
		  }
	  }
  }
  return Err::m_nOK;
}

//JVT-P031
ErrVal
H264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor)
{
  return m_pcNalUnitParser->initSODBNalUnit(pcBinDataAccessor);
}
//~JVT-P031

//JVT-S036  start
ErrVal
H264AVCDecoder::initPacketPrefix( BinDataAccessor*  pcBinDataAccessor,
                                  UInt&             ruiNalUnitType
                                  , Bool            bPreParseHeader 
                                  , Bool			      bConcatenated 
                                  , Bool&           rbStartDecoding
                                  ,SliceHeader     *pcSliceHeader
                           ,NalUnitParser*          pcNalUnitParser

  )
{
  ROF( m_bInitDone );


  ROT( NULL == pcBinDataAccessor );
  if ( NULL == pcBinDataAccessor->data() || 0 == pcBinDataAccessor->size() )
  {
    // switch 
    SliceHeader* pTmp = m_pcSliceHeader;
    m_pcSliceHeader   = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader = pTmp;


    m_bLastFrame = true;
    delete m_pcSliceHeader;
    m_pcSliceHeader = 0;
    rbStartDecoding = true; //JVT-P031

    return Err::m_nOK;
  }


  //JVT-P031
  Bool KeyPicFlag = false;
  getDecodedResolution(m_uiDecodedLayer);
  m_pcNalUnitParser->setDecodedLayer(m_uiDecodedLayer);
  //~JVT-P031
  UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
  RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, &KeyPicFlag,uiNumBytesRemoved, bPreParseHeader , bConcatenated) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC

  ruiNalUnitType = m_pcNalUnitParser->getNalUnitType();
  

//use the suffx NAL unit to set the values in the AVC compatible NAL unit
  pcNalUnitParser->setSvcMvcFlag( m_pcNalUnitParser->getSvcMvcFlag() );
  pcNalUnitParser->setPriorityID( m_pcNalUnitParser->getPriorityID() ); //JVT-W035
  pcNalUnitParser->setNonIDRFlag   ( m_pcNalUnitParser->getNonIDRFlag()    ); //JVT-W035  
  pcNalUnitParser->setViewId    ( m_pcNalUnitParser->getViewId()     );
  pcNalUnitParser->setAnchorPicFlag(m_pcNalUnitParser->getAnchorPicFlag());
	pcNalUnitParser->setInterViewFlag (m_pcNalUnitParser->getInterViewFlag()); //JVT-W056  Samsung

//TMM_EC }}
  if ( m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_PREFIX) 
  {

        RNOK( m_pcSliceReader->readSliceHeaderPrefix( m_pcNalUnitParser->getNalUnitType   (),
                                                  m_pcNalUnitParser->getSvcMvcFlag(),
                                                  
                                                  m_pcNalUnitParser->getNalRefIdc     (),
                                                  m_pcNalUnitParser->getLayerId		  (),
                                                  m_pcNalUnitParser->getQualityLevel  (),
                                                  pcSliceHeader
            ) 
      );

			return Err::m_nOK;	

  }

  return Err::m_nOK;
}

//JVT-S036  end

ErrVal
H264AVCDecoder::getBaseLayerPWTable( SliceHeader::PredWeightTable*& rpcPredWeightTable,
                                     UInt                           uiBaseLayerId,
                                     ListIdx                        eListIdx,
                                     Int                            iPoc )
{
  if( uiBaseLayerId /*|| m_apcMCTFDecoder[uiBaseLayerId]->isActive()*/ )
  {
    //RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerPWTable( rpcPredWeightTable, eListIdx, iPoc ) );
    return Err::m_nOK;
  }
  rpcPredWeightTable = &m_acLastPredWeightTable[eListIdx];
  return Err::m_nOK;
}
/*
ErrVal
H264AVCDecoder::getBaseLayerData( IntFrame*&      pcFrame,
                                  IntFrame*&      pcResidual,
                                  MbDataCtrl*&    pcMbDataCtrl,
                                  MbDataCtrl*&    pcMbDataCtrlEL,
                                  Bool&           rbConstrainedIPred,
                                  Bool&           rbSpatialScalability,
                                  UInt            uiLayerId,
                                  UInt            uiBaseLayerId,
                                  Int             iPoc,
                                  UInt            uiBaseQualityLevel) //JVT-T054
{
//JVT-T054{
  if(uiBaseLayerId == uiLayerId && (!m_apcMCTFDecoder[uiLayerId]->getAVCBased() || uiBaseQualityLevel != 0) )
  {
    rbSpatialScalability = false;
    RNOK( m_apcMCTFDecoder[uiLayerId]->getBaseLayerData( pcFrame, pcResidual, pcMbDataCtrl, pcMbDataCtrlEL, rbConstrainedIPred, rbSpatialScalability, iPoc ) );
  }
  else  
  {
    if(uiBaseLayerId == uiLayerId && m_apcMCTFDecoder[uiLayerId]->getAVCBased() && uiBaseQualityLevel == 0)
    {
      rbSpatialScalability = false;
      FrameUnit*  pcFrameUnit = m_pcFrameMng->getReconstructedFrameUnit( iPoc );
      ROF( pcFrameUnit );
      pcFrame             = rbSpatialScalability ? m_pcFrameMng->getRefinementIntFrame2() : m_pcFrameMng->getRefinementIntFrame();
      pcResidual          = pcFrameUnit ->getResidual();
      pcMbDataCtrl        = pcFrameUnit ->getMbDataCtrl();
      rbConstrainedIPred  = pcFrameUnit ->getContrainedIntraPred();
      m_apcMCTFDecoder[0]->setILPrediction(pcFrameUnit->getFGSIntFrame());
    }
    else
    {
//JVT-T054}
  if( uiBaseLayerId || (m_apcMCTFDecoder[uiBaseLayerId]->isActive() && !m_bOnlyAVCAtLayer) ) //JVT-T054
  {
    //===== base layer is scalable extension =====
    //--- get spatial resolution ratio ---
    // TMM_ESS {
	  if( m_apcMCTFDecoder[uiLayerId]->getFrameHeight() != m_apcMCTFDecoder[uiBaseLayerId]->getFrameHeight() &&
        m_apcMCTFDecoder[uiLayerId]->getFrameWidth () != m_apcMCTFDecoder[uiBaseLayerId]->getFrameWidth ()   )
    {
      rbSpatialScalability = true;
    }
    else
    {
      rbSpatialScalability = false;
    }
	  // TMM_ESS }

    //--- get data ---
	RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerData( pcFrame, pcResidual, pcMbDataCtrl, pcMbDataCtrlEL, rbConstrainedIPred, rbSpatialScalability, iPoc ) );
  }
  else
  {
    //===== base layer is standard H.264/AVC =====
    //--- get spatial resolution ratio ---
    // TMM_ESS {
	  if( m_apcMCTFDecoder[uiLayerId]->getFrameHeight() == 16 * m_pcVeryFirstSliceHeader->getSPS().getFrameHeightInMbs () &&
        m_apcMCTFDecoder[uiLayerId]->getFrameWidth () == 16 * m_pcVeryFirstSliceHeader->getSPS().getFrameWidthInMbs  ()   )
    {
      rbSpatialScalability = false;
    }
    else
    {
      rbSpatialScalability = true;
    }
	  // TMM_ESS }

    FrameUnit*  pcFrameUnit = m_pcFrameMng->getReconstructedFrameUnit( iPoc );
    ROF( pcFrameUnit );

//TMM_EC {{
    if ( m_apcMCTFDecoder[uiLayerId]->m_eErrorConcealTemp != EC_RECONSTRUCTION_UPSAMPLE)
      pcFrame           = rbSpatialScalability ? m_pcFrameMng->getRefinementIntFrame2() : m_pcFrameMng->getRefinementIntFrame();
    else
      pcFrame           = pcFrameUnit->getFGSIntFrame();
//TMM_EC }}
    pcResidual          = pcFrameUnit ->getResidual();
    pcMbDataCtrl        = pcFrameUnit ->getMbDataCtrl();
		pcMbDataCtrlEL      = m_pcBaseLayerCtrlEL;
    rbConstrainedIPred  = pcFrameUnit ->getContrainedIntraPred();
  }
//JVT-T054{
    }
  }
//JVT-T054}
  return Err::m_nOK;
}
*/ 
ErrVal
H264AVCDecoder::process( PicBuffer*       pcPicBuffer,
                         PicBufferList&   rcPicBufferOutputList,
                         PicBufferList&   rcPicBufferUnusedList,
                         PicBufferList&   rcPicBufferReleaseList )
 {
  ROF( m_bInitDone );

  RNOK( xInitSlice( m_pcSliceHeader ) );

  

  if( m_bLastFrame )
  {
      if( (m_uiRecLayerId > 0 || !m_bBaseLayerIsAVCCompatible) && m_pcNalUnitParser->getSvcMvcFlag())
    {
      PicBufferList cDummyList;
      
      for( UInt uiLayer = 0; uiLayer < m_uiRecLayerId; uiLayer++ )
      {
        if( uiLayer == 0 && m_bBaseLayerIsAVCCompatible )
        {
          RNOK( m_pcFrameMng->outputAll() );
          RNOK( m_pcFrameMng->setPicBufferLists( cDummyList, rcPicBufferReleaseList ) );
        }
        /*RNOK( m_apcMCTFDecoder[uiLayer]       ->finishProcess( cDummyList,
                                                               rcPicBufferUnusedList,
                                                               iMaxPoc ) );*/
      }
      /*RNOK  ( m_apcMCTFDecoder[m_uiRecLayerId]->finishProcess( rcPicBufferOutputList,
                                                               rcPicBufferUnusedList,
                                                               iMaxPoc = MSYS_INT_MIN ) );*/
    }
    else
    {
      //===== output all remaining frames in decoded picture buffer =====
//JVT-T054{
      PicBufferList cDummyList;
     
      if(m_bCGSSNRInAU)
      {
        RNOK( m_pcFrameMng->outputAll() );
        RNOK( m_pcFrameMng->setPicBufferLists( cDummyList, rcPicBufferReleaseList ) );
        /*RNOK  ( m_apcMCTFDecoder[m_uiRecLayerId]->finishProcess( rcPicBufferOutputList,
                                                               rcPicBufferUnusedList,
                                                               iMaxPoc = MSYS_INT_MIN ) );*/

      }
      else
      {
        RNOK( m_pcFrameMng->outputAll() );
      RNOK( m_pcFrameMng->setPicBufferLists( rcPicBufferOutputList, rcPicBufferReleaseList ) );
      }
//JVT-T054}
    }

    rcPicBufferUnusedList.pushBack( pcPicBuffer );
    return Err::m_nOK;
  }

  const NalUnitType eNalUnitType  = m_pcNalUnitParser->getNalUnitType();

  //====== check for AVC baselayer FGS refinement =====

  if( ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ) &&
      m_bActive &&
      m_pcSliceHeader->getLayerId()   == 0 &&
      m_pcSliceHeader->getSliceType() == F_SLICE ) 
  {
//TMM_EC {{
    if ( m_pcSliceHeader->getTrueSlice())
    {
		}
    else
    {
      //===== switch slice headers and update =====
      SliceHeader*  pcTemp  = m_pcSliceHeader;
      m_pcSliceHeader       = m_pcPrevSliceHeader;
      m_pcPrevSliceHeader   = pcTemp;
    }
//TMM_EC }}
    rcPicBufferUnusedList.pushBack( pcPicBuffer );
    RNOK( m_pcNalUnitParser->closeNalUnit() );
    return Err::m_nOK;
  }


  //===== decode NAL unit =====
  switch( eNalUnitType )
  {
  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    {

      if(!m_pcNalUnitParser->getSvcMvcFlag() )
      {
        ROF( m_pcSliceHeader );
        ROF( m_pcSliceHeader->getLayerId() == 0 );
        m_uiLastLayerId    = m_pcSliceHeader->getLayerId();
//JVT-T054{
        Bool bHighestLayer;
        m_bLastNalInAU = (m_uiNumOfNALInAU == 0);
        if(m_bFGSRefInAU)
          bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId );
        else
          bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId && m_bLastNalInAU);
//JVT-T054}
// JVT-Q054 Red. Picture {
        if ( NULL != m_pcSliceHeader_backup )
        {
          RNOK( m_pcSliceHeader->sliceHeaderBackup( m_pcSliceHeader_backup ) );
        }
// JVT-Q054 Red. Picture }

//	TMM EC {{
        //m_apcMCTFDecoder[m_uiLastLayerId+1]->m_bBaseLayerLost	=	!m_pcSliceHeader->getTrueSlice();

        if ( !m_pcSliceHeader->getTrueSlice())
        {
          RNOK( xProcessSliceVirtual( *m_pcSliceHeader, m_pcPrevSliceHeader, pcPicBuffer ) );				
        }
        else
        {
          RNOK( xProcessSlice( *m_pcSliceHeader, 
                               m_pcPrevSliceHeader, 
                               pcPicBuffer,                           
                               rcPicBufferOutputList,
                               rcPicBufferUnusedList,
                               bHighestLayer ) );  //JVT-T054

          m_bOnlyAVCAtLayer = true; //JVT-T054
        }
//TMM_EC }}

        PicBufferList   cDummyList;
        PicBufferList&  rcOutputList  = ( m_uiRecLayerId == 0 ? rcPicBufferOutputList : cDummyList );

        RNOK( m_pcFrameMng->setPicBufferLists( rcOutputList, rcPicBufferReleaseList ) );        
//JVT-T054{
        m_bAVCBased = true;
//JVT-T054}
      }
      else 
      {

        ROT( m_pcSliceHeader == NULL );
        m_uiLastLayerId = m_pcSliceHeader->getLayerId();
        m_bLastNalInAU = m_uiNumOfNALInAU == 0; //JVT-T054
// JVT-Q054 Red. Picture {
      if ( NULL != m_pcSliceHeader_backup )
      {
        RNOK( m_pcSliceHeader->sliceHeaderBackup( m_pcSliceHeader_backup ) );
      }
// JVT-Q054 Red. Picture }
      
      PicBufferList   cDummyList;
//JVT-T054{
      Bool bHighestLayer;
      if(m_bFGSRefInAU)
        bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId );
      else
        bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId && m_bLastNalInAU);
//JVT-T054}
//	TMM EC {{
      if ( m_pcNalUnitParser->getQualityLevel() == 0)
      {

      }
//TMM_EC }}
     
        RNOK( m_pcNalUnitParser                 ->closeNalUnit() );

      }

      return Err::m_nOK;
    }
    break;

  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
    {
      ROF( m_pcSliceHeader );
      ROF( m_pcSliceHeader->getLayerId() == 0 );
      m_uiLastLayerId    = m_pcSliceHeader->getLayerId();
      Bool bHighestLayer;
      m_bLastNalInAU = (m_uiNumOfNALInAU == 0);
      if(m_bFGSRefInAU)
        bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId );
      else
        bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId && m_bLastNalInAU);
      if ( NULL != m_pcSliceHeader_backup )
        RNOK( m_pcSliceHeader->sliceHeaderBackup( m_pcSliceHeader_backup ) );


		if ( !m_pcSliceHeader->getTrueSlice())
		{
			RNOK( xProcessSliceVirtual( *m_pcSliceHeader, m_pcPrevSliceHeader, pcPicBuffer ) );				
		}
		else
		{
			RNOK( xProcessSlice( *m_pcSliceHeader, 
								m_pcPrevSliceHeader, 
								pcPicBuffer, 
								rcPicBufferOutputList,
								rcPicBufferUnusedList,
								bHighestLayer ) ); 
			m_bOnlyAVCAtLayer = true; 
		}


      PicBufferList   cDummyList;
      PicBufferList&  rcOutputList  = ( (m_uiRecLayerId == 0) ? rcPicBufferOutputList : cDummyList ); //JVT-T054

      RNOK( m_pcFrameMng->setPicBufferLists( rcOutputList, rcPicBufferReleaseList ) );
      m_bAVCBased = true;
    }
    break;
  
  case NAL_UNIT_SPS:
  case NAL_UNIT_SUBSET_SPS:
    {
      break;
    }


  case NAL_UNIT_PPS:
  case NAL_UNIT_SEI:
	case NAL_UNIT_ACCESS_UNIT_DELIMITER:
	case NAL_UNIT_END_OF_SEQUENCE:
	case NAL_UNIT_END_OF_STREAM:
    {
    }
    break;
  
  default:
    {
      AF();
      return Err::m_nERR;
    }
    break;
  }

  rcPicBufferUnusedList.pushBack( pcPicBuffer );
  RNOK( m_pcNalUnitParser->closeNalUnit() );

  return Err::m_nOK;
}




ErrVal H264AVCDecoder::xStartSlice(Bool& bPreParseHeader, Bool& bLastFragment, Bool& bDiscardable, Bool UnitAVCFlag) //FRAG_FIX //TMM_EC  //JVT-S036 
{
  //JVT-P031
  SliceHeader * pSliceHeader = NULL;
  bLastFragment = false;
  if(m_pcNalUnitParser->getDiscardableFlag() == false || m_pcNalUnitParser->getLayerId() == m_uiDecodedLayer)
  {
      UInt uiPPSId = (m_pcSliceHeader != NULL) ? m_pcSliceHeader->getPicParameterSetId() : 0;
      UInt uiNumMbsInSlice = (m_pcSliceHeader != NULL) ? m_pcSliceHeader->getNumMbsInSlice() : 0;
      Bool bFGSCompSep = (m_pcSliceHeader != NULL) ? m_pcSliceHeader->getFgsComponentSep() : 0;
		if ( m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE_SCALABLE && !m_pcNalUnitParser->isTrueNalUnit())
		{
			RNOK( m_pcSliceReader->readSliceHeaderVirtual(m_pcNalUnitParser->getNalUnitType(),
																										//m_apcMCTFDecoder[1]->m_pcVeryFirstSliceHeader,
                                                                                                        m_pcVeryFirstSliceHeader,
																										m_uiDecompositionStages[m_uiNextLayerId],
																										m_uiMaxDecompositionStages,
																										m_uiGopSize[m_uiNextLayerId],
																										m_uiMaxGopSize,
																										*(Short*)(&m_pcNalUnitParser->m_pucBuffer[4]),
																										*(Short*)(&m_pcNalUnitParser->m_pucBuffer[6]),
																										m_pcNalUnitParser->m_pucBuffer[8],
																										pSliceHeader) );
			pSliceHeader->setTrueSlice( false);
		}
		else if ( m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE && !m_pcNalUnitParser->isTrueNalUnit())
		{
			RNOK( m_pcSliceReader->readSliceHeaderVirtual(m_pcNalUnitParser->getNalUnitType(),
																										m_pcVeryFirstSliceHeader,
																										m_uiDecompositionStages[m_uiNextLayerId],
																										m_uiMaxDecompositionStages,
																										m_uiGopSize[m_uiNextLayerId],
																										m_uiMaxGopSize,
																										*(Short*)(&m_pcNalUnitParser->m_pucBuffer[4]),
																										*(Short*)(&m_pcNalUnitParser->m_pucBuffer[6]),
																										m_pcNalUnitParser->m_pucBuffer[8],
																										pSliceHeader) );
			pSliceHeader->setTrueSlice( false);
		}
		else
		{

                  if(!m_pcNalUnitParser->getSvcMvcFlag() )
                  {
                    RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser->getNalUnitType   (),
                                                            m_pcNalUnitParser->getSvcMvcFlag(),	
                                                             m_pcNalUnitParser->getNonIDRFlag(),   //JVT-W035 
                                                            m_pcNalUnitParser->getAnchorPicFlag(),
                                                            m_pcNalUnitParser->getViewId(),
															m_pcNalUnitParser->getInterViewFlag(), //JVT-W056  Samsung
                                                            m_pcNalUnitParser->getNalRefIdc     (),
                                                            m_pcNalUnitParser->getLayerId       (),
                                                            m_pcNalUnitParser->getTemporalLevel (),
                                                            m_pcNalUnitParser->getQualityLevel  (),
                                                            pSliceHeader,
                                                            uiPPSId,
                                                            uiNumMbsInSlice,
                                                            bFGSCompSep
                                                            ,UnitAVCFlag	//JVT-S036 
                            ));
                  }
                  else
                    RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser->getNalUnitType   (),
                                                            m_pcNalUnitParser->getNalRefIdc     (),
                                                            m_pcNalUnitParser->getLayerId       (),
                                                            m_pcNalUnitParser->getTemporalLevel (),
                                                            m_pcNalUnitParser->getQualityLevel  (),
                                                            pSliceHeader,
                                                            uiPPSId,
                                                            uiNumMbsInSlice,
                                                            bFGSCompSep
                                                            ,UnitAVCFlag	//JVT-S036 
                            ));

                    pSliceHeader->setTrueSlice( true);
		}
//TMM_EC {{
		if (bPreParseHeader && m_pcSliceHeader != NULL && (m_pcSliceHeader->getPicOrderCntLsb() != pSliceHeader->getPicOrderCntLsb() || !m_pcSliceHeader->getTrueSlice()) && pSliceHeader->getSliceType() == F_SLICE && m_pcSliceHeader->getSliceType() != F_SLICE)
		{
			delete pSliceHeader; 
			pSliceHeader = 0;
			bLastFragment = true;
			bPreParseHeader	=	false;
			bDiscardable	=	true;
			return Err::m_nOK;
		}
//TMM_EC }}
    if( (!pSliceHeader->getFragmentedFlag()) || ( (pSliceHeader->getFragmentedFlag()) && (pSliceHeader->getFragmentOrder() == 0) ) )
    {
      if(bPreParseHeader) //FRAG_FIX
      {
// JVT-Q054 Red. Picture {
        if ( isRedundantPic() )
        {
          delete m_pcSliceHeader;
          m_pcSliceHeader = pSliceHeader;
        }
        else
        {
          delete m_pcPrevSliceHeader;
          m_pcPrevSliceHeader = m_pcSliceHeader;
          m_pcSliceHeader     = pSliceHeader;
        }
        //		  delete m_pcPrevSliceHeader;
        //      m_pcPrevSliceHeader = m_pcSliceHeader;
        //      m_pcSliceHeader     = pSliceHeader;
        m_uiLastFragOrder = 0;
// JVT-Q054 Red. Picture }
      } // FRAG_FIX
      else // memory leak fix provided by Nathalie
      {
        delete pSliceHeader; 
        pSliceHeader = 0;
      }
    }
    else
    {
      // just ensure that fragmented information are correct
      if(pSliceHeader->getFragmentOrder() != m_uiLastFragOrder+1)
      {
        printf("pb with fragment ordering information\n");
      }

      // set current slice header with last fragment info, in order to start the decoding process
      bLastFragment = true;
      delete pSliceHeader;
    }
  }
  else
  {
    // set current slice header with last fragment info, in order to start the decoding process
    bLastFragment = true;
  }
  //~JVT-P031
 
  return Err::m_nOK;
}



ErrVal
H264AVCDecoder::xZeroIntraMacroblocks( IntFrame*    pcFrame,
                                       MbDataCtrl*  pcMbDataCtrl,
                                       SliceHeader* pcSliceHeader )
{
  IntYuvPicBuffer*  pcPicBuffer       = pcFrame->getFullPelYuvBuffer ();
  UInt              uiFrameWidthInMB  = pcSliceHeader->getSPS().getFrameWidthInMbs ();
  UInt              uiFrameHeightInMB = pcSliceHeader->getSPS().getFrameHeightInMbs();
  UInt              uiMbNumber        = uiFrameWidthInMB * uiFrameHeightInMB;
	SliceHeader    *  pcSliceHeaderV    = pcMbDataCtrl->getSliceHeader();//TMM_EC
  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  IntYuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

  for( UInt uiMbIndex = 0; uiMbIndex < uiMbNumber; uiMbIndex++ )
  {
    MbDataAccess* pcMbDataAccess  = 0;

	UInt          uiMbY           = uiMbIndex / uiFrameWidthInMB;
    UInt          uiMbX           = uiMbIndex % uiFrameWidthInMB;
	RNOK( m_pcControlMng->initMbForFiltering(  pcMbDataAccess,uiMbY, uiMbX, pcSliceHeaderV->isMbAff() ) );

    RNOK( pcMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );


    if( pcMbDataAccess->getMbData().isIntra() )
    {
      pcPicBuffer->loadBuffer( &cZeroMbBuffer );
    }
  }
//TMM_EC{{ bug fix
	if(!pcSliceHeaderV->getTrueSlice()){
		RNOK(pcMbDataCtrl->initSlice(*pcSliceHeaderV,POST_PROCESS,false, NULL));}
//TMM_EC}} bug fix
  return Err::m_nOK;
}

//TMM_EC {{
ErrVal
H264AVCDecoder::xProcessSliceVirtual( SliceHeader&    rcSH,
                                      SliceHeader*    pcPrevSH,
                                      PicBuffer* &    rpcPicBuffer)
{
    //===== calculate POC =====
    if( rcSH.getLayerId() == 0 )
    {
        RNOK( m_pcPocCalculator->calculatePoc( rcSH ) );
    }
  UInt  uiMbRead;
  Bool  bNewFrame;
  Bool  bNewPic;

    //===== store prediction weights table for inter-layer prediction =====
  m_acLastPredWeightTable[LIST_0].copy( rcSH.getPredWeightTable( LIST_0 ) );
  m_acLastPredWeightTable[LIST_1].copy( rcSH.getPredWeightTable( LIST_1 ) );

  
  //===== set some basic parameters =====
  if( NULL == m_pcVeryFirstSliceHeader )
  {
    m_pcVeryFirstSliceHeader  = new SliceHeader( rcSH.getSPS(), rcSH.getPPS() );
  }

  printf("  Frame %4d ( LId 0, TL X, QL 0, AVC-%c, BId-1, AP 0, QP%3d, V )\n",
    rcSH.getPoc                    (),
    rcSH.getSliceType              () == B_SLICE ? 'B' : 
    rcSH.getSliceType              () == P_SLICE ? 'P' : 'I',
    rcSH.getPicQp                  () );

  //===== check if new pictures and initialization =====
  rcSH.compare( pcPrevSH, bNewPic, bNewFrame );
  if(pcPrevSH==NULL)
  {
//	  printf("NULL of previous slice header %d\n",rcSH.getPoc());
  }

  if( bNewFrame || m_bFrameDone )
  {
    RNOK( m_pcFrameMng->initFrame( rcSH, rpcPicBuffer ) );
    rpcPicBuffer = NULL;
  }
  if( bNewPic )
  {
    RNOK( m_pcFrameMng->initPic  ( rcSH ) );
  }
  else
  {
    rcSH.setFrameUnit( pcPrevSH->getFrameUnit() );
  }


  //===== set reference lists =====
  RNOK( m_pcFrameMng->setRefPicLists( rcSH, false ) );


  //===== parse slice =====
  RNOK( m_pcControlMng  ->initSlice ( rcSH, PARSE_PROCESS ) );
  uiMbRead	=	rcSH.getMbInPic();

  //===== decode slice =====
  Bool  bKeyPicture     = rcSH.getKeyPictureFlag();
  Bool  bConstrainedIP  = rcSH.getPPS().getConstrainedIntraPredFlag();

  Bool  bReconstruct    = (m_uiRecLayerId == 0) || ! bConstrainedIP;

  m_bReconstruct = bReconstruct;
  //***** NOTE: Motion-compensated prediction for non-key pictures is done in xReconstructLastFGS()
  bReconstruct   = (bReconstruct && bKeyPicture) || ! bConstrainedIP;

  RNOK( m_pcControlMng  ->initSlice ( rcSH, DECODE_PROCESS ) );

	if ( m_eErrorConceal == EC_BLSKIP || m_eErrorConceal == EC_TEMPORAL_DIRECT)

	{
		RNOK( m_pcSliceDecoder->processVirtual( rcSH, bReconstruct, uiMbRead ) );
	}
	else
	{
        Frame	*frame	=	(Frame*)(rcSH.getRefPicList( rcSH.getPicType() ,LIST_0 ).get(0).getFrame());
		m_pcFrameMng->getCurrentFrameUnit()->getFrame().getFullPelYuvBuffer()->loadBuffer( frame->getFullPelYuvBuffer());
	}

  Bool bPicDone;
  RNOK( m_pcControlMng->finishSlice( rcSH, bPicDone, m_bFrameDone ) );

  bPicDone	=	true;
  m_bFrameDone	=	true;
  
  if (IsSliceEndOfPic())
  {
		if ( m_eErrorConceal == EC_RECONSTRUCTION_UPSAMPLE || m_eErrorConceal == EC_FRAME_COPY)
		{
//			rcSH.getFrameUnit()->getFGSIntFrame()->copy( &m_pcFrameMng->getCurrentFrameUnit()->getFrame());
// memory 
		}
    // copy intra and inter prediction signal
//    m_pcFrameMng->getPredictionIntFrame()->getFullPelYuvBuffer()->copy( rcSH.getFrameUnit()->getFGSIntFrame()->getFullPelYuvBuffer());
// memory 
    // delete intra prediction
    RNOK( m_pcControlMng->initSlice( rcSH, POST_PROCESS));
//    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getFGSIntFrame(), rcSH.getFrameUnit()->getMbDataCtrl(), &rcSH ) );
// memory 
    // add residual and intra signal
//    rcSH.getFrameUnit()->getFGSIntFrame()->add( rcSH.getFrameUnit()->getResidual() );
//    rcSH.getFrameUnit()->getFGSIntFrame()->clip();

//    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getResidual(),
                                 //rcSH.getFrameUnit()->getMbDataCtrl(), m_pcVeryFirstSliceHeader ) );

    //===== deblocking of base representation =====
  
    RNOK( m_pcFrameMng->storePicture( rcSH ) );

    //===== init FGS decoder =====
//    RNOK( m_pcRQFGSDecoder->initPicture( &rcSH, rcSH.getFrameUnit()->getMbDataCtrl() ) );
  }

  if( m_bFrameDone )
  {
    DTRACE_NEWFRAME;
  }

  return Err::m_nOK;
}
//TMM_EC }}

ErrVal
H264AVCDecoder::xProcessSlice( SliceHeader& rcSH,
                               SliceHeader* pcPrevSH,
                               PicBuffer*&  rpcPicBuffer,
                               PicBufferList&   rcPicBufferOutputList,
                               PicBufferList&   rcPicBufferUnusedList,
                               Bool         bHighestLayer) //JVT-T054
{
    //===== calculate POC =====
    if( rcSH.getLayerId() == 0 )
    {
        RNOK( m_pcPocCalculator->calculatePoc( rcSH ) );
    }
    UInt  uiMbRead;

  Bool  bNewFrame;
  Bool  bNewPic;

  Bool bVeryFirstSlice=false;  

  //===== store prediction weights table for inter-layer prediction =====
  m_acLastPredWeightTable[LIST_0].copy( rcSH.getPredWeightTable( LIST_0 ) );
  m_acLastPredWeightTable[LIST_1].copy( rcSH.getPredWeightTable( LIST_1 ) );

  //===== set some basic parameters =====
  if( NULL == m_pcVeryFirstSliceHeader )
  {
    m_pcVeryFirstSliceHeader  = new SliceHeader( rcSH.getSPS(), rcSH.getPPS() );

    //--ICU/ETRI FMO Implementation
    m_pcVeryFirstSliceHeader->setSliceGroupChangeCycle( rcSH.getSliceGroupChangeCycle() ); // fix HS
    m_pcVeryFirstSliceHeader->FMOInit();  
    bVeryFirstSlice= true;

    // ICU/ETRI FGS_MOT_USE
    m_pcBaseLayerCtrlEL = new MbDataCtrl();
    m_pcBaseLayerCtrlEL->init(rcSH.getSPS());
  }
  else if ( m_bNewSPS && (rcSH.getSPS().getProfileIdc() != SCALABLE_PROFILE) )
  {
    delete m_pcVeryFirstSliceHeader;
    m_pcVeryFirstSliceHeader  = new SliceHeader( rcSH.getSPS(), rcSH.getPPS() );
    m_pcVeryFirstSliceHeader->setSliceGroupChangeCycle( rcSH.getSliceGroupChangeCycle() ); // fix HS
    m_pcVeryFirstSliceHeader->FMOInit();  // fix HS
  }
  m_bNewSPS = false;

#if JM_MVC_COMPATIBLE
#define DELTA_POCA  DELTA_POC
#else
#define DELTA_POCA  0
#endif

	PicType ePictype = rcSH.getPicType();
	printf("(%2d)  Slice POC %4d ( LId 0, TL X, QL 0, %s-%c, BId-1, AP 0, QP%3d, %s )\n",
		rcSH.getViewId                 (),
		rcSH.getPoc                () - DELTA_POCA,
		rcSH.isH264AVCCompatible () == 1 ? "AVC" : "MVC",
		rcSH.getSliceType              () == B_SLICE ? 'B' : 
		rcSH.getSliceType              () == P_SLICE ? 'P' : 'I',
		rcSH.getPicQp                  (),
		(ePictype == FRAME) ?  "FRAME" : ( (ePictype == TOP_FIELD) ? "TOP_FIELD" : "BOT_FIELD") );


  //===== check if new pictures and initialization =====
  rcSH.compare( pcPrevSH, bNewPic, bNewFrame );

  if(!bNewFrame)
  {
	  if(rcSH.getFieldPicFlag())
	  {
		  if(pcPrevSH->getFrameUnit()&&pcPrevSH->getFrameUnit()->getMbDataCtrl()->isFrameDone(*pcPrevSH))
			  bNewFrame=true;//lufeng: field for new frame
	  } 
  }

  if( bNewFrame )//|| m_bFrameDone )
  {
     RNOK( m_pcFrameMng->initFrame( rcSH, rpcPicBuffer ) );

    if(!bHighestLayer /*|| !m_apcMCTFDecoder[0]->isActive()*/) //JVT-T054
    {
      rpcPicBuffer = NULL;
    }
  }
  if( bNewPic )
  {
    RNOK( m_pcFrameMng->initPic  ( rcSH ) );
  }
  else
  {
    rcSH.setFrameUnit( pcPrevSH->getFrameUnit() );
  }


  //===== set reference lists =====
  RNOK( m_pcFrameMng->setRefPicLists( rcSH, false ) );
// Spatial direct fix: a inter-view (only) view component as a reference is not considered as short term ref. -Ying
  if ( rcSH.isInterB() )
  {
    RefPicList<RefPic>& rcList = rcSH.getRefPicList( rcSH.getPicType(),LIST_1);
    ROTRS( 0 == rcList.bufSize(), Err::m_nOK );
    Bool bList1ShortTerm = ( rcList.get(0).getFrame()->getViewId() == rcSH.getViewId() ? true : false ) ;
  
    rcSH.setList1FirstShortTerm ( bList1ShortTerm );
  }

  //===== parse slice =====
  RNOK( m_pcControlMng  ->initSlice ( rcSH, PARSE_PROCESS ) );

  RNOK( m_pcSliceReader ->process   ( rcSH, uiMbRead ) );

  //===== decode slice =====


  Bool  bKeyPicture     = rcSH.getKeyPictureFlag();
  Bool  bConstrainedIP  = rcSH.getPPS().getConstrainedIntraPredFlag();
  Bool  bReconstruct    = (m_uiRecLayerId == 0) || ! bConstrainedIP || bHighestLayer; //JVT-T054
  m_bReconstruct  = bReconstruct;
  //***** NOTE: Motion-compensated prediction for non-key pictures is done in xReconstructLastFGS()
  bReconstruct    = (bReconstruct && bKeyPicture) || ! bConstrainedIP;

  RNOK( m_pcControlMng  ->initSlice ( rcSH, DECODE_PROCESS ) );

  RNOK( m_pcSliceDecoder->process   ( rcSH, bReconstruct, uiMbRead ) );

  Bool bPicDone;
  RNOK( m_pcControlMng->finishSlice( rcSH, bPicDone, m_bFrameDone )  );

  if( bPicDone )
  {
    
    RNOK( m_pcControlMng->initSlice( rcSH, POST_PROCESS));
    
    //===== deblocking of base representation =====
    RNOK( m_pcLoopFilter->process( rcSH ) );
    
	RNOK( m_pcFrameMng->storePicture( rcSH ) );
    
    //===== init FGS decoder =====
    
 
  }

  if( m_bFrameDone )
  {
    DTRACE_NEWFRAME;
  }

  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::xInitSlice( SliceHeader* pcSliceHeader )
{
  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
// *LMH: Inverse MCTF
    UInt  uiLastLayer;
    if( m_uiLastLayerId != MSYS_UINT_MAX && m_uiLastLayerId == m_uiRecLayerId )
      uiLastLayer = m_uiRecLayerId;
    else
      uiLastLayer = MAX_LAYERS;
  }
  ROFRS( m_bActive, Err::m_nOK );

  //===== calculate POC =====
  if( pcSliceHeader && pcSliceHeader->getLayerId() == 0 )
  {
    RNOK( m_pcPocCalculator->calculatePoc( *pcSliceHeader ) );
  }

  if (pcSliceHeader!=NULL) {

	pcSliceHeader->setViewId        (m_pcNalUnitParser->getViewId());
	pcSliceHeader->setAVCFlag       (m_pcNalUnitParser->getNalUnitType()< 14); //JVT-W035
	pcSliceHeader->setSvcMvcFlag    (m_pcNalUnitParser->getSvcMvcFlag());
	pcSliceHeader->setAnchorPicFlag (m_pcNalUnitParser->getAnchorPicFlag());
	pcSliceHeader->setNonIDRFlag    (m_pcNalUnitParser->getNonIDRFlag());  
	pcSliceHeader->setInterViewFalg (m_pcNalUnitParser->getInterViewFlag()); //JVT-W056 Samsung
	pcSliceHeader->setReservedOneBit(1); // bug fix: prefix NAL (NTT)
  }

  return Err::m_nOK;
}

//JVT-T054{
Void  H264AVCDecoder::setFGSRefInAU(Bool &b)
{ 
  m_bFGSRefInAU = b;
}


ErrVal
H264AVCDecoder::freeDiffPrdRefLists( RefFrameList& diffPrdRefList)
{
  for(UInt i=0; i< diffPrdRefList.getSize(); i++)
  {
    diffPrdRefList.getEntry(i)->uninit();
    free(diffPrdRefList.getEntry(i));
  }

  return Err::m_nOK;
}

// JVT-Q054 Red. Picture {
ErrVal
H264AVCDecoder::checkRedundantPic()
{
  m_bRedundantPic = false;
  if ( m_bFirstSliceHeaderBackup && (NULL != m_pcSliceHeader) )
  {
    ROF( ( m_pcSliceHeader_backup = new SliceHeader ( m_pcSliceHeader->getSPS(), m_pcSliceHeader->getPPS()) ) );
    m_bFirstSliceHeaderBackup = false;
  }
  else
  {
    if ( ( NULL != m_pcSliceHeader ) && ( NULL != m_pcSliceHeader_backup ) )
    {
      const NalUnitType eNalUnitType  = m_pcNalUnitParser->getNalUnitType();
      if ( ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ) || ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
        || ( eNalUnitType == NAL_UNIT_CODED_SLICE ) || ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR ) )
      {
        Bool  bNewFrame  = true;
        RNOK( m_pcSliceHeader->compareRedPic ( m_pcSliceHeader_backup, bNewFrame ) );
        if (!bNewFrame)
        {
          m_bRedundantPic = true;
        }
      }
    }
  }
  return Err::m_nOK;
}


// JVT-Q054 Red. Picture }

H264AVC_NAMESPACE_END


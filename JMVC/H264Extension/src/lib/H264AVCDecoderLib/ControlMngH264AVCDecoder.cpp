#include <cstdio>
#include "H264AVCDecoderLib.h"
#include "ControlMngH264AVCDecoder.h"


H264AVC_NAMESPACE_BEGIN

ControlMngH264AVCDecoder::ControlMngH264AVCDecoder():
  m_pcMbDataCtrl          ( NULL ),
  m_pcFrameMng            ( NULL ),
  m_pcParameterSetMng     ( NULL ),
  m_pcSliceReader         ( NULL ),
  m_pcNalUnitParser       ( NULL ),
  m_pcSliceDecoder        ( NULL ),
  m_pcControlMng          ( NULL ),
  m_pcBitReadBuffer       ( NULL ),
  m_pcUvlcReader          ( NULL ),
  m_pcMbParser            ( NULL ),
  m_pcLoopFilter          ( NULL ),
  m_pcMbDecoder           ( NULL ),
  m_pcTransform           ( NULL ),
  m_pcIntraPrediction     ( NULL ),
  m_pcMotionCompensation  ( NULL ),
  m_pcQuarterPelFilter    ( NULL ),
  m_pcCabacReader         ( NULL ),
  m_pcSampleWeighting     ( NULL ),
  m_uiCurrLayer           ( MSYS_UINT_MAX ),
  m_bLayer0IsAVC          ( true )
  , m_bMbAff              ( false ) // for future use
{
  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_auiMbXinFrame           [uiLayer] = 0;
    m_auiMbYinFrame           [uiLayer] = 0;
    //m_apcMCTFDecoder          [uiLayer] = NULL;
    m_apcPocCalculator        [uiLayer] = NULL;
    m_apcYuvFullPelBufferCtrl [uiLayer] = NULL;
    m_uiInitilized            [uiLayer] = false;
	m_uiInitilized_MultiView  [uiLayer] = false;
  }
}



ControlMngH264AVCDecoder::~ControlMngH264AVCDecoder()
{
}




ErrVal
ControlMngH264AVCDecoder::init( FrameMng*            pcFrameMng,
                             ParameterSetMng*     pcParameterSetMng,
                             PocCalculator*       apcPocCalculator        [MAX_LAYERS],
                             SliceReader*         pcSliceReader,
                             NalUnitParser*       pcNalUnitParser,
                             SliceDecoder*        pcSliceDecoder,
                             BitReadBuffer*       pcBitReadBuffer,
                             UvlcReader*          pcUvlcReader,
                             MbParser*            pcMbParser,
                             LoopFilter*          pcLoopFilter,
                             MbDecoder*           pcMbDecoder,
                             Transform*           pcTransform,
                             IntraPrediction*     pcIntraPrediction,
                             MotionCompensation*  pcMotionCompensation,
                             YuvBufferCtrl*       apcYuvFullPelBufferCtrl [MAX_LAYERS],
                             QuarterPelFilter*    pcQuarterPelFilter,
                             CabacReader*         pcCabacReader,
                             SampleWeighting*     pcSampleWeighting,
                             //MCTFDecoder*         apcMCTFDecoder          [MAX_LAYERS],
                             H264AVCDecoder*         pcH264AVCDecoder )
{ 

  ROT( NULL == pcFrameMng );
  ROT( NULL == pcParameterSetMng );
  ROT( NULL == pcSliceReader );
  ROT( NULL == pcNalUnitParser );
  ROT( NULL == pcSliceDecoder );
  ROT( NULL == pcBitReadBuffer );
  ROT( NULL == pcUvlcReader );
  ROT( NULL == pcMbParser );
  ROT( NULL == pcLoopFilter );
  ROT( NULL == pcMbDecoder );
  ROT( NULL == pcTransform );
  ROT( NULL == pcIntraPrediction );
  ROT( NULL == pcMotionCompensation );
  ROT( NULL == pcQuarterPelFilter );
  ROT( NULL == pcCabacReader );
  ROT( NULL == pcSampleWeighting );
  ROT( NULL == pcH264AVCDecoder );

  m_bLayer0IsAVC          = true;
  m_uiCurrLayer           = MSYS_UINT_MAX;
  m_pcFrameMng            = pcFrameMng; 
  m_pcParameterSetMng     = pcParameterSetMng;
  m_pcSliceReader         = pcSliceReader; 
  m_pcNalUnitParser       = pcNalUnitParser; 
  m_pcSliceDecoder        = pcSliceDecoder; 
  m_pcBitReadBuffer       = pcBitReadBuffer; 
  m_pcUvlcReader          = pcUvlcReader; 
  m_pcMbParser            = pcMbParser; 
  m_pcLoopFilter          = pcLoopFilter; 
  m_pcMbDecoder           = pcMbDecoder; 
  m_pcTransform           = pcTransform; 
  m_pcIntraPrediction     = pcIntraPrediction; 
  m_pcMotionCompensation  = pcMotionCompensation; 
  m_pcQuarterPelFilter    = pcQuarterPelFilter; 
  m_pcCabacReader         = pcCabacReader; 
  m_pcSampleWeighting     = pcSampleWeighting; 
  m_pcH264AVCDecoder         = pcH264AVCDecoder;

  //ROT( NULL == apcMCTFDecoder );
  ROT( NULL == apcPocCalculator );
  ROT( NULL == apcYuvFullPelBufferCtrl );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    //ROT( NULL == apcMCTFDecoder         [uiLayer] );
    ROT( NULL == apcPocCalculator       [uiLayer] );
    ROT( NULL == apcYuvFullPelBufferCtrl[uiLayer] );

    //m_apcMCTFDecoder          [uiLayer] = apcMCTFDecoder          [uiLayer];
    m_apcPocCalculator        [uiLayer] = apcPocCalculator        [uiLayer];
    m_apcYuvFullPelBufferCtrl [uiLayer] = apcYuvFullPelBufferCtrl [uiLayer];
  }
  
  return Err::m_nOK;
}




ErrVal ControlMngH264AVCDecoder::uninit()
{
  return Err::m_nOK;
}



ErrVal ControlMngH264AVCDecoder::create( ControlMngH264AVCDecoder*& rpcControlMngH264AVCDecoder )
{
  rpcControlMngH264AVCDecoder = new ControlMngH264AVCDecoder;

  ROT( NULL == rpcControlMngH264AVCDecoder );
  return Err::m_nOK;
}




ErrVal ControlMngH264AVCDecoder::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal ControlMngH264AVCDecoder::initMbForParsing( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  UInt uiMbY, uiMbX;

  uiMbY = uiMbIndex         / m_auiMbXinFrame[m_uiCurrLayer];
  uiMbX = uiMbIndex - uiMbY * m_auiMbXinFrame[m_uiCurrLayer];

  RNOK( m_pcMbDataCtrl                          ->initMb( rpcMbDataAccess, uiMbY, uiMbX ) );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX, m_bMbAff ) ); // for future use

  return Err::m_nOK;
}

ErrVal ControlMngH264AVCDecoder::initMbForDecoding( MbDataAccess*& rpcMbDataAccess,UInt uiMbY, UInt uiMbX, Bool bMbAFF  )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  RNOK( m_pcMbDataCtrl                          ->initMb( rpcMbDataAccess, uiMbY, uiMbX ) );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX, bMbAFF ) ); // for future use
  RNOK( m_pcMotionCompensation                  ->initMb(                  uiMbY, uiMbX, *rpcMbDataAccess ) ) ;
  return Err::m_nOK;
}

ErrVal ControlMngH264AVCDecoder::initMbForFiltering( MbDataAccess*& rpcMbDataAccess,UInt uiMbY, UInt uiMbX, Bool bMbAFF  )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  RNOK( m_pcMbDataCtrl                          ->initMb( rpcMbDataAccess, uiMbY, uiMbX           ) );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX, bMbAFF ) ); // for future use
  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initMbForFiltering( UInt uiMbY, UInt uiMbX, Bool bMbAFF  )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX, bMbAFF ) ); // for future use
  return Err::m_nOK;
}


//ErrVal ControlMngH264AVCDecoder::initSlice0( SliceHeader *rcSH )
ErrVal ControlMngH264AVCDecoder::initSlice0( SliceHeader *rcSH, UInt NumOfViewsInTheStream )
{
  UInt  uiLayer             = rcSH->getLayerId                    ();
  ROTRS( ( rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE || rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR) && m_uiInitilized[uiLayer], Err::m_nOK );
  ROTRS( ( rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_SCALABLE || rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE) && m_uiInitilized_MultiView[uiLayer], Err::m_nOK );
  m_auiMbXinFrame[uiLayer]  = rcSH->getSPS().getFrameWidthInMbs   ();
  m_auiMbYinFrame[uiLayer]  = rcSH->getSPS().getFrameHeightInMbs  ();

  UInt uiSizeX = m_auiMbXinFrame  [uiLayer] << 4;
  UInt uiSizeY = m_auiMbYinFrame  [uiLayer] << 4;

  RNOK( m_apcYuvFullPelBufferCtrl [uiLayer]->initSlice( uiSizeY, uiSizeX, YUV_Y_MARGIN, YUV_X_MARGIN ) );

  if( uiLayer == 0 )
  {
    m_bLayer0IsAVC  = ( rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE || 
                        rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR);
    
    //RNOK( m_pcFrameMng->initSlice               ( rcSH ) );
    RNOK( m_pcFrameMng->initSlice               ( rcSH, NumOfViewsInTheStream ) );
    m_pcH264AVCDecoder->setReconstructionLayerId( uiLayer );
    m_pcH264AVCDecoder->setBaseAVCCompatible    ( m_bLayer0IsAVC );
  }
  else
  {
    m_pcH264AVCDecoder->setReconstructionLayerId( uiLayer );
  }

  if( (rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_SCALABLE ||  
      rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE) &&
			rcSH->getTrueSlice())//TMM_EC
  {
  }

  RNOK( xInitESS( rcSH ) );

  if ( rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE || rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR)
	m_uiInitilized[uiLayer] = true;
  if ( rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_SCALABLE || rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
	m_uiInitilized_MultiView[uiLayer] = true;



  return Err::m_nOK;
}


// TMM_ESS {
ErrVal ControlMngH264AVCDecoder::initSPS( SequenceParameterSet& rcSequenceParameterSet, UInt  uiLayer )
{
  m_auiMbXinFrame[uiLayer]  = rcSequenceParameterSet.getFrameWidthInMbs   ();
  m_auiMbYinFrame[uiLayer]  = rcSequenceParameterSet.getFrameHeightInMbs  ();

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::xInitESS( SliceHeader* pcSliceHeader )
{
  UInt uiLayer = pcSliceHeader->getLayerId();
  UInt uiBaseLayer = pcSliceHeader->getBaseLayerId();
//JVT-T054{
  if( pcSliceHeader->getQualityLevel() == 0)
  {
    pcSliceHeader->getSPS().getResizeParameters(&m_ResizeParameter[uiLayer]);
  }
  else
  {
    pcSliceHeader->getSPS().getResizeParameters(&m_ResizeParameterCGSSNR[uiLayer][pcSliceHeader->getQualityLevel()]);
  }
//JVT-T054}
  if (uiBaseLayer != MSYS_UINT_MAX )
  {
//JVT-T054{
    ResizeParameters * curr;
    if(pcSliceHeader->getQualityLevel() == 0)
    {
      curr = &m_ResizeParameter[uiLayer];
    }
    else
    {
      curr = &m_ResizeParameterCGSSNR[uiLayer][pcSliceHeader->getQualityLevel()];
    }
//JVT-T054}
    curr->m_iInWidth  = m_auiMbXinFrame  [uiBaseLayer] << 4;
    curr->m_iInHeight = m_auiMbYinFrame  [uiBaseLayer] << 4;

    bool is_crop_aligned = (curr->m_iPosX%16 == 0) && (curr->m_iPosY%16 == 0);
    if      ((curr->m_iInWidth == curr->m_iOutWidth) && (curr->m_iInHeight == curr->m_iOutHeight) &&
             is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
      curr->m_iSpatialScalabilityType = SST_RATIO_1;
    else if ((curr->m_iInWidth*2 == curr->m_iOutWidth) && (curr->m_iInHeight*2 == curr->m_iOutHeight) &&
             is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
      curr->m_iSpatialScalabilityType = SST_RATIO_2;
    else 
      curr->m_iSpatialScalabilityType = SST_RATIO_X;

    if ( curr->m_iExtendedSpatialScalability == ESS_NONE && curr->m_iSpatialScalabilityType > SST_RATIO_2 )
    {
      printf("\nControlMngH264AVCDecoder::initEES() - use of Extended Spatial Scalability not signaled\n");
      return Err::m_nERR;
    }
    //end 
    /*
//JVT-T054{
    if(pcSliceHeader->getQualityLevel() == 0)
    {
      m_apcMCTFDecoder[uiLayer]->setResizeParameters(&m_ResizeParameter[uiLayer]);
    }
    else
    {
      m_apcMCTFDecoder[uiLayer]->setResizeParametersCGSSNR(pcSliceHeader->getQualityLevel(), &m_ResizeParameterCGSSNR[uiLayer][pcSliceHeader->getQualityLevel()]);
    }
//JVT-T054}
*/
    if (curr->m_iExtendedSpatialScalability == ESS_SEQ)
    {
      printf("Extended Spatial Scalability - crop win: origin=(%3d,%3d) - size=(%3d,%3d)\n\n",
             curr->m_iPosX,curr->m_iPosY,curr->m_iOutWidth,curr->m_iOutHeight);
    }
    else if (curr->m_iExtendedSpatialScalability == ESS_PICT)
    {
      printf("Extended Spatial Scalability - crop win by picture\n\n");
    }
    
  }
	return Err::m_nOK;
}
// TMM_ESS }



ErrVal ControlMngH264AVCDecoder::initSlice( SliceHeader& rcSH, ProcessingState eProcessingState )
{
  m_uiCurrLayer   = rcSH.getLayerId();
  m_pcMbDataCtrl  = rcSH.getFrameUnit()->getMbDataCtrl();

  RNOK( m_pcMbDataCtrl->initSlice( rcSH, eProcessingState, true, NULL ) );
  RNOK( m_pcSampleWeighting->initSlice( rcSH ) );

  if( PARSE_PROCESS == eProcessingState && rcSH.getTrueSlice())//TMM_EC
  {
    MbSymbolReadIf* pcMbSymbolReadIf;
    
    if( rcSH.getPPS().getEntropyCodingModeFlag() )
    {
      pcMbSymbolReadIf = m_pcCabacReader;
    }
    else
    {
      pcMbSymbolReadIf = m_pcUvlcReader;
    }

    RNOK( pcMbSymbolReadIf->startSlice( rcSH ) );
    RNOK( m_pcMbParser->initSlice( pcMbSymbolReadIf ) );

  }

  if( DECODE_PROCESS == eProcessingState)
  {
    RNOK( m_pcMotionCompensation->initSlice( rcSH ) );
  }
  m_bMbAff = rcSH.isMbAff(); // for future use
  m_pcSliceHeader = &rcSH;
  return Err::m_nOK;
}



ErrVal ControlMngH264AVCDecoder::initSliceForReading( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getLayerId();

  MbSymbolReadIf* pcMbSymbolReadIf;
  
  if( rcSH.getPPS().getEntropyCodingModeFlag() )
  {
    pcMbSymbolReadIf = m_pcCabacReader;
  }
  else
  {
    pcMbSymbolReadIf = m_pcUvlcReader;
  }

	if ( rcSH.getTrueSlice())
	{
		RNOK( pcMbSymbolReadIf->startSlice( rcSH ) );
	}
  RNOK( m_pcMbParser->initSlice( pcMbSymbolReadIf ) );
  m_pcSliceHeader = &rcSH;

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initSliceForDecoding( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getLayerId();

  RNOK( m_pcMotionCompensation->initSlice( rcSH ) );
  RNOK( m_pcSampleWeighting->initSlice( rcSH ) );
  m_pcSliceHeader = &rcSH;


  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initSliceForFiltering( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getLayerId();

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::finishSlice( const SliceHeader& rcSH, Bool& rbPicDone, Bool& rbFrameDone )
{
  rbPicDone     = m_pcMbDataCtrl->isPicDone( rcSH );
  rbFrameDone   = m_pcMbDataCtrl->isFrameDone( rcSH );
  m_uiCurrLayer = MSYS_UINT_MAX;
  
  return Err::m_nOK;
}

H264AVC_NAMESPACE_END

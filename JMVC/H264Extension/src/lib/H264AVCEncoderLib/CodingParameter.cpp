#include <cstdio>
#include "H264AVCEncoderLib.h"
#include "H264AVCCommonLib.h"

#include "CodingParameter.h"
#include "SequenceStructure.h"

#include <math.h>

#define ROTREPORT(x,t) {if(x) {::printf("\n%s\n",t); assert(0); return Err::m_nInvalidParameter;} }

// h264 namepace begin
H264AVC_NAMESPACE_BEGIN



ErrVal MotionVectorSearchParams::check() const
{
  ROTREPORT( 4 < m_eSearchMode,   "No Such Search Mode 0==Block,1==Spiral,2==Log,3==Fast, 4==NewFast" )
  ROTREPORT( 3 < m_eFullPelDFunc, "No Such Search Func (Full Pel) 0==SAD,1==SSE,2==Hadamard,3==YUV-SAD" )
  ROTREPORT( 3 == m_eFullPelDFunc && (m_eSearchMode==2 || m_eSearchMode==3), "Log and Fast search not possible in comb. with distortion measure 3" )
  ROTREPORT( 2 < m_eSubPelDFunc,  "No Such Search Func (Sub Pel) 0==SAD,1==SSE,2==Hadamard" )
  ROTREPORT( 1 < m_uiDirectMode,  "Direct Mode Exceeds Supported Range 0==Temporal, 1==Spatial");

  return Err::m_nOK;
}


ErrVal LoopFilterParams::check() const
{
  ROTREPORT( 2 < getFilterIdc(),        "Loop Filter Idc exceeds supported range 0..2");  if( 69 != getAlphaOffset() )

  if( 69 != getAlphaOffset() )
  {
    ROTREPORT( 26 < getAlphaOffset(),       "Loop Filter Alpha Offset exceeds supported range -26..26");
    ROTREPORT( -26 > getAlphaOffset(),      "Loop Filter Alpha Offset exceeds supported range -26..26");
  }

  if( 69 != getBetaOffset() )
  {
    ROTREPORT( 26 < getBetaOffset(),        "Loop Filter Beta Offset exceeds supported range -26..26");
    ROTREPORT( -26 > getBetaOffset(),       "Loop Filter Beta Offset exceeds supported range -26..26");
  }

  return Err::m_nOK;
}



ErrVal LayerParameters::check()
{
  ROTREPORT( getFrameWidth              () % 16,        "Frame Width must be a multiple of 16" );
  ROTREPORT( getFrameHeight             () % 16,        "Frame Height must be a multiple of 16" );
  ROTREPORT( getInputFrameRate          () < 
             getOutputFrameRate         (),             "Output frame rate must be less than or equal to input frame rate" );
  ROTREPORT( getAdaptiveTransform       () > 2,         "FRExt mode not supported" );
  ROTREPORT( getMaxAbsDeltaQP           () > 7,         "MaxAbsDeltaQP not supported" );
  ROTREPORT( getNumFGSLayers            () > 3,         "Number of FGS layers not supported" );
  ROTREPORT( getInterLayerPredictionMode() > 2,         "Unsupported inter-layer prediction mode" );
  ROTREPORT( getMotionInfoMode          () > 2,         "Motion info mode not supported" );
  ROTREPORT( getClosedLoop              () > 2,         "Unsupported closed-loop mode" );

  ROTREPORT( getBaseLayerId() != MSYS_UINT_MAX && getBaseLayerId() >= getLayerId(), "BaseLayerId is not possible" );

  if( m_dQpModeDecisionLP == -1.0 )
  {
    m_dQpModeDecisionLP = m_dBaseQpResidual;
  }

  // JVT-S054 (ADD) ->
  if ( m_uiNumSliceGroupsMinus1 > 0 && m_uiSliceGroupMapType == 0 && m_uiSliceMode == 4)
  {
    UInt i, j, uiSliceId, uiTotalRun, uiNumSlicePerHeight, uiMBAddr, uiMBCount, uiFrameWidthInMbs, uiFrameHeightInMbs, uiFrameSizeInMbs;

    m_bSliceDivisionFlag = true;
    m_uiSliceDivisionType = 0;  // rectangular grid slice of constant size
    uiTotalRun = m_uiRunLengthMinus1[0] + 1;
    for (i=1; i<m_uiNumSliceGroupsMinus1; i++)
    {
      uiTotalRun += m_uiRunLengthMinus1[i] + 1;
      if ( m_uiRunLengthMinus1[i] != m_uiRunLengthMinus1[0] )
        m_uiSliceDivisionType = 1;
    }
    uiTotalRun += m_uiRunLengthMinus1[m_uiNumSliceGroupsMinus1] + 1;
    if ( m_uiRunLengthMinus1[m_uiNumSliceGroupsMinus1] > m_uiRunLengthMinus1[0] )
    {
      m_uiSliceDivisionType = 1;
    }

    if ( (uiTotalRun<<4) != m_uiFrameWidth )
    {
      printf("Unsupported IROI mode\n");
      m_bSliceDivisionFlag = false;
      return Err::m_nOK;
    }

    uiFrameWidthInMbs = m_uiFrameWidth>>4;
    uiFrameHeightInMbs = m_uiFrameHeight>>4;
    uiFrameSizeInMbs = uiFrameWidthInMbs*uiFrameHeightInMbs;
    uiNumSlicePerHeight = (uiFrameHeightInMbs+m_uiSliceArgument-1)/m_uiSliceArgument;
    m_uiNumSliceMinus1 = ((m_uiNumSliceGroupsMinus1+1) * uiNumSlicePerHeight) - 1;

    // Allocate memory for slice division info
    if (m_puiGridSliceWidthInMbsMinus1 != NULL)
      free(m_puiGridSliceWidthInMbsMinus1);
    m_puiGridSliceWidthInMbsMinus1 = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));

    if (m_puiGridSliceHeightInMbsMinus1 != NULL)
      free(m_puiGridSliceHeightInMbsMinus1);
    m_puiGridSliceHeightInMbsMinus1 = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));

    if (m_puiFirstMbInSlice != NULL)
      free(m_puiFirstMbInSlice);
    m_puiFirstMbInSlice = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));

    if (m_puiLastMbInSlice != NULL)
      free(m_puiLastMbInSlice);
    m_puiLastMbInSlice = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));

    if (m_puiSliceId != NULL)
      free(m_puiSliceId);
    m_puiSliceId = (UInt*)malloc(uiFrameSizeInMbs*sizeof(UInt));

    // Initialize slice division info
    uiSliceId=0;
    m_puiGridSliceWidthInMbsMinus1[uiSliceId] = m_uiRunLengthMinus1[0];
    m_puiGridSliceHeightInMbsMinus1[uiSliceId] = m_uiSliceArgument-1;
    m_puiFirstMbInSlice[uiSliceId] = 0;
    m_puiLastMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId] + m_puiGridSliceHeightInMbsMinus1[uiSliceId]*(m_uiFrameWidth>>4) + m_puiGridSliceWidthInMbsMinus1[uiSliceId];
    uiSliceId += uiNumSlicePerHeight;
    for (i=1; i<=m_uiNumSliceGroupsMinus1; i++)
    {
      m_puiGridSliceWidthInMbsMinus1[uiSliceId] = m_uiRunLengthMinus1[i];
      m_puiGridSliceHeightInMbsMinus1[uiSliceId] = m_uiSliceArgument-1;
      m_puiFirstMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId-uiNumSlicePerHeight] + m_uiRunLengthMinus1[i-1] + 1;
      m_puiLastMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId] + m_puiGridSliceHeightInMbsMinus1[uiSliceId]*(m_uiFrameWidth>>4) + m_puiGridSliceWidthInMbsMinus1[uiSliceId];
      uiSliceId += uiNumSlicePerHeight;
    }
    uiSliceId = 0;
    for (i=0; i<=m_uiNumSliceGroupsMinus1; i++)
    {
      uiSliceId++;
      for (j=1; j<uiNumSlicePerHeight; j++, uiSliceId++)
      {
        m_puiGridSliceWidthInMbsMinus1[uiSliceId] = m_puiGridSliceWidthInMbsMinus1[uiSliceId-1];
        m_puiGridSliceHeightInMbsMinus1[uiSliceId] = m_uiSliceArgument-1;
        if ( j == (uiNumSlicePerHeight-1) )
          m_puiGridSliceHeightInMbsMinus1[uiSliceId] = (m_uiFrameHeight>>4) - (j*m_uiSliceArgument) - 1;

        m_puiFirstMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId-1] + m_uiSliceArgument*(m_uiFrameWidth>>4);
        m_puiLastMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId] + m_puiGridSliceHeightInMbsMinus1[uiSliceId]*(m_uiFrameWidth>>4) + m_puiGridSliceWidthInMbsMinus1[uiSliceId];
      }
    }
    // Debug
    if (uiSliceId != (m_uiNumSliceMinus1+1))
    {
      printf("IROI error\n");
      m_bSliceDivisionFlag = false;
      return Err::m_nOK;
    }

    uiMBCount = 0;
    for (uiSliceId = 0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
    {
      uiMBAddr = m_puiFirstMbInSlice[uiSliceId];
      for (i = 0; i <= m_puiGridSliceHeightInMbsMinus1[uiSliceId]; i++, uiMBAddr+=uiFrameWidthInMbs)
      {
        for (j = 0; j <= m_puiGridSliceWidthInMbsMinus1[uiSliceId]; j++, uiMBCount++)
        {
          m_puiSliceId[uiMBAddr+j] = uiSliceId;
        }
      }
    }
    // Debug
    if (uiMBCount != uiFrameSizeInMbs)
    {
      printf("IROI error\n");
      m_bSliceDivisionFlag = false;
      return Err::m_nOK;
    }

    // Display slice division info.
    printf("IROI: Slice Division Type %d, Num Slice %d\n", m_uiSliceDivisionType, m_uiNumSliceMinus1+1);
    //for (i=0; i<=m_uiNumSliceMinus1; i++)
    //{
    //  printf("(%d, %d, %d, %d, %d)\n", i, m_puiGridSliceWidthInMbsMinus1[i], m_puiGridSliceHeightInMbsMinus1[i], m_puiFirstMbInSlice[i], m_puiLastMbInSlice[i]);
    //}
  }
  // JVT-S054 (ADD) <-

  //S051{
  ROTREPORT( getAnaSIP	()>0 && getEncSIP(),			"Unsupported SIP mode\n"); 
  //S051}

  return Err::m_nOK;
}


UInt CodingParameter::getLogFactor( Double r0, Double r1 )
{
  Double dLog2Factor  = log10( r1 / r0 ) / log10( 2.0 );
  Double dRound       = floor( dLog2Factor + 0.5 );
  Double dEpsilon     = 0.0001;

  if( dLog2Factor-dEpsilon < dRound && dRound < dLog2Factor+dEpsilon )
  {
    return (UInt)(dRound);
  }
  return MSYS_UINT_MAX;
}


//JVT-W080
ErrVal CodingParameter::savePDSParameters( UInt   uiNumView
												                 , UInt*  num_refs_list0_anc
												                 , UInt*  num_refs_list1_anc
												                 , UInt*  num_refs_list0_nonanc
												                 , UInt*  num_refs_list1_nonanc
												                 , UInt** PDIInitialDelayMinus2L0Anc
												                 , UInt** PDIInitialDelayMinus2L1Anc
												                 , UInt** PDIInitialDelayMinus2L0NonAnc
												                 , UInt** PDIInitialDelayMinus2L1NonAnc
												                 )
{
  for( UInt i = 0; i < uiNumView; i++ )
	{
		UInt j;
		for( j = 0; j < num_refs_list0_anc[i]; j++ )
		{
		  m_ppuiPdsInitialDelayMinus2L0Anc[i][j] = PDIInitialDelayMinus2L0Anc[i][j];
		}
		for( j = 0; j < num_refs_list1_anc[i]; j++ )
		{
		  m_ppuiPdsInitialDelayMinus2L1Anc[i][j] = PDIInitialDelayMinus2L0Anc[i][j];
		}
		for( j = 0; j < num_refs_list0_nonanc[i]; j++ )
		{
		  m_ppuiPdsInitialDelayMinus2L0NonAnc[i][j] = PDIInitialDelayMinus2L0NonAnc[i][j];
		}
		for( j = 0; j < num_refs_list1_nonanc[i]; j++ )
		{
		  m_ppuiPdsInitialDelayMinus2L1NonAnc[i][j] = PDIInitialDelayMinus2L0NonAnc[i][j];
		}
	}

	return Err::m_nOK;
}
//~JVT-W080

ErrVal CodingParameter::check()
{
  ROTS( m_cLoopFilterParams         .check() );
  ROTS( m_cMotionVectorSearchParams .check() );

  ROTREPORT( getMaximumFrameRate  () > (MAX_FRAMERATE + 0.00001),            "Frame rate NOT supported, bigger than MAX_FRAMERATE in CommonDefs.h" ); // Safe check. -Dong

  if( getMVCmode() )
  {

    //===== coder is operated in MVC mode =====
    //ROTREPORT( getFrameWidth        () <= 0 ||
    //           getFrameWidth        ()  % 16,             "Frame Width  must be greater than 0 and a multiple of 16" );
    //ROTREPORT( getFrameHeight       () <= 0 ||
    //           getFrameHeight       ()  % 16,             "Frame Height must be greater than 0 and a multiple of 16" );
    ROTREPORT( getMaximumFrameRate  () <= 0.0,            "Frame rate not supported" );
    ROTREPORT( getTotalFrames       () == 0,              "Total Number Of Frames must be greater than 0" );
    ROTREPORT( getSymbolMode        ()  > 1,              "Symbol mode not supported" );
    ROTREPORT( get8x8Mode           ()  > 2,              "FRExt mode not supported" );
  ROTREPORT( getMaximumFrameRate() <= 0.0,              "Maximum frame rate not supported" );
  ROTREPORT( getMaximumDelay    ()  < 0.0,              "Maximum delay must be greater than or equal to 0" );
  ROTREPORT( getTotalFrames     () == 0,                "Total Number Of Frames must be greater than 0" );

  ROTREPORT( getGOPSize         ()  < 1  ||
             getGOPSize         ()  > 64,               "GOP Size not supported" );
  UInt uiDecStages = getLogFactor( 1.0, getGOPSize() );
  ROTREPORT( uiDecStages == MSYS_UINT_MAX && getGOPSize()!= 12 && getGOPSize()!= 15,              "GOP Size must be a multiple of 2 or 12 or 15" );
// temporal scalability
  if(uiDecStages==MSYS_UINT_MAX) 
    uiDecStages= (UInt)( log10( m_uiGOPSize+.0) / log10( 2.0 ) +1 );
  setDecompositionStages( uiDecStages );

  ROTREPORT( getIntraPeriod     ()  <
             getGOPSize         (),                     "Intra period must be greater or equal to GOP size" );
  if( getIntraPeriod() == MSYS_UINT_MAX )
  {
    setIntraPeriodLowPass( MSYS_UINT_MAX );
  }
  else
  {
    UInt uiIntraPeriod = getIntraPeriod() / getGOPSize() - 1;
    ROTREPORT( getIntraPeriod() % getGOPSize(),         "Intra period must be a power of 2 of GOP size (or -1)" );
    setIntraPeriodLowPass( uiIntraPeriod );
  }

  ROTREPORT( getNumRefFrames    ()  < 1  ||
             getNumRefFrames    ()  > 15,               "Number of reference frames not supported" );

    return Err::m_nOK;
  }

  ROTREPORT( getMaximumFrameRate() <= 0.0,              "Maximum frame rate not supported" );
  ROTREPORT( getMaximumDelay    ()  < 0.0,              "Maximum delay must be greater than or equal to 0" );
  ROTREPORT( getTotalFrames     () == 0,                "Total Number Of Frames must be greater than 0" );

  ROTREPORT( getGOPSize         ()  < 1  ||
             getGOPSize         ()  > 64,               "GOP Size not supported" );
  UInt uiDecStages = getLogFactor( 1.0, getGOPSize() );
  ROTREPORT( uiDecStages == MSYS_UINT_MAX,              "GOP Size must be a multiple of 2" );
  setDecompositionStages( uiDecStages );

  ROTREPORT( getIntraPeriod     ()  <
             getGOPSize         (),                     "Intra period must be greater or equal to GOP size" );
  if( getIntraPeriod() == MSYS_UINT_MAX )
  {
    setIntraPeriodLowPass( MSYS_UINT_MAX );
  }
  else
  {
    UInt uiIntraPeriod = getIntraPeriod() / getGOPSize() - 1;
    ROTREPORT( getIntraPeriod() % getGOPSize(),         "Intra period must be a power of 2 of GOP size (or -1)" );
    setIntraPeriodLowPass( uiIntraPeriod );
  }

  ROTREPORT( getNumRefFrames    ()  < 1  ||
             getNumRefFrames    ()  > 15,               "Number of reference frames not supported" );
  ROTREPORT( getBaseLayerMode   ()  > 2,                "Base layer mode not supported" );
  ROTREPORT( getNumberOfLayers  ()  > MAX_LAYERS,       "Number of layers not supported" );



  Double  dMaxFrameDelay  = max( 0, m_dMaximumFrameRate * m_dMaximumDelay / 1000.0 );
  UInt    uiMaxFrameDelay = (UInt)floor( dMaxFrameDelay );

  for( UInt uiLayer = 0; uiLayer < getNumberOfLayers(); uiLayer++ )
  {
    LayerParameters*  pcLayer               = &m_acLayerParameters[uiLayer];

	  RNOK( pcLayer->check() );

    UInt              uiBaseLayerId         = uiLayer && pcLayer->getBaseLayerId() != MSYS_UINT_MAX ? pcLayer->getBaseLayerId() : MSYS_UINT_MAX;
    LayerParameters*  pcBaseLayer           = uiBaseLayerId != MSYS_UINT_MAX ? &m_acLayerParameters[uiBaseLayerId] : 0;
    UInt              uiLogFactorInOutRate  = getLogFactor( pcLayer->getOutputFrameRate (), pcLayer->getInputFrameRate() );
    UInt              uiLogFactorMaxInRate  = getLogFactor( pcLayer->getInputFrameRate  (), getMaximumFrameRate       () );

    // heiko.schwarz@hhi.fhg.de: add some additional check for input/output frame rates
    ROTREPORT( pcLayer->getInputFrameRate() < pcLayer->getOutputFrameRate(),  "Input frame rate must not be less than output frame rate" );
    ROTREPORT( pcLayer->getInputFrameRate() > getMaximumFrameRate(),          "Input frame rate must not be greater than maximum frame rate" );
    ROTREPORT( getDecompositionStages() < uiLogFactorMaxInRate + uiLogFactorInOutRate, "Number of decomposition stages is too small for the specified output rate" );

    ROTREPORT( uiLogFactorInOutRate == MSYS_UINT_MAX,   "Input frame rate must be a power of 2 of output frame rate" );
    ROTREPORT( uiLogFactorMaxInRate == MSYS_UINT_MAX,   "Maximum frame rate must be a power of 2 of input frame rate" );

    pcLayer->setNotCodedMCTFStages  ( uiLogFactorInOutRate );
    pcLayer->setTemporalResolution  ( uiLogFactorMaxInRate );
    pcLayer->setDecompositionStages ( getDecompositionStages() - uiLogFactorMaxInRate );
    pcLayer->setFrameDelay          ( uiMaxFrameDelay  /  ( 1 << uiLogFactorMaxInRate ) );

    if( pcBaseLayer ) // for sub-sequence SEI
    {
      ROTREPORT( pcLayer->getInputFrameRate() < pcBaseLayer->getInputFrameRate(), "Input frame rate less than base layer output frame rate" );
      UInt uiLogFactorRate = getLogFactor( pcBaseLayer->getInputFrameRate(), pcLayer->getInputFrameRate() );
      ROTREPORT( uiLogFactorRate == MSYS_UINT_MAX, "Input Frame rate must be a power of 2 from layer to layer" );
      pcLayer->setBaseLayerTempRes( uiLogFactorRate );


      ROTREPORT( pcLayer->getFrameWidth ()  < pcBaseLayer->getFrameWidth (), "Frame width  less than base layer frame width" );
      ROTREPORT( pcLayer->getFrameHeight()  < pcBaseLayer->getFrameHeight(), "Frame height less than base layer frame height" );
      UInt uiLogFactorWidth  = getLogFactor( pcBaseLayer->getFrameWidth (), pcLayer->getFrameWidth () );
     
      pcLayer->setBaseLayerSpatRes( uiLogFactorWidth );
			
// TMM_ESS {
      ResizeParameters * resize = pcLayer->getResizeParameters();
      if (resize->m_iExtendedSpatialScalability != ESS_NONE)
        {
          ROTREPORT(resize->m_iInWidth  % 16,   "Base layer width must be a multiple of 16" );
          ROTREPORT(resize->m_iInHeight % 16,   "Base layer height must be a multiple of 16" );
          if (resize->m_bCrop)
            {
              ROTREPORT( resize->m_iPosX % 2 , "Cropping Window must be even aligned" );
              ROTREPORT( resize->m_iPosY % 2 , "Cropping Window must be even aligned" );  
              ROTREPORT(resize->m_iGlobWidth  % 16, "Enhancement layer width must be a multiple of 16" );
              ROTREPORT(resize->m_iGlobHeight % 16, "Enhancement layer height must be a multiple of 16" );
            }
          else
            {
              resize->m_iGlobWidth = resize->m_iOutWidth;
              resize->m_iGlobHeight = resize->m_iOutHeight;
            }
          printf("\n\n*************************\n%dx%d  -> %dx%d %s -> %dx%d\n",
                 resize->m_iInWidth, resize->m_iInHeight,
                 resize->m_iOutWidth, resize->m_iOutHeight,
                 (resize->m_bCrop ? "Crop" : "No_Crop"),
                 resize->m_iGlobWidth, resize->m_iGlobHeight);
          printf("ExtendedSpatialScalability: %d    SpatialScalabilityType: %d\n",
            resize->m_iExtendedSpatialScalability,
                 resize->m_iSpatialScalabilityType);
        }
      else
        {
          printf("\n\n*************************\n No_Crop\n");
        }
// TMM_ESS }

      pcBaseLayer->setContrainedIntraForLP();
    }

    if( pcLayer->getBaseQualityLevel() > 3 )
      pcLayer->setBaseQualityLevel(3);

    if( uiLayer == 0 && pcLayer->getBaseQualityLevel() != 0 )
      pcLayer->setBaseQualityLevel(0);

    // pass parameters from command line to layer configurations
     if( m_dLowPassEnhRef >= 0 )
    {
      pcLayer->setLowPassEnhRef( m_dLowPassEnhRef );
    }
    if( m_uiBaseWeightZeroBaseBlock <= AR_FGS_MAX_BASE_WEIGHT || m_uiBaseWeightZeroBaseCoeff <= AR_FGS_MAX_BASE_WEIGHT )
    {
      pcLayer->setAdaptiveRefFGSWeights( m_uiBaseWeightZeroBaseBlock, m_uiBaseWeightZeroBaseCoeff );
    }
    if( m_uiFgsEncStructureFlag < MSYS_UINT_MAX )
    {
      pcLayer->setFgsEncStructureFlag( m_uiFgsEncStructureFlag );
    }
  }

 return Err::m_nOK;
}



H264AVC_NAMESPACE_END

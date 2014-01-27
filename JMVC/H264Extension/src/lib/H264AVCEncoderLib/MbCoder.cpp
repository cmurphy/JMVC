#include "H264AVCEncoderLib.h"
#include "MbCoder.h"
#include "H264AVCCommonLib/Tables.h"



H264AVC_NAMESPACE_BEGIN


MbCoder::MbCoder():
  m_pcMbSymbolWriteIf( NULL ),
  m_pcRateDistortionIf( NULL ),
  m_bInitDone( false ),
  m_bCabac( false ),
  m_bPrevIsSkipped( false )
{
}


MbCoder::~MbCoder()
{
}


ErrVal MbCoder::create( MbCoder*& rpcMbCoder )
{
  rpcMbCoder = new MbCoder;

  ROT( NULL == rpcMbCoder );

  return Err::m_nOK;
}

ErrVal MbCoder::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal MbCoder::initSlice(  const SliceHeader& rcSH,
                            MbSymbolWriteIf*   pcMbSymbolWriteIf,
                            RateDistortionIf*  pcRateDistortionIf )
{
  ROT( NULL == pcMbSymbolWriteIf );
  ROT( NULL == pcRateDistortionIf );

  m_pcMbSymbolWriteIf = pcMbSymbolWriteIf;
  m_pcRateDistortionIf = pcRateDistortionIf;

  m_bCabac          = rcSH.getPPS().getEntropyCodingModeFlag();
  m_bPrevIsSkipped  = false;

  m_bInitDone = true;

  return Err::m_nOK;
}


ErrVal MbCoder::uninit()
{
  m_pcMbSymbolWriteIf = NULL;
  m_pcRateDistortionIf = NULL;

  m_bInitDone = false;
  return Err::m_nOK;
}






ErrVal MbCoder::encode( MbDataAccess& rcMbDataAccess,
                        MbDataAccess* pcMbDataAccessBase,
                        Int           iSpatialScalabilityType,
                        Bool          bTerminateSlice, 
						Bool          bSendTerminateSlice)
{
  ROF( m_bInitDone );

  //===== skip flag =====
  Bool  bIsCoded  = ! rcMbDataAccess.isSkippedMb();

  RNOK( m_pcMbSymbolWriteIf->skipFlag( rcMbDataAccess, false ) );

  if( bIsCoded )
  {

		Bool bFieldFlagCoded = true;
    if( bFieldFlagCoded && rcMbDataAccess.getSH().isMbAff() && ( rcMbDataAccess.isTopMb() || m_bPrevIsSkipped ) )
    {
      RNOK( m_pcMbSymbolWriteIf->fieldFlag( rcMbDataAccess) );
    }

    //===== base layer mode flag and base layer refinement flag =====
    if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
    {
      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() == true )// TMM_ESS
      {
				if( rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
				{
					RNOK  ( m_pcMbSymbolWriteIf->BLSkipFlag( rcMbDataAccess ) );
				}
				else
				{
					ROF( rcMbDataAccess.getMbData().getBLSkipFlag () );
				}
// TMM_ESS {
      }
      else  // of if ( rcMbDataAccess.getMbData().getInCropWindowFlag() == true )
      {
          ROT  ( rcMbDataAccess.getMbData().getBLSkipFlag () );
      }
// TMM_ESS }
    }
    else
    {
      ROT  ( rcMbDataAccess.getMbData().getBLSkipFlag () );
    }



    //===== macroblock mode =====
    if( ! rcMbDataAccess.getMbData().getBLSkipFlag() )
    {
      MbMode  eMbModeOrg = rcMbDataAccess.getMbData().getMbMode();
      MbMode  eMbModeSet = ( eMbModeOrg == INTRA_BL ? INTRA_4X4 : eMbModeOrg );
      rcMbDataAccess.getMbData().setMbMode( eMbModeSet );
      RNOK( m_pcMbSymbolWriteIf->mbMode( rcMbDataAccess ) );
      rcMbDataAccess.getMbData().setMbMode( eMbModeOrg );
    }

    //--- reset motion pred flags ---
    if( rcMbDataAccess.getMbData().getBLSkipFlag() 
		|| rcMbDataAccess.getMbData().isIntra() || rcMbDataAccess.getMbData().getMbMode() == MODE_SKIP )
    {
      rcMbDataAccess.getMbMotionData( LIST_0 ).setMotPredFlag( false );
      rcMbDataAccess.getMbMotionData( LIST_1 ).setMotPredFlag( false );
    }
    else if( rcMbDataAccess.getSH().isInterB() )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP == rcMbDataAccess.getMbData().getBlkMode   ( c8x8Idx.b8x8Index() ) ||
            !rcMbDataAccess            .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), LIST_0 ) )
        {
          rcMbDataAccess.getMbMotionData( LIST_0 ).setMotPredFlag( false, c8x8Idx );
        }
        if( BLK_SKIP == rcMbDataAccess.getMbData().getBlkMode   ( c8x8Idx.b8x8Index() ) ||
            !rcMbDataAccess            .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), LIST_1 ) )
        {
          rcMbDataAccess.getMbMotionData( LIST_1 ).setMotPredFlag( false, c8x8Idx );
        }
      }
    }

    //===== prediction info =====
    if( ! rcMbDataAccess.getMbData().getBLSkipFlag() )
    {
      //===== BLOCK MODES =====
      if( rcMbDataAccess.getMbData().isInter8x8() )
      {
        RNOK( m_pcMbSymbolWriteIf->blockModes( rcMbDataAccess ) );
      }

      if( rcMbDataAccess.getMbData().isPCM() )
      {
        //===== PCM SAMPLES =====
        RNOK( m_pcMbSymbolWriteIf->samplesPCM( rcMbDataAccess ) );
      }
      else if( rcMbDataAccess.getMbData().isIntra() )
      {
        //===== INTRA PREDICTION MODES =====
        RNOK( xWriteIntraPredModes( rcMbDataAccess ) );
      }
      else
      {
        //===== MOTION INFORMATION =====
        MbMode eMbMode = rcMbDataAccess.getMbData().getMbMode();
        if( rcMbDataAccess.getSH().isInterB() )
        {
          RNOK( xWriteMotionPredFlags( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteMotionPredFlags( rcMbDataAccess, eMbMode, LIST_1 ) );
          RNOK( xWriteReferenceFrames( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteReferenceFrames( rcMbDataAccess, eMbMode, LIST_1 ) );
          RNOK( xWriteMotionVectors  ( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteMotionVectors  ( rcMbDataAccess, eMbMode, LIST_1 ) );
        }
        else
        {
          RNOK( xWriteMotionPredFlags( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteReferenceFrames( rcMbDataAccess, eMbMode, LIST_0 ) );
          RNOK( xWriteMotionVectors  ( rcMbDataAccess, eMbMode, LIST_0 ) );
        }
      }
    }


    //===== TEXTURE =====
    if( ! rcMbDataAccess.getMbData().isPCM() )
    {
      Bool bTrafo8x8Flag = ( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() &&
                           ( rcMbDataAccess.getMbData().getBLSkipFlag() ||
            						   ( rcMbDataAccess.getMbData().is8x8TrafoFlagPresent() &&
                              !rcMbDataAccess.getMbData().isIntra4x4() ) ) );
 			//-- JVT-R091
			RNOK( xWriteTextureInfo( rcMbDataAccess, pcMbDataAccessBase, rcMbDataAccess.getMbTCoeffs(), bTrafo8x8Flag ) );
			//--
    }
  }

  m_bPrevIsSkipped = !bIsCoded;

  ROTRS( ! bSendTerminateSlice, Err::m_nOK );

  //--- write terminating bit ---
  RNOK( m_pcMbSymbolWriteIf->terminatingBit ( bTerminateSlice ? 1:0 ) );

  if( bTerminateSlice )
  {
    RNOK( m_pcMbSymbolWriteIf->finishSlice() );
  }

  return Err::m_nOK;
}


ErrVal MbCoder::encodeMotion( MbDataAccess& rcMbDataAccess,
                              MbDataAccess* pcMbDataAccessBase )
{
  ROT( rcMbDataAccess.getMbData().isIntra() );
  //===== base mode flag =====
  RNOK( m_pcMbSymbolWriteIf->BLSkipFlag( rcMbDataAccess ) );
  ROTRS( rcMbDataAccess.getMbData().getBLSkipFlag(), Err::m_nOK );

  //===== macroblock mode =====
  RNOK( m_pcMbSymbolWriteIf->mbMode( rcMbDataAccess ) );
  //===== BLOCK MODES =====
  if( rcMbDataAccess.getMbData().isInter8x8() )
  {
    RNOK( m_pcMbSymbolWriteIf->blockModes( rcMbDataAccess ) );
  }
  //===== MOTION INFORMATION =====
  MbMode eMbMode = rcMbDataAccess.getMbData().getMbMode();
  if( rcMbDataAccess.getSH().isInterB() )
  {
    RNOK( xWriteMotionPredFlags_FGS ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );
    RNOK( xWriteMotionPredFlags_FGS ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_1 ) );
    RNOK( xWriteReferenceFrames     ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
    RNOK( xWriteReferenceFrames     ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
    RNOK( xWriteMotionVectors       ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
    RNOK( xWriteMotionVectors       ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
  }
  else
  {
    RNOK( xWriteMotionPredFlags_FGS ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );
    RNOK( xWriteReferenceFrames     ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
    RNOK( xWriteMotionVectors       ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
  }
  //===== residual prediction flag =====
  Bool bBaseCoeff = ( pcMbDataAccessBase->getMbData().getMbCbp() != 0 );
  RNOK( m_pcMbSymbolWriteIf->resPredFlag_FGS( rcMbDataAccess, bBaseCoeff ) );

  return Err::m_nOK;
}



ErrVal MbCoder::xWriteIntraPredModes( MbDataAccess& rcMbDataAccess )
{
  ROFRS( rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  if( rcMbDataAccess.getMbData().isIntra4x4() )
  {
    if( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() )
    {
      RNOK( m_pcMbSymbolWriteIf->transformSize8x8Flag( rcMbDataAccess ) );
    }

    if( rcMbDataAccess.getMbData().isTransformSize8x8() )
    {
      for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        RNOK( m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbDataAccess, cIdx ) );
      }
    }
    else
    {
      for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        RNOK( m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbDataAccess, cIdx ) );
      }
    }
  }

  if( rcMbDataAccess.getMbData().isIntra4x4() || rcMbDataAccess.getMbData().isIntra16x16() )
  {
    RNOK( m_pcMbSymbolWriteIf->intraPredModeChroma( rcMbDataAccess ) );
  }

  return Err::m_nOK;
}




ErrVal MbCoder::xWriteBlockMv( MbDataAccess& rcMbDataAccess, B8x8Idx c8x8Idx, ListIdx eLstIdx )
{
  BlkMode eBlkMode = rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() );

  ParIdx8x8 eParIdx = c8x8Idx.b8x8();
  switch( eBlkMode )
  {
    case BLK_8x8:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx ) );
      break;
    }
    case BLK_8x4:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_8x4_0 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_8x4_1 ) );
      break;
    }
    case BLK_4x8:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x8_0 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x8_1 ) );
      break;
    }
    case BLK_4x4:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_0 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_1 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_2 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_3 ) );
      break;
    }
    case BLK_SKIP:
    {
      break;
    }
    default:
    {
      AF();
      return Err::m_nERR;
    }
  }
  return Err::m_nOK;
}




ErrVal
MbCoder::xWriteMotionPredFlags_FGS( MbDataAccess&  rcMbDataAccess,
                                    MbDataAccess*  pcMbDataAccessBase, 
                                    MbMode         eMbMode,
                                    ListIdx        eLstIdx )
{
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  ROFRS  ( rcMbDataAccess.getSH().getAdaptivePredictionFlag (), Err::m_nOK );
  ROF    ( pcMbDataAccessBase );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      break;
    }

  case MODE_16x16:
    {
      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx ) );
      }
      break;
    }
    
  case MODE_16x8:
    {
      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      break;
    }
    
  case MODE_8x16:
    {
      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      break;
    }
    
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode   ( c8x8Idx.b8x8Index() ) &&
            rcMbDataAccess            .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx) &&
            pcMbDataAccessBase       ->getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx)   )
        {
          RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
        }
      }
      break;
    }
    
  default:
    {
      AF();
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}


ErrVal
MbCoder::xWriteMotionPredFlags( MbDataAccess&  rcMbDataAccess,
                                MbMode         eMbMode,
                                ListIdx        eLstIdx )
{ 
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  ROFRS  ( rcMbDataAccess.getSH().getAdaptivePredictionFlag (), Err::m_nOK );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      break;
    }

  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx ) );
      }
      break;
    }

  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      break;
    }

  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      break;
    }

  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode   ( c8x8Idx.b8x8Index() ) &&
           rcMbDataAccess             .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx) )
        {
          RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
        }
      }
      break;
    }

  default:
    {
      AF();
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}


ErrVal
MbCoder::xWriteReferenceFrames( MbDataAccess& rcMbDataAccess,
                                MbMode        eMbMode,
                                ListIdx       eLstIdx )
{
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
  {
    return Err::m_nOK;
  }

  Bool          bPred = rcMbDataAccess.getSH().getAdaptivePredictionFlag();
  MbMotionData& rcMot = rcMbDataAccess.getMbMotionData( eLstIdx );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      break;
    }
    
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag() ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx ) );
      }
      break;
    }
    
  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_16x8_0 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_16x8_1 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      break;
    }
    
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_8x16_0 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_8x16_1 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      break;
    }
    
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) &&
            rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) && ( ! bPred || ! rcMot.getMotPredFlag( c8x8Idx.b8x8() ) ) )
        {
          RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
        }
      }
      break;
    }
    
  default:
    {
      AF();
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}




ErrVal
MbCoder::xWriteMotionVectors( MbDataAccess& rcMbDataAccess,
                              MbMode        eMbMode,
                              ListIdx       eLstIdx )
{
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      return Err::m_nOK;
    }
  
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx ) );
      }
      return Err::m_nOK;
    }

  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      return Err::m_nOK;
    }
  
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      return Err::m_nOK;
    }
  
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) )
        {
          RNOK( xWriteBlockMv( rcMbDataAccess, c8x8Idx, eLstIdx ) );
        }
      }
      return Err::m_nOK;
    }
   
  default:
    {
      AF();
      return Err::m_nERR;
    }
  }

  return Err::m_nERR;
}


ErrVal MbCoder::xWriteTextureInfo( MbDataAccess&            rcMbDataAccess,
																	 MbDataAccess*						pcMbDataAccessBase,	// JVT-R091
                                   const MbTransformCoeffs& rcMbTCoeff,
                                   Bool											bTrafo8x8Flag
                                   )
{

  Bool bWriteDQp = true;
  UInt uiCbp = rcMbDataAccess.getMbData().getMbCbp();

  if( ! rcMbDataAccess.getMbData().isIntra16x16() )
  {
    RNOK( m_pcMbSymbolWriteIf->cbp( rcMbDataAccess ) );

    bWriteDQp = ( 0 != uiCbp );
  }


  if( bTrafo8x8Flag && ( rcMbDataAccess.getMbData().getMbCbp() & 0x0F ) )
  {
    ROT( rcMbDataAccess.getMbData().isIntra16x16() );
    ROT( rcMbDataAccess.getMbData().isIntra4x4  () );
    RNOK( m_pcMbSymbolWriteIf->transformSize8x8Flag( rcMbDataAccess ) );
  }

  if( bWriteDQp )
  {
    RNOK( m_pcMbSymbolWriteIf->deltaQp( rcMbDataAccess ) );
  }

  
  if( rcMbDataAccess.getMbData().getBLSkipFlag() ||
     !rcMbDataAccess.getMbData().isIntra() )
  {
    if( rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
    {
      if( ! rcMbDataAccess.getSH().isIntra() )
      {
        RNOK( m_pcMbSymbolWriteIf->resPredFlag( rcMbDataAccess ) );
        if ( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) && 
              rcMbDataAccess.getMbData().getBLSkipFlag() )
        {
          RNOK( m_pcMbSymbolWriteIf->smoothedRefFlag( rcMbDataAccess ) );
        }
      }
    }
  }

  if( rcMbDataAccess.getMbData().isIntra16x16() )
  {
    RNOK( xScanLumaIntra16x16( rcMbDataAccess, rcMbTCoeff, rcMbDataAccess.getMbData().isAcCoded() ) );
    RNOK( xScanChromaBlocks  ( rcMbDataAccess, rcMbTCoeff, rcMbDataAccess.getMbData().getCbpChroma16x16() ) );

    return Err::m_nOK;
  }



  if( rcMbDataAccess.getMbData().isTransformSize8x8() )
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      if( (uiCbp >> c8x8Idx.b8x8Index()) & 1 )
      {
        RNOK( m_pcMbSymbolWriteIf->residualBlock8x8( rcMbDataAccess, c8x8Idx, LUMA_SCAN ) );
      }
    }
  }
  else
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      if( (uiCbp >> c8x8Idx.b8x8Index()) & 1 )
      {
        for( S4x4Idx cIdx(c8x8Idx); cIdx.isLegal(c8x8Idx); cIdx++ )
        {
          RNOK( xScanLumaBlock( rcMbDataAccess, rcMbTCoeff, cIdx ) );
        }
      }
    }
  }


  RNOK( xScanChromaBlocks( rcMbDataAccess, rcMbTCoeff, rcMbDataAccess.getMbData().getCbpChroma4x4() ) );

  return Err::m_nOK;
}






ErrVal MbCoder::xScanLumaIntra16x16( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, Bool bAC )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, B4x4Idx(0), LUMA_I16_DC ) );
  ROFRS( bAC, Err::m_nOK );

  for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++)
  {
    RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, cIdx, LUMA_I16_AC ) );
  }

  return Err::m_nOK;
}


ErrVal MbCoder::xScanLumaBlock( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, LumaIdx cIdx )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, cIdx, LUMA_SCAN ) );
  return Err::m_nOK;
}


ErrVal MbCoder::xScanChromaDc( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(0), CHROMA_DC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(4), CHROMA_DC ) );

  return Err::m_nOK;
}


ErrVal MbCoder::xScanChromaAcU( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(0), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(1), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(2), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(3), CHROMA_AC ) );
  return Err::m_nOK;
}


ErrVal MbCoder::xScanChromaAcV( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(4), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(5), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(6), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(7), CHROMA_AC ) );
  return Err::m_nOK;
}


ErrVal MbCoder::xScanChromaBlocks( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, UInt uiChromCbp )
{
  ROTRS( 1 > uiChromCbp, Err::m_nOK );

  RNOK( xScanChromaDc ( rcMbDataAccess, rcTCoeff ) );

  ROTRS( 2 > uiChromCbp, Err::m_nOK );

  RNOK( xScanChromaAcU( rcMbDataAccess, rcTCoeff ) );
  RNOK( xScanChromaAcV( rcMbDataAccess, rcTCoeff ) );
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END

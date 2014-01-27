#include "H264AVCDecoderLib.h"
#include "H264AVCCommonLib/Tables.h"
#include "H264AVCCommonLib/ContextTables.h"
#include "H264AVCCommonLib/CabacContextModel2DBuffer.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/Transform.h"
#include "CabaDecoder.h"
#include "CabacReader.h"
#include "BitReadBuffer.h"
#include "DecError.h"

// h264 namepace begin
H264AVC_NAMESPACE_BEGIN

const int MAX_COEFF[9] = {8,16,16,16,15,4,4,15,15};
const int COUNT_THR[9] = { 3, 4, 4, 4, 3, 2, 2, 3, 3};

#if 0 // FAST_CABAC
#define RNOKCABAC( exp ) exp
#else
#define RNOKCABAC( exp ) RNOK(exp)
#endif


CabacReader::CabacReader():
m_cFieldFlagCCModel   ( 1,                3),
m_cFldMapCCModel      ( NUM_BLOCK_TYPES,  NUM_MAP_CTX),
m_cFldLastCCModel     ( NUM_BLOCK_TYPES,  NUM_LAST_CTX),
  m_cBCbpCCModel        ( NUM_BLOCK_TYPES,  NUM_BCBP_CTX ),
  m_cMapCCModel         ( NUM_BLOCK_TYPES,  NUM_MAP_CTX ),
  m_cLastCCModel        ( NUM_BLOCK_TYPES,  NUM_LAST_CTX ),
  m_cRefCCModel         ( 1,  10 ),
  m_cSigCCModel         ( 1,  10 ),
  m_cOneCCModel         ( NUM_BLOCK_TYPES,  NUM_ABS_CTX ),
  m_cAbsCCModel         ( NUM_BLOCK_TYPES,  NUM_ABS_CTX ),
  m_cChromaPredCCModel  ( 1,                4 ),
	m_cSRFlagCCModel      ( 1,                1 ),	// JVT-R091
  m_cBLSkipCCModel      ( 1,                4 ),

  m_cMbTypeCCModel      ( 3,                NUM_MB_TYPE_CTX ),
  m_cBlockTypeCCModel   ( 2,                NUM_B8_TYPE_CTX ),
  m_cMvdCCModel         ( 2,                NUM_MV_RES_CTX ),
  m_cRefPicCCModel      ( 2,                NUM_REF_NO_CTX ),
  m_cBLPredFlagCCModel  ( 2,                NUM_BL_PRED_FLAG_CTX ),
  m_cResPredFlagCCModel ( 1,                NUM_RES_PRED_FLAG_CTX ),
  m_cDeltaQpCCModel     ( 1,                NUM_DELTA_QP_CTX ),
  m_cIntraPredCCModel   ( 9,                NUM_IPR_CTX ),
  m_cCbpCCModel         ( 3,                NUM_CBP_CTX ),
  m_cBCbpEnhanceCCModel ( NUM_BLOCK_TYPES,  NUM_BCBP_CTX ),
  m_cCbpEnhanceCCModel  ( 3,                NUM_CBP_CTX ),
  m_cTransSizeCCModel   ( 1,                NUM_TRANSFORM_SIZE_CTX ),

  m_uiBitCounter        ( 0 ),
  m_uiPosCounter        ( 0 ),
  m_uiLastDQpNonZero    ( 0 )
{
}


CabacReader::~CabacReader()
{
}


ErrVal CabacReader::xInitContextModels( const SliceHeader& rcSliceHeader )
{
  Int  iQp    = rcSliceHeader.getPicQp();
  Bool bIntra = rcSliceHeader.isIntra();
  Int  iIndex = rcSliceHeader.getCabacInitIdc();

  if( bIntra )
  {
    RNOK( m_cMbTypeCCModel.initBuffer(      (Short*)INIT_MB_TYPE_I,       iQp ) );
    RNOK( m_cBlockTypeCCModel.initBuffer(   (Short*)INIT_B8_TYPE_I,       iQp ) );
    RNOK( m_cMvdCCModel.initBuffer(         (Short*)INIT_MV_RES_I,        iQp ) );
    RNOK( m_cRefPicCCModel.initBuffer(      (Short*)INIT_REF_NO_I,        iQp ) );
    RNOK( m_cBLPredFlagCCModel.initBuffer(  (Short*)INIT_BL_PRED_FLAG_I,  iQp ) );
    RNOK( m_cResPredFlagCCModel.initBuffer( (Short*)INIT_RES_PRED_FLAG_I, iQp ) );
    RNOK( m_cDeltaQpCCModel.initBuffer(     (Short*)INIT_DELTA_QP_I,      iQp ) );
    RNOK( m_cIntraPredCCModel.initBuffer(   (Short*)INIT_IPR_I,           iQp ) );
    RNOK( m_cChromaPredCCModel.initBuffer(  (Short*)INIT_CIPR_I,          iQp ) )
    RNOK( m_cBLSkipCCModel.initBuffer(      (Short*)INIT_BL_SKIP,         iQp ) );

		RNOK( m_cSRFlagCCModel.initBuffer(      (Short*)INIT_SR_FLAG,         iQp ) );	// JVT-R091

    RNOK( m_cCbpCCModel.initBuffer(         (Short*)INIT_CBP_I,           iQp ) );
    RNOK( m_cBCbpCCModel.initBuffer(        (Short*)INIT_BCBP_I,          iQp ) );
    RNOK( m_cMapCCModel.initBuffer(         (Short*)INIT_MAP_I,           iQp ) );
    RNOK( m_cLastCCModel.initBuffer(        (Short*)INIT_LAST_I,          iQp ) );
    RNOK( m_cSigCCModel.initBuffer(         (Short*)INIT_SIG,             iQp ) );
    RNOK( m_cRefCCModel.initBuffer(         (Short*)INIT_REF,             iQp ) );
    RNOK( m_cOneCCModel.initBuffer(         (Short*)INIT_ONE_I,           iQp ) );
    RNOK( m_cAbsCCModel.initBuffer(         (Short*)INIT_ABS_I,           iQp ) );

    RNOK( m_cCbpEnhanceCCModel.initBuffer(  (Short*)INIT_CBP_ENH_I,       iQp ) );
    RNOK( m_cBCbpEnhanceCCModel.initBuffer( (Short*)INIT_BCBP_ENH_I,      iQp ) );

    RNOK( m_cTransSizeCCModel.initBuffer(   (Short*)INIT_TRANSFORM_SIZE_I,iQp ) );
    RNOK( m_cFieldFlagCCModel.initBuffer(   (Short*)INIT_MB_AFF_I,        iQp ) );
    RNOK( m_cFldMapCCModel.initBuffer(      (Short*)INIT_FLD_MAP_I,       iQp ) );
    RNOK( m_cFldLastCCModel.initBuffer(     (Short*)INIT_FLD_LAST_I,      iQp ) );

  }
  else
  {
    RNOK( m_cMbTypeCCModel.initBuffer(      (Short*)INIT_MB_TYPE_P        [iIndex], iQp ) );
    RNOK( m_cBlockTypeCCModel.initBuffer(   (Short*)INIT_B8_TYPE_P        [iIndex], iQp ) );
    RNOK( m_cMvdCCModel.initBuffer(         (Short*)INIT_MV_RES_P         [iIndex], iQp ) );
    RNOK( m_cRefPicCCModel.initBuffer(      (Short*)INIT_REF_NO_P         [iIndex], iQp ) );
    RNOK( m_cBLPredFlagCCModel.initBuffer(  (Short*)INIT_BL_PRED_FLAG_P   [iIndex], iQp ) );
    RNOK( m_cResPredFlagCCModel.initBuffer( (Short*)INIT_RES_PRED_FLAG_P  [iIndex], iQp ) );
    RNOK( m_cDeltaQpCCModel.initBuffer(     (Short*)INIT_DELTA_QP_P       [iIndex], iQp ) );
    RNOK( m_cIntraPredCCModel.initBuffer(   (Short*)INIT_IPR_P            [iIndex], iQp ) );
    RNOK( m_cChromaPredCCModel.initBuffer(  (Short*)INIT_CIPR_P           [iIndex], iQp ) )
    RNOK( m_cBLSkipCCModel.initBuffer(      (Short*)INIT_BL_SKIP,                   iQp ) );

		RNOK( m_cSRFlagCCModel.initBuffer(      (Short*)INIT_SR_FLAG,                   iQp ) );	// JVT-R091

    RNOK( m_cCbpCCModel.initBuffer(         (Short*)INIT_CBP_P            [iIndex], iQp ) );
    RNOK( m_cBCbpCCModel.initBuffer(        (Short*)INIT_BCBP_P           [iIndex], iQp ) );
    RNOK( m_cMapCCModel.initBuffer(         (Short*)INIT_MAP_P            [iIndex], iQp ) );
    RNOK( m_cLastCCModel.initBuffer(        (Short*)INIT_LAST_P           [iIndex], iQp ) );
    RNOK( m_cSigCCModel.initBuffer(         (Short*)INIT_SIG                      , iQp ) );
    RNOK( m_cRefCCModel.initBuffer(         (Short*)INIT_REF                      , iQp ) );
    RNOK( m_cOneCCModel.initBuffer(         (Short*)INIT_ONE_P            [iIndex], iQp ) );
    RNOK( m_cAbsCCModel.initBuffer(         (Short*)INIT_ABS_P            [iIndex], iQp ) );

    RNOK( m_cCbpEnhanceCCModel.initBuffer(  (Short*)INIT_CBP_ENH_P        [iIndex], iQp ) );
    RNOK( m_cBCbpEnhanceCCModel.initBuffer( (Short*)INIT_BCBP_ENH_P       [iIndex], iQp ) );

    RNOK( m_cTransSizeCCModel.initBuffer(   (Short*)INIT_TRANSFORM_SIZE_P [iIndex], iQp ) );
    RNOK( m_cFieldFlagCCModel.initBuffer(   (Short*)INIT_MB_AFF_P         [iIndex], iQp ) );
    RNOK( m_cFldMapCCModel.initBuffer(      (Short*)INIT_FLD_MAP_P        [iIndex], iQp ) );
    RNOK( m_cFldLastCCModel.initBuffer(     (Short*)INIT_FLD_LAST_P       [iIndex], iQp ) );

  }



  return Err::m_nOK;
}


ErrVal CabacReader::create( CabacReader*& rpcCabacReader )
{
  rpcCabacReader = new CabacReader;

  ROT( NULL == rpcCabacReader );

  return Err::m_nOK;
}

ErrVal CabacReader::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal CabacReader::init( BitReadBuffer* pcBitReadBuffer )
{
  ROT( NULL == pcBitReadBuffer );

  RNOK( CabaDecoder::init( pcBitReadBuffer ) );

  return Err::m_nOK;
}


ErrVal CabacReader::uninit()
{
  RNOK( CabaDecoder::uninit() );

  return Err::m_nOK;
}





ErrVal CabacReader::startSlice( const SliceHeader& rcSliceHeader )
{
  m_uiLastDQpNonZero  = 0;

  RNOK( xInitContextModels( rcSliceHeader ) );

  RNOK( CabaDecoder::start() );

  return Err::m_nOK;
}


ErrVal CabacReader::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  UInt uiRefFrame;
  DECRNOK( xRefFrame( rcMbDataAccess, uiRefFrame, eLstIdx, PART_8x8_0 ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setRefIdx( uiRefFrame );
  return Err::m_nOK;
}


ErrVal CabacReader::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  UInt uiRefFrame;
  DECRNOK( xRefFrame( rcMbDataAccess, uiRefFrame, eLstIdx, ParIdx8x8( eParIdx ) ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setRefIdx( uiRefFrame, eParIdx );
  return Err::m_nOK;
}

ErrVal CabacReader::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  UInt uiRefFrame;
  DECRNOK( xRefFrame( rcMbDataAccess, uiRefFrame, eLstIdx, ParIdx8x8( eParIdx ) ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setRefIdx( uiRefFrame, eParIdx );
  return Err::m_nOK;
}

ErrVal CabacReader::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  )
{
  UInt uiRefFrame;
  DECRNOK( xRefFrame( rcMbDataAccess, uiRefFrame, eLstIdx, eParIdx ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setRefIdx( uiRefFrame, eParIdx );
  return Err::m_nOK;
}


ErrVal CabacReader::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  Bool  bFlag;
  DECRNOK( xMotionPredFlag( bFlag, eLstIdx ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setMotPredFlag( bFlag );
  return Err::m_nOK;
}
ErrVal CabacReader::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  Bool  bFlag;
  DECRNOK( xMotionPredFlag( bFlag, eLstIdx ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setMotPredFlag( bFlag, eParIdx );
  return Err::m_nOK;
}
ErrVal CabacReader::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  Bool  bFlag;
  DECRNOK( xMotionPredFlag( bFlag, eLstIdx ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setMotPredFlag( bFlag, eParIdx );
  return Err::m_nOK;
}
ErrVal CabacReader::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8 eParIdx  )
{
  Bool  bFlag;
  DECRNOK( xMotionPredFlag( bFlag, eLstIdx ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setMotPredFlag( bFlag, eParIdx );
  return Err::m_nOK;
}



ErrVal CabacReader::xRefFrame( MbDataAccess& rcMbDataAccess, UInt& ruiRefFrame, ListIdx eLstIdx, ParIdx8x8 eParIdx )
{
  UInt uiCtx = rcMbDataAccess.getCtxRefIdx( eLstIdx, eParIdx );

  RNOKCABAC( CabaDecoder::getSymbol( ruiRefFrame, m_cRefPicCCModel.get( 0, uiCtx ) ) );

  if ( 0 != ruiRefFrame )
  {
    RNOKCABAC( CabaDecoder::getUnarySymbol( ruiRefFrame, &m_cRefPicCCModel.get( 0, 4 ), 1 ) );
    ruiRefFrame++;
  }

  ruiRefFrame++;
  DTRACE_DO(if (eLstIdx==LIST_0))
      DTRACE_T( "ref_idx_l0" );
  DTRACE_DO(else)
      DTRACE_T( "ref_idx_l1" );
  const UInt uiMaxRefFrames = rcMbDataAccess.getNumActiveRef( eLstIdx );
  DECROT( ruiRefFrame > uiMaxRefFrames );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE(ruiRefFrame);
  DTRACE_N;

  return Err::m_nOK;
}


ErrVal CabacReader::xMotionPredFlag(  Bool& bFlag, ListIdx eLstIdx )
{
  UInt  uiCode;

  UInt  uiCtx = 0;
  RNOKCABAC( CabaDecoder::getSymbol( uiCode, m_cBLPredFlagCCModel.get( eLstIdx, uiCtx ) ) );

  bFlag = ( uiCode != 0 );

  DTRACE_T( "MotionPredFlag" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( uiCode );
  DTRACE_N;

  return Err::m_nOK;
}



ErrVal CabacReader::blockModes( MbDataAccess& rcMbDataAccess )
{
  UInt uiSymbol;
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    UInt uiBlockMode = 0;

    if( ! rcMbDataAccess.getSH().isInterB() )
    {
      RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 0, 1 ) ) );
      if( 0 == uiSymbol )
      {
        RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 0, 3 ) ) );
        if( 0 != uiSymbol )
        {
          RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 0, 4 ) ) );
          uiBlockMode = ( 0 != uiSymbol) ? 2 : 3;
        }
        else
        {
          uiBlockMode = 1;
        }
      }
    }
    else
    {
      RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 0 ) ) );
      if( 0 != uiSymbol )
      {
        RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 1 ) ) );
        if( 0 != uiSymbol )
        {
          RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 2 ) ) );
          if( 0 != uiSymbol )
          {
            RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
            if( 0 != uiSymbol )
            {
              uiBlockMode = 10;
              RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
              uiBlockMode += uiSymbol;
            }
            else
            {
              uiBlockMode = 6;
              RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
              uiBlockMode += uiSymbol << 1;
              RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
              uiBlockMode += uiSymbol;
            }
          }
          else
          {
            uiBlockMode = 2;
            RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
            uiBlockMode += uiSymbol << 1;
            RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
            uiBlockMode += uiSymbol;
          }
        }
        else
        {
          RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
          uiBlockMode = uiSymbol;
        }
        uiBlockMode++;
      }
    }

    DTRACE_T( "BlockMode" );
    DTRACE_TY( "ae(v)" );
    DTRACE_CODE(uiBlockMode);
    DTRACE_N;

    rcMbDataAccess.setConvertBlkMode( c8x8Idx.b8x8Index(), uiBlockMode );
  }

  return Err::m_nOK;
}


Bool CabacReader::isMbSkipped( MbDataAccess& rcMbDataAccess )
{
  ROTRS( rcMbDataAccess.getSH().isIntra(), false );
  UInt uiSymbol;

  if( rcMbDataAccess.getSH().isInterB() )
  {
 //   CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 7 + rcMbDataAccess.getCtxDirectMbWoCoeff() ) );
	 CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 7 + rcMbDataAccess.getCtxMbSkipped() ) );//lufeng: follow jsvm
	  rcMbDataAccess.getMbData().setSkipFlag(0!=uiSymbol);
  }
  else
  {
    CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, rcMbDataAccess.getCtxMbSkipped() ) );
  }
  rcMbDataAccess.getMbData().setSkipFlag(0!=uiSymbol);

  ROTRS( 0 == uiSymbol, false );
  m_uiLastDQpNonZero = 0; // no DeltaQP for Skipped Macroblock
  DTRACE_T( "MbMode" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( 0 );
  DTRACE_N;
  return true;
}


Bool CabacReader::isBLSkipped( MbDataAccess& rcMbDataAccess )
{
  UInt uiSymbol = 0;
  UInt uiCtx    = rcMbDataAccess.getCtxBLSkipFlag();

  CabaDecoder::getSymbol( uiSymbol, m_cBLSkipCCModel.get( 0, uiCtx ) );

  DTRACE_T( "BLSkipFlag" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( uiSymbol );
  DTRACE_N;

  rcMbDataAccess.getMbData().setBLSkipFlag( ( uiSymbol != 0 ) );
  return rcMbDataAccess.getMbData().getBLSkipFlag();
}


ErrVal CabacReader::resPredFlag( MbDataAccess& rcMbDataAccess )
{
  UInt  uiSymbol;
  UInt  uiCtx = ( rcMbDataAccess.getMbData().getBLSkipFlag() ? 0 : 1 );

  RNOK( CabaDecoder::getSymbol( uiSymbol, m_cResPredFlagCCModel.get( 0, uiCtx ) ) );
  rcMbDataAccess.getMbData().setResidualPredFlag( (uiSymbol!=0) );

  DTRACE_T( "ResidualPredFlag" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( uiSymbol );
  DTRACE_N;

  return Err::m_nOK;
}

ErrVal CabacReader::resPredFlag_FGS( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff )
{
  UInt  uiSymbol;
  UInt  uiCtx = ( bBaseCoeff ? 2 : 3 );

  RNOK( CabaDecoder::getSymbol( uiSymbol, m_cResPredFlagCCModel.get( 0, uiCtx ) ) );
  rcMbDataAccess.getMbData().setResidualPredFlag( (uiSymbol!=0) );

  DTRACE_T( "ResidualPredFlag" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( uiSymbol );
  DTRACE_N;

  return Err::m_nOK;
}


//-- JVT-R091
ErrVal CabacReader::smoothedRefFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiSymbol;
  RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cSRFlagCCModel.get( 0, 0 ) ) );
  rcMbDataAccess.getMbData().setSmoothedRefFlag( (uiSymbol!=0) );

  DTRACE_T( "SmoothedRefFlag:" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( uiSymbol );
  DTRACE_N;

  return Err::m_nOK;
}
//--

ErrVal CabacReader::mbMode( MbDataAccess& rcMbDataAccess )
{
  rcMbDataAccess.getMbData().setBCBPAll( 0 );

  UInt uiMbMode;
  UInt act_sym;
  UInt mod_sym;

  if( rcMbDataAccess.getSH().isIntra() )
  {
    RNOKCABAC( CabaDecoder::getSymbol( act_sym, m_cMbTypeCCModel.get( 0, rcMbDataAccess.getCtxMbIntra4x4() ) ) );

    if( 0 != act_sym )
    {
      RNOKCABAC( CabaDecoder::getTerminateBufferBit( act_sym ) )
      if( 0 != act_sym )
      {
        act_sym = 25;
      }
      else
      {
        RNOKCABAC( CabaDecoder::getSymbol( act_sym, m_cMbTypeCCModel.get( 0, 4 ) ) );
        act_sym = 12* act_sym + 1;

        RNOKCABAC( CabaDecoder::getSymbol( mod_sym, m_cMbTypeCCModel.get( 0, 5 ) ) );

        if( 0 != mod_sym )
        {
          RNOKCABAC( CabaDecoder::getSymbol( mod_sym, m_cMbTypeCCModel.get( 0, 6 ) ) );
          act_sym += ++mod_sym << 2;
        }

        RNOKCABAC( CabaDecoder::getSymbol( mod_sym, m_cMbTypeCCModel.get( 0, 7 ) ) );
        act_sym += mod_sym << 1;

        RNOKCABAC( CabaDecoder::getSymbol( mod_sym, m_cMbTypeCCModel.get( 0, 8 ) ) );
        act_sym += mod_sym;
      }
    }
    uiMbMode = act_sym;
  }
  else
  {
    uiMbMode = 0;
    UInt uiSymbol;

    if( ! rcMbDataAccess.getSH().isInterB() )
    {
      RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 4 ) ) );
      if( 0 != uiSymbol )
      {
        RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 7 ) ) );
        uiMbMode = ( 0 != uiSymbol ) ? 7 : 6;
      }
      else
      {
        RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 5 ) ) );
        if( 0 != uiSymbol )
        {
          RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 7 ) ) );
          uiMbMode = ( 0 != uiSymbol ) ? 2 : 3;
        }
        else
        {
          RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 6 ) ) );
          uiMbMode = ( 0 != uiSymbol ) ? 4 : 1;
        }
      }
    }
    else
    {
      RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, rcMbDataAccess.getCtxMbType() ) ) );
      if( 0 != uiSymbol )
      {
        RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 4 ) ) );
        if( 0 != uiSymbol )
        {
          RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 5 ) ) );
          if( 0 != uiSymbol )
          {
            uiMbMode = 12;
            RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            uiMbMode += uiSymbol << 3;
            RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            uiMbMode += uiSymbol << 2;
            RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            uiMbMode += uiSymbol << 1;
            if( 24 == uiMbMode)
            {
              uiMbMode = 11;
            }
            else
            {
              if( 26 == uiMbMode)
              {
                uiMbMode = 22;
              }
              else
              {
                if( 22 == uiMbMode)
                {
                  uiMbMode = 23;
                }
                RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
                uiMbMode += uiSymbol;
              }
            }
          }
          else
          {
            uiMbMode = 3;
            RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            uiMbMode += uiSymbol << 2;
            RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            uiMbMode += uiSymbol << 1;
            RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            uiMbMode += uiSymbol;
          }
        }
        else
        {
          RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
          uiMbMode = ( 0 != uiSymbol ) ? 2 : 1;
        }
      }
    }

    if( ! ( uiMbMode <= 6 || (rcMbDataAccess.getSH().isInterB() && uiMbMode <= 23) ) )
    {
      RNOKCABAC( CabaDecoder::getTerminateBufferBit( uiSymbol ) )
      if( 0 != uiSymbol )
      {
        uiMbMode += ( rcMbDataAccess.getSH().isInterB() ) ? 22 : 24;
      }
      else
      {
        RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 8 ) ) );
        uiMbMode += ( 0 != uiSymbol ) ? 12 : 0;

        RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 9 ) ) );
        if( 0 != uiSymbol )
        {
          RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 9 ) ) );
          uiMbMode += ( 0 != uiSymbol ) ? 8 : 4;
        }

        RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 10 ) ) );
        uiMbMode += uiSymbol << 1;
        RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 10 ) ) );
        uiMbMode += uiSymbol;
      }
    }
    if( ! rcMbDataAccess.getSH().isInterB() )
    {
      uiMbMode--;
    }
  }

  rcMbDataAccess.setConvertMbType( uiMbMode );

  DTRACE_T( "MbMode" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( ( ! rcMbDataAccess.getSH().isIntra()) ? uiMbMode+1:uiMbMode );
  DTRACE_N;

  return Err::m_nOK;
}




ErrVal CabacReader::xGetMvdComponent( Short& rsMvdComp, UInt uiAbsSum, UInt uiCtx )
{

  UInt uiLocalCtx = uiCtx;

  if( uiAbsSum >= 3)
  {
    uiLocalCtx += ( uiAbsSum > 32) ? 3 : 2;
  }

  rsMvdComp = 0;

  UInt uiSymbol;
  RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cMvdCCModel.get( 0, uiLocalCtx ) ) );

  ROTRS( 0 == uiSymbol, Err::m_nOK )

  RNOKCABAC( CabaDecoder::getExGolombMvd( uiSymbol, &m_cMvdCCModel.get( 1, uiCtx ), 3 ) );
  uiSymbol++;

  UInt uiSign;
  RNOKCABAC( CabaDecoder::getEpSymbol( uiSign ) );

  rsMvdComp = ( 0 != uiSign ) ? -(Int)uiSymbol : (Int)uiSymbol;

  return Err::m_nOK;
}


ErrVal CabacReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  Mv cMv;
  DECRNOK( xGetMvd( rcMbDataAccess, cMv, B4x4Idx(0), eLstIdx ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv );
  return Err::m_nOK;
}

ErrVal CabacReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  Mv cMv;
  DECRNOK( xGetMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx );
  return Err::m_nOK;
}

ErrVal CabacReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  Mv cMv;
  DECRNOK( xGetMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx );
  return Err::m_nOK;
}
ErrVal CabacReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  )
{
  Mv cMv;
  DECRNOK( xGetMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx );
  return Err::m_nOK;
}
ErrVal CabacReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx )
{
  Mv cMv;
  DECRNOK( xGetMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx+eSParIdx), eLstIdx ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx, eSParIdx );
  return Err::m_nOK;
}
ErrVal CabacReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx )
{
  Mv cMv;
  DECRNOK( xGetMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx+eSParIdx), eLstIdx ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx, eSParIdx );
  return Err::m_nOK;
}
ErrVal CabacReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx )
{
  Mv cMv;
  DECRNOK( xGetMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx+eSParIdx), eLstIdx ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx, eSParIdx );
  return Err::m_nOK;
}




ErrVal CabacReader::xGetMvd( MbDataAccess& rcMbDataAccess, Mv& rcMv, LumaIdx cIdx, ListIdx eLstIdx )
{
  Mv    cMvA;
  Mv    cMvB;
  Short sMvdComponent;

  rcMbDataAccess.getMvdAbove( cMvA, eLstIdx, cIdx );
  rcMbDataAccess.getMvdLeft ( cMvB, eLstIdx, cIdx );

  DECRNOK( xGetMvdComponent( sMvdComponent, cMvA.getAbsHor() + cMvB.getAbsHor(), 0 ) );
  rcMv.setHor( sMvdComponent );

  DTRACE_T( "Mvd: x" );
  DTRACE_TY( "ae(v)" );
  DTRACE_V( sMvdComponent );
  DTRACE_T( " above " );
  DTRACE_V( cMvA.getHor() );
  DTRACE_T( " left " );
  DTRACE_V( cMvB.getHor() );
  DTRACE_N;

  DECRNOK( xGetMvdComponent( sMvdComponent, cMvA.getAbsVer() + cMvB.getAbsVer(), 5 ) );
  rcMv.setVer( sMvdComponent );

  DTRACE_T( "Mvd: y" );
  DTRACE_TY( "ae(v)" );
  DTRACE_V( sMvdComponent );
  DTRACE_T( " above " );
  DTRACE_V( cMvA.getVer() );
  DTRACE_T( " left " );
  DTRACE_V( cMvB.getVer() );
  DTRACE_N;

  return Err::m_nOK;
}

ErrVal CabacReader::fieldFlag( MbDataAccess& rcMbDataAccess )
{
    UInt uiSymbol;
    RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cFieldFlagCCModel.get( 0, rcMbDataAccess.getCtxFieldFlag() ) ) );

    rcMbDataAccess.setFieldMode( uiSymbol != 0 );

    DTRACE_T( "FieldFlag:" );
    DTRACE_TY( "ae(v)" );
    DTRACE_CODE( uiSymbol );
    DTRACE_N;

    return Err::m_nOK;
}


ErrVal CabacReader::intraPredModeChroma( MbDataAccess& rcMbDataAccess )
{

  UInt uiSymbol;
  RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cChromaPredCCModel.get( 0, rcMbDataAccess.getCtxChromaPredMode() ) ) );

  if( uiSymbol )
  {
    RNOKCABAC( CabaDecoder::getUnaryMaxSymbol( uiSymbol, m_cChromaPredCCModel.get( 0 ) + 3, 0, 2 ) );
    uiSymbol++;
  }

  rcMbDataAccess.getMbData().setChromaPredMode( uiSymbol );
  DTRACE_T( "IntraPredModeChroma" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( uiSymbol );
  DTRACE_N;

  return Err::m_nOK;
}



ErrVal CabacReader::intraPredModeLuma( MbDataAccess& rcMbDataAccess, LumaIdx cIdx )
{
  UInt uiSymbol;
  UInt uiIPredMode;

  RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cIntraPredCCModel.get( 0, 0 ) ) );

  if( ! uiSymbol )
  {
    RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cIntraPredCCModel.get( 0, 1 ) ) );
    uiIPredMode  = uiSymbol;
    RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cIntraPredCCModel.get( 0, 1 ) ) );
    uiIPredMode |= (uiSymbol << 1);
    RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cIntraPredCCModel.get( 0, 1 ) ) );
    rcMbDataAccess.getMbData().intraPredMode( cIdx ) = ( uiIPredMode | (uiSymbol << 2) );
  }
  else
  {
    rcMbDataAccess.getMbData().intraPredMode( cIdx ) = -1;
  }

  DTRACE_T( "IntraPredModeLuma" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( rcMbDataAccess.getMbData().intraPredMode( cIdx ) );
  DTRACE_N;

  return Err::m_nOK;
}


ErrVal CabacReader::cbp( MbDataAccess& rcMbDataAccess )
{
  UInt uiCbp;
  UInt uiBit;
  UInt uiCtx = 0, a, b;

  a = rcMbDataAccess.getLeftLumaCbp ( B4x4Idx( 0 ) );
  b = rcMbDataAccess.getAboveLumaCbp( B4x4Idx( 0 ) ) << 1;

  RNOKCABAC( CabaDecoder::getSymbol( uiBit, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );
  uiCbp = uiBit;

  a = uiCbp & 1;
  b = rcMbDataAccess.getAboveLumaCbp( B4x4Idx( 2 ) ) << 1;

  RNOKCABAC( CabaDecoder::getSymbol( uiBit, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );
  uiCbp += uiBit << 1;

  a = rcMbDataAccess.getLeftLumaCbp ( B4x4Idx( 8 ) );
  b = (uiCbp  << 1) & 2;

  RNOKCABAC( CabaDecoder::getSymbol( uiBit, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );
  uiCbp += uiBit << 2;

  a = ( uiCbp >> 2 ) & 1;
  b = uiCbp & 2;

  RNOKCABAC( CabaDecoder::getSymbol( uiBit, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );
  uiCbp += uiBit << 3;

  uiCtx = 1;

  UInt  uiLeftChromaCbp   = rcMbDataAccess.getLeftChromaCbp ();
  UInt  uiAboveChromaCbp  = rcMbDataAccess.getAboveChromaCbp();

  a = uiLeftChromaCbp  > 0 ? 1 : 0;
  b = uiAboveChromaCbp > 0 ? 2 : 0;

  RNOKCABAC( CabaDecoder::getSymbol( uiBit, m_cCbpCCModel.get( uiCtx, a + b ) ) );

  if( uiBit )
  {
    a = uiLeftChromaCbp  > 1 ? 1 : 0;
    b = uiAboveChromaCbp > 1 ? 2 : 0;

    RNOKCABAC( CabaDecoder::getSymbol( uiBit, m_cCbpCCModel.get( ++uiCtx, a + b ) ) );
    uiCbp += (1 == uiBit) ? 32 : 16;
  }

  if( !uiCbp )
  {
    m_uiLastDQpNonZero = 0; // no DeltaQP for Macroblocks with zero Cbp
  }

  AOF_DBG( 48 >= uiCbp );

  DTRACE_T( "Cbp" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( uiCbp );
  DTRACE_N;

  rcMbDataAccess.getMbData().setMbCbp( uiCbp );
  return Err::m_nOK;
}







ErrVal CabacReader::residualBlock( MbDataAccess&  rcMbDataAccess,
                                   LumaIdx        cIdx,
                                   ResidualMode   eResidualMode,
                                   UInt&          ruiMbExtCbp )
{
    const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
    const UChar* pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;

  Bool bCoded;

  DTRACE_T( "LUMA:" );
  DTRACE_V( cIdx );
  DTRACE_N;

  DECRNOK( xReadBCbp( rcMbDataAccess, bCoded, eResidualMode, cIdx ) );

  if( ! bCoded )
  {
    ruiMbExtCbp &= ~(1 << cIdx.b4x4() );
    return Err::m_nOK;
  }

  switch( eResidualMode )
  {
  case LUMA_I16_AC:
  case LUMA_SCAN:
      {
          break;
      }
  case LUMA_I16_DC:
      {
          pucScan = (bFrame) ? g_aucLumaFrameDCScan : g_aucLumaFieldDCScan;
          break;
      }
  default:
      AOT( 1 );
      break;
  }

  TCoeff*       piCoeff   = rcMbDataAccess.getMbTCoeffs().get( cIdx );

  DECRNOK( xReadCoeff( piCoeff, eResidualMode, pucScan, !bFrame ) );

  return Err::m_nOK;
}




ErrVal CabacReader::residualBlock( MbDataAccess&  rcMbDataAccess,
                                   ChromaIdx      cIdx,
                                   ResidualMode   eResidualMode
                                   )
{
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar* pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;

  Bool bCoded;

  switch( eResidualMode )
  {
  case CHROMA_DC:
      {
          DTRACE_T( "CHROMA_DC:" );
          pucScan = (UChar*)g_aucIndexChromaDCScan;
          break;
      }
  case CHROMA_AC:
      {
          DTRACE_T( "CHROMA_AC:" );
          break;
      }
  default:
      AOT( 1 );
      break;
  }

  DTRACE_V( cIdx );
  DTRACE_N;

  TCoeff*       piCoeff   = rcMbDataAccess.getMbTCoeffs().get( cIdx );

  DECRNOK( xReadBCbp( rcMbDataAccess, bCoded, eResidualMode, cIdx ) );

  ROTRS( ! bCoded, Err::m_nOK );

  DECRNOK( xReadCoeff( piCoeff, eResidualMode, pucScan, !bFrame ) );

  return Err::m_nOK;
}



ErrVal CabacReader::deltaQp( MbDataAccess& rcMbDataAccess )
{
  UInt uiDQp;
  Int iDQp = 0;
  UInt uiCtx = m_uiLastDQpNonZero;

  RNOKCABAC( CabaDecoder::getSymbol( uiDQp, m_cDeltaQpCCModel.get( 0, uiCtx ) ) );

  m_uiLastDQpNonZero = uiDQp;

  if( uiDQp )
  {
    RNOKCABAC( CabaDecoder::getUnarySymbol( uiDQp, &m_cDeltaQpCCModel.get( 0, 2 ), 1 ) );

    iDQp = (uiDQp + 2) / 2;

    if( uiDQp & 1 )
    {
      iDQp = -iDQp;
    }
  }

  DTRACE_T( "DQp" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE ( iDQp );
  DTRACE_N;

  rcMbDataAccess.addDeltaQp( iDQp );

  Quantizer::setQp( rcMbDataAccess, false );

  return Err::m_nOK;
}


Bool CabacReader::isEndOfSlice()
{
  UInt uiEOS;

  CabaDecoder::getTerminateBufferBit( uiEOS );
  DTRACE_T( "EOS" );
  DTRACE_CODE( uiEOS );
  DTRACE_N;

  return (uiEOS == 1);
}


ErrVal CabacReader::xReadBCbp( MbDataAccess& rcMbDataAccess, Bool& rbCoded, ResidualMode eResidualMode, ChromaIdx cIdx )
{
  UInt uiBitPos;

  if( CHROMA_AC == eResidualMode )
  {
    uiBitPos = 16 + cIdx;
  }
  else if( CHROMA_DC == eResidualMode )
  {
    uiBitPos = 24 + cIdx.plane();
  }
  else
  {
    // full stop
    AF();
    return Err::m_nERR;
  }

  UInt uiCtx      = rcMbDataAccess.getCtxCodedBlockBit( uiBitPos );
  UInt uiBit;

  RNOKCABAC( CabaDecoder::getSymbol( uiBit, m_cBCbpCCModel.get( type2ctx1[ eResidualMode ], uiCtx) ) );

  rbCoded = ( uiBit == 1 );
  rcMbDataAccess.getMbData().setBCBP( uiBitPos, uiBit);

  return Err::m_nOK;
}


ErrVal CabacReader::xReadBCbp( MbDataAccess& rcMbDataAccess, Bool& rbCoded, ResidualMode eResidualMode, LumaIdx cIdx )
{
  UInt uiBitPos;

  if( LUMA_SCAN == eResidualMode || LUMA_I16_AC == eResidualMode )
  {
    uiBitPos = cIdx;
  }
  else if( LUMA_I16_DC == eResidualMode )
  {
    uiBitPos = 26;
  }
  else
  {
    // full stop
    AF();
    return Err::m_nERR;
  }

  UInt uiCtx      = rcMbDataAccess.getCtxCodedBlockBit( uiBitPos );
  UInt uiBit;

  RNOKCABAC( CabaDecoder::getSymbol( uiBit, m_cBCbpCCModel.get( type2ctx1[ eResidualMode ], uiCtx) ) );

  rbCoded = ( uiBit == 1 );
  rcMbDataAccess.getMbData().setBCBP( uiBitPos, uiBit);


  return Err::m_nOK;
}




ErrVal CabacReader::xReadCoeff( TCoeff*         piCoeff,
                               ResidualMode    eResidualMode,
                               const UChar*    pucScan,
                               Bool            bFieldModel
                               )
{
  CabacContextModel2DBuffer&  rcMapCCModel  = ( bFieldModel ? m_cFldMapCCModel  : m_cMapCCModel  );
  CabacContextModel2DBuffer&  rcLastCCModel = ( bFieldModel ? m_cFldLastCCModel : m_cLastCCModel );

  UInt uiStart = 0;
  UInt uiStop  = 15;

  if( CHROMA_AC   == eResidualMode  ||
      LUMA_I16_AC == eResidualMode    )
  {
    uiStart = 1;
  }
  if( CHROMA_DC   == eResidualMode    )
  {
    uiStop = 3;
  }

  UInt ui;
  for( ui = uiStart; ui < uiStop; ui++ ) // if last coeff is reached, it has to be significant
  {
    UInt uiSig;
    //--- read significance symbol ---
    RNOKCABAC( CabaDecoder::getSymbol( uiSig, rcMapCCModel.get( type2ctx2[eResidualMode], ui ) ) );

    if( uiSig )
    {
      piCoeff[pucScan[ui]] = uiSig;

      UInt uiLast;
      RNOKCABAC( CabaDecoder::getSymbol( uiLast, rcLastCCModel.get( type2ctx2[eResidualMode], ui ) ) );
      if( uiLast )
      {
        break;
      }

    }
  }
  //--- last coefficient must be significant if no last symbol was received ---
  if( ui == uiStop )
  {
    piCoeff[pucScan[ui]] = 1;
  }

  int   c1 = 1;
  int   c2 = 0;

  ui++;

  while( (ui--) != uiStart )
  {
    UInt uiCoeff = piCoeff[pucScan[ui]];
    if( uiCoeff )
    {
      UInt uiCtx = min (c1,4);

      RNOKCABAC( CabaDecoder::getSymbol( uiCoeff, m_cOneCCModel.get( type2ctx1[eResidualMode], uiCtx ) ) );

      if( 1 == uiCoeff )
      {
        uiCtx = min (c2,4);
        RNOKCABAC( CabaDecoder::getExGolombLevel( uiCoeff, m_cAbsCCModel.get( type2ctx1[eResidualMode], uiCtx ) ) );
        uiCoeff += 2;

        c1=0;
        c2++;
      }
      else if (c1)
      {
        uiCoeff++;
        c1++;
      }
      else
      {
        uiCoeff++;
      }

      UInt uiSign;
      RNOKCABAC( CabaDecoder::getEpSymbol( uiSign ) );

      piCoeff[pucScan[ui]] = ( uiSign ? -(Int)uiCoeff : (Int)uiCoeff );
    }
  }

  return Err::m_nOK;
}



ErrVal CabacReader::samplesPCM( MbDataAccess& rcMbDataAccess )
{
  RNOK( CabaDecoder::finish() );

  DTRACE_POS;
  DTRACE_T( "  PCM SAMPLES: " );

  AOF_DBG( rcMbDataAccess.getMbData().isPCM() );

  Pel* pDest = rcMbDataAccess.getMbTCoeffs().getPelBuffer();

  // get chroma mode
  const UInt uiFactor = 8*8;
  const UInt uiSize   = uiFactor*2*3;
  DECRNOK( m_pcBitReadBuffer->samples( pDest, uiSize ) );

  DTRACE_N;

  rcMbDataAccess.getMbData().setBCBPAll( 1 );

  RNOK( CabaDecoder::start() );
  return Err::m_nOK;
}







ErrVal CabacReader::transformSize8x8Flag( MbDataAccess& rcMbDataAccess)
{
  UInt uiSymbol;
  UInt uiCtx = rcMbDataAccess.getCtx8x8Flag();

  RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cTransSizeCCModel.get( 0, uiCtx ) ) );

  rcMbDataAccess.getMbData().setTransformSize8x8( uiSymbol != 0 );

  DTRACE_T( "transformSize8x8Flag:" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( rcMbDataAccess.getMbData().isTransformSize8x8() );
  DTRACE_N;

  return Err::m_nOK;
}


ErrVal CabacReader::intraPredModeLuma8x8( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx )
{
  UInt uiSymbol;
  UInt uiIPredMode;

  RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cIntraPredCCModel.get( 0, 0 ) ) );

  if( ! uiSymbol )
  {
    RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cIntraPredCCModel.get( 0, 1 ) ) );
    uiIPredMode  = uiSymbol;
    RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cIntraPredCCModel.get( 0, 1 ) ) );
    uiIPredMode |= (uiSymbol << 1);
    RNOKCABAC( CabaDecoder::getSymbol( uiSymbol, m_cIntraPredCCModel.get( 0, 1 ) ) );
    rcMbDataAccess.getMbData().intraPredMode( cIdx ) = ( uiIPredMode | (uiSymbol << 2) );
  }
  else
  {
    rcMbDataAccess.getMbData().intraPredMode( cIdx ) = -1;
  }

  DTRACE_T( "IntraPredModeLuma" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( rcMbDataAccess.getMbData().intraPredMode( cIdx ) );
  DTRACE_N;

  return Err::m_nOK;
}



ErrVal CabacReader::residualBlock8x8( MbDataAccess& rcMbDataAccess,
                                      B8x8Idx       cIdx )
{
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar* pucScan = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;

  DTRACE_T( "LUMA_8x8:" );
  DTRACE_V( cIdx.b8x8Index() );
  DTRACE_N;

  {
    UInt uiBitPos = cIdx;
    rcMbDataAccess.getMbData().setBCBP( uiBitPos,   1);
    rcMbDataAccess.getMbData().setBCBP( uiBitPos+1, 1);
    rcMbDataAccess.getMbData().setBCBP( uiBitPos+4, 1);
    rcMbDataAccess.getMbData().setBCBP( uiBitPos+5, 1);
  }

  TCoeff*     piCoeff     = rcMbDataAccess.getMbTCoeffs().get8x8( cIdx );
  const Int*  pos2ctx_map = (bFrame) ? pos2ctx_map8x8 : pos2ctx_map8x8i;
  UInt        uiStart     = 0;
  UInt        uiStop      = 63;
  UInt        ui;


  {
    const UInt          uiCtxOffset       = 2;
    CabacContextModel*  pcMapCCModel  = ( bFrame ? m_cMapCCModel  : m_cFldMapCCModel). get( uiCtxOffset );
    CabacContextModel*  pcLastCCModel = ( bFrame ? m_cLastCCModel : m_cFldLastCCModel).get( uiCtxOffset );

    for( ui = uiStart; ui < uiStop; ui++ ) // if last coeff is reached, it has to be significant
    {
      UInt uiSig;
      //--- read significance symbol ---
        RNOKCABAC( CabaDecoder::getSymbol( uiSig, pcMapCCModel[pos2ctx_map[ui]] ) );

      if( uiSig )
      {
        piCoeff[pucScan[ui]] = uiSig;
        UInt uiLast;

          RNOKCABAC( CabaDecoder::getSymbol( uiLast, pcLastCCModel[pos2ctx_last8x8[ui]] ) );

        if( uiLast )
        {
          break;
        }
      }
    }
    //--- last coefficient must be significant if no last symbol was received ---
    if( ui == uiStop )
    {
      piCoeff[pucScan[ui]] = 1;
    }
  }


  int   c1 = 1;
  int   c2 = 0;

  ui++;
  const UInt uiCtxOffset = 2;

  
  while( (ui--) != uiStart )
  {
    Int   iIndex  = pucScan[ui];
    UInt  uiCoeff = piCoeff[iIndex];
    
    if( uiCoeff )
    {
      UInt uiCtx = min (c1,4);

      RNOKCABAC( CabaDecoder::getSymbol( uiCoeff, m_cOneCCModel.get( uiCtxOffset, uiCtx ) ) );

      if( 1 == uiCoeff )
      {
        uiCtx = min (c2,4);
        RNOKCABAC( CabaDecoder::getExGolombLevel( uiCoeff, m_cAbsCCModel.get( uiCtxOffset, uiCtx ) ) );
        uiCoeff += 2;

        c1=0;
        c2++;
      }
      else if (c1)
      {
        uiCoeff++;
        c1++;
      }
      else
      {
        uiCoeff++;
      }

      UInt uiSign;
      RNOKCABAC( CabaDecoder::getEpSymbol( uiSign ) );

      piCoeff[iIndex] = ( uiSign ? -(Int)uiCoeff : (Int)uiCoeff );
    }
  }

  return Err::m_nOK;
}


Bool
CabacReader::RQpeekCbp4x4(    MbDataAccess&  rcMbDataAccessBase,
                               LumaIdx        cIdx )
{
  UInt    uiSymbol  = 0;

  uiSymbol = rcMbDataAccessBase.getMbData().getBCBP( cIdx );
  
  return ( uiSymbol == 1 );
}


Bool
CabacReader::RQdecodeBCBP_4x4( MbDataAccess&  rcMbDataAccessBase,
                               LumaIdx        cIdx )
{
  UInt    uiSymbol  = 0;
  UInt    uiCtx     = rcMbDataAccessBase.getCtxCodedBlockBit( cIdx );

  ANOK( CabaDecoder::getSymbol( uiSymbol, m_cBCbpCCModel.get( type2ctx1[LUMA_SCAN], uiCtx ) ) );
  DTRACE_T( "BCBP_4x4" );
  DTRACE_V( uiSymbol );
  DTRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( cIdx, uiSymbol );
  
  return ( uiSymbol == 1 );
}



Bool
CabacReader::RQdecodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx )
{
  UInt    uiSymbol  = 0;
  UInt    uiCtx     = rcMbDataAccessBase.getCtxCodedBlockBit( 24 + cIdx.plane() );

  ANOK( CabaDecoder::getSymbol( uiSymbol, m_cBCbpCCModel.get( type2ctx1[CHROMA_DC], uiCtx ) ) );
  DTRACE_T( "BCBP_ChromaDC" );
  DTRACE_V( uiSymbol );
  DTRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( 24 + cIdx.plane(), uiSymbol );
  
  return ( uiSymbol == 1 );
}


Bool
CabacReader::RQdecodeBCBP_ChromaAC( MbDataAccess&  rcMbDataAccessBase,
                                    ChromaIdx      cIdx )
{
  UInt    uiSymbol  = 0;
  UInt    uiCtx     = rcMbDataAccessBase.getCtxCodedBlockBit( 16 + cIdx );

  ANOK( CabaDecoder::getSymbol( uiSymbol, m_cBCbpCCModel.get( type2ctx1[CHROMA_AC], uiCtx ) ) );
  DTRACE_T( "BCBP_ChromaAC" );
  DTRACE_V( uiSymbol );
  DTRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( 16 + cIdx, uiSymbol );
  
  return ( uiSymbol == 1 );
}







Bool
CabacReader::RQdecodeCBP_Chroma( MbDataAccess& rcMbDataAccess,
                                 MbDataAccess& rcMbDataAccessBase )
{
  UInt  uiSymbol          = 0;
  UInt  uiLeftChromaCbp   = rcMbDataAccessBase.getLeftChromaCbpFGS ();
  UInt  uiAboveChromaCbp  = rcMbDataAccessBase.getAboveChromaCbpFGS();
  UInt  uiCtx             = ( uiLeftChromaCbp > 0 ? 1 : 0 ) + ( uiAboveChromaCbp > 0 ? 2 : 0 );

  ANOK( CabaDecoder::getSymbol( uiSymbol, m_cCbpCCModel.get( 1, uiCtx ) ) );
  DTRACE_T( "CBP_Chroma" );
  DTRACE_V( uiSymbol );
  DTRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | 0x10 );
  }
  return ( uiSymbol == 1 );
}

Bool
CabacReader::RQdecodeCBP_ChromaAC( MbDataAccess& rcMbDataAccess,
                                   MbDataAccess& rcMbDataAccessBase )
{
  UInt  uiSymbol          = 0;
  UInt  uiLeftChromaCbp   = rcMbDataAccessBase.getLeftChromaCbpFGS ();
  UInt  uiAboveChromaCbp  = rcMbDataAccessBase.getAboveChromaCbpFGS();
  UInt  uiCtx             = ( uiLeftChromaCbp > 1 ? 1 : 0 ) + ( uiAboveChromaCbp > 1 ? 2 : 0 );

  ANOK( CabaDecoder::getSymbol( uiSymbol, m_cCbpCCModel.get( 2, uiCtx ) ) );
  DTRACE_T( "CBP_ChromaAC" );
  DTRACE_V( uiSymbol );
  DTRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( ( rcMbDataAccessBase.getMbData().getMbCbp() & 0xF ) | 0x20 );
  }
  return ( uiSymbol == 1 );
}

Bool
CabacReader::RQdecodeCBP_8x8( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase,
                              B8x8Idx       c8x8Idx )
{
  UInt  uiSymbol        = 0;
  UInt  uiCurrentCbp    = rcMbDataAccessBase.getMbData().getMbCbp();
  UInt  uiLeftLumaCbp   = rcMbDataAccessBase.getLeftLumaCbpFGS ( c8x8Idx );
  UInt  uiAboveLumaCbp  = rcMbDataAccessBase.getAboveLumaCbpFGS( c8x8Idx );
  UInt  uiCtx           = 0;

  switch( c8x8Idx.b8x8Index() )
  {
  case 0:   uiCtx = 3 - uiLeftLumaCbp         - 2*uiAboveLumaCbp;       break;
  case 1:   uiCtx = 3 - ( uiCurrentCbp    &1) - 2*uiAboveLumaCbp;       break;
  case 2:   uiCtx = 3 - uiLeftLumaCbp         - ((uiCurrentCbp<<1)&2);  break;
  case 3:   uiCtx = 3 - ((uiCurrentCbp>>2)&1) - ( uiCurrentCbp    &2);  break;
  default:  AF();
  }

  ANOK( CabaDecoder::getSymbol( uiSymbol, m_cCbpCCModel.get( 0, uiCtx ) ) );
  DTRACE_T( "CBP_Luma" );
  DTRACE_V( uiSymbol );
  DTRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | ( 1 << c8x8Idx.b8x8Index() ) );
  }

  return ( uiSymbol == 1 );
}




ErrVal
CabacReader::RQdecodeDeltaQp( MbDataAccess& rcMbDataAccess)
{
  UInt  uiDQp = 0;
  Int   iDQp  = 0;
  UInt  uiCtx = 0;

  RNOK( CabaDecoder::getSymbol( uiDQp, m_cDeltaQpCCModel.get( 0, uiCtx ) ) );
  if( uiDQp )
  {
    RNOK( CabaDecoder::getUnarySymbol( uiDQp, &m_cDeltaQpCCModel.get( 0, 2 ), 1 ) );

    iDQp = (uiDQp + 2) / 2;

    if( uiDQp & 1 )
    {
      iDQp = -iDQp;
    }
  }
  DTRACE_T( "DQP" );
  DTRACE_V( uiDQp );
  DTRACE_N;

  rcMbDataAccess.addDeltaQp( iDQp );

  return Err::m_nOK;
}



ErrVal
CabacReader::RQdecode8x8Flag( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase ) 
{
  UInt uiSymbol = 0;
  UInt uiCtx    = rcMbDataAccessBase.getCtx8x8Flag();
 
  RNOK( CabaDecoder::getSymbol( uiSymbol, m_cTransSizeCCModel.get( 0, uiCtx ) ) );
  DTRACE_T( "TRAFO_8x8" );
  DTRACE_V( uiSymbol );
  DTRACE_N;

  rcMbDataAccess    .getMbData().setTransformSize8x8( uiSymbol == 1 );
  rcMbDataAccessBase.getMbData().setTransformSize8x8( uiSymbol == 1 );

  return Err::m_nOK;
}


ErrVal
CabacReader::RQdecodeTermBit ( UInt& ruiBit )
{
  RNOK( CabaDecoder::getTerminateBufferBit( ruiBit ) );
  DTRACE_T( "EOS" );
  DTRACE_V( ruiBit );
  DTRACE_N;

  return Err::m_nOK;
}


ErrVal
CabacReader::RQeo8b( Bool& bEob )
{
  bEob = false;
  return Err::m_nOK;
}


ErrVal
CabacReader::RQdecodeNewTCoeff_8x8( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx,
                                    UInt            uiScanIndex,
                                    Bool&           rbLast,
                                    UInt&           ruiNumCoefRead )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const UChar*  pucScan     = g_aucFrameScan64;

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  DTRACE_T( "LUMA_8x8_NEW" );
  DTRACE_V( c8x8Idx.b8x8Index() );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4(),   1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+1, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+4, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+5, 1 );

  RNOK( xRQdecodeNewTCoeffs( piCoeff, piCoeffBase, 64, 2, 2, pucScan, uiScanIndex, rbLast, ruiNumCoefRead, pos2ctx_last8x8, pos2ctx_map8x8, 4 ) );

  return Err::m_nOK;
}



ErrVal
CabacReader::RQdecodeTCoeffRef_8x8( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx,
                                    UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const UChar*  pucScan     = g_aucFrameScan64;

  DTRACE_T( "LUMA_8x8_REF" );
  DTRACE_V( c8x8Idx.b8x8Index() );
  DTRACE_V( uiScanIndex );
  DTRACE_N;


  UInt uiSig = 0;
  RNOK( CabaDecoder::getSymbol( uiSig, m_cRefCCModel.get( 0, 0 ) ) );

  if( uiSig )
  {
    UInt uiSymbol = 0;
    RNOK( CabaDecoder::getSymbol( uiSymbol, m_cRefCCModel.get( 0, 1 ) ) );
    UInt uiSignBL = ( piCoeffBase[pucScan[uiScanIndex]] < 0 ? 1 : 0 );
    UInt uiSignEL = ( uiSignBL ^ uiSymbol );

    piCoeff[pucScan[uiScanIndex]] = ( uiSig ? ( uiSignEL ? -1 : 1 ) : 0 );
  }

  return Err::m_nOK;
}






ErrVal
CabacReader::RQdecodeNewTCoeff_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      LumaIdx         cIdx,
                                      UInt            uiScanIndex,
                                      Bool&           rbLast,
                                      UInt&           ruiNumCoefRead )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = g_aucFrameScan;
  //UInt          uiStart     = 0; // unused variable. mwi
  UInt          uiStop      = 16;

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  DTRACE_T( "LUMA_4x4_NEW" );
  DTRACE_V( cIdx.b4x4() );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  RNOK( xRQdecodeNewTCoeffs( piCoeff, piCoeffBase, uiStop, type2ctx1[eResidualMode], type2ctx2[eResidualMode], pucScan, uiScanIndex, rbLast, ruiNumCoefRead ) );

  return Err::m_nOK;
}



ErrVal
CabacReader::RQdecodeTCoeffRef_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      LumaIdx         cIdx,
                                      UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = g_aucFrameScan;

  DTRACE_T( "LUMA_4x4_REF" );
  DTRACE_V( cIdx.b4x4() );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  RNOK( xRQdecodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );
  return Err::m_nOK;
}



ErrVal
CabacReader::RQdecodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex,
                                        Bool&           rbLast,
                                        UInt&           ruiNumCoefRead )
{
  // == Nokia, m11509
  //ruiLast                   = 0;
  // ==
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : g_aucFrameScan );
  //UInt          uiStart     = ( eResidualMode == CHROMA_AC ? 1 : 0  ); // unused variable. mwi
  UInt          uiStop      = ( eResidualMode == CHROMA_DC ? 4 : 16 );

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  DTRACE_T( "CHROMA_4x4_NEW" );
  DTRACE_V( cIdx );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  RNOK( xRQdecodeNewTCoeffs( piCoeff, piCoeffBase, uiStop, type2ctx1[eResidualMode], type2ctx2[eResidualMode], pucScan, uiScanIndex, rbLast, ruiNumCoefRead ) );

  return Err::m_nOK;
}



ErrVal
CabacReader::RQdecodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : g_aucFrameScan );

  DTRACE_T( "CHROMA_4x4_REF" );
  DTRACE_V( cIdx );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  RNOK( xRQdecodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );
  return Err::m_nOK;
}




ErrVal
CabacReader::xRQdecodeNewTCoeffs( TCoeff*       piCoeff,
                                  TCoeff*       piCoeffBase,
                                  UInt          uiStop,
                                  UInt          uiCtx1,
                                  UInt          uiCtx2,
                                  const UChar*  pucScan,
                                  UInt          uiScanIndex,
                                  Bool&         rbLast,
                                  UInt&         ruiNumCoefRead,
                                  const int*    paiCtxEobMap,
                                  const int*    paiCtxSigMap,
                                  UInt          uiStride )
{
  ruiNumCoefRead = 0;
  if( rbLast )
  {
    UInt uiSymbol;
    RNOK( CabaDecoder::getSymbol( uiSymbol, m_cLastCCModel.get( uiCtx2, paiCtxEobMap[uiScanIndex-1] ) ) );
    rbLast = (uiSymbol == 1);
    ROTRS(rbLast, Err::m_nOK);
  }

  //===== SIGNIFICANCE BIT ======
  UInt uiSig;
  do
  {
    ruiNumCoefRead++;

    UInt uiLastScanPosition = uiScanIndex + uiStride;
    while (uiLastScanPosition < uiStop && piCoeffBase[pucScan[uiLastScanPosition]])
      uiLastScanPosition += uiStride;
  
    if (uiLastScanPosition < uiStop)
    {
      RNOK( CabaDecoder::getSymbol( uiSig, m_cMapCCModel.get( uiCtx2, paiCtxSigMap[uiScanIndex] ) ) );
    } else {
      uiSig = 1;
    }

    if( uiSig )
    {
      break;
    }

    uiScanIndex += uiStride;
    while (uiScanIndex < uiStop && piCoeffBase[pucScan[uiScanIndex]])
      uiScanIndex += uiStride;
  }
  while ( true );
    UInt  uiCoeff = uiSig;
    UInt  uiSign  = 0;
    UInt  uiCtx   = 1;

    //===== SIGN =====
    RNOK( CabaDecoder::getEpSymbol( uiSign ) );

    //===== ABSOLUTE VALUE =====
    RNOK( CabaDecoder::getSymbol  ( uiCoeff, m_cOneCCModel.get( uiCtx1, uiCtx ) ) );

    if( uiCoeff )
    {
      uiCtx = 0;
      RNOK( CabaDecoder::getExGolombLevel( uiCoeff, m_cAbsCCModel.get( uiCtx1, uiCtx ) ) );

      uiCoeff += 2;
    }
    else
    {
      uiCoeff++;
    }

    piCoeff[pucScan[uiScanIndex]] = ( uiSign ? -(Int)uiCoeff : (Int)uiCoeff );


  return Err::m_nOK;
}






ErrVal
CabacReader::xRQdecodeTCoeffsRef( TCoeff*       piCoeff,
                                  TCoeff*       piCoeffBase,
                                  const UChar*  pucScan,
                                  UInt          uiScanIndex )
{
  UInt uiSig = 0;
  RNOK( CabaDecoder::getSymbol( uiSig, m_cRefCCModel.get( 0, 0 ) ) );

  if( uiSig )
  {
    UInt uiSymbol = 0;
    RNOK( CabaDecoder::getSymbol( uiSymbol, m_cRefCCModel.get( 0, 1 ) ) );
    UInt uiSignBL = ( piCoeffBase[pucScan[uiScanIndex]] < 0 ? 1 : 0 );
    UInt uiSignEL = ( uiSignBL ^ uiSymbol );

    piCoeff[pucScan[uiScanIndex]] = ( uiSig ? ( uiSignEL ? -1 : 1 ) : 0 );
  }

  return Err::m_nOK;
}


// == Nokia, m11509
ErrVal
CabacReader::RQdecodeCycleSymbol( UInt& uiCycle )
{
  UInt itSymbol;
  RNOK( CabaDecoder::getEpSymbol( itSymbol ) );
  // Changed mapping to match syntax in WD (justin.ridge@nokia.com)
  uiCycle = 1 + itSymbol;
  if (itSymbol == 1) {
    RNOK( CabaDecoder::getEpSymbol( itSymbol ) );
    uiCycle += itSymbol;
  }
  // heiko.schwarz@hhi.fhg.de: return added
  return Err::m_nOK;
}
// ==


H264AVC_NAMESPACE_END


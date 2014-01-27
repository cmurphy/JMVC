#include "H264AVCEncoderLib.h"
#include "UvlcWriter.h"
#include "H264AVCCommonLib/Tables.h"
#include "H264AVCCommonLib/TraceFile.h"

#define MAX_VALUE  0xdead
#define TOTRUN_NUM    15
#define RUNBEFORE_NUM  7

// h264 namepace begin
H264AVC_NAMESPACE_BEGIN

const UInt g_auiIncVlc[] = {0,3,6,12,24,48,32768};	// maximum vlc = 6

const UChar g_aucLenTableTZ16[TOTRUN_NUM][16] =
{

  { 1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9},
  { 3,3,3,3,3,4,4,4,4,5,5,6,6,6,6},
  { 4,3,3,3,4,4,3,3,4,5,5,6,5,6},
  { 5,3,4,4,3,3,3,4,3,4,5,5,5},
  { 4,4,4,3,3,3,3,3,4,5,4,5},
  { 6,5,3,3,3,3,3,3,4,3,6},
  { 6,5,3,3,3,2,3,4,3,6},
  { 6,4,5,3,2,2,3,3,6},
  { 6,6,4,2,2,3,2,5},
  { 5,5,3,2,2,2,4},
  { 4,4,3,3,1,3},
  { 4,4,2,1,3},
  { 3,3,1,2},
  { 2,2,1},
  { 1,1},
};

const UChar g_aucCodeTableTZ16[TOTRUN_NUM][16] =
{
  {1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1},
  {7,6,5,4,3,5,4,3,2,3,2,3,2,1,0},
  {5,7,6,5,4,3,4,3,2,3,2,1,1,0},
  {3,7,5,4,6,5,4,3,3,2,2,1,0},
  {5,4,3,7,6,5,4,3,2,1,1,0},
  {1,1,7,6,5,4,3,2,1,1,0},
  {1,1,5,4,3,3,2,1,1,0},
  {1,1,1,3,3,2,2,1,0},
  {1,0,1,3,2,1,1,1,},
  {1,0,1,3,2,1,1,},
  {0,1,1,2,1,3},
  {0,1,1,1,1},
  {0,1,1,1},
  {0,1,1},
  {0,1},
};


const UChar g_aucLenTableTZ4[3][4] =
{
  { 1, 2, 3, 3,},
  { 1, 2, 2, 0,},
  { 1, 1, 0, 0,},
};

const UChar g_aucCodeTableTZ4[3][4] =
{
  { 1, 1, 1, 0,},
  { 1, 1, 0, 0,},
  { 1, 0, 0, 0,},
};

const UChar g_aucLenTable3[7][15] =
{
  {1,1},
  {1,2,2},
  {2,2,2,2},
  {2,2,2,3,3},
  {2,2,3,3,3,3},
  {2,3,3,3,3,3,3},
  {3,3,3,3,3,3,3,4,5,6,7,8,9,10,11},
};

const UChar g_aucCodeTable3[7][15] =
{
  {1,0},
  {1,1,0},
  {3,2,1,0},
  {3,2,1,1,0},
  {3,2,3,2,1,0},
  {3,0,1,3,2,5,4},
  {7,6,5,4,3,2,1,1,1,1,1,1,1,1,1},
};

const UChar g_aucLenTableTO4[4][5] =
{
  { 2, 6, 6, 6, 6,},
  { 0, 1, 6, 7, 8,},
  { 0, 0, 3, 7, 8,},
  { 0, 0, 0, 6, 7,},
};

const UChar g_aucCodeTableTO4[4][5] =
{
  {1,7,4,3,2},
  {0,1,6,3,3},
  {0,0,1,2,2},
  {0,0,0,5,0},
};


const UChar g_aucLenTableTO16[3][4][17] =
{
  {   // 0702
    { 1, 6, 8, 9,10,11,13,13,13,14,14,15,15,16,16,16,16},
    { 0, 2, 6, 8, 9,10,11,13,13,14,14,15,15,15,16,16,16},
    { 0, 0, 3, 7, 8, 9,10,11,13,13,14,14,15,15,16,16,16},
    { 0, 0, 0, 5, 6, 7, 8, 9,10,11,13,14,14,15,15,16,16},
  },
  {
    { 2, 6, 6, 7, 8, 8, 9,11,11,12,12,12,13,13,13,14,14},
    { 0, 2, 5, 6, 6, 7, 8, 9,11,11,12,12,13,13,14,14,14},
    { 0, 0, 3, 6, 6, 7, 8, 9,11,11,12,12,13,13,13,14,14},
    { 0, 0, 0, 4, 4, 5, 6, 6, 7, 9,11,11,12,13,13,13,14},
  },
  {
    { 4, 6, 6, 6, 7, 7, 7, 7, 8, 8, 9, 9, 9,10,10,10,10},
    { 0, 4, 5, 5, 5, 5, 6, 6, 7, 8, 8, 9, 9, 9,10,10,10},
    { 0, 0, 4, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,10},
    { 0, 0, 0, 4, 4, 4, 4, 4, 5, 6, 7, 8, 8, 9,10,10,10},
  },
};

const UChar g_aucCodeTableTO16[3][4][17] =
{
  {
    { 1, 5, 7, 7, 7, 7,15,11, 8,15,11,15,11,15,11, 7,4},
    { 0, 1, 4, 6, 6, 6, 6,14,10,14,10,14,10, 1,14,10,6},
    { 0, 0, 1, 5, 5, 5, 5, 5,13, 9,13, 9,13, 9,13, 9,5},
    { 0, 0, 0, 3, 3, 4, 4, 4, 4, 4,12,12, 8,12, 8,12,8},
  },
  {
    { 3,11, 7, 7, 7, 4, 7,15,11,15,11, 8,15,11, 7, 9,7},
    { 0, 2, 7,10, 6, 6, 6, 6,14,10,14,10,14,10,11, 8,6},
    { 0, 0, 3, 9, 5, 5, 5, 5,13, 9,13, 9,13, 9, 6,10,5},
    { 0, 0, 0, 5, 4, 6, 8, 4, 4, 4,12, 8,12,12, 8, 1,4},
  },
  {
    {15,15,11, 8,15,11, 9, 8,15,11,15,11, 8,13, 9, 5,1},
    { 0,14,15,12,10, 8,14,10,14,14,10,14,10, 7,12, 8,4},
    { 0, 0,13,14,11, 9,13, 9,13,10,13, 9,13, 9,11, 7,3},
    { 0, 0, 0,12,11,10, 9, 8,13,12,12,12, 8,12,10, 6,2},
  },
};


const UChar COEFF_COST[16] =
{
  3, 2,2,1, 1,1,0,0,0,0,0,0,0,0,0,0
};

const UChar COEFF_COST8x8[64] =
{
  3,3,3,3,2,2,2,2,2,2,2,2,1,1,1,1,
  1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const UChar g_aucCbpIntra[48] =
{
   3, 29, 30, 17,
  31, 18, 37,  8,
  32, 38, 19,  9,
  20, 10, 11,  2,
  16, 33, 34, 21,
  35, 22, 39,  4,
  36, 40, 23,  5,
  24,  6,  7,  1,
  41, 42, 43, 25,
  44, 26, 46, 12,
  45, 47, 27, 13,
  28, 14, 15,  0
};


const UChar g_aucCbpInter[48] =
{
   0,  2,  3,  7,
   4,  8, 17, 13,
   5, 18,  9, 14,
  10, 15, 16, 11,
   1, 32, 33, 36,
  34, 37, 44, 40,
  35, 45, 38, 41,
  39, 42, 43, 19,
   6, 24, 25, 20,
  26, 21, 46, 28,
  27, 47, 22, 29,
  23, 30, 31, 12
};

const UInt g_auiISymCode[3][16] =
{ {  0,  1,  2,  3,  4,  5,  6,  7                                },
  {  0,  4,  5, 28,  6, 29, 30, 31                                },
  {  0,  4,  5, 28, 12, 29, 60,252, 13, 61,124,253,125,254,510,511}
};
const UInt g_auiISymLen[3][16] =
{ { 3, 3, 3, 3, 3, 3, 3, 3                        },
  { 1, 3, 3, 5, 3, 5, 5, 5                        },
  { 1, 3, 3, 5, 4, 5, 6, 8, 4, 6, 7, 8, 7, 8, 9, 9}
};

#define CAVLC_SYMGRP_SIZE       3 

const UInt g_auiRefSymCode[2][27] =
{
  { 
    0x1, 0x3, 0x5, 0x3, 0x5, 0x5, 0x4, 0x5, 0x5, 
    0x2, 0x4, 0x4, 0x3, 0x4, 0x3, 0x4, 0x2, 0x3, 
    0x3, 0x3, 0x3, 0x3, 0x1, 0x2, 0x2, 0x1, 0x0
  },
  {
    0x1, 0x7, 0x6, 0x7,	0x9, 0x8, 0x6, 0x7,	0x9,
    0x5, 0x6,	0x5, 0x8,	0x7, 0x6,	0x7, 0x5,	0x4,
    0x4, 0x4,	0x6, 0x5,	0x3, 0x2,	0x5, 0x1,	0x0
  }
};

const UInt g_auiRefSymLen[2][27] =
{
  {
    1, 4, 5, 3, 6, 8, 5, 7, 9,
    3, 6, 8, 6, 9,10, 7,10,12,
    5, 7, 9, 8,10,12, 9,12,12
  },
  {
    1, 5, 5, 4, 7, 7, 4, 7, 6,
    4, 7, 7, 6, 8, 8, 6, 8, 8,
    4, 7, 6, 6, 8, 8, 5, 8, 8
  }
};

UvlcWriter::UvlcWriter( Bool bTraceEnable ) :
  m_pcBitWriteBufferIf( NULL ),
  m_uiBitCounter( 0 ),
  m_uiPosCounter( 0 ),
  m_uiCoeffCost( 0 ),
  m_bTraceEnable( bTraceEnable ),
  m_bRunLengthCoding( false ),
  m_uiRun( 0 )
{
  m_pSymGrp    = new UcSymGrpWriter( this );
}


UvlcWriter::~UvlcWriter()
{
  delete m_pSymGrp; 
}


ErrVal UvlcWriter::create( UvlcWriter*& rpcUvlcWriter, Bool bTraceEnable )
{
  rpcUvlcWriter = new UvlcWriter( bTraceEnable );

  ROT( NULL == rpcUvlcWriter );

  return Err::m_nOK;
}


ErrVal UvlcWriter::destroy()
{
  delete this;
  return Err::m_nOK;
}

__inline ErrVal UvlcWriter::xWriteCode( UInt uiCode, UInt uiLength )
{
  AOT_DBG(uiLength<1);

  ErrVal retVal = m_pcBitWriteBufferIf->write( uiCode, uiLength );

  ETRACE_TY( " u(v)" );
  ETRACE_BITS( uiCode, uiLength );
  ETRACE_POS;

  ETRACE_CODE( uiCode );
  ETRACE_COUNT (uiLength);
  return retVal;
}


__inline ErrVal UvlcWriter::xWriteFlag( UInt uiCode )
{
  ErrVal retVal = m_pcBitWriteBufferIf->write( uiCode, 1 );

  ETRACE_TY( " u(1)" );
  ETRACE_BITS( uiCode, 1 );
  ETRACE_POS;

  ETRACE_CODE( uiCode );
  ETRACE_COUNT (1);
  return retVal;
}

ErrVal UvlcWriter::init(  BitWriteBufferIf* pcBitWriteBufferIf )
{
  ROT( NULL == pcBitWriteBufferIf );

  m_pcBitWriteBufferIf= pcBitWriteBufferIf;
  m_bRunLengthCoding  = false;
  m_uiRun             = 0;

  return Err::m_nOK;
}

ErrVal UvlcWriter::uninit()
{
  m_pcBitWriteBufferIf = NULL;
  return Err::m_nOK;
}

UInt UvlcWriter::getNumberOfWrittenBits()
{
  return m_pcBitWriteBufferIf->getNumberOfWrittenBits();
}


ErrVal UvlcWriter::startSlice( const SliceHeader& rcSliceHeader )
{
  m_bRunLengthCoding  = ! rcSliceHeader.isIntra();
  m_uiRun             = 0;
  return Err::m_nOK;
}

ErrVal UvlcWriter::startFragment() //JVT-P031
{
    RNOK( m_pcBitWriteBufferIf->writeAlignOne() ); //FIX_FRAG_CAVLC
    return Err::m_nOK;
}
//FIX_FRAG_CAVLC
ErrVal UvlcWriter::getLastByte(UChar &uiLastByte, UInt &uiLastBitPos)
{
  RNOK(m_pcBitWriteBufferIf->getLastByte(uiLastByte, uiLastBitPos));
  return Err::m_nOK;
}
ErrVal UvlcWriter::setFirstBits(UChar ucByte,UInt uiLastBitPos)
{
  RNOK( m_pcBitWriteBufferIf->write(ucByte,uiLastBitPos));
  return Err::m_nOK;
}
//~FIX_FRAG_CAVLC
ErrVal UvlcWriter::xWriteUvlcCode( UInt uiVal)
{
  UInt uiLength = 1;
  UInt uiTemp = ++uiVal;

  AOF_DBG( uiTemp );

  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  RNOK( m_pcBitWriteBufferIf->write( uiVal, uiLength ) );

  ETRACE_TY( "ue(v)" );
  ETRACE_BITS( uiVal, uiLength );
  ETRACE_POS;

  ETRACE_DO( uiVal-- );

  ETRACE_CODE( uiVal );
  ETRACE_COUNT (uiLength);

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteSvlcCode( Int iVal)
{
  UInt uiVal = xConvertToUInt( iVal );
  UInt uiLength = 1;
  UInt uiTemp = ++uiVal;

  AOF_DBG( uiTemp );

  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  RNOK( m_pcBitWriteBufferIf->write( uiVal, uiLength ) );

  ETRACE_TY( "ue(v)" );
  ETRACE_BITS( uiVal, uiLength );
  ETRACE_POS;

  ETRACE_CODE( iVal );
  ETRACE_COUNT (uiLength);

  return Err::m_nOK;
}

ErrVal UvlcWriter::writeUvlc( UInt uiCode, const Char* pcTraceString )
{
  ETRACE_TH( pcTraceString );

  RNOK( xWriteUvlcCode( uiCode ) );

  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::writeSvlc( Int iCode, const Char* pcTraceString )
{
  UInt uiCode;

  ETRACE_TH( pcTraceString );

  uiCode = xConvertToUInt( iCode );
  RNOK( xWriteUvlcCode( uiCode ) );

  ETRACE_TY( "se(v)" );
  ETRACE_CODE( iCode );

  ETRACE_N;
  return Err::m_nOK;
}

ErrVal UvlcWriter::writeFlag( Bool bFlag, const Char* pcTraceString )
{
  ETRACE_TH( pcTraceString );
  ETRACE_TY( " u(1)" );

  RNOK( xWriteFlag( bFlag ? 1:0) );

  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::writeSCode( Int iCode, UInt uiLength, const Char* pcTraceString )
{
  AOT_DBG(uiLength<1);
  ETRACE_TH( pcTraceString );
  ETRACE_TY( " i(v)" );

  UInt uiShift = 32 - uiLength;
  UInt uiCode = ((UInt)(iCode << uiShift)) >> uiShift;
  RNOK( m_pcBitWriteBufferIf->write( uiCode, uiLength ) );

  ETRACE_POS;
  ETRACE_CODE (iCode);
  ETRACE_BITS (uiCode, uiLength );
  ETRACE_COUNT(uiLength);
  ETRACE_N;
  return Err::m_nOK;
}

ErrVal UvlcWriter::writeCode( UInt uiCode, UInt uiLength, const Char* pcTraceString )
{
  AOT_DBG(uiLength<1);
  ETRACE_TH( pcTraceString );
  ETRACE_TY( " u(v)" );

  RNOK( m_pcBitWriteBufferIf->write( uiCode, uiLength ) );

  ETRACE_POS;
  ETRACE_CODE (uiCode);
  ETRACE_BITS (uiCode, uiLength );
  ETRACE_COUNT(uiLength);
  ETRACE_N;
  return Err::m_nOK;
}



ErrVal UvlcWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx();
  RNOK( xWriteRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), --uiRefFrame ) )
  return Err::m_nOK;
}

ErrVal UvlcWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx( eParIdx );
  RNOK( xWriteRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), --uiRefFrame ) )
  return Err::m_nOK;
}

ErrVal UvlcWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx( eParIdx );
  RNOK( xWriteRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), --uiRefFrame ) )
  return Err::m_nOK;
}

ErrVal UvlcWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8 eParIdx  )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx( eParIdx );
  RNOK( xWriteRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), --uiRefFrame ) )
  return Err::m_nOK;
}


ErrVal  UvlcWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  return xWriteMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag() );
}
ErrVal  UvlcWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx   )
{
  return xWriteMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag( eParIdx ) );
}
ErrVal  UvlcWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx   )
{
  return xWriteMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag( eParIdx ) );
}
ErrVal  UvlcWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8 eParIdx   )
{
  return xWriteMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag( eParIdx ) );
}



ErrVal UvlcWriter::blockModes( MbDataAccess& rcMbDataAccess )
{
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    ETRACE_T( "BlockMode" );

    UInt uiBlockMode = rcMbDataAccess.getConvertBlkMode( c8x8Idx.b8x8Index() );

    AOT_DBG( uiBlockMode > 12);

    RNOK( xWriteUvlcCode( uiBlockMode ) );

    ETRACE_N;
  }
  return Err::m_nOK;
}

ErrVal UvlcWriter::fieldFlag( MbDataAccess& rcMbDataAccess )//th
{
    ETRACE_T( "MbFieldFlag" );

    UInt uiBit = rcMbDataAccess.getMbData().getFieldFlag() ? 1 : 0;

    RNOK( xWriteFlag( uiBit ) );

    ETRACE_N;

    return Err::m_nOK;
}

ErrVal UvlcWriter::skipFlag( MbDataAccess& rcMbDataAccess, Bool bNotAllowed )
{
  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 0 );

  ROFRS( m_bRunLengthCoding, Err::m_nOK );

  if( ! bNotAllowed && rcMbDataAccess.isSkippedMb() )
  {
    m_uiRun++;
  }
  else
  {
    ETRACE_T( "Run" );
    RNOK( xWriteUvlcCode( m_uiRun ) );
    ETRACE_N;

    m_uiRun = 0;
  }

  rcMbDataAccess.getMbData().setSkipFlag( m_uiRun > 0 );

  return Err::m_nOK;
}


ErrVal UvlcWriter::BLSkipFlag( MbDataAccess& rcMbDataAccess )
{
  UInt  uiCode = ( rcMbDataAccess.getMbData().getBLSkipFlag() ? 1 : 0 );

  ETRACE_T( "BLSkipFlag" );
  RNOK( xWriteFlag( uiCode ) );
  ETRACE_N;

  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 0 );

  return Err::m_nOK;
}


ErrVal UvlcWriter::mbMode( MbDataAccess& rcMbDataAccess )
{
  UInt uiMbMode = rcMbDataAccess.getConvertMbType( );

  if( m_bRunLengthCoding )
  {
    uiMbMode--;
  }
  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 0 );
	 
	ETRACE_T( "MbMode" );
  RNOK( xWriteUvlcCode( uiMbMode ) );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcWriter::resPredFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiCode = ( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) ? 1 : 0 );

  ETRACE_T( "ResidualPredFlag" );
  RNOK( xWriteFlag( uiCode ) );
  ETRACE_N;

  return Err::m_nOK;
}

ErrVal UvlcWriter::resPredFlag_FGS( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff )
{
  UInt uiCode = ( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) ? 1 : 0 );

  ETRACE_T( "ResidualPredFlag" );
  RNOK( xWriteFlag( uiCode ) );
  ETRACE_N;

  return Err::m_nOK;
}


//-- JVT-R091
ErrVal UvlcWriter::smoothedRefFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiCode = ( rcMbDataAccess.getMbData().getSmoothedRefFlag() ? 1 : 0 );

  ETRACE_T( "SmoothedRefFlag" );
  RNOK( xWriteFlag( uiCode ) );
  ETRACE_N;

  return Err::m_nOK;
}
//--

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv();
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx, eSParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx, eSParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx, eSParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteMvd( Mv cMv )
{
  ETRACE_T( "Mvd: x" );

  UInt  uiTemp;
  Short sHor   = cMv.getHor();
  Short sVer   = cMv.getVer();

  uiTemp = xConvertToUInt( sHor );
  RNOK( xWriteUvlcCode( uiTemp ) );

  ETRACE_CODE( sHor );
  ETRACE_TY("se(v)");
  ETRACE_N;

  ETRACE_T( "Mvd: y" );

  uiTemp = xConvertToUInt( sVer );
  RNOK( xWriteUvlcCode( uiTemp ) );

  ETRACE_CODE( sVer );
  ETRACE_TY("se(v)");
  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::intraPredModeChroma( MbDataAccess& rcMbDataAccess )
{
  ETRACE_T( "IntraPredModeChroma" );

  AOT_DBG( 4 < rcMbDataAccess.getMbData().getChromaPredMode() );
  RNOK( xWriteUvlcCode( rcMbDataAccess.getMbData().getChromaPredMode() ) );

  ETRACE_N;

  return Err::m_nOK;
}

ErrVal UvlcWriter::intraPredModeLuma( MbDataAccess& rcMbDataAccess, LumaIdx cIdx )
{
  ETRACE_T( "IntraPredModeLuma" );
  ETRACE_POS;

  Int iIntraPredModeLuma = rcMbDataAccess.encodeIntraPredMode(cIdx);
  ROT( iIntraPredModeLuma > 7);

  UInt uiBits = (iIntraPredModeLuma < 0) ? 1 : 0;

  RNOK( m_pcBitWriteBufferIf->write( uiBits, 1 ) );
  ETRACE_BITS( uiBits,1 );
  ETRACE_DO( m_uiBitCounter = 1 );

  if( ! uiBits )
  {
    RNOK( m_pcBitWriteBufferIf->write( iIntraPredModeLuma, 3 ) );
    ETRACE_BITS( iIntraPredModeLuma, 3 );
    ETRACE_DO( m_uiBitCounter = 4 );
  }

  ETRACE_COUNT(m_uiBitCounter);
  ETRACE_CODE(iIntraPredModeLuma);
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcWriter::cbp( MbDataAccess& rcMbDataAccess )
{
  UInt uiCbp = rcMbDataAccess.getMbData().getMbCbp();
  ETRACE_T( "Cbp: " );
  ETRACE_X ( uiCbp );

  AOT_DBG( 48 < uiCbp );

  Bool bIntra = ( !rcMbDataAccess.getMbData().getBLSkipFlag() && rcMbDataAccess.getMbData().isIntra() );
  UInt uiTemp = ( bIntra ? g_aucCbpIntra[uiCbp]: g_aucCbpInter[uiCbp] );

  RNOK( xWriteUvlcCode( uiTemp ) );

  ETRACE_N;

  return Err::m_nOK;
}



const UChar g_aucTcoeffCDc[3][2]=
{
  {0,0},
  {2,6},
  {4,1}
};
const UChar g_aucRunCDc[4]=
{
  2,1,0,0
};



const UChar g_aucRunSScan[16]=
{
  4,2,2,1,1,1,1,1,1,1,0,0,0,0,0,0
};
const UChar g_aucTcoeffSScan[4][10]=
{
  { 1, 3, 5, 9,11,13,21,23,25,27},
  { 7,17,19, 0, 0, 0, 0, 0, 0, 0},
  {15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {29, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};




const UChar g_aucRunDScan[8]=
{
  9,3,1,1,1,0,0,0
};
const UChar g_aucTcoeffDScan[9][5] =
{
  { 1, 3, 7,15,17},
  { 5,19, 0, 0, 0},
  { 9,21, 0, 0, 0},
  {11, 0, 0, 0, 0},
  {13, 0, 0, 0, 0},
  {23, 0, 0, 0, 0},
  {25, 0, 0, 0, 0},
  {27, 0, 0, 0, 0},
  {29, 0, 0, 0, 0},
};



ErrVal UvlcWriter::residualBlock( MbDataAccess& rcMbDataAccess,
                                  LumaIdx       cIdx,
                                  ResidualMode  eResidualMode )
{
  const UChar*  pucScan;
  const TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );

  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());

  Int   iLevel;
  Int   iRun      = 0;
  UInt  uiPos     = 0;
  UInt  uiMaxPos  = 16;

  switch( eResidualMode )
  {
  case LUMA_I16_DC:
    {
      pucScan = (bFrame) ? g_aucLumaFrameDCScan : g_aucLumaFieldDCScan;
      uiMaxPos = 16;
      break;
    }
  case LUMA_I16_AC:
    {
      pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
      uiPos=1;
      break;
    }
  case LUMA_SCAN:
    {
	  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
      break;
    }
  default:
    return Err::m_nERR;
  }

  Int   aiLevelRun[32];
  UInt  uiTrailingOnes = 0;
  UInt  uiTotalRun     = 0;
  UInt  uiCoeffCnt     = 0;

  while( uiPos < uiMaxPos )
  {
    if( ( iLevel = piCoeff[ pucScan [ uiPos++ ] ]) )
    {
      if( abs(iLevel) == 1 )
      {
        m_uiCoeffCost += COEFF_COST[iRun];
        uiTrailingOnes++;
      }
      else
      {
        m_uiCoeffCost += MAX_VALUE;                // set high cost, shall not be discarded
        uiTrailingOnes = 0;
      }

      aiLevelRun[uiCoeffCnt]      = iLevel;
      aiLevelRun[uiCoeffCnt+0x10] = iRun;
      uiTotalRun += iRun;
      uiCoeffCnt++;
      iRun = 0;
    }
    else
    {
      iRun++;
    }
  }

  if( uiTrailingOnes > 3 )
  {
    uiTrailingOnes = 3;
  }


  switch( eResidualMode )
  {
  case LUMA_I16_DC:
    {
      ETRACE_T( "Luma:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 16, uiTotalRun );
      break;
    }
  case LUMA_I16_AC:
    {
      ETRACE_T( "Luma:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 15, uiTotalRun );
      break;
    }
  case LUMA_SCAN:
    {
      ETRACE_T( "Luma:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 16, uiTotalRun );
      break;
    }
  default:
    {
      AF();
    }
  }

  return Err::m_nOK;
}



ErrVal UvlcWriter::residualBlock( MbDataAccess& rcMbDataAccess,
                                  ChromaIdx     cIdx,
                                  ResidualMode  eResidualMode )
{
  const TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan;
  Int           iRun = 0, iLevel;
  UInt          uiPos, uiMaxPos;

  switch( eResidualMode )
  {
  case CHROMA_DC:
    {
      pucScan = g_aucIndexChromaDCScan;
      uiPos=0;  uiMaxPos= 4;
      break;
    }
  case CHROMA_AC:
    {
      pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
      uiPos=1;  uiMaxPos=16;
      break;
    }
  default:
    return Err::m_nERR;
  }

  Int aiLevelRun[32];

  UInt uiTrailingOnes = 0;
  UInt uiTotalRun     = 0;
  UInt uiCoeffCnt     = 0;

  while( uiPos < uiMaxPos )
  {
    if( ( iLevel = piCoeff[ pucScan [ uiPos++ ] ]) )
    {
      if( abs(iLevel) == 1 )
      {
        m_uiCoeffCost += COEFF_COST[iRun];
        uiTrailingOnes++;
      }
      else
      {
        m_uiCoeffCost += MAX_VALUE;                // set high cost, shall not be discarded
        uiTrailingOnes = 0;
      }

      aiLevelRun[uiCoeffCnt]      = iLevel;
      aiLevelRun[uiCoeffCnt+0x10] = iRun;
      uiTotalRun += iRun;
      uiCoeffCnt++;
      iRun = 0;
    }
    else
    {
      iRun++;
    }
  }

  if( uiTrailingOnes > 3 )
  {
    uiTrailingOnes = 3;
  }


  switch( eResidualMode )
  {
  case CHROMA_AC:
    {
      ETRACE_T( "CHROMA_AC:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 15, uiTotalRun );
      break;
    }
  case CHROMA_DC:
    {
      ETRACE_T( "CHROMA_DC:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xWriteTrailingOnes4( uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 4, uiTotalRun );
      break;
    }
  default:
    {
      AF();
    }
  }

  return Err::m_nOK;
}


ErrVal UvlcWriter::deltaQp( MbDataAccess& rcMbDataAccess )
{
  ETRACE_T ("DQp");

  RNOK( xWriteSvlcCode( rcMbDataAccess.getDeltaQp() ) );

  ETRACE_TY ("se(v)");
  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::finishSlice()
{
  if( m_bRunLengthCoding && m_uiRun )
  {
    ETRACE_T( "Run" );
    RNOK( xWriteUvlcCode( m_uiRun ) );
    ETRACE_N;
  }

  return Err::m_nOK;
}





ErrVal UvlcWriter::xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes )
{
  UInt uiCoeffCountCtx = rcMbDataAccess.getCtxCoeffCount( cIdx );

  xWriteTrailingOnes16( uiCoeffCountCtx, uiCoeffCount, uiTrailingOnes );

  rcMbDataAccess.getMbTCoeffs().setCoeffCount( cIdx, uiCoeffCount );

  return Err::m_nOK;
}

ErrVal UvlcWriter::xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes )
{
  UInt uiCoeffCountCtx = rcMbDataAccess.getCtxCoeffCount( cIdx );

  xWriteTrailingOnes16( uiCoeffCountCtx, uiCoeffCount, uiTrailingOnes );

  rcMbDataAccess.getMbTCoeffs().setCoeffCount( cIdx, uiCoeffCount );

  return Err::m_nOK;
}

ErrVal UvlcWriter::xWriteRunLevel( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt uiTotalRun )
{

  ROTRS( 0 == uiCoeffCnt, Err::m_nOK );

  if( uiTrailingOnes )
  {
    UInt uiBits = 0;
    Int n = uiTrailingOnes-1;
    for( UInt k = uiCoeffCnt; k > uiCoeffCnt-uiTrailingOnes; k--, n--)
    {
      if( aiLevelRun[k-1] < 0)
      {
        uiBits |= 1<<n;
      }
    }

    RNOK( m_pcBitWriteBufferIf->write( uiBits, uiTrailingOnes ))
    ETRACE_POS;
    ETRACE_T( "  TrailingOnesSigns: " );
    ETRACE_V( uiBits );
    ETRACE_N;
    ETRACE_COUNT(uiTrailingOnes);
  }


  Int iHighLevel = ( uiCoeffCnt > 3 && uiTrailingOnes == 3) ? 0 : 1;
  Int iVlcTable  = ( uiCoeffCnt > 10 && uiTrailingOnes < 3) ? 1 : 0;

  for( Int k = uiCoeffCnt - 1 - uiTrailingOnes; k >= 0; k--)
  {
    Int iLevel;
    iLevel = aiLevelRun[k];

    UInt uiAbsLevel = (UInt)abs(iLevel);

    if( iHighLevel )
    {
      iLevel -= ( iLevel > 0 ) ? 1 : -1;
	    iHighLevel = 0;
    }

    if( iVlcTable == 0 )
    {
	    xWriteLevelVLC0( iLevel );
    }
    else
    {
	    xWriteLevelVLCN( iLevel, iVlcTable );
    }

    // update VLC table
    if( uiAbsLevel > g_auiIncVlc[ iVlcTable ] )
    {
      iVlcTable++;
    }

    if( k == Int(uiCoeffCnt - 1 - uiTrailingOnes) && uiAbsLevel > 3)
    {
      iVlcTable = 2;
    }

  }

  ROFRS( uiCoeffCnt < uiMaxCoeffs, Err::m_nOK );


  iVlcTable = uiCoeffCnt-1;
  if( uiMaxCoeffs <= 4 )
  {
    xWriteTotalRun4( iVlcTable, uiTotalRun );
  }
  else
  {
    xWriteTotalRun16( iVlcTable, uiTotalRun );
  }

  // decode run before each coefficient
  uiCoeffCnt--;
  if( uiTotalRun > 0 && uiCoeffCnt > 0)
  {
    do
    {
      iVlcTable = (( uiTotalRun > RUNBEFORE_NUM) ? RUNBEFORE_NUM : uiTotalRun) - 1;
      UInt uiRun = aiLevelRun[uiCoeffCnt+0x10];

      xWriteRun( iVlcTable, uiRun );

      uiTotalRun -= uiRun;
      uiCoeffCnt--;
    } while( uiTotalRun != 0 && uiCoeffCnt != 0);
  }

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteTrailingOnes16( UInt uiLastCoeffCount, UInt uiCoeffCount, UInt uiTrailingOnes )
{
  UInt uiVal;
  UInt uiSize;

  ETRACE_POS;
  if( 3 == uiLastCoeffCount )
  {
    UInt uiBits = 3;
    if( uiCoeffCount )
    {
      uiBits = (uiCoeffCount-1)<<2 | uiTrailingOnes;
    }
    RNOK( m_pcBitWriteBufferIf->write( uiBits, 6) );
    ETRACE_DO( m_uiBitCounter = 6 );

    uiVal = uiBits;
    uiSize = 6;
  }
  else
  {
    RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount],
                                  g_aucLenTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount] ) );
    ETRACE_DO( m_uiBitCounter = g_aucLenTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount] );

    uiVal = g_aucCodeTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount];
    uiSize = g_aucLenTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount];
  }

  ETRACE_T( "  TrailingOnes16: Vlc: " );
  ETRACE_V( uiLastCoeffCount );
  ETRACE_T( " CoeffCnt: " );
  ETRACE_V( uiCoeffCount );
  ETRACE_T( " TraiOnes: " );
  ETRACE_V( uiTrailingOnes );
  ETRACE_N;
  ETRACE_COUNT(m_uiBitCounter);

  return Err::m_nOK;
}



ErrVal UvlcWriter::xWriteTrailingOnes4( UInt uiCoeffCount, UInt uiTrailingOnes )
{
  RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTableTO4[uiTrailingOnes][uiCoeffCount],
                                g_aucLenTableTO4[uiTrailingOnes][uiCoeffCount] ) );

  ETRACE_POS;
  ETRACE_T( "  TrailingOnes4: CoeffCnt: " );
  ETRACE_V( uiCoeffCount );
  ETRACE_T( " TraiOnes: " );
  ETRACE_V( uiTrailingOnes );
  ETRACE_N;
  ETRACE_COUNT(g_aucLenTableTO4[uiTrailingOnes][uiCoeffCount]);

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteTotalRun4( UInt uiVlcPos, UInt uiTotalRun )
{
  RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTableTZ4[uiVlcPos][uiTotalRun],
                                g_aucLenTableTZ4[uiVlcPos][uiTotalRun] ) );

  ETRACE_POS;
  ETRACE_T( "  TotalZeros4 vlc: " );
  ETRACE_V( uiVlcPos );
  ETRACE_T( " TotalRun: " );
  ETRACE_V( uiTotalRun );
  ETRACE_N;
  ETRACE_COUNT(g_aucLenTableTZ4[uiVlcPos][uiTotalRun]);

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteTotalRun16( UInt uiVlcPos, UInt uiTotalRun )
{
  RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTableTZ16[uiVlcPos][uiTotalRun],
                                g_aucLenTableTZ16[uiVlcPos][uiTotalRun] ) );

  ETRACE_POS;
  ETRACE_T( "  TotalRun16 vlc: " );
  ETRACE_V( uiVlcPos );
  ETRACE_T( " TotalRun: " );
  ETRACE_V( uiTotalRun );
  ETRACE_N;
  ETRACE_COUNT(g_aucLenTableTZ16[uiVlcPos][uiTotalRun]);

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteRun( UInt uiVlcPos, UInt uiRun  )
{
  RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTable3[uiVlcPos][uiRun],
                                g_aucLenTable3[uiVlcPos][uiRun] ) );

  ETRACE_POS;
  ETRACE_T( "  Run" );
  ETRACE_CODE( uiRun );
  ETRACE_COUNT (g_aucLenTable3[uiVlcPos][uiRun]);
  ETRACE_N;

  return Err::m_nOK;
}




ErrVal UvlcWriter::xWriteLevelVLC0( Int iLevel )
{

  UInt uiLength;
  UInt uiLevel = abs( iLevel );
  UInt uiSign = ((UInt)iLevel)>>31;

  UInt uiBits;

  if( 8 > uiLevel )
  {
    uiBits   = 1;
    uiLength = 2 * uiLevel - 1 + uiSign;
  }
  else if( 16 > uiLevel )
  {
    uiBits   = 2*uiLevel + uiSign;
    uiLength = 15 + 4;
  }
  else
  {
    uiBits   = 0x1000-32 + (uiLevel<<1) + uiSign;
    uiLength = 16 + 12;
  }


  RNOK( m_pcBitWriteBufferIf->write( uiBits, uiLength ) );

  ETRACE_POS;
  ETRACE_T( "  VLC0 lev " );
  ETRACE_CODE( iLevel );
  ETRACE_N;
  ETRACE_COUNT( uiLength );

  return Err::m_nOK;

}

ErrVal UvlcWriter::xWriteLevelVLCN( Int iLevel, UInt uiVlcLength )
{
  UInt uiLength;
  UInt uiLevel = abs( iLevel );
  UInt uiSign = ((UInt)iLevel)>>31;
  UInt uiBits;

  UInt uiShift = uiVlcLength-1;
  UInt uiEscapeCode = (0xf<<uiShift)+1;

  if( uiLevel < uiEscapeCode )
  {
    uiLevel--;
	  uiLength = (uiLevel>>uiShift) + uiVlcLength + 1;
    uiLevel &= ~((0xffffffff)<<uiShift);
	  uiBits   = (2<<uiShift) | 2*uiLevel | uiSign;
  }
  else
  {
	  uiLength = 28;
	  uiBits   = 0x1000 + 2*(uiLevel-uiEscapeCode) + uiSign;
  }



  RNOK( m_pcBitWriteBufferIf->write( uiBits, uiLength ) );

  ETRACE_POS;
  ETRACE_T( "  VLCN lev: " );
  ETRACE_CODE( iLevel );
  ETRACE_N;
  ETRACE_COUNT( uiLength );

  return Err::m_nOK;
}



ErrVal UvlcWriter::samplesPCM( MbDataAccess& rcMbDataAccess )
{
  ETRACE_POS;
  ETRACE_T( "  PCM SAMPLES: " );

  RNOK( m_pcBitWriteBufferIf->writeAlignZero() );

  AOF_DBG( rcMbDataAccess.getMbData().isPCM() );

  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 16 );
  Pel* pSrc = rcMbDataAccess.getMbTCoeffs().getPelBuffer();

  const UInt uiFactor = 8*8;
  const UInt uiSize   = uiFactor*2*3;
  RNOK( m_pcBitWriteBufferIf->samples( pSrc, uiSize ) );

  ETRACE_N;
  ETRACE_COUNT( uiFactor*6 );

  return Err::m_nOK;
}



ErrVal UvlcWriter::xWriteRefFrame( Bool bWriteBit, UInt uiRefFrame )
{
  ETRACE_T( "RefFrame" );

  if( bWriteBit )
  {
    RNOK( xWriteFlag( 1-uiRefFrame ) );
  }
  else
  {
    RNOK( xWriteUvlcCode( uiRefFrame ) );
  }

  ETRACE_V( uiRefFrame+1 );
  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteMotionPredFlag( Bool bFlag )
{
  ETRACE_T( "MotionPredFlag" );

  UInt  uiCode = ( bFlag ? 1 : 0 );
  RNOK( xWriteFlag( uiCode) );

  ETRACE_V( uiCode );
  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::transformSize8x8Flag( MbDataAccess& rcMbDataAccess ) 
{
  ETRACE_T( "transformSize8x8Flag:" );

  UInt  uiCode = rcMbDataAccess.getMbData().isTransformSize8x8() ? 1 : 0;
  RNOK( xWriteFlag( uiCode) );

  ETRACE_V( uiCode );
  ETRACE_N;
  return Err::m_nOK;
}





ErrVal UvlcWriter::residualBlock8x8( MbDataAccess&  rcMbDataAccess,
                                     B8x8Idx        c8x8Idx,
                                     ResidualMode   eResidualMode )
{
  ROF( eResidualMode == LUMA_SCAN );

  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;

  const TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get8x8( c8x8Idx );

  UInt  uiBlk;
  Int   iLevel;
  Int   iOverallRun = 0;
  UInt  uiPos       = 0;
  UInt  uiMaxPos    = 64;

  Int   aaiLevelRun     [4][32];
  Int   aiRun           [4]     = { 0, 0, 0, 0 };
  UInt  auiTrailingOnes [4]     = { 0, 0, 0, 0 };
  UInt  auiTotalRun     [4]     = { 0, 0, 0, 0 };
  UInt  auiCoeffCnt     [4]     = { 0, 0, 0, 0 };

  while( uiPos < uiMaxPos )
  {
    uiBlk = ( uiPos % 4 );

    if( ( iLevel = piCoeff[ pucScan[ uiPos++ ] ] ) )
    {
      if( abs(iLevel) == 1 )
      {
        m_uiCoeffCost         += COEFF_COST8x8[ iOverallRun ];
        auiTrailingOnes[uiBlk]++;
      }
      else
      {
        m_uiCoeffCost         += MAX_VALUE;
        auiTrailingOnes[uiBlk] = 0;
      }

      aaiLevelRun[uiBlk][auiCoeffCnt[uiBlk]]      = iLevel;
      aaiLevelRun[uiBlk][auiCoeffCnt[uiBlk]+0x10] = aiRun[uiBlk];
      auiTotalRun[uiBlk]  += aiRun[uiBlk];
      auiCoeffCnt[uiBlk]  ++;
      aiRun      [uiBlk]  = 0;
      iOverallRun         = 0;
    }
    else
    {
      aiRun[uiBlk]++;
      iOverallRun ++;
    }
  }


  //===== loop over 4x4 blocks =====
  for( uiBlk = 0; uiBlk < 4; uiBlk++ )
  {
    if( auiTrailingOnes[uiBlk] > 3 )
    {
      auiTrailingOnes[uiBlk] = 3;
    }
    B4x4Idx cIdx( c8x8Idx.b4x4() + 4*(uiBlk/2) + (uiBlk%2) );

    xPredictNonZeroCnt( rcMbDataAccess, cIdx, auiCoeffCnt[uiBlk], auiTrailingOnes[uiBlk] );
    xWriteRunLevel    ( aaiLevelRun[uiBlk],   auiCoeffCnt[uiBlk], auiTrailingOnes[uiBlk], 16, auiTotalRun[uiBlk] );
  }

  return Err::m_nOK;
}


Bool
UvlcWriter::RQencodeCBP_8x8( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase,
                              B8x8Idx       c8x8Idx )
{
  UInt uiSymbol  = ( ( rcMbDataAccess.getMbData().getMbCbp() >> c8x8Idx.b8x8Index() ) & 1 ? 1 : 0 );

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | ( 1 << c8x8Idx.b8x8Index() ) );
  }

  return ( uiSymbol == 1 );
}

Bool
UvlcWriter::RQpeekCbp4x4( MbDataAccess&  rcMbDataAccess,
                          MbDataAccess&  rcMbDataAccessBase,
                          LumaIdx        cIdx )
{
  UInt    uiSymbol  = 0;
  TCoeff* piCoeff   = rcMbDataAccess.    getMbTCoeffs().get( cIdx );
  TCoeff* piBCoeff  = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
    const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
  for( UInt ui = 0; ui < 16; ui++ )  
  {
    if( piCoeff[ pucScan[ui] ] && !piBCoeff[ pucScan[ui] ] )
    {
      uiSymbol = 1;
      break;
    }
  }
  return ( uiSymbol == 1 );
}

Bool
UvlcWriter::RQencodeBCBP_4x4( MbDataAccess&  rcMbDataAccess,
                               MbDataAccess&  rcMbDataAccessBase,
                               LumaIdx        cIdx )
{
  if ( (cIdx.x() %2) == 0 && (cIdx.y() %2) == 0)
  {
    // Write
    UInt    uiCode    = 0;
    UInt    uiLen     = 0;
    UInt uiFlip = (m_uiCbpStat4x4[1] > m_uiCbpStat4x4[0]) ? 1 : 0;
    UInt uiVlc = (m_uiCbpStat4x4[uiFlip] < 2*m_uiCbpStat4x4[1-uiFlip]) ? 0 : 2;

    for( Int iY=cIdx.y(); iY<cIdx.y()+2; iY++)
      for ( Int iX=cIdx.x(); iX<cIdx.x()+2; iX++)
      {
        UInt uiSymbol = 0;
        B4x4Idx cTmp(iY*4+iX);
        uiSymbol = RQpeekCbp4x4(rcMbDataAccess, rcMbDataAccessBase, cTmp);
        rcMbDataAccessBase.getMbData().setBCBP( cTmp, uiSymbol );
        uiCode <<= 1;
        uiCode |= uiSymbol;
        uiLen++;
        m_uiCbpStat4x4[uiSymbol]++;
      }

    if (uiFlip)
      uiCode = uiCode ^ ((1<<uiLen)-1);
    if (uiVlc == 0)
    {
      ANOK( xWriteCode( uiCode, uiLen ) );
    } else {
      ANOK( xWriteCode( g_auiISymCode[2][uiCode], g_auiISymLen[2][uiCode] ) );
    }
    // Scaling
    if (m_uiCbpStat4x4[0]+m_uiCbpStat4x4[1] > 512)
    {
      m_uiCbpStat4x4[0] >>= 1;
      m_uiCbpStat4x4[1] >>= 1;
    }
    ETRACE_T( "BCBP_4x4" );
    ETRACE_V( uiCode );
    ETRACE_N;
  }
  return RQpeekCbp4x4(rcMbDataAccess, rcMbDataAccessBase, cIdx);
}

Bool
UvlcWriter::RQencodeCBP_Chroma( MbDataAccess& rcMbDataAccess,
                                 MbDataAccess& rcMbDataAccessBase )
{
  UInt  uiSymbol          = ( ( rcMbDataAccess.getMbData().getMbCbp() >> 4 ) ? 1 : 0 );

  ETRACE_T( "CBP_Chroma" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | 0x10 );
  }
  return ( uiSymbol == 1 );
}

Bool
UvlcWriter::RQencodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx )
{
  UInt    uiSymbol  = 0;
  TCoeff* piCoeff   = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  TCoeff* piBCoeff  = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );

  for( UInt ui = 0; ui < 4; ui++ )  
  {
    if( piCoeff[ g_aucIndexChromaDCScan[ui] ] && !piBCoeff[ g_aucIndexChromaDCScan[ui] ] )
    {
      uiSymbol = 1;
      break;
    }
  }

  ANOK( xWriteFlag( uiSymbol ) );
  ETRACE_T( "BCBP_ChromaDC" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( 24 + cIdx.plane(), uiSymbol );
  
  return ( uiSymbol == 1 );
}


Bool
UvlcWriter::RQencodeBCBP_ChromaAC( MbDataAccess&  rcMbDataAccess,
                                    MbDataAccess&  rcMbDataAccessBase,
                                    ChromaIdx      cIdx )
{
  UInt    uiSymbol  = 0;
  TCoeff* piCoeff   = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  TCoeff* piBCoeff  = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
    const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());

  const UChar*  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
  for( UInt ui = 1; ui < 16; ui++ )  
  {
    if( piCoeff[ pucScan[ui] ] && !piBCoeff[ pucScan[ui] ] )
    {
      uiSymbol = 1;
      break;
    }
  }

  ANOK( xWriteFlag( uiSymbol ) );
  ETRACE_T( "BCBP_ChromaAC" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( 16 + cIdx, uiSymbol );
  
  return ( uiSymbol == 1 );
}

Bool
UvlcWriter::RQencodeCBP_ChromaAC( MbDataAccess& rcMbDataAccess,
                                   MbDataAccess& rcMbDataAccessBase )
{
  UInt  uiSymbol          = ( ( rcMbDataAccess.getMbData().getMbCbp() >> 5 ) ? 1 : 0 );

  ETRACE_T( "CBP_ChromaAC" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( ( rcMbDataAccessBase.getMbData().getMbCbp() & 0xF ) | 0x20 );
  }
  return ( uiSymbol == 1 );
}

ErrVal
UvlcWriter::RQencodeDeltaQp( MbDataAccess& rcMbDataAccess )
{
  ETRACE_T ("DQp");

  RNOK( xWriteSvlcCode( rcMbDataAccess.getDeltaQp() ) );

  ETRACE_TY ("se(v)");
  ETRACE_N;

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencode8x8Flag( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase ) 
{
  UInt uiSymbol = rcMbDataAccess.getMbData().isTransformSize8x8() ? 1 : 0;
 
  RNOK( xWriteFlag( uiSymbol ) );
  ETRACE_T( "TRAFO_8x8" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setTransformSize8x8( rcMbDataAccess.getMbData().isTransformSize8x8() );

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQeo8b( Bool& bEob )
{
  RNOK( xWriteFlag( bEob ? 1 : 0 ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeNewTCoeff_8x8( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx,
                                    UInt            uiScanIndex,
                                    Bool&           rbLast,
                                    UInt&           ruiNumCoefWritten )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_8x8_NEW" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4(),   1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+1, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+4, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+5, 1 );

  UInt auiEobShift[16];
  memset(auiEobShift, 0, sizeof(UInt)*16);

  RNOK( xRQencodeNewTCoeffs( piCoeff, piCoeffBase, uiScanIndex%4, 64, 4, pucScan, uiScanIndex, auiEobShift, rbLast, ruiNumCoefWritten ) );

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeNewTCoeff_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      LumaIdx         cIdx,
                                      UInt            uiScanIndex,
                                      Bool&           rbLast,
                                      UInt&           ruiNumCoefWritten )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;

  UInt          uiStart     = 0;
  UInt          uiStop      = 16;

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_4x4_NEW" );
  ETRACE_V( cIdx.b4x4() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeNewTCoeffs( piCoeff, piCoeffBase, uiStart, uiStop, 1, pucScan, uiScanIndex, m_auiShiftLuma, rbLast, ruiNumCoefWritten ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex,
                                        Bool&           rbLast,
                                        UInt&           ruiNumCoefWritten )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : ((bFrame) ? g_aucFrameScan : g_aucFieldScan));
  UInt          uiStart     = ( eResidualMode == CHROMA_AC ? 1 : 0  );
  UInt          uiStop      = ( eResidualMode == CHROMA_DC ? 4 : 16 );

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "CHROMA_4x4_NEW" );
  ETRACE_V( cIdx );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeNewTCoeffs( piCoeff, piCoeffBase, uiStart, uiStop, 1, pucScan, uiScanIndex, m_auiShiftChroma, rbLast, ruiNumCoefWritten ) );

  return Err::m_nOK;
}

ErrVal
UvlcWriter::xRQencodeNewTCoeffs( TCoeff*       piCoeff,
                                 TCoeff*       piCoeffBase,
                                 UInt          uiStart,
                                 UInt          uiStop,
                                 UInt          uiStride,
                                 const UChar*  pucScan,
                                 UInt          uiScanIndex,
                                 UInt*         pauiEobShift,
                                 Bool&         rbLast,
                                 UInt&         ruiNumCoefWritten )
{
  UInt ui;

  UInt uiCycle = 0;
  for ( ui=uiStart; ui<uiScanIndex; ui+=uiStride )
  {
    if ( !piCoeffBase[pucScan[ui]] && piCoeff[pucScan[ui]] )
    {
      uiCycle = ui/uiStride + 1;
    }
  }
  AOF( uiCycle < uiStop );
  Bool bSkipEob = !rbLast;
  ruiNumCoefWritten = 0;

  if( rbLast )
  {
    rbLast = true;
    for( ui = uiScanIndex; ui < uiStop; ui+=uiStride )
    {
      if( piCoeff[pucScan[ui]] && !piCoeffBase[pucScan[ui]] )
      {
        rbLast = false;
        break;
      }
    }
    if (rbLast) {

      UInt uiCountMag2;
      UInt uiLastPos = 1;
      for( ui = uiStart; ui < uiStop; ui+=uiStride )
      {
        if ( ! piCoeffBase[pucScan[ui]] )
          uiLastPos++;
        if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
        {
          uiLastPos = 1;
        }
      }
      RNOK( xRQencodeSigMagGreater1( piCoeff, piCoeffBase, uiLastPos, uiStart, uiStop, uiCycle, pucScan, uiCountMag2, uiStride ) );

      if ( uiCountMag2 == 0 )
      {
        RNOK( xWriteSigRunCode( min(pauiEobShift[uiCycle], uiLastPos), m_auiBestCodeTabMap[uiCycle] ) );
      }
    }
    ROTRS(rbLast, Err::m_nOK);
  } else
    rbLast = false;

  //===== SIGNIFICANCE BIT ======
  UInt uiSig;
  do
  {
    ruiNumCoefWritten++;

    UInt uiLastScanPosition = uiScanIndex + uiStride;
    while (uiLastScanPosition < uiStop && piCoeffBase[pucScan[uiLastScanPosition]])
      uiLastScanPosition += uiStride;

    if (uiLastScanPosition < uiStop)
    {
      uiSig = piCoeff[pucScan[uiScanIndex] ] ? 1 : 0;
    } else {
      uiSig = 1;
    }

    if( uiSig )
    {
      break;
    }

    uiScanIndex+=uiStride;
    while (uiScanIndex < uiStop && piCoeffBase[pucScan[uiScanIndex]])
      uiScanIndex+=uiStride;
  }
  while ( true );
  UInt uiSymbol = ruiNumCoefWritten - ((bSkipEob || ruiNumCoefWritten <= pauiEobShift[uiCycle]) ? 1 : 0);
  RNOK( xWriteSigRunCode( uiSymbol, m_auiBestCodeTabMap[uiCycle] ) );
  RNOK( xWriteFlag( piCoeff[pucScan[uiScanIndex]] < 0 ? 1 : 0 ) );

  // Check whether any more nonzero values
  Bool bFinished = true;
  for( ui=uiScanIndex+uiStride; ui<uiStop; ui+=uiStride )
  {
    bFinished &= ( piCoeffBase[pucScan[ui]] != 0 );
    if( !bFinished )
      break;
  }
  if( bFinished )
  {
    UInt uiCountMag2;
    RNOK( xRQencodeSigMagGreater1( piCoeff, piCoeffBase, 0, uiStart, uiStop, uiCycle, pucScan, uiCountMag2, uiStride ) );
    if( uiCountMag2 == 0 )
    {
      RNOK( xWriteSigRunCode( 0, m_auiBestCodeTabMap[uiCycle] ) );
    }
  }
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xRQencodeSigMagGreater1( TCoeff* piCoeff,
                                     TCoeff* piCoeffBase,
                                     UInt    uiBaseCode,
                                     UInt    uiStart,
                                     UInt    uiStop,
                                     UInt    uiVlcTable,
                                     const UChar*  pucScan,
                                     UInt&   ruiNumMagG1,
                                     UInt    uiStride )
{
  // Any magnitudes greater than one?
  ruiNumMagG1      = 0;
  UInt uiCountMag1 = 0;
  UInt uiMaxMag    = 0;
  UInt ui;
  for( ui = uiStart; ui < uiStop; ui+=uiStride )
  {
    if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
    {
      uiCountMag1++;
      UInt uiAbs = ( piCoeff[pucScan[ui]] < 0 ? -piCoeff[pucScan[ui]] : piCoeff[pucScan[ui]] );
      if ( uiAbs > 1 )
      {
        ruiNumMagG1++;
      }
      if ( uiAbs > uiMaxMag )
      {
        uiMaxMag = uiAbs;
      }
    }
  }

  if( ruiNumMagG1 == 0 )
  {
    return Err::m_nOK;
  }

  // Find optimal terminating code
  UInt uiTermSym;
  if ( uiMaxMag < 4 )
  {
    uiTermSym = 2*(ruiNumMagG1-1) + uiMaxMag%2;
  } else {
    uiTermSym = uiCountMag1*(uiMaxMag-2) + ruiNumMagG1 - 1;
  }
  RNOK( xWriteSigRunCode( uiBaseCode+uiTermSym+1, m_auiBestCodeTabMap[uiVlcTable] ) );

  UInt uiFlip      = 0;
  UInt uiRemaining = ruiNumMagG1;
  UInt uiBegin     = 0;
  UInt uiEnd       = uiCountMag1;
  UInt uiCount     = 0;
  for( ui = uiStart; ui < uiStop; ui+=uiStride )
  {
    if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
    {
      // Coding last value(s) may be unnecessary
      if ( uiRemaining == uiEnd-uiCount )
        break;
      // Range check for interval splitting
      uiCount++;
      if ( uiCount <= uiBegin )
        continue;
      if ( uiCount > uiEnd )
        break;
      UInt uiAbs = ( piCoeff[pucScan[ui]] < 0 ? -piCoeff[pucScan[ui]] : piCoeff[pucScan[ui]] );
      RNOK( xWriteFlag( (uiAbs > 1) ? 1 : 0 ) );
      uiRemaining -= ( uiAbs > 1 ) ? 1-uiFlip : uiFlip;
      if ( uiRemaining == 0 )
        break;
    }
  }
  UInt uiOutstanding = ruiNumMagG1;
  Bool bSeenMaxMag   = false;
  for( ui = uiStart; ui < uiStop; ui+=uiStride )
  {
    if( !bSeenMaxMag && uiOutstanding == 1 )
      break;
    if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
    {
      UInt uiAbs = ( piCoeff[pucScan[ui]] < 0 ? -piCoeff[pucScan[ui]] : piCoeff[pucScan[ui]] );
      bSeenMaxMag |= ( uiAbs == uiMaxMag );
      for ( UInt uiCutoff=1; uiAbs>uiCutoff && uiCutoff<uiMaxMag; uiCutoff++ )
      {
        RNOK( xWriteFlag( uiAbs > (uiCutoff+1) ) );
      }
      if( uiAbs > 1 )
        uiOutstanding--;
      if( uiOutstanding == 0 )
        break;
    }
  }
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeTCoeffRef_8x8( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx,
                                    UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;

  ETRACE_T( "LUMA_8x8_REF" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeTCoeffRef_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      LumaIdx         cIdx,
                                      UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;

  ETRACE_T( "LUMA_4x4_REF" );
  ETRACE_V( cIdx.b4x4() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xRQencodeTCoeffsRef( TCoeff*       piCoeff,
                                 TCoeff*       piCoeffBase,
                                 const UChar*  pucScan,
                                 UInt          uiScanIndex )
{
  if (m_uiCodedSymbols % 3 == m_uiFragmentedSymbols) {
    UInt uiCode = 0;
    UInt uiTable = m_pSymGrp->getTable();

    for (UInt ui = 0; ui < 3; ui++) {
      UInt uiSymbol = m_auiPrescannedSymbols[m_uiCodedSymbols + ui];
      m_pSymGrp->incrementCounter(uiSymbol);
      uiCode *= 3;
      uiCode += uiSymbol;
    }

    m_pSymGrp->setCodedFlag(true);
    RNOK(writeCode( g_auiRefSymCode[uiTable][uiCode], g_auiRefSymLen[uiTable][uiCode], "" ) );
  }
  if (m_uiFragmentedSymbols) {
    m_pSymGrp->setCodedFlag(true);
  }
  m_uiCodedSymbols++;

  return Err::m_nOK;
}

ErrVal
UvlcWriter::xRQprescanTCoeffsRef( TCoeff*       piCoeff,
                                  TCoeff*       piCoeffBase,
                                  const UChar*  pucScan,
                                  UInt          uiScanIndex)
{
  UInt  uiSig = ( piCoeff[pucScan[uiScanIndex]] ? 1 : 0 );
  UChar uiSym = 0;

  if(uiSig) 
  {
    UInt uiSignBL = ( piCoeffBase[pucScan[uiScanIndex]] < 0 ? 1 : 0 );
    UInt uiSignEL = ( piCoeff    [pucScan[uiScanIndex]] < 0 ? 1 : 0 );
    UInt uiSign = ( uiSignBL ^ uiSignEL );
    uiSym = (uiSign ? 2:1);
  }

  m_auiPrescannedSymbols[m_uiRefSymbols] = uiSym;
  m_uiRefSymbols++;

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : ((bFrame) ? g_aucFrameScan : g_aucFieldScan) );


  ETRACE_T( "CHROMA_4x4_REF" );
  ETRACE_V( cIdx );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeCycleSymbol( UInt uiCycle )
{
  RNOK( xWriteFlag( uiCycle > 0 ) );
  if ( uiCycle > 0 )
    RNOK( xWriteFlag( uiCycle - 1 ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xWriteGolomb(UInt uiSymbol, UInt uiK)
{
  UInt uiQ = uiSymbol / uiK;
  UInt uiR = uiSymbol - uiQ * uiK;
  UInt uiC = 0;
  UInt uiT = uiK >> 1;

  while ( uiT > 0 )
  {
    uiC++;
    uiT >>= 1;
  }

  // Unary part
  for ( UInt ui = 0; ui < uiQ; ui++ )
  {
    RNOK( xWriteFlag( 1 ) );
  }
  RNOK( xWriteFlag( 0 ) );

  // Binary part
  if ( uiR < uiC )
  {
    RNOK( xWriteCode( uiR, uiC ) );
  } else if ( uiC > 0 ) {
    RNOK( xWriteFlag( 1 ) );
    RNOK( xWriteCode( uiR - uiC, uiC ) );
  }
  ETRACE_N;

  return Err::m_nOK;
}

UInt
UvlcWriter::peekGolomb(UInt uiSymbol, UInt uiK)
{
  UInt uiQ = uiSymbol / uiK;
  UInt uiR = uiSymbol - uiQ * uiK;
  UInt uiC = 0;
  UInt uiT = uiK >> 1;

  while ( uiT > 0 )
  {
    uiC++;
    uiT >>= 1;
  }

  // Unary part
  uiT = uiQ + 1 + uiC;
  if ( uiR >= uiC && uiC > 0 )
  {
    uiT++;
  }

  return uiT;
}

ErrVal
UvlcWriter::RQencodeEobOffsets_Luma( UInt* pauiSeq )
{
  m_pSymGrp       ->Init();
  m_uiCbpStat4x4[0] = m_uiCbpStat4x4[1] = 0;
  m_uiCbpStats[0][0] = m_uiCbpStats[0][1] = m_uiCbpStats[1][0] = m_uiCbpStats[1][1] = 0;

  m_uiRefSymbols   = 0;
  m_uiCodedSymbols = 0;
  memset(m_auiPrescannedSymbols, 0, sizeof(UChar) * REFSYM_MB);
  m_uiFragmentedSymbols = 0;

  memcpy( m_auiShiftLuma, pauiSeq, sizeof(UInt)*16 );
  return xRQencodeEobOffsets(pauiSeq, 16);
}

ErrVal
UvlcWriter::RQencodeEobOffsets_Chroma( UInt* auiSeq )
{
  memcpy( m_auiShiftChroma, auiSeq, sizeof(UInt)*16 );
  m_auiShiftChroma[0] = 15;
  return xRQencodeEobOffsets(auiSeq+1, 15);
}

ErrVal
UvlcWriter::xRQencodeEobOffsets( UInt* auiSeq, UInt uiMax )
{
  UInt uiNumEnd = 0;
  for (UInt uiEc=0; uiEc<uiMax && auiSeq[uiEc] == uiMax-1 && uiNumEnd<3; uiEc++)
  {
    uiNumEnd++;
  }

  if ( uiNumEnd )
  {
    RNOK( xWriteGolomb( uiNumEnd-1, 1 ) );
  } else {
    ETRACE_T("Eob");
    RNOK( xWriteCode( 0x7, 3 ) );
    ETRACE_N;
  }
  RNOK( xWriteGolomb( auiSeq[uiNumEnd], 2 ) );
  RNOK( xEncodeMonSeq( auiSeq+uiNumEnd+1, auiSeq[uiNumEnd], uiMax-uiNumEnd-1 ) );
  ETRACE_N;
  return Err::m_nOK;
}

UInt g_auiSigRunTabCode[] = {0x01, 0x01, 0x01, 0x01, 0x00};
UInt g_auiSigRunTabCodeLen[] = {1, 2, 3, 4, 4};

ErrVal
UvlcWriter::RQencodeBestCodeTableMap( UInt* pauiTable, UInt uiMaxH )
{
  memset( m_auiBestCodeTabMap, 0, sizeof(UInt)*uiMaxH );
  Int uiW = uiMaxH-1;

  for(uiW = uiMaxH-1; uiW >= 0; uiW--)
  {
    if(pauiTable[uiW] != 0)
      break;
  }

  if(uiW < 0)
    uiW = 0;
  RNOK(xWriteCode(uiW, 4));

  for(UInt uiH = 0; uiH <= (UInt)uiW; uiH++)
  {
    m_auiBestCodeTabMap[uiH] = pauiTable[uiH];
    RNOK(xWriteCode(g_auiSigRunTabCode[pauiTable[uiH]], g_auiSigRunTabCodeLen[pauiTable[uiH]]));
  }

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQvlcFlush()
{
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQupdateVlcTable()
{
  m_pSymGrp->UpdateVlc();
  m_uiRefSymbols   = 0;
  m_uiCodedSymbols = 0;
  memset(m_auiPrescannedSymbols, 0, sizeof(UChar) * REFSYM_MB);
  return Err::m_nOK;
}

ErrVal 
UvlcWriter::RQcountFragmentedSymbols()
{
  if ((m_uiRefSymbols - m_uiCodedSymbols) > 0) {
    m_pSymGrp->setCodedFlag(false);
    switch (m_uiCodedSymbols%3) {
    case 0: m_uiFragmentedSymbols = 0; break;
    case 1: m_uiFragmentedSymbols = 2; break;
    case 2: m_uiFragmentedSymbols = 1; break;
    }
  }
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xEncodeMonSeq ( UInt* auiSeq, UInt uiStartVal, UInt uiLen )
{
  UInt uiRun   = 0;
  UInt uiLevel = uiStartVal;
  for (UInt uiPos=0; uiPos<uiLen && uiLevel > 0; uiPos++)
  {
    if (auiSeq[uiPos] == uiLevel)
    {
      uiRun++;
    } else {
      RNOK( xWriteGolomb( uiRun, 1 ) );
      uiRun = 1;
      uiLevel--;
      while ( uiLevel > auiSeq[uiPos] )
      {
        RNOK( xWriteGolomb( 0, 1 ) );
        uiLevel--;
      }
    }
  }
  if (uiLevel > 0)
  {
    RNOK( xWriteGolomb( uiRun, 1 ) );
  }
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xWriteSigRunCode ( UInt uiSymbol, UInt uiTableIdx )
{

  assert( uiTableIdx >= 0 && uiTableIdx <= 4 );
  if(uiTableIdx == 0)
  {
    // unary code 
    RNOK ( xWriteUnaryCode (uiSymbol) );
  }
  else if (uiTableIdx == 1)
  {
    RNOK ( xWriteCodeCB1 (uiSymbol) );
  }
  else if (uiTableIdx == 2)
  {
    RNOK ( xWriteCodeCB2 (uiSymbol) );
  }
  else if (uiTableIdx == 3)
  {
    if ( uiSymbol == 0 )
    {
      RNOK( xWriteFlag( 1 ) );
    }
    else
    {
      RNOK( xWriteFlag( 0 ) );
      RNOK( xWriteCodeCB2 (uiSymbol-1) );
    }
  }
  else // uiTableIdx == 4
  {
    if(uiSymbol == 0)
    {
      RNOK (xWriteFlag ( 1 ));
    }
    else
    {  
      RNOK (xWriteCodeCB1(uiSymbol+1));
    }
  }

  return Err::m_nOK;
}

ErrVal 
UvlcWriter::xWriteUnaryCode ( UInt uiSymbol )
{
  UInt uiStart = 0;
  do 
  {
    if(uiSymbol == uiStart)
    {
      RNOK( xWriteFlag (1) );
      break;
    }
    else 
    {
      RNOK( xWriteFlag (0) );
      uiStart++;
    }
  }
  while (true);
  return Err::m_nOK;
}

ErrVal 
UvlcWriter::xWriteCodeCB1 ( UInt uiSymbol )
{
  // this function writes codeword for the input symbol according to the {2, 2, 3, 3, 4, 4...} codebook
  for(UInt ui = 0; ui < uiSymbol/2; ui ++)
  {
    RNOK (xWriteFlag (0));
  }
  
  RNOK (xWriteCode((3-(uiSymbol%2)), 2)) ;

  return Err::m_nOK;
}

ErrVal
UvlcWriter::xWriteCodeCB2 ( UInt uiSymbol )
{
  // this function writes codeword for the input symbol according to the {2, 2, 2, 4, 4, 4...} codebook
  for(UInt ui = 0; ui < uiSymbol/3; ui ++)
  {
    RNOK(xWriteCode (0, 2));
  }

  RNOK (xWriteCode((3-(uiSymbol%3)), 2));

  return Err::m_nOK;
}

UcSymGrpWriter::UcSymGrpWriter( UvlcWriter* pParent )
{
  m_pParent      = pParent;
  Init();
}

ErrVal
UcSymGrpWriter::Init()
{
  m_uiCode         = 0;
  m_uiLen          = 0;
  m_auiSymCount[0] = m_auiSymCount[1] = m_auiSymCount[2] = 0;
  m_uiTable = 0;
  m_uiCodedFlag    = false;

  return Err::m_nOK;
}

ErrVal
UcSymGrpWriter::Write( UChar ucSym )
{
  // ucSym takes one of three values {0, 1, 2} 
  AOF((ucSym & 0xfc) == 0);

  {
    m_uiCode *= 3;
    m_uiCode += ucSym;
    m_auiSymCount[ucSym]++;
    m_uiLen++;

    if ( m_uiLen == CAVLC_SYMGRP_SIZE )
    {
      m_uiCodedFlag = true;
      RNOK( m_pParent->writeCode( g_auiRefSymCode[m_uiTable][m_uiCode], g_auiRefSymLen[m_uiTable][m_uiCode], "" ) );
      m_uiCode   = 0;
      m_uiLen    = 0;
    }
  }

  return Err::m_nOK;
}

Bool
UcSymGrpWriter::UpdateVlc()
{
  UInt uiFlag = m_uiCodedFlag;
  if (uiFlag) {
    // updating
    m_uiTable = 0;
    if (m_auiSymCount[0] < 2 *(m_auiSymCount[1] + m_auiSymCount[2]) ||
      m_auiSymCount[1] < 2 * m_auiSymCount[2]) {
      m_uiTable = 1;
    }

    // scaling
    m_auiSymCount[0] = (m_auiSymCount[0] >> 1);
    m_auiSymCount[1] = (m_auiSymCount[1] >> 1);
    m_auiSymCount[2] = (m_auiSymCount[2] >> 1);
    m_uiCodedFlag = false;

    m_pParent->resetFragmentedSymbols();
  }
  return (uiFlag != 0);
}

ErrVal
UcSymGrpWriter::Flush()
{
  if ( m_uiLen == 0 )
    return Err::m_nOK;

  {
    UInt uiMulti = 1;
    for(UInt ui = 0; ui < (CAVLC_SYMGRP_SIZE-m_uiLen); ui++)
      uiMulti *= 3;
    m_uiCode *= uiMulti; 
    m_uiLen = 0;
  }
  RNOK( m_pParent->writeCode( g_auiRefSymCode[m_uiTable][m_uiCode], g_auiRefSymLen[m_uiTable][m_uiCode], "" ) );

  m_uiCode = 0;
  m_uiLen  = 0;
  return Err::m_nOK;
}

H264AVC_NAMESPACE_END

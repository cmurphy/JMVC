#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/CommonTypes.h"
#include "H264AVCCommonLib/MbDataStruct.h"

#include<stdio.h>


H264AVC_NAMESPACE_BEGIN



ErrVal
MbDataStruct::save( FILE* pFile )
{
  ROF( pFile );

  UInt uiSave  = ::fwrite( this, sizeof(MbDataStruct), 1, pFile );

  ROF( uiSave == 1 );

  return Err::m_nOK;
}


ErrVal
MbDataStruct::load( FILE* pFile )
{
  ROF( pFile );

  UInt uiRead  = ::fread( this, sizeof(MbDataStruct), 1, pFile );

  ROF( uiRead == 1 );

  return Err::m_nOK;
}




const UChar MbDataStruct::m_aucACTab[6] =
{
  0,1,2,0,1,2
};

MbDataStruct::MbDataStruct()
: m_uiSliceId           ( 0 )
, m_bBLSkipFlag         ( false )
, m_eMbMode             ( MODE_SKIP )
, m_uiMbCbp             ( 0 )
, m_uiBCBP              ( 0 )
, m_usFwdBwd            ( 0 )
, m_ucChromaPredMode    ( 0 )
, m_ucQp                ( 0 )
, m_usResidualPredFlags ( 0 )
, m_bTransformSize8x8   ( false )
, m_bSkipFlag       ( true )
, m_bInCropWindowFlag ( false ) //TMM_ESS	
, m_bFieldFlag              ( 0 )
, m_uiMbCbpResidual         ( 0 )//loopfilter
, m_bSmoothedRefFlag		( false )	// JVT-R091
{
  DO_DBG( clearIntraPredictionModes( true ) );
  m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] = BLK_8x8;  //TMM_ESS
}


Void MbDataStruct::reset() 
{
  m_uiBCBP              = 0;
  m_usFwdBwd            = 0;
  m_uiSliceId           = 0;
  m_bBLSkipFlag         = false;
  m_eMbMode             = MODE_SKIP;
  m_uiMbCbp             = 0;
  m_ucChromaPredMode    = 0;
  m_ucQp                = 0;
  m_usResidualPredFlags = 0;
  m_bTransformSize8x8   = 0;
  m_bInCropWindowFlag   = false; //TMM_ESS	
	m_bSmoothedRefFlag		= false; // JVT-R091
  DO_DBG( clearIntraPredictionModes( true ) );
  m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] = BLK_8x8;  //TMM_ESS  
  m_bFieldFlag          = 0;
}


Void MbDataStruct::clear()
{
  m_usFwdBwd            = 0;
  m_bBLSkipFlag         = false;
  m_eMbMode             = MODE_SKIP;
  m_uiMbCbp             = 0;
  m_ucChromaPredMode    = 0;
  m_uiBCBP              = 0;
  m_usResidualPredFlags = 0;
  m_bTransformSize8x8   = 0;
  m_bInCropWindowFlag   = false; //TMM_ESS	
	m_bSmoothedRefFlag		= false; // JVT-R091
  clearIntraPredictionModes( true );
  m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] = BLK_8x8;  //TMM_ESS
    m_bSkipFlag = false; //th
  //m_bFieldFlag          = 0;//bug
}


Void MbDataStruct::clearIntraPredictionModes( Bool bAll )
{
  ::memset( m_ascIPredMode, DC_PRED, sizeof(UChar)* 16 );
  ROFVS( bAll );
  m_ucChromaPredMode = 0;
}



Void MbDataStruct::setMbCbp( UInt uiCbp )
{
  UInt uiExtMbCbp = 0;
  UInt uiMbCbpTmp = uiCbp;

  uiExtMbCbp += (uiCbp & 0x4) ? 0x33 : 0x00;
  uiExtMbCbp += (uiCbp & 0x8) ? 0xcc : 0x00;
  uiExtMbCbp <<= 8;
  uiExtMbCbp += (uiCbp & 0x1) ? 0x33 : 0x00;
  uiExtMbCbp += (uiCbp & 0x2) ? 0xcc : 0x00;
  uiExtMbCbp |= uiMbCbpTmp << 24;

  m_uiMbCbp = uiExtMbCbp;
}



Void MbDataStruct::setAndConvertMbExtCbp( UInt uiExtCbp )
{
  UInt uiMbCbp;
  {
    UInt uiCbp = uiExtCbp;
    uiMbCbp  = (0 != (uiCbp & 0x33)) ? 1 : 0;
    uiMbCbp += (0 != (uiCbp & 0xcc)) ? 2 : 0;
    uiCbp >>= 8;
    uiMbCbp += (0 != (uiCbp & 0x33)) ? 4 : 0;
    uiMbCbp += (0 != (uiCbp & 0xcc)) ? 8 : 0;
  }
  uiMbCbp += (uiExtCbp >> 16) << 4;

  m_uiMbCbp = (uiMbCbp<<24) | uiExtCbp;
}

Void MbDataStruct::copy( const MbDataStruct& rcMbDataStruct )
{
    copyFrom( rcMbDataStruct );
    m_bSkipFlag                 = rcMbDataStruct.m_bSkipFlag;
    m_bInCropWindowFlag         = rcMbDataStruct.m_bInCropWindowFlag;
}

Void MbDataStruct::copyFrom( const MbDataStruct& rcMbDataStruct )
{
  m_usFwdBwd            = rcMbDataStruct.m_usFwdBwd;
  m_uiSliceId           = rcMbDataStruct.m_uiSliceId;
  m_bBLSkipFlag         = rcMbDataStruct.m_bBLSkipFlag;
  m_eMbMode             = rcMbDataStruct.m_eMbMode;
  m_ucQp                = rcMbDataStruct.m_ucQp;
  m_uiMbCbp             = rcMbDataStruct.m_uiMbCbp;
  m_ucChromaPredMode    = rcMbDataStruct.m_ucChromaPredMode;
  m_uiBCBP              = rcMbDataStruct.m_uiBCBP;
  m_usResidualPredFlags = rcMbDataStruct.m_usResidualPredFlags;
  m_bTransformSize8x8   = rcMbDataStruct.m_bTransformSize8x8;
	m_bSmoothedRefFlag		= rcMbDataStruct.m_bSmoothedRefFlag; // JVT-R091
    m_bFieldFlag          = rcMbDataStruct.m_bFieldFlag;

  ::memcpy( m_aBlkMode,     rcMbDataStruct.m_aBlkMode,      sizeof(m_aBlkMode) );
  ::memcpy( m_ascIPredMode, rcMbDataStruct.m_ascIPredMode,  sizeof(m_ascIPredMode) );
}



ErrVal
MbDataStruct::upsampleMotion( const MbDataStruct& rcMbDataStruct, Par8x8 ePar8x8, Bool bDirect8x8 )
{
  //--- get & set fwd/bwd ---
  UShort  usFwdBwd8x8 = ( ( rcMbDataStruct.m_usFwdBwd >> ( ePar8x8 << 2 ) ) & 0x03 );
  m_usFwdBwd          = usFwdBwd8x8 + (usFwdBwd8x8<<4) + (usFwdBwd8x8<<8) + (usFwdBwd8x8<<12);

  //--- set block modes ---
  m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] = BLK_8x8;
  
  B8x8Idx c8x8Idx(ePar8x8);
  UInt uiCbp = rcMbDataStruct.getMbExtCbp() >> c8x8Idx.b4x4();

  UInt uiMbCbp = 0;
  uiMbCbp |= ((uiCbp&0x10) ? 0x33 : 0);
  uiMbCbp |= ((uiCbp&0x20) ? 0xcc : 0);
  uiMbCbp <<= 8; 
  uiMbCbp |= ((uiCbp&0x01) ? 0x33 : 0);
  uiMbCbp |= ((uiCbp&0x02) ? 0xcc : 0);

  setMbExtCbp( uiMbCbp );

  //--- set macroblock mode ---
  if      ( rcMbDataStruct.m_eMbMode >= INTRA_4X4 )
  {
    m_eMbMode = INTRA_4X4;
  }
  else if ( rcMbDataStruct.m_eMbMode == MODE_8x8 || rcMbDataStruct.m_eMbMode == MODE_8x8ref0 )
  {
    switch( rcMbDataStruct.m_aBlkMode[ePar8x8] )
    {
    case BLK_SKIP:  m_eMbMode = ( bDirect8x8 ? MODE_16x16 : MODE_8x8 );     break;
    case BLK_8x8:   m_eMbMode = MODE_16x16;   break;
    case BLK_8x4:   m_eMbMode = MODE_16x8;    break;
    case BLK_4x8:   m_eMbMode = MODE_8x16;    break;
    case BLK_4x4:   m_eMbMode = MODE_8x8;     break;
    }
  }
  else if ( rcMbDataStruct.m_eMbMode == MODE_SKIP )
  {
    m_eMbMode = ( bDirect8x8 ? MODE_16x16 : MODE_8x8 );
  }
  else
  {
    m_eMbMode = MODE_16x16;
  }

  return Err::m_nOK;
}



Bool
MbDataStruct::is8x8TrafoFlagPresent() const // only for MCTF case (skip mode)
{
  ROTRS( m_eMbMode == INTRA_BL, true  );
  ROTRS( m_eMbMode > INTRA_4X4, false );

  if( m_eMbMode == MODE_8x8 || m_eMbMode == MODE_8x8ref0 )
  {
    for( UInt n = 0; n < 4; n++ )
    {
      ROTRS( m_aBlkMode[n] > BLK_8x8, false );
    }
  }

  return true;
}

H264AVC_NAMESPACE_END

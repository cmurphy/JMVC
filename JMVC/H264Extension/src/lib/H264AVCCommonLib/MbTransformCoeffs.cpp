#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/MbTransformCoeffs.h"

#include <stdio.h>

H264AVC_NAMESPACE_BEGIN



ErrVal
MbTransformCoeffs::save( FILE* pFile )
{
  ROF( pFile );

  UInt uiSave  = ::fwrite( this, sizeof(MbTransformCoeffs), 1, pFile );

  ROF( uiSave == 1 );

  return Err::m_nOK;
}


ErrVal
MbTransformCoeffs::load( FILE* pFile )
{
  ROF( pFile );

  UInt uiRead  = ::fread( this, sizeof(MbTransformCoeffs), 1, pFile );

  ROF( uiRead == 1 );

  return Err::m_nOK;
}



Void MbTransformCoeffs::clear()
{
  for (Int i=0; i<24; i++)
    ::memset( &(m_aaiLevel[i][0]), 0, 16*sizeof(TCoeff) );
  
  ::memset( m_aaiLevel, 0, sizeof( m_aaiLevel ) );
  ::memset( m_aaucCoeffCount, 0, sizeof(m_aaucCoeffCount));
}

Void MbTransformCoeffs::clearAcBlk( ChromaIdx cChromaIdx )
{
  ::memset( &m_aaiLevel[16+cChromaIdx][1], 0, sizeof( TCoeff) * 15 );
}



Void
MbTransformCoeffs::clearLumaLevels()
{
  ::memset( &m_aaiLevel[0][0], 0, sizeof(TCoeff)*16*16 );
}

Void
MbTransformCoeffs::clearLumaLevels8x8( B8x8Idx c8x8Idx )
{
  UInt uiIndex = c8x8Idx.b8x8();
  ::memset( &m_aaiLevel[uiIndex  ][0], 0, sizeof(TCoeff)*16 );
  ::memset( &m_aaiLevel[uiIndex+1][0], 0, sizeof(TCoeff)*16 );
  ::memset( &m_aaiLevel[uiIndex+4][0], 0, sizeof(TCoeff)*16 );
  ::memset( &m_aaiLevel[uiIndex+5][0], 0, sizeof(TCoeff)*16 );
}

Void
MbTransformCoeffs::clearLumaLevels8x8Block( B8x8Idx c8x8Idx )
{
  ::memset( &m_aaiLevel[4*c8x8Idx.b8x8Index()][0], 0, sizeof(TCoeff)*64 );
}



Void MbTransformCoeffs::setAllCoeffCount( UChar ucCoeffCountValue )
{
  ::memset( m_aaucCoeffCount, ucCoeffCountValue, sizeof(m_aaucCoeffCount));
}

Void MbTransformCoeffs::copyFrom( MbTransformCoeffs& rcMbTransformCoeffs )
{
  ::memcpy( m_aaiLevel, rcMbTransformCoeffs.m_aaiLevel, sizeof( m_aaiLevel ) );
  ::memcpy( m_aaucCoeffCount, rcMbTransformCoeffs.m_aaucCoeffCount, sizeof( m_aaucCoeffCount ) );
}


MbTransformCoeffs::MbTransformCoeffs()
{
  clear();  
}







Void MbTransformCoeffs::clearNewAcBlk( ChromaIdx          cChromaIdx,
                                       MbTransformCoeffs& rcBaseMbTCoeffs )
{
  TCoeff* piCoeff     = m_aaiLevel[16+cChromaIdx];
  TCoeff* piCoeffBase = rcBaseMbTCoeffs.m_aaiLevel[16+cChromaIdx];

  for( UInt ui = 1; ui < 16; ui++ )
  {
    if( ! piCoeffBase[ui] )
    {
      piCoeff[ui] = 0;
    }
  }
}



Void
MbTransformCoeffs::clearNewLumaLevels( MbTransformCoeffs& rcBaseMbTCoeffs )
{
  TCoeff* piCoeff     = m_aaiLevel[0];
  TCoeff* piCoeffBase = rcBaseMbTCoeffs.m_aaiLevel[0];

  for( UInt ui = 0; ui < 16*16; ui++ )
  {
    if( ! piCoeffBase[ui] )
    {
      piCoeff[ui] = 0;
    }
  }
}

Void
MbTransformCoeffs::clearNewLumaLevels8x8( B8x8Idx             c8x8Idx,
                                          MbTransformCoeffs&  rcBaseMbTCoeffs )
{
  UInt auiOffset[4] = { 0, 1, 4, 5 };
  UInt uiIndex      = c8x8Idx.b8x8();

  for( UInt uiBlk = 0; uiBlk < 4; uiBlk++ )
  {
    TCoeff* piCoeff     = m_aaiLevel[uiIndex+auiOffset[uiBlk]];
    TCoeff* piCoeffBase = rcBaseMbTCoeffs.m_aaiLevel[uiIndex+auiOffset[uiBlk]];

    for( UInt ui = 0; ui < 16; ui++ )
    {
      if( ! piCoeffBase[ui] )
      {
        piCoeff[ui] = 0;
      }
    }
  }
}

Void
MbTransformCoeffs::clearNewLumaLevels8x8Block( B8x8Idx            c8x8Idx,
                                               MbTransformCoeffs& rcBaseMbTCoeffs )
{
  TCoeff* piCoeff     = m_aaiLevel[4*c8x8Idx.b8x8Index()];
  TCoeff* piCoeffBase = rcBaseMbTCoeffs.m_aaiLevel[4*c8x8Idx.b8x8Index()];

  for( UInt ui = 0; ui < 64; ui++ )
  {
    if( ! piCoeffBase[ui] )
    {
      piCoeff[ui] = 0;
    }
  }
}



H264AVC_NAMESPACE_END

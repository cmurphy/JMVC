#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvPicBuffer.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"




H264AVC_NAMESPACE_BEGIN


IntYuvMbBuffer::IntYuvMbBuffer()
: m_pPelCurrY( NULL )
, m_pPelCurrU( NULL )
, m_pPelCurrV( NULL )
{
  DO_DBG( ::memset( m_aucYuvBuffer, 0 , sizeof(m_aucYuvBuffer) ) );
}


IntYuvMbBuffer::~IntYuvMbBuffer()
{
}


Void IntYuvMbBuffer::loadIntraPredictors( IntYuvPicBuffer* pcSrcBuffer )
{
  Int y;

  XPel* pSrc = pcSrcBuffer->getMbLumAddr();
  XPel* pDes = getMbLumAddr();

  Int iSrcStride = pcSrcBuffer->getLStride();
  Int iDesStride = getLStride();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  ::memcpy( pDes, pSrc, sizeof(XPel)*21 );
  ::memcpy( pDes+iDesStride+17, pSrc+21, sizeof(XPel)*4 );

  for( y = 0; y < 16; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }

  pSrc = pcSrcBuffer->getMbCbAddr();
  pDes = getMbCbAddr();

  iSrcStride = pcSrcBuffer->getCStride();
  iDesStride = getCStride();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  ::memcpy( pDes, pSrc, sizeof(XPel)*9 );

  for( y = 0; y < 8; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }

  pSrc = pcSrcBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  ::memcpy( pDes, pSrc, sizeof(XPel)*9 );

  for( y = 0; y < 8; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }
}


Void IntYuvMbBuffer::loadBuffer( IntYuvPicBuffer* pcSrcBuffer )
{
  Int   y;
  XPel* pSrc;
  XPel* pDes;
  Int   iSrcStride;
  Int   iDesStride;

  pSrc = pcSrcBuffer->getMbLumAddr();
  pDes = getMbLumAddr();
  iDesStride = getLStride();
  iSrcStride = pcSrcBuffer->getLStride();

  for( y = 0; y < 16; y++ )
  {
    ::memcpy( pDes, pSrc, 16 * sizeof(XPel) );
    pDes += iDesStride;
    pSrc += iSrcStride;
  }

  pSrc = pcSrcBuffer->getMbCbAddr();
  pDes = getMbCbAddr();
  iDesStride = getCStride();
  iSrcStride = pcSrcBuffer->getCStride();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iDesStride;
    pSrc += iSrcStride;
  }

  pSrc = pcSrcBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
}

Void IntYuvMbBuffer::loadBuffer( YuvMbBuffer* pcSrcBuffer )
{
  Int   y,x;
  Pel* pSrc;
  XPel* pDes;
  Int   iSrcStride;
  Int   iDesStride;

  pSrc = pcSrcBuffer->getMbLumAddr();
  pDes = getMbLumAddr();
  iDesStride = getLStride();
  iSrcStride = pcSrcBuffer->getLStride();

  for( y = 0; y < 16; y++ )
  {
    for( x = 0; x < 16; x++ )
    {
      pDes[x] = pSrc[x];
    }
    pDes += iDesStride;
    pSrc += iSrcStride;
  }

  pSrc = pcSrcBuffer->getMbCbAddr();
  pDes = getMbCbAddr();
  iDesStride = getCStride();
  iSrcStride = pcSrcBuffer->getCStride();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      pDes[x] = pSrc[x];
    }
    pDes += iDesStride;
    pSrc += iSrcStride;
  }

  pSrc = pcSrcBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      pDes[x] = pSrc[x];
    }
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
}

Void
IntYuvMbBuffer::add( IntYuvMbBuffer& rcIntYuvMbBuffer )
{
  Int   y, x;
  Int   iStride = getLStride  ();
  XPel* pSrc    = rcIntYuvMbBuffer.getMbLumAddr();
  XPel* pDes    = getMbLumAddr();

  for( y = 0; y < 16; y++ )
  {
    for( x = 0; x < 16; x++ )   pDes[x] += pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }

  iStride = getCStride  ();
  pSrc    = rcIntYuvMbBuffer.getMbCbAddr ();
  pDes    = getMbCbAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] += pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }

  pSrc    = rcIntYuvMbBuffer.getMbCrAddr ();
  pDes    = getMbCrAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] += pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }
}



Void
IntYuvMbBuffer::clip()
{
  Int   y, x;
  Int   iStride = getLStride  ();
  XPel* pDes    = getMbLumAddr();

  for( y = 0; y < 16; y++ )
  {
    for( x = 0; x < 16; x++ )   pDes[x] = gClip( pDes[x] );
    pDes += iStride;
  }

  iStride = getCStride  ();
  pDes    = getMbCbAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] = gClip( pDes[x] );
    pDes += iStride;
  }

  pDes    = getMbCrAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] = gClip( pDes[x] );
    pDes += iStride;
  }
}


Bool
IntYuvMbBuffer::isZero()
{
  Int   x, y;
  XPel* pPel    = getMbLumAddr();
  Int   iStride = getLStride  ();

  for( y = 0; y < 16; y++ )
  {
    for( x = 0; x < 16; x++ )
    {
      if( pPel[x] )
      {
        return false;
      }
    }
    pPel += iStride;
  }

  pPel    = getMbCbAddr ();
  iStride = getCStride  ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      if( pPel[x] )
      {
        return false;
      }
    }
    pPel += iStride;
  }

  pPel    = getMbCrAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      if( pPel[x] )
      {
        return false;
      }
    }
    pPel += iStride;
  }

  return true;
}

Void
IntYuvMbBuffer::subtract( IntYuvMbBuffer& rcIntYuvMbBuffer )
{
  Int   y, x;
  Int   iStride = getLStride  ();
  XPel* pSrc    = rcIntYuvMbBuffer.getMbLumAddr();
  XPel* pDes    = getMbLumAddr();

  for( y = 0; y < 16; y++ )
  {
    for( x = 0; x < 16; x++ )   pDes[x] -= pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }

  iStride = getCStride  ();
  pSrc    = rcIntYuvMbBuffer.getMbCbAddr ();
  pDes    = getMbCbAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] -= pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }

  pSrc    = rcIntYuvMbBuffer.getMbCrAddr ();
  pDes    = getMbCrAddr ();

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8;  x++ )   pDes[x] -= pSrc[x];
    pDes += iStride;
    pSrc += iStride;
  }
}




Void IntYuvMbBuffer::loadChroma( IntYuvMbBuffer& rcSrcBuffer )
{
  const Int iStride = getCStride();
  XPel*     pDes    = getMbCbAddr();
  XPel*     pSrc    = rcSrcBuffer.getMbCbAddr();
  Int       y;

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }

  pDes = getMbCrAddr();
  pSrc = rcSrcBuffer.getMbCrAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }
}


Void IntYuvMbBuffer::loadLuma( IntYuvMbBuffer& rcSrcBuffer, LumaIdx c4x4Idx )
{
  const Int iStride = getLStride();
  XPel*     pDes    = getYBlk( c4x4Idx );
  XPel*     pSrc    = rcSrcBuffer.getYBlk( c4x4Idx );

  for( Int y = 0; y < 4; y++ )
  {
    ::memcpy( pDes, pSrc, 4 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }
}


Void IntYuvMbBuffer::loadLuma( IntYuvMbBuffer& rcSrcBuffer, B8x8Idx c8x8Idx )
{
  const Int iStride = getLStride();
  XPel*     pDes = getYBlk( c8x8Idx );
  XPel*     pSrc = rcSrcBuffer.getYBlk( c8x8Idx );

  for( Int y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pSrc, 8 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }
}


Void IntYuvMbBuffer::loadLuma( IntYuvMbBuffer& rcSrcBuffer )
{
  const Int iStride = getLStride();
  XPel*     pDes = getMbLumAddr();
  XPel*     pSrc = rcSrcBuffer.getMbLumAddr();

  for( Int y = 0; y < 16; y++ )
  {
    ::memcpy( pDes, pSrc, 16 * sizeof(XPel) );
    pDes += iStride;
    pSrc += iStride;
  }
}


Void IntYuvMbBuffer::setAllSamplesToZero()
{
  Int   y;
  XPel* pPel    = getMbLumAddr();
  Int   iStride = getLStride();

  for( y = 0; y < 16; y++ )
  {
    ::memset( pPel, 0x00, 16 * sizeof(XPel) );
    pPel += iStride;
  }

  pPel    = getMbCbAddr();
  iStride = getCStride();

  for( y = 0; y < 8; y++ )
  {
    ::memset( pPel, 0x00, 8 * sizeof(XPel) );
    pPel += iStride;
  }

  pPel    = getMbCrAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memset( pPel, 0x00, 8 * sizeof(XPel) );
    pPel += iStride;
  }
}

Void IntYuvMbBuffer::loadIntraPredictors( YuvPicBuffer* pcSrcBuffer )
{
  Int x,y;

  Pel* pSrc = pcSrcBuffer->getMbLumAddr();
  XPel* pDes = getMbLumAddr();

  Int iSrcStride = pcSrcBuffer->getLStride();
  Int iDesStride = getLStride();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  for( x = 0; x < 21; x++)
  {
    pDes[x] = pSrc[x];
  }
  for( x = 0; x < 4; x++)
  {
    pDes[x+iDesStride+17] = pSrc[x+21];
  }

  for( y = 0; y < 16; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }

  pSrc = pcSrcBuffer->getMbCbAddr();
  pDes = getMbCbAddr();

  iSrcStride = pcSrcBuffer->getCStride();
  iDesStride = getCStride();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  for( x = 0; x < 9; x++)
  {
    pDes[x] = pSrc[x];
  }

  for( y = 0; y < 8; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }

  pSrc = pcSrcBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  for( x = 0; x < 9; x++)
  {
    pDes[x] = pSrc[x];
  }

  for( y = 0; y < 8; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }
}


Void IntYuvMbBufferExtension::loadSurrounding( IntYuvPicBuffer* pcSrcBuffer )
{
  Int x, y;
  Int iDesStride = getLStride();
  Int iSrcStride = pcSrcBuffer->getLStride();
  XPel*     pSrc = pcSrcBuffer->getMbLumAddr();
  XPel*     pDes = getMbLumAddr();

  for( x = 0; x < 18; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < 16; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[16] = pSrc[16];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 18; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }

  iDesStride  = getCStride();
  iSrcStride  = pcSrcBuffer->getCStride();
  pSrc        = pcSrcBuffer->getMbCbAddr();
  pDes        = getMbCbAddr();

  for( x = 0; x < 10; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < 8; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[8]  = pSrc[8];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 10; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }

  pSrc        = pcSrcBuffer->getMbCrAddr();
  pDes        = getMbCrAddr();

  for( x = 0; x < 10; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < 8; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[8]  = pSrc[8];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 10; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }
}

Void IntYuvMbBufferExtension::loadSurrounding( YuvPicBuffer* pcSrcBuffer )
{
  Int x, y;
  Int iDesStride = getLStride();
  Int iSrcStride = pcSrcBuffer->getLStride();
  Pel*      pSrc = pcSrcBuffer->getMbLumAddr();
  XPel*     pDes = getMbLumAddr();

  for( x = 0; x < 18; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < 16; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[16] = pSrc[16];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 18; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }

  iDesStride  = getCStride();
  iSrcStride  = pcSrcBuffer->getCStride();
  pSrc        = pcSrcBuffer->getMbCbAddr();
  pDes        = getMbCbAddr();

  for( x = 0; x < 10; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < 8; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[8]  = pSrc[8];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 10; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }

  pSrc        = pcSrcBuffer->getMbCrAddr();
  pDes        = getMbCrAddr();

  for( x = 0; x < 10; x++ )
  {
    pDes[x-iDesStride-1] = pSrc[x-iSrcStride-1];
  }
  for( y = 0; y < 8; y++ )
  {
    pDes[-1] = pSrc[-1];
    pDes[8]  = pSrc[8];
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
  for( x = 0; x < 10; x++ )
  {
    pDes[x-1] = pSrc[x-1];
  }
}


Void IntYuvMbBufferExtension::xMerge( Int xDir, Int yDir, UInt uiSize, XPel *puc, Int iStride, Bool bPresent )
{
  XPel pPelH[9];
  XPel pPelV[9];
  Int  iAdd   =1;
  Int  x,y,xo;

  if( yDir < 0)	
  {
    puc    += iStride*(uiSize-1);
    iStride = -iStride; 
  }

  if( xDir < 0)	
  {
    puc    += (uiSize-1);
    iAdd   =-1;
  }

  for( x = 0; x <= (Int)uiSize; x++ )
  {
    pPelH[x] = puc[(x-1)*iAdd - iStride];
  }	

  for( y = 0; y <= (Int)uiSize; y++ )
  {
    pPelV[y] = puc[(y-1)*iStride - iAdd];
  }	

  if( ! bPresent )
  {
    pPelV[0] = pPelH[0] = (pPelH[1] + pPelV[1] + 1)>>1;
  }

  for( y = 0; y < (Int)uiSize; y++, puc += iStride )
  {
    for( xo = 0, x = 0; x < (Int)uiSize; x++, xo += iAdd )
    {
      const Int iOffset = x-y;
	    if( iOffset > 0 )
      {
        puc[xo] = (pPelH[ iOffset-1] + 2*pPelH[ iOffset] + pPelH[ iOffset+1] + 2)>>2;
      }
      else if( iOffset < 0 )
      {
        puc[xo] = (pPelV[-iOffset-1] + 2*pPelV[-iOffset] + pPelV[-iOffset+1] + 2)>>2;
      }
      else
      {
        puc[xo] = (pPelH[1] + 2*pPelV[0] + pPelV[1] + 2)>>2;
      }
    }
  }
}

Void IntYuvMbBufferExtension::mergeFromLeftAbove ( LumaIdx cIdx, Bool bCornerMbPresent )
{
  xMerge( 1, 1, 8, getYBlk( cIdx ), getLStride(), bCornerMbPresent );
  xMerge( 1, 1, 4, getUBlk( cIdx ), getCStride(), bCornerMbPresent );
  xMerge( 1, 1, 4, getVBlk( cIdx ), getCStride(), bCornerMbPresent );
}

Void IntYuvMbBufferExtension::mergeRightBelow    ( LumaIdx cIdx, Bool bCornerMbPresent )
{
  xMerge( -1, -1, 8, getYBlk( cIdx ), getLStride(), bCornerMbPresent );
  xMerge( -1, -1, 4, getUBlk( cIdx ), getCStride(), bCornerMbPresent );
  xMerge( -1, -1, 4, getVBlk( cIdx ), getCStride(), bCornerMbPresent );
}

Void IntYuvMbBufferExtension::mergeFromRightAbove( LumaIdx cIdx, Bool bCornerMbPresent )
{
  xMerge( -1, 1, 8, getYBlk( cIdx ), getLStride(), bCornerMbPresent );
  xMerge( -1, 1, 4, getUBlk( cIdx ), getCStride(), bCornerMbPresent );
  xMerge( -1, 1, 4, getVBlk( cIdx ), getCStride(), bCornerMbPresent );
}

Void IntYuvMbBufferExtension::mergeLeftBelow     ( LumaIdx cIdx, Bool bCornerMbPresent )
{
  xMerge( 1, -1, 8, getYBlk( cIdx ), getLStride(), bCornerMbPresent );
  xMerge( 1, -1, 4, getUBlk( cIdx ), getCStride(), bCornerMbPresent );
  xMerge( 1, -1, 4, getVBlk( cIdx ), getCStride(), bCornerMbPresent );
}

Void IntYuvMbBufferExtension::copyFromBelow      ( LumaIdx cIdx )
{
  XPel* pPel    = getYBlk( cIdx );
  Int   iStride = getLStride();
  Int   y;

  pPel += 8*iStride;
  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pPel-iStride, pPel, 8 * sizeof(XPel) );
    pPel -= iStride;
  }

  pPel    = getUBlk( cIdx );
  iStride = getCStride();

  pPel += 4*iStride;
  for( y = 0; y < 4; y++ )
  {
    ::memcpy( pPel-iStride, pPel, 4 * sizeof(XPel) );
    pPel -= iStride;
  }

  pPel    = getVBlk( cIdx );

  pPel += 4*iStride;
  for( y = 0; y < 4; y++ )
  {
    ::memcpy( pPel-iStride, pPel, 4 * sizeof(XPel) );
    pPel -= iStride;
  }
}

Void IntYuvMbBufferExtension::copyFromLeft       ( LumaIdx cIdx )
{
  XPel* pPel    = getYBlk( cIdx );
  Int   iStride = getLStride();
  Int   x, y;

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      pPel[x] = pPel[-1];
    }
    pPel += iStride;
  }

  pPel    = getUBlk( cIdx );
  iStride = getCStride();

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = pPel[-1];
    }
    pPel += iStride;
  }

  pPel    = getVBlk( cIdx );

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = pPel[-1];
    }
    pPel += iStride;
  }
}

Void IntYuvMbBufferExtension::copyFromAbove      ( LumaIdx cIdx )
{
  XPel* pPel    = getYBlk( cIdx );
  Int   iStride = getLStride();
  Int   y;

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pPel, pPel-iStride, 8 * sizeof(XPel) );
    pPel += iStride;
  }

  pPel    = getUBlk( cIdx );
  iStride = getCStride();

  for( y = 0; y < 4; y++ )
  {
    ::memcpy( pPel, pPel-iStride, 4 * sizeof(XPel) );
    pPel += iStride;
  }

  pPel    = getVBlk( cIdx );

  for( y = 0; y < 4; y++ )
  {
    ::memcpy( pPel, pPel-iStride, 4 * sizeof(XPel) );
    pPel += iStride;
  }
}

Void IntYuvMbBufferExtension::copyFromRight      ( LumaIdx cIdx )
{
  XPel* pPel    = getYBlk( cIdx );
  Int   iStride = getLStride();
  Int   y,x;

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      pPel[x] = pPel[8];
    }
    pPel += iStride;
  }

  pPel    = getUBlk( cIdx );
  iStride = getCStride();

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = pPel[4];
    }
    pPel += iStride;
  }

  pPel    = getVBlk( cIdx );

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = pPel[4];
    }
    pPel += iStride;
  }
}

Void IntYuvMbBufferExtension::xFill( LumaIdx cIdx, XPel cY, XPel cU, XPel cV )
{
  XPel* pPel    = getYBlk( cIdx );
  Int   iStride = getLStride();
  Int   x, y;

  for( y = 0; y < 8; y++ )
  {
    for( x = 0; x < 8; x++ )
    {
      pPel[x] = cY;
    }
    pPel += iStride;
  }

  pPel    = getUBlk( cIdx );
  iStride = getCStride();

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = cU;
    }
    pPel += iStride;
  }

  pPel    = getVBlk( cIdx );

  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      pPel[x] = cV;
    }
    pPel += iStride;
  }
}

Void IntYuvMbBufferExtension::copyFromLeftAbove  ( LumaIdx cIdx )
{
  XPel cY = *(getYBlk( cIdx ) - 1 - getLStride());
  XPel cU = *(getUBlk( cIdx ) - 1 - getCStride());
  XPel cV = *(getVBlk( cIdx ) - 1 - getCStride());

  xFill( cIdx, cY, cU, cV );
}

Void IntYuvMbBufferExtension::copyFromRightAbove ( LumaIdx cIdx )
{
  XPel cY = *(getYBlk( cIdx ) + 8 - getLStride());
  XPel cU = *(getUBlk( cIdx ) + 4 - getCStride());
  XPel cV = *(getVBlk( cIdx ) + 4 - getCStride());

  xFill( cIdx, cY, cU, cV );
}

Void IntYuvMbBufferExtension::copyFromLeftBelow  ( LumaIdx cIdx )
{
  XPel cY = *(getYBlk( cIdx ) - 1 + 8 * getLStride());
  XPel cU = *(getUBlk( cIdx ) - 1 + 4 * getCStride());
  XPel cV = *(getVBlk( cIdx ) - 1 + 4 * getCStride());

  xFill( cIdx, cY, cU, cV );
}

Void IntYuvMbBufferExtension::copyFromRightBelow ( LumaIdx cIdx )
{
  XPel cY = *(getYBlk( cIdx ) + 8 + 8 * getLStride());
  XPel cU = *(getUBlk( cIdx ) + 4 + 4 * getCStride());
  XPel cV = *(getVBlk( cIdx ) + 4 + 4 * getCStride());

  xFill( cIdx, cY, cU, cV );
}


H264AVC_NAMESPACE_END


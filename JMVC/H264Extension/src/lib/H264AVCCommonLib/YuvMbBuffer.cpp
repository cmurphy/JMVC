#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"


H264AVC_NAMESPACE_BEGIN


YuvMbBuffer::YuvMbBuffer():
  m_pPelCurr( NULL )
{
  DO_DBG( ::memset( m_aucYuvBuffer, 0 , sizeof(m_aucYuvBuffer) ) );
}

YuvMbBuffer::~YuvMbBuffer()
{
}

Void
YuvMbBuffer::setZero()
{
  ::memset( m_aucYuvBuffer, 0 , sizeof(m_aucYuvBuffer) );
}

Void YuvMbBuffer::loadIntraPredictors( YuvPicBuffer* pcSrcBuffer )
{
  Int y;

  Pel* pSrc = pcSrcBuffer->getMbLumAddr();
  Pel* pDes = getMbLumAddr();

  Int iSrcStride = pcSrcBuffer->getLStride();
  Int iDesStride = getLStride();

  pSrc -= iSrcStride+1;
  pDes -= iDesStride+1;

  ::memcpy( pDes, pSrc, sizeof(Pel)*21 );

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

  ::memcpy( pDes, pSrc, sizeof(Pel)*9 );

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

  ::memcpy( pDes, pSrc, sizeof(Pel)*9 );

  for( y = 0; y < 8; y++)
  {
    pSrc += iSrcStride;
    pDes += iDesStride;
    *pDes = *pSrc;
  }
}

Void YuvMbBuffer::loadBufferClip( IntYuvMbBuffer* pcSrcBuffer )
{
  Int   y,x;
  XPel* pSrc;
  Pel* pDes;
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
      pDes[x] = gClip( pSrc[x] );
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
      pDes[x] = gClip( pSrc[x] );
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
      pDes[x] = gClip( pSrc[x] );
    }
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
}


Void YuvMbBuffer::loadBuffer( IntYuvMbBuffer* pcSrcBuffer )
{
  Int   y,x;
  XPel* pSrc;
  Pel* pDes;
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
      pDes[x] = (Pel)pSrc[x];
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
      pDes[x] = (Pel)pSrc[x];
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
      pDes[x] = (Pel)pSrc[x];
    }
    pDes += iDesStride;
    pSrc += iSrcStride;
  }
}


H264AVC_NAMESPACE_END

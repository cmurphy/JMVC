#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"

H264AVC_NAMESPACE_BEGIN

YuvPicBuffer::YuvPicBuffer( YuvBufferCtrl& rcYuvBufferCtrl, PicType ePicType ):
m_rcBufferParam   ( rcYuvBufferCtrl.getBufferParameter( ePicType ) ),
m_ePicType        ( ePicType ),
  m_rcYuvBufferCtrl ( rcYuvBufferCtrl ),
  m_iStride         ( 0 ),
  m_pPelCurr        ( NULL ),
  m_pucYuvBuffer    ( NULL ),
  m_pucOwnYuvBuffer ( NULL )
{
}


YuvPicBuffer::~YuvPicBuffer()
{
}



ErrVal YuvPicBuffer::init( Pel*& rpucYuvBuffer )
{
  ROT( NULL != m_pucYuvBuffer );
  ROF( m_rcYuvBufferCtrl.isInitDone() )
  m_iStride = m_rcBufferParam.getStride();

  UInt uiSize;
  RNOK( m_rcYuvBufferCtrl.getChromaSize( uiSize ) );

  if( NULL == rpucYuvBuffer )
  {
    m_pucOwnYuvBuffer = new Pel[ 6 * uiSize ];
    ROT( NULL == m_pucOwnYuvBuffer );
    rpucYuvBuffer = m_pucYuvBuffer = m_pucOwnYuvBuffer;
  }
  else
  {
    m_pucOwnYuvBuffer = NULL;
    m_pucYuvBuffer = rpucYuvBuffer;
  }

  m_pucYuvBuffer[(6 * uiSize)-1] = 0xde;

  m_rcYuvBufferCtrl.initMb();
  return Err::m_nOK;
}



ErrVal YuvPicBuffer::uninit()
{
  if( m_pucOwnYuvBuffer )
  {
    delete [] m_pucOwnYuvBuffer;
  }
  m_pucYuvBuffer    = NULL;
  m_pucOwnYuvBuffer = NULL;
  m_pPelCurr = NULL;
  m_iStride = 0;

  return Err::m_nOK;
}

ErrVal YuvPicBuffer::loadBuffer( YuvMbBuffer *pcYuvMbBuffer )
{
  Pel   *pDes       = getMbLumAddr();
  Pel   *pScr       = pcYuvMbBuffer->getMbLumAddr();
  Int   iSrcStride  = pcYuvMbBuffer->getLStride();
  Int   iDesStride  = getLStride();
  UInt  y;

  for( y = 0; y < 16; y++ )
  {
    ::memcpy( pDes, pScr, 16* sizeof(Pel) );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  iDesStride  >>= 1;
  iSrcStride  = pcYuvMbBuffer->getCStride();
  pScr = pcYuvMbBuffer->getMbCbAddr();
  pDes = getMbCbAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pScr, 8* sizeof(Pel) );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  pScr = pcYuvMbBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pScr, 8* sizeof(Pel) );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  return Err::m_nOK;
}

// HS: decoder robustness
ErrVal
YuvPicBuffer::copy( YuvPicBuffer* pcPicBuffer )
{
  Pel   *pDes   = getLumOrigin();
  Pel   *pScr   = pcPicBuffer->getMbLumAddr();
  Int   iStride = getLStride();
  Int   iWidth  = getLWidth();
  Int   iHeight = getLHeight();
  UInt  y;

  for( y = 0; y < (UInt)iHeight; y++ )
  {
    ::memcpy( pDes, pScr, iWidth* sizeof(Pel) );
    pDes += iStride,
    pScr += iStride;
  }

  iWidth >>= 1;
  iHeight >>=1;
  iStride  >>= 1;
  pScr = pcPicBuffer->getCbOrigin();
  pDes = getCbOrigin();

  for( y = 0; y < (UInt)iHeight; y++ )
  {
    ::memcpy( pDes, pScr, iWidth* sizeof(Pel) );
    pDes += iStride,
    pScr += iStride;
  }

  pScr = pcPicBuffer->getCrOrigin();
  pDes = getCrOrigin();

  for( y = 0; y < (UInt)iHeight; y++ )
  {
    ::memcpy( pDes, pScr, iWidth* sizeof(Pel) );
    pDes += iStride,
    pScr += iStride;
  }

  return Err::m_nOK;
}

ErrVal YuvPicBuffer::fillMargin()
{
  m_rcYuvBufferCtrl.initMb();

  xFillPlaneMargin( getMbLumAddr(), getLHeight(), getLWidth(), getLStride(), getLXMargin(), getLYMargin() );
  xFillPlaneMargin( getMbCbAddr(),  getCHeight(), getCWidth(), getCStride(), getCXMargin(), getCYMargin() );
  xFillPlaneMargin( getMbCrAddr(),  getCHeight(), getCWidth(), getCStride(), getCXMargin(), getCYMargin() );

  return Err::m_nOK;
}


Void YuvPicBuffer::xFillPlaneMargin( Pel *pucDest, Int iHeight, Int iWidth, Int iStride, Int iXMargin, Int iYMargin )
{
  Pel* puc;
  Int n;

  // left and right borders at once
  puc = pucDest;
  for( n = 0; n < iHeight; n++)
  {
    // left border lum
    ::memset( puc - iXMargin, puc[0],         iXMargin );
    // right border lum
    ::memset( puc + iWidth,  puc[iWidth - 1], iXMargin );
    puc += iStride;
  }

  // bot border lum
  puc = pucDest - iXMargin + iStride * iHeight;
  UInt uiSize = iWidth + 2*iXMargin;
  for( n = 0; n < iYMargin; n++)
  {
    ::memcpy( puc, puc - iStride, uiSize );
    puc += iStride;
  }

  // top border lum
  puc = pucDest - iXMargin;
  for( n = 0; n < iYMargin; n++)
  {
    ::memcpy( puc - iStride, puc, uiSize );
    puc -= iStride;
  }
}




ErrVal YuvPicBuffer::loadBuffer( YuvPicBuffer *pcSrcYuvPicBuffer )
{
  pcSrcYuvPicBuffer->m_rcYuvBufferCtrl.initMb();
  m_rcYuvBufferCtrl.initMb();

  Pel   *pDes       = getMbLumAddr();
  Pel   *pScr       = pcSrcYuvPicBuffer->getMbLumAddr();
  Int   iSrcStride  = pcSrcYuvPicBuffer->getLStride();
  Int   iDesStride  = getLStride();
  UInt  uiHeight    = getLHeight();
  UInt  uiSize      = getLWidth() * sizeof(Pel);
  UInt  y;

  for( y = 0; y < uiHeight; y++ )
  {
    ::memcpy( pDes, pScr, uiSize );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  iDesStride  >>= 1;
  iSrcStride  >>= 1;
  uiSize      >>= 1;
  pScr = pcSrcYuvPicBuffer->getMbCbAddr();
  pDes = getMbCbAddr();

  for( y = 0; y < uiHeight; y+=2 )
  {
    ::memcpy( pDes, pScr, uiSize );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  pScr = pcSrcYuvPicBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  for( y = 0; y < uiHeight; y+=2 )
  {
    ::memcpy( pDes, pScr, uiSize );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  return Err::m_nOK;
}
//TMM_EC }}

Void YuvPicBuffer::xCopyFillPlaneMargin( Pel *pucSrc, Pel *pucDest, Int iHeight, Int iWidth, Int iStride, Int iXMargin, Int iYMargin )
{
    Pel* puc;
    Int n;

    Int iOffset = -iXMargin;
    // rec area + left and right borders at once
    UInt uiSize = iWidth + 2*iXMargin;
    for( n = 0; n < iHeight; n++)
    {
        ::memcpy( pucDest + iOffset, pucSrc + iOffset, uiSize );
        iOffset += iStride;
    }

    // bot border lum
    puc = pucDest - iXMargin + iStride * iHeight;
    for( n = 0; n < iYMargin; n++)
    {
        ::memcpy( puc, puc - iStride, uiSize );
        puc += iStride;
    }

    // top border lum
    puc = pucDest - iXMargin;
    for( n = 0; n < iYMargin; n++)
    {
        ::memcpy( puc - iStride, puc, uiSize );
        puc -= iStride;
    }
}

ErrVal YuvPicBuffer::loadBufferAndFillMargin( YuvPicBuffer *pcSrcYuvPicBuffer )
{
    m_rcYuvBufferCtrl.initMb();

    ROT( pcSrcYuvPicBuffer->getLHeight() * pcSrcYuvPicBuffer->getLStride() != getLHeight() * getLStride() );

    //if( pcSrcYuvPicBuffer->m_ePicType != FRAME )
    //{
    //    AOT( m_ePicType != FRAME )
    //        Int iDesOffset  = pcSrcYuvPicBuffer->m_ePicType == BOT_FIELD ? getLStride() : -getLStride();

    //    xCopyFillPlaneMargin( pcSrcYuvPicBuffer->getMbLumAddr(), iDesOffset + getMbLumAddr(), pcSrcYuvPicBuffer->getLHeight(), pcSrcYuvPicBuffer->getLWidth(), pcSrcYuvPicBuffer->getLStride(), pcSrcYuvPicBuffer->getLXMargin(), pcSrcYuvPicBuffer->getLYMargin() );
    //    iDesOffset >>= 1;
    //    xCopyFillPlaneMargin( pcSrcYuvPicBuffer->getMbCbAddr(),  iDesOffset + getMbCbAddr(),  pcSrcYuvPicBuffer->getCHeight(), pcSrcYuvPicBuffer->getCWidth(), pcSrcYuvPicBuffer->getCStride(), pcSrcYuvPicBuffer->getCXMargin(), pcSrcYuvPicBuffer->getCYMargin() );
    //    xCopyFillPlaneMargin( pcSrcYuvPicBuffer->getMbCrAddr(),  iDesOffset + getMbCrAddr(),  pcSrcYuvPicBuffer->getCHeight(), pcSrcYuvPicBuffer->getCWidth(), pcSrcYuvPicBuffer->getCStride(), pcSrcYuvPicBuffer->getCXMargin(), pcSrcYuvPicBuffer->getCYMargin() );

    //    return Err::m_nOK;
    //}
	
	//lufeng: use bot field to fill entire frame at one time and fill frame margin
	 if( pcSrcYuvPicBuffer->m_ePicType != FRAME )
    {
        AOT( m_ePicType != FRAME )
            Int iDesOffset  = pcSrcYuvPicBuffer->m_ePicType == BOT_FIELD ? getLStride() : 0;

        xCopyFillPlaneMargin( pcSrcYuvPicBuffer->getMbLumAddr() - iDesOffset , getMbLumAddr(), getLHeight(), getLWidth(), getLStride(), getLXMargin(), getLYMargin() );
        iDesOffset >>= 1;
        xCopyFillPlaneMargin( pcSrcYuvPicBuffer->getMbCbAddr() - iDesOffset ,  getMbCbAddr(),  getCHeight(), getCWidth(), getCStride(), getCXMargin(), getCYMargin() );
        xCopyFillPlaneMargin( pcSrcYuvPicBuffer->getMbCrAddr() - iDesOffset ,  getMbCrAddr(),  getCHeight(), getCWidth(), getCStride(), getCXMargin(), getCYMargin() );

        return Err::m_nOK;
    }

    Int iSrcOffset  = pcSrcYuvPicBuffer->getPicType() == BOT_FIELD ? - pcSrcYuvPicBuffer->getCStride() : 0;
    iSrcOffset += getPicType() == BOT_FIELD ? getCStride() : 0;

    xCopyFillPlaneMargin( iSrcOffset + pcSrcYuvPicBuffer->getMbLumAddr(), getMbLumAddr(), getLHeight(), getLWidth(), getLStride(), getLXMargin(), getLYMargin() );
    iSrcOffset >>= 1;
    xCopyFillPlaneMargin( iSrcOffset + pcSrcYuvPicBuffer->getMbCbAddr(),  getMbCbAddr(),  getCHeight(), getCWidth(), getCStride(), getCXMargin(), getCYMargin() );
    xCopyFillPlaneMargin( iSrcOffset + pcSrcYuvPicBuffer->getMbCrAddr(),  getMbCrAddr(),  getCHeight(), getCWidth(), getCStride(), getCXMargin(), getCYMargin() );

    return Err::m_nOK;
}

H264AVC_NAMESPACE_END

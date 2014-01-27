#if !defined(AFX_INTERPOLATIONFILTER_H__79B49DF9_A41E_4043_AD0E_CAAC3A0A9DA1__INCLUDED_)
#define AFX_INTERPOLATIONFILTER_H__79B49DF9_A41E_4043_AD0E_CAAC3A0A9DA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvPicBuffer.h"

H264AVC_NAMESPACE_BEGIN

typedef Void (*FilterBlockFunc)( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
typedef Void (*XFilterBlockFunc)( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );

class H264AVCCOMMONLIB_API QuarterPelFilter
{
protected:
	QuarterPelFilter();
	virtual ~QuarterPelFilter();

public:
  static ErrVal create( QuarterPelFilter*& rpcQuarterPelFilter );
  ErrVal destroy();
  virtual ErrVal init();
  ErrVal uninit();

  Void predBlkBilinear( IntYuvMbBuffer*     pcDesBuffer, IntYuvPicBuffer*     pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);
  Void predBlk4Tap    ( IntYuvMbBuffer*     pcDesBuffer, IntYuvPicBuffer*     pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);
  Bool getClipMode    ()        { return m_bClip; }
  Void setClipMode( Bool bEnableClip ) { m_bClip = bEnableClip; }
  virtual ErrVal filterFrame( YuvPicBuffer* pcPelBuffer, YuvPicBuffer* pcHalfPelBuffer );
  virtual ErrVal filterFrame( IntYuvPicBuffer* pcPelBuffer, IntYuvPicBuffer* pcHalfPelBuffer );
  Void filterBlock( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize, UInt uiFilter )
  {
    m_afpFilterBlockFunc[uiFilter]( pDes, pSrc, iSrcStride, uiXSize, uiYSize );
  }
  Void filterBlock( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize, UInt uiFilter )
  {
    m_afpXFilterBlockFunc[uiFilter]( pDes, pSrc, iSrcStride, uiXSize, uiYSize );
  }


  Void predBlk( YuvMbBuffer* pcDesBuffer, YuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);
  Void predBlk( IntYuvMbBuffer*     pcDesBuffer, IntYuvPicBuffer*     pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);

  Void weightOnEnergy(UShort *usWeight, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX );
  Void xUpdInterpBlnr(Int* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, 
                                    UInt uiSizeY, UInt uiSizeX );
  Void xUpdInterp4Tap(Int* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, 
                                    UInt uiSizeY, UInt uiSizeX );
  Void xUpdInterpChroma( Int* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX );

protected:
  virtual Void xPredElse( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDy2Dx13( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDx2Dy13( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDx2Dy2( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDx0Dy13( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDx0Dy2( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDy0Dx13( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDy0Dx2( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );
  
  virtual Void xPredDx2( Short* psDest, Pel*  pucSrc, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );

  Void xPredElse    ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDy2Dx13 ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx2Dy13 ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx2Dy2  ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx0Dy13 ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx0Dy2  ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );
  Void xPredDy0Dx13 ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, UInt uiSizeY, UInt uiSizeX );
  Void xPredDy0Dx2  ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx2     ( XXPel* psDest,  XPel*  pucSrc, Int iSrcStride, UInt uiSizeY,   UInt uiSizeX );


  static Void xFilter1( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xFilter2( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xFilter3( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xFilter4( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );

  static Void xXFilter1( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xXFilter2( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xXFilter3( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xXFilter4( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );

  Int xClip( Int iPel ) { return ( m_bClip ? gClip( iPel ) : iPel); }

protected:
  Bool m_bClip;
  FilterBlockFunc m_afpFilterBlockFunc[4];
  XFilterBlockFunc m_afpXFilterBlockFunc[4];
};

#if AR_FGS_COMPENSATE_SIGNED_FRAME
#define SIGNED_ROUNDING(x, o, s)    ( ( (x) >= 0 ) ? ( ( (x) + (o) ) >> s ) : -( ( -(x) + (o) ) >> s ) )
#endif

H264AVC_NAMESPACE_END

#endif // !defined(AFX_INTERPOLATIONFILTER_H__79B49DF9_A41E_4043_AD0E_CAAC3A0A9DA1__INCLUDED_)

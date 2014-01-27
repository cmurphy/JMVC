#if !defined(AFX_INVERSETRANSFORM_H__B2D732EC_10EA_4C2F_9387_4456CFCA4439__INCLUDED_)
#define AFX_INVERSETRANSFORM_H__B2D732EC_10EA_4C2F_9387_4456CFCA4439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/Quantizer.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"

H264AVC_NAMESPACE_BEGIN


class H264AVCCOMMONLIB_API Transform :
public Quantizer
{
protected:
	Transform();
	virtual ~Transform();

public:
  static ErrVal create( Transform*& rpcTransform );
  ErrVal destroy();
  Void setClipMode( Bool bEnableClip ) { m_bClip = bEnableClip; }

  Bool getClipMode()                   { return m_bClip; }
  Void e4x4Trafo          ( XPel*               pOrg,
                            Int                 iStride,
                            TCoeff*             piCoeff )
  {
    x4x4Trafo( pOrg, iStride, piCoeff );
  }

  Void e4x4InverseTrafo   ( XPel*               pOrg,
                            Int                 iStride,
                            TCoeff*             piCoeff )
  {
    x4x4InverseTrafo( pOrg, iStride, piCoeff );
  }

  Void eForTransformChromaDc    ( TCoeff* piCoeff )
  {
    xForTransformChromaDc( piCoeff );
  }

  Void e8x8Trafo                ( XPel*               pOrg, 
                                  Int                 iStride,
                                  TCoeff*             piCoeff )
  {
    x8x8Trafo( pOrg, iStride, piCoeff );
  }

  Void e8x8InverseTrafo         ( XPel*               pOrg, 
                                  Int                 iStride,
                                  TCoeff*             piCoeff )
  {
    Int i, j;

    // pOrg is used as predictor in the function
    for( i = 0; i < 8; i ++ )
    {
      for( j = 0; j < 8; j ++ )
        pOrg[i * iStride + j] = 0;
    }

    invTransform8x8Blk( pOrg, iStride, piCoeff );
  }

  ErrVal        transform8x8Blk           ( IntYuvMbBuffer* pcOrgData, IntYuvMbBuffer* pcPelData, TCoeff* piCoeff, const UChar* pucScale, UInt& ruiAbsSum );
  ErrVal        transform4x4Blk           ( IntYuvMbBuffer* pcOrgData, IntYuvMbBuffer* pcPelData, TCoeff* piCoeff, const UChar* pucScale, UInt& ruiAbsSum );
  ErrVal        transformMb16x16          ( IntYuvMbBuffer* pcOrgData, IntYuvMbBuffer* pcPelData, TCoeff* piCoeff, const UChar* pucScale, UInt& ruiDcAbs,  UInt& ruiAcAbs );
  ErrVal        transformChromaBlocks     ( XPel* pucOrg, XPel* pucRec, Int iStride, TCoeff* piCoeff, TCoeff* piQuantCoeff, const UChar* pucScale, UInt& ruiDcAbs, UInt& ruiAcAbs );

  ErrVal        invTransform8x8Blk        ( XPel* puc, Int iStride, TCoeff* piCoeff );
  ErrVal        invTransform4x4Blk        ( XPel* puc, Int iStride, TCoeff* piCoeff );
  ErrVal        invTransformChromaBlocks  ( XPel* puc, Int iStride, TCoeff* piCoeff );
  
  ErrVal        invTransform4x4Blk        ( Pel*  puc, Int iStride, TCoeff* piCoeff );
  ErrVal        invTransformChromaBlocks  ( Pel*  puc, Int iStride, TCoeff* piCoeff );
  
  ErrVal        invTransformDcCoeff       ( TCoeff* piCoeff, Int iQpScale );
  Void          invTransformChromaDc      ( TCoeff* piCoeff, Int iQpScale );


  ErrVal        transformMb               ( TCoeff*         piCoeff,
                                            IntYuvMbBuffer& rcYuvMbBuffer,
                                            Int             iQpLuma,
                                            Int             iQpChroma,
                                            Int             iQuantOffsetDiv );
  ErrVal        inverseTransformMb        ( IntYuvMbBuffer& rcYuvMbBuffer,
                                            TCoeff*         piCoeff,
                                            Int             iQpLuma,
                                            Int             iQpChroma );

  ErrVal        requant4x4Block           ( IntYuvMbBuffer& rcResData,
                                            TCoeff*         piCoeff,
                                            TCoeff*         piCoeffBase,
                                            const UChar*    pucScale,
                                            Bool            bFirstIsDc,
                                            UInt&           ruiAbsSum );
  ErrVal        requantLumaDcCoeffs       ( TCoeff*         piCoeff,
                                            TCoeff*         piCoeffBase,
                                            const UChar*    pucScale,
                                            UInt&           ruiAbsSum );
  ErrVal        requant8x8Block           ( IntYuvMbBuffer& rcResData,
                                            TCoeff*         piCoeff,
                                            TCoeff*         piCoeffBase,
                                            const UChar*    pucScale,
                                            UInt&           ruiAbsSum );
  ErrVal        requantChroma             ( IntYuvMbBuffer& rcResData,
                                            TCoeff*         piCoeff,
                                            TCoeff*         piCoeffBase,
                                            const UChar*    pucScaleU,
                                            const UChar*    pucScaleV,
                                            UInt&           ruiDcAbs,
                                            UInt&           ruiAcAbs );


private:
  Void xForTransform8x8Blk      ( XPel* pucOrg, XPel* pucRec, Int iStride, TCoeff* piPredCoeff );
  Void xForTransform4x4Blk      ( XPel* pucOrg, XPel* pucRec, Int iStride, TCoeff* piPredCoeff );
  
  Void xInvTransform4x4Blk      ( XPel* puc, Int iStride, TCoeff* piCoeff );
  Void xInvTransform4x4Blk      ( Pel*  puc, Int iStride, TCoeff* piCoeff );
  
  Void xInvTransform4x4BlkNoAc  ( XPel* puc, Int iStride, TCoeff* piCoeff );
  Void xInvTransform4x4BlkNoAc  ( Pel*  puc, Int iStride, TCoeff* piCoeff );

  Void xForTransformChromaDc    ( TCoeff* piCoeff );
  Void xForTransformLumaDc      ( TCoeff* piCoeff );
  
  Int  xRound                   ( Int i     )             { return ((i)+(1<<5))>>6; }
  Int  xClip                    ( Int iPel  )             { return ( m_bClip ? gClip( iPel ) : iPel); }

  Void xQuantDequantUniform8x8      ( TCoeff* piQCoeff, TCoeff* piCoeff, const QpParameter& rcQp, const UChar* pucScale, UInt& ruiAbsSum );
  Void xQuantDequantUniform4x4      ( TCoeff* piQCoeff, TCoeff* piCoeff, const QpParameter& rcQp, const UChar* pucScale, UInt& ruiAbsSum );
  Void xQuantDequantNonUniformLuma  ( TCoeff* piQCoeff, TCoeff* piCoeff, const QpParameter& rcQp, const UChar* pucScale, UInt& ruiDcAbs, UInt& ruiAcAbs );
  Void xQuantDequantNonUniformChroma( TCoeff* piQCoeff, TCoeff* piCoeff, const QpParameter& rcQp, const UChar* pucScale, UInt& ruiDcAbs, UInt& ruiAcAbs );

  Void x4x4Trafo          ( XPel*               pOrg,
                            Int                 iStride,
                            TCoeff*             piCoeff );
  Void x4x4InverseTrafo   ( XPel*               pOrg,
                            Int                 iStride,
                            TCoeff*             piCoeff );
  Void x4x4Quant          ( TCoeff*             piQCoeff,
                            TCoeff*             piCoeff,
                            const QpParameter&  rcQp );
  Void x4x4Dequant        ( TCoeff*             piQCoeff,
                            TCoeff*             piCoeff,
                            const QpParameter&  rcQp );

  Void x8x8Trafo                ( XPel*               pOrg, 
                                  Int                 iStride,
                                  TCoeff*             piCoeff );
  Void xRequantUniform4x4       ( TCoeff*             piCoeff,
                                  TCoeff*             piCoeffBase,
                                  const QpParameter&  rcQp,
                                  const UChar*        pucScale,
                                  UInt&               ruiAbsSum );
  Void xRequantUniform8x8       ( TCoeff*             piCoeff,
                                  TCoeff*             piCoeffBase,
                                  const QpParameter&  rcQp,
                                  const UChar*        pucScale,
                                  UInt&               ruiAbsSum );
  Void xRequantNonUniformChroma ( TCoeff*             piCoeff,
                                  TCoeff*             piCoeffBase,
                                  const QpParameter&  rcQp,
                                  const UChar*        pucScale,
                                  UInt&               ruiDcAbs,
                                  UInt&               ruiAcAbs );

protected:
  const SliceHeader*  m_pcSliceHeader;
  Bool                m_bClip;
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_INVERSETRANSFORM_H__B2D732EC_10EA_4C2F_9387_4456CFCA4439__INCLUDED_)

#if !defined(AFX_MOTIONCOMPENSATION_H__820D6942_007B_42EA_838B_AC025E866DBE__INCLUDED_)
#define AFX_MOTIONCOMPENSATION_H__820D6942_007B_42EA_838B_AC025E866DBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/MotionVectorCalculation.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvPicBuffer.h"
#include "H264AVCCommonLib/IntFrame.h"


H264AVC_NAMESPACE_BEGIN


class SampleWeighting;
class QuarterPelFilter;
class FrameMng;
class Frame;

class Transform;

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

class H264AVCCOMMONLIB_API MotionCompensation :
public MotionVectorCalculation
{
protected:

  class MC8x8D
  {
  public:
    MC8x8D( Par8x8 ePar8x8 ) :  m_cIdx( B8x8Idx(ePar8x8) )  { clear(); }
    Void clear()
    {
      m_apcRefBuffer[0] = m_apcRefBuffer[1] = NULL;
      m_apcPW[0]        = m_apcPW[1]        = NULL;
    }

    B4x4Idx         m_cIdx;
    PW              m_acPW[2];
    const PW*       m_apcPW[2];
    YuvPicBuffer*   m_apcRefBuffer[2];
    Mv3D            m_aacMv[2][6];
    Short           m_sChromaOffset[2];
  };

  class IntMC8x8D
  {
  public:
    IntMC8x8D( Par8x8 ePar8x8 ) :  m_cIdx( B8x8Idx(ePar8x8) )  { clear(); }
    Void clear()
    {
      m_apcRefBuffer[0] = m_apcRefBuffer[1] = NULL;
      m_apcPW[0]        = m_apcPW[1]        = NULL;
    }

    B4x4Idx           m_cIdx;
    PW                m_acPW[2];
    const PW*         m_apcPW[2];
    IntYuvPicBuffer*  m_apcRefBuffer[2];
    Mv3D              m_aacMv[2][6];
    Mv3D              m_aacMvd[2][6];  // differential motion vector 
    Short           m_sChromaOffset[2];
  };

protected:
	MotionCompensation();
	virtual ~MotionCompensation();

public:
  static ErrVal create( MotionCompensation*& rpcMotionCompensation );
  ErrVal destroy();

  ErrVal init( QuarterPelFilter* pcQuarterPelFilter,
               Transform*        pcTransform,
               SampleWeighting* pcSampleWeighting);

  ErrVal initSlice( const SliceHeader& rcSH );
  ErrVal uninit();

  ErrVal compensateMb( MbDataAccess& rcMbDataAccess, YuvMbBuffer* pcRecBuffer, Bool bFaultTolerant, Bool bCalcMv = true );
  ErrVal calculateMb( MbDataAccess& rcMbDataAccess, Bool bFaultTolerant );


  ErrVal compensateMb     ( MbDataAccess&   rcMbDataAccess,
                            RefFrameList&   rcRefFrameList0,
                            RefFrameList&   rcRefFrameList1,
                            IntYuvMbBuffer* pcRecBuffer,
                            Bool            bCalcMv );
  ErrVal compensateSubMb  ( B8x8Idx         c8x8Idx,
                            MbDataAccess&   rcMbDataAccess,
                            RefFrameList&   rcRefFrameList0,
                            RefFrameList&   rcRefFrameList1,
                            IntYuvMbBuffer* pcRecBuffer,
                            Bool            bCalcMv,
                            Bool            bFaultTolerant );
  Void xAdjustResidualRefBlk          ( XPel*           piResidualRef,
                                        UInt            uiBlkWidth,
                                        UInt            uiBlkHeight,
                                        Int             iStride,
                                        UChar*          pucSigMap,
                                        Bool            bNonzeroBaseBlock,
                                        Int             iBcbpCtx,
                                        UInt            uiWeightZeroBlk,
                                        UInt            uiWeightZeroCoeff);

  Void xAdjustResidualRefBlkSpatial   ( XPel*           piResidualRef,
                                        UInt            uiBlkWidth,
                                        UInt            uiBlkHeight,
                                        Int             iStride,
                                        UInt            uiWeightZeroBlk);

  Void xAdjustResidualRefBlkFrequency ( XPel*           piResidualRef,
                                        UInt            uiBlkWidth,
                                        UInt            uiBlkHeight,
                                        Int             iStride,
                                        UChar*          pucSigMap,
                                        UInt            uiWeightZeroCoeff);

  Void xAdjustChromaResidualRefBlock  ( XPel*           piResidualRef,
                                        Int             iStride,
                                        UChar*          pusSigMap,
                                        UInt            uiWeightZeroCoeff);

  ErrVal xCompensateMbAllModes        ( MbDataAccess&   rcMbDataAccess, 
                                        RefFrameList&   rcRefFrameList0, 
                                        RefFrameList&   rcRefFrameList1, 
                                        IntYuvMbBuffer* pcYuvMbBuffer);

  ErrVal updateMb(MbDataAccess&   rcMbDataAccess,
                  IntFrame*       pcMCFrame,
                  IntFrame*       pcPrdFrame,
                  ListIdx         eListPrd,
                  Int             iRefIdx); 

  ErrVal updateSubMb( B8x8Idx         c8x8Idx,
                      MbDataAccess&   rcMbDataAccess,
                      IntFrame*       pcMCFrame,
                      IntFrame*       pcPrdFrame,
                      ListIdx         eListPrd );

  Void xUpdateMb8x8Mode(    B8x8Idx         c8x8Idx,
                            MbDataAccess&   rcMbDataAccess,
                            IntFrame*       pcMCFrame,
                            IntFrame*       pcPrdFrame,
                            ListIdx         eListPrd );

  ErrVal updateDirectBlock( MbDataAccess&   rcMbDataAccess, 
                            IntFrame*       pcMCFrame,
                            IntFrame*       pcPrdFrame,
                            ListIdx         eListPrd,
                            Int             iRefIdx,                                             
                            B8x8Idx         c8x8Idx );

  Void xUpdateBlk( IntFrame* pcPrdFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D );
  Void xUpdateBlk( IntFrame* pcPrdFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx );

  Void xUpdateLuma( IntFrame* pcPrdFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, UShort *usWeight );
  Void xUpdateLuma( IntFrame* pcPrdFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx, UShort *usWeight );

  Void updateBlkAdapt( IntYuvPicBuffer* pcSrcBuffer, IntYuvPicBuffer* pcDesBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX, 
                                      UShort *usWeight);

  Void xUpdAdapt( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, 
                                    UInt uiSizeY, UInt uiSizeX, UShort weight, UShort wMax );

  __inline Void xUpdateChroma( IntYuvPicBuffer* pcSrcBuffer, IntYuvPicBuffer* pcDesBuffer,  LumaIdx cIdx, Mv cMv, 
    Int iSizeY, Int iSizeX, UShort *usWeight);
  Void xUpdateChroma( IntFrame* pcSrcFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx, UShort *usWeight );
  Void xUpdateChroma( IntFrame* pcSrcFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, UShort *usWeight );
  __inline Void xUpdateChromaPel( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX, UShort weight );

  ErrVal calcMvMb   (                   MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase );
  ErrVal calcMvSubMb( B8x8Idx c8x8Idx,  MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase );

  ErrVal compensateDirectBlock( MbDataAccess& rcMbDataAccess, YuvMbBuffer *pcRecBuffer, B8x8Idx c8x8Idx, Bool& rbValid, Bool bFaultTolerant, Bool bCalcMv = true );
  ErrVal compensateDirectBlock( MbDataAccess& rcMbDataAccess, IntYuvMbBuffer *pcRecBuffer, B8x8Idx c8x8Idx, RefFrameList& rcRefFrameListL0, RefFrameList& rcRefFrameListL1 );
  ErrVal initMb( UInt uiMbY, UInt uiMbX, MbDataAccess& rcMbDataAccess);



protected:
  Void xPredMb8x8Mode( MbDataAccess& rcMbDataAccess, YuvMbBuffer* pcRecBuffer );
  Void xPredMb8x8Mode( B8x8Idx c8x8Idx, MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame0, const IntFrame* pcRefFrame1, IntYuvMbBuffer* pcRecBuffer );
 
  Void xPredLuma(   YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D );
  Void xPredChroma( YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D );

  Void xPredLuma(   YuvMbBuffer* apcTarBuffer[2], Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx );
  Void xPredChroma( YuvMbBuffer* apcTarBuffer[2], Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx );


  Void xPredLuma  ( IntYuvMbBuffer* pcRecBuffer,      Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D );
  Void xPredLuma  ( IntYuvMbBuffer* apcTarBuffer[2],  Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx );
  Void xPredChroma( IntYuvMbBuffer* pcRecBuffer,      Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D );
  Void xPredChroma( IntYuvMbBuffer* apcTarBuffer[2],  Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx );


private:
    __inline Short xCorrectChromaMv( const MbDataAccess& rcMbDataAccess, PicType eRefPicType );
  __inline Void xGetMbPredData( MbDataAccess& rcMbDataAccess, MC8x8D& rcMC8x8D );
  __inline Void xGetBlkPredData( MbDataAccess& rcMbDataAccess, MC8x8D& rcMC8x8D, BlkMode eBlkMode );
  __inline Void xPredChromaPel( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX );
  __inline Void xPredChroma( YuvMbBuffer* pcDesBuffer, YuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);

  __inline Void xGetMbPredData  ( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame, IntMC8x8D& rcMC8x8D );
  __inline Void xGetBlkPredData ( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame, IntMC8x8D& rcMC8x8D, BlkMode eBlkMode );

  __inline Void xPredChromaPel  ( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX );
  __inline Void xPredChroma     ( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);

  __inline Void xGetMbPredData  ( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame0, const IntFrame* pcRefFrame1, IntMC8x8D& rcMC8x8D );
  __inline Void xGetBlkPredData ( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame0, const IntFrame* pcRefFrame1, IntMC8x8D& rcMC8x8D, BlkMode eBlkMode );



protected:
  QuarterPelFilter* m_pcQuarterPelFilter;
  Transform*        m_pcTransform;
  SampleWeighting* m_pcSampleWeighting;
  Mv   m_cMin;
  Mv   m_cMax;
  UInt m_uiMbInFrameY;
  UInt m_uiMbInFrameX;
  int m_curMbX;
  int m_curMbY;
};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

#define DMV_THRES   5


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MOTIONCOMPENSATION_H__820D6942_007B_42EA_838B_AC025E866DBE__INCLUDED_)

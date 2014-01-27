#if !defined(AFX_INTYUVPICBUFFER_H__5AB262CF_4876_47A2_97A8_5500F7416A8C__INCLUDED_)
#define AFX_INTYUVPICBUFFER_H__5AB262CF_4876_47A2_97A8_5500F7416A8C__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "DownConvert.h"



H264AVC_NAMESPACE_BEGIN



class IntYuvMbBuffer;
class MbDataCtrl;



class H264AVCCOMMONLIB_API IntYuvPicBuffer
{
public:
  IntYuvPicBuffer         ( YuvBufferCtrl& rcYuvBufferCtrl, PicType ePicType );
	virtual ~IntYuvPicBuffer();

  const Int     getLStride    ()                const { return m_iStride;   }
  const Int     getCStride    ()                const { return m_iStride>>1;}
  
  XPel*         getLumBlk     ()                      { return m_pPelCurrY; }
  XPel*         getCbBlk      ()                      { return m_pPelCurrU; }
  XPel*         getCrBlk      ()                      { return m_pPelCurrV; }

  Void          set4x4Block   ( LumaIdx cIdx )
  {
    m_pPelCurrY = m_pucYuvBuffer + m_rcBufferParam.getYBlk( cIdx );
    m_pPelCurrU = m_pucYuvBuffer + m_rcBufferParam.getUBlk( cIdx );
    m_pPelCurrV = m_pucYuvBuffer + m_rcBufferParam.getVBlk( cIdx );
  }

  // Hanke@RWTH
  Bool          isCurr4x4BlkNotZero ( LumaIdx cIdx );
  Bool          isLeft4x4BlkNotZero ( LumaIdx cIdx );
  Bool          isAbove4x4BlkNotZero( LumaIdx cIdx );
 
  XPel*         getYBlk       ( LumaIdx cIdx )        { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getYBlk( cIdx ); }
  XPel*         getUBlk       ( LumaIdx cIdx )        { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getUBlk( cIdx ); }
  XPel*         getVBlk       ( LumaIdx cIdx )        { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getVBlk( cIdx ); }
  
  XPel*         getMbLumAddr  ()                      { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbLum(); }
  XPel*         getMbCbAddr   ()                      { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbCb (); }
  XPel*         getMbCrAddr   ()                      { AOF_DBG(m_pucYuvBuffer); return m_pucYuvBuffer + m_rcBufferParam.getMbCr (); }

  const Int     getLWidth     ()                const { return m_rcBufferParam.getWidth ();    }
  const Int     getLHeight    ()                const { return m_rcBufferParam.getHeight();    }
  const Int     getCWidth     ()                const { return m_rcBufferParam.getWidth ()>>1; }
  const Int     getCHeight    ()                const { return m_rcBufferParam.getHeight()>>1; }
  
  const Int     getLXMargin   ()                const { return m_rcYuvBufferCtrl.getXMargin(); }
  const Int     getLYMargin   ()                const { return m_rcYuvBufferCtrl.getYMargin(); }
  const Int     getCXMargin   ()                const { return m_rcYuvBufferCtrl.getXMargin()>>1; }
  const Int     getCYMargin   ()                const { return m_rcYuvBufferCtrl.getYMargin()>>1; }

  Bool          isValid       ()                      { return NULL != m_pucYuvBuffer; }

  XPel*         getLumOrigin  ()                const { return m_pucYuvBuffer + m_rcYuvBufferCtrl.getLumOrigin( m_ePicType ); }
  XPel*         getCbOrigin   ()                const { return m_pucYuvBuffer + m_rcYuvBufferCtrl.getCbOrigin ( m_ePicType ); }
  XPel*         getCrOrigin   ()                const { return m_pucYuvBuffer + m_rcYuvBufferCtrl.getCrOrigin ( m_ePicType ); }

  
  ErrVal        loadFromPicBuffer       ( PicBuffer*        pcPicBuffer );
  ErrVal        storeToPicBuffer        ( PicBuffer*        pcPicBuffer );

  ErrVal        loadBuffer              ( IntYuvMbBuffer*   pcYuvMbBuffer );
  ErrVal        fillMargin              ();

  ErrVal        prediction              ( IntYuvPicBuffer*  pcSrcYuvPicBuffer, IntYuvPicBuffer*  pcMCPYuvPicBuffer );
  ErrVal        update                  ( IntYuvPicBuffer*  pcSrcYuvPicBuffer, IntYuvPicBuffer*  pcMCPYuvPicBuffer, UInt uiShift );
  ErrVal        inversePrediction       ( IntYuvPicBuffer*  pcSrcYuvPicBuffer, IntYuvPicBuffer*  pcMCPYuvPicBuffer );
  ErrVal        inverseUpdate           ( IntYuvPicBuffer*  pcSrcYuvPicBuffer, IntYuvPicBuffer*  pcMCPYuvPicBuffer, UInt uiShift );

  ErrVal        update                  ( IntYuvPicBuffer*  pcSrcYuvPicBuffer, IntYuvPicBuffer*  pcMCPYuvPicBuffer0, IntYuvPicBuffer*  pcMCPYuvPicBuffer1 );
  ErrVal        inverseUpdate           ( IntYuvPicBuffer*  pcSrcYuvPicBuffer, IntYuvPicBuffer*  pcMCPYuvPicBuffer0, IntYuvPicBuffer*  pcMCPYuvPicBuffer1 );

  ErrVal        copy                    ( IntYuvPicBuffer*  pcSrcYuvPicBuffer );
//	TMM_EC {{
  ErrVal        copy                    ( YuvPicBuffer*  pcSrcYuvPicBuffer );
//	TMM_EC }}
  ErrVal        copyMSB8BitsMB          ( IntYuvPicBuffer*  pcSrcYuvPicBuffer );
  ErrVal        setZeroMB               ();

  ErrVal        subtract                ( IntYuvPicBuffer*  pcSrcYuvPicBuffer0, IntYuvPicBuffer* pcSrcYuvPicBuffer1 );
  ErrVal        add                     ( IntYuvPicBuffer*  pcSrcYuvPicBuffer );

  ErrVal        addWeighted             ( IntYuvPicBuffer*  pcSrcYuvPicBuffer, Double dWeight );

  ErrVal        dumpLPS                 ( FILE* pFile );
  ErrVal        dumpHPS                 ( FILE* pFile, MbDataCtrl* pcMbDataCtrl );

  ErrVal        init                    ( XPel*&            rpucYuvBuffer );
  ErrVal        uninit                  ();
  Void          setZero                 ();
  ErrVal        clip                    ();

  ErrVal        loadFromFile8Bit        ( FILE* pFile );

  ErrVal        getSSD                  ( Double& dSSDY, Double& dSSDU, Double& dSSDV, PicBuffer* pcOrgPicBuffer );

    // TMM_ESS {
    ErrVal        upsampleResidual        ( DownConvert& rcDownConvert, ResizeParameters *pcParameters, MbDataCtrl* pcMbDataCtrl, Bool bClip );
    ErrVal        upsample                ( DownConvert& rcDownConvert, ResizeParameters *pcParameters, Bool bClip );
    // TMM_ESS }
  ErrVal        setNonZeroFlags         ( UShort* pusNonZeroFlags, UInt uiStride );

  ErrVal        clear();

	//-- JVT-R091
	ErrVal				smoothMbInside					();
	ErrVal				smoothMbTop							();
	ErrVal				smoothMbLeft						();
	//--
	// JVT-R057 LA-RDO{
	YuvBufferCtrl& getYuvBufferCtrl(){ return m_rcYuvBufferCtrl;}
	// JVT-R057 LA-RDO} 

    YuvBufferCtrl& getBufferCtrl()  { return m_rcYuvBufferCtrl; }//th dirty hack
    XPel* getBuffer()               { return m_pucYuvBuffer; }//th dirty hack II

    ErrVal loadBufferAndFillMargin( IntYuvPicBuffer *pcSrcYuvPicBuffer );
    ErrVal loadBuffer( IntYuvPicBuffer *pcSrcYuvPicBuffer );
protected:
    Void xCopyFillPlaneMargin( XPel *pucSrc, XPel *pucDest, Int iHeight, Int iWidth, Int iStride, Int iXMargin, Int iYMargin );//th
    Void xCopyPlane( XPel *pucSrc, XPel *pucDest, Int iHeight, Int iWidth, Int iStride );
  Void xFillPlaneMargin     ( XPel *pucDest, Int iHeight, Int iWidth, Int iStride, Int iXMargin, Int iYMargin );

protected:
  const YuvBufferCtrl::YuvBufferParameter&  m_rcBufferParam;
  YuvBufferCtrl&                            m_rcYuvBufferCtrl;

  Int             m_iStride;
  XPel*           m_pPelCurrY;
  XPel*           m_pPelCurrU;
  XPel*           m_pPelCurrV;
  
  XPel*           m_pucYuvBuffer;
  XPel*           m_pucOwnYuvBuffer;
  PicType         m_ePicType;
};



H264AVC_NAMESPACE_END



#endif // !defined(AFX_INTYUVPICBUFFER_H__5AB262CF_4876_47A2_97A8_5500F7416A8C__INCLUDED_)

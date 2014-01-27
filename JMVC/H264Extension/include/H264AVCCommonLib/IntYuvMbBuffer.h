#if !defined(AFX_INTYUVMBBUFFER_H__C9AFC3F1_5EEF_43A6_8105_C3BCD7B098FA__INCLUDED_)
#define AFX_INTYUVMBBUFFER_H__C9AFC3F1_5EEF_43A6_8105_C3BCD7B098FA__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN


class IntYuvPicBuffer;
class YuvMbBuffer;


#define OFFSET 19
class H264AVCCOMMONLIB_API IntYuvMbBuffer
{
public:
	IntYuvMbBuffer();
	virtual ~IntYuvMbBuffer();

  XPel*     getLumBlk     ()                      { return m_pPelCurrY; }
  XPel*     getCbBlk      ()                      { return m_pPelCurrU; }
  XPel*     getCrBlk      ()                      { return m_pPelCurrV; }

  Void      set4x4Block   ( LumaIdx cIdx )
  {
    m_pPelCurrY = &m_aucYuvBuffer[   MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2)];
    m_pPelCurrU = &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)];
    m_pPelCurrV = &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH + 16 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)];
  }

  const Int getLStride    ()                const { return MB_BUFFER_WIDTH;}
  const Int getCStride    ()                const { return MB_BUFFER_WIDTH;}

  XPel*     getYBlk       ( LumaIdx cIdx )        { return &m_aucYuvBuffer[   MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2)]; }
  XPel*     getUBlk       ( LumaIdx cIdx )        { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)]; }
  XPel*     getVBlk       ( LumaIdx cIdx )        { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH + 16 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)]; }
  XPel*     getCBlk       ( ChromaIdx cIdx )      { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2) + 12*cIdx.plane()]; }

  
  XPel*     getMbLumAddr  ()                      { return &m_aucYuvBuffer[   MB_BUFFER_WIDTH +  4]; }
  XPel*     getMbCbAddr   ()                      { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH +  4]; }
  XPel*     getMbCrAddr   ()                      { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH + 16];  }

  const Int getLWidth     ()                const { return 16; }
  const Int getLHeight    ()                const { return 16; }
  const Int getCWidth     ()                const { return 8; }
  const Int getCHeight    ()                const { return 8; }


  Void loadBuffer           ( YuvMbBuffer* pcSrcBuffer );
  Void loadBuffer           ( IntYuvPicBuffer*  pcSrcBuffer );

  Void loadLuma             ( IntYuvMbBuffer&   rcSrcBuffer, LumaIdx c4x4Idx);
  Void loadLuma             ( IntYuvMbBuffer&   rcSrcBuffer, B8x8Idx c8x8Idx);
  Void loadLuma             ( IntYuvMbBuffer&   rcSrcBuffer );
  Void loadChroma           ( IntYuvMbBuffer&   rcSrcBuffer );

  Void loadIntraPredictors  ( IntYuvPicBuffer*  pcSrcBuffer );
  Void loadIntraPredictors( YuvPicBuffer* pcSrcBuffer );

  Void setAllSamplesToZero  ();

  Void  add         ( IntYuvMbBuffer& rcIntYuvMbBuffer );
  Void  subtract    ( IntYuvMbBuffer& rcIntYuvMbBuffer );
  Void  clip        ();
  Bool  isZero      ();


protected:
  XPel* m_pPelCurrY;
  XPel* m_pPelCurrU;
  XPel* m_pPelCurrV;
  XPel  m_aucYuvBuffer[MB_BUFFER_WIDTH*29];
};

class H264AVCCOMMONLIB_API IntYuvMbBufferExtension : public IntYuvMbBuffer
{
public:
  Void loadSurrounding( IntYuvPicBuffer* pcSrcBuffer );
  Void loadSurrounding( YuvPicBuffer* pcSrcBuffer );

  Void mergeFromLeftAbove ( LumaIdx cIdx, Bool bCornerMbPresent );
  Void mergeRightBelow    ( LumaIdx cIdx, Bool bCornerMbPresent );
  Void mergeFromRightAbove( LumaIdx cIdx, Bool bCornerMbPresent );
  Void mergeLeftBelow     ( LumaIdx cIdx, Bool bCornerMbPresent );

  Void copyFromBelow      ( LumaIdx cIdx );
  Void copyFromLeft       ( LumaIdx cIdx );
  Void copyFromAbove      ( LumaIdx cIdx );
  Void copyFromRight      ( LumaIdx cIdx );

  Void copyFromLeftAbove  ( LumaIdx cIdx );
  Void copyFromRightAbove ( LumaIdx cIdx );
  Void copyFromLeftBelow  ( LumaIdx cIdx );
  Void copyFromRightBelow ( LumaIdx cIdx );

  Void xFill( LumaIdx cIdx, XPel cY, XPel cU, XPel cV );
  Void xMerge( Int xDir, Int yDir, UInt uiSize, XPel *puc, Int iStride, Bool bPresent );

};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_INTYUVMBBUFFER_H__C9AFC3F1_5EEF_43A6_8105_C3BCD7B098FA__INCLUDED_)

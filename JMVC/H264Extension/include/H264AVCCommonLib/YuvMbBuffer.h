#if !defined(AFX_YUVMBBUFFER_H__F4C18313_4C6D_4412_96E3_BADC7F1AE13F__INCLUDED_)
#define AFX_YUVMBBUFFER_H__F4C18313_4C6D_4412_96E3_BADC7F1AE13F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN


class IntYuvMbBuffer;

class H264AVCCOMMONLIB_API YuvMbBuffer
{
public:
	YuvMbBuffer();
	virtual ~YuvMbBuffer();

  Pel* getYBlk( LumaIdx cIdx )
  {
    return &m_aucYuvBuffer[   MB_BUFFER_WIDTH + 4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2)];
  }
  Pel* getLumBlk()       { return m_pPelCurr; }
  const Int getLStride()      const { return MB_BUFFER_WIDTH;}
  const Int getCStride()      const { return MB_BUFFER_WIDTH;}

  Pel* getUBlk( LumaIdx cIdx )
  {
    return &m_aucYuvBuffer[18*MB_BUFFER_WIDTH + 4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)];
  }
  Pel* getVBlk( LumaIdx cIdx )
  {
    return &m_aucYuvBuffer[18*MB_BUFFER_WIDTH + 16 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)];
  }
  Void set4x4Block( LumaIdx cIdx )
  {
    m_pPelCurr = &m_aucYuvBuffer[   MB_BUFFER_WIDTH + 4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2)];
  }

  Pel* getMbLumAddr()   { return &m_aucYuvBuffer[   MB_BUFFER_WIDTH +  4]; }
  Pel* getMbCbAddr()    { return &m_aucYuvBuffer[18*MB_BUFFER_WIDTH +  4]; }
  Pel* getMbCrAddr()    { return &m_aucYuvBuffer[18*MB_BUFFER_WIDTH + 16];  }

  const Int getLWidth()       const { return 16; }
  const Int getLHeight()      const { return 16; }
  const Int getCWidth()       const { return 8; }
  const Int getCHeight()      const { return 8; }

  Void loadIntraPredictors( YuvPicBuffer* pcSrcBuffer );
  Void loadBuffer( IntYuvMbBuffer* pcSrcBuffer );
  Void loadBufferClip( IntYuvMbBuffer* pcSrcBuffer );
  Void  setZero();

protected:
  Pel* m_pPelCurr;
  Pel m_aucYuvBuffer[MB_BUFFER_WIDTH*26];
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_YUVMBBUFFER_H__F4C18313_4C6D_4412_96E3_BADC7F1AE13F__INCLUDED_)

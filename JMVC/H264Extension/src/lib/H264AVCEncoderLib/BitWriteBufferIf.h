#if !defined(AFX_BITWRITEBUFFERIF_H__7F264021_5671_490D_8B36_571A1F6E3E38__INCLUDED_)
#define AFX_BITWRITEBUFFERIF_H__7F264021_5671_490D_8B36_571A1F6E3E38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



H264AVC_NAMESPACE_BEGIN


class BitWriteBufferIf
{
protected:
  BitWriteBufferIf() {}
	virtual ~BitWriteBufferIf() {}

public:
  virtual ErrVal write( UInt uiBits, UInt uiNumberOfBits = 1) = 0;

  virtual UInt getNumberOfWrittenBits() = 0 ;

  virtual ErrVal writeAlignZero() = 0;
  virtual ErrVal writeAlignOne() = 0;
  virtual ErrVal flushBuffer() = 0;
  virtual ErrVal samples( const Pel* pPel, UInt uiNumberOfSamples ) = 0;
  virtual ErrVal getLastByte(UChar &uiLastByte, UInt &uiLastBitPos) = 0; //FIX_FRAG_CAVLC
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_BITWRITEBUFFERIF_H__7F264021_5671_490D_8B36_571A1F6E3E38__INCLUDED_)

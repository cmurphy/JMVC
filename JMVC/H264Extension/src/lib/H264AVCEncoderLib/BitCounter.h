#if !defined(AFX_BITCOUNTER_H__5062A2A9_6E04_4211_A337_9BDED6D88A82__INCLUDED_)
#define AFX_BITCOUNTER_H__5062A2A9_6E04_4211_A337_9BDED6D88A82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BitWriteBufferIf.h"


H264AVC_NAMESPACE_BEGIN

class BitCounter :
public BitWriteBufferIf
{
public:
  static ErrVal create( BitCounter*& rpcBitCounter );
  ErrVal destroy();

  ErrVal init()                                           { m_uiBitCounter = 0; return Err::m_nOK; }
  ErrVal uninit()                                         { m_uiBitCounter = 0; return Err::m_nOK; }

	BitCounter();
	virtual ~BitCounter();
  ErrVal write( UInt uiBits, UInt uiNumberOfBits = 1)     { m_uiBitCounter += uiNumberOfBits; return Err::m_nOK; }

  ErrVal samples( const Pel* pPel, UInt uiNumberOfSamples ) { m_uiBitCounter+=8*uiNumberOfSamples; return Err::m_nERR; }

  UInt getNumberOfWrittenBits()                           { return m_uiBitCounter; }

  Bool isByteAligned()                                    { return (0 == (m_uiBitCounter & 0x03)); }
  Bool isWordAligned()                                    { return (0 == (m_uiBitCounter & 0x1f)); }

  ErrVal writeAlignZero()                                 { return Err::m_nERR; }
  ErrVal writeAlignOne()                                  { return Err::m_nERR; }
  ErrVal flushBuffer()                                    { m_uiBitCounter = 0; return Err::m_nOK; }

  ErrVal   getLastByte(UChar &uiLastByte, UInt &uiLastBitPos) { return Err::m_nERR;} //FIX_FRAG_CAVLC

private:
  UInt m_uiBitCounter;
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_BITCOUNTER_H__5062A2A9_6E04_4211_A337_9BDED6D88A82__INCLUDED_)

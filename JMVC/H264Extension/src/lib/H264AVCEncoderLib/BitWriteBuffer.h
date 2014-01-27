#if !defined(AFX_BITWRITEBUFFER_H__F1308E37_7998_4953_9F78_FF6A3DBC22B5__INCLUDED_)
#define AFX_BITWRITEBUFFER_H__F1308E37_7998_4953_9F78_FF6A3DBC22B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BitWriteBufferIf.h"

H264AVC_NAMESPACE_BEGIN

class BitWriteBuffer :
public BitWriteBufferIf
{
protected:
	BitWriteBuffer();
	virtual ~BitWriteBuffer();

public:
  static ErrVal create( BitWriteBuffer*& rpcBitWriteBuffer );
  ErrVal destroy();

  ErrVal init();
  ErrVal uninit() { return init(); }

  ErrVal initPacket( UInt32* pulBits, UInt uiPacketLength );

  ErrVal write( UInt uiBits, UInt uiNumberOfBits = 1);

  UInt getNumberOfWrittenBits() { return  m_uiBitsWritten; }

  ErrVal samples( const Pel* pPel, UInt uiNumberOfSamples );

  ErrVal flushBuffer();
  ErrVal writeAlignZero();
  ErrVal writeAlignOne();

  ErrVal loadContent( BitWriteBuffer& rcBitWriteBuffer );

  ErrVal getLastByte(UChar &uiLastByte, UInt &uiLastBitPos);//FIX_FRAG_CAVLC

protected:
  UInt32  xSwap( UInt32 ul )
  {
    // heiko.schwarz@hhi.fhg.de: support for BSD systems as proposed by Steffen Kamp [kamp@ient.rwth-aachen.de]
#ifdef MSYS_BIG_ENDIAN
    return ul;
#else
    UInt32 ul2;

    ul2  = ul>>24;
    ul2 |= (ul>>8) & 0x0000ff00;
    ul2 |= (ul<<8) & 0x00ff0000;
    ul2 |= ul<<24;

    return ul2;
#endif
  }

protected:
  UInt   m_uiDWordsLeft;
  UInt   m_uiBitsWritten;
  Int    m_iValidBits;
  UInt32 m_ulCurrentBits;      // Dong: Use 32-bit fixed length
  UInt32*  m_pulStreamPacket;  // Dong: Use 32-bit fixed length
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_BITWRITEBUFFER_H__F1308E37_7998_4953_9F78_FF6A3DBC22B5__INCLUDED_)

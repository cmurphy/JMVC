#if !defined(AFX_BITREADBUFFER_H__F1308E37_7998_4953_9F78_FF6A3DBC22B5__INCLUDED_)
#define AFX_BITREADBUFFER_H__F1308E37_7998_4953_9F78_FF6A3DBC22B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

H264AVC_NAMESPACE_BEGIN

class BitReadBuffer
{
public:
  class ReadStop
  {
  };

protected:
	BitReadBuffer();
	virtual ~BitReadBuffer();

public:
  static ErrVal create( BitReadBuffer*& rpcBitReadBuffer );
  ErrVal destroy();
  Void setModeCabac()
  { // problem with cabac, cause cabac decoder uses stop bit + trailing stuffing bits
    m_uiBitsLeft = 8*((m_uiBitsLeft+7)/8);
  }
  ErrVal init() { return Err::m_nOK; }
  ErrVal uninit() { return Err::m_nOK; }

  ErrVal initPacket( UInt32* puiBits, UInt uiBitsInPacket);

  ErrVal get  ( UInt& ruiBits, UInt uiNumberOfBits );
  ErrVal get  ( UInt& ruiBits);
  Void   show ( UInt& ruiBits, UInt uiNumberOfBits = 1 );
  ErrVal flush( UInt uiNumberOfBits );

  ErrVal samples( Pel* pPel, UInt uiNumberOfSamples );

  Int getBitsUntilByteAligned()     {  return m_iValidBits & (0x7);  }
  Bool isWordAligned()              {  return( 0 == (m_iValidBits & (0x1f)) );  }
  Bool isByteAligned()              {  return( 0 == (m_iValidBits & (0x7)) );   }

  Bool isValid();
  UInt getBytesLeft()               {  return(m_uiBitsLeft/8); }//JVT-P031
  UInt getBitsLeft()                {  return(m_uiBitsLeft); }//JVT-P031

private:
  __inline Void xReadNextWord();

  UInt32 xSwap( UInt32 ul )
  {
    // heiko.schwarz@hhi.fhg.de: support for BSD systems as proposed by Steffen Kamp [kamp@ient.rwth-aachen.de]
#ifdef MSYS_BIG_ENDIAN
    return ul;
#else
    UInt32 ul2;

    ul2  = (ul>>24)& 0x000000ff;
    ul2 |= (ul>>8) & 0x0000ff00;
    ul2 |= (ul<<8) & 0x00ff0000;
    ul2 |= (ul<<24)& 0xff000000;

    return ul2;
#endif
  }

protected:
  UInt   m_uiDWordsLeft;
  UInt   m_uiBitsLeft;
  Int    m_iValidBits;
  UInt32 m_ulCurrentBits;    // Dong: Use 32-bit fixed length
  UInt   m_uiNextBits;
  UInt32* m_pulStreamPacket; // Dong: Use 32-bit fixed length
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_BITREADBUFFER_H__F1308E37_7998_4953_9F78_FF6A3DBC22B5__INCLUDED_)

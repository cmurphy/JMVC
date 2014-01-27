#ifndef _MVC_SCALABLE_MODIFY_CODE_
#define _MVC_SCALABLE_MODIFY_CODE_

#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/Sei.h"

//class ScalableCodeIf
//{
//protected:
//	ScalableCodeIf()	{}
//	virtual ~ScalableCodeIf()	{}
//
//public:
//	virtual ErrVal WriteUVLC( UInt uiValue ) = 0;
//	virtual ErrVal WriteFlag( Bool bFlag ) = 0;
//	virtual ErrVal WriteCode( UInt uiValue, UInt uiLength ) = 0;
//	virtual SEICode	( h264::SEI::ScalableSei* pcScalableSei, ScalableCodeIf *pcScalableCodeIf ) = 0;
//	virtual UInt	 getNumberOfWrittenBits() = 0;
//};

class MVCScalableModifyCode// : public ScalableCodeIf
{
public:
	MVCScalableModifyCode();
	virtual ~MVCScalableModifyCode();

public:
	//static ErrVal Create( ScalableModifyCode* pcScalableModifyCode );
	ErrVal Destroy( Void );
	ErrVal init( ULong* pulStream );
	ErrVal WriteUVLC( UInt uiValue );
	ErrVal WriteFlag( Bool bFlag );
	ErrVal WriteCode( UInt uiValue, UInt uiLength );
	ErrVal SEICode	( h264::SEI::ViewScalabilityInfoSei* pcViewScalInfoSei, MVCScalableModifyCode *pcScalableModifyCode );
	UInt	 getNumberOfWrittenBits() { return m_uiBitsWritten; }
	ErrVal Write		( UInt uiBits, UInt uiNumberOfBits );
	ErrVal WritePayloadHeader ( enum h264::SEI::MessageType eType, UInt uiSize );
	ErrVal WriteTrailingBits ();
	ErrVal WriteAlignZero ();
	ErrVal flushBuffer();
	ErrVal ConvertRBSPToPayload( UChar* m_pucBuffer, UChar pucStreamPacket[], UInt& uiBits, UInt uiHeaderBytes );
   
protected:
	ULong  xSwap( ULong ul )
	{
		// heiko.schwarz@hhi.fhg.de: support for BSD systems as proposed by Steffen Kamp [kamp@ient.rwth-aachen.de]
#ifdef MSYS_BIG_ENDIAN
		return ul;
#else
		UInt ul2;

		ul2  = ul>>24;
		ul2 |= (ul>>8) & 0x0000ff00;
		ul2 |= (ul<<8) & 0x00ff0000;
		ul2 |= ul<<24;

		return ul2;
#endif
	}

	BinData *m_pcBinData;
	ULong *m_pulStreamPacket;
	UInt m_uiBitCounter;
	UInt m_uiPosCounter;
	UInt m_uiDWordsLeft;
	UInt m_uiBitsWritten;
	UInt m_iValidBits;
	ULong m_ulCurrentBits;
	UInt m_uiCoeffCost;
};

#endif





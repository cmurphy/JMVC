

#ifndef _MVC_SCALABLE_TEST_CODE_
#define _MVC_SCALABLE_TEST_CODE_

#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/Sei.h"

class MVCScalableTestCode// : public ScalableCodeIf
{
public:
	MVCScalableTestCode();
	virtual ~MVCScalableTestCode();

public:
	//static ErrVal Create( ScalableTestCode*& rpcScalableTestCode );
	ErrVal Destroy();
	ErrVal init() { m_uiBitCounter = 0; return Err::m_nOK; }
	ErrVal Uninit()	{ m_uiBitCounter = 0; return Err::m_nOK; }
	ErrVal SEICode	( h264::SEI::ViewScalabilityInfoSei* pcViewScalInfoSei, MVCScalableTestCode *pcScalableTestCode );
	ErrVal WriteUVLC( UInt uiValue );
	ErrVal WriteFlag( Bool bFlag ) { m_uiBitCounter++; return Err::m_nOK; }
	ErrVal WriteCode( UInt uiValue, UInt uiLength ) { m_uiBitCounter += uiLength; return Err::m_nOK; }

	UInt getNumberOfWrittenBits() { return m_uiBitCounter; }

protected:
	UInt m_uiBitCounter;

};




#endif




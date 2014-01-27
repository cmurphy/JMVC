#include "H264AVCDecoderLib.h"
#include "H264AVCCommonLib/CabacTables.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/CabacContextModel.h"
#include "BitReadBuffer.h"
#include "CabaDecoder.h"
#include "DecError.h"


#if 0 // FAST_CABAC
#define RNOKCABAC( exp ) exp
#define ROTRSCABAC( exp, err ) ROTVS(exp)
#define ROFRSCABAC( exp, err ) ROFVS(exp)
#else
#define RNOKCABAC( exp )       RNOK(exp)
#define ROTRSCABAC( exp, err ) ROTRS(exp,err)
#define ROFRSCABAC( exp, err ) ROFRS(exp,err)
#endif


H264AVC_NAMESPACE_BEGIN


CabaDecoder::CabaDecoder() :
  m_pcBitReadBuffer( NULL ),
  m_uiRange( 0 ),
  m_uiValue( 0 ),
  m_uiWord( 0 ),
  m_uiBitsLeft( 0 )
{
}


CabaDecoder::~CabaDecoder()
{

}


ErrVal CabaDecoder::init( BitReadBuffer* pcBitReadBuffer )
{
  ROT( NULL == pcBitReadBuffer )

  m_pcBitReadBuffer = pcBitReadBuffer;
  return Err::m_nOK;
}



__inline Void CabaDecoder::xReadBit( UInt& ruiValue )
{
  if( 0 == m_uiBitsLeft-- )
  {
    m_pcBitReadBuffer->get( m_uiWord, 8 );
    m_uiBitsLeft = 7;
  }
  ruiValue += ruiValue + ((m_uiWord >> 7)&1);
  m_uiWord <<= 1;
}



ErrVal CabaDecoder::finish()
{
  return Err::m_nOK;
}

ErrVal CabaDecoder::start()
{
  m_uiRange     = HALF-2;
  m_uiValue     = 0;
  m_uiWord      = 0;
  m_uiBitsLeft  = 0;


  RNOK( m_pcBitReadBuffer->flush( m_pcBitReadBuffer->getBitsUntilByteAligned() ) );
  m_pcBitReadBuffer->setModeCabac();

  while( ! m_pcBitReadBuffer->isWordAligned() && ( 8 > m_uiBitsLeft) )
  {
    UInt uiByte;
    m_pcBitReadBuffer->get( uiByte, 8 );
    m_uiWord <<= 8;
    m_uiWord += uiByte;
    m_uiBitsLeft += 8;
  }

  m_uiWord <<= 8-m_uiBitsLeft;

  for( UInt n = 0; n < B_BITS-1; n++ )
  {
    xReadBit( m_uiValue );
  }

  return Err::m_nOK;
}



ErrVal CabaDecoder::getTerminateBufferBit( UInt& ruiBit )
{
  UInt uiRange = m_uiRange-2;
  UInt uiValue = m_uiValue;

  DTRACE_V (g_nSymbolCounter[g_nLayer]++);
  DTRACE_T ("  ");
  DTRACE_X (m_uiRange);


  if( uiValue >= uiRange )
  {
    ruiBit = 1;
  }
  else
  {
    ruiBit = 0;

	  while( uiRange < QUARTER )
	  {
		  uiRange += uiRange;
      xReadBit( uiValue );
	  }

    m_uiRange = uiRange;
    m_uiValue = uiValue;
  }

  DTRACE_T ("  -  ");
  DTRACE_V (ruiBit);
  DTRACE_N;
  return Err::m_nOK;
}


ErrVal CabaDecoder::uninit()
{
  m_pcBitReadBuffer = NULL;
  m_uiRange = 0;
  m_uiValue = 0;
  return Err::m_nOK;
}




ErrVal CabaDecoder::getSymbol( UInt& ruiSymbol, CabacContextModel& rcCCModel )
{
  UInt uiRange = m_uiRange;
  UInt uiValue = m_uiValue;

  DTRACE_V (g_nSymbolCounter[g_nLayer]++);
  DTRACE_T ("  ");
  DTRACE_X (m_uiRange);
  DTRACE_T ("  ");
  DTRACE_V (rcCCModel.getState());
  DTRACE_T ("  ");
  DTRACE_V (rcCCModel.getMps());

  {

    UInt uiLPS;

    uiLPS = g_aucLPSTable64x4[rcCCModel.getState()][(uiRange>>6) & 0x03];
		uiRange -= uiLPS;

		if( uiValue < uiRange )
    {
			ruiSymbol = rcCCModel.getMps();
  		rcCCModel.setState( g_aucACNextStateMPS64[ rcCCModel.getState() ] );
    }
    else
    {
      uiValue -= uiRange;
      uiRange  = uiLPS;

			ruiSymbol = 1 - rcCCModel.getMps();

      if( ! rcCCModel.getState() )
      {
				rcCCModel.toggleMps();
      }

			rcCCModel.setState( g_aucACNextStateLPS64[ rcCCModel.getState() ] );
    }
  }

  DTRACE_T ("  -  ");
  DTRACE_V (ruiSymbol);
  DTRACE_N;

  while( uiRange < QUARTER )
  {
    uiRange += uiRange;
    xReadBit( uiValue );
  }

  m_uiRange = uiRange;
  m_uiValue = uiValue;

  return Err::m_nOK;
}


ErrVal CabaDecoder::getEpSymbol( UInt& ruiSymbol )
{
  DTRACE_V (g_nSymbolCounter[g_nLayer]++);
  DTRACE_T ("  ");
  DTRACE_X (m_uiRange);

  UInt uiValue = m_uiValue;

  xReadBit( uiValue );

	if( uiValue >= m_uiRange )
	{
		ruiSymbol = 1;
		uiValue -= m_uiRange;
	}
	else
  {
		ruiSymbol = 0;
  }

  DTRACE_T ("  -  ");
  DTRACE_V (ruiSymbol);
  DTRACE_N;

  m_uiValue = uiValue;

  return Err::m_nOK;
}




ErrVal CabaDecoder::getExGolombLevel( UInt& ruiSymbol, CabacContextModel& rcCCModel  )
{
  UInt uiSymbol;
  UInt uiCount = 0;
  do
  {
    RNOKCABAC( getSymbol( uiSymbol, rcCCModel ) );
    uiCount++;
  }
  while( uiSymbol && (uiCount != 13));

  ruiSymbol = uiCount-1;

	if( uiSymbol )
  {
    RNOKCABAC( getEpExGolomb( uiSymbol, 0 ) );
    ruiSymbol += uiSymbol+1;
  }

  return Err::m_nOK;
}



ErrVal CabaDecoder::getExGolombMvd( UInt& ruiSymbol, CabacContextModel* pcCCModel, UInt uiMaxBin )
{
  UInt uiSymbol;

  RNOKCABAC( getSymbol( ruiSymbol, pcCCModel[0] ) );

  ROTRSCABAC( 0 == ruiSymbol, Err::m_nOK );

  RNOKCABAC( getSymbol( uiSymbol, pcCCModel[1] ) );

  ruiSymbol = 1;

  ROTRSCABAC( 0 == uiSymbol, Err::m_nOK );

  pcCCModel += 2;
  UInt uiCount = 2;

  do
  {
    if( uiMaxBin == uiCount )
    {
      pcCCModel++;
    }
    RNOKCABAC( getSymbol( uiSymbol, *pcCCModel ) );
    uiCount++;
  }
  while( uiSymbol && (uiCount != 8));

  ruiSymbol = uiCount-1;

	if( uiSymbol )
  {
    RNOKCABAC( getEpExGolomb( uiSymbol, 3 ) );
    ruiSymbol += uiSymbol+1;
  }

  return Err::m_nOK;
}


ErrVal CabaDecoder::getEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;


  while( uiBit )
  {
    RNOKCABAC( getEpSymbol( uiBit ) );
    uiSymbol += uiBit << uiCount++;
  }

  uiCount--;
	while( uiCount-- )
  {
    RNOKCABAC( getEpSymbol( uiBit ) );
  	uiSymbol += uiBit << uiCount;
  }

  ruiSymbol = uiSymbol;
  return Err::m_nOK;
}


ErrVal CabaDecoder::getUnaryMaxSymbol( UInt& ruiSymbol, CabacContextModel* pcCCModel, Int iOffset, UInt uiMaxSymbol )
{
  RNOKCABAC( getSymbol( ruiSymbol, pcCCModel[0] ) );

  ROTRSCABAC( 0 == ruiSymbol, Err::m_nOK );
  ROTRSCABAC( 1 == uiMaxSymbol, Err::m_nOK );

  UInt uiSymbol = 0;
  UInt uiCont;

  do
  {
    RNOKCABAC( getSymbol( uiCont, pcCCModel[ iOffset ] ) );
    uiSymbol++;
  }
  while( uiCont && (uiSymbol < uiMaxSymbol-1) );

  if( uiCont && (uiSymbol == uiMaxSymbol-1) )
  {
    uiSymbol++;
  }

  ruiSymbol = uiSymbol;
  return Err::m_nOK;
}


ErrVal CabaDecoder::getUnarySymbol( UInt& ruiSymbol, CabacContextModel* pcCCModel, Int iOffset )
{
  RNOKCABAC( getSymbol( ruiSymbol, pcCCModel[0] ) );

  ROTRSCABAC( 0 == ruiSymbol, Err::m_nOK );

  UInt uiSymbol = 0;
  UInt uiCont;

  do
  {
    RNOKCABAC( getSymbol( uiCont, pcCCModel[ iOffset ] ) );
    uiSymbol++;
  }
  while( uiCont );

  ruiSymbol = uiSymbol;
  return Err::m_nOK;
}

H264AVC_NAMESPACE_END


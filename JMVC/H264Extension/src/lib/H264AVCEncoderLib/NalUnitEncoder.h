#if !defined(AFX_NALUNITENCODER_H__DA2EE2CC_46F5_4F11_B046_FA18CD441B65__INCLUDED_)
#define AFX_NALUNITENCODER_H__DA2EE2CC_46F5_4F11_B046_FA18CD441B65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCEncoder.h"
#include "H264AVCCommonLib/Sei.h"



H264AVC_NAMESPACE_BEGIN


class BitWriteBuffer;


class NalUnitEncoder
{
protected:
	NalUnitEncoder          ();
	virtual ~NalUnitEncoder ();

public:
  static ErrVal create    ( NalUnitEncoder*&            rpcNalUnitEncoder );
  ErrVal        destroy   ();

  ErrVal init             ( BitWriteBuffer*             pcBitWriteBuffer,
                            HeaderSymbolWriteIf*        pcHeaderSymbolWriteIf,
                            HeaderSymbolWriteIf*        pcHeaderSymbolTestIf  );
  ErrVal uninit           ();

  ErrVal initNalUnit      ( BinDataAccessor*            pcBinDataAccessor );
  ErrVal closeNalUnit     ( UInt&                       ruiBits );

  ErrVal write            ( const SequenceParameterSet& rcSPS );
  ErrVal write            ( const PictureParameterSet&  rcPPS );
  ErrVal write            ( const SliceHeader&          rcSH  );
  ErrVal write            ( SEI::MessageList&           rcSEIMessageList );

  ErrVal writeNesting     ( SEI::MessageList&           rcSEIMessageList );//SEI LSJ
protected:
  ErrVal xConvertRBSPToPayload( UInt& ruiBytesWritten,
                                UInt  uiHeaderBytes );
  ErrVal xWriteTrailingBits   ( UInt  uiFixedNumberOfBits = 0);

protected:
  Bool                  m_bIsUnitActive;
  BitWriteBuffer*       m_pcBitWriteBuffer;
  HeaderSymbolWriteIf*  m_pcHeaderSymbolWriteIf;
  HeaderSymbolWriteIf*  m_pcHeaderSymbolTestIf;
  BinDataAccessor*      m_pcBinDataAccessor;
  UChar*                m_pucBuffer;
  UChar*                m_pucTempBuffer;
  UInt                  m_uiPacketLength;
  NalUnitType           m_eNalUnitType;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_NALUNITENCODER_H__DA2EE2CC_46F5_4F11_B046_FA18CD441B65__INCLUDED_)

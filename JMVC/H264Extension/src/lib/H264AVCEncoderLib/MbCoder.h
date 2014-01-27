#if !defined(AFX_MBCODER_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_)
#define AFX_MBCODER_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MbSymbolWriteIf.h"
#include "RateDistortionIf.h"


H264AVC_NAMESPACE_BEGIN

class MbCoder
{
protected:
	MbCoder();
	virtual ~MbCoder();

public:
  static ErrVal create( MbCoder*& rpcMbCoder );
  ErrVal destroy();

  ErrVal initSlice( const SliceHeader& rcSH,
                    MbSymbolWriteIf* pcMbSymbolWriteIf,
                    RateDistortionIf* pcRateDistortionIf );


  ErrVal uninit();

  ErrVal  encode            ( MbDataAccess& rcMbDataAccess,
                              MbDataAccess* pcMbDataAccessBase,
                              Int									iSpatialScalabilityType,
                              Bool          bTerminateSlice, 
							  Bool          bSendTerminateSlice);

  ErrVal  encodeMotion      ( MbDataAccess& rcMbDataAccess,
                              MbDataAccess* pcMbDataAccessBase );
  UInt    getBitCount       ()  { return m_pcMbSymbolWriteIf->getNumberOfWrittenBits(); }

protected:
  ErrVal xWriteIntraPredModes ( MbDataAccess& rcMbDataAccess );
  
  
  
  ErrVal xWriteMotionPredFlags_FGS( MbDataAccess& rcMbDataAccess,
                                    MbDataAccess* pcMbDataAccessBase,
                                    MbMode        eMbMode,
                                    ListIdx       eLstIdx );
  ErrVal xWriteMotionPredFlags    ( MbDataAccess& rcMbDataAccess,
                                    MbMode        eMbMode,
                                    ListIdx       eLstIdx );
  ErrVal xWriteReferenceFrames    ( MbDataAccess& rcMbDataAccess,
                                    MbMode        eMbMode,
                                    ListIdx       eLstIdx );
  ErrVal xWriteMotionVectors      ( MbDataAccess& rcMbDataAccess,
                                    MbMode        eMbMode,
                                    ListIdx       eLstIdx );
  
  //-- JVT-R091
	ErrVal xWriteTextureInfo    ( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase, const MbTransformCoeffs& rcMbTCoeff, Bool bTrafo8x8Flag );
	//--
  ErrVal xWriteBlockMv        ( MbDataAccess& rcMbDataAccess, B8x8Idx c8x8Idx, ListIdx eLstIdx );


  ErrVal xScanLumaIntra16x16  ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, Bool bAC );
  ErrVal xScanLumaBlock       ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, LumaIdx cIdx );
  ErrVal xScanChromaDc        ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff );
  ErrVal xScanChromaAcU       ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff );
  ErrVal xScanChromaAcV       ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff );
  ErrVal xScanChromaBlocks    ( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, UInt uiChromCbp );

protected:
  MbSymbolWriteIf* m_pcMbSymbolWriteIf;
  RateDistortionIf* m_pcRateDistortionIf;

  Bool m_bInitDone;
  Bool  m_bCabac;
  Bool  m_bPrevIsSkipped;
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_MBCODER_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_)

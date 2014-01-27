#if !defined(AFX_CABACWRITER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_)
#define AFX_CABACWRITER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MbSymbolWriteIf.h"
#include "H264AVCCommonLib/CabacContextModel2DBuffer.h"
#include "BitWriteBufferIf.h"
#include "CabaEncoder.h"
#include "H264AVCCommonLib/ContextTables.h"


H264AVC_NAMESPACE_BEGIN

class CabacWriter :
public MbSymbolWriteIf
, private CabaEncoder
{
protected:
	CabacWriter();
	virtual ~CabacWriter();

public:
  static ErrVal create( CabacWriter*& rpcCabacWriter );
  ErrVal destroy();

  ErrVal init( BitWriteBufferIf* pcBitWriteBufferIf );
  ErrVal uninit();

  ErrVal  startSlice( const SliceHeader& rcSliceHeader );
  ErrVal startFragment(); //JVT-P031
  ErrVal  getLastByte(UChar &uiLastByte, UInt &uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  setFirstBits(UChar ucByte,UInt uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  finishSlice();

  //===== BCBP =====
  Bool    RQencodeBCBP_4x4        ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    LumaIdx         cIdx );
  Bool    RQencodeBCBP_ChromaDC   ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx );
  Bool    RQencodeBCBP_ChromaAC   ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx );

  //===== CBP =====
  Bool    RQencodeCBP_Chroma      ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  Bool    RQencodeCBP_ChromaAC    ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  Bool    RQencodeCBP_8x8         ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx );

  //===== Delta QP and transform size =====
  ErrVal  RQencodeDeltaQp         ( MbDataAccess&   rcMbDataAccess );
  ErrVal  RQencode8x8Flag         ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  ErrVal  RQencodeTermBit         ( UInt            uiBit );

  //===== coefficients =====
  ErrVal  RQencodeNewTCoeff_8x8    ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     B8x8Idx         c8x8Idx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefWritten );
  ErrVal  RQeo8b                   ( Bool&           bEob );
  ErrVal  RQencodeTCoeffRef_8x8    ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     B8x8Idx         c8x8Idx,
                                     UInt            uiScanIndex );
  ErrVal  RQencodeNewTCoeff_Luma   ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     LumaIdx         cIdx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefWritten );
  ErrVal  RQencodeTCoeffRef_Luma   ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     LumaIdx         cIdx,
                                     UInt            uiScanIndex );
  ErrVal  RQencodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     ChromaIdx       cIdx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefWritten );
  ErrVal  RQencodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     ChromaIdx       cIdx,
                                     UInt            uiScanIndex );
  ErrVal  RQencodeCycleSymbol      ( UInt            uiCycle );
  Bool    RQpeekCbp4x4(MbDataAccess& rcMbDataAccess, MbDataAccess&  rcMbDataAccessBase, LumaIdx cIdx);
  ErrVal  RQencodeEobOffsets_Luma  ( UInt* pauiSeq ) { return Err::m_nOK; };
  ErrVal  RQencodeEobOffsets_Chroma( UInt* pauiSeq ) { return Err::m_nOK; };
  ErrVal  RQencodeBestCodeTableMap ( UInt* pauiTable, UInt uiMaxH ) { return Err::m_nOK; };
  ErrVal  RQupdateVlcTable         () { return Err::m_nOK; };
  ErrVal  RQvlcFlush               () { return Err::m_nOK; };

    ErrVal  fieldFlag           ( MbDataAccess& rcMbDataAccess ); //th
  ErrVal  blockModes( MbDataAccess& rcMbDataAccess );
  ErrVal  mbMode( MbDataAccess& rcMbDataAccess/*, Bool bBLQRefFlag*/ );
  ErrVal  resPredFlag( MbDataAccess& rcMbDataAccess );
  ErrVal  resPredFlag_FGS( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff );
	ErrVal  smoothedRefFlag( MbDataAccess& rcMbDataAccess );	// JVT-R091

  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );

  ErrVal  cbp( MbDataAccess& rcMbDataAccess );

  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, ResidualMode eResidualMode );
  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode );

  ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess );
  ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx, ResidualMode eResidualMode );

  ErrVal  deltaQp( MbDataAccess& rcMbDataAccess );
  ErrVal  intraPredModeLuma( MbDataAccess& rcMbDataAccess, LumaIdx cIdx );
  ErrVal  intraPredModeChroma( MbDataAccess& rcMbDataAccess );
  ErrVal  samplesPCM( MbDataAccess& rcMbDataAccess );
  ErrVal  skipFlag( MbDataAccess& rcMbDataAccess, Bool bNotAllowed );
  ErrVal  BLSkipFlag( MbDataAccess& rcMbDataAccess );

  ErrVal  terminatingBit ( UInt uiIsLast );
  UInt getNumberOfWrittenBits();

protected:
  ErrVal xInitContextModels( const SliceHeader& rcSliceHeader );

  ErrVal  xRQencodeNewTCoeffs ( TCoeff*       piCoeff,
                                TCoeff*       piCoeffBase,
                                UInt          uiStop,
                                UInt          uiCtx1,
                                UInt          uiCtx2,
                                const UChar*  pucScan,
                                UInt          uiScanIndex,
                                Bool&         rbLast,
                                UInt&         ruiNumCoefWritten,
                                const int*    paiCtxEobMap = pos2ctx_nomap,
                                const int*    paiCtxSigMap = pos2ctx_nomap,
                                UInt          uiStride = 1 );
  ErrVal  xRQencodeTCoeffsRef ( TCoeff*       piCoeff,
                                TCoeff*       piCoeffBase,
                                const UChar*  pucScan,
                                UInt          uiScanIndex );
  

  ErrVal xWriteMvdComponent( Short sMvdComp, UInt uiAbsSum, UInt uiCtx );
  ErrVal xWriteMvd( MbDataAccess& rcMbDataAccess, Mv cMv, LumaIdx cIdx, ListIdx eLstIdx );

  ErrVal xRefFrame      ( MbDataAccess& rcMbDataAccess, UInt uiRefFrame, ListIdx eLstIdx, ParIdx8x8 eParIdx );
  ErrVal xMotionPredFlag( Bool bFlag,      ListIdx eLstIdx );

  ErrVal xWriteBCbp( MbDataAccess& rcMbDataAccess, UInt uiNumSig, ResidualMode eResidualMode, LumaIdx cIdx );
  ErrVal xWriteBCbp( MbDataAccess& rcMbDataAccess, UInt uiNumSig, ResidualMode eResidualMode, ChromaIdx cIdx );

ErrVal xWriteCoeff( UInt         uiNumSig,
                                 TCoeff*      piCoeff,
                                 ResidualMode eResidualMode,
                                 const UChar* pucScan,
								 Bool            bFieldModel);
  UInt   xGetNumberOfSigCoeff( TCoeff* piCoeff, ResidualMode eResidualMode, const UChar* pucScan );

  ErrVal xWriteBlockMode( UInt uiBlockMode );


protected:
    CabacContextModel2DBuffer m_cFieldFlagCCModel;
    CabacContextModel2DBuffer m_cFldMapCCModel;
    CabacContextModel2DBuffer m_cFldLastCCModel;
  CabacContextModel2DBuffer m_cBLSkipCCModel;

  CabacContextModel2DBuffer m_cBCbpCCModel;
  CabacContextModel2DBuffer m_cMapCCModel;
  CabacContextModel2DBuffer m_cLastCCModel;

  CabacContextModel2DBuffer m_cRefCCModel;
  CabacContextModel2DBuffer m_cSigCCModel;
  
  CabacContextModel2DBuffer m_cOneCCModel;
  CabacContextModel2DBuffer m_cAbsCCModel;
  CabacContextModel2DBuffer m_cChromaPredCCModel;

  CabacContextModel2DBuffer m_cMbTypeCCModel;
  CabacContextModel2DBuffer m_cBlockTypeCCModel;
  CabacContextModel2DBuffer m_cMvdCCModel;
  CabacContextModel2DBuffer m_cRefPicCCModel;
  CabacContextModel2DBuffer m_cBLPredFlagCCModel;
  CabacContextModel2DBuffer m_cResPredFlagCCModel;
  CabacContextModel2DBuffer m_cDeltaQpCCModel;
  CabacContextModel2DBuffer m_cIntraPredCCModel;
  CabacContextModel2DBuffer m_cCbpCCModel;
	CabacContextModel2DBuffer m_cSRFlagCCModel;	// JVT-R091

  CabacContextModel2DBuffer m_cBCbpEnhanceCCModel;
  CabacContextModel2DBuffer m_cCbpEnhanceCCModel;
  CabacContextModel2DBuffer m_cTransSizeCCModel;

  const SliceHeader* m_pcSliceHeader;
  UInt m_uiBitCounter;
  UInt m_uiPosCounter;
  UInt m_uiLastDQpNonZero;
  Bool m_bTraceEnable;
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_CABACWRITER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_)

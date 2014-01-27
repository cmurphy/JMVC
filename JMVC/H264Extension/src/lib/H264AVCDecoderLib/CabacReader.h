#if !defined(AFX_CABACREADER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_)
#define AFX_CABACREADER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MbSymbolReadIf.h"
#include "CabaDecoder.h"
#include "H264AVCCommonLib/CabacContextModel2DBuffer.h"
#include "H264AVCCommonLib/Quantizer.h"
#include "H264AVCCommonLib/ContextTables.h"

H264AVC_NAMESPACE_BEGIN


class CabacReader :
public MbSymbolReadIf
, private CabaDecoder
, public Quantizer

{
protected:
  CabacReader();
  virtual ~CabacReader();

public:
  static ErrVal create        ( CabacReader*& rpcCabacReader );
  ErrVal        destroy       ();

  ErrVal  startSlice          ( const SliceHeader& rcSliceHeader );
  ErrVal  finishSlice         ( ) { return Err::m_nOK; }

  ErrVal  init                ( BitReadBuffer* pcBitReadBuffer );
  ErrVal  uninit              ();

  Bool    RQdecodeBCBP_4x4        ( MbDataAccess&   rcMbDataAccessBase,
                                    LumaIdx         cIdx );
  Bool    RQdecodeBCBP_ChromaDC   ( MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx );
  Bool    RQdecodeBCBP_ChromaAC   ( MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx );
  Bool    RQdecodeCBP_Chroma      ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  Bool    RQdecodeCBP_ChromaAC    ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  Bool    RQdecodeCBP_8x8         ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx );
  ErrVal  RQdecodeDeltaQp         ( MbDataAccess&   rcMbDataAccess );
  ErrVal  RQdecode8x8Flag         ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  ErrVal  RQdecodeTermBit         ( UInt&           ruiBit );



  ErrVal  RQdecodeNewTCoeff_8x8    ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     B8x8Idx         c8x8Idx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefRead );
  ErrVal  RQeo8b                   ( Bool&           bEob );
  ErrVal  RQdecodeTCoeffRef_8x8    ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     B8x8Idx         c8x8Idx,
                                     UInt            uiScanIndex );
  ErrVal  RQdecodeNewTCoeff_Luma   ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     LumaIdx         cIdx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefRead );
  ErrVal  RQdecodeTCoeffRef_Luma   ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     LumaIdx         cIdx,
                                     UInt            uiScanIndex );
  ErrVal  RQdecodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     ChromaIdx       cIdx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefRead );
  ErrVal  RQdecodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     ChromaIdx       cIdx,
                                     UInt            uiScanIndex );
  ErrVal  RQdecodeCycleSymbol      ( UInt&           uiCycle );
  ErrVal  RQdecodeEobOffsets_Luma  () { return Err::m_nOK; };
  ErrVal  RQdecodeEobOffsets_Chroma() { return Err::m_nOK; };
  ErrVal  RQdecodeBestCodeTableMap ( UInt            uiMaxH ) { return Err::m_nOK; };
  ErrVal  RQupdateVlcTable         () { return Err::m_nOK; };
  ErrVal  RQvlcFlush               () { return Err::m_nOK; };
  Bool    RQpeekCbp4x4( MbDataAccess&  rcMbDataAccessBase, LumaIdx cIdx);
  
  Bool    isEndOfSlice        ();
  Bool    isMbSkipped         ( MbDataAccess& rcMbDataAccess );
  Bool    isBLSkipped         ( MbDataAccess& rcMbDataAccess );


  ErrVal  blockModes          ( MbDataAccess& rcMbDataAccess );
  ErrVal  mbMode              ( MbDataAccess& rcMbDataAccess );
  ErrVal  resPredFlag         ( MbDataAccess& rcMbDataAccess );
  ErrVal  resPredFlag_FGS     ( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff );
  ErrVal  smoothedRefFlag     ( MbDataAccess& rcMbDataAccess );	// JVT-R091

  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );

  ErrVal  cbp                 ( MbDataAccess& rcMbDataAccess );
  ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  residualBlock       ( MbDataAccess& rcMbDataAccess, LumaIdx   cIdx, ResidualMode eResidualMode, UInt& ruiMbExtCbp );
  ErrVal  residualBlock       ( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode );
  
  ErrVal  deltaQp             ( MbDataAccess& rcMbDataAccess );
  ErrVal  intraPredModeLuma   ( MbDataAccess& rcMbDataAccess, LumaIdx cIdx );
  ErrVal  intraPredModeChroma ( MbDataAccess& rcMbDataAccess );
  ErrVal  samplesPCM          ( MbDataAccess& rcMbDataAccess );
  ErrVal  fieldFlag           ( MbDataAccess& rcMbDataAccess );

  ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx );
  ErrVal  intraPredModeLuma8x8( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx );
  ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess);

protected:
  ErrVal  xRQdecodeNewTCoeffs ( TCoeff*       piCoeff,
                                TCoeff*       piCoeffBase,
                                UInt          uiStop,
                                UInt          uiCtx1,
                                UInt          uiCtx2,
                                const UChar*  pucScan,
                                UInt          uiScanIndex,
                                Bool&         rbLast,
                                UInt&         ruiNumCoefRead,
                                const int*    paiCtxEobMap = pos2ctx_nomap,
                                const int*    paiCtxSigMap = pos2ctx_nomap,
                                UInt          uiStride = 1 );
  ErrVal  xRQdecodeTCoeffsRef ( TCoeff*       piCoeff,
                                TCoeff*       piCoeffBase,
                                const UChar*  pucScan,
                                UInt          uiScanIndex );

  ErrVal xGetMvd( MbDataAccess& rcMbDataAccess, Mv& rcMv, LumaIdx cIdx, ListIdx eLstIdx );

  ErrVal xInitContextModels( const SliceHeader& rcSliceHeader );
  ErrVal xGetMvdComponent( Short& rsMvdComp, UInt uiAbsSum, UInt uiCtx );
  ErrVal xRefFrame      ( MbDataAccess& rcMbDataAccess, UInt& ruiRefFrame, ListIdx eLstIdx, ParIdx8x8 eParIdx );
  ErrVal xMotionPredFlag( Bool& bFlag,       ListIdx eLstIdx );

  ErrVal xReadBCbp( MbDataAccess& rcMbDataAccess, Bool& rbCoded, ResidualMode eResidualMode, LumaIdx cIdx );
  ErrVal xReadBCbp( MbDataAccess& rcMbDataAccess, Bool& rbCoded, ResidualMode eResidualMode, ChromaIdx cIdx );

  ErrVal xReadCoeff( TCoeff*        piCoeff,
      ResidualMode   eResidualMode,
      const UChar*   pucScan, 
      Bool           bFieldModel);

protected:
    CabacContextModel2DBuffer m_cFieldFlagCCModel;
    CabacContextModel2DBuffer m_cFldMapCCModel;
    CabacContextModel2DBuffer m_cFldLastCCModel;

  CabacContextModel2DBuffer m_cBCbpCCModel;
  CabacContextModel2DBuffer m_cMapCCModel;
  CabacContextModel2DBuffer m_cLastCCModel;
  CabacContextModel2DBuffer m_cRefCCModel;
  CabacContextModel2DBuffer m_cSigCCModel;
  CabacContextModel2DBuffer m_cOneCCModel;
  CabacContextModel2DBuffer m_cAbsCCModel;
  CabacContextModel2DBuffer m_cChromaPredCCModel;
  CabacContextModel2DBuffer m_cBLSkipCCModel;

  CabacContextModel2DBuffer m_cMbTypeCCModel;
  CabacContextModel2DBuffer m_cBlockTypeCCModel;
  CabacContextModel2DBuffer m_cMvdCCModel;
  CabacContextModel2DBuffer m_cRefPicCCModel;
  CabacContextModel2DBuffer m_cBLPredFlagCCModel;
  CabacContextModel2DBuffer m_cResPredFlagCCModel;
  CabacContextModel2DBuffer m_cDeltaQpCCModel;
  CabacContextModel2DBuffer m_cIntraPredCCModel;
  CabacContextModel2DBuffer m_cCbpCCModel;
  CabacContextModel2DBuffer m_cBCbpEnhanceCCModel;
  CabacContextModel2DBuffer m_cCbpEnhanceCCModel;
  CabacContextModel2DBuffer m_cTransSizeCCModel;
	CabacContextModel2DBuffer m_cSRFlagCCModel;	// JVT-R091

  UInt m_uiBitCounter;
  UInt m_uiPosCounter;
  UInt m_uiLastDQpNonZero;
};

H264AVC_NAMESPACE_END


#endif // !defined(AFX_CABACREADER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_)

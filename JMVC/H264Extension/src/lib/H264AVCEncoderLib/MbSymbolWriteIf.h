#if !defined(AFX_MBSYMBOLWRITEIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_)
#define AFX_MBSYMBOLWRITEIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



// h264 namepace begin
H264AVC_NAMESPACE_BEGIN

enum ResidualMode
{
  LUMA_I16_DC  = 0,
  LUMA_I16_AC     ,
  LUMA_SCAN       ,
  CHROMA_DC       ,
  CHROMA_AC
  , LUMA_8X8
};



class MbSymbolWriteIf
{
protected:
  MbSymbolWriteIf() {}
	virtual ~MbSymbolWriteIf() {}

public:
  virtual ErrVal  blockModes          ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  mbMode              ( MbDataAccess& rcMbDataAccess /*, Bool bBLQRefFlag*/ ) = 0;
  virtual ErrVal  resPredFlag         ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  resPredFlag_FGS     ( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff ) = 0;
	virtual ErrVal  smoothedRefFlag     ( MbDataAccess& rcMbDataAccess ) = 0;	// JVT-R091

  virtual ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;
  virtual ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx ) = 0;
  virtual ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx ) = 0;
  virtual ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx ) = 0;

  virtual ErrVal  cbp                 ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;

  virtual ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;

  virtual ErrVal  residualBlock       ( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, ResidualMode eResidualMode ) = 0;
  virtual ErrVal  residualBlock       ( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode ) = 0;

  virtual ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx, ResidualMode eResidualMode ) = 0;
  
  virtual ErrVal  deltaQp             ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  intraPredModeLuma   ( MbDataAccess& rcMbDataAccess, LumaIdx cIdx ) = 0;
  virtual ErrVal  intraPredModeChroma ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  samplesPCM          ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  skipFlag            ( MbDataAccess& rcMbDataAccess, Bool bNotAllowed ) = 0;
  virtual ErrVal  BLSkipFlag          ( MbDataAccess& rcMbDataAccess ) = 0;

  virtual ErrVal  terminatingBit      ( UInt uiIsLast ) = 0;
  virtual UInt    getNumberOfWrittenBits() = 0;
  virtual ErrVal  fieldFlag           ( MbDataAccess& rcMbDataAccess ) = 0;

  virtual ErrVal  startSlice          ( const SliceHeader& rcSliceHeader ) = 0;
  virtual ErrVal  startFragment       () = 0; //JVT-P031
  virtual ErrVal  getLastByte         (UChar &uiLastByte, UInt &uiLastBitPos) = 0; //FIX_FRAG_CAVLC
  virtual ErrVal  setFirstBits(UChar ucByte,UInt uiLastBitPos) = 0; //FIX_FRAG_CAVLC
  virtual ErrVal  finishSlice         ( ) = 0;
  // FGS-related
  virtual Bool    RQencodeCBP_8x8( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase, B8x8Idx c8x8Idx ) = 0;
  virtual Bool    RQencodeBCBP_4x4( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase, LumaIdx cIdx ) = 0;
  virtual Bool    RQencodeCBP_Chroma( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase ) = 0;
  virtual Bool    RQencodeBCBP_ChromaAC( MbDataAccess&  rcMbDataAccess, MbDataAccess&  rcMbDataAccessBase, ChromaIdx cIdx ) = 0;
  virtual Bool    RQencodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccess, MbDataAccess&   rcMbDataAccessBase, ChromaIdx cIdx ) = 0;
  virtual Bool    RQencodeCBP_ChromaAC( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase ) = 0;
  virtual ErrVal  RQencodeDeltaQp( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  RQencode8x8Flag( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase ) = 0;
  virtual ErrVal  RQencodeNewTCoeff_8x8( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      B8x8Idx         c8x8Idx,
                                      UInt            uiScanIndex,
                                      Bool&           rbLast,
                                      UInt&           ruiNumCoefWritten ) = 0;
  virtual ErrVal  RQeo8b( Bool& bEob ) = 0;
  virtual ErrVal RQencodeNewTCoeff_Luma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        LumaIdx         cIdx,
                                        UInt            uiScanIndex,
                                        Bool&           rbLast,
                                        UInt&           ruiNumCoefWritten ) = 0;
  virtual ErrVal RQencodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                          MbDataAccess&   rcMbDataAccessBase,
                                          ResidualMode    eResidualMode,
                                          ChromaIdx       cIdx,
                                          UInt            uiScanIndex,
                                          Bool&           rbLast,
                                          UInt&           ruiNumCoefWritten ) = 0;
  virtual ErrVal RQencodeTCoeffRef_8x8( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      B8x8Idx         c8x8Idx,
                                      UInt            uiScanIndex ) = 0;
  virtual ErrVal RQencodeTCoeffRef_Luma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        LumaIdx         cIdx,
                                        UInt            uiScanIndex ) = 0;
  virtual ErrVal RQencodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex ) = 0;
  virtual ErrVal RQencodeCycleSymbol( UInt uiCycle ) = 0;
  virtual ErrVal RQencodeTermBit ( UInt uiIsLast ) = 0;
  virtual Bool   RQpeekCbp4x4(MbDataAccess& rcMbDataAccess, MbDataAccess&  rcMbDataAccessBase, LumaIdx cIdx) = 0;
  virtual ErrVal RQencodeEobOffsets_Luma   ( UInt* pauiSeq ) = 0;
  virtual ErrVal RQencodeEobOffsets_Chroma ( UInt* pauiSeq ) = 0;
  virtual ErrVal RQencodeBestCodeTableMap ( UInt* pauiTable, UInt uiMaxH ) = 0;
  virtual ErrVal RQupdateVlcTable          () = 0;
  virtual ErrVal RQvlcFlush                () = 0;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBSYMBOLWRITEIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_)

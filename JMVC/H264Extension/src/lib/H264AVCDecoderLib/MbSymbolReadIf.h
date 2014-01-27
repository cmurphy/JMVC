#if !defined(AFX_MBSYMBOLREADIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_)
#define AFX_MBSYMBOLREADIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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


class MbSymbolReadIf
{
protected:
  MbSymbolReadIf() {}
	virtual ~MbSymbolReadIf() {}

public:
  virtual Bool    isMbSkipped ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual Bool    isBLSkipped ( MbDataAccess& rcMbDataAccess ) = 0;

  virtual Bool    isEndOfSlice() = 0;
  virtual ErrVal  blockModes  ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  mbMode      ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  resPredFlag ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  resPredFlag_FGS ( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff ) = 0;
	virtual ErrVal  smoothedRefFlag ( MbDataAccess& rcMbDataAccess ) = 0;	// JVT-R091

  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx ) = 0;

  virtual ErrVal  cbp( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;

  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;

  virtual ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, LumaIdx   cIdx, ResidualMode eResidualMode, UInt& ruiMbExtCbp) = 0;
  virtual ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode ) = 0;

  virtual ErrVal  deltaQp             ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  intraPredModeLuma   ( MbDataAccess& rcMbDataAccess, LumaIdx cIdx ) = 0;
  virtual ErrVal  intraPredModeChroma ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  fieldFlag           ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  samplesPCM          ( MbDataAccess& rcMbDataAccess ) = 0;

  virtual ErrVal  startSlice          ( const SliceHeader& rcSliceHeader ) = 0;
  virtual ErrVal  finishSlice         ( ) = 0;

  virtual ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess) = 0;
  virtual ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx ) = 0;
  virtual ErrVal  intraPredModeLuma8x8( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx ) = 0;
  virtual ErrVal  RQdecodeCycleSymbol ( UInt& uiCycle ) = 0;
  virtual ErrVal  RQdecodeDeltaQp     ( MbDataAccess&   rcMbDataAccess ) = 0;
  virtual ErrVal  RQdecode8x8Flag     ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase ) = 0;
  virtual Bool    RQdecodeBCBP_4x4     ( MbDataAccess&   rcMbDataAccessBase,
                                         LumaIdx         cIdx ) = 0;
  virtual Bool    RQdecodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccessBase,
                                         ChromaIdx       cIdx ) = 0;
  virtual Bool    RQdecodeBCBP_ChromaAC( MbDataAccess&   rcMbDataAccessBase,
                                         ChromaIdx       cIdx ) = 0;
  virtual Bool    RQdecodeCBP_Chroma   ( MbDataAccess&   rcMbDataAccess,
                                         MbDataAccess&   rcMbDataAccessBase ) = 0;
  virtual Bool    RQdecodeCBP_ChromaAC ( MbDataAccess&   rcMbDataAccess,
                                         MbDataAccess&   rcMbDataAccessBase ) = 0;
  virtual Bool    RQdecodeCBP_8x8      ( MbDataAccess&   rcMbDataAccess,
                                         MbDataAccess&   rcMbDataAccessBase,
                                         B8x8Idx         c8x8Idx ) = 0;
  virtual ErrVal  RQdecodeTermBit      ( UInt&           ruiBit ) = 0;
  virtual ErrVal  RQdecodeNewTCoeff_8x8    ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             B8x8Idx         c8x8Idx,
                                             UInt            uiScanIndex,
                                             Bool&           rbLast,
                                             UInt&           ruiNumCoefRead ) = 0;
  virtual ErrVal  RQeo8b( Bool& bEob ) = 0;
  virtual ErrVal  RQdecodeTCoeffRef_8x8    ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             B8x8Idx         c8x8Idx,
                                             UInt            uiScanIndex ) = 0;
  virtual ErrVal  RQdecodeNewTCoeff_Luma   ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             ResidualMode    eResidualMode,
                                             LumaIdx         cIdx,
                                             UInt            uiScanIndex,
                                             Bool&           rbLast,
                                             UInt&           ruiNumCoefRead ) = 0;
  virtual ErrVal  RQdecodeTCoeffRef_Luma   ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             LumaIdx         cIdx,
                                             UInt            uiScanIndex ) = 0;
  virtual ErrVal  RQdecodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             ResidualMode    eResidualMode,
                                             ChromaIdx       cIdx,
                                             UInt            uiScanIndex,
                                             Bool&           rbLast,
                                             UInt&           ruiNumCoefRead ) = 0;
  virtual ErrVal  RQdecodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             ResidualMode    eResidualMode,
                                             ChromaIdx       cIdx,
                                             UInt            uiScanIndex ) = 0;
  virtual ErrVal  RQdecodeEobOffsets_Luma  () = 0;
  virtual ErrVal  RQdecodeEobOffsets_Chroma() = 0;
  virtual ErrVal  RQdecodeBestCodeTableMap ( UInt            uiMaxH ) = 0;
  virtual Bool RQpeekCbp4x4( MbDataAccess& rcMbDataAccessBase, LumaIdx cIdx) = 0;
  virtual ErrVal  RQupdateVlcTable         () = 0;
  virtual ErrVal  RQvlcFlush               () = 0;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBSYMBOLREADIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_)

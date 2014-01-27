#if !defined(AFX_UVLCWRITER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_)
#define AFX_UVLCWRITER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MbSymbolWriteIf.h"
#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"
#include "BitWriteBufferIf.h"



H264AVC_NAMESPACE_BEGIN

#define REFSYM_MB                                     384

class UcSymGrpWriter; 

class UvlcWriter :
public MbSymbolWriteIf
, public HeaderSymbolWriteIf

{
protected:
	UvlcWriter( Bool bTraceEnable = false );
	virtual ~UvlcWriter();
  ErrVal xRQencodeNewTCoeffs( TCoeff*       piCoeff,
                                    TCoeff*       piCoeffBase,
                                    UInt          uiStart,
                                    UInt          uiStop,
                                    UInt          uiStride,
                                    const UChar*  pucScan,
                                    UInt          uiScanIndex,
                                    UInt*         pauiEobShift,
                                    Bool&         rbLast,
                                    UInt&         ruiNumCoefWritten );
  ErrVal xRQencodeTCoeffsRef( TCoeff*       piCoeff,
                                  TCoeff*       piCoeffBase,
                                  const UChar*  pucScan,
                                  UInt          uiScanIndex );

  ErrVal xRQencodeSigMagGreater1( TCoeff* piCoeff, TCoeff* piCoeffBase, UInt uiBaseCode, UInt uiStart, UInt uiStop, UInt uiVlcTable, const UChar*  pucScan, UInt& ruiNumMagG1, UInt uiStride = 1 );
  ErrVal xWriteSigRunCode ( UInt uiSymbol, UInt uiTableIdx );
  ErrVal xWriteUnaryCode (UInt uiSymbol );
  ErrVal xWriteCodeCB1 (UInt uiSymbol );
  ErrVal xWriteCodeCB2 (UInt uiSymbol );
  ErrVal xWriteGolomb(UInt uiSymbol, UInt uiK);
  ErrVal xEncodeMonSeq ( UInt* auiSeq, UInt uiStartVal, UInt uiLen );
  ErrVal xRQencodeEobOffsets ( UInt* auiSeq, UInt uiLen );
  UInt m_auiShiftLuma[16];
  UInt m_auiShiftChroma[16];
  UInt m_auiBestCodeTabMap[16];
  UInt m_uiCbpStats[3][2];
  UcSymGrpWriter* m_pSymGrp; 
  UInt m_uiCbpStat4x4[2];

public:
  static ErrVal create( UvlcWriter*& rpcUvlcWriter, Bool bTraceEnable = true );
  ErrVal destroy();

  ErrVal init(  BitWriteBufferIf* pcBitWriteBufferIf );
  ErrVal uninit();

  ErrVal  startSlice( const SliceHeader& rcSliceHeader );
  ErrVal  startFragment(); //JVT-P031
  ErrVal  getLastByte(UChar &uiLastByte, UInt &uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  setFirstBits(UChar ucByte,UInt uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  finishSlice();

  ErrVal closeSlice();

  ErrVal  blockModes( MbDataAccess& rcMbDataAccess );
  ErrVal  mbMode( MbDataAccess& rcMbDataAccess/*, Bool bBLQRefFlag*/ );
  ErrVal  resPredFlag( MbDataAccess& rcMbDataAccess );
  ErrVal  resPredFlag_FGS( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff );
  ErrVal  fieldFlag           ( MbDataAccess& rcMbDataAccess ); //th
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

  ErrVal  terminatingBit ( UInt uiIsLast ) { return Err::m_nOK;}

  ErrVal writeUvlc( UInt uiCode, const Char* pcTraceString );
  ErrVal writeSvlc( Int iCode, const Char* pcTraceString );
  ErrVal writeCode( UInt uiCode, UInt uiLength, const Char* pcTraceString );
  ErrVal writeSCode( Int iCode, UInt uiLength, const Char* pcTraceString );
  ErrVal writeFlag( Bool bFlag, const Char* pcTraceString );

  UInt getNumberOfWrittenBits();

  UInt xGetCoeffCost() { return m_uiCoeffCost; }
  Void xSetCoeffCost(UInt uiCost) { m_uiCoeffCost = uiCost; }

  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes );
  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes );

  ErrVal xWriteMvd( Mv cMv );

  ErrVal xWriteTrailingOnes4( UInt uiCoeffCount, UInt uiTrailingOnes );
  ErrVal xWriteTrailingOnes16( UInt uiLastCoeffCount, UInt uiCoeffCount, UInt uiTrailingOnes );
  ErrVal xWriteTotalRun4( UInt uiVlcPos, UInt uiTotalRun );
  ErrVal xWriteTotalRun16( UInt uiVlcPos, UInt uiTotalRun );
  ErrVal xWriteLevelVLC0( Int iLevel );
  ErrVal xWriteLevelVLCN( Int iLevel, UInt uiVlcLength );
  ErrVal xWriteRun( UInt uiVlcPos, UInt uiRun );
  ErrVal xWriteRunLevel( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt uiTotalRun );
  ErrVal xWriteRefFrame( Bool bWriteBit, UInt uiRefFrame );
  ErrVal xWriteMotionPredFlag( Bool bFlag );

  ErrVal xWriteUvlcCode( UInt uiVal);
  ErrVal xWriteSvlcCode( Int iVal);

  ErrVal xRQprescanTCoeffsRef( TCoeff*       piCoeff,
                               TCoeff*       piCoeffBase,
                               const UChar*  pucScan,
                               UInt          uiScanIndex );

  UInt xConvertToUInt( Int iValue )  {  return ( iValue <= 0) ? -iValue<<1 : (iValue<<1)-1; }

  Bool    RQencodeCBP_8x8( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase, B8x8Idx c8x8Idx );
  Bool    RQencodeBCBP_4x4( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase, LumaIdx cIdx );
  Bool    RQencodeCBP_Chroma( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase );
  Bool    RQencodeBCBP_ChromaAC( MbDataAccess&  rcMbDataAccess, MbDataAccess&  rcMbDataAccessBase, ChromaIdx cIdx );
  Bool    RQencodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccess, MbDataAccess&   rcMbDataAccessBase, ChromaIdx cIdx );
  Bool    RQencodeCBP_ChromaAC( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase );
  ErrVal  RQencodeDeltaQp( MbDataAccess& rcMbDataAccess );
  ErrVal  RQencode8x8Flag( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase );
  ErrVal  RQencodeNewTCoeff_8x8( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      B8x8Idx         c8x8Idx,
                                      UInt            uiScanIndex,
                                      Bool&           rbLast,
                                      UInt&           ruiNumCoefWritten );
  ErrVal  RQeo8b                    ( Bool&           bEob );
  ErrVal RQencodeNewTCoeff_Luma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        LumaIdx         cIdx,
                                        UInt            uiScanIndex,
                                        Bool&           rbLast,
                                        UInt&           ruiNumCoefWritten );
  ErrVal RQencodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                          MbDataAccess&   rcMbDataAccessBase,
                                          ResidualMode    eResidualMode,
                                          ChromaIdx       cIdx,
                                          UInt            uiScanIndex,
                                          Bool&           rbLast,
                                          UInt&           ruiNumCoefWritten );
  ErrVal RQencodeTCoeffRef_8x8( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      B8x8Idx         c8x8Idx,
                                      UInt            uiScanIndex );
  ErrVal RQencodeTCoeffRef_Luma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        LumaIdx         cIdx,
                                        UInt            uiScanIndex );
  ErrVal RQencodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex );
  ErrVal RQencodeCycleSymbol( UInt uiCycle );
  ErrVal RQencodeTermBit ( UInt uiIsLast ) { return Err::m_nOK;}
  Bool   RQpeekCbp4x4(MbDataAccess& rcMbDataAccess, MbDataAccess&  rcMbDataAccessBase, LumaIdx cIdx);
  ErrVal RQencodeEobOffsets_Luma ( UInt* auiSeq );
  ErrVal RQencodeEobOffsets_Chroma( UInt* auiSeq );
  ErrVal RQencodeBestCodeTableMap( UInt* auiTable, UInt uiMaxH );
  ErrVal RQupdateVlcTable         ();
  ErrVal RQvlcFlush               ();
  static UInt   peekGolomb(UInt uiSymbol, UInt uiK);
  ErrVal RQcountFragmentedSymbols();
  ErrVal resetFragmentedSymbols() {m_uiFragmentedSymbols = 0; return Err::m_nOK;}
private:
  __inline ErrVal xWriteCode( UInt uiCode, UInt uiLength );
  __inline ErrVal xWriteFlag( UInt uiCode );

protected:
  BitWriteBufferIf* m_pcBitWriteBufferIf;
  UInt m_uiBitCounter;
  UInt m_uiPosCounter;

  UInt m_uiCoeffCost;
  Bool m_bTraceEnable;

  Bool m_bRunLengthCoding;
  UInt m_uiRun;

  UChar m_auiPrescannedSymbols[REFSYM_MB];
  UInt  m_uiRefSymbols;
  UInt  m_uiCodedSymbols;
  UInt  m_uiFragmentedSymbols;
};

class UcSymGrpWriter
{
public:
  UcSymGrpWriter( UvlcWriter* pParent );
  ErrVal Init();
  ErrVal Flush();
  ErrVal Write( UChar ucBit );
  Bool   UpdateVlc();
  UInt   getTable()                   { return m_uiTable;       }
  Void   setCodedFlag(UInt uiFlag)    { m_uiCodedFlag = uiFlag; }
  Void   incrementCounter(UInt uiSym) { m_auiSymCount[uiSym]++; }

protected:
  UvlcWriter* m_pParent;
  UInt m_auiSymCount[3];
  UInt m_uiTable;
  UInt m_uiCodedFlag;
  UInt m_uiCode;
  UInt m_uiLen;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_UVLCWRITER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_)

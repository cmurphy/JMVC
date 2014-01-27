#ifndef __ASSEMBLER_H_D65BE9B4_A8DA_11D3_AFE7_005004464B77
#define __ASSEMBLER_H_D65BE9B4_A8DA_11D3_AFE7_005004464B77

#include "H264AVCCommonLib/Sei.h"

#include "ReadBitstreamFile.h"
#include "WriteBitstreamToFile.h"
#include "AssemblerParameter.h"



enum NalUnitType
{
  NAL_UNIT_EXTERNAL                 = 0,
  NAL_UNIT_CODED_SLICE              = 1,
  NAL_UNIT_CODED_SLICE_DATAPART_A   = 2,
  NAL_UNIT_CODED_SLICE_DATAPART_B   = 3,
  NAL_UNIT_CODED_SLICE_DATAPART_C   = 4,
  NAL_UNIT_CODED_SLICE_IDR          = 5,
  NAL_UNIT_SEI                      = 6,
  NAL_UNIT_SPS                      = 7,
  NAL_UNIT_PPS                      = 8,
  NAL_UNIT_ACCESS_UNIT_DELIMITER    = 9,
  NAL_UNIT_END_OF_SEQUENCE          = 10,
  NAL_UNIT_END_OF_STREAM            = 11,
  NAL_UNIT_FILLER_DATA              = 12,
  NAL_UNIT_SUBSET_SPS               = 15,

  NAL_UNIT_CODED_SLICE_PREFIX       = 14, //
  NAL_UNIT_CODED_SLICE_SCALABLE     = 20,
  NAL_UNIT_CODED_SLICE_IDR_SCALABLE = 21
};



class Assembler  
{
protected:
	Assembler();
 ~Assembler();

public:
  static ErrVal create              ( Assembler*&         rpcExtractor );
  ErrVal        init                ( AssemblerParameter* pcAssemblerParameter ) ;
  ErrVal        go                  () ;
  ErrVal        destroy             () ;


protected:
  ErrVal        xAnalyse            () ;
  
  ErrVal        xWriteViewScalSEIToBuffer( h264::SEI::ViewScalabilityInfoSei* pcScalableSei, BinData* pcBinData ); //SEI LSJ
protected:
  ReadBitstreamIf**             m_ppcReadBitstream;
  WriteBitstreamIf*             m_pcWriteBitstream;
  AssemblerParameter*           m_pcAssemblerParameter;

//SEI LSJ{
  h264::H264AVCPacketAnalyzer*  m_pcH264AVCPacketAnalyzer;
  h264::SEI::ViewScalabilityInfoSei**  pcTmpViewScalInfoSei;
//SEI LSJ}

  UChar                         m_aucStartCodeBuffer[5];
  BinData                       m_cBinDataStartCode;

  FILE*                         m_pcTraceFile;
  FILE*                         m_pcAssemblerTraceFile;
  
  
  UInt                          m_uiNumViews;
  UInt                          m_uiTempViewDecOrder;
  Bool                          m_bSuffix; 
//  UInt							m_uiSuffixUnitEnable;


};

#endif //__ASSEMBLER_H_D65BE9B4_A8DA_11D3_AFE7_005004464B77


#ifndef __EXTRACTOR_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
#define __EXTRACTOR_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79


#define MAX_PACKET_SIZE 1000000
// JVT-S080 LMI
// If defined to 1, the initial scalability_info SEI should be updated after an extraction;
// Otherwise, it's unchanged and followed by a layers_not_present scalable SEI. 
#define UPDATE_SCALABLE_SEI 1
#include "H264AVCCommonLib/Sei.h"

#include "ReadBitstreamFile.h"
#include "WriteBitstreamToFile.h"
#include "ExtractorParameter.h"

#define MAX_ROIS			5


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

  NAL_UNIT_CODED_SLICE_SCALABLE     = 20,
  NAL_UNIT_CODED_SLICE_IDR_SCALABLE = 21
};


class Extractor  
{
protected:
	Extractor();
	virtual ~Extractor();

public:
  static ErrVal create              ( Extractor*&         rpcExtractor );
  ErrVal        init                ( ExtractorParameter* pcExtractorParameter );
  ErrVal        go                  ();
  ErrVal        destroy             ();


  ErrVal	    xWriteViewScalSEIToBuffer(h264::SEI::ViewScalabilityInfoSei *pcViewScalSei, BinData *pcBinData);

protected:
  ErrVal		xExtractOperationPoints ();
  ErrVal		xDisplayOperationPoints ();


  // ROI ICU/ETRI
  ErrVal		xChangeViewScalSEIMessage( BinData *pcBinData, h264::SEI::SEIMessage* pcSEIMessage, UInt uiOpId, UInt*& uiViewId, UInt* uiNewNumViews );


protected:
  ReadBitstreamIf*              m_pcReadBitstream;
  WriteBitstreamIf*             m_pcWriteBitstream;
  ExtractorParameter*           m_pcExtractorParameter;
  h264::H264AVCPacketAnalyzer*  m_pcH264AVCPacketAnalyzer;

  UChar                         m_aucStartCodeBuffer[5];
  BinData                       m_cBinDataStartCode;

};
class ExtractStop{};
#endif //__EXTRACTOR_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79


#if !defined(AFX_CREATERH264AVCDECODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_)
#define AFX_CREATERH264AVCDECODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/FrameMng.h" // hwsun, fix meomory for field coding

#include "H264AVCCommonLib/Sei.h"
#include "../../test/H264AVCDecoderLibTest/DecoderParameter.h"
#define MAX_ROI_NUM	5

class H264AVCDecoder;
class ControlMngH264AVCDecoder;
class SliceReader;
class SliceDecoder;
class UvlcReader;
class MbParser;
class MbDecoder;
class NalUnitParser;
class BitReadBuffer;
class CabacReader;
class CabaDecoder;

class MbData;
class Frame;
class FrameMng;
class LoopFilter;
class Transform;
class IntraPrediction;
class MotionCompensation;
class YuvBufferCtrl;
class QuarterPelFilter;
class SampleWeighting;
class ParameterSetMng;
class PocCalculator;

class ReconstructionBypass;


H264AVC_NAMESPACE_BEGIN

class H264AVCDECODERLIB_API CreaterH264AVCDecoder
{
protected:
	CreaterH264AVCDecoder();
	virtual ~CreaterH264AVCDecoder();

public:
  static ErrVal create( CreaterH264AVCDecoder*& rpcCreaterH264AVCDecoder );

  H264AVCDecoder *getH264AVCDecoder() const { return m_pcH264AVCDecoder; } //JVT-S036 lsj
  ErrVal destroy    ();
  ErrVal init       (Bool bOpenTrace, DecoderParameter* Dec_Param); 


  NalUnitParser* getNalUnitParser() const {return m_pcNalUnitParser;}

  //JVT-V054
  UInt* getViewCodingOrder();
  UInt* getViewCodingOrder_SubStream();

  Int getMaxEtrDPB() { return m_pcFrameMng->getMaxEntriesinDPB(); } // hwsun, fix meomory for field coding

    void addViewCodingOrder();

  ErrVal uninit     ( Bool bCloseTrace ); 
  ErrVal process    ( PicBuffer*        pcPicBuffer,
                      PicBufferList&    rcPicBufferOutputList,
                      PicBufferList&    rcPicBufferUnusedList,
                      PicBufferList&    rcPicBufferReleaseList );
  ErrVal initPacket ( BinDataAccessor*  pcBinDataAccessor,
	                    UInt&             ruiNalUnitType,
	                    UInt&             uiMbX,
	                    UInt&             uiMbY,
	                    UInt&             uiSize
						//,UInt&				uiNonRequiredPic //NonRequired JVT-Q066
                        //JVT-P031
	                    ,Bool              bPreParseHeader //FRAG_FIX
	                    , Bool			bConcatenated //FRAG_FIX_3
                      ,Bool&             rbStartDecoding,
                      UInt&             ruiStartPos,
                      UInt&             ruiEndPos,
                      Bool&              bFragmented,
                      Bool&              bDiscardable,
                      //~JVT-P031
					  UInt NumOfViewInTheStream,
                      Bool& bSkip
                      ); 

//JVT-S036  start
  ErrVal initPacketSuffix( BinDataAccessor*  pcBinDataAccessor,
								                  UInt&             ruiNalUnitType,
								         		  Bool             bPreParseHeader, 
								                  Bool			bConcatenated, 
												  Bool&			 rbStarDecoding
												  ,CreaterH264AVCDecoder*   pcH264AVCDecoder
												  ,Bool&		SuffixEnable
						 ); 
//JVT-S036  end

  //JVT-P031
  ErrVal initPacket ( BinDataAccessor*  pcBinDataAccessor);
  Void decreaseNumOfNALInAU();
  Void    setDependencyInitialized(Bool b);
  UInt    getNumOfNALInAU();
  Void    initNumberOfFragment();
  //~JVT-P031
  Void    setFGSRefInAU(Bool &b); //JVT-T054
  ErrVal  checkSliceLayerDependency ( BinDataAccessor*  pcBinDataAccessor,
                                      Bool&             bFinishChecking );

  UInt isNonRequiredPic();	//NonRequired JVT-Q066	
  Bool isRedundantPic();  // JVT-Q054 Red. Picture
  ErrVal  checkRedundantPic();  // JVT-Q054 Red. Picture

//	TMM_EC {{
  ErrVal  checkSliceGap ( BinDataAccessor*  pcBinDataAccessor,
                          MyList<BinData*>&	cVirtualSliceList );
	ErrVal	setec( UInt uiErrorConceal);
//  TMM_EC }}

	Void setAVCFlag ( Bool aFlag ) { UnitAVCFlag = aFlag; } //JVT-S036 

  Void	  RoiDecodeInit();

  Void    setCrop(UInt *uiCrop);

protected:
  ErrVal xCreateDecoder();

protected:
  H264AVCDecoder*         m_pcH264AVCDecoder;
//  RQFGSDecoder*           m_pcRQFGSDecoder; ying
//  DecodedPicBuffer*       m_apcDecodedPicBuffer     [MAX_LAYERS];
  //MCTFDecoder*            m_apcMCTFDecoder          [MAX_LAYERS];
  FrameMng*               m_pcFrameMng;
  ParameterSetMng*        m_pcParameterSetMng;
  PocCalculator*          m_apcPocCalculator        [MAX_LAYERS];
  SliceReader*            m_pcSliceReader;
  NalUnitParser*          m_pcNalUnitParser;
  SliceDecoder*           m_pcSliceDecoder;
  ControlMngH264AVCDecoder*  m_pcControlMng;
  BitReadBuffer*          m_pcBitReadBuffer;
  UvlcReader*             m_pcUvlcReader;
  MbParser*               m_pcMbParser;
  LoopFilter*             m_pcLoopFilter;
  MbDecoder*              m_pcMbDecoder;
  Transform*              m_pcTransform;
  IntraPrediction*        m_pcIntraPrediction;
  MotionCompensation*     m_pcMotionCompensation;
  YuvBufferCtrl*          m_apcYuvFullPelBufferCtrl [MAX_LAYERS];
  QuarterPelFilter*       m_pcQuarterPelFilter;
  CabacReader*            m_pcCabacReader;
  SampleWeighting*        m_pcSampleWeighting;
  ReconstructionBypass*   m_pcReconstructionBypass;

  Bool					  UnitAVCFlag;    //JVT-S036 
};







struct PacketDescription
{
  Bool  ParameterSet;
  Bool  Scalable;
  UInt  Layer;
//SEI {
  UInt  ViewId;
  Bool  bSvcMvcFlag;
  Bool  bAnchorPicFlag;
//SEI }
  UInt  Level;
  UInt  FGSLayer;
  Bool  ApplyToNext;
  UInt  NalUnitType; 
  UInt  SPSid;
  UInt  PPSid;
  UInt  SPSidRefByPPS[256];
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  UInt auiDeltaBytesRateOfLevelQL[MAX_NUM_RD_LEVELS];
  UInt auiQualityLevelQL[MAX_NUM_RD_LEVELS];
  UInt uiNumLevelsQL;
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
  UInt uiPId;
  Bool bDiscardable;//JVT-P031
  Bool bFragmentedFlag;//JVT-P031
  UInt NalRefIdc;

  //-- 2006.0604
  UInt uiFirstMb;
  Bool  bEnableQLTruncation; //JVT-T054
};



class H264AVCDECODERLIB_API H264AVCPacketAnalyzer
{
protected:
	H264AVCPacketAnalyzer();
	virtual ~H264AVCPacketAnalyzer();

public:
  static ErrVal create  ( H264AVCPacketAnalyzer*&  rpcH264AVCPacketAnalyzer );
  ErrVal        destroy ();
  ErrVal        init    ();
  ErrVal        uninit  ();
  ErrVal        process ( BinData*              pcBinData,
                          PacketDescription&    rcPacketDescription,
                          SEI::SEIMessage*&     pcScalableSEIMessage );
  ErrVal	      processSEIAndMVC ( BinData*				pcBinData,	SEI::ViewScalabilityInfoSei*&		pcSEIMessage );//SEI  //fix Nov. 30
  ErrVal        isMVCProfile( BinData*				pcBinData, Bool & b);// fix Nov. 30 
  SEI::NonRequiredSei*	getNonRequiredSEI()	{return m_pcNonRequiredSEI;}
  UInt					getNonRequiredSeiFlag() { return m_uiNonRequiredSeiFlag;}

  int m_uiNum_layers; //
  int m_ID_ROI[MAX_SCALABLE_LAYERS];
  int m_ID_Dependency[MAX_SCALABLE_LAYERS];
  int m_silceIDOfSubPicLayer[MAX_SCALABLE_LAYERS];
  int m_layer_id;//


  // ROI ICU/ETRI DS
  UInt Num_Related_ROI[MAX_NUM_LAYER]; 
  UInt m_uiNumSliceGroupsMinus1;
  UInt addrFirstMB;
  UInt uiaAddrFirstMBofROIs[256][MAX_ROI_NUM];  
  UInt uiaAddrLastMBofROIs[256][MAX_ROI_NUM];  


protected:
  ErrVal        xCreate ();

protected:
  BitReadBuffer*    m_pcBitReadBuffer;
  UvlcReader*       m_pcUvlcReader;
  NalUnitParser*    m_pcNalUnitParser;
  UInt              m_auiDecompositionStages[MAX_LAYERS];
  UInt              m_uiStdAVCOffset;

/*  UInt              m_uiTemporalLevelList[1 << PRI_ID_BITS];
  UInt              m_uiDependencyIdList [1 << PRI_ID_BITS];
  UInt              m_uiQualityLevelList [1 << PRI_ID_BITS];
JVT-S036  */
  SEI::NonRequiredSei*  m_pcNonRequiredSEI;
  UInt					m_uiNonRequiredSeiFlag;
  UInt					m_uiPrevPicLayer;
  UInt					m_uiCurrPicLayer;

  Bool					m_bAVCCompatible;//BUG FIX Kai Zhang
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_CREATERH264AVCDECODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_)

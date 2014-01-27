#if !defined  _PIC_ENCODER_INCLUDED_
#define       _PIC_ENCODER_INCLUDED_

#include "InputPicBuffer.h"
#include "SequenceStructure.h"
#include "RecPicBuffer.h"


#include "Multiview.h"


H264AVC_NAMESPACE_BEGIN

typedef MyList<UInt>        UIntList; // 

class CodingParameter;
class ParameterSetMng;
class SliceEncoder;
class LoopFilter;
class PocCalculator;
class NalUnitEncoder;
class YuvBufferCtrl;
class QuarterPelFilter;
class MotionEstimation;
class ControlMngIf;


class PicEncoder
{
public:
  PicEncoder                                    ();
  virtual ~PicEncoder                           ();

  static ErrVal   create                        ( PicEncoder*&                rpcPicEncoder );
  ErrVal          destroy                       ();

  ErrVal          init                          ( CodingParameter*            pcCodingParameter,
                                                  ControlMngIf*               pcControlMng,
                                                  SliceEncoder*               pcSliceEncoder,
                                                  LoopFilter*                 pcLoopFilter,
                                                  PocCalculator*              pcPocCalculator,
                                                  NalUnitEncoder*             pcNalUnitEncoder,
                                                  YuvBufferCtrl*              pcYuvBufferCtrlFullPel,
                                                  YuvBufferCtrl*              pcYuvBufferCtrlHalfPel,
                                                  QuarterPelFilter*           pcQuarterPelFilter,
                                                  MotionEstimation*           pcMotionEstimation );
  ErrVal          uninit                        ();
  
  ErrVal          writeAndInitParameterSets     ( ExtBinDataAccessor*         pcExtBinDataAccessor,
                                                  Bool&                       rbMoreSets,
												   UInt Num_Of_Views_Minus_1);
  ErrVal          process                       ( PicBuffer*                  pcInputPicBuffer,
                                                  PicBufferList&              rcOutputList,
                                                  PicBufferList&              rcUnusedList,
                                                  ExtBinDataAccessorList&     rcExtBinDataAccessorList );
  ErrVal          finish                        ( PicBufferList&              rcOutputList,
                                                  PicBufferList&              rcUnusedList );


  
  
  Bool		getSvcMvcFlag ()  const { return m_SvcMvcFlag;    } // u(1)
  UInt 		getAVCFlag()      const { return m_bAVCFlag;      }
  UInt		getViewId()       const { return m_CurrentViewId; } // u(10) 
  
  bool TimeForVFrameP (const int currentFrameNum) const;
  ErrVal		xWritePrefixUnit    ( ExtBinDataAccessorList& rcExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit );//JVT-W035

  //SEI LSJ{
  Double* dMVCGetFramerate	()			 { return m_dMVCFinalFramerate; }
  Double* dMVCGetBitrate	()			 { return m_dMVCFinalBitrate; }
  Double* dMVCGetSeqBits	()			 { return m_adMVCSeqBits; }
  Double  dGetMaxBitrate    ()			 { return m_adMaxBitRate; }
  ErrVal  xModifyMaxBitrate ( UInt uiBits );
//SEI LSJ}
//JVT-W080
	Void   setPdsEnable              ( UInt   uiValue ) { m_uiPdsEnable                = uiValue; }
	Void   setPdsInitialDelayMinus2L0( UInt** uiValue ) { m_ppuiPdsInitialDelayMinus2L0= uiValue; }
	Void   setPdsInitialDelayMinus2L1( UInt** uiValue ) { m_ppuiPdsInitialDelayMinus2L1= uiValue; }
	Void   setPdsBlockSize           ( UInt   uiValue ) { m_uiPdsBlockSize             = uiValue; }
	UInt   getPdsEnable              ()         const { return m_uiPdsEnable;                  }
	UInt** getPdsInitialDelayMinus2L0()         const { return m_ppuiPdsInitialDelayMinus2L0;  }
	UInt** getPdsInitialDelayMinus2L1()         const { return m_ppuiPdsInitialDelayMinus2L1;  }
	UInt   getPdsBlockSize           ()         const { return m_uiPdsBlockSize;               }
//~JVT-W080
	Bool		derivation_Inter_View_Flag (UInt View_id, SliceHeader& rcSliceHeader);// JVT-W056 samsung
	Void		setpicEncoder						(PicEncoder *picencoder) { m_picEncoder = picencoder;}
	PicEncoder* getpicEncoder				() {return m_picEncoder;}

private:
  //===== initializations =====
  ErrVal          xInitSPS                      ( Bool bAVCSPS );
  ErrVal          xInitPPS                      ( Bool bAVCSPS );
  ErrVal          xInitParameterSets            ();
//  {{
  //===== RPLR and MMCO commands =====
  ErrVal        xGetFrameNumList    ( FrameSpec&  rcFrameSpec,  UIntList& rcFrameNumList, ListIdx eLstIdx, UInt uiCurrBasePos );
  ErrVal        xSetRplrAndMmco( FrameSpec& rcFrameSpec );
  ErrVal        xSetRplr            ( RplrBuffer& rcRplrBuffer, UIntList cFrameNumList, UInt uiCurrFrameNr );
  ErrVal  xInitReordering               ( UInt          uiFrameIdInGOP);

  ErrVal  xGetListSizes                 ( UInt                        uiTemporalLevel,
                                          UInt                        uiFrameIdInGOP,
                                          UInt                        auiPredListSize[2]);
  ErrVal  xGetListSizesSpecial          ( UInt                        uiTemporalLevel,
                                          UInt                        uiFrameIdInGOP,
                                          UInt                        auiPredListSize[2]);

  ErrVal  xInitReorderingInterView  ( SliceHeader*&               rpcSliceHeader ); // JVT-V043  related

  ErrVal  xInitSliceHeader          ( SliceHeader*&               rpcSliceHeader,
                                      FrameSpec&                  rcFrameSpec,
                                      Double&                     dLambda, 
                                      Bool                        fakeHeader=false,
									  PicType                     ePicType=FRAME);

  UIntList                      m_cLPFrameNumList;                    
//  }}
  ErrVal          xInitPredWeights              ( SliceHeader&                rcSliceHeader );

  //===== create and delete memory =====
  ErrVal          xCreateData                   ();
  ErrVal          xDeleteData                   ();

  //===== packet management =====
  ErrVal          xInitExtBinDataAccessor       ( ExtBinDataAccessor&         rcExtBinDataAccessor );
  ErrVal          xAppendNewExtBinDataAccessor  ( ExtBinDataAccessorList&     rcExtBinDataAccessorList,
                                                  ExtBinDataAccessor*         pcExtBinDataAccessor );

  //===== encoding =====
  ErrVal          xStartPicture                 ( RecPicBufUnit&              rcRecPicBufUnit,
                                                  SliceHeader&                rcSliceHeader,
                                                  RefFrameList&               rcList0,
                                                  RefFrameList&               rcList1 );
  ErrVal          xEncodePicture                ( ExtBinDataAccessorList&     rcExtBinDataAccessorList,
                                                  RecPicBufUnit&              rcRecPicBufUnit,
                                                  SliceHeader&                rcSliceHeader,
                                                  Double                      dLambda,
                                                  UInt&                       ruiBits);
  ErrVal          xFinishPicture                ( RecPicBufUnit&              rcRecPicBufUnit,
                                                  SliceHeader&                rcSliceHeader,
                                                  RefFrameList&               rcList0,
                                                  RefFrameList&               rcList1,
                                                  UInt                        uiBits );
  ErrVal          xGetPSNR                      ( RecPicBufUnit&              rcRecPicBufUnit,
                                                  Double*                     adPSNR );

ErrVal
xFieldList(    SliceHeader&   rcSliceHeader,
                           RefFrameList&  rcList,
                           RefFrameList&  rcListTemp );//lufeng

// ying GOP stucture support, simplify the configuration files {{
  ErrVal          xInitFrameSpec               ();
  ErrVal          xGetNextFrameSpec            ();
  ErrVal          xUpdateFrameSepNextGOP       ();
  ErrVal          xUpdateFrameSepNextGOPFinish ();
  ErrVal          xInitFrameSpecSpecial        ();
  ErrVal          xInitFrameSpecHierarchical   ();
  ErrVal          xGetNextFrameSpecSpecial     ();
//  }}

ErrVal xSetRefPictures(SliceHeader&                rcSliceHeader,
                       RefFrameList&               rcList0,
                       RefFrameList&               rcList1);

private:
  Bool                        m_bInit;
  Bool                        m_bInitParameterSets;

  //===== members =====
  BinData                     m_cBinData;
  ExtBinDataAccessor          m_cExtBinDataAccessor;
  FrameSpec                   m_cFrameSpecification;
//  {{
  FrameSpec                   m_acFrameSpecification[34];
  UInt                        m_uiAnchorFrameNumber ;
  UInt                        m_uiGOPSize;
  UInt                        m_uiTotalFrames;
  UInt                        m_uiMaxTL;
  UInt                        m_uiFrameDelay;
  UInt                        m_uiMaxNumRefFrames;
  UInt                        m_uiMaxFrameNum;
  UInt                        m_uiAnchorNumFwdViewRef;
  UInt                        m_uiAnchorNumBwdViewRef;
  UInt                        m_uiNonAncNumFwdViewRef;
  UInt                        m_uiNonAncNumBwdViewRef;

  Bool                        m_bInterPridPicsFirst; //JVT-V043 enc

  UInt                        m_uiMaxFrameNumList0;
  UInt                        m_uiMaxFrameNumList1;
  UInt                        m_uiProcessingPocInGOP;
  Bool                        m_bSpecialGOP;
  UInt                        m_uiGOPSizeReal;
  UInt                        m_uiMinTempLevelLastGOP;
  SpsMvcExtension            *m_pcSPSMVCExt;
//  }}
  SequenceStructure*          m_pcSequenceStructure;
  InputPicBuffer*             m_pcInputPicBuffer;
  SequenceParameterSet*       m_pcSPS;
  SequenceParameterSet*       m_pcSPSBase; //
  PictureParameterSet*        m_pcPPSBase;
  PictureParameterSet*        m_pcPPS;
  
  RecPicBuffer*               m_pcRecPicBuffer;//buffer for recpics

  //===== references =====
  CodingParameter*            m_pcCodingParameter;
  ControlMngIf*               m_pcControlMng;
  SliceEncoder*               m_pcSliceEncoder;
  LoopFilter*                 m_pcLoopFilter;
  PocCalculator*              m_pcPocCalculator;
  NalUnitEncoder*             m_pcNalUnitEncoder;
  YuvBufferCtrl*              m_pcYuvBufferCtrlFullPel;
  YuvBufferCtrl*              m_pcYuvBufferCtrlHalfPel;
  QuarterPelFilter*           m_pcQuarterPelFilter;
  MotionEstimation*           m_pcMotionEstimation;
	PicEncoder*									m_picEncoder; //JVT-W056

  //===== fixed coding parameters =====
  UInt                        m_uiFrameWidthInMb;
  UInt                        m_uiFrameHeightInMb;
  UInt                        m_uiMbNumber;

  //===== variable parameters =====
  UInt                        m_uiFrameNum;
  UInt                        m_uiIdrPicId;
  UInt                        m_uiWrittenBytes;
  UInt                        m_uiCodedFrames;
  Double                      m_dSumYPSNR;
  Double                      m_dSumUPSNR;
  Double                      m_dSumVPSNR;
//JVT-W080
	UInt                        m_uiPdsEnable;
	UInt                        m_uiPdsBlockSize;
	UInt**                      m_ppuiPdsInitialDelayMinus2L0;
	UInt**                      m_ppuiPdsInitialDelayMinus2L1;
//~JVT-W080

  //===== auxiliary buffers =====
  UInt                        m_uiWriteBufferSize;                  // size of temporary write buffer
  UChar*                      m_pucWriteBuffer;                     // write buffer

  Bool                        m_bTraceEnable;
//SEI LSJ{
  Double					  m_dMVCFinalBitrate  [MAX_DSTAGES_MVC];
  Double					  m_dMVCFinalFramerate[MAX_DSTAGES_MVC];
  Double                      m_adMVCSeqBits         [MAX_DSTAGES_MVC+1];
  UInt                        m_auiMVCNumFramesCoded [MAX_DSTAGES_MVC+1];
  Double					  m_adMVCPicBits	  [MAX_FRAMERATE];
  Double					  m_adMaxBitRate;
  UInt						  m_uiNumPics;
//SEI LSJ}

  //===== multiview stuff

  // The m_vFramePeriod controls period insertion of V-frames.  A V-frame
  // is a frame which may do prediction from other sequences at the same
  // time index, but does not do any temporal prediciton.  This class
  // does not actually implement a V-frame (that is done by RecPicBuffer).
  // Instead, this class just keeps track of the V-frame period and when
  // V-frames should happen using the TimeForVFrameP member function.
  unsigned int  m_vFramePeriod;
  unsigned int	m_CurrentViewId;
  
  Bool          m_bAVCFlag;
  Bool m_SvcMvcFlag;
  



public:
  MultiviewReferencePictureManager m_MultiviewRefPicManager;

};


H264AVC_NAMESPACE_END


#endif // _PIC_ENCODER_INCLUDED_

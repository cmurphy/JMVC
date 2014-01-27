#if !defined(AFX_H264AVCDECODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)
#define AFX_H264AVCDECODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/Sei.h" 
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "DownConvert.h"
#include "H264AVCCommonLib/MotionCompensation.h"

// TMM_ESS 
#include "ResizeParameters.h"


H264AVC_NAMESPACE_BEGIN

class SliceReader;
class SliceDecoder;
class FrameMng;
class PocCalculator;
class LoopFilter;
class HeaderSymbolReadIf;
class ParameterSetMng;
class NalUnitParser;
class ControlMngIf;


class H264AVCDECODERLIB_API H264AVCDecoder
{ 
protected:
	H264AVCDecoder         ();
  virtual ~H264AVCDecoder();

public:
  static  ErrVal create ( H264AVCDecoder*& rpcH264AVCDecoder );
  ErrVal destroy();
  ErrVal init   ( 
                  SliceReader*        pcSliceReader,
                  SliceDecoder*       pcSliceDecoder,
                  FrameMng*           pcFrameMng,
                  NalUnitParser*      pcNalUnitParser,
                  ControlMngIf*       pcControlMng,
                  LoopFilter*         pcLoopFilter,
                  HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                  ParameterSetMng*    pcParameterSetMng,
                  PocCalculator*      pcPocCalculator,
                  MotionCompensation* pcMotionCompensation );
  ErrVal uninit ();

//JVT-V054
  SliceHeader *getPrevSliceHeader() const { return m_pcPrevSliceHeader ; }

//JVT-S036 start
  SliceHeader *getSliceHeader() const { return m_pcSliceHeader ; }
  ErrVal  initPacketPrefix( BinDataAccessor*  pcBinDataAccessor,
                            UInt&             ruiNalUnitType
                            , Bool            bPreParseHeader //FRAG_FIX
                            , Bool			      bConcatenated //FRAG_FIX_3
                            , Bool&           rbStartDecoding
                            ,SliceHeader *   pcSliceHeader
                           ,NalUnitParser*          pcNalUnitParser
    );
//JVT-S036 lsj end
  ErrVal  initPacket( BinDataAccessor*  pcBinDataAccessor,
	                    UInt&             ruiNalUnitType,
	                    UInt&             ruiMbX,
	                    UInt&             ruiMbY,
	                    UInt&             ruiSize
						//,UInt&				ruiNonRequiredPic  //NonRequired JVT-Q066
                        //JVT-P031
	                      ,Bool              bPreParseHeader //FRAG_FIX
		                  , Bool			bConcatenated //FRAG_FIX_3
                        ,Bool&             rbStartDecoding,
                        UInt&             ruiStartPos,
                        UInt&             ruiEndPos,
                        Bool&             bFragmented,
                        Bool&             bDiscardable
                        //~JVT-P031
						,Bool&			  UnitAVCFlag	//JVT-S036 
						,UInt NumOfViewsInTheStream
                        ,Bool& bSkip); 
//Dec. 1 fix view order for base view {{                        
  UInt*   getViewOrder  () { return m_puiViewOrder; } 
  UInt*   getViewOrder_SubStream  () { return m_puiViewOrder_SubStream; } 
  void addViewOrder()
  {
	  	m_puiViewOrder=new UInt[1];
		m_puiViewOrder[0]=0;
  }
//}}           
  //JVT-P031
  ErrVal  initPacket( BinDataAccessor*  pcBinDataAccessor);
  Void    getDecodedResolution(UInt &uiLayerId);
  UInt    getNumOfNALInAU() {return m_uiNumOfNALInAU;}
  Void    decreaseNumOfNALInAU(){ m_uiNumOfNALInAU--;}
  Void    setDependencyInitialized(Bool b) { m_bDependencyInitialized = b;}
  Void    initNumberOfFragment();
  //~JVT-P031

  UInt    getCrop(UInt ui)
  {
	  if(ui<4) 
		  return m_uiCropOffset[ui];
	  else
		  return 0;
  };

  ErrVal  process   ( PicBuffer*        pcPicBuffer,
                      PicBufferList&    rcPicBufferOutputList,
                      PicBufferList&    rcPicBufferUnusedList,
                      PicBufferList&    rcPicBufferReleaseList );

  // ROI DECODE Init ICU/ETRI
  Void	  RoiDecodeInit() 
  {
	    m_bCurNalIsEndOfPic		= false;
		m_iCurNalSpatialLayer 	= -1;
		m_iNextNalSpatialLayer  = -1;
		m_iCurNalPOC			= -1;
		m_iNextNalPOC			= -1;
		m_iCurNalFirstMb		= -1;
  }

  Bool	  IsSliceEndOfPic()				{ return m_bCurNalIsEndOfPic;  }
  /*
  ErrVal  getBaseLayerData              ( IntFrame*&      pcFrame,
                                          IntFrame*&      pcResidual,
                                          MbDataCtrl*&    pcMbDataCtrl,
                                          MbDataCtrl*&    pcMbDataCtrlEL,
                                          Bool&           rbConstrainedIPred,
                                          Bool&           rbSpatialScalability,
                                          UInt            uiLayerId,
                                          UInt            uiBaseLayerId,
                                          Int             iPoc,
                                          UInt            uiBaseQualityLevel); //JVT-T054
                                          */
  ErrVal  getBaseLayerPWTable          ( SliceHeader::PredWeightTable*& rpcPredWeightTable,
                                         UInt                           uiBaseLayerId,
                                         ListIdx                        eListIdx,
                                         Int                            iPoc );

  Void    setBaseAVCCompatible        ( Bool                        bAVCCompatible )    { m_bBaseLayerIsAVCCompatible = bAVCCompatible; }
  Void    setReconstructionLayerId    ( UInt                        uiLayerId )         { m_uiRecLayerId = uiLayerId; }
  Void    setVeryFirstSPS             (  SequenceParameterSet* pcSPS )                  { m_pcVeryFirstSPS = pcSPS; }

  ErrVal  calculatePoc                ( NalUnitType       eNalUnitType,
                                        SliceHeader&      rcSliceHeader,
                                        Int&              slicePoc  );
  ErrVal  checkSliceLayerDependency   ( BinDataAccessor*  pcBinDataAccessor,
                                        Bool&             bFinishChecking 
									    ,Bool&			  UnitAVCFlag    //JVT-S036 	
									   );
//	TMM_EC {{
	Bool		checkSEIForErrorConceal();
  ErrVal  checkSliceGap   ( BinDataAccessor*  pcBinDataAccessor,
                            MyList<BinData*>&	cVirtualSliceList 
							,Bool&			  UnitAVCFlag		//JVT-S036 
						   );
	ErrVal	setec( UInt uiErrorConceal) { m_eErrorConceal = (ERROR_CONCEAL)(EC_NONE + uiErrorConceal); if ( m_eErrorConceal == 0) m_bNotSupport = true; return	Err::m_nOK;}
	UInt	m_uiNextFrameNum;
	UInt	m_uiNextLayerId;
	UInt	m_uiNextPoc;
	UInt	m_uiNextTempLevel;
	UInt	*m_pauiPocInGOP[MAX_LAYERS];
	UInt	*m_pauiFrameNumInGOP[MAX_LAYERS];
	UInt	*m_pauiTempLevelInGOP[MAX_LAYERS];
	UInt	m_uiDecompositionStages[MAX_LAYERS];
	UInt	m_uiNumLayers;
	UInt	m_uiFrameIdx[MAX_LAYERS];
	ERROR_CONCEAL	m_eErrorConceal;
	UInt	m_uiDefNumLayers;
	UInt	m_uiDefDecompositionStages[MAX_LAYERS];
	UInt	m_uiMaxDecompositionStages;
	UInt	m_uiMaxGopSize;
	UInt	m_uiGopSize[MAX_LAYERS];
	Bool	m_bNotSupport;
	UInt	m_uiMaxLayerId;
//  TMM_EC }}
  Void    setQualityLevelForPrediction( UInt ui ) { m_uiQualityLevelForPrediction = ui; }

  UInt isNonRequiredPic()						  { return m_uiNonRequiredPic;  } //NonRequired JVT-Q066
  Bool isRedundantPic()             { return m_bRedundantPic; }  // JVT-Q054 Red. Picture
  ErrVal  checkRedundantPic();  // JVT-Q054 Red. Picture
  Void    setFGSRefInAU(Bool &b); //JVT-T054
protected:

  ErrVal  xInitSlice                ( SliceHeader*    pcSliceHeader );
  ErrVal  xStartSlice               ( Bool& bPreParseHeader, Bool& bLastFragment, Bool& bDiscardable, Bool UnitAVCFlag); //FRAG_FIX //TMM_EC//JVT-S036 
  // TMM_EC {{
  ErrVal  xProcessSliceVirtual      ( SliceHeader&    rcSH,
	                                    SliceHeader* pcPrevSH,
									                    PicBuffer* &    rpcPicBuffer);
  // TMM_EC }}
  ErrVal  xProcessSlice             ( SliceHeader&    rcSH,
                                      SliceHeader*    pcPrevSH,
                                      PicBuffer*&     rpcPicBuffer,
                                      PicBufferList&   rcPicBufferOutputList,
                                      PicBufferList&   rcPicBufferUnusedList,
                                      Bool            bHighestLayer); //JVT-T054

  ErrVal  xZeroIntraMacroblocks     ( IntFrame*       pcFrame,
                                      MbDataCtrl*     pcMbDataCtrl,
                                      SliceHeader*    pcSliceHeader );

  ErrVal  freeDiffPrdRefLists       ( RefFrameList& diffPrdRefList);

protected:
  SliceReader*                  m_pcSliceReader;
  SliceDecoder*                 m_pcSliceDecoder;
  FrameMng*                     m_pcFrameMng;
  NalUnitParser*                m_pcNalUnitParser;
  ControlMngIf*                 m_pcControlMng;
  LoopFilter*                   m_pcLoopFilter;
  HeaderSymbolReadIf*           m_pcHeaderSymbolReadIf;
  ParameterSetMng*              m_pcParameterSetMng;
  PocCalculator*                m_pcPocCalculator;
  SliceHeader*                  m_pcSliceHeader;
  SliceHeader*                  m_pcPrevSliceHeader;
  SliceHeader*                  m_pcSliceHeader_backup; //JVT-Q054 Red. Picture
  Bool                          m_bFirstSliceHeaderBackup;  //JVT-Q054 Red. Picture
  Bool                          m_bRedundantPic;  // JVT-Q054 Red. Picture
  Bool                          m_bInitDone;
  Bool                          m_bLastFrame;
  Bool                          m_bFrameDone;
  MotionCompensation*           m_pcMotionCompensation;


  PicBuffer*                    m_pcFGSPicBuffer;

  Bool                          m_bEnhancementLayer;
  Bool                          m_bActive;
  Bool                          m_bReconstruct;
  Bool                          m_bBaseLayerIsAVCCompatible;
	Bool                          m_bNewSPS;
  UInt                          m_uiRecLayerId;
  UInt                          m_uiLastLayerId;
  SequenceParameterSet*         m_pcVeryFirstSPS;
  SliceHeader*                  m_pcVeryFirstSliceHeader;

  Bool                          m_bCheckNextSlice;
  Bool                          m_bDependencyInitialized;

  Int                           m_iLastPocChecked;
  Int                           m_iFirstSlicePoc;

  Int                           m_iFirstLayerIdx;
  Int                           m_iLastLayerIdx;
  Bool                          m_bBaseLayerAvcCompliant;

  Int                           m_auiBaseLayerId[MAX_LAYERS];
  Int                           m_auiBaseQualityLevel[MAX_LAYERS];

  // should this layer be decoded at all, and up to which FGS layer should be decoded
  UInt                          m_uiQualityLevelForPrediction;

  Bool                          m_bFGSCodingMode;
  UInt                          m_uiGroupingSize;
  UInt                          m_uiPosVect[16];

//Dec. 1 fix view order for base view {{                        
  UInt*                         m_puiViewOrder;
  UInt*                         m_puiViewOrder_SubStream;
//}}          

//SEI {
  UInt							m_uiScalableNestingSeiFlag;
  UInt							m_uiScalableNestingSeiRead;
  UInt							m_uiSnapshotSeiFlag;
  Bool							m_bAllPicturesInAuFlag;
  UInt							m_uiNumPicturesMinus1;
  UInt						    m_uiTemporalId;
  UInt							m_uiPicId[MAX_PICTURES_IN_ACCESS_UNIT];
  UInt							m_uiSnapShotId;
  int							m_uiPoc;

  Bool							m_bOpPresentFlag;
  UInt						    m_uiOperationPointId;
  UInt							m_uiNumActiveViews;
  UInt*							m_uiActiveViewId;					
  UInt							m_uiNumDecodeViews;
  UInt							m_uiNumOpMinus1;
  UInt							m_uiNumViews[MAX_OPERATION_POINTS];
  UInt*							m_OpViewId[MAX_OPERATION_POINTS];
  //JVT-AB025 {{
  UInt              m_uiTargetViewId;   
  UInt              *m_auiViewOrderIndex;
  UInt              *m_auiNumNonReqViewCop;
  UInt              **m_aauiIndexDelta;
  UInt              **m_aauiNonReqViewOrderIndex;
  UInt              m_uiNumTargetViewMinus1;
  UInt              m_uiOPNotPresentSeiFlag; 
  UInt              m_uiNumNotPresentOP;
  UInt              m_auiNotPresentOPID[MAX_OPERATION_POINTS];
  //JVT-AB025 }}
//SEI }

  UInt							m_uiMultiviewSceneInfoSeiFlag; // SEI JVT-W060
  UInt							m_uiMaxDisparity; // SEI JVT-W060
  UInt							m_uiMultiviewAcquisitionInfoSeiFlag; // SEI JVT-W060
  SEI::NonRequiredSei*			m_pcNonRequiredSei;
  UInt							m_uiNonRequiredSeiReadFlag;
	UInt							m_uiNonRequiredSeiRead;
	UInt							m_uiNonRequiredPic;	//NonRequired JVT-Q066	
  UInt							m_uiPrevPicLayer;
  UInt							m_uiCurrPicLayer;
  //JVT-P031
  UInt                          m_uiFirstFragmentPPSId;
  UInt                          m_uiFirstFragmentNumMbsInSlice;
  Bool                          m_bFirstFragmentFGSCompSep;
  UInt                          m_uiLastFragOrder;
  UInt                          m_uiNumberOfFragment[MAX_LAYERS];
  UInt                          m_uiNumberOfSPS;
  UInt                          m_uiSPSId[MAX_LAYERS];
  UInt                          m_uiDecodedLayer;
  UInt                          m_uiNumOfNALInAU;
  SliceHeader*                  m_pcSliceHeaderStored;
  Int                           m_iPrevPoc;
  //~JVT-P031

  UInt                    m_uiCropOffset[4];//lufeng

  SliceHeader::PredWeightTable  m_acLastPredWeightTable[2];

  // ROI DECODE ICU/ETRI
  int	m_iCurNalSpatialLayer;
  int	m_iNextNalSpatialLayer;
			   
  int	m_iCurNalPOC;
  int	m_iNextNalPOC;
  int   m_iCurNalFirstMb;

  bool	m_bCurNalIsEndOfPic;
  bool	m_bFirstFGS;

//JVT-T054{
  Bool                          m_bLastNalInAU;
  Bool                          m_bFGSRefInAU;
  Bool                          m_bAVCBased;
  Bool                          m_bCGSSNRInAU;
  Bool                          m_bOnlyAVCAtLayer;
//JVT-T054}
public:
  MbDataCtrl*         m_pcBaseLayerCtrlEL;
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_H264AVCDECODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)

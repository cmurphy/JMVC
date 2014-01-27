#if !defined(AFX_SLICEREADER_H__5B23143A_D267_40C2_908E_164029C1298E__INCLUDED_)
#define AFX_SLICEREADER_H__5B23143A_D267_40C2_908E_164029C1298E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/HeaderSymbolReadIf.h"
#include "H264AVCCommonLib/ControlMngIf.h"

H264AVC_NAMESPACE_BEGIN

class MbParser;
class ParameterSetMng;

class SliceReader
{
protected:
	SliceReader();
	virtual ~SliceReader();

public:
  static ErrVal create( SliceReader*& rpcSliceReader );
  ErrVal destroy();
  ErrVal init(  HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                ParameterSetMng* pcParameterSetMng,
                MbParser* pcMbParser,
                ControlMngIf* pcControlMng );
  ErrVal uninit();

  // JVT-S054 (2) (REPLACE)
  //ErrVal process( const SliceHeader& rcSH, UInt& ruiMbRead );

  ErrVal process( SliceHeader& rcSH, UInt& ruiMbRead );

  
  ErrVal readSliceHeader  ( NalUnitType   eNalUnitType,

                            Bool m_svc_mvc_flag,	
                            Bool bNonIDRFlag, // JVT-W035 
                            Bool bAnchorPicFlag,
                            UInt uiViewId,
							Bool uiInterViewFlag, //JVT-W056 Samsung

                            NalRefIdc     eNalRefIdc,
                            UInt          uiLayerId,
                            UInt          uiTemporalLevel,
                            UInt          uiQualityLevel,
                            SliceHeader*& rpcSH
                            //JVT-P031
                            ,UInt         uiFirstFragSHPPSId
                            ,UInt         uiFirstFragNumMbsInSlice
                            ,Bool         bFirstFragFGSCompSep
                            //~JVT-P031
							,Bool		  UnitAVCFlag	//JVT-S036 lsj
                            );

//Purvin: original function
  ErrVal readSliceHeader  ( NalUnitType   eNalUnitType,
                            NalRefIdc     eNalRefIdc,
                            UInt          uiLayerId,
                            UInt          uiTemporalLevel,
                            UInt          uiQualityLevel,
                            SliceHeader*& rpcSH
                            //JVT-P031
                            ,UInt         uiFirstFragSHPPSId
                            ,UInt         uiFirstFragNumMbsInSlice
                            ,Bool         bFirstFragFGSCompSep
                            //~JVT-P031
                            ,Bool		  UnitAVCFlag	//JVT-S036 
    );
  
  ErrVal readSliceHeaderPrefix( NalUnitType   eNalUnitType,
							 Bool m_svc_mvc_flag,	
							 //JVT-W035
								NalRefIdc     eNalRefIdc,
								UInt		  uiLayerId,
								UInt		  uiQualityLevel,
								SliceHeader*  pcSliceHeader
							  );					//JVT-S036 


  //TMM_EC {{
	ErrVal	readSliceHeaderVirtual(	NalUnitType   eNalUnitType,
		                              SliceHeader	*rpcVeryFirstSliceHeader,
																	UInt	uiDecompositionStages,
																	UInt	uiMaxDecompositionStages,
																	UInt	uiGopSize,
																	UInt	uiMaxGopSize,
																	UInt	uiFrameNum,
																	UInt	uiPocLsb,
																	UInt	uiTemporalLevel,
																	SliceHeader*& rpcSH);
  //TMM_EC }}
  ErrVal  read           ( SliceHeader&   rcSH,
                           MbDataCtrl*    pcMbDataCtrl,
                           MbDataCtrl*    pcMbDataCtrlBase,
                           Int             iSpatialScalabilityType,
                           UInt           uiMbInRow,
                           UInt&          ruiMbRead );
//	TMM_EC {{
	ErrVal  readVirtual    ( SliceHeader&   rcSH,
                           MbDataCtrl*    pcMbDataCtrl,
                           MbDataCtrl*    pcMbDataCtrlRef,
                           MbDataCtrl*    pcMbDataCtrlBase,
                           Int             iSpatialScalabilityType,
                           UInt           uiMbInRow,
                           UInt&          ruiMbRead,
													 ERROR_CONCEAL      m_eErrorConceal);

protected:
  HeaderSymbolReadIf* m_pcHeaderReadIf;
  ParameterSetMng *m_pcParameterSetMng;
  MbParser* m_pcMbParser;
  ControlMngIf* m_pcControlMng;
  Bool m_bInitDone;
//JVT-S036  start
  UInt KeyPictureFlag;
  Bool uiAdaptiveRefPicMarkingModeFlag;
  MmcoBuffer m_cMmmcoBufferSuffix; 
	UInt PPSId_AVC, SPSId_AVC;
	UInt POC_AVC;
//JVT-S036  end 

};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_SLICEREADER_H__5B23143A_D267_40C2_908E_164029C1298E__INCLUDED_)

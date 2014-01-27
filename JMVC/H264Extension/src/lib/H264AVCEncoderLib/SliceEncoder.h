#if !defined(AFX_SLICEENCODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_)
#define AFX_SLICEENCODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/ControlMngIf.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "MbEncoder.h"


H264AVC_NAMESPACE_BEGIN


class CodingParameter;
class MbCoder;
class PocCalculator;


class SliceEncoder
{
protected:
	SliceEncoder();
	virtual ~SliceEncoder();

public:
  static ErrVal create( SliceEncoder*& rpcSliceEncoder );
  ErrVal destroy();
  ErrVal init( MbEncoder* pcMbEncoder,
               MbCoder* pcMbCoder,
               ControlMngIf* pcControlMng,
               CodingParameter* pcCodingParameter,
               PocCalculator* pcPocCalculator,
               Transform* pcTransform);

  ErrVal uninit();

  MbEncoder*  getMbEncoder            () { return m_pcMbEncoder; }



  ErrVal      encodeSlice             ( SliceHeader&  rcSliceHeader,
                                        IntFrame*     pcFrame,
                                        MbDataCtrl*   pcMbDataCtrl,
                                        RefFrameList& rcList0,
                                        RefFrameList& rcList1,
                                        UInt          uiMbInRow,
                                        Double        dlambda	);
  ErrVal      encodeSliceMbAff             ( SliceHeader&  rcSliceHeader,
                                      IntFrame*     pcFrame,
                                      MbDataCtrl*   pcMbDataCtrl,
                                      RefFrameList& rcList0,
                                      RefFrameList& rcList1,
                                      UInt          uiMbInRow,
                                      Double        dlambda	);
//TMM_WP
  ErrVal xSetPredWeights( SliceHeader& rcSliceHeader, 
                          IntFrame* pOrgFrame,
                          RefFrameList& rcList0,
                          RefFrameList& rcList1 );


  ErrVal xInitDefaultWeights(Double *pdWeights, 
                             UInt uiLumaWeightDenom, 
                             UInt uiChromaWeightDenom);

//TMM_WP

  //S051{
  Void		setUseBDir			(Bool b){m_pcMbEncoder->setUseBDir(b);}
  //S051}
//JVT-W080
	Void        setPdsEnable              ( UInt   uiValue ) { m_uiPdsEnable                 = uiValue; }
	Void        setPdsInitialDelayMinus2L0( UInt** uiValue ) { m_ppuiPdsInitialDelayMinus2L0 = uiValue; }
	Void        setPdsInitialDelayMinus2L1( UInt** uiValue ) { m_ppuiPdsInitialDelayMinus2L1 = uiValue; }
	Void        setPdsBlockSize           ( UInt   uiValue ) { m_uiPdsBlockSize              = uiValue; }
	UInt        getPdsEnable              ()         const { return m_uiPdsEnable;                  }
	UInt**      getPdsInitialDelayMinus2L0()         const { return m_ppuiPdsInitialDelayMinus2L0;  }
	UInt**      getPdsInitialDelayMinus2L1()         const { return m_ppuiPdsInitialDelayMinus2L1;  }
	UInt        getPdsBlockSize           ()         const { return m_uiPdsBlockSize;               }
//~JVT-W080
protected:
  MbEncoder* m_pcMbEncoder;
  MbCoder* m_pcMbCoder;
  ControlMngIf* m_pcControlMng;
  CodingParameter* m_pcCodingParameter;
  PocCalculator*   m_pcPocCalculator;
  Transform* m_pcTransform;
  Bool m_bInitDone;
  UInt  m_uiFrameCount;
  SliceType m_eSliceType;
  Bool m_bTraceEnable;
//JVT-W080
	UInt   m_uiPdsEnable;
	UInt   m_uiPdsBlockSize;
	UInt** m_ppuiPdsInitialDelayMinus2L0;
	UInt** m_ppuiPdsInitialDelayMinus2L1;
//~JVT-W080

};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_SLICEENCODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_)

#if !defined(AFX_SLICEDECODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_)
#define AFX_SLICEDECODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/ControlMngIf.h"

H264AVC_NAMESPACE_BEGIN

class MbDecoder;
class IntFrame;
class Transform;

class SliceDecoder
{
protected:
	SliceDecoder();
	virtual ~SliceDecoder();

public:
  static ErrVal create( SliceDecoder*&      rpcSliceDecoder );
  ErrVal destroy();

  ErrVal init         ( MbDecoder*          pcMbDecoder,
                        ControlMngIf*       pcControlMng,
                        Transform*          pcTransform );
  ErrVal uninit       ();

//	TMM_EC {{
	ErrVal processVirtual( const SliceHeader& rcSH,
                         Bool                bReconstructAll,
                         UInt uiMbRead );
//	TMM_EC }}
  ErrVal process      ( const SliceHeader&  rcSH,
                        Bool                bReconstructAll,
                        UInt                uiMbRead );
  ErrVal decode       ( SliceHeader&        rcSH,
                        MbDataCtrl*         pcMbDataCtrl,
                        MbDataCtrl*         pcMbDataCtrlBase,
                        IntFrame*           pcFrame,
                        IntFrame*           pcResidual,
                        IntFrame*           pcPredSignal,
                        IntFrame*           pcBaseLayer,
                        IntFrame*           pcBaseLayerResidual,
                        RefFrameList*       pcRefFrameList0,
                        RefFrameList*       pcRefFrameList1,
                        Bool                bReconstructAll,
                        UInt                uiMbInRow,
                        UInt                uiMbRead );
  ErrVal compensatePrediction( SliceHeader& rcSH );

protected:
  MbDecoder*    m_pcMbDecoder;
  ControlMngIf* m_pcControlMng;
  Transform*    m_pcTransform;
  Bool          m_bInitDone;

};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_SLICEDECODER_H__A0183156_A54B_425D_8CF1_D350651F638C__INCLUDED_)

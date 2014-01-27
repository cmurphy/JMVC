#if !defined(AFX_MBDECODER_H__F725C8AD_2589_44AD_B904_62FE2A7F7D8D__INCLUDED_)
#define AFX_MBDECODER_H__F725C8AD_2589_44AD_B904_62FE2A7F7D8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/Transform.h"

H264AVC_NAMESPACE_BEGIN

class Transform;
class IntraPrediction;
class MotionCompensation;
class FrameMng;

class IntFrame;
class IntFrameBase;
class IntYuvPicBuffer;
class IntYuvPicBufferBase;
class IntYuvMbBuffer;
class IntYuvMbBufferBase;


class MbDecoder
{
protected:
	MbDecoder();
	virtual ~MbDecoder();

public:
  static ErrVal create          ( MbDecoder*&         rpcMbDecoder );
  ErrVal destroy                ();

  ErrVal init                   ( Transform*          pcTransform,
                                  IntraPrediction*    pcIntraPrediction,
                                  MotionCompensation* pcMotionCompensation,
                                  FrameMng*           pcFrameMng  );
  ErrVal uninit                 ();

  ErrVal process                ( MbDataAccess& rcMbDataAccess,
                                  Bool          bReconstructAll  );
  ErrVal decode                 ( MbDataAccess& rcMbDataAccess,
                                  MbDataAccess* pcMbDataAccessBase,
                                  IntFrame*     pcFrame,
                                  IntFrame*     pcResidual,
                                  IntFrame*     pcPredSignal,
                                  IntFrame*     pcBaseLayer,
                                  IntFrame*     pcBaseLayerResidual,
                                  RefFrameList* pcRefFrameList0,
                                  RefFrameList* pcRefFrameList1,
                                  Bool          bReconstructAll );
  ErrVal calcMv                 ( MbDataAccess& rcMbDataAccess,
                                  MbDataAccess* pcMbDataAccessBaseMotion );
  ErrVal compensatePrediction   ( MbDataAccess& rcMbDataAccess );

protected:
	ErrVal xDecodeMbPCM           ( MbDataAccess&     rcMbDataAccess,
                                  YuvMbBuffer&      rcRecYuvBuffer );
  ErrVal xDecodeMbInter         ( MbDataAccess&     rcMbDataAccess,
                                  YuvMbBuffer&      rcRecYuvBuffer, 
                                  IntYuvMbBuffer&   rcPredIntYuvMbBuffer,
                                  IntYuvMbBuffer&   rcResIntYuvMbBuffer,
                                  Bool              bReconstruct );

  ErrVal xDecodeChroma          ( MbDataAccess&     rcMbDataAccess,
                                  YuvMbBuffer&      rcRecYuvBuffer,
                                  UInt              uiChromaCbp,
                                  Bool              bPredChroma );
  
  
  ErrVal xDecodeMbPCM           ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvPicBuffer*  pcRecYuvBuffer );
  ErrVal xDecodeMbIntra4x4      ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvMbBuffer&   cYuvMbBuffer
                                  ,IntYuvMbBuffer&  rcPredBuffer );
  ErrVal xDecodeMbIntra8x8      ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvMbBuffer&   cYuvMbBuffer
                                  ,IntYuvMbBuffer&  rcPredBuffer );
  ErrVal xDecodeMbIntra16x16    ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvMbBuffer&   cYuvMbBuffer
                                  ,IntYuvMbBuffer&  rcPredBuffer );
  ErrVal xDecodeMbIntraBL       ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvPicBuffer*  pcRecYuvBuffer,
                                  IntYuvMbBuffer&   rcPredBuffer,
                                  IntYuvPicBuffer*  pcBaseYuvBuffer );
  ErrVal xDecodeMbInter         ( MbDataAccess&     rcMbDataAccess,
                                  MbDataAccess*     pcMbDataAccessBase,
                                  IntYuvMbBuffer&   rcPredBuffer,
                                  IntYuvPicBuffer*  pcRecYuvBuffer,
                                  IntFrame*         pcResidual,
                                  IntFrame*         pcBaseResidual,
                                  RefFrameList&     rcRefFrameList0, 
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bReconstruct );
  ErrVal xDecodeChroma          ( MbDataAccess&     rcMbDataAccess,
                                  IntYuvMbBuffer&   rcRecYuvBuffer,
                                  IntYuvMbBuffer&   rcPredBuffer,
                                  UInt              uiChromaCbp,
                                  Bool              bPredChroma );
  
  
  ErrVal xScaleTCoeffs          ( MbDataAccess&      rcMbDataAccess );
  ErrVal xScale4x4Block         ( TCoeff*            piCoeff,
                                  const UChar*       pucScale,
                                  UInt               uiStart,
                                  const QpParameter& rcQP );
  ErrVal xScale8x8Block         ( TCoeff*            piCoeff,
                                  const UChar*       pucScale,
                                  const QpParameter& rcQP );

  ErrVal xPredictionFromBaseLayer( MbDataAccess&  rcMbDataAccess,
                                   MbDataAccess*  pcMbDataAccessBase );
protected:
  MbTransformCoeffs   m_cTCoeffs;
  
  Transform*          m_pcTransform;
  IntraPrediction*    m_pcIntraPrediction;
  MotionCompensation* m_pcMotionCompensation;
  FrameMng*           m_pcFrameMng;
  Bool                m_bInitDone;
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_MBDECODER_H__F725C8AD_2589_44AD_B904_62FE2A7F7D8D__INCLUDED_)

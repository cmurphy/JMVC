#if !defined(AFX_CONTROLMNG_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_)
#define AFX_CONTROLMNG_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/ControlMngIf.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/PocCalculator.h"
#include "SliceReader.h"
#include "NalUnitParser.h"
#include "SliceDecoder.h"
#include "BitReadBuffer.h"
#include "UvlcReader.h"
#include "MbParser.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "MbDecoder.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/IntraPrediction.h"
#include "H264AVCCommonLib/MotionCompensation.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "CabacReader.h"
#include "H264AVCCommonLib/SampleWeighting.h"
//#include "GOPDecoder.h"
#include "H264AVCDecoder.h"

// TMM_ESS 
#include "ResizeParameters.h"

//class MCTFDecoder; 
H264AVC_NAMESPACE_BEGIN

class ControlMngH264AVCDecoder : public ControlMngIf
{
protected:
	ControlMngH264AVCDecoder();
	virtual ~ControlMngH264AVCDecoder();

public:
  static ErrVal create( ControlMngH264AVCDecoder*& rpcControlMngH264AVCDecoder );

  ErrVal init(  FrameMng*           pcFrameMng,
                ParameterSetMng*    pcParameterSetMng,
                PocCalculator*      apcPocCalculator      [MAX_LAYERS],
                SliceReader*        pcSliceReader,
                NalUnitParser*      pcNalUnitParser,
                SliceDecoder*       pcSliceDecoder,
                BitReadBuffer*      pcBitReadBuffer,
                UvlcReader*         pcUvlcReader,
                MbParser*           pcMbParser,
                LoopFilter*         pcLoopFilter,
                MbDecoder*          pcMbDecoder,
                Transform*          pcTransform,
                IntraPrediction*    pcIntraPrediction,
                MotionCompensation* pcMotionCompensation,
                YuvBufferCtrl*      pcYuvFullPelBufferCtrl[MAX_LAYERS],
                QuarterPelFilter*   pcQuarterPelFilter,
                CabacReader*        pcCabacReader,
                SampleWeighting*    pcSampleWeighting,
                /*MCTFDecoder*        apcMCTFDecoder        [MAX_LAYERS],*/
                H264AVCDecoder*        pcH264AVCDecoder );

  ErrVal uninit();
  ErrVal destroy();

  ErrVal initMbForParsing     ( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex );
  ErrVal initMbForDecoding    (MbDataAccess*& rpcMbDataAccess,UInt uiMbY, UInt uiMbX, Bool bMbAFF  );
    ErrVal initMbForFiltering   ( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAFF );
    ErrVal initMbForFiltering   ( UInt uiMbY, UInt uiMbX, Bool bMbAFF );



//  ErrVal initSlice0           (SliceHeader *rcSH);
  ErrVal initSlice0           (SliceHeader *rcSH, UInt NumOfViewsInTheStream);
  
  // TMM_ESS 
  ErrVal initSPS              ( SequenceParameterSet& rcSequenceParameterSet,
                                UInt  uiLayer );
	
  ErrVal initSPS              ( SequenceParameterSet& rcSequenceParameterSet );
  ErrVal initParameterSets    ( const SequenceParameterSet& rcSPS,
                                const PictureParameterSet&  rcPPSLP,
                                const PictureParameterSet&  rcPPSHP  )    { return Err::m_nERR; }

  ErrVal initSlice            ( SliceHeader& rcSH, ProcessingState eProcessingState );
  ErrVal finishSlice          ( const SliceHeader& rcSH, Bool& rbPicDone, Bool& rbFrameDone );
  ErrVal initSliceForCoding   ( const SliceHeader& rcSH ) { return Err::m_nERR; }
  ErrVal initSliceForReading  ( const SliceHeader& rcSH );
  ErrVal initSliceForDecoding ( const SliceHeader& rcSH );
  ErrVal initSliceForFiltering( const SliceHeader& rcSH );

  //ErrVal initMbForCoding      ( MbDataAccess& rcMbDataAccess, UInt uiMbIndex ) { return Err::m_nERR; }
  ErrVal initMbForCoding( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAFF, Bool bFieldFlag ){ return Err::m_nERR; }

  UvlcReader*  getUvlcReader()  { return m_pcUvlcReader;  };
  CabacReader* getCabacReader() { return m_pcCabacReader; };

//TMM_WP
  ErrVal initSliceForWeighting   ( const SliceHeader& rcSH ) {  return m_pcSampleWeighting->initSlice( rcSH ); }
//TMM_WP
  MbDataCtrl* getMbDataCtrl() { return m_pcMbDataCtrl;} //JVT-T054
protected:
  ErrVal xInitESS             ( SliceHeader* pcSliceHeader );


protected:
  UInt                    m_uiCurrLayer;
  Bool                    m_bLayer0IsAVC;
  UInt                    m_auiMbXinFrame           [MAX_LAYERS]; 
  UInt                    m_auiMbYinFrame           [MAX_LAYERS];
  MbDataCtrl*             m_pcMbDataCtrl;

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
  //MCTFDecoder*            m_apcMCTFDecoder          [MAX_LAYERS];
  H264AVCDecoder*         m_pcH264AVCDecoder;
  Bool                    m_uiInitilized            [MAX_LAYERS];
  Bool                    m_uiInitilized_MultiView            [MAX_LAYERS];
  ResizeParameters        m_ResizeParameter[MAX_LAYERS]; // TMM_ESS
  ResizeParameters        m_ResizeParameterCGSSNR[MAX_LAYERS][MAX_FGS_LAYERS]; // JVT-T054
  Bool                    m_bMbAff; //for future use
  const SliceHeader* m_pcSliceHeader;
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_CONTROLMNG_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_)

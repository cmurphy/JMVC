#if !defined(AFX_H264AVCENCODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)
#define AFX_H264AVCENCODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "H264AVCEncoderLib.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "DownConvert.h"
#include "H264AVCCommonLib/Sei.h"  //NonRequired JVT-Q066 (06-04-08)


H264AVC_NAMESPACE_BEGIN


class H264AVCEncoder;
class SliceHeader;
class SliceEncoder;
class FrameMng;
class PocCalculator;
class LoopFilter;
class CodingParameter;
class LayerParameters;
class RateDistortionIf;
class HeaderSymbolWriteIf;
class NalUnitEncoder;
class ControlMngIf;
class ParameterSetMng;
class ControlMngH264AVCEncoder;
class MotionEstimation;
class IntFrame;

class PocCalculator;
class CodingParameter;
class NalUnitEncoder;
class ControlMngIf;
class ParameterSetMng;
class FrameMng;


#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


typedef MyList<UInt>        UIntList;


class H264AVCENCODERLIB_API AccessUnit
{
public:
  AccessUnit  ( Int         iPoc )  : m_iPoc( iPoc )   
  , m_pcNonRequiredSei ( NULL )  //NonRequired JVT-Q066 (06-04-08)
  {}
  ~AccessUnit ()                                            {}

  Int                     getPoc          () const          { return m_iPoc; }
  ExtBinDataAccessorList& getNalUnitList  ()                { return m_cNalUnitList; }
  //NonRequired JVT-Q066 (06-04-08){{
  ErrVal				  CreatNonRequiredSei()				{ RNOK(SEI::NonRequiredSei::create( m_pcNonRequiredSei)) return Err::m_nOK;}
  SEI::NonRequiredSei*	  getNonRequiredSei()				{ return m_pcNonRequiredSei; }
  //NonRequired JVT-Q066 (06-04-08)}}

private:
  Int                     m_iPoc;
  ExtBinDataAccessorList  m_cNalUnitList;
  SEI::NonRequiredSei*	  m_pcNonRequiredSei; //NonRequired JVT-Q066 (06-04-08)
};


class H264AVCENCODERLIB_API AccessUnitList
{
public:
  AccessUnitList  ()  {}
  ~AccessUnitList ()  {}

  Void        clear           ()            { m_cAccessUnitList.clear(); }
  AccessUnit& getAccessUnit   ( Int iPoc )
  {
    std::list<AccessUnit>::iterator  iter = m_cAccessUnitList.begin();
    std::list<AccessUnit>::iterator  end  = m_cAccessUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter).getPoc() == iPoc )
      {
        return (*iter);
      }
    }
    AccessUnit cAU( iPoc );
    m_cAccessUnitList.push_back( cAU );
    return m_cAccessUnitList.back();
  }
  Void        emptyNALULists  ( ExtBinDataAccessorList& rcOutputList )
  {
    while( ! m_cAccessUnitList.empty() )
    {
      ExtBinDataAccessorList& rcNaluList = m_cAccessUnitList.front().getNalUnitList();
      rcOutputList += rcNaluList;
      rcNaluList.clear();
      m_cAccessUnitList.pop_front();
    }
  }

private:
  std::list<AccessUnit>  m_cAccessUnitList;
};



class H264AVCENCODERLIB_API H264AVCEncoder
{
protected:
	H264AVCEncoder();
	virtual ~H264AVCEncoder();

public:
  static  ErrVal create ( H264AVCEncoder*&  rpcH264AVCEncoder );
  virtual ErrVal destroy();
  virtual ErrVal init   ( 
                          ParameterSetMng*  pcParameterSetMng,
                          PocCalculator*    pcPocCalculator,
                          NalUnitEncoder*   pcNalUnitEncoder,
                          ControlMngIf*     pcControlMng,
                          CodingParameter*  pcCodingParameter,
                          FrameMng*         pcFrameMng );
  virtual ErrVal uninit ();

  ErrVal writeParameterSets ( ExtBinDataAccessor*       pcExtBinDataAccessor,
                              Bool&                     rbMoreSets );
 //JVT-W080
	ErrVal writePDSSEIMessage ( ExtBinDataAccessor* pcExtBinDataAccessor
														, const UInt uiSPSId
														, const UInt uiNumView
													  , UInt* num_refs_list0_anc
														, UInt* num_refs_list1_anc 
													  , UInt* num_refs_list0_nonanc
														, UInt* num_refs_list1_nonanc 
														, UInt  PDSInitialDelayAnc
														, UInt  PDSInitialDelayNonAnc
														);
//~JVT-W080

  ErrVal finish             ( ExtBinDataAccessorList&   rcExtBinDataAccessorList, 
                              PicBufferList*            apcPicBufferOutputList,
                              PicBufferList*            apcPicBufferUnusedList,
                              UInt&                     ruiNumCodedFrames,
                              Double&                   rdHighestLayerOutputRate );

             
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  ErrVal writeQualityLevelInfosSEI( ExtBinDataAccessor* pcExtBinDataAccessor, 
                                    UInt*               uiaQualityLevel, 
                                    UInt *              uiaDelta, 
                                    UInt                uiNumLevels, 
                                    UInt                uiLayer ) ;
    //}}Quality level estimation and modified truncation- JVTO044 and m12007

  ErrVal writeNestingSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor, Double* dSeqBits );                                           //SEI LSJ
  ErrVal writeViewScalInfoSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor, Double* dBitRate, Double* dFrameRate, Double dMaxRate ); //SEI LSJ
  ErrVal writeMultiviewSceneInfoSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor, Double* dSeqBits );   // SEI JVT-W060
  ErrVal writeMultiviewAcquisitionInfoSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor, Double* dSeqBits );   // SEI JVT-W060

  Void setScalableSEIMessage  ()       { m_bScalableSeiMessage = true; }
	Bool bGetScalableSeiMessage	() const { return m_bScalableSeiMessage; }
	Void SetVeryFirstCall				()			 { m_bVeryFirstCall = true; }
	Double* dGetFramerate				()			 { return m_dFinalFramerate; }
	Double* dGetBitrate					()			 { return m_dFinalBitrate; }
	Double m_aaauidSeqBits [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
// BUG_FIX liuhui{
	UInt   getScalableLayerId( UInt uiLayer, UInt uiTempLevel, UInt uiFGS ) const { return m_aaauiScalableLayerId[uiLayer][uiTempLevel][uiFGS]; }
	Double m_aaadSingleLayerBitrate[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
	UInt   m_aaauiScalableLayerId[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
// BUG_FIX liuhui}
// JVT-S080 LMI {
  ErrVal xWriteScalableSEILayersNotPresent( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiInputLayers, UInt* m_layer_id);
  ErrVal xWriteScalableSEIDependencyChange( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiNumLayers, UInt* uiLayerId, Bool* pbLayerDependencyInfoPresentFlag, 
												  UInt* uiNumDirectDependentLayers, UInt** puiDirectDependentLayerIdDeltaMinus1, UInt* puiLayerDependencyInfoSrcLayerIdDeltaMinus1);
// JVT-S080 LMI }
protected:
  ErrVal xInitParameterSets ();
  ErrVal xWriteScalableSEI  ( ExtBinDataAccessor*       pcExtBinDataAccessor );
  ErrVal xWriteScalableSEICGSSNR  ( ExtBinDataAccessor*       pcExtBinDataAccessor ); //JVT-T054
	ErrVal xWriteSubPicSEI		( ExtBinDataAccessor*				pcExtBinDataAccessor );
	ErrVal xWriteSubPicSEI( ExtBinDataAccessor* pcExtBinDataAccessor, UInt layer_id ) ;
	ErrVal xWriteMotionSEI( ExtBinDataAccessor* pcExtBinDataAccessor, UInt sg_id ) ;
//JVT-W080
	ErrVal xWritePDSSEI(  ExtBinDataAccessor* pcExtBinDataAccessor
									          					, const UInt uiSPSId
																			, const UInt uiNumView
														          , UInt* num_refs_list0_anc
														          , UInt* num_refs_list1_anc
														          , UInt* num_refs_list0_nonanc
														          , UInt* num_refs_list1_nonanc
																			, UInt  PDSInitialDelayAnc
																			, UInt  PDSInitialDelayNonAnc
														         );
//~JVT-W080

protected:
  std::list<SequenceParameterSet*>  m_cUnWrittenSPS;
  std::list<PictureParameterSet*>   m_cUnWrittenPPS;
  PicBufferList                     m_acOrgPicBufferList[MAX_LAYERS];
  PicBufferList                     m_acRecPicBufferList[MAX_LAYERS];
  ParameterSetMng*                  m_pcParameterSetMng;
  PocCalculator*                    m_pcPocCalculator;
  NalUnitEncoder*                   m_pcNalUnitEncoder;
  ControlMngIf*                     m_pcControlMng;
  CodingParameter*                  m_pcCodingParameter;
  FrameMng*                         m_pcFrameMng;
  Bool                              m_bVeryFirstCall;
  Bool                              m_bInitDone;
  Bool                              m_bTraceEnable;

	Bool															m_bScalableSeiMessage;
  Double														m_dFinalBitrate[MAX_LAYERS * MAX_DSTAGES * MAX_QUALITY_LEVELS];
	Double														m_dFinalFramerate[MAX_LAYERS * MAX_DSTAGES * MAX_QUALITY_LEVELS];
  AccessUnitList                    m_cAccessUnitList;
};


#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif


H264AVC_NAMESPACE_END


#endif // !defined(AFX_H264AVCENCODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)

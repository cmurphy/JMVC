#include <cstdio>
#include "H264AVCEncoderLib.h"
#include "H264AVCCommonLib.h"

#include "H264AVCCommonLib/PocCalculator.h"
#include "H264AVCCommonLib/ControlMngIf.h"
#include "H264AVCCommonLib/CFMO.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "CodingParameter.h"
#include "PicEncoder.h"
#include "RecPicBuffer.h"
#include "NalUnitEncoder.h"
#include "SliceEncoder.h"


H264AVC_NAMESPACE_BEGIN


PicEncoder::PicEncoder()
: m_bInit                   ( false )
, m_bInitParameterSets      ( false )
, m_pcSequenceStructure     ( NULL )
, m_pcInputPicBuffer        ( NULL )
, m_pcSPS                   ( NULL )
, m_pcSPSBase               ( NULL )
, m_pcPPS                   ( NULL )
, m_pcPPSBase               ( NULL )
, m_pcRecPicBuffer          ( NULL )
, m_pcCodingParameter       ( NULL )
, m_pcControlMng            ( NULL )
, m_pcSliceEncoder          ( NULL )
, m_pcLoopFilter            ( NULL )
, m_pcPocCalculator         ( NULL )
, m_pcNalUnitEncoder        ( NULL )
, m_pcYuvBufferCtrlFullPel  ( NULL )
, m_pcYuvBufferCtrlHalfPel  ( NULL )
, m_pcQuarterPelFilter      ( NULL )
, m_pcMotionEstimation      ( NULL )
//JVT-W080
, m_uiPdsEnable                ( 0 )
, m_uiPdsBlockSize             ( 0 )
, m_ppuiPdsInitialDelayMinus2L0( 0 )
, m_ppuiPdsInitialDelayMinus2L1( 0 )
//~JVT-W080
, m_uiFrameWidthInMb        ( 0 )
, m_uiFrameHeightInMb       ( 0 )
, m_uiMbNumber              ( 0 )
, m_uiFrameNum              ( 0 )
, m_uiIdrPicId              ( 0 )
, m_uiCodedFrames           ( 0 )
, m_dSumYPSNR               ( 0.0 )
, m_dSumUPSNR               ( 0.0 )
, m_dSumVPSNR               ( 0.0 )
, m_uiWrittenBytes          ( 0 )
, m_uiWriteBufferSize       ( 0 )
, m_pucWriteBuffer          ( NULL )
, m_bTraceEnable            ( true )
{
}


PicEncoder::~PicEncoder()
{
  uninit();
}


ErrVal
PicEncoder::create( PicEncoder*& rpcPicEncoder )
{
  rpcPicEncoder = new PicEncoder;
	rpcPicEncoder->setpicEncoder(rpcPicEncoder);// JVT-W056
	ROF( rpcPicEncoder );
  return Err::m_nOK;
}

ErrVal
PicEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal
PicEncoder::init( CodingParameter*    pcCodingParameter,
                  ControlMngIf*       pcControlMng,
                  SliceEncoder*       pcSliceEncoder,
                  LoopFilter*         pcLoopFilter,
                  PocCalculator*      pcPocCalculator,
                  NalUnitEncoder*     pcNalUnitEncoder,
                  YuvBufferCtrl*      pcYuvBufferCtrlFullPel,
                  YuvBufferCtrl*      pcYuvBufferCtrlHalfPel,
                  QuarterPelFilter*   pcQuarterPelFilter,
                  MotionEstimation*   pcMotionEstimation )
{
  ROF( pcCodingParameter      );
  ROF( pcControlMng           );
  ROF( pcSliceEncoder         );
  ROF( pcLoopFilter           );
  ROF( pcPocCalculator        );
  ROF( pcNalUnitEncoder       );
  ROF( pcYuvBufferCtrlFullPel );
  ROF( pcYuvBufferCtrlHalfPel );
  ROF( pcQuarterPelFilter     );
  ROF( pcMotionEstimation     );

  m_pcCodingParameter       = pcCodingParameter;
  m_pcControlMng            = pcControlMng;
  m_pcSliceEncoder          = pcSliceEncoder;
  m_pcLoopFilter            = pcLoopFilter;
  m_pcPocCalculator         = pcPocCalculator;
  m_pcNalUnitEncoder        = pcNalUnitEncoder;
  m_pcYuvBufferCtrlFullPel  = pcYuvBufferCtrlFullPel;
  m_pcYuvBufferCtrlHalfPel  = pcYuvBufferCtrlHalfPel;
  m_pcQuarterPelFilter      = pcQuarterPelFilter;
  m_pcMotionEstimation      = pcMotionEstimation;

  //----- create objects -----
  RNOK( RecPicBuffer      ::create( m_pcRecPicBuffer ) );
  RNOK( SequenceStructure ::create( m_pcSequenceStructure ) );
  RNOK( InputPicBuffer    ::create( m_pcInputPicBuffer ) );

  //----- init objects -----
  RNOK( m_pcRecPicBuffer  ->init( m_pcYuvBufferCtrlFullPel, m_pcYuvBufferCtrlHalfPel ) );
  RNOK( m_pcInputPicBuffer->init() );
//JVT-W080
 	m_uiPdsEnable = m_pcCodingParameter->getMVCmode() && m_pcCodingParameter->getPdsEnable();
	m_uiPdsBlockSize      =   getPdsEnable() ? m_pcCodingParameter->getPdsBlockSize() : 0;
//~JVT-W080

  RNOK( xInitFrameSpec());
  //----- init parameters -----
  m_uiWrittenBytes          = 0;
  m_uiCodedFrames           = 0;
  m_dSumYPSNR               = 0.0;
  m_dSumVPSNR               = 0.0;
  m_dSumUPSNR               = 0.0;
  m_bInit                   = true;

//SEI {
  m_adMaxBitRate = 0.0;
  m_uiNumPics = 0;
  UInt ui;
  for( ui = 0; ui < MAX_DSTAGES_MVC; ui++ )
  {
	  m_dMVCFinalBitrate[ui] = 0;
	  m_dMVCFinalFramerate[ui] = 0;
	  m_adMVCSeqBits[ui] = 0;
	  m_auiMVCNumFramesCoded[ui] = 0;
  }
  m_adMVCSeqBits[ui] = 0;
  m_auiMVCNumFramesCoded[ui] = 0;
  for( ui = 0; ui < MAX_FRAMERATE; ui++ )
	  m_adMVCPicBits[ui] = 0;
//SEI }

  return Err::m_nOK;
}


ErrVal
PicEncoder::uninit()
{
  //----- free allocated memory -----
  RNOK( xDeleteData() );

  //----- free allocated member -----
  if( m_pcRecPicBuffer )
  {
    RNOK( m_pcRecPicBuffer->uninit  () );
    RNOK( m_pcRecPicBuffer->destroy () );
  }
  if( m_pcSPS )
  {
    RNOK( m_pcSPS->destroy() );
  }
  if ( m_pcSPSBase )
  {
    RNOK( m_pcSPSBase->destroy() );
  }
  if( m_pcPPS )
  {
    RNOK( m_pcPPS->destroy() );
  }
  if ( m_pcPPSBase )
  {
    RNOK ( m_pcPPSBase->destroy () );
  }
  if( m_pcSequenceStructure )
  {
    RNOK( m_pcSequenceStructure->uninit () );
    RNOK( m_pcSequenceStructure->destroy() );
  }
  if( m_pcInputPicBuffer )
  {
    RNOK( m_pcInputPicBuffer->uninit  () );
    RNOK( m_pcInputPicBuffer->destroy () );
  }

  m_pcRecPicBuffer      = NULL;
  m_pcSequenceStructure = NULL;
  m_pcInputPicBuffer    = NULL;
  m_pcSPS               = NULL;
  m_pcSPSBase           = NULL;
  m_pcPPS               = NULL;
  m_pcPPSBase           = NULL;
  m_bInitParameterSets  = false;
  m_bInit               = false;

  return Err::m_nOK;
}


ErrVal
PicEncoder::writeAndInitParameterSets( ExtBinDataAccessor* pcExtBinDataAccessor,
                                       Bool&               rbMoreSets,
									   UInt Num_Of_Views_Minus_1)
{
  if( ! m_pcSPSBase)
  {
    //===== create base SPS =====    
    RNOK( xInitSPS(true) );

    //===== write SPS =====
    UInt uiSPSBits = 0;
    RNOK( m_pcNalUnitEncoder->initNalUnit ( pcExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->write       ( *m_pcSPSBase ) );
    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiSPSBits ) );
    m_uiWrittenBytes += ( ( uiSPSBits >> 3 ) + 4 );
	
	m_adMVCSeqBits[0] += uiSPSBits; //SEI 

  }  
  //else if ( ! m_pcSPS  && Num_Of_Views_Minus_1>0 )	
  else if ( ! m_pcSPS )	  
  {
    //===== create SPS =====
    RNOK( xInitSPS(false) );

    //===== write SPS =====
    UInt uiSPSBits = 0;
    RNOK( m_pcNalUnitEncoder->initNalUnit ( pcExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->write       ( *m_pcSPS ) );
    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiSPSBits ) );
    m_uiWrittenBytes += ( ( uiSPSBits >> 3 ) + 4 );	  
    
	m_adMVCSeqBits[0] += uiSPSBits; //SEI 

  }
  else if ( !m_pcPPSBase)
  {
    RNOK( xInitPPS(true) );

    //===== write PPS =====
    UInt uiPPSBits = 0;
    RNOK( m_pcNalUnitEncoder->initNalUnit ( pcExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->write       ( *m_pcPPSBase ) );
    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiPPSBits ) );
    m_uiWrittenBytes += ( ( uiPPSBits >> 3 ) + 4 );

	m_adMVCSeqBits[0] += uiPPSBits; //SEI 
    
  }
  //else if( ! m_pcPPS && Num_Of_Views_Minus_1>0 )
  else if( ! m_pcPPS )
  {
    //===== create PPS =====
    RNOK( xInitPPS(false) );

    //===== write PPS =====
    UInt uiPPSBits = 0;
    RNOK( m_pcNalUnitEncoder->initNalUnit ( pcExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->write       ( *m_pcPPS ) );
    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiPPSBits ) );
    m_uiWrittenBytes += ( ( uiPPSBits >> 3 ) + 4 );

	m_adMVCSeqBits[0] += uiPPSBits; //SEI 

    //===== init pic encoder with parameter sets =====
    RNOK( xInitParameterSets() );
  }
  
  //===== set status =====
  rbMoreSets = ! m_bInitParameterSets;

  return Err::m_nOK;
} 
 
// ying {{ new configuration file
ErrVal
PicEncoder::xInitFrameSpecSpecial() // for 12 and 15 only
{
  UInt uiFrameNumInGOP=1;
  UInt auiPredListSize[2];
  
  if(m_uiGOPSize==12)
  {
    m_acFrameSpecification[12].init('I',uiFrameNumInGOP++,12,0);
    m_acFrameSpecification[ 6].init('B',uiFrameNumInGOP++, 6,1);
    m_acFrameSpecification[ 3].init('B',uiFrameNumInGOP++, 3,2);
    m_acFrameSpecification[ 9].init('B',uiFrameNumInGOP++, 9,2);
    m_acFrameSpecification[ 2].init('B',uiFrameNumInGOP++, 2,3);    
    m_acFrameSpecification[ 5].init('B',uiFrameNumInGOP++, 5,3);
    m_acFrameSpecification[ 8].init('B',uiFrameNumInGOP++, 8,3);
    m_acFrameSpecification[11].init('B',uiFrameNumInGOP++,11,3);
    m_acFrameSpecification[ 1].init('b',uiFrameNumInGOP  , 1,4);
    m_acFrameSpecification[ 4].init('b',uiFrameNumInGOP  , 4,4);
    m_acFrameSpecification[ 7].init('b',uiFrameNumInGOP  , 7,4);
    m_acFrameSpecification[10].init('b',uiFrameNumInGOP  ,10,4);

    m_acFrameSpecification[12].updateAnchor(m_pcCodingParameter->getIntraPeriod());
  }
  else if (m_uiGOPSize==15)
  {
    m_acFrameSpecification[15].init('I',uiFrameNumInGOP++,15,0);
    m_acFrameSpecification[ 8].init('B',uiFrameNumInGOP++, 8,1);
    m_acFrameSpecification[ 4].init('B',uiFrameNumInGOP++, 4,2);
    m_acFrameSpecification[12].init('B',uiFrameNumInGOP++,12,2);
    m_acFrameSpecification[ 2].init('B',uiFrameNumInGOP++, 2,3);
    m_acFrameSpecification[ 6].init('B',uiFrameNumInGOP++, 6,3);
    m_acFrameSpecification[10].init('B',uiFrameNumInGOP++,10,3);
    m_acFrameSpecification[14].init('B',uiFrameNumInGOP++,14,3);
    m_acFrameSpecification[ 1].init('b',uiFrameNumInGOP  , 1,4);
    m_acFrameSpecification[ 3].init('b',uiFrameNumInGOP  , 3,4);
    m_acFrameSpecification[ 5].init('b',uiFrameNumInGOP  , 5,4);
    m_acFrameSpecification[ 7].init('b',uiFrameNumInGOP  , 7,4);
    m_acFrameSpecification[ 9].init('b',uiFrameNumInGOP  , 9,4);
    m_acFrameSpecification[11].init('b',uiFrameNumInGOP  ,11,4);
    m_acFrameSpecification[13].init('b',uiFrameNumInGOP  ,13,4);

    m_acFrameSpecification[15].updateAnchor(m_pcCodingParameter->getIntraPeriod());
  }

  
  //put the code together
   for(UInt uiFrame = 1 ; uiFrame <= m_uiGOPSize; uiFrame++ )
   {
     xGetListSizesSpecial( m_acFrameSpecification[uiFrame].getTemporalLayer(), uiFrame, auiPredListSize);

     // PATCH BEGIN -Samsung
     // To allow usage of “IntraPeriod?parameter in case of “GOPSize?equal 12 or 15.
     if((uiFrame == m_uiGOPSize) && ((m_pcCodingParameter->getIntraPeriod() != m_uiGOPSize)))
         auiPredListSize[0] = 1;
     //  PATCH END

     m_acFrameSpecification[uiFrame].setNumRefIdxActive( LIST_0, auiPredListSize[0]);
     m_acFrameSpecification[uiFrame].setNumRefIdxActive( LIST_1, auiPredListSize[1]);
     SliceType     eSliceType      = ( auiPredListSize[1] ? B_SLICE : auiPredListSize[0] ? P_SLICE : I_SLICE );
     m_acFrameSpecification[uiFrame].setSliceType(eSliceType);
   }
   return Err::m_nOK;
}

ErrVal
PicEncoder::xInitFrameSpecHierarchical()
{
  UInt uiFrameNumInGOP=1;
  UInt uiTemporalLevel;
  UInt auiPredListSize[2];

  
  for( uiTemporalLevel = 0; uiTemporalLevel <= m_uiMaxTL; uiTemporalLevel++ )
  {
    UInt      uiStep    = ( 1 << ( m_uiMaxTL - uiTemporalLevel ) );
    for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGOPSize; uiFrameId += ( uiStep << 1 ) )
    {
      UChar        ucType;
      if(uiTemporalLevel==0)              ucType = 'I';
      else if(uiTemporalLevel==m_uiMaxTL) ucType = 'b';
      else                                ucType = 'B';
      m_acFrameSpecification[uiFrameId].init(ucType, uiFrameNumInGOP, uiFrameId, uiTemporalLevel );

      if (m_acFrameSpecification[uiFrameId].getNalRefIdc()) uiFrameNumInGOP++;

      if (uiTemporalLevel==0) m_acFrameSpecification[uiFrameId].updateAnchor(m_pcCodingParameter->getIntraPeriod());
      //put the code together
      xGetListSizes ( uiTemporalLevel, uiFrameId, auiPredListSize);
      m_acFrameSpecification[uiFrameId].setNumRefIdxActive( LIST_0, auiPredListSize[0]);
      m_acFrameSpecification[uiFrameId].setNumRefIdxActive( LIST_1, auiPredListSize[1]);
      SliceType     eSliceType      = ( auiPredListSize[1] ? B_SLICE : auiPredListSize[0] ? P_SLICE : I_SLICE );
      m_acFrameSpecification[uiFrameId].setSliceType(eSliceType);
    }
  }
    
  return Err::m_nOK;
}
ErrVal
PicEncoder::xInitFrameSpec()
{
  m_uiGOPSizeReal    = m_pcCodingParameter->getGOPSize();
  m_uiTotalFrames       = m_pcCodingParameter->getTotalFrames();
  m_uiMaxTL             = m_pcCodingParameter->getDecompositionStages();
  m_uiMaxNumRefFrames   = m_pcCodingParameter->getNumRefFrames();
  m_pcSPSMVCExt         = &m_pcCodingParameter->SpsMVC;
  m_uiFrameDelay        = (UInt) m_pcCodingParameter->getMaximumDelay();
  m_uiMaxFrameNum       = (1 << m_pcCodingParameter->getLog2MaxFrameNum());
  m_uiAnchorFrameNumber = 0;

  m_CurrentViewId       = m_pcCodingParameter->getCurentViewId();
//   m_ViewLevel           = m_pcCodingParameter->getViewLevel(); JVT-W035
  m_bAVCFlag            = m_pcCodingParameter->getAVCFlag();
  m_SvcMvcFlag          = (!m_pcCodingParameter->getMVCmode() );

  m_bInterPridPicsFirst = m_pcCodingParameter->getInterPredPicsFirst()!=0; // JVT-V043 encoder
  // with some assumption, can be modified later
  m_uiAnchorNumFwdViewRef     = m_pcSPSMVCExt->getNumAnchorRefsForListX(getViewId(), LIST_0);
  m_uiAnchorNumBwdViewRef     = m_pcSPSMVCExt->getNumAnchorRefsForListX(getViewId(), LIST_1);

  m_uiNonAncNumFwdViewRef     = m_pcSPSMVCExt->getNumNonAnchorRefsForListX(getViewId(), LIST_0);
  m_uiNonAncNumBwdViewRef     = m_pcSPSMVCExt->getNumNonAnchorRefsForListX(getViewId(), LIST_1);  
  
  m_vFramePeriod        = m_pcCodingParameter->getIntraPeriod();
  //m_bCompleteGOP        = m_uiTotalFrames< (m_uiGOPSize +1) ? false : true ;
  m_bSpecialGOP         = false;

  UInt                  uiFrameNumInGOP=0;
//  UInt uiTemporalLevel;
//  UInt auiPredListSize[2];

  
  if(m_uiTotalFrames< (m_uiGOPSizeReal +1) ) m_uiGOPSize = m_uiTotalFrames -1; 
  else m_uiGOPSize= m_uiGOPSizeReal;

  m_acFrameSpecification[uiFrameNumInGOP++].init('A', 0, 0, 0);
  // complete GOP 12 (15)
  if( (m_uiGOPSizeReal==12 || m_uiGOPSizeReal==15) && m_uiGOPSize == m_uiGOPSizeReal)
  {
    m_bSpecialGOP   = true;
    m_uiGOPSizeReal = m_uiGOPSize;
    RNOK(xInitFrameSpecSpecial());
  }
  else
  {
     RNOK(xInitFrameSpecHierarchical());
  }
 
  m_uiMinTempLevelLastGOP =0; // to save dpb a fix here March 20
  for(UInt uiFrame = 0 ; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    RNOK( xInitReordering( uiFrame ) );
  }
  
  m_cFrameSpecification.copy( m_acFrameSpecification[0] );
  m_uiProcessingPocInGOP =0;
  
  return Err::m_nOK;  
}
ErrVal
PicEncoder::xGetNextFrameSpecSpecial()
{

  return Err::m_nOK;
}
ErrVal
PicEncoder::xGetNextFrameSpec()
{
  if( m_uiProcessingPocInGOP ==0 ) 
// the key picture is coded
  {
// for GOPSize 12 and 15 when the GOP is complete
    if(m_bSpecialGOP && m_uiGOPSizeReal == m_uiGOPSize)

    {
       m_uiProcessingPocInGOP=m_uiGOPSizeReal;
       m_cFrameSpecification.copy( m_acFrameSpecification[m_uiProcessingPocInGOP] );  
       return Err::m_nOK;
    }

// for all other cases: GOPSize is 2^n or the uncompleted GOP
    for(UInt templevel=0; templevel<=m_uiMaxTL; templevel++)
    {
      UInt uiStartNumTL = 1<<(m_uiMaxTL-templevel);
      if( uiStartNumTL <=m_uiGOPSize)
      {
        m_uiProcessingPocInGOP = uiStartNumTL; 
        m_cFrameSpecification.copy( m_acFrameSpecification[m_uiProcessingPocInGOP] );  
        return Err::m_nOK;
      }
    }
  }
//at the end of a complete GOP for hierarchical B
  if( m_uiProcessingPocInGOP == (1<<m_uiMaxTL) -1 && m_uiProcessingPocInGOP!=m_uiGOPSize)
  {
    // others
    xUpdateFrameSepNextGOP();
    m_uiProcessingPocInGOP=0;
    return xGetNextFrameSpec();
  }

  if( m_uiGOPSize == 1)
  {
      // others
      xUpdateFrameSepNextGOP();
      m_uiProcessingPocInGOP=0;
      return xGetNextFrameSpec();
  }  

//at the end of a complete special GOP
  if(m_bSpecialGOP && m_acFrameSpecification [m_uiProcessingPocInGOP].getTemporalLayer()==m_uiMaxTL && m_uiProcessingPocInGOP + 2 >=m_uiGOPSizeReal ) 
  {
    // others
    xUpdateFrameSepNextGOP();
    m_uiProcessingPocInGOP=0;
    return xGetNextFrameSpec();
  }

// other cases
// for hierarchical B and the uncompleted GOP /////////////////////////
  if(!m_bSpecialGOP || m_uiGOPSizeReal > m_uiGOPSize )
  {
    UInt      uiStep    = ( 1 << ( m_uiMaxTL - m_acFrameSpecification[m_uiProcessingPocInGOP].getTemporalLayer() ) );
    UInt      uiNextFrameIdInGOP = (uiStep<<1)+m_uiProcessingPocInGOP;
    if (uiNextFrameIdInGOP>m_uiGOPSize || (uiNextFrameIdInGOP == (1 <<m_uiMaxTL) ))
    {
      m_uiProcessingPocInGOP = (uiStep >>1);
    }
    else 
    {
      m_uiProcessingPocInGOP = uiNextFrameIdInGOP;
    }
  }
// for hierarchical B and the uncompleted GOP /////////////////////////
  else  // for the completed special GOP
  {
    Bool bFindInSameTL=false;
    UInt uiCurrTL     =m_acFrameSpecification[m_uiProcessingPocInGOP].getTemporalLayer();
    for(UInt uiPocInGOP= m_uiProcessingPocInGOP+1; uiPocInGOP< m_uiGOPSize; uiPocInGOP++)
      if( m_acFrameSpecification[uiPocInGOP].getTemporalLayer() == uiCurrTL)
      {
        m_uiProcessingPocInGOP = uiPocInGOP;
        bFindInSameTL=true;
        break;
      }
    if(!bFindInSameTL)
      for(UInt i=0; i< m_uiGOPSize; i++)
        if( m_acFrameSpecification[i].getTemporalLayer() == uiCurrTL+1)
        {
          m_uiProcessingPocInGOP =i; break;
        }
  }

  m_cFrameSpecification.copy( m_acFrameSpecification[m_uiProcessingPocInGOP] );  
  return Err::m_nOK;
}

ErrVal
PicEncoder::xUpdateFrameSepNextGOPFinish()
{
  UInt uiFrameNumStart     = m_acFrameSpecification[1].getFrameNum();
  // bug fix
  // assume that the [1] frame is the highest temporal level frame and always has the 
  // lagest frame_num of the previous GOP. Or we can search the largets frame_num
  UInt uiContFrameNumStart = m_acFrameSpecification[m_uiGOPSize].getContFrameNumber();
  m_uiGOPSize=m_uiTotalFrames-m_uiAnchorFrameNumber-1;
  
  if ( m_uiGOPSize==0) return Err::m_nOK;

  UInt uiTemporalLevel;
  UInt auiPredListSize[2];
  
  m_uiMinTempLevelLastGOP = m_uiMaxTL; // to save dpb 

  for( uiTemporalLevel = 0; uiTemporalLevel <= m_uiMaxTL; uiTemporalLevel++ )
  {
    UInt      uiStep    = ( 1 << ( m_uiMaxTL - uiTemporalLevel ) );
    for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGOPSize; uiFrameId += ( uiStep << 1 ) )
    {
      UChar        ucType;
      if(uiTemporalLevel==0)              ucType = 'I';
      else if(uiTemporalLevel==m_uiMaxTL) ucType = 'b';
      else                                ucType = 'B';
      m_acFrameSpecification[uiFrameId].init(ucType, uiFrameNumStart, uiFrameId, uiTemporalLevel );
      m_acFrameSpecification[uiFrameId].updateNumbers(0, uiContFrameNumStart, m_uiMaxFrameNum);
      // update frame_num and framenumcon
      if (m_acFrameSpecification[uiFrameId].getNalRefIdc()) uiFrameNumStart++;

      if (uiTemporalLevel==0) m_acFrameSpecification[uiFrameId].updateAnchor(m_pcCodingParameter->getIntraPeriod());

      //put the code together  
      xGetListSizes ( uiTemporalLevel, uiFrameId, auiPredListSize);
      m_acFrameSpecification[uiFrameId].setNumRefIdxActive( LIST_0, auiPredListSize[0]);
      m_acFrameSpecification[uiFrameId].setNumRefIdxActive( LIST_1, auiPredListSize[1]);
      SliceType     eSliceType      = ( auiPredListSize[1] ? B_SLICE : auiPredListSize[0] ? P_SLICE : I_SLICE );
      m_acFrameSpecification[uiFrameId].setSliceType(eSliceType);
      if (uiTemporalLevel< m_uiMinTempLevelLastGOP ) m_uiMinTempLevelLastGOP = uiTemporalLevel; // to save dpb 
    }
  }
  
  for(UInt uiFrame = 1 ; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    RNOK( xInitReordering( uiFrame ) );
  }
  m_uiAnchorFrameNumber = m_uiTotalFrames -1;
  return Err::m_nOK;
}
ErrVal
PicEncoder::xUpdateFrameSepNextGOP()
{
  m_uiAnchorFrameNumber+=m_uiGOPSize;
  m_acFrameSpecification[0].copy(m_acFrameSpecification[m_uiGOPSize]);
  if( m_uiAnchorFrameNumber+1>=m_uiTotalFrames ) // bug fix 30 Dec 2006
//end coding
  {
    return Err::m_nOK;
  }
  if( m_uiAnchorFrameNumber+m_uiGOPSize+1 > m_uiTotalFrames)
//reach the uncomplete GOP
  {
    return xUpdateFrameSepNextGOPFinish();
  }

// usually periodic GOP
  UInt uiFrameNumGap = m_uiGOPSize>1 ? m_uiGOPSize/2: m_uiGOPSize ; // works only for hierarchical
// update for special GOP
  if(m_bSpecialGOP) uiFrameNumGap = 1 << (m_uiMaxTL-1);

  UInt  auiPredListSize[2];

  const UInt  uiMaxFrameNumber  = m_uiMaxFrameNum;
  
//  m_uiFrameNum + = uiFrameNumGap; 
//  if ( m_uiFrameNum > 
  for(UInt i=1; i<=m_uiGOPSize; i++)
    m_acFrameSpecification[i].updateNumbers( uiFrameNumGap, m_uiGOPSize, uiMaxFrameNumber );
  
  m_acFrameSpecification[m_uiGOPSize].updateAnchor(m_pcCodingParameter->getIntraPeriod());
  //put the code together 
  if (!m_bSpecialGOP )
    xGetListSizes ( 0, m_uiGOPSize, auiPredListSize);
  else 
  {
    xGetListSizesSpecial( 0, m_uiGOPSize, auiPredListSize);
    // PATCH Samsung
    // To allow usage of .IntraPeriod. parameter in case of .GOPSize. equal 12 or 15. 
    if((m_pcCodingParameter->getIntraPeriod() != m_uiGOPSize))
      auiPredListSize[0] = 1;
  }

  m_acFrameSpecification[m_uiGOPSize].setNumRefIdxActive( LIST_0, auiPredListSize[0]);
  m_acFrameSpecification[m_uiGOPSize].setNumRefIdxActive( LIST_1, auiPredListSize[1]);
  
  for(UInt uiFrame = 1 ; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    RNOK( xInitReordering( uiFrame ) );
  }

  return Err::m_nOK;
}

ErrVal
PicEncoder::process( PicBuffer*               pcInputPicBuffer,
                    PicBufferList&           rcOutputList,
                    PicBufferList&           rcUnusedList,
                    ExtBinDataAccessorList&  rcExtBinDataAccessorList )
{
    ROF( m_bInitParameterSets );

    //===== add picture to input picture buffer =====
    RNOK( m_pcInputPicBuffer->add( pcInputPicBuffer ) );

    //===== encode following access units that are stored in input picture buffer =====
    while( true )
    {
        InputAccessUnit* pcInputAccessUnit = NULL;

        //----- get next frame specification and input access unit -----
        if( ! m_pcInputPicBuffer->empty() )
        {

            pcInputAccessUnit       = m_pcInputPicBuffer->remove( m_cFrameSpecification.getContFrameNumber() );
        }
        if( ! pcInputAccessUnit )
        {
            break;
        }
        //----- initialize picture -----
        Double          dLambda         = 0;
        UInt            uiPictureBits   = 0;
        SliceHeader*    pcSliceHeader   = 0;
		SliceHeader*    pcSliceHeaderTemp = 0;
        RecPicBufUnit*  pcRecPicBufUnit = 0;//rec pic for current coded frame/field
        PicBuffer*      pcOrigPicBuffer = pcInputAccessUnit->getInputPicBuffer();//input frame

		const Bool bFieldCoded = ( m_pcCodingParameter->getPAff()==1? true : 
			(m_pcCodingParameter->getPAff()==2? (( rand()%2 ) ? true : false) : false));
		
		RNOK( xInitSliceHeader( pcSliceHeaderTemp, m_cFrameSpecification, dLambda, false, bFieldCoded?TOP_FIELD:FRAME  ) );

		if (   (NAL_UNIT_CODED_SLICE_IDR != m_cFrameSpecification.getNalUnitType() || !this->getAVCFlag())&&
            (m_MultiviewRefPicManager.CountNumMultiviewReferenceStreams() > 0) ) {
                Double          dLambdaForMVC   = 0;
                SliceHeader*    pcSliceHeaderForMVC   = 0;
                //RNOK( xInitSliceHeader( pcSliceHeaderForMVC, m_cFrameSpecification, 
                //    dLambdaForMVC, /* fakeHeader = */ true ) );//get frame poc
                //int poc = pcSliceHeaderForMVC->getPoc();
 
                UInt poc=m_cFrameSpecification.getContFrameNumber();

                RNOK( xInitSliceHeader( pcSliceHeaderForMVC, m_cFrameSpecification, 
                    dLambdaForMVC, /* fakeHeader = */ true ,bFieldCoded?TOP_FIELD:FRAME ) );//init frame/field SH


                m_MultiviewRefPicManager.AddMultiviewReferencesPicturesToBuffer
                    (m_pcRecPicBuffer, pcSliceHeaderForMVC, rcOutputList, 
                    rcUnusedList, *m_pcSPS, poc , this->TimeForVFrameP(poc));
                delete pcSliceHeaderForMVC;
        }

		RNOK( m_pcRecPicBuffer->initCurrRecPicBufUnit( pcRecPicBufUnit, pcOrigPicBuffer, pcSliceHeaderTemp, rcOutputList, rcUnusedList ) );//init pcRecPicBufUnit
        pcRecPicBufUnit->getRecFrame()->clearPicStat();//frame not encoded

        const Bool m_bBotFieldFirst=false;
        UInt    uiNumPics     = ( bFieldCoded ? 2 : 1 );
        PicType eFirstPicType = ( bFieldCoded ? ( m_bBotFieldFirst ? BOT_FIELD : TOP_FIELD ) : FRAME );
        PicType eLastPicType  = ( bFieldCoded ? ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) : FRAME );
		UInt    uiFirstPicPoc = 0;

        //----- encoding -----
        for( UInt uiPicType = 0; uiPicType < uiNumPics; uiPicType++ )
        {
            PicType ePicType    = ( uiPicType ? eLastPicType : eFirstPicType );

            //----- initialize picture -----
            dLambda         = 0;
            uiPictureBits   = 0;

            RNOK( xInitSliceHeader( pcSliceHeader, m_cFrameSpecification, dLambda, false, ePicType ) );
			if(uiPicType == 1)pcSliceHeader->setPoc(uiFirstPicPoc,eFirstPicType);//set first field poc 
            pcRecPicBufUnit->getRecFrame()->setPoc( *pcSliceHeader );//update poc for frame/field
            
            // JVT-V043 encoder
            m_pcRecPicBuffer->SetPictureEncoder(this->getpicEncoder()); //JVT-W056
            RNOK( xInitReorderingInterView ( pcSliceHeader) );

            //JVT-W080
            if( getPdsEnable() && m_cFrameSpecification.isAnchor() )
            {
                setPdsInitialDelayMinus2L0( m_pcCodingParameter->getPdsInitialDelayMinus2L0Anc() );
                setPdsInitialDelayMinus2L1( m_pcCodingParameter->getPdsInitialDelayMinus2L1Anc() );
            }
            else if( getPdsEnable() )
            {
                setPdsInitialDelayMinus2L0( m_pcCodingParameter->getPdsInitialDelayMinus2L0NonAnc() );
                setPdsInitialDelayMinus2L1( m_pcCodingParameter->getPdsInitialDelayMinus2L1NonAnc() );	
            }
            //~JVT-W080    

			//----- encoding -----
            RNOK( xEncodePicture( rcExtBinDataAccessorList, *pcRecPicBufUnit, *pcSliceHeader, dLambda, uiPictureBits ) );
            m_uiWrittenBytes += ( uiPictureBits >> 3 );

            //SEI {
            m_adMVCSeqBits[pcSliceHeader->getTemporalLevel()] += uiPictureBits;
            m_auiMVCNumFramesCoded[pcSliceHeader->getTemporalLevel()] ++;
            xModifyMaxBitrate( uiPictureBits );
            //SEI }

			if(uiPicType == 0)uiFirstPicPoc=pcSliceHeader->getPoc(eFirstPicType);
			
            if (uiPicType == 0 && bFieldCoded)   // Dong. Bug fix for sliding window with interlace mode
                m_pcRecPicBuffer->store2(pcRecPicBufUnit->isNeededForRef());

            //----- reset -----
            delete pcSliceHeader;
			pcSliceHeader=0;
		}//end of paff test

		 m_MultiviewRefPicManager.RemoveMultiviewReferencesPicturesFromBuffer(m_pcRecPicBuffer);

         //----- store picture -----
         RNOK( m_pcRecPicBuffer->store( pcRecPicBufUnit, pcSliceHeaderTemp, rcOutputList, rcUnusedList ) );
		 delete pcSliceHeaderTemp;
		//----- reset -----
        delete pcInputAccessUnit;
        xGetNextFrameSpec(); //new configuration file

    }

    return Err::m_nOK;
}

ErrVal
PicEncoder::finish( PicBufferList&  rcOutputList,
                    PicBufferList&  rcUnusedList )
{
  RNOK( m_pcRecPicBuffer->clear( rcOutputList, rcUnusedList ) );

  //===== output summary =====
  printf("\n %5d frames encoded:  Y %7.4lf dB  U %7.4lf dB  V %7.4lf dB\n"
       "\n     average bit rate:  %.4lf kbit/s  [%d byte for %.3lf sec]\n\n",
    m_uiCodedFrames,
    m_dSumYPSNR / (Double)m_uiCodedFrames,
    m_dSumUPSNR / (Double)m_uiCodedFrames,
    m_dSumVPSNR / (Double)m_uiCodedFrames,
    0.008*(Double)m_uiWrittenBytes/(Double)m_uiCodedFrames*m_pcCodingParameter->getMaximumFrameRate(),
    m_uiWrittenBytes,
    (Double)m_uiCodedFrames/m_pcCodingParameter->getMaximumFrameRate() 
    );
//SEI {
  UInt ui;
  for( ui = 1; ui <= MAX_DSTAGES_MVC; ui++ )
  {
    m_adMVCSeqBits[ui] += m_adMVCSeqBits[ui-1];
    m_auiMVCNumFramesCoded[ui] += m_auiMVCNumFramesCoded[ui-1];
  }
  for( ui = 0; ui < MAX_DSTAGES_MVC; ui++ )
  {
    Double  dFps    = (m_pcCodingParameter->getMaximumFrameRate()) *  m_auiMVCNumFramesCoded[ui] / m_uiCodedFrames;
    Double  dScale  = dFps / 1000 / (Double)m_auiMVCNumFramesCoded[ui];
    m_dMVCFinalBitrate[ui] = m_adMVCSeqBits[ui] * dScale;
	m_dMVCFinalFramerate[ui] = dFps;
  }
//SEI }
  return Err::m_nOK;
}

//SEI {
ErrVal
PicEncoder::xModifyMaxBitrate( UInt uiBits )
{
  m_uiNumPics++;
  if(m_uiNumPics > (UInt)m_pcCodingParameter->getMaximumFrameRate())
  {
    m_uiNumPics--;
	for( UInt i = 0; i < m_uiNumPics - 1; i++ )
		m_adMVCPicBits[i] = m_adMVCPicBits[i+1];
	m_adMVCPicBits[m_uiNumPics-1] = uiBits;
  }
  else
  	m_adMVCPicBits[m_uiNumPics-1] = uiBits;
  Double TotalBits = 0;
  for( UInt i = 0; i < m_uiNumPics; i++ )
	  TotalBits += m_adMVCPicBits[i];
  TotalBits = TotalBits * 0.001 * (m_pcCodingParameter->getMaximumFrameRate()) / m_uiNumPics;
  if( TotalBits > m_adMaxBitRate )
	  m_adMaxBitRate = TotalBits;
  return Err::m_nOK;
}
//SEI }

//  AVC base and multiview SPSs and PPSs {{
ErrVal
PicEncoder::xInitSPS( Bool bAVCSPS )
{
  ROF( m_bInit );

  UInt  uiLevelIdc;	
  UInt uiDPBSize;
  UInt mvcScaleFactor=1;
  UInt NumViews=1;
  UInt              uiCropLeft          = 0;
  UInt              uiCropTop           = 0;
  UInt              uiCropRight         = m_pcCodingParameter->getHorPadding() / 2;                                            // chroma_format_idc is always equal to 1
  UInt              uiCropBottom        = m_pcCodingParameter->getVerPadding() / ( m_pcCodingParameter->isInterlaced() ? 4 : 2 ); // chroma_format_idc is always equal to 1

  if (bAVCSPS)
	  printf("\nInitilizating parameters for NAL_UNIT_SPS...\n");
  else
	  printf("\nInitilizating parameters for NAL_UNIT_SUBSET_SPS...\n");

  SequenceParameterSet*&  rpcSPS = bAVCSPS ?  m_pcSPSBase: m_pcSPS; 
  ROT( rpcSPS );
  //===== determine parameters =====
  UInt  uiSPSId     = bAVCSPS ? 0 : 1 ;
  UInt  uiMbX       = (m_pcCodingParameter->getFrameWidth () + 15) >> 4;
  UInt uiMbY		= m_pcCodingParameter->isInterlaced() ? ( ( m_pcCodingParameter->getFrameHeight() + 31 ) >> 5 ) << 1 : ( m_pcCodingParameter->getFrameHeight() + 15 ) >> 4 ; 
  //UInt  uiMbY       = m_pcCodingParameter->getFrameHeight() >> 4;
  UInt  uiOutFreq   = (UInt)ceil( m_pcCodingParameter->getMaximumFrameRate() );
  UInt  uiMvRange   = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange()*4;

 
  //===== create parameter sets =====
  RNOK( SequenceParameterSet::create( rpcSPS ) );
 if( !bAVCSPS)  
  {
    m_pcSPS->SpsMVC = new SpsMvcExtension; 
    memcpy( m_pcSPS->SpsMVC, &(m_pcCodingParameter->SpsMVC), sizeof ( SpsMvcExtension )) ;
	NumViews = rpcSPS->SpsMVC->getNumViewMinus1()+1;
  }
  else //  AVC base SPS
     m_pcSPSBase->SpsMVC = NULL;
  
	
  if (!bAVCSPS)
	  mvcScaleFactor=2;
 
  //uiDPBSize           = max(2, 1 << m_pcCodingParameter->getDecompositionStages());//BUG_FIX @20090218
  uiDPBSize = 4+ max (2, (1 <<(m_pcCodingParameter->getDecompositionStages()-1)) + m_pcCodingParameter->getDecompositionStages());
  if (!bAVCSPS)
	  uiDPBSize += 4;	 // up-to extra 4 interview pictures  
  m_pcCodingParameter->setDPBSize      ( uiDPBSize ); // set the encoder DPB size

  
  UInt decDPBSize = 1<<(int)(ceil((float)log((float)m_pcCodingParameter->getGOPSize())/(float)log(2.0))-1);
  decDPBSize = (decDPBSize+1)*NumViews; // time-first & hierarchical-B decoding
  if (NumViews>1)
	  decDPBSize +=2;// time-first & hierarchical-B decoding
  uiLevelIdc  = SequenceParameterSet::getLevelIdc( uiMbY, uiMbX, uiOutFreq, uiMvRange, decDPBSize, NumViews ) ; 
  if (uiLevelIdc== MSYS_UINT_MAX ) {
	  printf("Warning: With hierarchical-B & time-first decoding, the current configuration for full-view decoding requires %d DPB-frames.",decDPBSize);
	  printf("However, it exceeds the limit of MaxDpbFrames as specified in H.10.2.\n");
	  if (m_pcCodingParameter->getDPBConformanceCheck())
	  {
		  printf("Encoder terminates.\n");
	  	  exit(1);	
	  } else
	  {
		  printf("Assigning an arbitrary level_idc = 51\n\n");
		  uiLevelIdc = 51;
	  }

  } 

  //===== set SPS parameters =====
  rpcSPS->setNalUnitType                           ( bAVCSPS ?  NAL_UNIT_SPS : NAL_UNIT_SUBSET_SPS );
  rpcSPS->setLayerId                               ( 0 );

  rpcSPS->setProfileIdc                            ( bAVCSPS ? ( m_pcCodingParameter->get8x8Mode() >0  ? HIGH_PROFILE : MAIN_PROFILE  ) :
	  ((m_pcCodingParameter->getMbAff() != 0 || m_pcCodingParameter->getPAff() != 0 )? STEREO_HIGH_PROFILE : MVC_PROFILE ));//lufeng

  rpcSPS->setConstrainedSet0Flag                   ( false );
  rpcSPS->setConstrainedSet1Flag                   ( false );
  rpcSPS->setConstrainedSet2Flag                   ( false );
  rpcSPS->setConstrainedSet3Flag                   ( false );
  if (rpcSPS->getProfileIdc() == HIGH_PROFILE || rpcSPS->getProfileIdc() == MAIN_PROFILE)
	rpcSPS->setConstrainedSet4Flag                   ( true );
  else
	rpcSPS->setConstrainedSet4Flag                   ( false );
  if (rpcSPS->getProfileIdc() == MULTI_VIEW_PROFILE && NumViews<3)
	rpcSPS->setConstrainedSet5Flag                   ( true );
  else
	rpcSPS->setConstrainedSet5Flag                   ( false );
  rpcSPS->setLevelIdc                              ( uiLevelIdc );
  rpcSPS->setSeqParameterSetId                     ( uiSPSId );
  rpcSPS->setSeqScalingMatrixPresentFlag           ( m_pcCodingParameter->get8x8Mode() > 1 );
  rpcSPS->setLog2MaxFrameNum                       ( m_pcCodingParameter->getLog2MaxFrameNum() );
  ROT(m_pcCodingParameter->getPicOrderCntType()==2 && m_pcCodingParameter->getGOPSize()>1);//Poc type 2 supported when GOPSize =1
  rpcSPS->setPicOrderCntType                       ( m_pcCodingParameter->getPicOrderCntType() );
  rpcSPS->setLog2MaxPicOrderCntLsb                 ( m_pcCodingParameter->getLog2MaxPocLsb() );
  //rpcSPS->setNumRefFrames                          ( min (16, m_pcCodingParameter->getDPBSize()/mvcScaleFactor) ); 
  
  rpcSPS->setRequiredFrameNumUpdateBehaviourFlag   ( true );
  rpcSPS->setFrameWidthInMbs                       ( uiMbX );
  rpcSPS->setFrameHeightInMbs                      ( uiMbY );
  rpcSPS->setDirect8x8InferenceFlag                ( true );
  rpcSPS->setCropOffset(uiCropLeft, uiCropRight, uiCropTop, uiCropBottom);

  rpcSPS->setMbAdaptiveFrameFieldFlag( (m_pcCodingParameter->getMbAff()?true:false) ); //th test
  if( rpcSPS->getMbAdaptiveFrameFieldFlag() && uiMbY % 2)
  {
      printf(" mbaff ignored ");
      rpcSPS->setMbAdaptiveFrameFieldFlag( false ); //not allowed
  }
    rpcSPS->setFrameMbsOnlyFlag( ! (m_pcCodingParameter->getMbAff() != 0 || m_pcCodingParameter->getPAff() != 0 ));

  rpcSPS->setCurrentViewId(m_pcCodingParameter->getCurentViewId());
  
  UInt uiMaxFramesInDPB = rpcSPS->getMaxDPBSize(mvcScaleFactor);
 
#if REDUCE_MAX_FRM_DPB 
  //uiMaxFramesInDPB = min ( mvcScaleFactor*uiMaxFramesInDPB , (max(1,(UInt)ceil((double)log((double)NumViews)/log(2.)))*16) );
  uiMaxFramesInDPB = min ( uiMaxFramesInDPB , (max(1,(UInt)ceil((double)log((double)NumViews)/log(2.)))*16) );
#endif

  if( (m_pcCodingParameter->getMbAff() != 0 || m_pcCodingParameter->getPAff() != 0 ))

	  rpcSPS->setNumRefFrames ( min (16, uiMaxFramesInDPB/mvcScaleFactor) -  m_uiGOPSize/2); //Need to consider non-ref frames cslim 130909
  else
	  rpcSPS->setNumRefFrames ( min (16, uiMaxFramesInDPB/mvcScaleFactor) ); 
  printf("bAVCSPS=%d NumViews=%d Max_NumRefFrames=%d encDPBsize=%d decDPBSize=%d LevelIdc=%d\n",bAVCSPS,NumViews, rpcSPS->getNumRefFrames(), uiDPBSize, uiMaxFramesInDPB, uiLevelIdc);


    

  return Err::m_nOK;
}


ErrVal
PicEncoder::xInitPPS( Bool bAVCSPS )
{
  ROF( m_bInit );
//  ROF( m_pcSPS );
  PictureParameterSet *&  rpcPPS = bAVCSPS ?  m_pcPPSBase: m_pcPPS; 
  SequenceParameterSet*&  rpcSPS = bAVCSPS ?  m_pcSPSBase: m_pcSPS; 
  ROF( rpcSPS );
  ROT( rpcPPS );

  //===== determine parameters =====
  UInt  uiPPSId     = bAVCSPS ? 0 : 1 ;
  
  //===== create PPS =====
  RNOK( PictureParameterSet ::create( rpcPPS ) );

  //===== set PPS parameters =====
  rpcPPS->setNalUnitType                           ( NAL_UNIT_PPS );
  rpcPPS->setPicParameterSetId                     ( uiPPSId );
  rpcPPS->setSeqParameterSetId                     ( rpcSPS->getSeqParameterSetId() );
  rpcPPS->setEntropyCodingModeFlag                 ( m_pcCodingParameter->getSymbolMode() != 0 );
  rpcPPS->setPicOrderPresentFlag                   ( true );
  rpcPPS->setNumRefIdxActive                       ( LIST_0, m_pcCodingParameter->getNumRefFrames() );
  rpcPPS->setNumRefIdxActive                       ( LIST_1, m_pcCodingParameter->getNumRefFrames() );
  rpcPPS->setPicInitQp                             ( min( 51, max( 0, (Int)m_pcCodingParameter->getBasisQp() ) ) );
  rpcPPS->setChomaQpIndexOffset                    ( 0 );
  rpcPPS->setDeblockingFilterParametersPresentFlag ( ! m_pcCodingParameter->getLoopFilterParams().isDefault() );
  rpcPPS->setConstrainedIntraPredFlag              ( false );
  rpcPPS->setRedundantPicCntPresentFlag            ( false );  //JVT-Q054 Red. Picture
  rpcPPS->setTransform8x8ModeFlag                  ( m_pcCodingParameter->get8x8Mode() > 0 );
  rpcPPS->setPicScalingMatrixPresentFlag           ( false );
  rpcPPS->set2ndChromaQpIndexOffset                ( 0 );
  rpcPPS->setNumSliceGroupsMinus1                  ( 0 );

  //===== prediction weights =====
//  m_pcPPS->setWeightedPredFlag                      ( WEIGHTED_PRED_FLAG );
//  m_pcPPS->setWeightedBiPredIdc                     ( WEIGHTED_BIPRED_IDC );

//TMM_WP
    rpcPPS->setWeightedPredFlag                   (m_pcCodingParameter->getIPMode()!=0);
    rpcPPS->setWeightedBiPredIdc                  (m_pcCodingParameter->getBMode());  
//TMM_WP

  rpcPPS->setCurrentViewId(m_pcCodingParameter->getCurentViewId());

  return Err::m_nOK;
}
//  AVC base and multiview SPSs and PPSs }}

ErrVal
PicEncoder::xInitParameterSets()
{
  //===== init control manager =====
  RNOK( m_pcControlMng->initParameterSets( *m_pcSPS, *m_pcPPS, *m_pcPPS ) );

  //===== set fixed parameters =====
  m_uiFrameWidthInMb      = m_pcSPS->getFrameWidthInMbs  ();
  m_uiFrameHeightInMb     = m_pcSPS->getFrameHeightInMbs ();
  m_uiMbNumber            = m_uiFrameWidthInMb * m_uiFrameHeightInMb;

  //===== re-allocate dynamic memory =====
  RNOK( xDeleteData() );
  RNOK( xCreateData() );

  //===== init objects =====
  RNOK( m_pcRecPicBuffer      ->initSPS ( *m_pcSPS ) );
  //==== initialize variable parameters =====
  m_uiFrameNum            = 0;
  m_uiIdrPicId            = 0;
  m_bInitParameterSets    = true;
  
  return Err::m_nOK;
}


ErrVal
PicEncoder::xCreateData()
{
  //===== write buffer =====
  UInt  uiNum4x4Blocks        = m_uiFrameWidthInMb * m_uiFrameHeightInMb * 4 * 4;
  m_uiWriteBufferSize         = 3 * ( uiNum4x4Blocks * 4 * 4 );
  ROFS( ( m_pucWriteBuffer   = new UChar [ m_uiWriteBufferSize ] ) );

  return Err::m_nOK;
}

ErrVal
PicEncoder::xDeleteData()
{

  //===== write buffer =====
  delete [] m_pucWriteBuffer;
  m_pucWriteBuffer    = 0;
  m_uiWriteBufferSize = 0;

  return Err::m_nOK;
}
// rplr and mmco:  {{
ErrVal  
PicEncoder::xSetRplr(  RplrBuffer&  rcRplrBuffer,
                       UIntList     cFrameNumList,
                       UInt         uiCurrFrameNr )
{
  rcRplrBuffer.clear();
  
  if( cFrameNumList.empty() )
  {
    rcRplrBuffer.setRefPicListReorderingFlag( false );
    return Err::m_nOK;
  }

  UIntList::iterator  iter            = cFrameNumList.begin();
  UInt                uiCurrReorderNr = uiCurrFrameNr;
  UInt                uiCount         = 0;
  Int                 iSum            = 0;
  Bool                bNeg            = false;
  Bool                bPos            = false;
  
  for( ; iter != cFrameNumList.end(); iter++)
  {
    Int  iDiff = *iter - uiCurrReorderNr;
    AOF( iDiff );

    if( iDiff < 0 )
    {
      Int iVal  = -iDiff - 1;
      rcRplrBuffer.set( uiCount++, Rplr( RPLR_NEG, iVal) );
      bNeg      = true;
      iSum     += iVal;
    }
    else
    {
      Int iVal  =  iDiff - 1;
      rcRplrBuffer.set( uiCount++, Rplr( RPLR_POS, iVal) );
      bPos      = true;
      iSum     += iVal;
    }
    uiCurrReorderNr = *iter;
  }
  rcRplrBuffer.set( uiCount++, Rplr( RPLR_END ) );
  rcRplrBuffer.setRefPicListReorderingFlag( true );

//force temporal RPLR for MVC slices only
  if( iSum == 0 && ( bPos == ! bNeg ) && m_bAVCFlag ) // purvin 
    //&& !m_bForceReOrderingCommands )   
  {
    rcRplrBuffer.clear();
    rcRplrBuffer.setRefPicListReorderingFlag( false );
  }

  return Err::m_nOK;
}

ErrVal  
PicEncoder::xGetFrameNumList( FrameSpec& rcFrameSpec, UIntList& rcFrameNumList, ListIdx eLstIdx, UInt uiCurrBasePos )
{
  rcFrameNumList.clear();

  const UInt uiLevel   = rcFrameSpec.getTemporalLayer();
  const UInt uiMaxSize = rcFrameSpec.getNumRefIdxActive( eLstIdx );
  ROF( uiMaxSize );

// for list1;
  if( eLstIdx == LIST_1 )
  {
    for( Int i = (Int)uiCurrBasePos+1; i <= (Int)m_uiGOPSize; i++ )
    {
      if( m_acFrameSpecification[i].getTemporalLayer()< uiLevel && m_acFrameSpecification[i].getNalRefIdc()>0)
      {
        rcFrameNumList.push_back( m_acFrameSpecification[i].getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }

	 if(rcFrameNumList.size() < uiMaxSize) // maybe not required 
	 {
		for( Int i = uiCurrBasePos-1; i >= 0; i-- )
		{
      if( m_acFrameSpecification[i].getTemporalLayer() < uiLevel && m_acFrameSpecification[i].getNalRefIdc()>0)
			{
        rcFrameNumList.push_back( m_acFrameSpecification[i].getFrameNum());
				if( rcFrameNumList.size() == uiMaxSize )
				{
					break;
				}
			}
		}
	 }
  }
  else // for list0 
  {
    for( Int i = uiCurrBasePos-1; i >= 0; i-- )
    {
      if( m_acFrameSpecification[i].getTemporalLayer()< uiLevel && m_acFrameSpecification[i].getNalRefIdc()>0)
      {
        rcFrameNumList.push_back( m_acFrameSpecification[i].getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
	 if(rcFrameNumList.size() < uiMaxSize)   // maybe not required 
	 {
		for( Int i = (Int)uiCurrBasePos+1; i <= (Int)m_uiGOPSize; i++ ) 
		{
      if( m_acFrameSpecification[i].getTemporalLayer() < uiLevel && m_acFrameSpecification[i].getNalRefIdc()>0)
			{
				rcFrameNumList.push_back( m_acFrameSpecification[i].getFrameNum() );
				if( rcFrameNumList.size() == uiMaxSize )
				{
					break;
				}
			}
		}
	 }
  }

  ROF( rcFrameNumList.size() == uiMaxSize );

  return Err::m_nOK;
}

ErrVal  
PicEncoder::xSetRplrAndMmco( FrameSpec& rcFrameSpec )
{
  // clear L1 rplr buffer
  rcFrameSpec.getRplrBuffer(LIST_1)->setRefPicListReorderingFlag( false );
  rcFrameSpec.getRplrBuffer(LIST_1)->clear();

  // clear mmco buffer
  rcFrameSpec.getMmcoBuffer()->clear();
  rcFrameSpec.setAdaptiveRefPicBufferingFlag( false );

  UInt uiCurrFrameNr = rcFrameSpec.getFrameNum(); // should be mode by MaxFrameNum
                                                  // do it later

  const UInt  uiMaxFrameNumber  = m_uiMaxFrameNum;
  // leave if idr
  if( rcFrameSpec.isIdrNalUnit())
  {
    m_cLPFrameNumList.clear();
    m_cLPFrameNumList.push_front( uiCurrFrameNr );
    return Err::m_nOK;
  }

  // generate rplr commands
  AOT( m_cLPFrameNumList.size() < rcFrameSpec.getNumRefIdxActive( LIST_0 ) );
  UIntList            cTempList;
  UIntList::iterator  iter = m_cLPFrameNumList.begin();
  for( UInt n = 0; n < rcFrameSpec.getNumRefIdxActive( LIST_0 ); n++ )
  {
    cTempList.push_back( *iter++ );
  }
  xSetRplr( *rcFrameSpec.getRplrBuffer(LIST_0), cTempList, uiCurrFrameNr );

  // calculate number of mmco commands
  
  const Int   iDiffA             = m_cLPFrameNumList.front() - uiCurrFrameNr;
  UInt        uiDiffA            = ( uiMaxFrameNumber - iDiffA ) % uiMaxFrameNumber;

  // generate mmco commands for inter b frames
  UInt uiPos = 0;
  while( --uiDiffA )
  {
    rcFrameSpec.getMmcoBuffer()->set( uiPos++, Mmco( MMCO_SHORT_TERM_UNUSED, uiDiffA-1 ) );
  }

  // generate mmco command for high-pass frame
  UInt uiNeedLowPassBefore = max( 1, rcFrameSpec.getNumRefIdxActive( LIST_0 ) );
  if( m_cLPFrameNumList.size() > uiNeedLowPassBefore )
  {
    const Int iDiffB   = m_cLPFrameNumList.popBack() - uiCurrFrameNr;
    UInt      uiDiffB  = ( uiMaxFrameNumber - iDiffB ) % uiMaxFrameNumber;
    rcFrameSpec.getMmcoBuffer()->set( uiPos++, Mmco( MMCO_SHORT_TERM_UNUSED, uiDiffB-1 ) );
  }

  // end of command list
  if ( uiPos )
  {
	rcFrameSpec.getMmcoBuffer()->set( uiPos, Mmco( MMCO_END) );

	rcFrameSpec.setAdaptiveRefPicBufferingFlag( true );
  }
  else
	rcFrameSpec.setAdaptiveRefPicBufferingFlag( false );

  // insert frame_num
  m_cLPFrameNumList.push_front( uiCurrFrameNr );

  return Err::m_nOK;
}

ErrVal  
PicEncoder::xInitReordering ( UInt uiFrameIdInGOP )
{

  FrameSpec * pcFrameSpec=&m_acFrameSpecification[uiFrameIdInGOP];
// JVT-V043
  
  //===== set RPLR and MMCO =====
  if( pcFrameSpec->getTemporalLayer() == m_uiMinTempLevelLastGOP && pcFrameSpec->getNalRefIdc()>0 ) // change 0 to m_uiMinTempLevelLastGOP
  {
    //===== low-pass frames =====
    RNOK( xSetRplrAndMmco( *pcFrameSpec ) );

    m_uiMinTempLevelLastGOP = 0; // change back 
  }
  else 
  {
    UIntList cFrameNumList;
    pcFrameSpec->getMmcoBuffer()->clear();
    pcFrameSpec->setAdaptiveRefPicBufferingFlag( false );

    if( pcFrameSpec->getSliceType() == B_SLICE )
    {
      // RefPicList0
      if ( m_pcSPSMVCExt->getNumRefsForListX(m_CurrentViewId, LIST_0, pcFrameSpec->isAnchor() ) > 0 )
      {
        RNOK( xGetFrameNumList( *pcFrameSpec, cFrameNumList, LIST_0, uiFrameIdInGOP ) );
        RNOK( xSetRplr        ( *pcFrameSpec->getRplrBuffer(LIST_0), cFrameNumList, pcFrameSpec->getFrameNum()) );
      }
      else
      {
        pcFrameSpec->getRplrBuffer( LIST_0 )->clear();
        pcFrameSpec->getRplrBuffer( LIST_0 )->setRefPicListReorderingFlag( false );
      }
      // RefPicList1
      if ( m_pcSPSMVCExt->getNumRefsForListX(m_CurrentViewId, LIST_1, pcFrameSpec->isAnchor()) > 0 )
      {
        RNOK( xGetFrameNumList( *pcFrameSpec, cFrameNumList, LIST_1, uiFrameIdInGOP ) );
        RNOK( xSetRplr        ( *pcFrameSpec->getRplrBuffer(LIST_1), cFrameNumList, pcFrameSpec->getFrameNum() ) );
      }
      else
      {
        pcFrameSpec->getRplrBuffer( LIST_1 )->clear();
        pcFrameSpec->getRplrBuffer( LIST_1 )->setRefPicListReorderingFlag( false );
      }
    }
    else if ( pcFrameSpec->getSliceType() == P_SLICE )
    {
//      ROF( pcFrameSpec->getSliceType() == P_SLICE );
// bug fix for num_ref_active_minus1 equal to 0. 

      RNOK( xGetFrameNumList( *pcFrameSpec,                       cFrameNumList, LIST_0, uiFrameIdInGOP ) );
      RNOK( xSetRplr        ( *pcFrameSpec->getRplrBuffer(LIST_0), cFrameNumList, pcFrameSpec->getFrameNum() ) );

      pcFrameSpec->getRplrBuffer( LIST_1 )->clear();
      pcFrameSpec->getRplrBuffer( LIST_1 )->setRefPicListReorderingFlag( false );
    }
    else
    {
      pcFrameSpec->getRplrBuffer( LIST_0 )->clear();
      pcFrameSpec->getRplrBuffer( LIST_0 )->setRefPicListReorderingFlag( false );
      pcFrameSpec->getRplrBuffer( LIST_1 )->clear();
      pcFrameSpec->getRplrBuffer( LIST_1 )->setRefPicListReorderingFlag( false );
    }
  }

  return Err::m_nOK;

}

ErrVal
PicEncoder::xGetListSizesSpecial ( UInt  uiTemporalLevel,
                                   UInt  uiFrameIdInGOP,
                                   UInt  auiPredListSize[2])
{
// do not consider the delay as input 
  auiPredListSize[0] = 0;
  auiPredListSize[1] = 0;
  Int iPocInGOP; 
  for( iPocInGOP = (Int) (uiFrameIdInGOP-1); iPocInGOP>= 0 ; iPocInGOP-- )
  {
    if(m_acFrameSpecification[iPocInGOP].getTemporalLayer()<uiTemporalLevel && m_acFrameSpecification[iPocInGOP].getNalRefIdc())
    {
      auiPredListSize[0]++;
    }
  }

  for( iPocInGOP = (Int) uiFrameIdInGOP+1; iPocInGOP<= (Int) m_uiGOPSize; iPocInGOP++ )
  {
    if(m_acFrameSpecification[iPocInGOP].getTemporalLayer()<uiTemporalLevel && m_acFrameSpecification[iPocInGOP].getNalRefIdc())
    {
      auiPredListSize[1]++;
    }
  }

  UInt uiMaxNumActiveList0 = m_uiMaxNumRefFrames;
  UInt uiMaxNumActiveList1 = m_uiMaxNumRefFrames;
// for anchor pictures these values shall be 0 anyway 
  auiPredListSize[0]    = min( uiMaxNumActiveList0, auiPredListSize[0] );
  auiPredListSize[1]    = min( uiMaxNumActiveList1, auiPredListSize[1] );

  return Err::m_nOK;
}

ErrVal
PicEncoder::xGetListSizes ( UInt  uiTemporalLevel,
                            UInt  uiFrameIdInGOP,
                            UInt  auiPredListSize[2])
{
  //----- get delay decomposition stages -----
  UInt  uiDelayDecompositionStages = 0;
  for( ; m_uiFrameDelay >> uiDelayDecompositionStages; uiDelayDecompositionStages++ );
  uiDelayDecompositionStages = min( m_uiMaxTL, uiDelayDecompositionStages );


  //----- loop over prediction and update steps -----
  for( UInt uiLevel = uiTemporalLevel; uiLevel <= m_uiMaxTL; uiLevel++ )
  {
    //----- get parameters base GOP size and cut-off frame id -----
    UInt  uiBaseLevel       = m_uiMaxTL - uiLevel;
    UInt  uiFrameIdLevel    = uiFrameIdInGOP >> uiBaseLevel;
    UInt  uiBaseGOPSize     = ( 1 << uiDelayDecompositionStages ) >> min( uiBaseLevel, uiDelayDecompositionStages );
    UInt  uiCutOffFrame     = max( 0, Int( uiBaseGOPSize - ( m_uiFrameDelay >> uiBaseLevel ) - 1 ) );

    if( uiLevel == uiTemporalLevel )
    {
      //=========== PREDICTION LIST SIZES ==========
      auiPredListSize[0]    = ( uiFrameIdLevel + 1 ) >> 1;
      UInt  uiFrameIdWrap   = ( uiFrameIdLevel % uiBaseGOPSize );
      if( uiFrameIdWrap > uiCutOffFrame )
      {
        auiPredListSize[1]  = ( uiBaseGOPSize - uiFrameIdWrap + 1 ) >> 1;
      }
      else
      {
        auiPredListSize[1]  = ( uiCutOffFrame - uiFrameIdWrap + 1 ) >> 1;
      }
      
      UInt uiMaxNumActiveList0 = m_uiMaxNumRefFrames;
      UInt uiMaxNumActiveList1 = m_uiMaxNumRefFrames;
// for anchor pictures these values shall be 0 anyway 
      auiPredListSize[0]    = min( uiMaxNumActiveList0, auiPredListSize[0] );
      auiPredListSize[1]    = min( uiMaxNumActiveList1, auiPredListSize[1] );

      //----- take into account actual GOP size -----
      {
        UInt  uiMaxL1Size   = ( ( m_uiGOPSize >> uiBaseLevel ) + 1 - uiFrameIdLevel ) >> 1;
        auiPredListSize[1]  = min( uiMaxL1Size,         auiPredListSize[1] );
      }
    }
  }
  
  UInt  uiCurrFrame   = (   m_uiAnchorFrameNumber) + uiFrameIdInGOP;
	UInt  uiIntraPeriod = m_vFramePeriod;
  if( ( uiCurrFrame % uiIntraPeriod ) == 0 )
	{
	  auiPredListSize[0] = 0;
	  auiPredListSize[1] = 0;
	}


  return Err::m_nOK;
}

ErrVal
PicEncoder::xInitReorderingInterView (SliceHeader*&     rpcSliceHeader)
{
  SliceType eSliceType        = rpcSliceHeader->getSliceType();
  SpsMvcExtension * pcSPSMVC  = m_pcSPS->getSpsMVC();
  UInt  uiCurrViewId          = rpcSliceHeader->getViewId();
  Bool  bAnchor               = rpcSliceHeader->getAnchorPicFlag();
  UInt  uiMaxLists            = ( eSliceType == B_SLICE ? 2 : eSliceType == P_SLICE ? 1 : 0 );
  Bool  bInterPredFirst       = m_bInterPridPicsFirst; 
  UInt  uiIdentifier          = 0;
  UInt  uiCommand             = 0;
// JVT-W040 just to make sure
  if ( ! bInterPredFirst )
    rpcSliceHeader->setDirectSpatialMvPredFlag            ( true );

  for( UInt uiList = 0; uiList < uiMaxLists; uiList++ )
  {
     ListIdx eListIdx  = ListIdx( uiList );
    // generate RPLR commands for inter-view prediciton pictures
    RplrBuffer * pcRplrBufferInterView = new RplrBuffer;
    RplrBuffer * pRplr = &rpcSliceHeader->getRplrBuffer( eListIdx );
    UInt        idx          = 0;

    for (idx=0; idx< pcSPSMVC->getNumRefsForListX (uiCurrViewId, eListIdx, bAnchor); idx++)
    {
        pcRplrBufferInterView->set( idx, Rplr(RPLR_VIEW_POS, 0));
    }
    
    pcRplrBufferInterView->set( idx, Rplr(RPLR_END));
    // update the RPLR commands for all
    UInt      uiIndex          = 0;
    UInt      uiIndexInterView =0;
    if (bInterPredFirst)
    {
      while( RPLR_END != ( uiCommand = pRplr->get( uiIndex ).getCommand( uiIdentifier ) ) )
        uiIndex++;
      while( RPLR_END != ( uiCommand = pcRplrBufferInterView->get( uiIndexInterView).getCommand( uiIdentifier ) ) )
      {
        pRplr->set( uiIndex+uiIndexInterView, pcRplrBufferInterView->get( uiIndexInterView)) ;
        uiIndexInterView++;
      }
      pRplr->set( uiIndex+uiIndexInterView, Rplr(RPLR_END)); 
      if( uiIndex+uiIndexInterView > 0) pRplr->setRefPicListReorderingFlag (true);
    }
    else
    {
      while( RPLR_END != ( uiCommand = pcRplrBufferInterView->get( uiIndexInterView).getCommand( uiIdentifier ) ) )
        uiIndexInterView++;  
      while( RPLR_END != ( uiCommand = pRplr->get( uiIndex ).getCommand( uiIdentifier ) ) )
      {
        pcRplrBufferInterView->set( uiIndex+uiIndexInterView, pRplr->get( uiIndex ));
        uiIndex++;
      }
      pcRplrBufferInterView->set( uiIndex+uiIndexInterView, Rplr(RPLR_END)); 
      uiIndexInterView = 0;
      while( RPLR_END != ( uiCommand = pcRplrBufferInterView->get( uiIndexInterView).getCommand( uiIdentifier ) ) )
      {
        pRplr->set( uiIndexInterView, pcRplrBufferInterView->get( uiIndexInterView) );
        uiIndexInterView++;  
      }
      pRplr->set(uiIndexInterView, Rplr(RPLR_END));        
      if( uiIndexInterView >0) pRplr->setRefPicListReorderingFlag ( true ) ;
    }
    delete pcRplrBufferInterView;
  }

  return Err::m_nOK;

}

ErrVal
PicEncoder::xInitSliceHeader( SliceHeader*&     rpcSliceHeader,
                  			      FrameSpec&  rcFrameSpec,
                              Double&           rdLambda,
                              Bool              fakeHeader,
							  PicType           ePicType)
{
  ROF( m_bInitParameterSets );

  //===== create new slice header =====
  if ( getAVCFlag() )
    rpcSliceHeader = new SliceHeader( *m_pcSPSBase, *m_pcPPSBase );
  else 
    rpcSliceHeader = new SliceHeader( *m_pcSPS, *m_pcPPS );
  ROF( rpcSliceHeader );

  //===== determine parameters =====
  Double dQp      = m_pcCodingParameter->getBasisQp() + m_pcCodingParameter->getDeltaQpLayer( rcFrameSpec.getTemporalLayer() );
  Int    iQp      = min( 51, max( 0, (Int)dQp ) );
  rdLambda        = 0.85 * pow( 2.0, min( 52.0, dQp ) / 3.0 - 4.0 );

  //===== set NAL unit header =====
  rpcSliceHeader->setNalRefIdc                          ( rcFrameSpec.getNalRefIdc    () );
  rpcSliceHeader->setNalUnitType                        ( rcFrameSpec.getNalUnitType  () );
  rpcSliceHeader->setLayerId                            ( 0 );
  rpcSliceHeader->setTemporalLevel                      ( rcFrameSpec.getTemporalLayer() );
  rpcSliceHeader->setQualityLevel                       ( 0 );
  rpcSliceHeader->setKeyPictureFlag                     ( rcFrameSpec.getTemporalLayer() == 0 );
  rpcSliceHeader->setSimplePriorityId                   ( 0 );
  rpcSliceHeader->setDiscardableFlag                    ( false );
  rpcSliceHeader->setReservedZeroBit                   ( false ); //JVT-S036 //rpcSliceHeader->setExtensionFlag  ( false );
  rpcSliceHeader->setViewId(this->getViewId());	
  rpcSliceHeader->setAVCFlag( getAVCFlag()!=0);  //JVT-W035

  if (fakeHeader)//bad method: for multiview header
  {
      rpcSliceHeader->setFieldPicFlag                   ( false );
      rpcSliceHeader->setBottomFieldFlag                ( false );
  }
  else
  {
      rpcSliceHeader->setFieldPicFlag                   ( ePicType != FRAME );//lufeng
      rpcSliceHeader->setBottomFieldFlag                ( ePicType == BOT_FIELD );//lufeng
  }


  //===== set general parameters =====
  rpcSliceHeader->setFirstMbInSlice                     ( 0 );
  rpcSliceHeader->setLastMbInSlice                      ( m_uiMbNumber - 1 );
  rpcSliceHeader->setSliceType                          ( rcFrameSpec.getSliceType    () );
  rpcSliceHeader->setFrameNum                           ( rcFrameSpec.getFrameNum     () );
  rpcSliceHeader->setNumMbsInSlice                      ( m_uiMbNumber );
  rpcSliceHeader->setIdrPicId                           ( m_uiIdrPicId );
  rpcSliceHeader->setDirectSpatialMvPredFlag            ( true ); //JVT-W040
  rpcSliceHeader->setBaseLayerId                        ( MSYS_UINT_MAX );
  rpcSliceHeader->setBaseQualityLevel                   ( 3 );
  rpcSliceHeader->setAdaptivePredictionFlag             ( false );
  rpcSliceHeader->setNoOutputOfPriorPicsFlag            ( true );
  rpcSliceHeader->setCabacInitIdc                       ( 0 );
  rpcSliceHeader->setSliceHeaderQp                      ( iQp );
  rpcSliceHeader->setFragmentedFlag                     ( false );
  rpcSliceHeader->setFragmentOrder                      ( 0 );
  rpcSliceHeader->setLastFragmentFlag                   ( true );
  rpcSliceHeader->setBaseLayerUsesConstrainedIntraPred  ( false );
  rpcSliceHeader->setFgsComponentSep                    ( false );

//cs, fix a bug (add #if)
  rpcSliceHeader->setAnchorPicFlag                      ( rcFrameSpec.isAnchor() && ePicType != BOT_FIELD);


  //===== set deblocking filter parameters =====
  if( rpcSliceHeader->getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    rpcSliceHeader->getDeblockingFilterParameter().setDisableDeblockingFilterIdc(   m_pcCodingParameter->getLoopFilterParams().getFilterIdc   () );
    rpcSliceHeader->getDeblockingFilterParameter().setSliceAlphaC0Offset        ( 2*m_pcCodingParameter->getLoopFilterParams().getAlphaOffset () );
    rpcSliceHeader->getDeblockingFilterParameter().setSliceBetaOffset           ( 2*m_pcCodingParameter->getLoopFilterParams().getBetaOffset  () );
  }
  //
  rpcSliceHeader->setNonIDRFlag( (rcFrameSpec.getNalUnitType () == NAL_UNIT_CODED_SLICE_IDR ) ? false : true ); // JVT-W035 
  rpcSliceHeader->setSimplePriorityId( rpcSliceHeader->getViewId()== 0 ? rpcSliceHeader->getTemporalLevel() : ( m_uiMaxTL+rpcSliceHeader->getViewId()%2 +1) ); // JVT-W035
  //===== set prediction and update list sizes =====
  //===== reference picture list ===== (init with default data, later updated)

  {
    //--- prediction ---
    UInt auiNumViewRef [2];

    if( rcFrameSpec.getSliceType()==I_SLICE) rcFrameSpec.setNumRefIdxActive(LIST_0, 0);
    if( rcFrameSpec.getSliceType()!=B_SLICE) rcFrameSpec.setNumRefIdxActive(LIST_1, 0);

	if(ePicType==FRAME)
	{
		auiNumViewRef [LIST_0] = ( TimeForVFrameP(rcFrameSpec.getContFrameNumber()) ? m_uiAnchorNumFwdViewRef : rcFrameSpec.getNumRefIdxActive(LIST_0) + m_uiNonAncNumFwdViewRef );
		auiNumViewRef [LIST_1] = ( TimeForVFrameP(rcFrameSpec.getContFrameNumber()) ? m_uiAnchorNumBwdViewRef : rcFrameSpec.getNumRefIdxActive(LIST_1) + m_uiNonAncNumBwdViewRef );
	}
	else//lufeng: field ref num modify, not support anchor picture
	{
		auiNumViewRef [LIST_0]=m_uiAnchorNumFwdViewRef;
		auiNumViewRef [LIST_1]=m_uiAnchorNumBwdViewRef;
		if(!TimeForVFrameP(rcFrameSpec.getContFrameNumber()))//not anchor
		{
			auiNumViewRef [LIST_0]=m_uiNonAncNumFwdViewRef;
			auiNumViewRef [LIST_1]=m_uiNonAncNumBwdViewRef;
			auiNumViewRef [LIST_0]+=rcFrameSpec.getNumRefIdxActive(LIST_0)*2;
			auiNumViewRef [LIST_1]+=rcFrameSpec.getNumRefIdxActive(LIST_1)*2;
		}
		if(ePicType==BOT_FIELD)
		{
			if(rcFrameSpec.getNalRefIdc())
				auiNumViewRef [LIST_0]++;//ref to top
			if(NAL_UNIT_CODED_SLICE_IDR)
			{
				rpcSliceHeader->setNalUnitType( NAL_UNIT_CODED_SLICE );
		        rpcSliceHeader->setNonIDRFlag( true); // JVT-W035
			}
		}
	}

    SliceType eSliceType =( auiNumViewRef [LIST_1] > 0 ? B_SLICE : auiNumViewRef [LIST_0] > 0 ? P_SLICE : I_SLICE);
    rpcSliceHeader->setSliceType( eSliceType );
    rpcSliceHeader->setNumRefIdxActive( LIST_0, auiNumViewRef[LIST_0] );
    rpcSliceHeader->setNumRefIdxActive( LIST_1, auiNumViewRef[LIST_1] );
    UInt  uiMaxLists = ( eSliceType == B_SLICE ? 2 : eSliceType == P_SLICE ? 1 : 0 );
    for( UInt uiList = 0; uiList < uiMaxLists; uiList++ )
    {
      ListIdx eListIdx  = ListIdx( uiList );
      

      if( rpcSliceHeader->getPPS().getNumRefIdxActive( eListIdx ) != rpcSliceHeader->getNumRefIdxActive( eListIdx))
      {
        rpcSliceHeader->setNumRefIdxActiveOverrideFlag( true );
      }
    }
    //===== set MMCO commands =====
    if(rcFrameSpec.isIdrNalUnit() && rpcSliceHeader->getSliceType() == I_SLICE)
    {
      rpcSliceHeader->setAdaptiveRefPicBufferingFlag(false);
      rpcSliceHeader->getRplrBuffer( LIST_0 ).setRefPicListReorderingFlag(false);
      rpcSliceHeader->getRplrBuffer( LIST_1 ).setRefPicListReorderingFlag(false);
    }
    else
    {
	 if( ! (rpcSliceHeader->getSPS().getFrameMbsOnlyFlag()) )//not support rplr&mmco for interlaced coding now
	 {
		 rpcSliceHeader->getRplrBuffer( LIST_0 ).setRefPicListReorderingFlag( false );
		 rpcSliceHeader->getRplrBuffer( LIST_1 ).setRefPicListReorderingFlag( false );
		 rpcSliceHeader->setAdaptiveRefPicBufferingFlag(false);
	 }
	 else{
     if( rcFrameSpec.getMmcoBuf() )
     {
      rpcSliceHeader->setAdaptiveRefPicBufferingFlag( rcFrameSpec.getAdaptiveRefPicBufferingFlag());
      rpcSliceHeader->getMmcoBuffer().copy( *rcFrameSpec.getMmcoBuf() );
     }

     //===== set RPRL commands =====
     if( rcFrameSpec.getRplrBuf( LIST_0 ) )
     {
      rpcSliceHeader->getRplrBuffer( LIST_0 ).setRefPicListReorderingFlag( rcFrameSpec.getRplrBuf( LIST_0 )->getRefPicListReorderingFlag() );
      rpcSliceHeader->getRplrBuffer( LIST_0 ).copy( *(StatBuf<Rplr,32>*)rcFrameSpec.getRplrBuf( LIST_0 ) );
     }
     if( rcFrameSpec.getRplrBuf( LIST_1 ) )
     {
      rpcSliceHeader->getRplrBuffer( LIST_1 ).setRefPicListReorderingFlag( rcFrameSpec.getRplrBuf( LIST_1 )->getRefPicListReorderingFlag() );
      rpcSliceHeader->getRplrBuffer( LIST_1 ).copy( *(StatBuf<Rplr,32>*)rcFrameSpec.getRplrBuf( LIST_1 ) );
     }
    }
	}
  }

  RNOK( m_pcPocCalculator->setPoc( *rpcSliceHeader, rcFrameSpec.getContFrameNumber()
	  * (rpcSliceHeader->getSPS().getFrameMbsOnlyFlag()?1:2)+(ePicType==BOT_FIELD?1:0)) );//lufeng
  
#if 0
  //===== initialize prediction weights =====
  RNOK( xInitPredWeights( *rpcSliceHeader ) );
#endif 

  //===== flexible macroblock ordering =====
  rpcSliceHeader->setSliceGroupChangeCycle( 1 );
  rpcSliceHeader->FMOInit();

  
  return Err::m_nOK;

}

// rplr and mmco:  }}

ErrVal
PicEncoder::xInitPredWeights( SliceHeader& rcSliceHeader )
{
  if( rcSliceHeader.isInterP() )
  {
    RNOK( rcSliceHeader.getPredWeightTable( LIST_0 ).init( 64 ) );

    if( rcSliceHeader.getPPS().getWeightedPredFlag() )
    {
      rcSliceHeader.setLumaLog2WeightDenom  ( 6 );
      rcSliceHeader.setChromaLog2WeightDenom( 6 );
      RNOK( rcSliceHeader.getPredWeightTable( LIST_0 ).initDefaults( rcSliceHeader.getLumaLog2WeightDenom(), rcSliceHeader.getChromaLog2WeightDenom() ) );
      RNOK( rcSliceHeader.getPredWeightTable( LIST_0 ).createRandomParameters() );
    }
  }
  else if( rcSliceHeader.isInterB() )
  {
    RNOK( rcSliceHeader.getPredWeightTable( LIST_0 ).init( 64 ) );
    RNOK( rcSliceHeader.getPredWeightTable( LIST_1 ).init( 64 ) );

    if( rcSliceHeader.getPPS().getWeightedBiPredIdc() == 1 )
    {
      rcSliceHeader.setLumaLog2WeightDenom  ( 6 );
      rcSliceHeader.setChromaLog2WeightDenom( 6 );
      RNOK( rcSliceHeader.getPredWeightTable( LIST_0 ).initDefaults( rcSliceHeader.getLumaLog2WeightDenom(), rcSliceHeader.getChromaLog2WeightDenom() ) );
      RNOK( rcSliceHeader.getPredWeightTable( LIST_1 ).initDefaults( rcSliceHeader.getLumaLog2WeightDenom(), rcSliceHeader.getChromaLog2WeightDenom() ) );
      RNOK( rcSliceHeader.getPredWeightTable( LIST_0 ).createRandomParameters() );
      RNOK( rcSliceHeader.getPredWeightTable( LIST_1 ).createRandomParameters() );
    }
  }
  return Err::m_nOK;
}


ErrVal
PicEncoder::xInitExtBinDataAccessor( ExtBinDataAccessor& rcExtBinDataAccessor )
{
  ROF( m_pucWriteBuffer );
  m_cBinData.reset          ();
  m_cBinData.set            ( m_pucWriteBuffer, m_uiWriteBufferSize );
  m_cBinData.setMemAccessor ( rcExtBinDataAccessor );

  return Err::m_nOK;
}


ErrVal
PicEncoder::xAppendNewExtBinDataAccessor( ExtBinDataAccessorList& rcExtBinDataAccessorList,
                                          ExtBinDataAccessor*     pcExtBinDataAccessor )
{
  ROF( pcExtBinDataAccessor );
  ROF( pcExtBinDataAccessor->data() );

  UInt    uiNewSize     = pcExtBinDataAccessor->size();
  UChar*  pucNewBuffer  = new UChar [ uiNewSize ];
  ROF( pucNewBuffer );
  ::memcpy( pucNewBuffer, pcExtBinDataAccessor->data(), uiNewSize * sizeof( UChar ) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor );

  m_cBinData              .reset          ();
  m_cBinData              .set            (  pucNewBuffer, uiNewSize );
  m_cBinData              .setMemAccessor ( *pcNewExtBinDataAccessor );
  rcExtBinDataAccessorList.push_back      (  pcNewExtBinDataAccessor );

  m_cBinData              .reset          ();
  m_cBinData              .setMemAccessor ( *pcExtBinDataAccessor );

  return Err::m_nOK;
}



ErrVal
PicEncoder::xEncodePicture( ExtBinDataAccessorList& rcExtBinDataAccessorList,
                            RecPicBufUnit&          rcRecPicBufUnit,
                            SliceHeader&            rcSliceHeader,
                            Double                  dLambda,
                            UInt&                   ruiBits  )
{
  UInt  uiBits = 0;
    if (rcSliceHeader.getPicType() !=FRAME)
    {
             m_pcRecPicBuffer->SetCodeAsVFrameFlag(false);
    }
    else
    {
         m_pcRecPicBuffer->SetCodeAsVFrameFlag(TimeForVFrameP
        	  (m_cFrameSpecification.getContFrameNumber()));
		//			        (rcSliceHeader.getPoc()));//lufeng: not support VFrameP for field coding
    }
  rcSliceHeader.setSvcMvcFlag(this->getSvcMvcFlag());
  rcSliceHeader.setAVCFlag( this->getAVCFlag()!=0);  //JVT-W035
  rcSliceHeader.setViewId(this->getViewId());
  rcSliceHeader.setAnchorPicFlag(TimeForVFrameP(rcSliceHeader.getPoc()));
  rcSliceHeader.setReservedOneBit(1); // bug fix: prefix NAL (NTT)
  rcSliceHeader.setInterViewFalg(this->derivation_Inter_View_Flag(this->getViewId(), rcSliceHeader)); // JVT-W056 Samsung

  if( rcSliceHeader.getPicType()!=FRAME )
    {
		if(rcRecPicBufUnit.getRecFrame())
          RNOK( rcRecPicBufUnit.getRecFrame()->addFieldBuffer( rcSliceHeader.getPicType() ));
    }

  //===== start picture =====
  RefFrameList  cList0, cList1;
  RNOK( xStartPicture( rcRecPicBufUnit, rcSliceHeader, cList0, cList1 ) );
//  bug fix for TD March 19
  xSetRefPictures(rcSliceHeader, cList0, cList1);
//TMM_WP
  if(rcSliceHeader.getSliceType() == P_SLICE)
      m_pcSliceEncoder->xSetPredWeights( rcSliceHeader, 
                                         rcRecPicBufUnit.getRecFrame(),
                                         cList0,
                                         cList1);
  else if(rcSliceHeader.getSliceType() == B_SLICE)
      m_pcSliceEncoder->xSetPredWeights( rcSliceHeader, 
                                         rcRecPicBufUnit.getRecFrame(),
                                         cList0,
                                         cList1);
      
//TMM_WP

  //===== encoding of slice groups =====
  for( Int iSliceGroupID = 0; ! rcSliceHeader.getFMO()->SliceGroupCompletelyCoded( iSliceGroupID ); iSliceGroupID++ )
  {
    UInt  uiBitsSlice = 0;

    //----- init slice size -----
    rcSliceHeader.setFirstMbInSlice( rcSliceHeader.getFMO()->getFirstMacroblockInSlice( iSliceGroupID ) );
    rcSliceHeader.setLastMbInSlice ( rcSliceHeader.getFMO()->getLastMBInSliceGroup    ( iSliceGroupID ) );

    
// JVT-W035 {{
	if ( (rcSliceHeader.getNalUnitType() == NAL_UNIT_CODED_SLICE|| rcSliceHeader.getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR ) && (rcSliceHeader.getAVCFlag()) )
    {
          RNOK( xWritePrefixUnit( rcExtBinDataAccessorList, rcSliceHeader, uiBits ) );
    }
// JVT-W035 }}
    //----- init NAL unit -----
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

    //----- write slice header -----
    ETRACE_NEWSLICE;

    RNOK( m_pcNalUnitEncoder->write ( rcSliceHeader ) );
//JVT-W080
		if( getPdsEnable() )
		{
			m_pcSliceEncoder->setPdsEnable( getPdsEnable() );
		  m_pcSliceEncoder->setPdsInitialDelayMinus2L0( getPdsInitialDelayMinus2L0() );
		  m_pcSliceEncoder->setPdsInitialDelayMinus2L1( getPdsInitialDelayMinus2L1() );
		  m_pcSliceEncoder->setPdsBlockSize( getPdsBlockSize() );
			//re-set PdsBlockSize to be one row
			UInt uiPdsBlockSize = rcSliceHeader.getSPS().getFrameWidthInMbs();
			m_pcSliceEncoder->setPdsBlockSize( uiPdsBlockSize );
		}
//~JVT-W080

    //----- real coding -----

        //lufeng: add mbaff here
		if (rcSliceHeader.isMbAff()&&!rcSliceHeader.getFieldPicFlag())
	{
		RNOK( m_pcSliceEncoder->encodeSliceMbAff( rcSliceHeader,
			rcRecPicBufUnit.getRecFrame  (),
			rcRecPicBufUnit.getMbDataCtrl(),
			cList0,
			cList1,
			m_uiFrameWidthInMb,
			dLambda ) );
	}
	else
	{
		RNOK( m_pcSliceEncoder->encodeSlice( rcSliceHeader,
			rcRecPicBufUnit.getRecFrame  ()->getPic(rcSliceHeader.getPicType()),
			rcRecPicBufUnit.getMbDataCtrl(),
			cList0,
			cList1,
			m_uiFrameWidthInMb,
			dLambda ) );
	}


    //----- close NAL unit -----
    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBitsSlice ) );
    RNOK( xAppendNewExtBinDataAccessor( rcExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
    uiBitsSlice += 4*8;
    uiBits      += uiBitsSlice;
  }

  //===== finish =====
  RNOK( xFinishPicture( rcRecPicBufUnit, rcSliceHeader, cList0, cList1, uiBits ) );
  ruiBits += uiBits;

  return Err::m_nOK;
}


ErrVal
PicEncoder::xWritePrefixUnit( ExtBinDataAccessorList& rcExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit ) //JVT-W035
{
  UInt uiBit = 0;
  Bool m_bWriteSuffixUnit = true; 

  if( m_bWriteSuffixUnit )
  {
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

	  NalUnitType eNalUnitType = rcSH.getNalUnitType();
  	rcSH.setAVCCompatible( true ); 
// JVT-W035
    if( eNalUnitType == NAL_UNIT_CODED_SLICE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
    {
      rcSH.setNalUnitType( NAL_UNIT_CODED_SLICE_PREFIX );
    }
	  else
	  {
		  return Err::m_nERR;
	  }

    RNOK( m_pcNalUnitEncoder->write( rcSH ) ); 

    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );
    RNOK( xAppendNewExtBinDataAccessor( rcExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
    uiBit += 4*8;
    ruiBit += uiBit;

	  rcSH.setNalUnitType( eNalUnitType );

  }
  return Err::m_nOK;
}



ErrVal          
PicEncoder::xSetRefPictures(SliceHeader&                rcSliceHeader,
                            RefFrameList&               rcList0,
                            RefFrameList&               rcList1 )
{
  ROTRS( rcSliceHeader.isIntra(), Err::m_nOK );

  if( rcSliceHeader.isInterP() )
  {
      rcSliceHeader.setRefFrameList(&rcList0,rcSliceHeader.getPicType() ,LIST_0);
  }
  else // rcSliceHeader.isInterB()
  {
      rcSliceHeader.setRefFrameList(&rcList0,rcSliceHeader.getPicType() ,LIST_0);
      rcSliceHeader.setRefFrameList(&rcList1,rcSliceHeader.getPicType() ,LIST_1);
// spatial direct Ying March 17 2010  
   rcSliceHeader.setList1FirstShortTerm ( rcList1.getEntry(0)->getViewId()== rcSliceHeader.getViewId()) ;

  }
  return Err::m_nOK;
}

ErrVal
PicEncoder::xFieldList(    SliceHeader&   rcSliceHeader,
                           RefFrameList&  rcList,
                           RefFrameList&  rcListTemp )
{
	PicType               eCurrPicType      = rcSliceHeader.getPicType();
    PicType               eOppositePicture  = ( eCurrPicType == TOP_FIELD ? BOT_FIELD : TOP_FIELD );

    //----- initialize field list for short term pictures -----
    UInt  uiCurrentParityIndex  = 0;
    UInt  uiOppositeParityIndex = 0;

    while( uiCurrentParityIndex < rcListTemp.getActive() || uiOppositeParityIndex < rcListTemp.getActive() )
    {
        //--- current parity ---
        while( uiCurrentParityIndex < rcListTemp.getActive() )
        {
			if( rcListTemp[++uiCurrentParityIndex]->isPicReady( eCurrPicType ) )
            {
				rcList.add(rcListTemp[uiCurrentParityIndex]->getPic(eCurrPicType));
                break;
            }
        }
        //--- opposite parity ---
		while( uiOppositeParityIndex < rcListTemp.getActive() )
        {
			if( rcListTemp[++uiOppositeParityIndex]->isPicReady( eOppositePicture ) )
            {
				rcList.add(rcListTemp[uiOppositeParityIndex]->getPic(eOppositePicture));
                break;
            }
        }
    }

    return Err::m_nOK;
}

ErrVal
PicEncoder::xStartPicture( RecPicBufUnit& rcRecPicBufUnit,
                           SliceHeader&   rcSliceHeader,
                           RefFrameList&  rcList0,
                           RefFrameList&  rcList1 )
{
  //===== initialize reference picture lists and update slice header =====
  //RefFrameList rcListTemp0, rcListTemp1;
//===== initialize reference picture lists and update slice header =====

#if 0 // hwsun, fix a bug (disable)
  ROTRS( rcSliceHeader.isIntra(), Err::m_nOK );
#endif

  RNOK( m_pcRecPicBuffer->getRefLists( rcList0, rcList1, rcSliceHeader,m_pcQuarterPelFilter ) );//deal with the ref_list
  

  //===== reset macroblock data =====

  if(rcSliceHeader.getPicType()!=BOT_FIELD)//new frame
  {
   	  RNOK( rcRecPicBufUnit.getMbDataCtrl()->reset() );
	  RNOK( rcRecPicBufUnit.getMbDataCtrl()->clear() );
  }
  return Err::m_nOK;
}


ErrVal
PicEncoder::xFinishPicture( RecPicBufUnit&  rcRecPicBufUnit,
                            SliceHeader&    rcSliceHeader,
                            RefFrameList&   rcList0,
                            RefFrameList&   rcList1,
                            UInt            uiBits )
{
  //===== uninit half-pel data =====
  UInt uiPos;
  for( uiPos = 0; uiPos < rcList0.getActive(); uiPos++ )
  {
    IntFrame* pcRefFrame = rcList0.getEntry( uiPos );
    if( pcRefFrame->isExtended() )
    {
      pcRefFrame->clearExtended();
    }
    if( pcRefFrame->isHalfPel() )
    {
      pcRefFrame->uninitHalfPel();
    }
  }
  for( uiPos = 0; uiPos < rcList1.getActive(); uiPos++ )
  {
    IntFrame* pcRefFrame = rcList1.getEntry( uiPos );
    if( pcRefFrame->isExtended() )
    {
      pcRefFrame->clearExtended();
    }
    if( pcRefFrame->isHalfPel() )
    {
      pcRefFrame->uninitHalfPel();
    }
  }

  //===== deblocking =====
  RNOK( m_pcLoopFilter->process( rcSliceHeader,
                                 rcRecPicBufUnit.getRecFrame(),
                                 rcRecPicBufUnit.getMbDataCtrl(),
                                 rcRecPicBufUnit.getMbDataCtrl(),
                                 m_uiFrameWidthInMb,
                                 &rcList0,
                                 &rcList1,
                                 false) );

  //===== get PSNR =====
  if(rcSliceHeader.getPicType()==FRAME||rcSliceHeader.getPicType()==BOT_FIELD)//lufeng: support field
  {
	  Double dPSNR[3];
	  RNOK( xGetPSNR( rcRecPicBufUnit, dPSNR ) );

	  //===== output =====
	  printf( "%4d %c %s %s %4d  QP%3d  Y %7.4lf dB  U %7.4lf dB  V %7.4lf dB   bits%8d\n",
		m_uiCodedFrames,
		rcSliceHeader.getSliceType  ()==I_SLICE ? 'I' :
		rcSliceHeader.getSliceType  ()==P_SLICE ? 'P' : 'B',
		rcSliceHeader.getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR ? "IDR" :
		rcSliceHeader.getNalRefIdc  ()==NAL_REF_IDC_PRIORITY_LOWEST ? "   " : "REF",
			rcSliceHeader.getInterViewFlag() == true ? "REF_VIEW" : "       ", //JVT-W056  samsung
		rcSliceHeader.getPoc(),
		rcSliceHeader.getPicQp(),
		dPSNR[0],
		dPSNR[1],
		dPSNR[2],
		uiBits
		);
	  
	  //===== update parameters =====
	  m_uiCodedFrames++;
	  ETRACE_NEWFRAME;

  }
  else
  {
	  //===== output =====
	  printf( "%4d %c %s %s %4d  QP%3d  bits%8d\n",
		m_uiCodedFrames,
		rcSliceHeader.getSliceType  ()==I_SLICE ? 'I' :
		rcSliceHeader.getSliceType  ()==P_SLICE ? 'P' : 'B',
		rcSliceHeader.getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR ? "IDR" :
		rcSliceHeader.getNalRefIdc  ()==NAL_REF_IDC_PRIORITY_LOWEST ? "   " : "REF",
			rcSliceHeader.getInterViewFlag() == true ? "REF_VIEW" : "       ", //JVT-W056  samsung
		rcSliceHeader.getPoc(),
		rcSliceHeader.getPicQp(),
		uiBits
		);
  }

  return Err::m_nOK;
}


ErrVal
PicEncoder::xGetPSNR( RecPicBufUnit&  rcRecPicBufUnit,
                      Double*         adPSNR )
{
  //===== reset buffer control =====
  RNOK( m_pcYuvBufferCtrlFullPel->initMb() );

  //===== set parameters =====
const YuvBufferCtrl::YuvBufferParameter&  cBufferParam  = m_pcYuvBufferCtrlFullPel->getBufferParameter(FRAME);
  IntFrame*                                 pcFrame       = rcRecPicBufUnit.getRecFrame  ();
  PicBuffer*                                pcPicBuffer   = rcRecPicBufUnit.getPicBuffer ();
  
  //===== calculate PSNR =====
  Pel*    pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbLum();
  XPel*   pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbLumAddr();
  Int     iStride   = cBufferParam.getStride();
  Int     iWidth    = cBufferParam.getWidth ();
  Int     iHeight   = cBufferParam.getHeight();
  UInt    uiSSDY    = 0;
  UInt    uiSSDU    = 0;
  UInt    uiSSDV    = 0;
  Int     x, y;

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
      uiSSDY   += iDiff * iDiff;
    }
    pPelOrig += iStride;
    pPelRec  += iStride;
  }

  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbCb();
  pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbCbAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
      uiSSDU   += iDiff * iDiff;
    }
    pPelOrig += iStride;
    pPelRec  += iStride;
  }

  pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbCr();
  pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbCrAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
      uiSSDV   += iDiff * iDiff;
    }
    pPelOrig += iStride;
    pPelRec  += iStride;
  }

  Double fRefValueY = 255.0 * 255.0 * 16.0 * 16.0 * (Double)m_uiMbNumber;
  Double fRefValueC = fRefValueY / 4.0;
  adPSNR[0]         = ( uiSSDY ? 10.0 * log10( fRefValueY / (Double)uiSSDY ) : 99.99 );
  adPSNR[1]         = ( uiSSDU ? 10.0 * log10( fRefValueC / (Double)uiSSDU ) : 99.99 );
  adPSNR[2]         = ( uiSSDV ? 10.0 * log10( fRefValueC / (Double)uiSSDV ) : 99.99 );
  m_dSumYPSNR      += adPSNR[0];
  m_dSumUPSNR      += adPSNR[1];
  m_dSumVPSNR      += adPSNR[2];

  return Err::m_nOK;
}


// Determine if the frame at currentFrameNum should be coded as a v-frame.
bool PicEncoder::TimeForVFrameP (const int currentFrameNum) const {
  if (0 == m_vFramePeriod) return false;
  else return (0 == (currentFrameNum % m_vFramePeriod));
}

Bool
PicEncoder::derivation_Inter_View_Flag(UInt View_Id, SliceHeader& Sliceheader){     //JVT-W056

	UInt num_view;
  UInt i,j,vcOrder;
	num_view = m_pcSPS->getSpsMVC()->getNumViewMinus1()+1;
	if (Sliceheader.getAnchorPicFlag()){
    for(i = 0; i< num_view; i++){
      vcOrder = m_pcSPS->getSpsMVC()->getViewCodingOrder()[i];
      for(j=0; j< m_pcSPS->getSpsMVC()->getNumAnchorRefsForListX(vcOrder, 0); j++){
        if(View_Id == m_pcSPS->getSpsMVC()->getAnchorRefForListX(vcOrder, j, 0))
          return true;
      }
      for(j=0; j< m_pcSPS->getSpsMVC()->getNumAnchorRefsForListX(vcOrder, 1); j++){
        if(View_Id == m_pcSPS->getSpsMVC()->getAnchorRefForListX(vcOrder, j, 1))
          return true;
      }
    }
  }
  else{
    for(i = 0; i< num_view; i++){
			vcOrder = m_pcSPS->getSpsMVC()->getViewCodingOrder()[i];
			for(j=0; j< m_pcSPS->getSpsMVC()->getNumNonAnchorRefsForListX(vcOrder, 0); j++){
        if(View_Id == m_pcSPS->getSpsMVC()->getNonAnchorRefForListX(vcOrder, j, 0))
          return true;
      }
      for(j=0; j< m_pcSPS->getSpsMVC()->getNumNonAnchorRefsForListX(vcOrder, 1); j++){
        if(View_Id == m_pcSPS->getSpsMVC()->getNonAnchorRefForListX(vcOrder, j, 1))
          return true;
      }
    }	
  }

  return false;
}

H264AVC_NAMESPACE_END


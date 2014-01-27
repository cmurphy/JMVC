#if !defined(AFX_INTFRAME_H__98AFB9AC_5EE3_45A9_B09B_859511AC9090__INCLUDED_)
#define AFX_INTFRAME_H__98AFB9AC_5EE3_45A9_B09B_859511AC9090__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/IntYuvPicBuffer.h"


#include "H264AVCCommonLib/MbDataCtrl.h"
#include "DownConvert.h"

// TMM_ESS 
#include "ResizeParameters.h"






H264AVC_NAMESPACE_BEGIN


class QuarterPelFilter;
class MbDataCtrl;
class RecPicBufUnit;


class YuvBufferCtrl; 


class DPBUnit
{
protected:
  DPBUnit           ();
  virtual ~DPBUnit  ();

public:
  static ErrVal create        ( DPBUnit*&                   rpcDPBUnit,
                                YuvBufferCtrl&              rcYuvBufferCtrl,
                                const SequenceParameterSet& rcSPS );
  ErrVal        destroy       ();

  ErrVal        init          ( Int       iPoc,
                                UInt      uiFrameNum,
                                UInt      uiTemporalLevel,
                                Bool      bKeyPicture,
                                Bool      bNeededForReference,
                                Bool      bConstrainedIPred,
                                UInt      uiQualityLevel); //JVT-T054
  ErrVal        initNonEx     ( Int       iPoc,
                                UInt      uiFrameNum );
  ErrVal        initBase      ( DPBUnit&  rcDPBUnit,
                                IntFrame* pcFrameBaseRep );
  ErrVal        uninit        ();


  ErrVal        markNonRef    ();
  ErrVal        markOutputted ();


  Int           getPoc        ()  const { return m_iPoc; }
  UInt          getFrameNum   ()  const { return m_uiFrameNum; }
  UInt          getTLevel     ()  const { return m_uiTemporalLevel; }
  Bool          isKeyPic      ()  const { return m_bKeyPicture; }
  Bool          isExisting    ()  const { return m_bExisting; }
  Bool          isNeededForRef()  const { return m_bNeededForReference; }
  Bool          isOutputted   ()  const { return m_bOutputted; }
  Bool          isBaseRep     ()  const { return m_bBaseRepresentation; }
  Bool          isConstrIPred ()  const { return m_bConstrainedIntraPred; }
  IntFrame*     getFrame      ()        { return m_pcFrame; }
  ControlData&  getCtrlData   ()        { return m_cControlData; }
  Int           getPicNum     ( UInt uiCurrFrameNum, 
                                UInt uiMaxFrameNum ) const
  {
    if( m_uiFrameNum > uiCurrFrameNum )
    {
      return (Int)m_uiFrameNum - (Int)uiMaxFrameNum;
    }
    return (Int)m_uiFrameNum;
  }
//JVT-T054{
  UInt          getQualityLevel() const { return m_uiQualityLevel;}
  Void          setMbDataCtrl(MbDataCtrl* pcMbDataCtrl) { m_cControlData.setMbDataCtrl(pcMbDataCtrl);}
//JVT-T054}
private:
  Int         m_iPoc;
  UInt        m_uiFrameNum;
  UInt        m_uiTemporalLevel;
  Bool        m_bKeyPicture;
  Bool        m_bExisting;
  Bool        m_bNeededForReference;
  Bool        m_bOutputted;
  Bool        m_bBaseRepresentation;
  IntFrame*   m_pcFrame;
  ControlData m_cControlData;
  Bool        m_bConstrainedIntraPred;
  UInt        m_uiQualityLevel; //JVT-T054
};

typedef MyList<DPBUnit*>  DPBUnitList;



class H264AVCCOMMONLIB_API IntFrame
{
public:
    IntFrame                ( YuvBufferCtrl&    rcYuvFullPelBufferCtrl,
        YuvBufferCtrl&    rcYuvHalfPelBufferCtrl,
        PicType ePicType = FRAME);
	virtual ~IntFrame       ();

  ErrVal  init            ( Bool              bHalfPel = false );
  ErrVal  initHalfPel     ();
  ErrVal  uninit          ();
  ErrVal  uninitHalfPel   ();

  ErrVal  load            ( PicBuffer*        pcPicBuffer );
  ErrVal  store           ( PicBuffer*        pcPicBuffer );
  ErrVal extendFrame( QuarterPelFilter* pcQuarterPelFilter, PicType ePicType, Bool bFrameMbsOnlyFlag );
  ErrVal  extendFrame     ( QuarterPelFilter* pcQuarterPelFilter );

  Void      setDPBUnit      ( DPBUnit*  pcDPBUnit ) { m_pcDPBUnit = pcDPBUnit; }
  DPBUnit*  getDPBUnit      ()                      { return m_pcDPBUnit; }

  Void            setRecPicBufUnit( RecPicBufUnit* pcUnit ) { m_pcDPBUnit = (DPBUnit*)(Void*)pcUnit; }
  RecPicBufUnit*  getRecPicBufUnit()                        { return (RecPicBufUnit*)(Void*)m_pcDPBUnit; }
  ErrVal clip()
  {
    RNOK( getFullPelYuvBuffer()->clip() );
    return Err::m_nOK;
  }
  
  ErrVal prediction       ( IntFrame* pcMCPFrame, IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->prediction       ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame->getFullPelYuvBuffer() ) );
    return Err::m_nOK;
  }
  
  ErrVal update           ( IntFrame* pcMCPFrame, IntFrame* pcSrcFrame, UInt uiShift )
  {
    RNOK( getFullPelYuvBuffer()->update           ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame->getFullPelYuvBuffer(), uiShift ) );
    return Err::m_nOK;
  }
  
  ErrVal inverseUpdate    ( IntFrame* pcMCPFrame, IntFrame* pcSrcFrame, UInt uiShift )
  {
    RNOK( getFullPelYuvBuffer()->inverseUpdate    ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame->getFullPelYuvBuffer(), uiShift ) );
    return Err::m_nOK;
  }


  ErrVal update           ( IntFrame* pcMCPFrame0, IntFrame* pcMCPFrame1, IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->update           ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame0->getFullPelYuvBuffer(), pcMCPFrame1->getFullPelYuvBuffer() ) );
    return Err::m_nOK;
  }
  
  ErrVal inverseUpdate    ( IntFrame* pcMCPFrame0, IntFrame* pcMCPFrame1, IntFrame* pcSrcFrame )
  {
		if (pcMCPFrame0 && pcMCPFrame1){
			RNOK( getFullPelYuvBuffer()->inverseUpdate    ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame0->getFullPelYuvBuffer(), pcMCPFrame1->getFullPelYuvBuffer() ) );
		}else if (pcMCPFrame0){
			RNOK( getFullPelYuvBuffer()->inverseUpdate    ( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame0->getFullPelYuvBuffer(), (IntYuvPicBuffer*)NULL ) );
		}else{
			RNOK( getFullPelYuvBuffer()->inverseUpdate    ( pcSrcFrame->getFullPelYuvBuffer(), (IntYuvPicBuffer*)NULL, pcMCPFrame1->getFullPelYuvBuffer() ) );
		}
    return Err::m_nOK;
  }

  ErrVal inversePrediction( IntFrame* pcMCPFrame, IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->inversePrediction( pcSrcFrame->getFullPelYuvBuffer(), pcMCPFrame->getFullPelYuvBuffer() ) );
    return Err::m_nOK;
  }



  ErrVal  copyAll     ( IntFrame* pcSrcFrame )
  {
// JVT-Q065 EIDR{
	  if(!m_bUnusedForRef)
	  {
		  m_bUnusedForRef = pcSrcFrame->getUnusedForRef();
	  }
// JVT-Q065 EIDR}
    m_iPOC        = pcSrcFrame->m_iPOC;
    m_iTopFieldPoc  = pcSrcFrame->m_iTopFieldPoc;
    m_iBotFieldPoc  = pcSrcFrame->m_iBotFieldPoc;
    RNOK( m_cFullPelYuvBuffer.copy( &pcSrcFrame->m_cFullPelYuvBuffer ) );
  
    return Err::m_nOK;
  }

  ErrVal  copy        ( IntFrame* pcSrcFrame )
  {
	m_bUnusedForRef = pcSrcFrame->getUnusedForRef();// JVT-Q065 EIDR

    RNOK( getFullPelYuvBuffer()->copy( pcSrcFrame->getFullPelYuvBuffer()) );
    return Err::m_nOK;
  }
//	TMM_EC {{
	ErrVal  copy        ( Frame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->copy( pcSrcFrame->getFullPelYuvBuffer()) );
    return Err::m_nOK;
  }
//TMM_EC }}  
  
  ErrVal  subtract    ( IntFrame* pcSrcFrame0, IntFrame* pcSrcFrame1 )
  {
    RNOK( getFullPelYuvBuffer()->subtract( pcSrcFrame0->getFullPelYuvBuffer(), pcSrcFrame1->getFullPelYuvBuffer() ) );
    return Err::m_nOK;
  }
  
  ErrVal  add         ( IntFrame* pcSrcFrame )
  {
    RNOK( getFullPelYuvBuffer()->add ( pcSrcFrame->getFullPelYuvBuffer()) );
    return Err::m_nOK;
  }
  
  ErrVal  setZero     ()
  {
    getFullPelYuvBuffer()->setZero();
    return Err::m_nOK;
  }

  ErrVal  setNonZeroFlags( UShort* pusNonZeroFlags, UInt uiStride )
  {
    return getFullPelYuvBuffer()->setNonZeroFlags( pusNonZeroFlags, uiStride );
  }

  ErrVal getSSD( Double& dSSDY, Double& dSSDU, Double& dSSDV, PicBuffer* pcOrgPicBuffer )
  {
    RNOK( m_cFullPelYuvBuffer.getSSD( dSSDY, dSSDU, dSSDV, pcOrgPicBuffer ) );
    return Err::m_nOK;
  }

  ErrVal dump( FILE* pFile, Int uiBandType, MbDataCtrl* pcMbDataCtrl )
  {
    if( uiBandType != 0 )
    {
      RNOK( getFullPelYuvBuffer()->dumpHPS( pFile, pcMbDataCtrl ) );
    }
    else
    {
      RNOK( getFullPelYuvBuffer()->dumpLPS( pFile ) );
    }
    return Err::m_nOK;
  }


  ErrVal loadFromFile8BitAndFillMargin( FILE* pFILE )
  {
    RNOK( getFullPelYuvBuffer()->loadFromFile8Bit( pFILE ) );
    RNOK( getFullPelYuvBuffer()->fillMargin      () );
    return Err::m_nOK;
  }

// TMM_ESS {
	ErrVal upsample     ( DownConvert& rcDownConvert, ResizeParameters* pcParameters, Bool bClip )
  {
    RNOK( getFullPelYuvBuffer()->upsample( rcDownConvert, pcParameters, bClip ) );
    return Err::m_nOK;
  }

  ErrVal upsampleResidual ( DownConvert& rcDownConvert, ResizeParameters* pcParameters, MbDataCtrl* pcMbDataCtrl, Bool bClip )
  {
    RNOK( getFullPelYuvBuffer()->upsampleResidual( rcDownConvert, pcParameters, pcMbDataCtrl, bClip ) );
    return Err::m_nOK;
  }
// TMM_ESS }


  IntYuvPicBuffer*  getFullPelYuvBuffer     ()        { return &m_cFullPelYuvBuffer; }
  IntYuvPicBuffer*  getHalfPelYuvBuffer     ()        { return &m_cHalfPelYuvBuffer; }

  Int   getPOC()          const   { return m_iPOC; }
  Void  setPOC( Int iPoc)         { m_iPOC = iPoc; }

//JVT-S036 lsj{
  Int	getFrameNum()	  const		{ return m_iFrameNum; }
  Void  setFrameNum( Int iNum )		{ m_iFrameNum = iNum; }
//JVT-S036 }

// JVT-Q065 EIDR{
  Bool	getUnusedForRef()			  { return m_bUnusedForRef; }
  Void	setUnusedForRef( Bool b )	  { m_bUnusedForRef = b; }
// JVT-Q065 EIDR}

  Bool  isHalfPel()   { return m_bHalfPel; }

  Void  setHalfPel(Bool b)   { m_bHalfPel=b; }//lufeng

  Bool  isExtended () { return m_bExtended; }
  Void  clearExtended() { m_bExtended = false; }

  Void  setExtended  ()                  { m_bExtended = true; }

  Bool  isPocAvailable()           const { return m_bPocIsSet; }
  Int   getPoc        ()           const { return m_iPOC; }
  Int   getTopFieldPoc()           const { return m_iTopFieldPoc; }
  Int   getBotFieldPoc()           const { return m_iBotFieldPoc; }
  Void  setPoc        ( Int iPoc )       { m_iPOC = iPoc; m_bPocIsSet = true; }
  Void  setPoc        ( const SliceHeader& rcSH )
  {
      ASSERT( m_ePicType==FRAME );
      const PicType ePicType = rcSH.getPicType();

      if( ePicType & TOP_FIELD )
      {
          m_iTopFieldPoc = rcSH.getTopFieldPoc();
          if( m_pcIntFrameTopField && m_pcIntFrameBotField )
          {
              m_pcIntFrameTopField->setPoc( m_iTopFieldPoc );
              setPoc( m_pcIntFrameBotField->isPocAvailable() ? max( m_pcIntFrameBotField->getPoc(), m_iTopFieldPoc ) : m_iTopFieldPoc );
          }
      }
      if( ePicType & BOT_FIELD )
      {
          m_iBotFieldPoc = rcSH.getBotFieldPoc();
          if( m_pcIntFrameTopField && m_pcIntFrameBotField )
          {
              m_pcIntFrameBotField->setPoc( m_iBotFieldPoc );
              setPoc( m_pcIntFrameTopField->isPocAvailable() ? min( m_pcIntFrameTopField->getPoc(), m_iBotFieldPoc ) : m_iBotFieldPoc );
          }
      }
      if( ! m_pcIntFrameTopField || ! m_pcIntFrameBotField )
      {
          setPoc( min( m_iTopFieldPoc, m_iBotFieldPoc ) ); // Dong: max -> min. Output correct POC.
      }
  }

  IntFrame* getPic( PicType ePicType);
  ErrVal addFrameFieldBuffer();//th 
  ErrVal removeFieldBuffers();//th
  ErrVal removeFrameFieldBuffer();

  ErrVal addFieldBuffer( PicType ePicType);//th 
  ErrVal removeFieldBuffer( PicType ePicType);//th

  IntFrame* getTopField() { AOT( FRAME != m_ePicType); AOT(NULL == m_pcIntFrameTopField); return m_pcIntFrameTopField; } //th
  IntFrame* getBotField() { AOT( FRAME != m_ePicType); AOT(NULL == m_pcIntFrameBotField); return m_pcIntFrameBotField; } //th

  Void clearPicStat()//lufeng
  {
    m_ePicStat=NOT_SPECIFIED;
  }
  Void updatePicStat(PicType ePicType)//lufeng
  {
	  if(m_ePicStat==FRAME)return;
	  if(ePicType==m_ePicStat)return;
	  if(ePicType==FRAME)m_ePicStat=FRAME;
	  else if(m_ePicStat==NOT_SPECIFIED)
	  {
		m_ePicStat=ePicType;
	  }
	  else
	  {
		m_ePicStat=FRAME;
	  }

  }
  Bool isPicReady(PicType ePicType)//lufeng
  {
	  if(ePicType==FRAME)return (m_ePicStat==FRAME);
	  else if(m_ePicStat==FRAME)return true;
	  return (m_ePicStat==ePicType);
  }

  UInt  getViewId()      const   { return m_uiViewId; }
  Void  setViewId( UInt uiViewId ) { m_uiViewId = uiViewId; }


  // JVT-R057 LA-RDO{
  Void   initChannelDistortion();
  Void   uninitChannelDistortion()  { 
	  if(m_piChannelDistortion) 
		  delete[] m_piChannelDistortion; 
  }
  UInt*   getChannelDistortion()   { return  m_piChannelDistortion;}
  Void   copyChannelDistortion(IntFrame*p1);
  Void   zeroChannelDistortion();
  Void   setChannelDistortion(IntFrame*p1) { if(p1) m_piChannelDistortion=p1->m_piChannelDistortion; else m_piChannelDistortion=NULL;}
  // JVT-R057 LA-RDO}  

	PicType getPicType()              const { return m_ePicType; }
	Void setPicType(PicType e)       { m_ePicType = e; }

protected:
  IntYuvPicBuffer m_cFullPelYuvBuffer;
  IntYuvPicBuffer m_cHalfPelYuvBuffer;
  
  Int             m_iPOC;
  Bool            m_bHalfPel;
  Bool            m_bExtended;

  DPBUnit*        m_pcDPBUnit;

  	Bool            m_bPocIsSet;
    Int             m_iTopFieldPoc;
    Int             m_iBotFieldPoc;
  PicType         m_ePicType;

  IntFrame* m_pcIntFrameTopField;//th
  IntFrame* m_pcIntFrameBotField;//th

  PicType       m_ePicStat;//lufeng

  Bool			  m_bUnusedForRef; // JVT-Q065 EIDR
  // JVT-R057 LA-RDO{
  UInt*            m_piChannelDistortion;
  // JVT-R057 LA-RDO}

    Int			  m_iFrameNum; //JVT-S036 
  UInt            m_uiViewId;

};


H264AVCCOMMONLIB_API extern __inline ErrVal gSetFrameFieldLists ( RefFrameList& rcTopFieldList, RefFrameList& rcBotFieldList, RefFrameList& rcRefFrameList )
{
    ROTRS( NULL == &rcRefFrameList, Err::m_nOK );

    rcTopFieldList.reset();
    rcBotFieldList.reset();

    const Int iMaxEntries = min( rcRefFrameList.getSize(), rcRefFrameList.getActive() );
    for( Int iFrmIdx = 0; iFrmIdx < iMaxEntries; iFrmIdx++ )
    {
        IntFrame* pcTopField = rcRefFrameList.getEntry( iFrmIdx )->getPic( TOP_FIELD );
        IntFrame* pcBotField = rcRefFrameList.getEntry( iFrmIdx )->getPic( BOT_FIELD );
        rcTopFieldList.add( pcTopField );
        rcTopFieldList.add( pcBotField );
        rcBotFieldList.add( pcBotField );
        rcBotFieldList.add( pcTopField );
    }

    return Err::m_nOK;
}



H264AVCCOMMONLIB_API extern __inline ErrVal gSetFrameFieldArrays( IntFrame* apcFrame[4], IntFrame* pcFrame )
{
    if( pcFrame == NULL )
    {
        apcFrame[0] = NULL;
        apcFrame[1] = NULL;
        apcFrame[2] = NULL;
        apcFrame[3] = NULL;
    }
    else
    {
        RNOK( pcFrame->addFrameFieldBuffer() );
        apcFrame[0] = pcFrame->getPic( TOP_FIELD );
        apcFrame[1] = pcFrame->getPic( BOT_FIELD );
        apcFrame[2] = pcFrame->getPic( FRAME     );
        apcFrame[3] = pcFrame->getPic( FRAME     );
    }
    return Err::m_nOK;
}


H264AVC_NAMESPACE_END


#endif // !defined(AFX_INTFRAME_H__98AFB9AC_5EE3_45A9_B09B_859511AC9090__INCLUDED_)

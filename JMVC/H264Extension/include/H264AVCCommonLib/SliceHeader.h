#if !defined(AFX_SLICEHEADER_H__G31F1842_FFCD_42AD_A981_7BD2736A4431__INCLUDED_)
#define AFX_SLICEHEADER_H__G31F1842_FFCD_42AD_A981_7BD2736A4431__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/SliceHeaderBase.h"



H264AVC_NAMESPACE_BEGIN


class FrameUnit;


#if defined( WIN32 )
# pragma warning( disable: 4275 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API SliceHeader
: public SliceHeaderBase
, protected CostData
{
public:
	SliceHeader         ( const SequenceParameterSet& rcSPS,
                        const PictureParameterSet&  rcPPS );
	virtual ~SliceHeader();

    Void getMbPositionFromAddress( UInt& ruiMbY, UInt& ruiMbX, const UInt uiMbAddress ) const; 
    Void getMbPositionFromAddress( UInt& ruiMbY, UInt& ruiMbX, UInt& ruiMbIndex, const UInt uiMbAddress ) const ;
    UInt getMbIndexFromAddress( UInt uiMbAddress ) const; //th

  ErrVal  compare     ( const SliceHeader*          pcSH,
      Bool&                       rbNewPic,
      Bool&                       rbNewFrame ) const;

// JVT-Q054 Red. Picture {
  ErrVal  compareRedPic     ( const SliceHeader*          pcSH,
                              Bool&                       rbNewFrame ) const;
  ErrVal  sliceHeaderBackup ( SliceHeader*                pcSH       );
// JVT-Q054 Red. Picture }


  Bool    isIntra     ()  const   { return m_eSliceType == I_SLICE; }
  Bool    isInterP    ()  const   { return m_eSliceType == P_SLICE; }
  Bool    isInterB    ()  const   { return m_eSliceType == B_SLICE; }

  Bool    isMbAff     ()  const   { return ( ! getFieldPicFlag() && getSPS().getMbAdaptiveFrameFieldFlag() ); }  // for future use

  const RefPicList<RefPic>& getRefPicList( PicType ePicType, ListIdx eListIdx ) const
  {
      return m_aacRefPicList[ePicType-1][eListIdx];
  }
  RefPicList<RefPic>& getRefPicList( PicType ePicType, ListIdx eListIdx )
  {
      return m_aacRefPicList[ePicType-1][eListIdx];
  }
  UInt  getRefListSize( ListIdx eListIdx ) const
  {
      return m_aacRefPicList[getPicType()-1][eListIdx].size();
  }
  const RefPic& getRefPic( UInt uiFrameId, PicType ePicType, ListIdx eLstIdx ) const
  {
      uiFrameId--;
      AOT_DBG( eLstIdx > 2 );
      return m_aacRefPicList[ePicType-1][eLstIdx].get( uiFrameId );
  }
  Void  setRefFrameList ( RefFrameList* pc,
      PicType       ePicType,
      ListIdx       eListIdx  )  
  { 
	  m_aapcRefFrameList[ePicType-1][eListIdx]  = pc; 
  }
// ying spatial direct, march 17th 
  Void  setList1FirstShortTerm ( Bool b )
  {
    m_bList1ShortTerm =     b;
  }
  Bool getList1FirstShortTerm ( )
  {
    return m_bList1ShortTerm;
  }

  Void  setTopFieldPoc  ( Int           i  )  { m_iTopFieldPoc        = i;  }
  Void  setBotFieldPoc  ( Int           i  )  { m_iBotFieldPoc        = i;  }


  Void  setPoc          ( Int           i  )  { m_iPoc                = i; }
  Void  setLastMbInSlice( UInt          ui )  { m_uiLastMbInSlice     = ui; }
  Void  setFrameUnit    ( FrameUnit*    pc )  { m_pcFrameUnit         = pc; }
  Void  setRefFrameList ( RefFrameList* pc,
                          ListIdx       e  )  { m_apcRefFrameList[e]  = pc; }

  Int             getTopFieldPoc        ()                    const { return m_iTopFieldPoc; }
  Int             getBotFieldPoc        ()                    const { return m_iBotFieldPoc; }
  Int             getPoc            ()                    const { return ( m_bFieldPicFlag ? ( m_bBottomFieldFlag ? m_iBotFieldPoc : m_iTopFieldPoc ) : min( m_iTopFieldPoc, m_iBotFieldPoc ) ); }
  Int             getPoc            ( PicType ePicType )  const { return ( ePicType == FRAME ? min( m_iTopFieldPoc, m_iBotFieldPoc ) : ePicType == BOT_FIELD ? m_iBotFieldPoc : m_iTopFieldPoc ); }
  Void             setPoc            ( UInt poc, PicType ePicType ) 
                   { 
					   if( ePicType == BOT_FIELD )
					   { m_iBotFieldPoc=poc; }
					   else if(ePicType == TOP_FIELD )
					   {m_iTopFieldPoc=poc ; }
                    }
  RefFrameList*   getRefFrameList       ( PicType ePicType,
      ListIdx eLstIdx )   const { return m_aapcRefFrameList[ePicType-1][eLstIdx]; }

  UInt            getLastMbInSlice      ()                    const { return m_uiLastMbInSlice; }
  FrameUnit*      getFrameUnit          ()                    const { return m_pcFrameUnit; }
  FrameUnit*      getFrameUnit          ()                          { return m_pcFrameUnit; }
  RefFrameList*   getRefFrameList       ( ListIdx eLstIdx )   const { return m_apcRefFrameList[eLstIdx]; }
  CostData&       getCostData           ()                          { return *this; }
  const CostData& getCostData           ()                    const { return *this; }
  UChar           getChromaQp           ( UChar   ucLumaQp )  const { return g_aucChromaScale[ gClipMinMax( ucLumaQp + getPPS().getChomaQpIndexOffset(), 0, 51 ) ];}
  const Bool      isScalingMatrixPresent( UInt    uiMatrix )  const { return NULL != m_acScalingMatrix.get( uiMatrix ); }
  const UChar*    getScalingMatrix      ( UInt    uiMatrix )  const { return m_acScalingMatrix.get( uiMatrix ); }
  
  Int             getDistScaleFactor    ( PicType eMbPicType,
      SChar   sL0RefIdx,
      SChar   sL1RefIdx ) const;
  Int             getDistScaleFactorScal( PicType eMbPicType,
      SChar   sL0RefIdx,
      SChar   sL1RefIdx ) const;

  //	TMM_EC {{
  Int             getDistScaleFactorVirtual( PicType eMbPicType,
      SChar   sL0RefIdx,
      SChar   sL1RefIdx,
      RefFrameList& rcRefFrameListL0, 
      RefFrameList& rcRefFrameListL1 ) const;
  //  TMM_EC }}

  Int             getDistScaleFactorWP  ( const Frame*    pcFrameL0, const Frame*     pcFrameL1 )  const;
  Int             getDistScaleFactorWP  ( const IntFrame* pcFrameL0, const IntFrame*  pcFrameL1 )  const;
  Void            setFGSCodingMode      ( Bool b  )            { m_bFGSCodingMode = b;     }
  Void            setGroupingSize       ( UInt ui )            { m_uiGroupingSize = ui;    }
  Void            setPosVect            ( UInt ui, UInt uiVal) { m_uiPosVect[ui]  = uiVal; }
  Bool            getFGSCodingMode      ()                     { return m_bFGSCodingMode;  }
  UInt            getGroupingSize       ()                     { return m_uiGroupingSize;  }
  UInt            getPosVect            ( UInt ui )            { return m_uiPosVect[ui];   }

protected:
  ErrVal          xInitScalingMatrix    ();


protected:
    RefPicList<RefPic>      m_aacRefPicList[3][2];
    RefFrameList*           m_aapcRefFrameList[3][2];
    Int                     m_iTopFieldPoc;
    Int                     m_iBotFieldPoc;
    RefPicList<RefPic>      m_acRefPicList[2];
    Int                     m_iPoc;
  UInt                    m_uiLastMbInSlice;
  FrameUnit*              m_pcFrameUnit;
  StatBuf<const UChar*,8> m_acScalingMatrix;
  RefFrameList*           m_apcRefFrameList[2];
  Bool                    m_bFGSCodingMode;
  UInt                    m_uiGroupingSize;
  UInt                    m_uiPosVect[16];
//  ying Spatial Direct
  Bool                    m_bList1ShortTerm;
};


#if defined( WIN32 )
# pragma warning( default: 4251 )
# pragma warning( default: 4275 )
#endif


typedef SliceHeader::DeblockingFilterParameter DFP;



H264AVC_NAMESPACE_END


#endif // !defined(AFX_SLICEHEADER_H__G31F1842_FFCD_42AD_A981_7BD2736A4431__INCLUDED_)

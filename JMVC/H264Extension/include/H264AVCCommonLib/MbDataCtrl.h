#if !defined(AFX_MBDATACTRL_H__50D2B462_28AB_46CA_86AC_35502BD296BC__INCLUDED_)
#define AFX_MBDATACTRL_H__50D2B462_28AB_46CA_86AC_35502BD296BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif








class H264AVCCOMMONLIB_API MbDataCtrl
{
public:
	MbDataCtrl();
  ~MbDataCtrl();

public:
  ErrVal getBoundaryMask( Int iMbY, Int iMbX, UInt& ruiMask ) const ;
  ErrVal initMb( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, const Int iForceQp = -1 );
  ErrVal initMb( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, const Bool bFieldFlag, const Int iForceQp );
  ErrVal init( const SequenceParameterSet& rcSPS );
//	TMM_EC {{
  ErrVal initMbTDEnhance( MbDataAccess*& rpcMbDataAccess, MbDataCtrl *pcMbDataCtrl, MbDataCtrl *pcMbDataCtrlRef, UInt uiMbY, UInt uiMbX, const Int iForceQp = -1 );
//  TMM_EC }}

  ErrVal uninit();
  ErrVal reset();
  ErrVal initSlice( SliceHeader& rcSH, ProcessingState eProcessingState, Bool bDecoder, MbDataCtrl* pcMbDataCtrl );

  Bool isPicDone( const SliceHeader& rcSH );
  Bool isFrameDone( const SliceHeader& rcSH );
  Bool isInitDone() {return m_bInitDone;}

  UInt  getSize() { return m_uiSize; }

  const MbData& getMbData( UInt uiIndex )   const { AOT_DBG( uiIndex >= m_uiSize );  return m_pcMbData[ uiIndex ]; }
  const Bool isPicCodedField( )              const { return m_bPicCodedField; }

  MbData& getMbDataByIndex( UInt uiIndex )        { AOT_DBG( uiIndex >= m_uiSize );  return m_pcMbData[ uiIndex ]; }

  ErrVal saveQpAndCbp( MbDataCtrl *pSrcMbDataCtrl )
  {
    for( UInt uiMbIdx = 0; uiMbIdx < getSize(); uiMbIdx++ )
    {
      getMbDataByIndex( uiMbIdx ).setQp      ( pSrcMbDataCtrl->getMbData( uiMbIdx ).getQp() );
      getMbDataByIndex( uiMbIdx ).setTransformSize8x8 ( pSrcMbDataCtrl->getMbData( uiMbIdx ).isTransformSize8x8() );
      getMbDataByIndex( uiMbIdx ).setMbExtCbp( pSrcMbDataCtrl->getMbData( uiMbIdx ).getMbExtCbp() );
    }

    return Err::m_nOK;
  }

  ErrVal clear() { return xResetData(); }

  MbData& getMbData( UInt uiMbX, UInt uiMbY )   { AOT_DBG( uiMbY*m_uiMbStride+uiMbX+m_uiMbOffset >= m_uiSize );  return m_pcMbData[uiMbY*m_uiMbStride+uiMbX+m_uiMbOffset]; }
//	TMM_EC {{
	MbData& getMbData( UInt uiMbX, UInt uiMbY ) const  { AOT_DBG( uiMbY*m_uiMbStride+uiMbX+m_uiMbOffset >= m_uiSize );  return m_pcMbData[uiMbY*m_uiMbStride+uiMbX+m_uiMbOffset]; }
//  TMM_EC }}

  ErrVal        switchMotionRefinement();

  ErrVal        copyMotion    ( MbDataCtrl& rcMbDataCtrl );
	// TMM_ESS {
  ErrVal        copyMotionBL  ( MbDataCtrl& rcMbDataCtrl, ResizeParameters* pcParameters  );
  ErrVal        upsampleMotion ( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
	// TMM_ESS }
//JVT-T054{
  ErrVal        copyMbCBP( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
  ErrVal        initMbCBP( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
//JVT-T054}
  //--ICU/ETRI FMO Implementation
  const Int getSliceGroupIDofMb(Int mb);

  ErrVal        initFgsBQData             ( UInt uiNumMb );
  ErrVal        uninitFgsBQData           ();
  ErrVal        storeFgsBQLayerQpAndCbp   ();
  ErrVal        switchFgsBQLayerQpAndCbp  ();

//TMM_WP
  SliceHeader*  getSliceHeader      ()  { return  m_pcSliceHeader;      }
//TMM_WP

  // ICU/ETRI FGS_MOT_USE
	Bool xGetDirect8x8InferenceFlagPublic() { return m_bDirect8x8InferenceFlag; }
  Void xSetDirect8x8InferenceFlag(Bool b) { m_bDirect8x8InferenceFlag = b; }

  UInt GetMbStride() { return m_uiMbStride; }
  Void SetMbStride(UInt Stride) { m_uiMbStride = Stride; }  

  // JVT-S054 (ADD)
  UInt getSliceId() const { return m_uiSliceId;}

protected:
  const MbData& xGetOutMbData()            const { return m_pcMbData[m_uiSize]; }
  const MbData& xGetRefMbData( UInt uiSliceId, Int uiCurrSliceID, Int iMbY, Int iMbX, Bool bLoopFilter ); 
  const MbData& xGetColMbData( UInt uiIndex );

  ErrVal xCreateData( UInt uiSize );
  ErrVal xDeleteData();
  ErrVal xResetData();

  Bool xGetDirect8x8InferenceFlag() { return m_bDirect8x8InferenceFlag; }

// TMM_ESS_UNIFIED {
  ErrVal  xUpsampleMotionDyad(MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
  ErrVal  xUpsampleMotionESS(MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
// TMM_ESS_UNIFIED }

protected:
  DynBuf<DFP*>        m_cpDFPBuffer;
  MbTransformCoeffs*  m_pcMbTCoeffs;
  MbMotionData*       m_apcMbMotionData[2];
  MbMotionData*       m_apcMbMotionDataBase[2];
  MbMvData*           m_apcMbMvdData[2];
  MbData*             m_pcMbData;
  MbDataAccess*       m_pcMbDataAccess;
  SliceHeader*        m_pcSliceHeader;
  UChar               m_ucLastMbQp;
  UInt                m_uiMbStride;
  UInt                m_uiMbOffset;
  Int                 m_iMbPerLine;
  Int                 m_iMbPerColumn;
  UInt                m_uiSize;
  UInt                m_uiMbProcessed;
  UInt                m_uiSliceId;
  Int                 m_iColocatedOffset;
  ProcessingState     m_eProcessingState;
  const MbDataCtrl*   m_pcMbDataCtrl0L1;
  Bool                m_bUseTopField;
  Bool                m_bPicCodedField;
  Bool                m_bInitDone;
  Bool                m_bDirect8x8InferenceFlag;
  UChar*        m_pacFgsBQMbQP;
  UInt*         m_pauiFgsBQMbCbp;
  UInt*         m_pauiFgsBQBCBP;
  Bool*         m_pabFgsBQ8x8Trafo;

};



class ControlData
{
public:
  ControlData   ();
  ~ControlData  ();

  Void          clear               ();
  ErrVal        init                ( SliceHeader*  pcSliceHeader,
                                      MbDataCtrl*   pcMbDataCtrl,
                                      Double        dLambda );
  ErrVal        init                ( SliceHeader*  pcSliceHeader );

  Double        getLambda           ()  { return  m_dLambda;            }
  MbDataCtrl*   getMbDataCtrl       ()  { return  m_pcMbDataCtrl;       }
  Bool          isInitialized       ()  { return  m_pcMbDataCtrl != 0;  }

  ErrVal        setMbDataCtrl       ( MbDataCtrl* pcMbDataCtrl )
  {
    m_pcMbDataCtrl = pcMbDataCtrl;
    return Err::m_nOK;
  }

  	SliceHeader*  getSliceHeader      ( PicType ePicType = FRAME ) { return ( ePicType==BOT_FIELD ) ? m_pcSliceHeaderBot : m_pcSliceHeader;	}
    ErrVal        setSliceHeader      ( SliceHeader* pcSliceHeader,
        PicType      ePicType = FRAME )
    {
        if( ePicType==BOT_FIELD )
        {
            m_pcSliceHeaderBot = pcSliceHeader;
        }
        else
        {
            m_pcSliceHeader = pcSliceHeader;
        }

        return Err::m_nOK;
    }

  ErrVal        activateMbDataCtrlForQpAndCbp( Bool bNormalMbDataCtrl )
  {
    // restore the state first
    if( m_bIsNormalMbDataCtrl != bNormalMbDataCtrl )
    {
      switchFGSLayerQpAndCbp();
    }
    m_bIsNormalMbDataCtrl = bNormalMbDataCtrl;

    return Err::m_nOK;
  }

  ErrVal        saveMbDataQpAndCbp()
  {
    // should not happen, not designed so general
    ROT( ! m_bIsNormalMbDataCtrl );
    storeFGSLayerQpAndCbp();
    return Err::m_nOK;
  }

  IntFrame*     getBaseLayerRec     ()  { return  m_pcBaseLayerRec;     }
  IntFrame*     getBaseLayerSbb     ()  { return  m_pcBaseLayerSbb;     }
  MbDataCtrl*   getBaseLayerCtrl    ()  { return  m_pcBaseLayerCtrl;    }
  ControlData*  getBaseCtrlData     ()  { return  m_pcBaseCtrlData;     }
  UInt          getUseBLMotion      ()  { return  m_uiUseBLMotion;      }
  
  Void          setBaseLayerRec     ( IntFrame*   pcBaseLayerRec  )   { m_pcBaseLayerRec    = pcBaseLayerRec;   }
  Void          setBaseLayerSbb     ( IntFrame*   pcBaseLayerSbb  )   { m_pcBaseLayerSbb    = pcBaseLayerSbb;   }
  Void          setBaseLayerCtrl    ( MbDataCtrl* pcBaseLayerCtrl )   { m_pcBaseLayerCtrl   = pcBaseLayerCtrl;  }
  Void          setBaseCtrlData     ( ControlData*pcBaseCtrlData  )   { m_pcBaseCtrlData    = pcBaseCtrlData;   }
  Void          setUseBLMotion      ( UInt        uiUseBLMotion   )   { m_uiUseBLMotion     = uiUseBLMotion;    }

  Void          setLambda           ( Double d ) { m_dLambda = d; }

  Void          setScalingFactor    ( Double  d ) { m_dScalingFactor      = d; }
  Double        getScalingFactor    ()  const     { return m_dScalingFactor;      }

  Void          setBaseLayer        ( UInt  uiBaseLayerId, UInt  uiBaseLayerIdMotion )
  {
    m_uiBaseLayerId = uiBaseLayerId; m_uiBaseLayerIdMotion = uiBaseLayerIdMotion; 
  }

  UInt          getBaseLayerId    () { return m_uiBaseLayerId; }
  UInt          getBaseLayerIdMotion()  { return m_uiBaseLayerIdMotion; }

// TMM_ESS {
  Void          setSpatialScalabilityType ( int iSST )   { m_iSpatialScalabilityType = iSST; }
  Int           getSpatialScalabilityType ()             { return m_iSpatialScalabilityType; }
  Void          setSpatialScalability ( Bool b )         { m_bSpatialScalability = b; }
  Bool          getSpatialScalability ()                 { return m_bSpatialScalability; }
// TMM_ESS }


  RefFrameList& getPrdFrameList     ( UInt uiList )   { return m_acPrdFrameList          [uiList]; }
  
  ErrVal        initFGSData             ( UInt uiNumMb );
  ErrVal        uninitFGSData           ();
  ErrVal        storeFGSLayerQpAndCbp   ();
  ErrVal        switchFGSLayerQpAndCbp  ();

  ErrVal        initBQData            ( UInt uiNumMb );
  ErrVal        uninitBQData          ();
  ErrVal        storeBQLayerQpAndCbp  ();
  ErrVal        switchBQLayerQpAndCbp ();

private:
  MbDataCtrl*   m_pcMbDataCtrl;
  SliceHeader*  m_pcSliceHeader;
   SliceHeader*  m_pcSliceHeaderBot;

  Double        m_dLambda;

  IntFrame*     m_pcBaseLayerRec;
  IntFrame*     m_pcBaseLayerSbb;
  MbDataCtrl*   m_pcBaseLayerCtrl;
  ControlData*  m_pcBaseCtrlData;
  UInt          m_uiUseBLMotion;

  Double        m_dScalingFactor;

  UInt          m_uiBaseLayerId;
  UInt          m_uiBaseLayerIdMotion;

  Int           m_iSpatialScalabilityType; // TMM_ESS
  Bool          m_bSpatialScalability;     // TMM_ESS  

  
  UChar*        m_pacFGSMbQP;
  UInt*         m_pauiFGSMbCbp;
  Bool*         m_pabFGS8x8Trafo;
  Bool          m_bIsNormalMbDataCtrl;

  //===== base quality data (CBP, QP, 8x8flag) =====
  UChar*        m_pacBQMbQP;
  UInt*         m_pauiBQMbCbp;
  Bool*         m_pabBQ8x8Trafo;
  MbMode*       m_paeBQMbMode;
  UShort*       m_pusBQFwdBwd;
  MbMotionData* m_paacBQMotionData[2];

  RefFrameList  m_acPrdFrameList[2];

};






#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif



H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBDATACTRL_H__50D2B462_28AB_46CA_86AC_35502BD296BC__INCLUDED_)

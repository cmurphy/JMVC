#define _ABT_FLAG_IN_SLICE_HEADER_
#include "H264AVCDecoderLib.h"

#include "ControlMngH264AVCDecoder.h"
#include "H264AVCCommonLib/TraceFile.h"

#include "MbParser.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/ParameterSetMng.h"

#include "SliceReader.h"
#include "DecError.h"

#include "H264AVCCommonLib/CFMO.h"

H264AVC_NAMESPACE_BEGIN


SliceReader::SliceReader():
  m_pcHeaderReadIf( NULL ),
  m_pcParameterSetMng(NULL),
  m_pcMbParser( NULL ),
  m_pcControlMng( NULL ),
  m_bInitDone( false)
{
}


SliceReader::~SliceReader()
{
}


ErrVal SliceReader::create( SliceReader*& rpcSliceReader )
{
  rpcSliceReader = new SliceReader;

  ROT( NULL == rpcSliceReader );

  return Err::m_nOK;
}


ErrVal SliceReader::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal SliceReader::init( HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                          ParameterSetMng* pcParameterSetMng,
                          MbParser* pcMbParser,
                          ControlMngIf* pcControlMng )
{
  ROT( m_bInitDone );
  ROT( NULL == pcHeaderSymbolReadIf );
  ROT( NULL == pcMbParser );
  ROT( NULL == pcControlMng );


  m_pcParameterSetMng = pcParameterSetMng;
  m_pcControlMng      = pcControlMng;
  m_pcHeaderReadIf    = pcHeaderSymbolReadIf;
  m_pcMbParser        = pcMbParser;

  m_bInitDone = true;
  return Err::m_nOK;
}


ErrVal SliceReader::uninit()
{
  ROF( m_bInitDone );
  m_pcHeaderReadIf = NULL;
  m_pcMbParser = NULL;
  m_bInitDone = false;

  return Err::m_nOK;
}


// JVT-S054 (2) (REPLACE)
//ErrVal SliceReader::process( const SliceHeader& rcSH, UInt& ruiMbRead )
ErrVal SliceReader::process( SliceHeader& rcSH, UInt& ruiMbRead )
{
  int sgId = rcSH.getFMO()->getSliceGroupId(rcSH.getFirstMbInSlice());  
  int pocOrder = rcSH.getPicOrderCntLsb();
  rcSH.getFMO()->setCodedSG(sgId, pocOrder);  

  ROF( m_bInitDone );

  //====== initialization ======
  UInt  uiMbAddress       = rcSH.getFirstMbInSlice();
  //UInt  uiLastMbAddress   = rcSH.getMbInPic()-1;
  Bool  bEndOfSlice       = false;

  //===== loop over macroblocks =====
  for(  ruiMbRead = 0;uiMbAddress!=UInt(-1)&&!bEndOfSlice;ruiMbRead++ )//!bEndOfSlice; uiMbAddress++ )
  {
    DTRACE_NEWMB( uiMbAddress );
    MbDataAccess* pcMbDataAccess;

    RNOK( m_pcControlMng->initMbForParsing( pcMbDataAccess, rcSH.getMbIndexFromAddress( uiMbAddress )  ) );
    if( rcSH.isMbAff() && uiMbAddress % 2 == 0 ) // for future use
    {
        pcMbDataAccess->setFieldMode( pcMbDataAccess->getDefaultFieldFlag() );
    }
    rcSH.setLastMbInSlice( uiMbAddress );
    pcMbDataAccess->getMbData().deactivateMotionRefinement();

	if( rcSH.isMbAff() && uiMbAddress % 2 == 0 )
    {
      pcMbDataAccess->setFieldMode( pcMbDataAccess->getDefaultFieldFlag() );
    }

    DECRNOK( m_pcMbParser->process( *pcMbDataAccess, bEndOfSlice) );

    // JVT-S054 (2) (ADD)
    rcSH.setLastMbInSlice(uiMbAddress);
    //--ICU/ETRI FMO Implementation
    uiMbAddress  = rcSH.getFMO()->getNextMBNr(uiMbAddress ); 

  }
  // JVT-S054 (2) (ADD)
  rcSH.setNumMbsInSlice(ruiMbRead);

  return Err::m_nOK;
}







ErrVal  SliceReader::read( SliceHeader&   rcSH,
                           MbDataCtrl*    pcMbDataCtrl,
                           MbDataCtrl*    pcMbDataCtrlBase,
                           Int            iSpatialScalabilityType,
                           UInt           uiMbInRow,
                           UInt&          ruiMbRead )
{
  ROF( m_bInitDone );

  UInt  uiMbAddress   = rcSH.getFirstMbInSlice();
  UInt  uiNumMbInPic  = rcSH.getSPS().getMbInFrame();
  Bool  bEndOfSlice   = false;

  RNOK( pcMbDataCtrl->initSlice( rcSH, PARSE_PROCESS, true, NULL ) );

  //===== loop over macroblocks =====
  for( ruiMbRead = 0; !bEndOfSlice; ) //--ICU/ETRI FMO Implementation  
  {
    DTRACE_NEWMB( uiMbAddress );

    UInt          uiMbY               = uiMbAddress / uiMbInRow;
    UInt          uiMbX               = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;
    Bool          bCropWindowFlag     = pcMbDataCtrl->getMbData( uiMbX, uiMbY ).getInCropWindowFlag();

    RNOK( pcMbDataCtrl        ->initMb    ( pcMbDataAccess,     uiMbY, uiMbX ) );
    pcMbDataAccess->getMbData().deactivateMotionRefinement();
    pcMbDataAccess->getMbData().setInCropWindowFlag( bCropWindowFlag );

    if  ( pcMbDataCtrlBase )
    {
      RNOK( pcMbDataCtrlBase  ->initMb    ( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    RNOK( m_pcMbParser        ->read      ( *pcMbDataAccess,
                                            pcMbDataAccessBase,
                                            iSpatialScalabilityType,
                                            bEndOfSlice  ) );
    ruiMbRead++;
	  if(ruiMbRead == uiNumMbInPic) bEndOfSlice = true; //FRAG_FIX
    // JVT-S054 (2) (ADD)
    rcSH.setLastMbInSlice(uiMbAddress);
    //--ICU/ETRI FMO Implementation
    uiMbAddress  = rcSH.getFMO()->getNextMBNr(uiMbAddress ); 

  }

  // JVT-S054 (2) (ADD)
  rcSH.setNumMbsInSlice(ruiMbRead);

  //ICU/ETRI FGS FMO
  int sgId = rcSH.getFMO()->getSliceGroupId(rcSH.getFirstMbInSlice());  
  int pocOrder = rcSH.getPicOrderCntLsb();

  rcSH.getFMO()->setCodedSG(sgId, pocOrder);  

  //--ICU/ETRI FMO Implementation
  // JVT-S054 (REMOVE)
  //ROF( ruiMbRead == rcSH.getNumMbInSlice());

  return Err::m_nOK;
}




//TMM_EC {{
ErrVal  SliceReader::readVirtual( SliceHeader&   rcSH,
																	MbDataCtrl*    pcMbDataCtrl,
																	MbDataCtrl*    pcMbDataCtrlRef,
																	MbDataCtrl*    pcMbDataCtrlBase,
																	Int            iSpatialScalabilityType,
																	UInt           uiMbInRow,
																	UInt&          ruiMbRead,
																	ERROR_CONCEAL			m_eErrorConceal)
{
  ROF( m_bInitDone );

  UInt  uiMbAddress   = rcSH.getFirstMbInSlice();
  UInt  uiNumMbInPic  = rcSH.getSPS().getMbInFrame();
  Bool  bEndOfSlice   = false;

  RNOK( pcMbDataCtrl->initSlice( rcSH, PARSE_PROCESS, true, NULL ) );

  //===== loop over macroblocks =====
	for( ruiMbRead = 0; ruiMbRead < uiNumMbInPic; uiMbAddress++ )
  {
    DTRACE_NEWMB( uiMbAddress );

    UInt          uiMbY               = uiMbAddress / uiMbInRow;
    UInt          uiMbX               = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

		if ( rcSH.getTrueSlice() || rcSH.m_eErrorConceal != EC_TEMPORAL_DIRECT)
		{
			RNOK( pcMbDataCtrl        ->initMb    ( pcMbDataAccess,     uiMbY, uiMbX ) );
		}
		else
		{
			RNOK( pcMbDataCtrl        ->initMbTDEnhance( pcMbDataAccess, pcMbDataCtrl, pcMbDataCtrlRef, uiMbY, uiMbX ) );
		}
    pcMbDataAccess->getMbData().deactivateMotionRefinement();
		if  ( pcMbDataCtrlBase )
    {
      RNOK( pcMbDataCtrlBase  ->initMb    ( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
		RNOK( m_pcMbParser        ->readVirtual( *pcMbDataAccess,
                                            pcMbDataAccessBase,
                                            iSpatialScalabilityType,
                                            bEndOfSlice,
																						m_eErrorConceal) );
		ruiMbRead++;
  }
  ROF( ruiMbRead == uiNumMbInPic );

  return Err::m_nOK;
}
//TMM_EC }}
//TMM_EC {{
ErrVal 
SliceReader::readSliceHeaderVirtual(	NalUnitType   eNalUnitType,
																			SliceHeader	*rpcVeryFirstSliceHeader,
																			UInt	uiDecompositionStages,
																			UInt  uiMaxDecompositionStages,
																			UInt	uiGopSize,
																			UInt	uiMaxGopSize,
																			UInt	uiFrameNum,
																			UInt	uiPoc,
																			UInt	uiTemporalLevel,
																			SliceHeader*& rpcSH)
{
  SequenceParameterSet* pcSPS;
  PictureParameterSet*  pcPPS;

	UInt	uiPPSId	=	rpcVeryFirstSliceHeader->getPPS().getPicParameterSetId();

  RNOK( m_pcParameterSetMng ->get    ( pcPPS, uiPPSId) );
  RNOK( m_pcParameterSetMng ->get    ( pcSPS, pcPPS->getSeqParameterSetId() ) );

  rpcSH = new SliceHeader ( *pcSPS, *pcPPS );
  ROF( rpcSH );

  rpcSH->setNalUnitType   ( eNalUnitType    );

  if(eNalUnitType==NAL_UNIT_CODED_SLICE_SCALABLE)
  {
		rpcSH->setLayerId       ( 1       );
    rpcSH->setBaseLayerId(MSYS_UINT_MAX); // will be modified later
  }
  else
  {
    rpcSH->setLayerId(0);
		rpcSH->setBaseLayerId   ( MSYS_UINT_MAX       );
  }
  rpcSH->setTemporalLevel ( uiTemporalLevel );
  rpcSH->setQualityLevel  ( 0       );
  rpcSH->setFirstMbInSlice( 0       );
	rpcSH->setFragmentedFlag( false);

	UInt	uiMaxPocLsb		=	1 << rpcSH->getSPS().getLog2MaxPicOrderCntLsb();
	rpcSH->setFrameNum( uiFrameNum);
	rpcSH->setPicOrderCntLsb( uiPoc % uiMaxPocLsb);
    if( rpcSH->getPicType() == FRAME )
    {
        rpcSH->setTopFieldPoc( uiPoc );
        rpcSH->setBotFieldPoc( uiPoc );
    }
    else if ( rpcSH->getPicType() == TOP_FIELD )
    {
        rpcSH->setTopFieldPoc( uiPoc );
    }
    else
    {
        rpcSH->setBotFieldPoc( uiPoc );
    }
	rpcSH->setAdaptivePredictionFlag(1);
	rpcSH->setDirectSpatialMvPredFlag(false);
	rpcSH->setNumRefIdxActiveOverrideFlag( true);
	rpcSH->setNumRefIdxActive( LIST_0, 1);

	if ( rpcSH->getPicOrderCntLsb() % uiMaxGopSize == 0 || (uiGopSize - ((rpcSH->getPicOrderCntLsb() % uiMaxGopSize) >> (uiMaxDecompositionStages-uiDecompositionStages)) < (unsigned)( 1<<(uiDecompositionStages-uiTemporalLevel) ) ) )
	{
		rpcSH->setSliceType     ( P_SLICE );
		if( rpcSH->getPicOrderCntLsb() % (1<<(uiMaxDecompositionStages-uiDecompositionStages+1)) == 0)
			rpcSH->setNalRefIdc   ( NAL_REF_IDC_PRIORITY_HIGHEST);
		else
			rpcSH->setNalRefIdc     ( NAL_REF_IDC_PRIORITY_HIGH);
		rpcSH->setKeyPictureFlag( 1);
	}
  else
	{
		rpcSH->setSliceType     ( B_SLICE );
		if( rpcSH->getPicOrderCntLsb() % (1<<(uiMaxDecompositionStages-uiDecompositionStages+1)) == 0)
			rpcSH->setNalRefIdc   ( NAL_REF_IDC_PRIORITY_LOW);
		else
			rpcSH->setNalRefIdc     ( NAL_REF_IDC_PRIORITY_LOWEST);
		rpcSH->setNumRefIdxActive( LIST_1, 1);
		rpcSH->setKeyPictureFlag( 0);
	}
  //if(eNalUnitType==NAL_UNIT_CODED_SLICE||)
//key picture MMCO for base and enhancement layer
  {
		if(rpcSH->getPoc() % uiMaxGopSize == 0  || (uiGopSize - ((rpcSH->getPicOrderCntLsb() % uiMaxGopSize) >> (uiMaxDecompositionStages-uiDecompositionStages)) < (unsigned)( 1<<(uiDecompositionStages-uiTemporalLevel) ) ) )
	  {
			UInt index=rpcSH->getPoc() / uiMaxGopSize;
		  if( index>0 )rpcSH->setAdaptiveRefPicBufferingFlag(true);
      else    	   rpcSH->setAdaptiveRefPicBufferingFlag(false);

		  if(index>1)
			{
        Bool bNumber2Gop= index >2 ? true : false;
				rpcSH->setDefualtMmcoBuffer(uiDecompositionStages,bNumber2Gop);
		  }
  	  rpcSH->setSliceType(P_SLICE);
  	  rpcSH->setNalRefIdc(NAL_REF_IDC_PRIORITY_HIGHEST);
	  }

		if(rpcSH->getPoc() % uiMaxGopSize == 0  || (uiGopSize - ((rpcSH->getPicOrderCntLsb() % uiMaxGopSize) >> (uiMaxDecompositionStages-uiDecompositionStages)) < (unsigned)( 1<<(uiDecompositionStages-uiTemporalLevel) ) ) )
	  {
			UInt index=rpcSH->getPoc() / uiMaxGopSize;
		  if( index>1 )
		  {
				rpcSH->getRplrBuffer(LIST_0).setRefPicListReorderingFlag(true);
				rpcSH->getRplrBuffer(LIST_0).clear();
				rpcSH->getRplrBuffer(LIST_0).set(0,Rplr(RPLR_NEG,uiGopSize/2-1));
		  }
		  else 
		  {
			  rpcSH->getRplrBuffer(LIST_0).setRefPicListReorderingFlag(false);
		  }
  	
	  }
  }

	//weighted prediction
	RNOK( rpcSH->getPredWeightTable(LIST_0).init( 64 ) );
  RNOK( rpcSH->getPredWeightTable(LIST_1).init( 64 ) );
  
  return Err::m_nOK;
}
//TMM_EC }}

ErrVal
SliceReader::readSliceHeader( NalUnitType   eNalUnitType,
                              Bool m_svc_mvc_flag,	// this m_xxx is not good
                              Bool bNonIDRFlag,        // JVT-W035    
                              Bool          bAnchorPicFlag,
                              UInt          uiViewId,
					   		  Bool			uiInterViewFlag,  // JVT-W056  Samsung
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
							  ,Bool			UnitAVCFlag    //JVT-S036 lsj
                              )
{

  Bool                  bScalable         = ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ||
                                              eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE );
  UInt                  uiFirstMbInSlice;
  UInt                  uiSliceType;
  UInt                  uiPPSId;
  SequenceParameterSet* pcSPS;
  PictureParameterSet*  pcPPS;


  //===== read first parameters =====
  UInt uiNumMbsInSlice = 0;
  UInt uiFragOrder = 0;
  Bool bFragFlag = false;
  Bool bLastFragFlag = false;
  Bool bFGSCompSep = false;

  MmcoBuffer eMmcoBaseBuffer;

//  if(bScalable && m_view_level == 0)
  if( eNalUnitType == NAL_UNIT_CODED_SLICE_PREFIX) //JVT-W035
        return Err::m_nOK;

  RNOK( m_pcHeaderReadIf    ->getUvlc( uiFirstMbInSlice,  "SH: first_mb_in_slice" ) );
  RNOK( m_pcHeaderReadIf    ->getUvlc( uiSliceType,       "SH: slice_type" ) );
  if( uiSliceType > 4 )  // Remove "&& ! bScalable" to decode slice_type > 4
  {
      uiSliceType -= 5;
  }
  RNOK( m_pcHeaderReadIf    ->getUvlc( uiPPSId,           "SH: pic_parameter_set_id" ) );
  RNOK( m_pcParameterSetMng ->get    ( pcPPS, uiPPSId) );
  RNOK( m_pcParameterSetMng ->get    ( pcSPS, pcPPS->getSeqParameterSetId() ) );


  //===== create and initialize slice header =====
  rpcSH = new SliceHeader ( *pcSPS, *pcPPS );
  ROF( rpcSH );
  rpcSH->setNalUnitType   ( eNalUnitType    );
  rpcSH->setNalRefIdc     ( eNalRefIdc      );
  rpcSH->setAnchorPicFlag ( bAnchorPicFlag  );
  rpcSH->setViewId        ( uiViewId        );
  rpcSH->setNonIDRFlag    ( bNonIDRFlag     ); //JVT-W035 
	rpcSH->setInterViewFalg (uiInterViewFlag);   //JVT-W056  Samsung
  rpcSH->setSvcMvcFlag    ( m_svc_mvc_flag);
  rpcSH->setFirstMbInSlice( uiFirstMbInSlice);
  rpcSH->setSliceType     ( SliceType( uiSliceType ) );
  rpcSH->setNumMbsInSlice (  uiNumMbsInSlice);
  
//JVT-W035
  rpcSH->setAVCFlag       ( !bScalable  );

  rpcSH->setLayerId       ( 0       );
  rpcSH->setTemporalLevel ( 0 );
  rpcSH->setQualityLevel  ( 0  );
  rpcSH->setFragmentedFlag( bFragFlag );
  rpcSH->setFragmentOrder ( uiFragOrder );
  rpcSH->setLastFragmentFlag( bLastFragFlag );
  rpcSH->setFgsComponentSep(bFGSCompSep);
 
  //===== read remaining parameters =====
  RNOK( rpcSH->read( m_pcHeaderReadIf ) ); 


  if ( eNalUnitType == NAL_UNIT_CODED_SLICE ||
       eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
	  UnitAVCFlag = true;
	  PPSId_AVC = uiPPSId;
	  SPSId_AVC = pcPPS->getSeqParameterSetId();
	  POC_AVC = rpcSH->getPicOrderCntLsb();
  }

  //--ICU/ETRI FMO Implementation 
  rpcSH->FMOInit();
  // JVT-S054 (2) (ADD) ->
  if (uiNumMbsInSlice != 0)
    rpcSH->setLastMbInSlice(rpcSH->getFMO()->getLastMbInSlice(uiFirstMbInSlice, uiNumMbsInSlice));
  else
  // JVT-S054 (2) (ADD) <-
    rpcSH->setLastMbInSlice(rpcSH->getFMO()->getLastMBInSliceGroup(rpcSH->getFMO()->getSliceGroupId(uiFirstMbInSlice)));

  return Err::m_nOK;

}

ErrVal
SliceReader::readSliceHeader( NalUnitType   eNalUnitType,
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
                              ,Bool			UnitAVCFlag    //JVT-S036 
  )
{

  Bool                  bScalable         = ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ||
                                              eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE );

  UInt                  uiFirstMbInSlice;
  UInt                  uiSliceType;
  UInt                  uiPPSId;
  SequenceParameterSet* pcSPS;
  PictureParameterSet*  pcPPS;



  //===== read first parameters =====
  //JVT-P031
  UInt uiFragOrder = 0;
  Bool bFragFlag = false;
  Bool bLastFragFlag = false;
  UInt uiNumMbsInSlice = 0;
  Bool bFGSCompSep = false;

 
//JVT-S036  start
  Bool KeyPicFlag = false;
  Bool eAdaptiveRefPicMarkingModeFlag = false;
  MmcoBuffer eMmcoBaseBuffer;
//JVT-S036  end

  if( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE || 
      eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE       )

  {
   
	 //JVT-S036  start
	 if( uiLayerId == 0 && uiQualityLevel == 0 && UnitAVCFlag )
     {
		 RNOK( m_pcParameterSetMng ->get    ( pcPPS, PPSId_AVC ) );
         RNOK( m_pcParameterSetMng ->get    ( pcSPS, SPSId_AVC ) );

		 rpcSH = new SliceHeader ( *pcSPS, *pcPPS );
   		 ROF( rpcSH );

		 if( eNalRefIdc != 0)
		 {
			 RNOK( m_pcHeaderReadIf->getFlag( KeyPicFlag,                        "SH: key_pic_flag"));
			 if( KeyPicFlag && eNalUnitType != 21)
			 {
				 RNOK(m_pcHeaderReadIf->getFlag( eAdaptiveRefPicMarkingModeFlag,			"DRPM: adaptive_ref_pic_marking_mode_flag"));
				 if( eAdaptiveRefPicMarkingModeFlag )
				 {  
					 RNOK( rpcSH->getMmcoBaseBuffer().read( m_pcHeaderReadIf ) );
				 }		
			 }
		 }
		 rpcSH->setNalUnitType   ( eNalUnitType    );
		 rpcSH->setNalRefIdc     ( eNalRefIdc      );
		 rpcSH->setLayerId       ( uiLayerId       );
		 rpcSH->setTemporalLevel ( uiTemporalLevel );
		 rpcSH->setQualityLevel  ( uiQualityLevel  );
		 rpcSH->setKeyPicFlagScalable( KeyPicFlag      );
		 rpcSH->setAdaptiveRefPicMarkingFlag( eAdaptiveRefPicMarkingModeFlag ); 
		 rpcSH->setPicOrderCntLsb( POC_AVC );
//		 UnitAVCFlag = false;

		 return Err::m_nOK;
	 }
	 else 
	 {
//JVT-S036  end
		 // slice_header_in_scalable_extenson() begins
      RNOK( m_pcHeaderReadIf    ->getUvlc( uiFirstMbInSlice,  "SH: first_mb_in_slice" ) );
      RNOK( m_pcHeaderReadIf    ->getUvlc( uiSliceType,       "SH: slice_type" ) );
      if( uiSliceType > 4 ) // Remove "&& ! bScalable" to decode slice_type > 4
      {
        uiSliceType -= 5;
      }
      if(uiSliceType == F_SLICE)
      {
        RNOK( m_pcHeaderReadIf    ->getFlag( bFragFlag,  "SH: fgs_frag_flag" ) );
        if(bFragFlag)
        {
          RNOK( m_pcHeaderReadIf    ->getUvlc( uiFragOrder,  "SH: fgs_frag_order" ) );
          if(uiFragOrder!=0)
          {
            RNOK( m_pcHeaderReadIf    ->getFlag( bLastFragFlag,  "SH: fgs_last_frag_flag" ) );
          }
       }
		  if(uiFragOrder == 0 )
		  {
			  RNOK( m_pcHeaderReadIf    ->getUvlc( uiNumMbsInSlice,  "SH: num_mbs_in_slice" ) );
			  RNOK( m_pcHeaderReadIf    ->getFlag( bFGSCompSep,  "SH: fgs_comp_sep" ) );
		  }
	  }
	  if(uiFragOrder == 0)
      {
           RNOK( m_pcHeaderReadIf    ->getUvlc( uiPPSId,           "SH: pic_parameter_set_id" ) );
      }
      else
      {
          // Get PPS Id from first fragment 
        uiPPSId = uiFirstFragSHPPSId;
        uiNumMbsInSlice = uiFirstFragNumMbsInSlice;
        bFGSCompSep = bFirstFragFGSCompSep;
      }
      RNOK( m_pcParameterSetMng ->get    ( pcPPS, uiPPSId) );
      RNOK( m_pcParameterSetMng ->get    ( pcSPS, pcPPS->getSeqParameterSetId() ) );
	 }//JVT-S036   
  }
  else
  {

      RNOK( m_pcHeaderReadIf    ->getUvlc( uiFirstMbInSlice,  "SH: first_mb_in_slice" ) );
      RNOK( m_pcHeaderReadIf    ->getUvlc( uiSliceType,       "SH: slice_type" ) );
      if( uiSliceType > 4 ) // Remove "&& ! bScalable" to decode slice_type > 4
      {
          uiSliceType -= 5;
      }
      RNOK( m_pcHeaderReadIf    ->getUvlc( uiPPSId,           "SH: pic_parameter_set_id" ) );
      RNOK( m_pcParameterSetMng ->get    ( pcPPS, uiPPSId) );
      RNOK( m_pcParameterSetMng ->get    ( pcSPS, pcPPS->getSeqParameterSetId() ) );
  }


  //===== create and initialize slice header =====
  rpcSH = new SliceHeader ( *pcSPS, *pcPPS );
  ROF( rpcSH );

  rpcSH->setNalUnitType   ( eNalUnitType    );
  rpcSH->setNalRefIdc     ( eNalRefIdc      );
  rpcSH->setLayerId       ( uiLayerId       );
  rpcSH->setTemporalLevel ( uiTemporalLevel );
  rpcSH->setQualityLevel  ( uiQualityLevel  );
  rpcSH->setFirstMbInSlice( uiFirstMbInSlice);
  rpcSH->setSliceType     ( SliceType( uiSliceType ) );
  rpcSH->setFragmentedFlag( bFragFlag );
  rpcSH->setFragmentOrder ( uiFragOrder );
  rpcSH->setLastFragmentFlag( bLastFragFlag );
  rpcSH->setFgsComponentSep(bFGSCompSep);
  rpcSH->setNumMbsInSlice(uiNumMbsInSlice);
  //~JVT-P031
  rpcSH->setAVCFlag        ( ! bScalable); //JVT-W035
  //===== read remaining parameters =====
  if(uiFragOrder == 0) //JVT-P031
  {
      RNOK( rpcSH->read( m_pcHeaderReadIf ) ); 
  }
//JVT-S036  start
  else
  {
	  RNOK( m_pcHeaderReadIf->getFlag( KeyPicFlag,                        "SH: key_pic_flag"));
	  rpcSH->setKeyPicFlagScalable( KeyPicFlag );  //JVT-S036 
  }
  if (!rpcSH->getSPS().getFrameMbsOnlyFlag())
  {
      if ((!rpcSH->getFieldPicFlag()) && (rpcSH->getSPS().getMbAdaptiveFrameFieldFlag()))
      {
          rpcSH->setFirstMbInSlice  ( uiFirstMbInSlice << 1);
      }
  }
  if ( eNalUnitType == NAL_UNIT_CODED_SLICE ||
	   eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )

  {
	  UnitAVCFlag = true;
	  PPSId_AVC = uiPPSId;
	  SPSId_AVC = pcPPS->getSeqParameterSetId();
	  POC_AVC = rpcSH->getPicOrderCntLsb();
  }
//JVT-S036  end

  //--ICU/ETRI FMO Implementation 
  rpcSH->FMOInit();
  // JVT-S054 (2) (ADD) ->
  if (uiNumMbsInSlice != 0)
    rpcSH->setLastMbInSlice(rpcSH->getFMO()->getLastMbInSlice(uiFirstMbInSlice, uiNumMbsInSlice));
  else
  // JVT-S054 (2) (ADD) <-
    rpcSH->setLastMbInSlice(rpcSH->getFMO()->getLastMBInSliceGroup(rpcSH->getFMO()->getSliceGroupId(uiFirstMbInSlice)));

  return Err::m_nOK;
}


//JVT-S036  start
ErrVal
SliceReader::readSliceHeaderPrefix( NalUnitType   eNalUnitType,
							 Bool m_svc_mvc_flag,	 //JVT-W035						 
									NalRefIdc     eNalRefIdc,
									UInt		  uiLayerId,
									UInt		  uiQualityLevel,
									SliceHeader*  pcSliceHeader
								  )
{


  //===== read first parameters =====
  
  Bool eAdaptiveRefPicMarkingModeFlag = false;
  Bool KeyPicFlag = false;

//JVT-W035 {{ prefix 

	  if (m_svc_mvc_flag==0 && eNalUnitType == NAL_UNIT_CODED_SLICE_PREFIX)
		  return Err::m_nOK;

//JVT-W035 }} mvc prefix end
  if( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE || 
      eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE       )
  {
	  


	 if( uiLayerId == 0 && uiQualityLevel == 0 ) 
     {

		 if( eNalRefIdc != 0)
		 {
			 RNOK( m_pcHeaderReadIf->getFlag( KeyPicFlag,                        "SH: key_pic_flag"));
			 if( KeyPicFlag && eNalUnitType != 21)
			 {
				 RNOK(m_pcHeaderReadIf->getFlag( eAdaptiveRefPicMarkingModeFlag,			"DRPM: adaptive_ref_pic_marking_mode_flag"));
				 if( eAdaptiveRefPicMarkingModeFlag )
				 {  
					 RNOK( pcSliceHeader->getMmcoBaseBuffer().read( m_pcHeaderReadIf ) );
				 }		
			 }
		 }

		 pcSliceHeader->setKeyPicFlagScalable( KeyPicFlag ); //JVT-S036 
		 pcSliceHeader->setAdaptiveRefPicMarkingFlag( eAdaptiveRefPicMarkingModeFlag );

		 return Err::m_nOK;
	 }
	 else
		 return Err::m_nERR;
  }
  else
	  return Err::m_nERR;
}
//JVT-S036  end

H264AVC_NAMESPACE_END

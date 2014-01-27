#include "H264AVCDecoderLib.h"
#include "MbDecoder.h"
#include "SliceDecoder.h"
#include "H264AVCCommonLib/SliceHeader.h"
#include "DecError.h"

#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/IntFrame.h"

#include "H264AVCCommonLib/CFMO.h"

H264AVC_NAMESPACE_BEGIN


SliceDecoder::SliceDecoder():
  m_pcMbDecoder ( NULL ),
  m_pcControlMng( NULL ),
  m_pcTransform ( NULL ),
  m_bInitDone   ( false)
{
}

SliceDecoder::~SliceDecoder()
{
}

ErrVal
SliceDecoder::create( SliceDecoder*& rpcSliceDecoder )
{
  rpcSliceDecoder = new SliceDecoder;
  ROT( NULL == rpcSliceDecoder );
  return Err::m_nOK;
}


ErrVal
SliceDecoder::destroy()
{
  ROT( m_bInitDone );
  delete this;
  return Err::m_nOK;
}

ErrVal
SliceDecoder::init( MbDecoder*    pcMbDecoder,
                    ControlMngIf* pcControlMng,
                    Transform*    pcTransform)
{
  ROT( m_bInitDone );
  ROT( NULL == pcMbDecoder );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcTransform );

  m_pcTransform   = pcTransform;
  m_pcMbDecoder   = pcMbDecoder;
  m_pcControlMng  = pcControlMng;
  m_bInitDone     = true;

  return Err::m_nOK;
}


ErrVal
SliceDecoder::uninit()
{
  ROF( m_bInitDone );
  m_pcMbDecoder   =  NULL;
  m_pcControlMng  =  NULL;
  m_bInitDone     = false;
  return Err::m_nOK;
}

// TMM_EC {{
ErrVal SliceDecoder::processVirtual(const SliceHeader& rcSH, Bool bReconstructAll, UInt uiMbRead)
{
 ROF( m_bInitDone );

  //====== initialization ======
  UInt  uiMbAddress   = rcSH.getFirstMbInSlice();

  //===== loop over macroblocks =====
  for( ; uiMbRead; uiMbAddress++ )
  {
    MbDataAccess* pcMbDataAccess;
	UInt uiMbY, uiMbX, uiMbIndex;
    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbIndex, uiMbAddress );
	RNOK( m_pcControlMng->initMbForDecoding( pcMbDataAccess, uiMbY, uiMbX , rcSH.isMbAff()) );

	pcMbDataAccess->getMbData().setMbMode(MODE_SKIP);
	pcMbDataAccess->getMbData().deactivateMotionRefinement();

  	RNOK( m_pcMbDecoder->process( *pcMbDataAccess, bReconstructAll ) );
    uiMbRead--;
  }

  return Err::m_nOK;	
}
//TMM_EC }}

ErrVal
SliceDecoder::process( const SliceHeader& rcSH, Bool bReconstructAll, UInt uiMbRead )
{
  ROF( m_bInitDone );

  //====== initialization ======
  UInt  uiMbAddress         = rcSH.getFirstMbInSlice();


  //===== loop over macroblocks =====
  for( ; uiMbRead; uiMbRead--) //--ICU/ETRI FMO Implementation 
  {

    MbDataAccess* pcMbDataAccess;
	UInt uiMbY, uiMbX, uiMbIndex;


    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbIndex, uiMbAddress );
		RNOK( m_pcControlMng->initMbForDecoding( pcMbDataAccess, uiMbY, uiMbX , rcSH.isMbAff()) );


    RNOK( m_pcMbDecoder ->process( *pcMbDataAccess, bReconstructAll ) );
    //--ICU/ETRI FMO Implementation
    uiMbAddress=rcSH.getFMO()->getNextMBNr(uiMbAddress);
  }

  return Err::m_nOK;
}


ErrVal
SliceDecoder::decode( SliceHeader&   rcSH,
                      MbDataCtrl*    pcMbDataCtrl,
                      MbDataCtrl*    pcMbDataCtrlBase,
                      IntFrame*      pcFrame,
                      IntFrame*      pcResidual,
                      IntFrame*      pcPredSignal,
                      IntFrame*      pcBaseLayer,
                      IntFrame*      pcBaseLayerResidual,
                      RefFrameList*  pcRefFrameList0,
                      RefFrameList*  pcRefFrameList1,
                      Bool           bReconstructAll,
                      UInt           uiMbInRow, 
                      UInt           uiMbRead )
{
  ROF( m_bInitDone );

  //====== initialization ======
  UInt  uiMbAddress         = rcSH.getFirstMbInSlice();

  RNOK( pcMbDataCtrl->initSlice( rcSH, DECODE_PROCESS, true, NULL ) );

  //===== loop over macroblocks =====
  for( ; uiMbRead; )  //--ICU/ETRI FMO Implementation  //  for( UInt uiMbAddress = rcSH.getFirstMbInSlice(); uiMbRead; uiMbAddress++, uiMbRead-- )
  {
	UInt uiMbY, uiMbX, uiMbIndex;
    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbIndex, uiMbAddress );
    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

    RNOK( pcMbDataCtrl  ->initMb            (  pcMbDataAccess,     uiMbY, uiMbX ) );
    if( pcMbDataCtrlBase )
    {
      RNOK( pcMbDataCtrlBase->initMb        (  pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
		RNOK( m_pcControlMng->initMbForDecoding( pcMbDataAccess, uiMbY, uiMbX , rcSH.isMbAff()) );
	
    RNOK( m_pcMbDecoder ->decode            ( *pcMbDataAccess,
                                              pcMbDataAccessBase,
                                              pcFrame,
                                              pcResidual,
                                              pcPredSignal,
                                              pcBaseLayer,
                                              pcBaseLayerResidual,
                                              pcRefFrameList0,
                                              pcRefFrameList1,
                                              bReconstructAll ) );

   uiMbRead--;

//TMM_EC {{
	 if ( rcSH.getTrueSlice())
	 {
//TMM_EC }}
		//--ICU/ETRI FMO Implementation
		 uiMbAddress=rcSH.getFMO()->getNextMBNr(uiMbAddress);
	 }
	 else
	 {
		 uiMbAddress++;
	 }
  }
  return Err::m_nOK;
}

ErrVal
SliceDecoder::compensatePrediction( SliceHeader&   rcSH )
{
  ROF( m_bInitDone );

  //====== initialization ======
  RNOK( m_pcControlMng->initSlice( rcSH, DECODE_PROCESS ) );

  UInt uiFirstMbInSlice;
  UInt uiLastMbInSlice;
  FMO* pcFMO = rcSH.getFMO();  

  for(Int iSliceGroupID=0; !pcFMO->SliceGroupCompletelyCoded(iSliceGroupID); iSliceGroupID++)   
  {
  	if (false == pcFMO->isCodedSG(iSliceGroupID))
	  {
	    continue;
	  }

  	uiFirstMbInSlice = pcFMO->getFirstMacroblockInSlice(iSliceGroupID);
	  uiLastMbInSlice = pcFMO->getLastMBInSliceGroup(iSliceGroupID);
    //===== loop over macroblocks =====
    for(UInt uiMbIndex = uiFirstMbInSlice; uiMbIndex<=uiLastMbInSlice;)
    {
      MbDataAccess* pcMbDataAccess  = 0;
	UInt uiMbY, uiMbX;
	    const UInt uiMbsInRow = rcSH.getSPS().getFrameWidthInMbs();
            uiMbY = ( uiMbIndex / uiMbsInRow );
        uiMbX = ( uiMbIndex % uiMbsInRow );
		RNOK( m_pcControlMng->initMbForDecoding( pcMbDataAccess, uiMbY, uiMbX , rcSH.isMbAff()) );
      RNOK( m_pcMbDecoder ->compensatePrediction( *pcMbDataAccess            ) );

  	  uiMbIndex = rcSH.getFMO()->getNextMBNr(uiMbIndex);    
    }
  }
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END

#include "H264AVCEncoderLib.h"
#include "MbEncoder.h"
#include "SliceEncoder.h"
#include "MbCoder.h"
#include "CodingParameter.h"
#include "RecPicBuffer.h"
#include "H264AVCCommonLib/PocCalculator.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/Transform.h"

#include "H264AVCCommonLib/CFMO.h"

H264AVC_NAMESPACE_BEGIN


SliceEncoder::SliceEncoder():
  m_pcMbEncoder       ( NULL ),
  m_pcMbCoder         ( NULL ),
  m_pcControlMng      ( NULL ),
  m_pcCodingParameter ( NULL ),
  m_pcPocCalculator   ( NULL ),
  m_bInitDone         ( false ),
  m_uiFrameCount(0),
  m_eSliceType        ( I_SLICE ),
  m_bTraceEnable      ( true ),
  m_pcTransform       ( NULL )
//JVT-W080
, m_uiPdsEnable                (0)
, m_uiPdsBlockSize             (0)
, m_ppuiPdsInitialDelayMinus2L0(0)
, m_ppuiPdsInitialDelayMinus2L1(0)
//~JVT-W080
{
}


SliceEncoder::~SliceEncoder()
{
}

ErrVal SliceEncoder::create( SliceEncoder*& rpcSliceEncoder )
{
  rpcSliceEncoder = new SliceEncoder;

  ROT( NULL == rpcSliceEncoder );

  return Err::m_nOK;
}


ErrVal SliceEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal SliceEncoder::init( MbEncoder* pcMbEncoder,
                           MbCoder* pcMbCoder,
                           ControlMngIf* pcControlMng,
                           CodingParameter* pcCodingParameter,
                           PocCalculator* pcPocCalculator,
                           Transform* pcTransform)
{
  ROT( m_bInitDone );
  ROT( NULL == pcMbEncoder );
  ROT( NULL == pcMbCoder );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcPocCalculator );
  ROT( NULL == pcTransform );

  m_pcTransform = pcTransform;
  m_pcMbEncoder = pcMbEncoder;
  m_pcMbCoder = pcMbCoder;
  m_pcControlMng = pcControlMng;
  m_pcCodingParameter = pcCodingParameter;
  m_pcPocCalculator = pcPocCalculator;


  m_uiFrameCount = 0;
  m_eSliceType =  I_SLICE;

  m_bTraceEnable = true;

  m_bInitDone = true;
  return Err::m_nOK;
}


ErrVal SliceEncoder::uninit()
{
  ROF( m_bInitDone );
  m_pcMbEncoder =  NULL;
  m_pcMbCoder =  NULL;
  m_pcControlMng =  NULL;
  m_bInitDone = false;

  m_uiFrameCount = 0;
  m_eSliceType =  I_SLICE;
  m_bTraceEnable = false;
  return Err::m_nOK;
}


ErrVal
SliceEncoder::encodeSlice( SliceHeader&  rcSliceHeader,
                           IntFrame*     pcFrame,
                           MbDataCtrl*   pcMbDataCtrl,
                           RefFrameList& rcList0,
                           RefFrameList& rcList1,
                           UInt          uiMbInRow,
                           Double        dlambda )
{
  ROF( pcFrame );
  ROF( pcMbDataCtrl );

  //===== get co-located picture =====
  MbDataCtrl* pcMbDataCtrlL1 = NULL;
  if( rcList1.getActive() && rcList1.getEntry( 0 )->getRecPicBufUnit() )
  {
    pcMbDataCtrlL1 = rcList1.getEntry( 0 )->getRecPicBufUnit()->getMbDataCtrl();
  }
  ROT( rcSliceHeader.isInterB() && ! pcMbDataCtrlL1 );

  //===== initialization =====
  RNOK( pcMbDataCtrl  ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
  RNOK( m_pcControlMng->initSliceForCoding( rcSliceHeader ) );

	UInt uiPos;
	for( uiPos = 0; uiPos < rcList0.getActive(); uiPos++ )
    {
	  IntFrame* pcRefFrame = rcList0.getEntry(uiPos);
	  pcRefFrame->getFullPelYuvBuffer()->fillMargin();
	}
	for( uiPos = 0; uiPos < rcList1.getActive(); uiPos++ )
    {
	  IntFrame* pcRefFrame = rcList1.getEntry(uiPos);
	  pcRefFrame->getFullPelYuvBuffer()->fillMargin();
	}
	//lufeng:frame/field margin

//JVT-W080
	if( getPdsEnable() )
	{
		m_pcMbEncoder->setPdsEnable( getPdsEnable() );
	  m_pcMbEncoder->setFrameWidthInMbs( rcSliceHeader.getSPS().getFrameWidthInMbs() );
		m_pcMbEncoder->setPdsBlockSize( getPdsBlockSize() );
		UInt **ppuiPdsInitialDelayMinus2L0 = getPdsInitialDelayMinus2L0();
		UInt **ppuiPdsInitialDelayMinus2L1 = getPdsInitialDelayMinus2L1();
		m_pcMbEncoder->setPdsInitialDelayMinus2L0( ppuiPdsInitialDelayMinus2L0[rcSliceHeader.getViewId()] );
		m_pcMbEncoder->setPdsInitialDelayMinus2L1( ppuiPdsInitialDelayMinus2L1[rcSliceHeader.getViewId()] );
	}
//~JVT-W080
  //===== loop over macroblocks =====
  for( UInt uiMbAddress = rcSliceHeader.getFirstMbInSlice(); uiMbAddress <= rcSliceHeader.getLastMbInSlice(); uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr( uiMbAddress ) )
  {
    ETRACE_NEWMB( uiMbAddress );

    UInt          uiMbY           = uiMbAddress / uiMbInRow;
    UInt          uiMbX           = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl  ->initMb          (  pcMbDataAccess, uiMbY, uiMbX ) );
 
        RNOK( m_pcControlMng    ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX,  0,0) );

//JVT-W080
    if( getPdsEnable() )
		{
		  m_pcMbEncoder->setCurrMBX             (  uiMbX );
		  m_pcMbEncoder->setCurrMBY             (  uiMbY );
		}
//~JVT-W080

    Double cost;
    RNOK( m_pcMbEncoder ->encodeMacroblock( *pcMbDataAccess,
                                             pcFrame,
											 pcFrame,
                                             rcList0,
                                             rcList1,
                                             m_pcCodingParameter->getMotionVectorSearchParams().getNumMaxIter(),
                                             m_pcCodingParameter->getMotionVectorSearchParams().getIterSearchRange(),
                                             dlambda,
                                             cost,
											 true) );

    RNOK( m_pcMbCoder   ->encode          ( *pcMbDataAccess, NULL, SST_RATIO_1,
                                             ( uiMbAddress == rcSliceHeader.getLastMbInSlice() ) , true ) );
  }

  return Err::m_nOK;
}


/*
* lufeng: MbAff Slice encoding
*/
ErrVal
SliceEncoder::encodeSliceMbAff( SliceHeader&  rcSliceHeader,
                          IntFrame*     pcFrame,
                          MbDataCtrl*   pcMbDataCtrl,
                          RefFrameList& rcList0,
                          RefFrameList& rcList1,
                          UInt          uiMbInRow,
                          Double        dlambda )
{
    ROF( pcFrame );
    ROF( pcMbDataCtrl );



    //===== get co-located picture =====
    MbDataCtrl* pcMbDataCtrlL1 = NULL;
    if( rcList1.getActive() && rcList1.getEntry( 0 )->getRecPicBufUnit() )
    {
        pcMbDataCtrlL1 = rcList1.getEntry( 0 )->getRecPicBufUnit()->getMbDataCtrl();
    }
    ROT( rcSliceHeader.isInterB() && ! pcMbDataCtrlL1 );

    //===== initialization =====
    RNOK( pcMbDataCtrl  ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );//working...
    RNOK( m_pcControlMng->initSliceForCoding( rcSliceHeader ) );
    //JVT-W080
    if( getPdsEnable() )
    {
        m_pcMbEncoder->setPdsEnable( getPdsEnable() );
        m_pcMbEncoder->setFrameWidthInMbs( rcSliceHeader.getSPS().getFrameWidthInMbs() );
        m_pcMbEncoder->setPdsBlockSize( getPdsBlockSize() );
        UInt **ppuiPdsInitialDelayMinus2L0 = getPdsInitialDelayMinus2L0();
        UInt **ppuiPdsInitialDelayMinus2L1 = getPdsInitialDelayMinus2L1();
        m_pcMbEncoder->setPdsInitialDelayMinus2L0( ppuiPdsInitialDelayMinus2L0[rcSliceHeader.getViewId()] );
        m_pcMbEncoder->setPdsInitialDelayMinus2L1( ppuiPdsInitialDelayMinus2L1[rcSliceHeader.getViewId()] );
    }
    //~JVT-W080

   RefFrameList acRefFrameList0[2];
   RefFrameList acRefFrameList1[2];

   IntFrame* apcFrame[4]={NULL, NULL, NULL, NULL};

   //lufeng: save origframe for multipass encoding
   IntFrame*  pcOrgFrame = new IntFrame( pcFrame->getFullPelYuvBuffer()->getBufferCtrl(), 
                pcFrame->getHalfPelYuvBuffer()->getBufferCtrl(), 
                FRAME );
       XPel* pHPData = 0;
   pcOrgFrame->getFullPelYuvBuffer()->init(pHPData);
   pcOrgFrame->getFullPelYuvBuffer()->loadBuffer(pcFrame->getFullPelYuvBuffer());
//   pcOrigFrame->getHalfPelYuvBuffer()->loadBuffer(pcFrame->getHalfPelYuvBuffer());

   IntFrame* apcOrgFrame[4]={NULL, NULL, NULL, NULL};
	RNOK( gSetFrameFieldArrays(apcOrgFrame, pcOrgFrame));
   
	RNOK( gSetFrameFieldArrays(apcFrame, pcFrame));

   RNOK( gSetFrameFieldLists(acRefFrameList0[0],acRefFrameList0[1],rcList0));
   RNOK( gSetFrameFieldLists(acRefFrameList1[0],acRefFrameList1[1],rcList1));

   RefFrameList* apcRefFrameList0[4];
   RefFrameList* apcRefFrameList1[4];

   apcRefFrameList0[0] = ( NULL == &rcList0 ) ? NULL : &acRefFrameList0[0];
   apcRefFrameList0[1] = ( NULL == &rcList0 ) ? NULL : &acRefFrameList0[1];
   apcRefFrameList1[0] = ( NULL == &rcList1 ) ? NULL : &acRefFrameList1[0];
   apcRefFrameList1[1] = ( NULL == &rcList1 ) ? NULL : &acRefFrameList1[1];
   apcRefFrameList0[2] = apcRefFrameList0[3] = &rcList0;
   apcRefFrameList1[2] = apcRefFrameList1[3] = &rcList1;


   IntYuvMbBuffer acIntYuvMbBuffer[2];

   MbDataBuffer acMbData[2];

   Bool   abSkipModeAllowed[4] = {true,true,true,true};

    //===== loop over macroblocks =====
    Int eP;
    //note: no fmo!
    for( UInt uiMbAddress = rcSliceHeader.getFirstMbInSlice(); uiMbAddress <= rcSliceHeader.getLastMbInSlice(); uiMbAddress+=2 )
    {
        Double adCost[2]  = {0,0};

        for( eP = 0; eP < 4; eP++ )
        {
            ETRACE_NEWMB( uiMbAddress );
	
			if(eP==2)
			{
					UInt uiPos;
					for( uiPos = 0; uiPos < rcList0.getActive(); uiPos++ )
					  {
						  IntFrame* pcRefFrame = rcList0.getEntry(uiPos);
						  pcRefFrame->getFullPelYuvBuffer()->fillMargin();
						}
					for( uiPos = 0; uiPos < rcList1.getActive(); uiPos++ )
					  {
						  IntFrame* pcRefFrame = rcList1.getEntry(uiPos);
						  pcRefFrame->getFullPelYuvBuffer()->fillMargin();
						}
					//lufeng: bug fixed (frame/field margin for mb in mbaff)
			}
			else if(eP==0)
			{
					UInt uiPos;
					for( uiPos = 0; uiPos < rcList0.getActive(); uiPos++ )
					  {
						  IntFrame* pcRefFrame = rcList0.getEntry(uiPos);
						  pcRefFrame->getTopField()->getFullPelYuvBuffer()->fillMargin();
						  pcRefFrame->getBotField()->getFullPelYuvBuffer()->fillMargin();
						}
					for( uiPos = 0; uiPos < rcList1.getActive(); uiPos++ )
					  {
						  IntFrame* pcRefFrame = rcList1.getEntry(uiPos);
						  pcRefFrame->getTopField()->getFullPelYuvBuffer()->fillMargin();
						  pcRefFrame->getBotField()->getFullPelYuvBuffer()->fillMargin();
						}
					//lufeng: bug fixed (frame/field margin for mb in mbaff)
			}

            MbDataAccess* pcMbDataAccess     = NULL;
            Double        dCost = 0;
            UInt          uiMbY, uiMbX;

            const Bool    bField = (eP < 2);
            const UInt    uiMbAddressMbAff = uiMbAddress+(eP%2);
            rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddressMbAff );

            //here re-calculate x,y
            RNOK( pcMbDataCtrl  ->initMb          (  pcMbDataAccess, uiMbY, uiMbX ) );
            RNOK( m_pcControlMng    ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX,  true ,bField) );//not set for last 2 params

            //JVT-W080
            if( getPdsEnable() )
            {
                m_pcMbEncoder->setCurrMBX             (  uiMbX );
                m_pcMbEncoder->setCurrMBY             (  uiMbY );
            }
            //~JVT-W080

            if( eP % 2 == 0 )
            {
                abSkipModeAllowed[eP] = pcMbDataAccess->getDefaultFieldFlag()&(eP<2); // do not move
            }
            else if( eP%2 == 1 )
            {
                const MbData&	rcTopMb = pcMbDataAccess->getMbDataComplementary();
                if (pcMbDataAccess->getDefaultFieldFlag() != rcTopMb.getFieldFlag() )
                    abSkipModeAllowed[eP] = false;
            }



            pcMbDataAccess->setFieldMode( eP < 2 );
			
            //===== initialisation =====
      /*      RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX, true ) );
            RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX, true ) );
            RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );*/

            //encode macroblock
                    RNOK( m_pcMbEncoder ->encodeMacroblock( *pcMbDataAccess,
                        apcFrame[eP],
						apcOrgFrame[eP],
                        *apcRefFrameList0    [eP],
                        *apcRefFrameList1    [eP],
                        m_pcCodingParameter->getMotionVectorSearchParams().getNumMaxIter(),
                        m_pcCodingParameter->getMotionVectorSearchParams().getIterSearchRange(),
                        dlambda, 
                        dCost,
						abSkipModeAllowed[eP]));

            if( adCost[eP>>1] != DOUBLE_MAX )
            {
                adCost [eP>>1] += dCost;
            }
            if( bField )
            {
                acMbData[eP].copy( pcMbDataAccess->getMbData() );
                (&acIntYuvMbBuffer[eP])->loadBuffer( apcFrame[eP]->getFullPelYuvBuffer() );
            }
        }
        Bool bFieldMode = adCost[0] < adCost[1];//choose by cost
		
#if RANDOM_CHOOSE
		bFieldMode=(int)rand()%2;//lufeng: random choose
#endif

        for( eP = 0; eP < 2; eP++ )
        {
            MbDataAccess* pcMbDataAccess     = NULL;
            UInt          uiMbY, uiMbX;
            const UInt    uiMbAddressMbAff   = uiMbAddress+eP;

            ETRACE_NEWMB(uiMbAddressMbAff);

            rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddressMbAff );

            //here re-calculate x,y
            RNOK( pcMbDataCtrl  ->initMb          (  pcMbDataAccess, uiMbY, uiMbX ) );
            RNOK( m_pcControlMng    ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX,  true ,bFieldMode) );//not set for last 2 params
       
            if( bFieldMode )
            {
                pcMbDataAccess->getMbData().copy( acMbData[eP] );
                apcFrame[eP]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBuffer     [eP] );
            }
			//printf("MB: %d, ff mode: %d,  mbmode: %d\n",uiMbAddressMbAff,bFieldMode,pcMbDataAccess->getMbPicType());

            RNOK( m_pcMbCoder   ->encode          ( *pcMbDataAccess, NULL, SST_RATIO_1,
                ( uiMbAddressMbAff == rcSliceHeader.getLastMbInSlice() ), (eP == 1) )  );
        }
    }
    return Err::m_nOK;
}


//TMM_WP
ErrVal SliceEncoder::xInitDefaultWeights(Double *pdWeights, UInt uiLumaWeightDenom, 
                                         UInt uiChromaWeightDenom)
{
    const Int iLumaWeight = 1 << uiLumaWeightDenom;
    const Int iChromaWeight = 1 << uiChromaWeightDenom;
    
    pdWeights[0] = iLumaWeight;
    pdWeights[1] = pdWeights[2] = iChromaWeight;

    return Err::m_nOK;
}

ErrVal SliceEncoder::xSetPredWeights( SliceHeader& rcSH, 
                                      IntFrame* pOrgFrame,
                                      RefFrameList& rcRefFrameList0,
                                      RefFrameList& rcRefFrameList1)
    
{
  RNOK( rcSH.getPredWeightTable(LIST_0).uninit() );
  RNOK( rcSH.getPredWeightTable(LIST_1).uninit() );
  RNOK( rcSH.getPredWeightTable(LIST_0).init( rcSH.getNumRefIdxActive( LIST_0) ) );
  RNOK( rcSH.getPredWeightTable(LIST_1).init( rcSH.getNumRefIdxActive( LIST_1) ) );

  ROTRS( rcSH.isIntra(), Err::m_nOK );

  const SampleWeightingParams& rcSWP = m_pcCodingParameter->getSampleWeightingParams(rcSH.getLayerId());

  { // determine denoms
    const UInt uiLumaDenom = rcSWP.getLumaDenom();
    rcSH.setLumaLog2WeightDenom  ( ( uiLumaDenom == MSYS_UINT_MAX ) ? gIntRandom(0,7) : uiLumaDenom );

    const UInt uiChromaDenom = rcSWP.getChromaDenom();
    rcSH.setChromaLog2WeightDenom( ( uiChromaDenom == MSYS_UINT_MAX ) ? gIntRandom(0,7) : uiChromaDenom );
  }

  const Int iChromaScale = 1<<rcSH.getChromaLog2WeightDenom();
  const Int iLumaScale   = 1<<rcSH.getLumaLog2WeightDenom();

   m_pcControlMng->initSliceForWeighting(rcSH);

  if( rcSH.isInterB() )
  {
      ROTRS( 1 != rcSH.getPPS().getWeightedBiPredIdc(), Err::m_nOK );
  }
  else
  {
    ROTRS( ! rcSH.getPPS().getWeightedPredFlag(), Err::m_nOK );
  }

  if( rcSH.isInterB() )
  {
      RNOK( rcSH.getPredWeightTable(LIST_1).initDefaults( rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom() ) );
  }
  RNOK( rcSH.getPredWeightTable(LIST_0).initDefaults( rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom() ) );

  Double afFwWeight[MAX_REF_FRAMES][3];
  Double afBwWeight[MAX_REF_FRAMES][3];

  Double afFwOffsets[MAX_REF_FRAMES][3];
  Double afBwOffsets[MAX_REF_FRAMES][3];

  Double fDiscardThr = m_pcCodingParameter->getSampleWeightingParams(rcSH.getLayerId()).getDiscardThr();

  /* init arrays with default weights */
  for (UInt x = 0; x < MAX_REF_FRAMES; x++)
  {
      xInitDefaultWeights(afFwWeight[x], rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom());
      xInitDefaultWeights(afBwWeight[x], rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom());

      afFwOffsets[x][0] = afFwOffsets[x][1] = afFwOffsets[x][2] = 0;
      afBwOffsets[x][0] = afBwOffsets[x][1] = afBwOffsets[x][2] = 0;
  }
  
  if( rcSH.isInterB() )
  {
      RNOK( m_pcMbEncoder->getPredWeights( rcSH, LIST_1, afBwWeight, 
                                           pOrgFrame, rcRefFrameList1 ) );      
      RNOK( rcSH.getPredWeightTable( LIST_1).setPredWeightsAndFlags( iLumaScale, iChromaScale, 
                                                                     afBwWeight, fDiscardThr ) );
  }

  RNOK( m_pcMbEncoder->getPredWeights( rcSH, LIST_0, afFwWeight, pOrgFrame, rcRefFrameList0 ) );
  RNOK( rcSH.getPredWeightTable( LIST_0).setPredWeightsAndFlags( iLumaScale, iChromaScale, 
                                                                 afFwWeight, fDiscardThr ) );  

  return Err::m_nOK;
}
//TMM_WP


H264AVC_NAMESPACE_END

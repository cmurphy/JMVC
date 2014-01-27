#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/FrameUnit.h"

#include "H264AVCCommonLib/CFMO.h"
#include <math.h>

H264AVC_NAMESPACE_BEGIN


#define MEDIAN(a,b,c)  ((a)>(b)?(a)>(c)?(b)>(c)?(b):(c):(a):(b)>(c)?(a)>(c)?(a):(c):(b))


MbDataCtrl::MbDataCtrl():
  m_pcMbTCoeffs     ( NULL ),
  m_pcMbData        ( NULL ),
  m_pcMbDataAccess  ( NULL ),
  m_pcSliceHeader   ( NULL ),
  m_ucLastMbQp      ( 0 ),
  m_uiMbStride      ( 0 ),
  m_uiMbOffset      ( 0 ),
  m_iMbPerLine      ( 0 ),
  m_iMbPerColumn    ( 0 ),
  m_uiSize          ( 0 ),
  m_uiMbProcessed   ( 0 ),
  m_uiSliceId       ( 0 ),
  m_iColocatedOffset( 0 ),
  m_eProcessingState( PRE_PROCESS),
  m_pcMbDataCtrl0L1 ( NULL ),
  m_bUseTopField    ( false ),
  m_bPicCodedField  ( false ),
  m_bInitDone       ( false )
, m_pacFgsBQMbQP    ( 0 )
, m_pauiFgsBQMbCbp  ( 0 )
, m_pauiFgsBQBCBP   ( 0 )
, m_pabFgsBQ8x8Trafo( 0 )
{
  m_apcMbMvdData    [LIST_0]  = NULL;
  m_apcMbMvdData    [LIST_1]  = NULL;
  m_apcMbMotionData [LIST_0]  = NULL;
  m_apcMbMotionData [LIST_1]  = NULL;
  m_apcMbMotionDataBase [LIST_0]  = NULL;
  m_apcMbMotionDataBase [LIST_1]  = NULL;
}

MbDataCtrl::~MbDataCtrl()
{
  AOT( xDeleteData() );
  AOT( m_bInitDone );
}

ErrVal MbDataCtrl::xCreateData( UInt uiSize )
{
  uiSize++;

  ROT( NULL == ( m_pcMbTCoeffs         = new MbTransformCoeffs [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionData[0]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionData[1]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionDataBase[0]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionDataBase[1]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMvdData[0]     = new MbMvData          [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMvdData[1]     = new MbMvData          [ uiSize ] ) );
  ROT( NULL == ( m_pcMbData            = new MbData            [ uiSize ] ) );


  for( UInt uiIdx = 0; uiIdx < uiSize; uiIdx++ )
  {
    m_pcMbData[ uiIdx ].init( m_pcMbTCoeffs        + uiIdx,
                              m_apcMbMvdData   [0] + uiIdx,
                              m_apcMbMvdData   [1] + uiIdx,
                              m_apcMbMotionData[0] + uiIdx,
                              m_apcMbMotionData[1] + uiIdx,
                              m_apcMbMotionDataBase[0] + uiIdx,
                              m_apcMbMotionDataBase[1] + uiIdx );
  }

  // clear outside mb data
  m_pcMbData[uiSize-1].getMbTCoeffs().setAllCoeffCount( 0 );
  m_pcMbData[uiSize-1].initMbData( 0, MSYS_UINT_MAX );

  return Err::m_nOK;
}

ErrVal MbDataCtrl::xDeleteData()
{
  H264AVC_DELETE_CLASS( m_pcMbDataAccess );

  H264AVC_DELETE( m_pcMbTCoeffs );
  H264AVC_DELETE( m_apcMbMvdData[1] );
  H264AVC_DELETE( m_apcMbMvdData[0] );
  H264AVC_DELETE( m_apcMbMotionData[1] );
  H264AVC_DELETE( m_apcMbMotionData[0] );
  H264AVC_DELETE( m_apcMbMotionDataBase[1] );
  H264AVC_DELETE( m_apcMbMotionDataBase[0] );
  H264AVC_DELETE( m_pcMbData );

  m_uiSize          = 0;
  return Err::m_nOK;
}

ErrVal MbDataCtrl::xResetData()
{
  UInt uiIdx;
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_pcMbData[ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_pcMbTCoeffs[ uiIdx ].clear();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMvdData[0][ uiIdx ].clear();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMvdData[1][ uiIdx ].clear();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionData[0][ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionData[1][ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionDataBase[0][ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionDataBase[1][ uiIdx ].reset();
  }

  return Err::m_nOK;
}


Bool MbDataCtrl::isPicDone( const SliceHeader& rcSH )
{
	return ( m_uiMbProcessed == rcSH.getSPS().getMbInFrame() || m_uiMbProcessed == rcSH.getMbInPic());
}

Bool MbDataCtrl::isFrameDone( const SliceHeader& rcSH )
{
  return ( m_uiMbProcessed == rcSH.getSPS().getMbInFrame());
}


ErrVal MbDataCtrl::init( const SequenceParameterSet& rcSPS )
{
  AOT_DBG( m_bInitDone );

  UInt uiSize = rcSPS.getMbInFrame();

  ROT( 0 == uiSize );
  if( m_uiSize == uiSize )
  {
    RNOK( xResetData() );
  }
  else
  {
    RNOK( xDeleteData() );
    RNOK( xCreateData( uiSize ) );
    m_uiSize = uiSize;
  }

  m_iMbPerLine = rcSPS.getFrameWidthInMbs();

  RNOK( m_cpDFPBuffer.init( uiSize+1 ) );
  m_cpDFPBuffer.clear();

  m_bInitDone     = true;

  return Err::m_nOK;
}

//JVT-T054{

ErrVal
MbDataCtrl::copyMbCBP( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters )
{
  Int  iMbOrigX = pcParameters->m_iPosX/16;
  Int  iMbOrigY = pcParameters->m_iPosY/16;
  Int  iMbEndX = iMbOrigX + pcParameters->m_iOutWidth/16;
  Int  iMbEndY = iMbOrigY + pcParameters->m_iOutHeight/16;

 for( Int iMbY = iMbOrigY ; iMbY < iMbEndY; iMbY++)
  for(Int iMbX = iMbOrigX ; iMbX < iMbEndX; iMbX++)
	 {
    MbData& rcMbDes = m_pcMbData[iMbY*rcBaseMbDataCtrl.m_uiMbStride + iMbX];
    RNOK( rcMbDes.copyMbCbp( rcBaseMbDataCtrl.m_pcMbData[(iMbY - iMbOrigY)*rcBaseMbDataCtrl.m_uiMbStride + (iMbX - iMbOrigX)]) );
	 }
  return Err::m_nOK;
}
ErrVal
MbDataCtrl::initMbCBP( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters )
{
  Int  iMbOrigX = pcParameters->m_iPosX/16;
  Int  iMbOrigY = pcParameters->m_iPosY/16;
  Int  iMbEndX = iMbOrigX + pcParameters->m_iOutWidth/16;
  Int  iMbEndY = iMbOrigY + pcParameters->m_iOutHeight/16;

 for( Int iMbY = iMbOrigY ; iMbY < iMbEndY; iMbY++)
  for(Int iMbX = iMbOrigX ; iMbX < iMbEndX; iMbX++)
	 {
    MbData& rcMbDes = m_pcMbData[iMbY*rcBaseMbDataCtrl.m_uiMbStride + iMbX];
    RNOK( rcMbDes.initMbCbp() );
	 }
  return Err::m_nOK;
}
//JVT-T054}

ErrVal
MbDataCtrl::switchMotionRefinement()
{
  for( UInt n = 0; n < m_uiSize; n++ )
    m_pcMbData[n].switchMotionRefinement();
  return Err::m_nOK;
}


ErrVal
MbDataCtrl::copyMotion( MbDataCtrl& rcMbDataCtrl )
{
  for( UInt n = 0; n < m_uiSize; n++ )
  {
    RNOK( m_pcMbData[n].copyMotion( rcMbDataCtrl.m_pcMbData[n], m_uiSliceId ) );
  }
  return Err::m_nOK;
}

// TMM_ESS_UNIFIED {
// motion copying (upsampling with factor=1) with/without cropping (MB aligned)
ErrVal
MbDataCtrl::copyMotionBL( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters )
{
  Bool bDirect8x8 = rcBaseMbDataCtrl.xGetDirect8x8InferenceFlag();

  Int  iMbOrigX = pcParameters->m_iPosX/16;
  Int  iMbOrigY = pcParameters->m_iPosY/16;
  Int  iMbEndX = iMbOrigX + pcParameters->m_iOutWidth/16;
  Int  iMbEndY = iMbOrigY + pcParameters->m_iOutHeight/16;

 for( Int iMbY = iMbOrigY ; iMbY < iMbEndY; iMbY++)
  for(Int iMbX = iMbOrigX ; iMbX < iMbEndX; iMbX++)
	 {
    MbData& rcMbDes = m_pcMbData[iMbY*rcBaseMbDataCtrl.m_uiMbStride + iMbX];
    RNOK( rcMbDes.copyMotionBL( rcBaseMbDataCtrl.m_pcMbData[(iMbY - iMbOrigY)*rcBaseMbDataCtrl.m_uiMbStride + (iMbX - iMbOrigX)], bDirect8x8, m_uiSliceId ) );
    rcMbDes.setInCropWindowFlag( true );
	 }
  return Err::m_nOK;
}


// motion upsampling (upsampling with factor=2) with/without cropping (MB aligned)
ErrVal
MbDataCtrl::xUpsampleMotionDyad( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters )
{
	Bool bDirect8x8 = rcBaseMbDataCtrl.xGetDirect8x8InferenceFlag();
	Int  iMbX,iMbY,iBaseMbY,iBaseMbX,iPar;
	UInt uiBaseMbStride=rcBaseMbDataCtrl.m_uiMbStride;

	Int  iMbOrigX = pcParameters->m_iPosX/16;
	Int  iMbOrigY = pcParameters->m_iPosY/16;
	Int  iMbEndX = iMbOrigX + pcParameters->m_iOutWidth/16;
	Int  iMbEndY = iMbOrigY + pcParameters->m_iOutHeight/16;

	//loop on scaled base window MBs
	for( iMbY = iMbOrigY,iBaseMbY = 0 ; iMbY < iMbEndY; iMbY+=2,iBaseMbY++)
		for( iMbX = iMbOrigX,iBaseMbX = 0 ; iMbX < iMbEndX; iMbX+=2,iBaseMbX++)
			for( iPar = 0; iPar < 4;              iPar++  )
			{
				MbData& rcMbDes = m_pcMbData[(iMbY+(iPar/2))*m_uiMbStride+(iMbX+(iPar%2))];
				MbData& rcMbSrc = rcBaseMbDataCtrl.m_pcMbData[iBaseMbY*uiBaseMbStride+iBaseMbX];

				Par8x8  ePar    = Par8x8( iPar );
				rcMbDes.setInCropWindowFlag( true );
				RNOK( rcMbDes.upsampleMotion( rcMbSrc, ePar, bDirect8x8 ) );
			}

			return Err::m_nOK;
}


// motion upsampling with any cropping and upsampling factor
ErrVal
MbDataCtrl::xUpsampleMotionESS( MbDataCtrl& rcBaseMbDataCtrl,ResizeParameters* pcParameters )
{
  Bool bDirect8x8 = rcBaseMbDataCtrl.xGetDirect8x8InferenceFlag();
  
  if( pcParameters->m_iExtendedSpatialScalability == ESS_PICT )
  {
  Int index = m_pcSliceHeader->getPoc();
  pcParameters->setPOC(index);
  pcParameters->setCurrentPictureParametersWith(index);
  }

  Int     iScaledBaseOrigX = pcParameters->m_iPosX;
  Int     iScaledBaseOrigY = pcParameters->m_iPosY; 
  Int     iScaledBaseWidth = pcParameters->m_iOutWidth;
  Int     iScaledBaseHeight = pcParameters->m_iOutHeight;

  Int  aiPelOrig[2];  

  // loop on MBs of high res picture
  //--------------------------------
  for( Int iMbY = 0; iMbY < m_iMbPerColumn; iMbY++ )
  {
    for( Int iMbX = 0; iMbX < m_iMbPerLine;   iMbX++ )
    {
      // get current high res MB and upsampling
      MbData& rcMbDes = m_pcMbData[iMbY*m_uiMbStride+iMbX];

      // check if MB is inside cropping window - if not, no upsampling is performed
      if ( (iMbX >= (iScaledBaseOrigX+15) / 16) && (iMbX < (iScaledBaseOrigX+iScaledBaseWidth) / 16) &&
           (iMbY >= (iScaledBaseOrigY+15) / 16) && (iMbY < (iScaledBaseOrigY+iScaledBaseHeight) / 16) )
      {
          aiPelOrig[0]=(Int)16*iMbX-iScaledBaseOrigX;
          aiPelOrig[1]=(Int)16*iMbY-iScaledBaseOrigY;
          RNOK(rcMbDes.upsampleMotionESS(rcBaseMbDataCtrl.m_pcMbData, 
                                         rcBaseMbDataCtrl.m_uiMbStride,
                                         aiPelOrig,
										                     bDirect8x8,
                                         pcParameters));
           rcMbDes.setInCropWindowFlag( true );
		  }
      else
      {
          rcMbDes.noUpsampleMotion();
      }
	 } // end of for( Int iMbX = 0; iMbX < m_iMbPerLine;   iMbX++ )
  } // end of for( Int iMbY = 0; iMbY < m_iMbPerColumn; iMbY++ )

  return Err::m_nOK;
}
ErrVal
MbDataCtrl::upsampleMotion( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters )
{
  if(NULL==pcParameters) 
  return copyMotion(rcBaseMbDataCtrl);

	if(pcParameters->m_iSpatialScalabilityType==SST_RATIO_1)
  return copyMotionBL(rcBaseMbDataCtrl, pcParameters);
  
  return xUpsampleMotionESS(rcBaseMbDataCtrl, pcParameters);
}
// TMM_ESS_UNIFIED }


ErrVal MbDataCtrl::uninit()
{
  m_ucLastMbQp      = 0;
  m_uiMbStride      = 0;
  m_uiMbOffset      = 0;
  m_iMbPerLine      = 0;
  m_iMbPerColumn    = 0;
  m_uiMbProcessed   = 0;
  m_uiSliceId       = 0;
  m_pcMbDataCtrl0L1 = 0;

  for( UInt n = 0; n < m_cpDFPBuffer.size(); n++ )
  {
    delete m_cpDFPBuffer.get( n );
    m_cpDFPBuffer.set( n, NULL );
  }
  RNOK( m_cpDFPBuffer.uninit() );

  m_bInitDone = false;
  return Err::m_nOK;
}


ErrVal MbDataCtrl::reset()
{
  m_ucLastMbQp      = 0;
  m_uiMbProcessed   = 0;
  m_uiSliceId       = 0;
  m_pcMbDataCtrl0L1 = 0;
  return Err::m_nOK;
}

ErrVal MbDataCtrl::initSlice( SliceHeader& rcSH, ProcessingState eProcessingState, Bool bDecoder, MbDataCtrl* pcMbDataCtrl )
{
  AOF_DBG( m_bInitDone );

  m_eProcessingState  = eProcessingState;
  m_pcMbDataCtrl0L1   = NULL;

  m_iColocatedOffset  = 0;
  m_bUseTopField      = false;
  m_bPicCodedField    = rcSH.getFieldPicFlag();

  if( rcSH.isInterB() )
  {
    if( ((rcSH.getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_SCALABLE &&
          rcSH.getNalUnitType() != NAL_UNIT_CODED_SLICE_SCALABLE) || (!rcSH.getSvcMvcFlag()) ) && bDecoder
          && rcSH.getRefListSize( LIST_1 )
		  )
    {
      const RefPic& rcRefPic0L1 = rcSH.getRefPic( 1, rcSH.getPicType(), LIST_1 );
      AOF_DBG( rcRefPic0L1.isAvailable() );
      const FrameUnit* pcFU = rcRefPic0L1.getPic().getFrameUnit();
      Int iCurrPoc    = rcSH.getPoc();
      Int iTopDiffPOC = iCurrPoc - pcFU->getTopField().getPOC();
      Int iBotDiffPOC = iCurrPoc - pcFU->getBotField().getPOC();

      m_bUseTopField    = (abs( iTopDiffPOC ) < abs( iBotDiffPOC ) );

      m_pcMbDataCtrl0L1 = pcFU->getMbDataCtrl();

      if( FRAME != rcSH.getPicType() )
      {
          if( rcRefPic0L1.getFrame()->getPicType() != rcSH.getPicType() && m_pcMbDataCtrl0L1->isPicCodedField() )
          {
              m_iColocatedOffset = m_iMbPerLine;
          }
      }
	 

    }

    if( pcMbDataCtrl )
    {
      m_pcMbDataCtrl0L1 = pcMbDataCtrl;
    }

    if(!bDecoder)
    { // Fix by hwsun@Panasonic & Dong
      if (eProcessingState == ENCODE_PROCESS)
      {
        if( (rcSH.getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_SCALABLE &&
          rcSH.getNalUnitType() != NAL_UNIT_CODED_SLICE_SCALABLE) || (!rcSH.getSvcMvcFlag()) )
        {
          PicType picType = rcSH.getPicType();
          RefFrameList* pRefFrameList1 = rcSH.getRefFrameList(picType, LIST_1);

          Int iCurrPoc    = rcSH.getPoc();
          Int iTopDiffPOC = iCurrPoc - pRefFrameList1->getEntry( 0 )->getTopFieldPoc();
          Int iBotDiffPOC = iCurrPoc - pRefFrameList1->getEntry( 0 )->getBotFieldPoc();

          m_bUseTopField    = (abs( iTopDiffPOC ) < abs( iBotDiffPOC ) );

          if( FRAME != rcSH.getPicType() )
          {
            if( pRefFrameList1->getEntry( 0 )->getPicType() != rcSH.getPicType() && m_pcMbDataCtrl0L1->isPicCodedField() )
            {
              m_iColocatedOffset = m_iMbPerLine;
            }
          }
        }
      }
      else
      {
        m_bUseTopField = true;//lufeng: default
      }
    }
  }

  if( PARSE_PROCESS == m_eProcessingState || ENCODE_PROCESS == m_eProcessingState )
  {
    m_uiSliceId++;

    //manu.mathew@samsung : memory leak fix
    if( m_cpDFPBuffer.get( m_uiSliceId ) )
    {
      delete m_cpDFPBuffer.get( m_uiSliceId );
      m_cpDFPBuffer.set( m_uiSliceId, NULL );
    }
    //--

    m_cpDFPBuffer.set( m_uiSliceId, rcSH.getDeblockingFilterParameter().getCopy() );
    m_bDirect8x8InferenceFlag = rcSH.getSPS().getDirect8x8InferenceFlag();
  }
  m_pcSliceHeader = &rcSH;


  Int iMbPerColumn  = rcSH.getSPS().getFrameHeightInMbs ();
  m_iMbPerLine      = rcSH.getSPS().getFrameWidthInMbs  ();

  m_uiMbOffset      = rcSH.getBottomFieldFlag() ? 1 * m_iMbPerLine : 0;
  m_uiMbStride      = rcSH.getFieldPicFlag()    ? 2 * m_iMbPerLine : m_iMbPerLine;
  m_iMbPerColumn    = rcSH.getFieldPicFlag()    ? iMbPerColumn/2 : iMbPerColumn;

  m_ucLastMbQp      = rcSH.getPicQp();

  H264AVC_DELETE_CLASS( m_pcMbDataAccess );
  return Err::m_nOK;
}


const MbData& MbDataCtrl::xGetColMbData( UInt uiIndex )
{
  return (( m_pcMbDataCtrl0L1 == NULL ) ? xGetOutMbData() : m_pcMbDataCtrl0L1->getMbData( uiIndex ));
}

const MbData& MbDataCtrl::xGetRefMbData( UInt uiSliceId, Int uiCurrSliceID, Int iMbY, Int iMbX, Bool bLoopFilter )
{
  // check whether ref mb is inside
  ROTRS( iMbX < 0,               xGetOutMbData() );
  ROTRS( iMbY < 0,               xGetOutMbData() );
  ROTRS( iMbX >= m_iMbPerLine,   xGetOutMbData() );
  ROTRS( iMbY >= m_iMbPerColumn, xGetOutMbData() );

  //--ICU/ETRI FMO Implementation
  ROTRS( uiCurrSliceID != getSliceGroupIDofMb(iMbY * (m_uiMbStride>>(UInt)m_pcSliceHeader->getFieldPicFlag()) + iMbX/* + m_uiMbOffset*/ ) , xGetOutMbData() );

  // get the ref mb data
  const MbData& rcMbData = getMbData( iMbY * m_uiMbStride + iMbX + m_uiMbOffset );
  // test slice id
  return (( rcMbData.getSliceId() == uiSliceId || bLoopFilter ) ? rcMbData : xGetOutMbData() );
}


ErrVal MbDataCtrl::initMb( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, const Bool bFieldFlag, const Int iForceQp )
{
    UInt     uiCurrIdx    = uiMbY        * m_uiMbStride + uiMbX + m_uiMbOffset;
    MbData&  rcMbDataCurr = m_pcMbData[ uiCurrIdx ];
    rcMbDataCurr.setFieldFlag( bFieldFlag );

    return initMb( rpcMbDataAccess, uiMbY, uiMbX, iForceQp );
}

ErrVal MbDataCtrl::initMb( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, const Int iForceQp )
{
    ROF( m_bInitDone );

    AOT_DBG( uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset >= m_uiSize );

    Bool     bLf          = (m_eProcessingState == POST_PROCESS);
    Bool     bMbaff       = m_pcSliceHeader->isMbAff();
    Bool     bTopMb       = ((bMbaff && (uiMbY % 2)) ? false : true);
    UInt     uiMbYComp    = ( bMbaff ? ( bTopMb ? uiMbY+1 : uiMbY-1 ) : uiMbY );
    UInt     uiCurrIdx    = uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset;
    UInt     uiCompIdx    = uiMbYComp    * m_uiMbStride + uiMbX + m_uiMbOffset;
    ROT( uiCompIdx >= m_uiSize );
    ROT( uiCurrIdx >= m_uiSize );
    MbData&  rcMbDataCurr = m_pcMbData[ uiCurrIdx ];
    MbData&  rcMbDataComp = m_pcMbData[ uiCompIdx ];

    //----- get co-located MbIndex -----
    UInt     uiIdxColTop;
    UInt     uiIdxColBot;
    if( ! m_pcSliceHeader->getFieldPicFlag() )
    {
        UInt  uiMbYColTop = 2 * ( uiMbY / 2 );
        uiIdxColTop       = uiMbYColTop * m_uiMbStride + uiMbX + m_uiMbOffset;
        uiIdxColBot       = uiIdxColTop + m_uiMbStride;
        //    if( uiIdxColBot >= m_uiSize ) //th bug
        if( uiIdxColBot >= m_pcSliceHeader->getMbInPic() ) //th bug
        {
            uiIdxColBot = uiIdxColTop;
        }
    }
    else if( ! m_pcSliceHeader->getBottomFieldFlag() )
    {
        uiIdxColTop       = uiCurrIdx   + m_iColocatedOffset;
        uiIdxColBot       = uiIdxColTop - m_iColocatedOffset + m_iMbPerLine;
    }
    else
    {
        uiIdxColBot       = uiCurrIdx   - m_iColocatedOffset;
        uiIdxColTop       = uiIdxColBot + m_iColocatedOffset - m_iMbPerLine;
    }

    if( m_pcMbDataAccess )
    {
        m_ucLastMbQp = m_pcMbDataAccess->getMbData().getQp();
    }

    UInt uiSliceId = rcMbDataCurr.getSliceId();

    if( PARSE_PROCESS == m_eProcessingState || ENCODE_PROCESS == m_eProcessingState)
    {
        if( 0 == uiSliceId )
        {
            uiSliceId = m_uiSliceId;
            rcMbDataCurr.getMbTCoeffs().clear();
            rcMbDataCurr.initMbData( m_ucLastMbQp, uiSliceId );
            rcMbDataCurr.clear();
            m_uiMbProcessed++;
        }
        else
        {
			//allready assigned;
            if( ENCODE_PROCESS != m_eProcessingState )
            {
                AF();
            }
            else
            {
                if( iForceQp != -1 )
                {
                    m_ucLastMbQp = iForceQp;
                }
            }
        }
    }


  Int icurrSliceGroupID = getSliceGroupIDofMb(uiMbY * (m_uiMbStride>>(UInt)m_pcSliceHeader->getFieldPicFlag()) + uiMbX ); 
        Bool bColocatedField = ( m_pcMbDataCtrl0L1 == NULL) ? true : m_pcMbDataCtrl0L1->isPicCodedField();
    m_pcMbDataAccess = new (m_pcMbDataAccess) MbDataAccess(
        rcMbDataCurr,                                      // current
        rcMbDataComp,                                      // complementary
        xGetRefMbData( uiSliceId,icurrSliceGroupID, uiMbY,   uiMbX-1, bLf ), // left
        xGetRefMbData( uiSliceId,icurrSliceGroupID, uiMbY-1, uiMbX  , bLf ), // above
        xGetRefMbData( uiSliceId,icurrSliceGroupID, uiMbY-1, uiMbX-1, bLf ), // above left
        ((bMbaff && (uiMbY % 2 == 1)) ? xGetOutMbData() : xGetRefMbData( uiSliceId,icurrSliceGroupID, uiMbY-1, uiMbX+1, bLf )), // above right
        xGetRefMbData( uiSliceId, icurrSliceGroupID,uiMbY-2, uiMbX  , bLf ), // above above
        xGetRefMbData( uiSliceId, icurrSliceGroupID,uiMbY-2, uiMbX-1, bLf ), // above above left
        xGetRefMbData( uiSliceId, icurrSliceGroupID,uiMbY-2, uiMbX+1, bLf ), // above above right
        xGetRefMbData( uiSliceId, icurrSliceGroupID,uiMbY+1, uiMbX-1, bLf ), // below left
        xGetOutMbData(),                                   // unvalid
        xGetColMbData( uiIdxColTop ),
        xGetColMbData( uiIdxColBot ),
        *m_pcSliceHeader,
        *m_cpDFPBuffer.get( uiSliceId ),
        uiMbX,
        uiMbY,
        bTopMb,
        m_bUseTopField,
        bColocatedField,
        m_ucLastMbQp );

    ROT( NULL == m_pcMbDataAccess );

    rpcMbDataAccess = m_pcMbDataAccess;

    return Err::m_nOK;
}

//TMM_EC {{
ErrVal MbDataCtrl::initMbTDEnhance( MbDataAccess*& rpcMbDataAccess, MbDataCtrl *pcMbDataCtrl, MbDataCtrl *pcMbDataCtrlRef, UInt uiMbY, UInt uiMbX, const Int iForceQp )
{
    ROF( m_bInitDone );

    AOT_DBG( uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset >= m_uiSize );

    Bool     bLf          = (m_eProcessingState == POST_PROCESS);
    Bool     bMbaff       = m_pcSliceHeader->isMbAff();
    Bool     bTopMb       = ((bMbaff && (uiMbY % 2)) ? false : true);
    UInt     uiMbYComp    = ( bMbaff ? ( bTopMb ? uiMbY+1 : uiMbY-1 ) : uiMbY );
    UInt     uiCurrIdx    = uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset;
    UInt     uiCompIdx    = uiMbYComp    * m_uiMbStride + uiMbX + m_uiMbOffset;
    ROT( uiCompIdx >= m_uiSize );
    ROT( uiCurrIdx >= m_uiSize );
    MbData&  rcMbDataCurr = m_pcMbData[ uiCurrIdx ];
    MbData&  rcMbDataComp = m_pcMbData[ uiCompIdx ];

    //----- get co-located MbIndex -----
    UInt     uiIdxColTop;
    UInt     uiIdxColBot;
    if( ! m_pcSliceHeader->getFieldPicFlag() )
    {
        UInt  uiMbYColTop = 2 * ( uiMbY / 2 );
        uiIdxColTop       = uiMbYColTop * m_uiMbStride + uiMbX + m_uiMbOffset;
        uiIdxColBot       = uiIdxColTop + m_uiMbStride;
        //    if( uiIdxColBot >= m_uiSize ) //th bug
        if( uiIdxColBot >= m_pcSliceHeader->getMbInPic() ) //th bug
        {
            uiIdxColBot = uiIdxColTop;
        }
    }
    else if( ! m_pcSliceHeader->getBottomFieldFlag() )
    {
        uiIdxColTop       = uiCurrIdx   + m_iColocatedOffset;
        uiIdxColBot       = uiIdxColTop - m_iColocatedOffset + m_iMbPerLine;
    }
    else
    {
        uiIdxColBot       = uiCurrIdx   - m_iColocatedOffset;
        uiIdxColTop       = uiIdxColBot + m_iColocatedOffset - m_iMbPerLine;
    }

    if( m_pcMbDataAccess )
    {
        m_ucLastMbQp = m_pcMbDataAccess->getMbData().getQp();
    }

    UInt uiSliceId = rcMbDataCurr.getSliceId();
    if( PARSE_PROCESS == m_eProcessingState || ENCODE_PROCESS == m_eProcessingState)
    {
        if( 0 == uiSliceId )
        {
            uiSliceId = m_uiSliceId;
            rcMbDataCurr.getMbTCoeffs().clear();
            rcMbDataCurr.initMbData( m_ucLastMbQp, uiSliceId );
            rcMbDataCurr.clear();
            m_uiMbProcessed++;
        }
        else
        {
            //allready assigned;
            if( ENCODE_PROCESS != m_eProcessingState )
            {
                AF();
            }
            else
            {
                if( iForceQp != -1 )
                {
                    m_ucLastMbQp = iForceQp;
                }
            }
        }
    }

    Int icurrSliceGroupID = getSliceGroupIDofMb(uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset);

    Bool bColocatedField = ( m_pcMbDataCtrl0L1 == NULL) ? true : m_pcMbDataCtrl0L1->isPicCodedField();
    m_pcMbDataAccess = new (m_pcMbDataAccess) MbDataAccess(
        rcMbDataCurr,                                      // current
        rcMbDataComp,                                      // complementary
        xGetRefMbData( uiSliceId,icurrSliceGroupID, uiMbY,   uiMbX-1, bLf ), // left
        xGetRefMbData( uiSliceId,icurrSliceGroupID, uiMbY-1, uiMbX  , bLf ), // above
        xGetRefMbData( uiSliceId,icurrSliceGroupID, uiMbY-1, uiMbX-1, bLf ), // above left
        ((bMbaff && (uiMbY % 2 == 1)) ? xGetOutMbData() : xGetRefMbData( uiSliceId,icurrSliceGroupID, uiMbY-1, uiMbX+1, bLf )), // above right
        xGetRefMbData( uiSliceId, icurrSliceGroupID,uiMbY-2, uiMbX  , bLf ), // above above
        xGetRefMbData( uiSliceId, icurrSliceGroupID,uiMbY-2, uiMbX-1, bLf ), // above above left
        xGetRefMbData( uiSliceId, icurrSliceGroupID,uiMbY-2, uiMbX+1, bLf ), // above above right
        xGetRefMbData( uiSliceId, icurrSliceGroupID,uiMbY+1, uiMbX-1, bLf ), // below left
        xGetOutMbData(),                                   // unvalid
        pcMbDataCtrlRef->getMbData(uiIdxColTop),
        pcMbDataCtrlRef->getMbData(uiIdxColBot),
        *m_pcSliceHeader,
        *m_cpDFPBuffer.get( uiSliceId ),
        uiMbX,
        uiMbY,
        bTopMb,
        m_bUseTopField,
        bColocatedField,
        m_ucLastMbQp );

    ROT( NULL == m_pcMbDataAccess );

    rpcMbDataAccess = m_pcMbDataAccess;

    return Err::m_nOK;
}
//TMM_EC }}

ErrVal
MbDataCtrl::storeFgsBQLayerQpAndCbp()
{
  ROF( m_pacFgsBQMbQP );
  ROF( m_pauiFgsBQMbCbp );
  ROF( m_pauiFgsBQBCBP );
  ROF( m_pabFgsBQ8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < getSize(); uiMbIndex++ )
  {
    m_pacFgsBQMbQP     [uiMbIndex] = getMbData( uiMbIndex ).getQp();
    m_pauiFgsBQMbCbp   [uiMbIndex] = getMbData( uiMbIndex ).getMbExtCbp();
    m_pauiFgsBQBCBP    [uiMbIndex] = getMbData( uiMbIndex ).getBCBP();
    m_pabFgsBQ8x8Trafo [uiMbIndex] = getMbData( uiMbIndex ).isTransformSize8x8();
    
  }
  return Err::m_nOK;
}

ErrVal
MbDataCtrl::switchFgsBQLayerQpAndCbp()
{
  ROF( m_pacFgsBQMbQP );
  ROF( m_pauiFgsBQMbCbp );
  ROF( m_pauiFgsBQBCBP );
  ROF( m_pabFgsBQ8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < getSize(); uiMbIndex++ )
  {
    UChar ucQP  = getMbData( uiMbIndex ).getQp();
    UInt  uiCbp = getMbData( uiMbIndex ).getMbExtCbp();
    UInt  uiBCBP= getMbData( uiMbIndex ).getBCBP();
    Bool  bT8x8 = getMbData( uiMbIndex ).isTransformSize8x8();

    getMbDataByIndex( uiMbIndex ).setQp               ( m_pacFgsBQMbQP     [uiMbIndex] );
    getMbDataByIndex( uiMbIndex ).setMbExtCbp         ( m_pauiFgsBQMbCbp   [uiMbIndex] );
    getMbDataByIndex( uiMbIndex ).setBCBP             ( m_pauiFgsBQBCBP    [uiMbIndex] );
    getMbDataByIndex( uiMbIndex ).setTransformSize8x8 ( m_pabFgsBQ8x8Trafo [uiMbIndex] );

    m_pacFgsBQMbQP     [uiMbIndex] = ucQP;
    m_pauiFgsBQMbCbp   [uiMbIndex] = uiCbp;
    m_pauiFgsBQBCBP    [uiMbIndex] = uiBCBP;
    m_pabFgsBQ8x8Trafo [uiMbIndex] = bT8x8;
  }
  return Err::m_nOK;
}

ErrVal
MbDataCtrl::initFgsBQData( UInt uiNumMb )
{
  ROT( m_pacFgsBQMbQP );
  ROT( m_pauiFgsBQMbCbp );
  ROT( m_pauiFgsBQBCBP );
  ROT( m_pabFgsBQ8x8Trafo );
  ROFS( ( m_pacFgsBQMbQP      = new UChar [uiNumMb] ) );
  ROFS( ( m_pauiFgsBQMbCbp    = new UInt  [uiNumMb] ) );
  ROFS( ( m_pauiFgsBQBCBP     = new UInt  [uiNumMb] ) );
  ROFS( ( m_pabFgsBQ8x8Trafo  = new Bool  [uiNumMb] ) );
  return Err::m_nOK;
}

ErrVal
MbDataCtrl::uninitFgsBQData()
{
  delete [] m_pacFgsBQMbQP;
  delete [] m_pauiFgsBQMbCbp;
  delete [] m_pauiFgsBQBCBP;
  delete [] m_pabFgsBQ8x8Trafo;
  m_pacFgsBQMbQP      = 0;
  m_pauiFgsBQMbCbp    = 0;
  m_pauiFgsBQBCBP    = 0;
  m_pabFgsBQ8x8Trafo  = 0;
  return Err::m_nOK;
}









ControlData::ControlData()
: m_pcMbDataCtrl        ( 0   )
, m_pcSliceHeader       ( 0   )
, m_pcSliceHeaderBot     ( 0 )
, m_dLambda             ( 0   )
, m_pcBaseLayerRec      ( 0   )
, m_pcBaseLayerSbb      ( 0   )
, m_pcBaseLayerCtrl     ( 0   )
, m_pcBaseCtrlData      ( 0   )
, m_uiUseBLMotion       ( 0   )
, m_dScalingFactor      ( 1.0 )
, m_pacFGSMbQP          ( 0 )
, m_pauiFGSMbCbp        ( 0 )
, m_pabFGS8x8Trafo      ( 0 )
, m_bIsNormalMbDataCtrl ( true )
, m_pacBQMbQP           ( 0 )
, m_pauiBQMbCbp         ( 0 )
, m_pabBQ8x8Trafo       ( 0 )
, m_paeBQMbMode         ( 0 )
, m_pusBQFwdBwd         ( 0 )
, m_iSpatialScalabilityType ( 0 ) 
, m_bSpatialScalability ( false)//SSUN@SHARP
{
  m_paacBQMotionData[0] = m_paacBQMotionData[1] = 0;
}

ControlData::~ControlData()
{
  AOT( m_pacBQMbQP );
  AOT( m_pauiBQMbCbp );
  AOT( m_pabBQ8x8Trafo );
  AOT( m_pacFGSMbQP );
  AOT( m_pauiFGSMbCbp );
  AOT( m_pabFGS8x8Trafo );
}

Void
ControlData::clear()
{
  m_pcBaseLayerRec      = 0;
  m_pcBaseLayerSbb      = 0;
  m_pcBaseLayerCtrl     = 0;
  m_uiUseBLMotion       = 0;
  m_dScalingFactor      = 1.0;

  m_bIsNormalMbDataCtrl = true;
}

ErrVal
ControlData::init( SliceHeader*  pcSliceHeader,
                   MbDataCtrl*   pcMbDataCtrl,
                   Double        dLambda )
{
  ROF( pcSliceHeader );
  ROF( pcMbDataCtrl  );

  m_pcSliceHeader = pcSliceHeader;
  m_pcMbDataCtrl  = pcMbDataCtrl;
  m_dLambda       = dLambda;
  
  m_pcBaseLayerRec      = 0;
  m_pcBaseLayerSbb      = 0;
  m_pcBaseLayerCtrl     = 0;
  m_uiUseBLMotion       = 0;

  return Err::m_nOK;
}

ErrVal
ControlData::init( SliceHeader*  pcSliceHeader )
{
  ROF( pcSliceHeader );
  ROF( m_pcMbDataCtrl  );

  m_pcSliceHeader = pcSliceHeader;
  
  m_pcBaseLayerRec      = 0;
  m_pcBaseLayerSbb      = 0;
  m_pcBaseLayerCtrl     = 0;
  m_uiUseBLMotion       = 0;

  return Err::m_nOK;
}

ErrVal
ControlData::initBQData( UInt uiNumMb )
{
  ROT( m_pacBQMbQP );
  ROT( m_pauiBQMbCbp );
  ROT( m_pabBQ8x8Trafo );
  ROFS( ( m_pacBQMbQP      = new UChar [uiNumMb] ) );
  ROFS( ( m_pauiBQMbCbp    = new UInt  [uiNumMb] ) );
  ROFS( ( m_pabBQ8x8Trafo  = new Bool  [uiNumMb] ) );
  ROFS( ( m_paeBQMbMode    = new MbMode[uiNumMb] ) );
  ROFS( ( m_pusBQFwdBwd    = new UShort[uiNumMb] ) );
  ROFS( ( m_paacBQMotionData[0] = new MbMotionData[uiNumMb] ) );
  ROFS( ( m_paacBQMotionData[1] = new MbMotionData[uiNumMb] ) );
  return Err::m_nOK;
}

ErrVal
ControlData::uninitBQData()
{
  delete [] m_pacBQMbQP;
  delete [] m_pauiBQMbCbp;
  delete [] m_pabBQ8x8Trafo;
  delete [] m_paeBQMbMode;
  delete [] m_pusBQFwdBwd;
  delete [] m_paacBQMotionData[0];
  delete [] m_paacBQMotionData[1];
  m_pacBQMbQP     = 0;
  m_pauiBQMbCbp   = 0;
  m_pabBQ8x8Trafo = 0;
  m_paeBQMbMode   = 0;
  m_pusBQFwdBwd   = 0;
  m_paacBQMotionData[0] = 0;
  m_paacBQMotionData[1] = 0;
  return Err::m_nOK;
}


ErrVal
ControlData::storeBQLayerQpAndCbp()
{
  ROF( m_pacBQMbQP );
  ROF( m_pauiBQMbCbp );
  ROF( m_pabBQ8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < m_pcMbDataCtrl->getSize(); uiMbIndex++ )
  {
    m_pacBQMbQP     [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getQp();
    m_pauiBQMbCbp   [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbExtCbp();
    m_pabBQ8x8Trafo [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).isTransformSize8x8();
    m_paeBQMbMode   [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMode();
    m_pusBQFwdBwd   [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getFwdBwd();
    m_paacBQMotionData[0][uiMbIndex].copyFrom( m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMotionData( ListIdx( 0 ) ) );
    m_paacBQMotionData[1][uiMbIndex].copyFrom( m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMotionData( ListIdx( 1 ) ) );
  }
  return Err::m_nOK;
}

ErrVal
ControlData::switchBQLayerQpAndCbp()
{
  ROF( m_pacBQMbQP );
  ROF( m_pauiBQMbCbp );
  ROF( m_pabBQ8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < m_pcMbDataCtrl->getSize(); uiMbIndex++ )
  {
    UChar ucQP  = m_pcMbDataCtrl->getMbData( uiMbIndex ).getQp();
    UInt  uiCbp = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbExtCbp();
    Bool  bT8x8 = m_pcMbDataCtrl->getMbData( uiMbIndex ).isTransformSize8x8();

    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setQp               ( m_pacBQMbQP     [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setMbExtCbp         ( m_pauiBQMbCbp   [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setTransformSize8x8 ( m_pabBQ8x8Trafo [uiMbIndex] );

    m_pacBQMbQP     [uiMbIndex] = ucQP;
    m_pauiBQMbCbp   [uiMbIndex] = uiCbp;
    m_pabBQ8x8Trafo [uiMbIndex] = bT8x8;

    MbMode       eMbMode  = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMode();
    UShort       usFwdBwd = m_pcMbDataCtrl->getMbData( uiMbIndex ).getFwdBwd();

    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setMbMode           ( m_paeBQMbMode [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setFwdBwd           ( m_pusBQFwdBwd [uiMbIndex] );

    m_paeBQMbMode   [uiMbIndex] = eMbMode;
    m_pusBQFwdBwd   [uiMbIndex] = usFwdBwd;

    for( UInt ui = 0; ui < 2; ui++ )
    {
      MbMotionData cMbMotionData;
      cMbMotionData.copyFrom( m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMotionData( ListIdx( ui ) ) );
      m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).getMbMotionData( ListIdx( ui ) ).copyFrom( m_paacBQMotionData[ui][uiMbIndex] );
      m_paacBQMotionData[ui][uiMbIndex].copyFrom( cMbMotionData );
    }
  }
  return Err::m_nOK;
}




ErrVal
ControlData::initFGSData( UInt uiNumMb )
{
  ROT( m_pacFGSMbQP );
  ROT( m_pauiFGSMbCbp );
  ROT( m_pabFGS8x8Trafo );
  ROFS( ( m_pacFGSMbQP      = new UChar [uiNumMb] ) );
  ROFS( ( m_pauiFGSMbCbp    = new UInt  [uiNumMb] ) );
  ROFS( ( m_pabFGS8x8Trafo  = new Bool  [uiNumMb] ) );
  return Err::m_nOK;
}

ErrVal
ControlData::uninitFGSData()
{
  delete [] m_pacFGSMbQP;
  delete [] m_pauiFGSMbCbp;
  delete [] m_pabFGS8x8Trafo;
  m_pacFGSMbQP      = 0;
  m_pauiFGSMbCbp    = 0;
  m_pabFGS8x8Trafo  = 0;
  return Err::m_nOK;
}

ErrVal
ControlData::storeFGSLayerQpAndCbp()
{
  ROF( m_pacFGSMbQP );
  ROF( m_pauiFGSMbCbp );
  ROF( m_pabFGS8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < m_pcMbDataCtrl->getSize(); uiMbIndex++ )
  {
    m_pacFGSMbQP     [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getQp();
    m_pauiFGSMbCbp   [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbExtCbp();
    m_pabFGS8x8Trafo [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).isTransformSize8x8();
  }
  return Err::m_nOK;
}

ErrVal
ControlData::switchFGSLayerQpAndCbp()
{
  ROF( m_pacFGSMbQP );
  ROF( m_pauiFGSMbCbp );
  ROF( m_pabFGS8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < m_pcMbDataCtrl->getSize(); uiMbIndex++ )
  {
    UChar ucQP  = m_pcMbDataCtrl->getMbData( uiMbIndex ).getQp();
    UInt  uiCbp = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbExtCbp();
    Bool  bT8x8 = m_pcMbDataCtrl->getMbData( uiMbIndex ).isTransformSize8x8();

    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setQp               ( m_pacFGSMbQP     [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setMbExtCbp         ( m_pauiFGSMbCbp   [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setTransformSize8x8 ( m_pabFGS8x8Trafo [uiMbIndex] );

    m_pacFGSMbQP     [uiMbIndex] = ucQP;
    m_pauiFGSMbCbp   [uiMbIndex] = uiCbp;
    m_pabFGS8x8Trafo [uiMbIndex] = bT8x8;
  }
  return Err::m_nOK;
}











ErrVal MbDataCtrl::getBoundaryMask( Int iMbY, Int iMbX, UInt& ruiMask ) const 
{
  UInt     uiCurrIdx    = iMbY * m_uiMbStride + iMbX + m_uiMbOffset;
  AOT( uiCurrIdx >= m_uiSize );

  ruiMask               = 0;

  ROTRS( m_pcMbData[uiCurrIdx].isIntra(), Err::m_nOK );

  Bool bLeftAvailable   = ( iMbX > 0 );
  Bool bTopAvailable    = ( iMbY > 0 );
  Bool bRightAvailable  = ( iMbX < m_iMbPerLine-1 );
  Bool bBottomAvailable = ( iMbY < m_iMbPerColumn-1 );

  if( bTopAvailable )
  {
    {
    Int iIndex = uiCurrIdx - m_uiMbStride;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x01 :0;
    }

    if( bLeftAvailable )
    {
      Int iIndex = uiCurrIdx - m_uiMbStride - 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x80 :0;
    }

    if( bRightAvailable )
    {
      Int iIndex = uiCurrIdx - m_uiMbStride + 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x02 :0;
    }
  }

  if( bBottomAvailable )
  {
    {
    Int iIndex = uiCurrIdx + m_uiMbStride;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x10 :0;
    }

    if( bLeftAvailable )
    {
      Int iIndex = uiCurrIdx  + m_uiMbStride - 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x20 :0;
    }

    if( bRightAvailable )
    {
      Int iIndex = uiCurrIdx + m_uiMbStride + 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x08 :0;
    }
  }

  if( bLeftAvailable )
  {
    Int iIndex = uiCurrIdx-1;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x40 :0;
  }

  if( bRightAvailable )
  {
    Int iIndex = uiCurrIdx + 1;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x04 :0;
  }
  return Err::m_nOK;
}

const Int MbDataCtrl::getSliceGroupIDofMb(Int mb)
{
  Int iRefSliceID ;
  if(m_pcSliceHeader->getFMO() != NULL)
	iRefSliceID =m_pcSliceHeader->getFMO()->getSliceGroupId(mb );
  else
	iRefSliceID =-1;

  return iRefSliceID ;
}

H264AVC_NAMESPACE_END


#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/IntFrame.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"




H264AVC_NAMESPACE_BEGIN


//////////////////////////////////////////////////////////////////////////
// DPB UNIT
//////////////////////////////////////////////////////////////////////////
DPBUnit::DPBUnit()
: m_iPoc                ( MSYS_INT_MIN )
, m_uiFrameNum          ( MSYS_UINT_MAX )
, m_uiTemporalLevel     ( MSYS_UINT_MAX )
, m_bKeyPicture         ( false )
, m_bExisting           ( false )
, m_bNeededForReference ( false )
, m_bOutputted          ( false )
, m_bBaseRepresentation ( false )
, m_pcFrame             ( 0 )
, m_cControlData        ()
, m_bConstrainedIntraPred( false )
//JVT-T054{
, m_uiQualityLevel      ( 0 )
//JVT-T054}
{
}



DPBUnit::~DPBUnit()
{
  MbDataCtrl*   pcMbDataCtrl  = m_cControlData.getMbDataCtrl  ();
  SliceHeader*  pcSliceHeader = m_cControlData.getSliceHeader ();
  //m_cControlData.getMbDataCtrl()->uninitFgsBQData(); 
  if( pcMbDataCtrl )
  {
//JVT-T054{
    m_cControlData.getMbDataCtrl()->uninitFgsBQData();
//JVT-T054}
    pcMbDataCtrl->uninit();
  }
  delete pcMbDataCtrl;
  delete pcSliceHeader;

  if( m_pcFrame )
  {
    m_pcFrame->uninit();
  }
  delete m_pcFrame;
}



ErrVal
DPBUnit::create( DPBUnit*&                    rpcDPBUnit,
                 YuvBufferCtrl&               rcYuvBufferCtrl,
                 const SequenceParameterSet&  rcSPS )
{
  rpcDPBUnit = new DPBUnit();
  ROF( rpcDPBUnit );

  MbDataCtrl* pcMbDataCtrl = 0;
  ROFS( ( rpcDPBUnit->m_pcFrame    = new IntFrame  ( rcYuvBufferCtrl, rcYuvBufferCtrl ) ) );
  ROFS( ( pcMbDataCtrl             = new MbDataCtrl()                                   ) );
  RNOK (   rpcDPBUnit->m_pcFrame    ->init          ()               );
           rpcDPBUnit->m_pcFrame    ->setDPBUnit    ( rpcDPBUnit   );
  RNOK (   pcMbDataCtrl             ->init          ( rcSPS        ) );
  RNOK (   rpcDPBUnit->m_cControlData.setMbDataCtrl ( pcMbDataCtrl ) );
  RNOK  (  rpcDPBUnit->m_cControlData.getMbDataCtrl()->initFgsBQData   ( pcMbDataCtrl->getSize() ) );

  return Err::m_nOK;
}



ErrVal
DPBUnit::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal
DPBUnit::init( Int  iPoc,
               UInt uiFrameNum,
               UInt uiTemporalLevel,
               Bool bKeyPicture,
               Bool bNeededForReference,
               Bool bConstrainedIPred,
               UInt uiQualityLevel) //JVT-T054
{
  m_iPoc                = iPoc;
  m_uiFrameNum          = uiFrameNum;
  m_uiTemporalLevel     = uiTemporalLevel;
  m_bKeyPicture         = bKeyPicture;
  m_bExisting           = true;
  m_bNeededForReference = bNeededForReference;
  m_bOutputted          = false;
  m_bBaseRepresentation = false;
  m_bConstrainedIntraPred = bConstrainedIPred;
//JVT-T054{
  m_uiQualityLevel      = uiQualityLevel;
//JVT-T054}
  return Err::m_nOK;
}



ErrVal
DPBUnit::initNonEx( Int   iPoc,
                    UInt  uiFrameNum )
{
  m_iPoc                = iPoc;
  m_uiFrameNum          = uiFrameNum;
  m_uiTemporalLevel     = MSYS_UINT_MAX;
  m_bKeyPicture         = false;
  m_bExisting           = false;
  m_bNeededForReference = true;
  m_bOutputted          = false;
  m_bBaseRepresentation = false;
  m_bConstrainedIntraPred = false;
  return Err::m_nOK;
}



ErrVal
DPBUnit::initBase( DPBUnit&   rcDPBUnit,
                   IntFrame*  pcFrameBaseRep )
{
  ROT( rcDPBUnit.m_bBaseRepresentation );
  m_iPoc                = rcDPBUnit.m_iPoc;
  m_uiFrameNum          = rcDPBUnit.m_uiFrameNum;
  m_uiTemporalLevel     = rcDPBUnit.m_uiTemporalLevel;
  m_bKeyPicture         = rcDPBUnit.m_bKeyPicture;
  m_bExisting           = rcDPBUnit.m_bExisting;
  m_bNeededForReference = rcDPBUnit.m_bNeededForReference;
  m_bOutputted          = rcDPBUnit.m_bOutputted;
  m_bConstrainedIntraPred = rcDPBUnit.m_bConstrainedIntraPred;
  m_bBaseRepresentation = true;
//JVT-T054{
  m_uiQualityLevel      = rcDPBUnit.m_uiQualityLevel;
//JVT-T054}
  RNOK( m_pcFrame->copyAll( pcFrameBaseRep ) );
  m_pcFrame->setPOC( m_iPoc );
  return Err::m_nOK;
}



ErrVal
DPBUnit::uninit()
{
  m_iPoc                = MSYS_INT_MIN;
  m_uiFrameNum          = MSYS_UINT_MAX;
  m_uiTemporalLevel     = MSYS_UINT_MAX;
  m_bKeyPicture         = false;
  m_bExisting           = false;
  m_bNeededForReference = false;
  m_bOutputted          = false;
  m_bBaseRepresentation = false;
  m_bConstrainedIntraPred = false;
  return Err::m_nOK;
}



ErrVal
DPBUnit::markNonRef()
{
  ROF( m_bNeededForReference );
  m_bNeededForReference = false;
  return Err::m_nOK;
}



ErrVal
DPBUnit::markOutputted()
{
  ROT( m_bOutputted );
  m_bOutputted = true;
  return Err::m_nOK;
}




IntFrame::IntFrame( YuvBufferCtrl& rcYuvFullPelBufferCtrl,
                   YuvBufferCtrl& rcYuvHalfPelBufferCtrl,
                   PicType ePicType )
                   : m_cFullPelYuvBuffer     ( rcYuvFullPelBufferCtrl, ePicType ),
                   m_cHalfPelYuvBuffer     ( rcYuvHalfPelBufferCtrl, ePicType ),
                   m_ePicType              ( ePicType ),
                   m_pcIntFrameTopField( NULL ),//th
                   m_pcIntFrameBotField( NULL ),//th
  m_bHalfPel              ( false ),
  m_bExtended             ( false ),
  m_pcDPBUnit             ( 0 )
  , m_bUnusedForRef       ( false) // JVT-Q065 EIDR
  ,m_piChannelDistortion   ( 0 )     // JVT-R057 LA-RDO
  ,m_iPOC                  (-600)     // random number
{
  m_iTopFieldPoc = m_iPOC*2;
  m_iBotFieldPoc = m_iPOC*2 + 1;    // give some random numbers
  m_uiViewId = 0;
  m_ePicStat = NOT_SPECIFIED;
}

IntFrame::~IntFrame()
{
}


ErrVal IntFrame::init( Bool bHalfPel )
{
  XPel* pData = 0;
  RNOK( m_cFullPelYuvBuffer.init( pData ) );

  if( bHalfPel )
  {
    XPel* pHPData = 0;
    RNOK( m_cHalfPelYuvBuffer.init( pHPData ) );
    m_bHalfPel = true;
  }
  m_bExtended = false;

  return Err::m_nOK;
}


ErrVal IntFrame::initHalfPel()
{
  XPel* pHPData = 0;
  if(m_cHalfPelYuvBuffer.getBuffer()!=NULL)m_cHalfPelYuvBuffer.uninit();
  RNOK( m_cHalfPelYuvBuffer.init( pHPData ) );
  m_bExtended = false;
  m_bHalfPel  = true;

  return Err::m_nOK;
}


ErrVal IntFrame::uninit()
{
    if( m_pcIntFrameTopField )
    {
        RNOK( m_pcIntFrameTopField->uninit() );
        delete m_pcIntFrameTopField;
        m_pcIntFrameTopField = NULL;
    }

    if( m_pcIntFrameBotField )
    {
        RNOK( m_pcIntFrameBotField->uninit() );
        delete m_pcIntFrameBotField;
        m_pcIntFrameBotField = NULL;
    }
  m_bPocIsSet = false;

  RNOK( m_cFullPelYuvBuffer.uninit() );
  RNOK( m_cHalfPelYuvBuffer.uninit() );
  m_bHalfPel  = false;
  m_bExtended = false;
  
  return Err::m_nOK;
}

ErrVal IntFrame::uninitHalfPel()
{
  RNOK( m_cHalfPelYuvBuffer.uninit() );
  m_bExtended = false;
  m_bHalfPel  = false;

  if( m_ePicType==FRAME && NULL != m_pcIntFrameTopField )
  {
      RNOK( m_pcIntFrameTopField->uninitHalfPel() );
  }
  if( m_ePicType==FRAME && NULL != m_pcIntFrameBotField )
  {
      RNOK( m_pcIntFrameBotField->uninitHalfPel() );
  }
  return Err::m_nOK;
}



ErrVal IntFrame::load( PicBuffer* pcPicBuffer )
{
  RNOK( m_cFullPelYuvBuffer.loadFromPicBuffer( pcPicBuffer ) );
  return Err::m_nOK;
}

ErrVal IntFrame::store( PicBuffer* pcPicBuffer )
{
  RNOK( m_cFullPelYuvBuffer.storeToPicBuffer( pcPicBuffer ) );
  return Err::m_nOK;
}

IntFrame* IntFrame::getPic( PicType ePicType)//th
{
    switch( ePicType )
    {
    case FRAME:
        return this;
    case TOP_FIELD:
        ASSERT( m_pcIntFrameTopField != NULL );
        return m_pcIntFrameTopField;
    case BOT_FIELD:
        ASSERT( m_pcIntFrameBotField != NULL );
        return m_pcIntFrameBotField;
    default:
        return NULL;
    }
    return NULL;
}

ErrVal IntFrame::removeFieldBuffers()//th
{
#ifdef EXT_CHECK_1_GOP
    getFullPelYuvBuffer()->m_bExtended = false;
#endif
    if( NULL != m_pcIntFrameTopField )
    {
        m_pcIntFrameTopField->uninit();
        delete m_pcIntFrameTopField;
        m_pcIntFrameTopField = NULL;
    }

    if( NULL != m_pcIntFrameBotField )
    {
        m_pcIntFrameBotField->uninit();
        delete m_pcIntFrameBotField;
        m_pcIntFrameBotField = NULL;
    }
    return Err::m_nOK;
}


ErrVal IntFrame::addFieldBuffer( PicType ePicType)//th 
{
    AOT( m_ePicType != FRAME );

    if( ePicType == TOP_FIELD)
    {
        if( NULL != m_pcIntFrameTopField )
        {
            RNOK( m_pcIntFrameTopField->uninit() );
        }
        if( NULL == m_pcIntFrameTopField )
        {
            AOT( NULL != m_pcIntFrameTopField );

            ROT( NULL == ( m_pcIntFrameTopField = new IntFrame( getFullPelYuvBuffer()->getBufferCtrl(), 
                getHalfPelYuvBuffer()->getBufferCtrl(), 
                TOP_FIELD )));
        }
        XPel* pcBuffer = getFullPelYuvBuffer()->getBuffer();
        RNOK( m_pcIntFrameTopField->getFullPelYuvBuffer()->init( pcBuffer ));
        RNOK( m_pcIntFrameTopField->getFullPelYuvBuffer()->fillMargin());

        m_pcIntFrameTopField->setPoc( m_iTopFieldPoc );
        //   m_pcIntFrameTopField->setLongTerm( m_bLongTerm );
    }
    else if( ePicType == BOT_FIELD)
    {
        if( NULL != m_pcIntFrameBotField )
        {
            RNOK( m_pcIntFrameBotField->uninit() );
        }
        if( NULL == m_pcIntFrameBotField )
        {
            AOT( NULL != m_pcIntFrameBotField );

            ROT( NULL == ( m_pcIntFrameBotField = new IntFrame( getFullPelYuvBuffer()->getBufferCtrl(), 
                getHalfPelYuvBuffer()->getBufferCtrl(), 
                BOT_FIELD )));
        }

        XPel* pcBuffer = getFullPelYuvBuffer()->getBuffer();
        RNOK( m_pcIntFrameBotField->getFullPelYuvBuffer()->init( pcBuffer ));
        RNOK( m_pcIntFrameBotField->getFullPelYuvBuffer()->fillMargin());

        m_pcIntFrameBotField->setPoc( m_iBotFieldPoc );
        //   m_pcIntFrameBotField->setLongTerm( m_bLongTerm );
    }
    else
    {
        AOT(1);
    }

    return Err::m_nOK;
}

ErrVal IntFrame::removeFieldBuffer( PicType ePicType)//th
{
    AOT( m_ePicType != FRAME );

    if( ePicType == TOP_FIELD)
    {
        //otherwise the frame was not added using addFieldBuffer
        if( NULL != m_pcIntFrameTopField && NULL == m_pcIntFrameBotField)
        {
            m_pcIntFrameTopField->uninit();
            delete m_pcIntFrameTopField;
            m_pcIntFrameTopField = NULL;
        }
    }
    else if( ePicType == BOT_FIELD)
    {
        //otherwise the frame was not added using addFieldBuffer
        if( NULL != m_pcIntFrameBotField && NULL == m_pcIntFrameTopField)
        {
            m_pcIntFrameBotField->uninit();
            delete m_pcIntFrameBotField;
            m_pcIntFrameBotField = NULL;
        }
    }
    else
    {
        AOT(1);
    }

    return Err::m_nOK;
}

ErrVal IntFrame::removeFrameFieldBuffer()
{
    ASSERT( m_ePicType==FRAME );

    RNOK( removeFieldBuffer( TOP_FIELD ) );
    RNOK( removeFieldBuffer( BOT_FIELD ) );

    return Err::m_nOK;
}

ErrVal IntFrame::addFrameFieldBuffer()//th
{
//    if( NULL != m_pcIntFrameTopField && NULL != m_pcIntFrameBotField)
//    {
//#ifdef EXT_CHECK_1_GOP
//        AOT(                       getFullPelYuvBuffer()->m_bExtended )
//            AOT( m_pcIntFrameTopField->getFullPelYuvBuffer()->m_bExtended )
//            AOT( m_pcIntFrameBotField->getFullPelYuvBuffer()->m_bExtended )
//#endif
//            m_pcIntFrameTopField->uninit();
//        m_pcIntFrameBotField->uninit();
//        XPel* pFullPel = getFullPelYuvBuffer()->getBuffer();
//        RNOK( m_pcIntFrameTopField->getFullPelYuvBuffer()->init( pFullPel ));
//        RNOK( m_pcIntFrameBotField->getFullPelYuvBuffer()->init( pFullPel ));
//        return Err::m_nOK;
//    }
//
//    if( NULL == m_pcIntFrameTopField || NULL == m_pcIntFrameBotField)
//    {
//        ROT( NULL != m_pcIntFrameTopField );
//        ROT( NULL != m_pcIntFrameTopField );
//        YuvBufferCtrl& rcYuvFullPelBufferCtrl = getFullPelYuvBuffer()->getBufferCtrl();
//        YuvBufferCtrl& rcYuvHalfPelBufferCtrl = getHalfPelYuvBuffer()->getBufferCtrl();
//
//        ROT( NULL == ( m_pcIntFrameTopField = new IntFrame( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, TOP_FIELD )));
//        ROT( NULL == ( m_pcIntFrameBotField = new IntFrame( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, BOT_FIELD )));
//        XPel* pFullPel = getFullPelYuvBuffer()->getBuffer();
//        RNOK( m_pcIntFrameTopField->getFullPelYuvBuffer()->init( pFullPel ));
//        RNOK( m_pcIntFrameBotField->getFullPelYuvBuffer()->init( pFullPel ));
//        m_pcIntFrameTopField->m_bExtended = false;
//        m_pcIntFrameBotField->m_bExtended = false;
//    }
//    return Err::m_nOK;

    ASSERT( m_ePicType==FRAME );

    RNOK( addFieldBuffer( TOP_FIELD ) );
    RNOK( addFieldBuffer( BOT_FIELD ) );

    return Err::m_nOK;
}

ErrVal IntFrame::extendFrame( QuarterPelFilter* pcQuarterPelFilter )
{
  Bool bNoHalfPel = ( NULL == pcQuarterPelFilter );
  m_bExtended     = true;
  
  // perform border padding on the full pel buffer
  RNOK( getFullPelYuvBuffer()->fillMargin( ) );

  // if cond is true no sub pel buffer is used
  ROTRS( bNoHalfPel, Err::m_nOK );

  // create half pel samples
  ANOK( pcQuarterPelFilter->filterFrame( getFullPelYuvBuffer(), getHalfPelYuvBuffer() ) );

  return Err::m_nOK;
}

ErrVal IntFrame::extendFrame( QuarterPelFilter* pcQuarterPelFilter, PicType ePicType, Bool bFrameMbsOnlyFlag )
{
    ASSERT( m_ePicType==FRAME );

    const Bool bNoHalfPel = ( NULL == pcQuarterPelFilter );

    if( NULL != m_pcIntFrameTopField || NULL != m_pcIntFrameBotField )
    {
        RNOK( removeFrameFieldBuffer() );
    }

    // perform border padding on the full pel buffer
    RNOK( getFullPelYuvBuffer()->fillMargin( ) );
    m_bExtended     = true;

    if( ! bFrameMbsOnlyFlag )
    {
        if( ePicType==FRAME )
        {
            if( NULL == m_pcIntFrameTopField || NULL == m_pcIntFrameBotField )
            {
                ROT( NULL != m_pcIntFrameTopField );
                ROT( NULL != m_pcIntFrameBotField );
    //            YuvBufferCtrl& rcYuvFullPelBufferCtrl = getFullPelYuvBuffer()->getBufferCtrl();
    //            YuvBufferCtrl& rcYuvHalfPelBufferCtrl = getHalfPelYuvBuffer()->getBufferCtrl();

    //            ROT( NULL == ( m_pcIntFrameTopField = new IntFrame( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, TOP_FIELD )));
    //            ROT( NULL == ( m_pcIntFrameBotField = new IntFrame( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, BOT_FIELD )));
    //            XPel* pFullPel = NULL;//getFullPelYuvBuffer()->getBuffer();
    //            RNOK( m_pcIntFrameTopField->getFullPelYuvBuffer()->init( pFullPel ));
				//pFullPel = NULL;
    //            RNOK( m_pcIntFrameBotField->getFullPelYuvBuffer()->init( pFullPel ));
                addFieldBuffer(TOP_FIELD);//lufeng: same buffer for field and frame
                addFieldBuffer(BOT_FIELD);

                m_pcIntFrameTopField->m_bExtended = false;
				m_pcIntFrameTopField->setPoc(m_iTopFieldPoc);
                m_pcIntFrameBotField->m_bExtended = false;
				m_pcIntFrameBotField->setPoc(m_iBotFieldPoc);
            }
        }
        else
        {
            RNOK( addFrameFieldBuffer() );
        }

		//lufeng
		m_pcIntFrameTopField->setDPBUnit(getDPBUnit());
		m_pcIntFrameBotField->setDPBUnit(getDPBUnit());
        m_pcIntFrameTopField->setViewId(getViewId());
        m_pcIntFrameBotField->setViewId(getViewId());
			
        // perform border padding on the full pel buffer
        RNOK( m_pcIntFrameTopField->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );
        RNOK( m_pcIntFrameBotField->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );
        m_pcIntFrameTopField->setExtended();
        m_pcIntFrameBotField->setExtended();

        if( ! bNoHalfPel )
        {
            RNOK( m_pcIntFrameTopField->initHalfPel(  ) );
            RNOK( m_pcIntFrameBotField->initHalfPel(  ) );
        }

    }

    // if cond is true no sub pel buffer is used
    ROTRS( bNoHalfPel, Err::m_nOK );

    // create half pel samples
    RNOK( pcQuarterPelFilter->filterFrame(                         getFullPelYuvBuffer(),                       getHalfPelYuvBuffer() ) );

    if( ! bFrameMbsOnlyFlag )
    {
        RNOK( pcQuarterPelFilter->filterFrame( m_pcIntFrameTopField->getFullPelYuvBuffer(), m_pcIntFrameTopField->getHalfPelYuvBuffer() ) );
        RNOK( pcQuarterPelFilter->filterFrame( m_pcIntFrameBotField->getFullPelYuvBuffer(), m_pcIntFrameBotField->getHalfPelYuvBuffer() ) );
    }

    return Err::m_nOK;
}

// JVT-R057 LA-RDO}
Void IntFrame::initChannelDistortion()
{
	if(!m_piChannelDistortion)
	{
		UInt  uiMbY  = getFullPelYuvBuffer()->getLHeight()/4;
		UInt  uiMbX  = getFullPelYuvBuffer()->getLWidth()/4;
		UInt  uiSize = uiMbX*uiMbY;
		m_piChannelDistortion= new UInt[uiSize];
	}
}





Void IntFrame::copyChannelDistortion(IntFrame*p1)
{
	UInt  uiMbY  = getFullPelYuvBuffer()->getLHeight()/16;
	UInt  uiMbX  = getFullPelYuvBuffer()->getLWidth()/16;
	for(UInt y=0;y<uiMbY*4;y++)
	{
		for(UInt x=0;x<uiMbX*4;x++)
		{ 
			m_piChannelDistortion[y*(uiMbX*4)+x]=p1->m_piChannelDistortion[y*(uiMbX*4)+x];
		}
	}
}




Void IntFrame::zeroChannelDistortion()
{
	UInt  uiMbY  = getFullPelYuvBuffer()->getLHeight()/16;
	UInt  uiMbX  = getFullPelYuvBuffer()->getLWidth()/16;
	for(UInt y=0;y<uiMbY*4;y++)
	{
		for(UInt x=0;x<uiMbX*4;x++)
		{ 
			m_piChannelDistortion[y*(uiMbX*4)+x]=0;
		}
	}
}

// JVT-R057 LA-RDO}

H264AVC_NAMESPACE_END



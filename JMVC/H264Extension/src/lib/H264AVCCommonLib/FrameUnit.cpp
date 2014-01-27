#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/FrameUnit.h"


H264AVC_NAMESPACE_BEGIN


FrameUnit::FrameUnit( YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, Bool bOriginal )
: m_cFrame         ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME )
, m_cTopField      ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, TOP_FIELD )
, m_cBotField      ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, BOT_FIELD )
, m_cResidual      ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME )
, m_iMaxPOC        ( MSYS_INT_MIN )
, m_eAvailable     ( NOT_SPECIFIED )
, m_ePicStruct     ( PS_NOT_SPECIFIED )     
, m_bFieldCoded    ( false )
, m_cMbDataCtrl    ( )
, m_pcPicBuffer    ( NULL )
, m_uiFrameNumber  ( MSYS_UINT_MAX )
, m_bOriginal      ( bOriginal )
, m_bInitDone      ( false )
, m_BaseRepresentation ( false )   //JVT-S036 lsj
{
    m_uiStatus = 0;
}

FrameUnit::~FrameUnit()
{

}

ErrVal FrameUnit::create( FrameUnit*& rpcFrameUnit, YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& 

                         rcYuvHalfPelBufferCtrl, Bool bOriginal )
{
    rpcFrameUnit = new FrameUnit( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, bOriginal);
    ROT( NULL == rpcFrameUnit );
    return Err::m_nOK;
}


ErrVal FrameUnit::destroy()
{
    AOT_DBG( m_bInitDone );

    delete this;
    return Err::m_nOK;
}


ErrVal FrameUnit::init( const SliceHeader& rcSH, PicBuffer *pcPicBuffer )
{
    ROT( NULL == pcPicBuffer );
    ROT( NULL != m_pcPicBuffer );

    m_pcPicBuffer   = pcPicBuffer;
    m_uiFrameNumber = rcSH.getFrameNum();

    RNOK( m_cFrame.   init( m_pcPicBuffer->getBuffer(), this ) );
    RNOK( m_cTopField.init( m_pcPicBuffer->getBuffer(), this ) );
    RNOK( m_cBotField.init( m_pcPicBuffer->getBuffer(), this ) );
    m_ePicStruct    = m_pcPicBuffer->getPicStruct();
    m_pcPicBuffer->setUsed();

#ifdef LF_INTERLACE_FIX
    m_bOriginal = (NULL == &rcSH.getSPS());
#endif //LF_INTERLACE_FIX
    ROTRS( m_bOriginal, Err::m_nOK );
    RNOK( m_cMbDataCtrl.init( rcSH.getSPS() ) );

    m_uiStatus   = 0;
    m_eAvailable = NOT_SPECIFIED;
    m_iMaxPOC    = 0;

    UInt uiStamp = max( m_cFrame.stamp(), max( m_cTopField.stamp(), m_cBotField.stamp() ) ) + 1;
    m_cTopField.stamp() = uiStamp;
    m_cBotField.stamp() = uiStamp;
    m_cFrame.   stamp() = uiStamp;

    m_bInitDone = true;

    m_bConstrainedIntraPred = rcSH.getPPS().getConstrainedIntraPredFlag();

    m_cMbDataCtrl.initFgsBQData(m_cMbDataCtrl.getSize());

    return Err::m_nOK;
}


// HS: decoder robustness
ErrVal FrameUnit::init( const SliceHeader& rcSH, FrameUnit& rcFrameUnit )
{
    ROT( NULL != m_pcPicBuffer );

    m_uiFrameNumber = rcSH.getFrameNum();

    RNOK( m_cFrame.   init( NULL, this ) );
    m_cFrame.getFullPelYuvBuffer()->copy( rcFrameUnit.getFrame().getFullPelYuvBuffer() );
    m_cFrame.getFullPelYuvBuffer()->fillMargin();
    RNOK( m_cTopField.init( NULL, this ) );
    RNOK( m_cBotField.init( NULL, this ) );
    m_cTopField.getFullPelYuvBuffer()->copy( rcFrameUnit.getTopField().getFullPelYuvBuffer() );
    m_cBotField.getFullPelYuvBuffer()->copy( rcFrameUnit.getBotField().getFullPelYuvBuffer() );
    m_cTopField.getFullPelYuvBuffer()->fillMargin();
    m_cBotField.getFullPelYuvBuffer()->fillMargin();

    RNOK( m_cMbDataCtrl.init( rcSH.getSPS() ) );

    m_uiStatus = 0;
    m_eAvailable = NOT_SPECIFIED;
    m_iMaxPOC    = 0;

    UInt uiStamp = max( m_cFrame.stamp(), max( m_cTopField.stamp(), m_cBotField.stamp() ) ) + 1;
    m_cTopField.stamp() = uiStamp;
    m_cBotField.stamp() = uiStamp;
    m_cFrame.   stamp() = uiStamp;

    m_bInitDone = true;

    m_bConstrainedIntraPred = rcSH.getPPS().getConstrainedIntraPredFlag();

    setOutputDone();

    return Err::m_nOK;
}

Void FrameUnit::setTopFieldPoc( Int iPoc )
{
    m_cTopField.setPOC( iPoc );
    m_cFrame   .setPOC( m_cBotField.isPOCAvailable() ? min( m_cBotField.getPOC(), iPoc ) : iPoc );
    m_iMaxPOC   =     ( m_cBotField.isPOCAvailable() ? max( m_cBotField.getPOC(), iPoc ) : iPoc );
}

Void FrameUnit::setBotFieldPoc( Int iPoc )
{
    m_cBotField.setPOC( iPoc );
    m_cFrame   .setPOC( m_cTopField.isPOCAvailable() ? min( m_cTopField.getPOC(), iPoc ) : iPoc );
    m_iMaxPOC   =     ( m_cTopField.isPOCAvailable() ? max( m_cTopField.getPOC(), iPoc ) : iPoc );
}

//JVT-S036  start //this two funcs not treated
ErrVal FrameUnit::copyBase( const SliceHeader& rcSH, FrameUnit& rcFrameUnit )
{
    ROT( NULL != m_pcPicBuffer );

    m_uiFrameNumber = rcSH.getFrameNum();

    RNOK( m_cFrame.   init( NULL, this ) );
    m_cFrame.getFullPelYuvBuffer()->copy( rcFrameUnit.getFrame().getFullPelYuvBuffer() );
    m_cFrame.getFullPelYuvBuffer()->fillMargin();

    RNOK( m_cMbDataCtrl.init( rcSH.getSPS() ) );

    m_iMaxPOC = rcFrameUnit.getMaxPOC();
    m_uiStatus = rcFrameUnit.getStatus();  //JVT-S036 

    UInt uiStamp = m_cFrame.stamp() + 1;
    m_cFrame.   stamp() = uiStamp;
    m_cFrame.setPOC(rcFrameUnit.getFrame().getPOC());//JVT-S036 

    m_pcPicBuffer = rcFrameUnit.getPicBuffer();
    m_pcPicBuffer->setUsed(); //JVT-S036 

    //  m_cResidual.init( false );
    //  m_cResidual.getFullPelYuvBuffer()->clear();
    m_bInitDone = true;

    //  m_cFGSIntFrame.init( false);
    //  m_pcFGSPicBuffer = NULL;

    m_bConstrainedIntraPred = rcSH.getPPS().getConstrainedIntraPredFlag();

    return Err::m_nOK;
}
ErrVal FrameUnit::uninitBase()  
{ 
    m_cFrame.uninit();
    m_pcPicBuffer->setUnused();
    m_pcPicBuffer = NULL;

    return Err::m_nOK;
}
//JVT-S036  end

ErrVal FrameUnit::uninit()
{
    m_uiStatus = 0;

    m_pcPicBuffer = NULL;

    m_cMbDataCtrl.uninitFgsBQData();

    m_uiFrameNumber = 0;
    m_uiStatus      = 0;
    m_iMaxPOC       = 0;

    if( ! m_bOriginal )
    {
        RNOK( m_cMbDataCtrl.uninit() );
    }
    RNOK( m_cFrame.uninit() );
    RNOK( m_cTopField.uninit() );
    RNOK( m_cBotField.uninit() );
    m_bInitDone = false;
    return Err::m_nOK;
}

RefPic FrameUnit::getRefPic( PicType ePicType, const RefPic& rcRefPic ) const
{
    const Frame& rcPic = getPic( ePicType );

    if( rcPic.stamp() == rcRefPic.getStamp() )
    {
        return RefPic( &rcPic, rcPic.stamp() );
    }
    return RefPic( &rcPic, 0 );
}

Void FrameUnit::setUnused( PicType ePicType )
{
    m_uiStatus &= ~(ePicType + ( ePicType << 2));
    getPic(ePicType).stamp()++;
    if( ePicType == FRAME )
    {
        getTopField().stamp()++;
        getBotField().stamp()++;
    }
    else
    {
        getFrame().stamp()++;
    }
}

Void FrameUnit::addPic( PicType ePicType, Bool bFieldCoded, UInt uiIdrPicId )
{ 
    m_eAvailable = PicType( m_eAvailable + ePicType); 
    AOT_DBG( m_eAvailable > FRAME );

    m_bFieldCoded = bFieldCoded;

    ROTVS( m_ePicStruct >= PS_BOT_TOP ); // other values are set explicitly
    if( ePicType == FRAME )
    {
        m_ePicStruct = PS_FRAME;
    }
    else
    {
        if( m_eAvailable == FRAME )
        {
            m_ePicStruct = (( m_cTopField.getPOC() < m_cBotField.getPOC() ) ?  PS_TOP_BOT : PS_BOT_TOP);
        }
        else
        {
            m_ePicStruct = (( ePicType == TOP_FIELD ) ?  PS_TOP : PS_BOT);
        }
    }
}

H264AVC_NAMESPACE_END

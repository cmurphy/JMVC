#include "H264AVCDecoderLib.h"


#include "H264AVCDecoder.h"
//#include "GOPDecoder.h"
#include "ControlMngH264AVCDecoder.h"
#include "SliceReader.h"
#include "SliceDecoder.h"
#include "UvlcReader.h"
#include "MbParser.h"
#include "MbDecoder.h"
#include "NalUnitParser.h"
#include "BitReadBuffer.h"
#include "CabacReader.h"
#include "CabaDecoder.h"
//#include "FGSSubbandDecoder.h"

#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/IntraPrediction.h"
#include "H264AVCCommonLib/MotionCompensation.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/SampleWeighting.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/PocCalculator.h"

#include "CreaterH264AVCDecoder.h"

#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"



H264AVC_NAMESPACE_BEGIN


CreaterH264AVCDecoder::CreaterH264AVCDecoder():
  m_pcH264AVCDecoder      ( NULL ),
  m_pcFrameMng            ( NULL ),
  m_pcParameterSetMng     ( NULL ),
  m_pcSliceReader         ( NULL ),
  m_pcNalUnitParser       ( NULL ),
  m_pcSliceDecoder        ( NULL ),
  m_pcControlMng          ( NULL ),
  m_pcBitReadBuffer       ( NULL ),
  m_pcUvlcReader          ( NULL ),
  m_pcMbParser            ( NULL ),
  m_pcLoopFilter          ( NULL ),
  m_pcMbDecoder           ( NULL ),
  m_pcTransform           ( NULL ),
  m_pcIntraPrediction     ( NULL ),
  m_pcMotionCompensation  ( NULL ),
  m_pcQuarterPelFilter    ( NULL ),
  m_pcCabacReader         ( NULL ),
  m_pcSampleWeighting     ( NULL )
{
  //::memset( m_apcDecodedPicBuffer,     0x00, MAX_LAYERS * sizeof( Void* ) );
  //::memset( m_apcMCTFDecoder,          0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcPocCalculator,        0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcYuvFullPelBufferCtrl, 0x00, MAX_LAYERS * sizeof( Void* ) );
}




CreaterH264AVCDecoder::~CreaterH264AVCDecoder()
{
}

ErrVal
CreaterH264AVCDecoder::process( PicBuffer*     pcPicBuffer,
                             PicBufferList& rcPicBufferOutputList,
                             PicBufferList& rcPicBufferUnusedList,
                             PicBufferList& rcPicBufferReleaseList )
{
  return m_pcH264AVCDecoder->process( pcPicBuffer,
                                   rcPicBufferOutputList,
                                   rcPicBufferUnusedList,
                                   rcPicBufferReleaseList );
}


ErrVal
CreaterH264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor,
								                  UInt&             ruiNalUnitType,
								                  UInt&             uiMbX,
								                  UInt&             uiMbY,
								                  UInt&             uiSize
												  //,UInt&			  uiNonRequiredPic //NonRequired JVT-Q066
                                                  //JVT-P031
								                  ,Bool             bPreParseHeader //FRAG_FIX
								                  , Bool			bConcatenated //FRAG_FIX_3
                                  ,Bool&            rbStartDecoding,
                                UInt&             ruiStartPos,
                                UInt&             ruiEndPos,
                                Bool&              bFragmented,
                                Bool&              bDiscardable,
                                //~JVT-P031
                                UInt NumOfViewsInTheStream,	
								Bool&  bSkip								) 
{
	return m_pcH264AVCDecoder->initPacket( pcBinDataAccessor,
		                                      ruiNalUnitType,
		                                      uiMbX,
		                                      uiMbY,
											  uiSize
											  //,uiNonRequiredPic  //NonRequired JVT-Q066
                                              //JVT-P031
		                                      , bPreParseHeader //FRAG_FIX
		                                      , bConcatenated //FRAG_FIX_3
                                              ,rbStartDecoding,
                                              ruiStartPos,
                                              ruiEndPos,
                                              bFragmented,
                                              bDiscardable
                                              //~JVT-P031
											  ,UnitAVCFlag  //JVT-S036 lsj 
  											  , NumOfViewsInTheStream
                                              ,bSkip );
}

//JVT-S036  start
ErrVal
CreaterH264AVCDecoder::initPacketSuffix( BinDataAccessor*  pcBinDataAccessor,
								                  UInt&             ruiNalUnitType,
								         		  Bool             bPreParseHeader, //FRAG_FIX
								                  Bool			bConcatenated, //FRAG_FIX_3
												  Bool&			 rbStarDecoding
											      ,CreaterH264AVCDecoder* pcH264AVCDecoder
												  ,Bool&		SuffixEnable
                                  		) 
{
  return m_pcH264AVCDecoder->initPacketPrefix( pcBinDataAccessor,
                                               ruiNalUnitType
                                               , bPreParseHeader //FRAG_FIX
                                               , bConcatenated,//FRAG_FIX_3
                                               rbStarDecoding
                                               ,pcH264AVCDecoder->getH264AVCDecoder()->getSliceHeader()
                                               ,pcH264AVCDecoder->getNalUnitParser()
    );
}
//JVT-S036  end

//JVT-P031
ErrVal
CreaterH264AVCDecoder::initPacket ( BinDataAccessor*  pcBinDataAccessor)
{
  return( m_pcH264AVCDecoder->initPacket(pcBinDataAccessor) );
}
Void
CreaterH264AVCDecoder::decreaseNumOfNALInAU()
{
    m_pcH264AVCDecoder->decreaseNumOfNALInAU();
}
Void
CreaterH264AVCDecoder::setDependencyInitialized(Bool b)
{
    m_pcH264AVCDecoder->setDependencyInitialized(b);
}
UInt
CreaterH264AVCDecoder::getNumOfNALInAU()
{
    return m_pcH264AVCDecoder->getNumOfNALInAU();
}
Void CreaterH264AVCDecoder::initNumberOfFragment()
{
    m_pcH264AVCDecoder->initNumberOfFragment();
}
//~JVT-P031
//JVT-T054{
Void
CreaterH264AVCDecoder::setFGSRefInAU(Bool &b)
{ 
  m_pcH264AVCDecoder->setFGSRefInAU(b);
}
//JVT-T054}
ErrVal
CreaterH264AVCDecoder::checkSliceLayerDependency( BinDataAccessor*  pcBinDataAccessor,
                                                  Bool&             bFinishChecking )
{
	return m_pcH264AVCDecoder->checkSliceLayerDependency( pcBinDataAccessor, bFinishChecking
														 ,UnitAVCFlag   //JVT-S036 
													    );
}

//NonRequired JVT-Q066
UInt 
CreaterH264AVCDecoder::isNonRequiredPic()
{
	return m_pcH264AVCDecoder->isNonRequiredPic(); 
}
//TMM_EC {{
ErrVal 
CreaterH264AVCDecoder::checkSliceGap( BinDataAccessor*  pcBinDataAccessor,
                                      MyList<BinData*>&	cVirtualSliceList)
{
  return m_pcH264AVCDecoder->checkSliceGap( pcBinDataAccessor, cVirtualSliceList 
											,UnitAVCFlag			//JVT-S036 
										   );
}
ErrVal
CreaterH264AVCDecoder::setec( UInt uiErrorConceal)
{
  return m_pcH264AVCDecoder->setec( uiErrorConceal);
}
//TMM_EC }}


// FMO DECODE Init ICU/ETRI DS
Void	  
CreaterH264AVCDecoder::RoiDecodeInit() 
{
	m_pcH264AVCDecoder->RoiDecodeInit();
}

//frame crop
Void    
CreaterH264AVCDecoder::setCrop(UInt* uiCrop)
{
	int i;
	for(i=0;i<4;i++)
		*(uiCrop+i)=m_pcH264AVCDecoder->getCrop(i);
}

//JVT-V054
UInt*
CreaterH264AVCDecoder::getViewCodingOrder()
{
  return m_pcH264AVCDecoder->getViewOrder();
// Dec. 1 fix 
}

UInt*
CreaterH264AVCDecoder::getViewCodingOrder_SubStream()
{
  return m_pcH264AVCDecoder->getViewOrder_SubStream();
// Dec. 1 fix 
}

//lufeng: add vieworder
void
CreaterH264AVCDecoder::addViewCodingOrder()
{
	m_pcH264AVCDecoder->addViewOrder();
}

ErrVal CreaterH264AVCDecoder::create( CreaterH264AVCDecoder*& rpcCreaterH264AVCDecoder )
{
  rpcCreaterH264AVCDecoder = new CreaterH264AVCDecoder;
  ROT( NULL == rpcCreaterH264AVCDecoder );
  RNOK( rpcCreaterH264AVCDecoder->xCreateDecoder() )
  return Err::m_nOK;
}



ErrVal CreaterH264AVCDecoder::xCreateDecoder()
{
  RNOK( ParameterSetMng       ::create( m_pcParameterSetMng ) );
  RNOK( FrameMng              ::create( m_pcFrameMng ) );
  RNOK( BitReadBuffer         ::create( m_pcBitReadBuffer ) );
  RNOK( NalUnitParser         ::create( m_pcNalUnitParser) );
  RNOK( SliceReader           ::create( m_pcSliceReader ) );
  RNOK( SliceDecoder          ::create( m_pcSliceDecoder ) );
  RNOK( UvlcReader            ::create( m_pcUvlcReader ) );
  RNOK( CabacReader           ::create( m_pcCabacReader ) );
  RNOK( MbParser              ::create( m_pcMbParser ) );
  RNOK( MbDecoder             ::create( m_pcMbDecoder ) );
  RNOK( LoopFilter            ::create( m_pcLoopFilter ) );
  RNOK( IntraPrediction       ::create( m_pcIntraPrediction ) );
  RNOK( MotionCompensation    ::create( m_pcMotionCompensation ) );
  RNOK( H264AVCDecoder           ::create( m_pcH264AVCDecoder ) );  
  RNOK( ControlMngH264AVCDecoder ::create( m_pcControlMng ) );
  RNOK( ReconstructionBypass     ::create(m_pcReconstructionBypass) );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( PocCalculator       ::create( m_apcPocCalculator        [uiLayer] ) );
    RNOK( YuvBufferCtrl       ::create( m_apcYuvFullPelBufferCtrl [uiLayer] ) );
  }

  RNOK( SampleWeighting     ::create( m_pcSampleWeighting ) );
  RNOK( QuarterPelFilter    ::create( m_pcQuarterPelFilter ) );
  RNOK( Transform           ::create( m_pcTransform ) );

  return Err::m_nOK;
}


ErrVal CreaterH264AVCDecoder::destroy()
{
  RNOK( m_pcFrameMng              ->destroy() );
  RNOK( m_pcSliceDecoder          ->destroy() );
  RNOK( m_pcSliceReader           ->destroy() );
  RNOK( m_pcBitReadBuffer         ->destroy() );
  RNOK( m_pcUvlcReader            ->destroy() );
  RNOK( m_pcMbParser              ->destroy() );
  RNOK( m_pcLoopFilter            ->destroy() );
  RNOK( m_pcMbDecoder             ->destroy() );
  RNOK( m_pcTransform             ->destroy() );
  RNOK( m_pcIntraPrediction       ->destroy() );
  RNOK( m_pcMotionCompensation    ->destroy() );
  RNOK( m_pcQuarterPelFilter      ->destroy() );
  RNOK( m_pcCabacReader           ->destroy() );
  RNOK( m_pcNalUnitParser         ->destroy() );
  RNOK( m_pcParameterSetMng       ->destroy() );
  RNOK( m_pcSampleWeighting       ->destroy() );
  RNOK( m_pcH264AVCDecoder        ->destroy() );
  RNOK( m_pcControlMng            ->destroy() );
  RNOK( m_pcReconstructionBypass  ->destroy() );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( m_apcPocCalculator       [uiLayer]->destroy() );
    RNOK( m_apcYuvFullPelBufferCtrl[uiLayer]->destroy() );
  }

  delete this;
  return Err::m_nOK;
}


ErrVal CreaterH264AVCDecoder::init( Bool bOpenTrace, DecoderParameter *Dec_Param)
{
  if( bOpenTrace )
  {
    INIT_DTRACE(0, Dec_Param->getNumOfViews());
    OPEN_DTRACE;
  }
  
  UnitAVCFlag = false;   //JVT-S036 

  RNOK( m_pcBitReadBuffer         ->init() );
  RNOK( m_pcNalUnitParser         ->init( m_pcBitReadBuffer ));
  RNOK( m_pcUvlcReader            ->init( m_pcBitReadBuffer ) );
  RNOK( m_pcCabacReader           ->init( m_pcBitReadBuffer ) );
  RNOK( m_pcQuarterPelFilter      ->init() );
  RNOK( m_pcParameterSetMng       ->init() );
  RNOK( m_pcSampleWeighting       ->init() );
  RNOK( m_pcFrameMng              ->init( m_apcYuvFullPelBufferCtrl[0] ) );
  RNOK( m_pcSliceDecoder          ->init( m_pcMbDecoder,
                                          m_pcControlMng,
                                          m_pcTransform) );
  RNOK( m_pcSliceReader           ->init( m_pcUvlcReader,
                                          m_pcParameterSetMng,
                                          m_pcMbParser,
                                          m_pcControlMng ) );
  RNOK( m_pcMbParser              ->init( m_pcTransform  ) );
  RNOK( m_pcLoopFilter            ->init( m_pcControlMng , m_pcReconstructionBypass  ) );
     
  RNOK( m_pcIntraPrediction       ->init() );
  RNOK( m_pcMotionCompensation    ->init( m_pcQuarterPelFilter,
                                          m_pcTransform,
                                          m_pcSampleWeighting ) );
  RNOK( m_pcMbDecoder             ->init( m_pcTransform,
                                          m_pcIntraPrediction,
                                          m_pcMotionCompensation,
                                          m_pcFrameMng ) );

  RNOK( m_pcH264AVCDecoder           ->init( //m_apcMCTFDecoder,
                                          m_pcSliceReader,
                                          m_pcSliceDecoder,
                                          m_pcFrameMng,
                                          m_pcNalUnitParser,
                                          m_pcControlMng,
                                          m_pcLoopFilter,
                                          m_pcUvlcReader,
                                          m_pcParameterSetMng,
                                          m_apcPocCalculator[0],
                                          m_pcMotionCompensation) );

  RNOK( m_pcReconstructionBypass  ->init() );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    
  }

  RNOK( m_pcControlMng            ->init( m_pcFrameMng,
                                          m_pcParameterSetMng,
                                          m_apcPocCalculator,
                                          m_pcSliceReader,
                                          m_pcNalUnitParser,
                                          m_pcSliceDecoder,
                                          m_pcBitReadBuffer,
                                          m_pcUvlcReader,
                                          m_pcMbParser,
                                          m_pcLoopFilter,
                                          m_pcMbDecoder,
                                          m_pcTransform,
                                          m_pcIntraPrediction,
                                          m_pcMotionCompensation,
                                          m_apcYuvFullPelBufferCtrl,
                                          m_pcQuarterPelFilter,
                                          m_pcCabacReader,
                                          m_pcSampleWeighting,
                                          //m_apcMCTFDecoder,
                                          m_pcH264AVCDecoder ) );


  return Err::m_nOK;
}




ErrVal CreaterH264AVCDecoder::uninit( Bool bCloseTrace )
{
  RNOK( m_pcSampleWeighting       ->uninit() );
  RNOK( m_pcQuarterPelFilter      ->uninit() );
  RNOK( m_pcFrameMng              ->uninit() );
  RNOK( m_pcParameterSetMng       ->uninit() );
  RNOK( m_pcSliceDecoder          ->uninit() );
  RNOK( m_pcSliceReader           ->uninit() );
  RNOK( m_pcBitReadBuffer         ->uninit() );
  RNOK( m_pcUvlcReader            ->uninit() );
  RNOK( m_pcMbParser              ->uninit() );
  RNOK( m_pcLoopFilter            ->uninit() );
  RNOK( m_pcMbDecoder             ->uninit() );
  RNOK( m_pcIntraPrediction       ->uninit() );
  RNOK( m_pcMotionCompensation    ->uninit() );
  RNOK( m_pcCabacReader           ->uninit() );
  RNOK( m_pcH264AVCDecoder        ->uninit() );
//  RNOK( m_pcRQFGSDecoder          ->uninit() );
  RNOK( m_pcControlMng            ->uninit() );
  RNOK( m_pcReconstructionBypass  ->uninit() );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    //RNOK( m_apcDecodedPicBuffer    [uiLayer] ->uninit() );
    //RNOK( m_apcMCTFDecoder         [uiLayer] ->uninit() );
    RNOK( m_apcYuvFullPelBufferCtrl[uiLayer] ->uninit() );
  }


  if( bCloseTrace )
  {
    CLOSE_DTRACE;
  }

  return Err::m_nOK;
}



















H264AVCPacketAnalyzer::H264AVCPacketAnalyzer()
: m_pcBitReadBuffer       ( NULL )
, m_pcUvlcReader          ( NULL )
, m_pcNalUnitParser       ( NULL )
, m_pcNonRequiredSEI	  ( NULL )
, m_uiStdAVCOffset         ( 0 )
, m_bAVCCompatible			(false)//BUG FIX Kai Zhang
{
	for(int iLayer=0;iLayer<MAX_SCALABLE_LAYERS;iLayer++)
	{
		m_silceIDOfSubPicLayer[iLayer] = -1;
	}
}



H264AVCPacketAnalyzer::~H264AVCPacketAnalyzer()
{
}



ErrVal
H264AVCPacketAnalyzer::process( BinData*            pcBinData,
                                PacketDescription&  rcPacketDescription,
                                SEI::SEIMessage*&   pcScalableSEIMessage )
{
 
			
  pcScalableSEIMessage      = 0;
  UChar       ucByte        = (pcBinData->data())[0];
  NalUnitType eNalUnitType  = NalUnitType ( ucByte  & 0x1F );
  NalRefIdc   eNalRefIdc    = NalRefIdc   ( ucByte >> 5 );
  UInt        uiLayer       = 0;
  UInt        uiLevel       = 0;
  UInt        uiFGSLayer    = 0;
  Bool        bApplyToNext  = false;
  //{{Variable Lengh NAL unit header data with priority and dead substream flag
  //France Telecom R&D- (nathalie.cammas@francetelecom.com)
  UInt		  uiSimplePriorityId = 0;
  Bool		  bDiscardableFlag = false;
  Bool		  bReservedZeroBit = false; //JVT-S036 
//SEI {
  Bool		  bSvcMvcFlag = false;
  Bool		  bAnchorPicFlag = false;
  UInt		  uiViewId = 0;
//SEI 
  Bool bFragmentedFlag = false; //JVT-P031
  UInt uiFragmentOrder = 0; //JVT-P031
  //Bool bLastFragmentFlag = false; //JVT-P031
  rcPacketDescription.uiNumLevelsQL = 0;
  
	for(UInt ui = 0; ui < MAX_NUM_RD_LEVELS; ui++)
	{
		rcPacketDescription.auiDeltaBytesRateOfLevelQL[ui] = 0;
		rcPacketDescription.auiQualityLevelQL[ui] = 0;
	}
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
  Bool        bScalable     = ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
                                eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE       );
  UInt        uiSPSid       = 0;
  UInt        uiPPSid       = 0;
  Bool        bParameterSet = ( eNalUnitType == NAL_UNIT_SPS                      || eNalUnitType == NAL_UNIT_SUBSET_SPS ||
                                eNalUnitType == NAL_UNIT_PPS                        );


  if( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||
      eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE   )
  {
    ucByte             = (pcBinData->data())[1];
	  uiSimplePriorityId = ( ucByte >> 2);
	  bDiscardableFlag	 = ( ucByte >> 1) & 1;
  	bSvcMvcFlag = ( ( ucByte >> 7 ) !=0);
	  if( !bSvcMvcFlag )
	  {
                                                    // 1 bit
      //Bool bNonIDRFlag     = ( ucByte >> 6)  & 0x01;     // 1 bit
      uiSimplePriorityId   = ( ucByte     ) & 0x3f ;    // 6
      // view_id
      ucByte               = pcBinData->data()[2];
      uiViewId             = ( ucByte     ) & 0xff ;    // 8 bit first
      uiViewId           <<=2;
      ucByte               = pcBinData->data()[3];
      uiViewId            += ( ucByte >>6  ) & 0x03;     // 2 bit more

      //UInt  uiTemporalId   = ( ucByte >>3 )  & 0x07;     // 3 bit 
		  bAnchorPicFlag       = ( ucByte >>2 )  & 0x01;     // 1 bit
		  //Bool  bInterViewFlag = ( ucByte >>1	)	 & 0x01;     // 1 bit  

		  //Bool  b_Res1Zero     = ( ucByte     )  & 0x01;           // 1 bit
		}
    else
    {
// SVC ... ignore	    
      uiSimplePriorityId = ( ucByte >> 1) & 63;
      bDiscardableFlag	 = ( ucByte ) & 1;
      ucByte      = pcBinData->data()[2];
      bReservedZeroBit   = ( ucByte     ) & 1; 
      uiLevel     = ( ucByte >> 4 ) & 7;
      uiLayer     = ( ucByte >> 1 ) & 7;
      uiFGSLayer  = ( ucByte      ) & 1;
      ucByte      = pcBinData->data()[3];
      uiFGSLayer  = uiFGSLayer*2 + ( ucByte >> 7);
    }
  }
  else if( eNalUnitType == NAL_UNIT_CODED_SLICE     ||
           eNalUnitType == NAL_UNIT_CODED_SLICE_IDR   )
  {
    uiLevel     = ( eNalRefIdc > 0 ? 0 : 1+m_uiStdAVCOffset);
	  m_bAVCCompatible=true;//BUG FIX Kai Zhang
  }
  else if( eNalUnitType == NAL_UNIT_SEI )
  {
    UInt32* pulData = (UInt32*)( pcBinData->data() + 1 );
    UInt    uiSize  =     8 * ( pcBinData->size() - 1 ) - 1;
    RNOK( m_pcBitReadBuffer->initPacket( pulData, uiSize ) );

    uiSize = pcBinData->byteSize();
    BinData cBinData( new UChar[uiSize], uiSize );
    memcpy( cBinData.data(), pcBinData->data(), uiSize );
    BinDataAccessor cBinDataAccessor;
    cBinData.setMemAccessor( cBinDataAccessor );

    UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
    RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor, NULL, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC
    SEI::MessageList cMessageList;
    //RNOK( SEI::read( m_pcUvlcReader, cMessageList ) );

	/* SEI JVT-W060 */
  	RNOK( SEI::read( m_pcUvlcReader, cMessageList /*, Save_NumViewsMinus1*/ ) ); // Nov. 30
	/* ~SEI JVT-W060 */
   
    SEI::MessageList::iterator iter = cMessageList.begin();
    while( ! cMessageList.empty() )
    {
      SEI::SEIMessage* pcSEIMessage = cMessageList.popBack();

      switch( pcSEIMessage->getMessageType() )
      {
        case SEI::VIEW_SCALABILITY_INFO_SEI:
        {
          pcScalableSEIMessage = pcSEIMessage;
          bApplyToNext = true;
          break;
        }
        case SEI::SUB_SEQ_INFO:
        {
          SEI::SubSeqInfo* pcSubSeqInfo = (SEI::SubSeqInfo*) pcSEIMessage;
          uiLevel       = pcSubSeqInfo->getSubSeqLayerNum();
          uiLayer       = 0;
          bApplyToNext  = true;
          delete pcSEIMessage;
          break;
        }
        case SEI::SCALABLE_SEI:
        {
	        uiLevel = 0;
	        uiLayer = 0;
	        pcScalableSEIMessage = pcSEIMessage;
	        {
		        //====set parameters used for further parsing =====
		        SEI::ScalableSei* pcSEI		= (SEI::ScalableSei*)pcSEIMessage;
		        UInt uiNumScalableLayers  = pcSEI->getNumLayersMinus1() + 1;
		        for(UInt uiIndex = 0; uiIndex < uiNumScalableLayers; uiIndex++ )
		        {
			        if( pcSEI->getDependencyId( uiIndex ) == 0 )
			        {
        // BUG_FIX liuhui{
				        m_uiStdAVCOffset = pcSEI->getTemporalLevel( uiIndex );
				        pcSEI->setStdAVCOffset( m_uiStdAVCOffset );
				        break;
        // BUG_FIX liuhui}
			        }
			        else
				        break;
		        }
	        }

		        SEI::ScalableSei* pcSEI		= (SEI::ScalableSei*)pcSEIMessage;
		        m_uiNum_layers = pcSEI->getNumLayersMinus1() + 1;   		
		        for(int i=0; i< m_uiNum_layers; i++)
	        {				  
		        m_ID_ROI[i] = pcSEI->getRoiId(i);
		        m_ID_Dependency[i] = pcSEI->getDependencyId(i);
	        }
	        break;
        }

        case SEI::MOTION_SEI:
        {
	        SEI::MotionSEI* pcSEI           = (SEI::MotionSEI*)pcSEIMessage;

	        m_silceIDOfSubPicLayer[m_layer_id] = pcSEI->m_slice_group_id[0];
	        break;
        }

      // JVT-S080 LMI {
        case SEI::SCALABLE_SEI_LAYERS_NOT_PRESENT:
        case SEI::SCALABLE_SEI_DEPENDENCY_CHANGE:
        {
	        pcScalableSEIMessage = pcSEIMessage;
	        break;
        }
        // JVT-S080 LMI }
        case SEI::SUB_PIC_SEI:
        {
	        SEI::SubPicSei* pcSEI    = (SEI::SubPicSei*)pcSEIMessage;
	        m_layer_id					= pcSEI->getLayerId();
	        bApplyToNext  = true;
                break;
        }
      
        case SEI::NON_REQUIRED_SEI: 
        {
	        m_pcNonRequiredSEI = (SEI::NonRequiredSei*) pcSEIMessage;
	        m_uiNonRequiredSeiFlag = 1;  
	        break;
        }
      //SEI {
        case SEI::SCALABLE_NESTING_SEI:
        {
          Bool bAllPicturesInAuFlag;
          UInt uiNumPicturesMinus1;
          UInt *puiPicId, uiTemporalId;
          SEI::ScalableNestingSei* pcSEI = (SEI::ScalableNestingSei*)pcSEIMessage;
          bAllPicturesInAuFlag = pcSEI->getAllPicturesInAuFlag();
          if( bAllPicturesInAuFlag == 0 )
          {
            uiNumPicturesMinus1 = pcSEI->getNumPicturesMinus1();
            puiPicId = new UInt[uiNumPicturesMinus1+1];
            for( UInt uiIndex = 0; uiIndex <= uiNumPicturesMinus1; uiIndex++ )
            {
              puiPicId[uiIndex] = pcSEI->getPicId(uiIndex);
            }

            uiTemporalId = pcSEI->getTemporalId();

            delete puiPicId;
          }
                bApplyToNext = true;
          break;
        }

      //SEI }
        case SEI::MULTIVIEW_SCENE_INFO_SEI: // SEI JVT-W060
        {
          UInt uiMaxDisparity;

	        SEI::MultiviewSceneInfoSei* pcSEI = (SEI::MultiviewSceneInfoSei*)pcSEIMessage;
	        uiMaxDisparity = pcSEI->getMaxDisparity();
          bApplyToNext = true;
          break;
        }
        case SEI::MULTIVIEW_ACQUISITION_INFO_SEI: // SEI JVT-W060
        {
          // I comment the following line to remove the warning
          //SEI::MultiviewAcquisitionInfoSei* pcSEI = (SEI::MultiviewAcquisitionInfoSei*)pcSEIMessage; 
          // This is introduced by MERL, it should be checked
          // ying
          bApplyToNext = true;
          break;
        }
      default:
        {
          delete pcSEIMessage;
        }
      }
    }
    m_pcNalUnitParser->closeNalUnit();
  }

  if( eNalUnitType != NAL_UNIT_SEI )
  {
    UInt32*  pulData = (UInt32*)( pcBinData->data() + 1 );
    UInt    uiSize  =     8 * ( pcBinData->size() - 1 ) - 1;
    RNOK( m_pcBitReadBuffer->initPacket( pulData, uiSize ) );

    uiSize = pcBinData->byteSize();
    BinData cBinData( new UChar[uiSize], uiSize );
    memcpy( cBinData.data(), pcBinData->data(), uiSize );
    BinDataAccessor cBinDataAccessor;
    cBinData.setMemAccessor( cBinDataAccessor );
    m_pcNalUnitParser->setCheckAllNALUs(true); //JVT-P031
	  UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC

  	RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor, NULL, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC

    m_pcNalUnitParser->setCheckAllNALUs(false);//JVT-P031
  
    // get the SPSid
    if(eNalUnitType == NAL_UNIT_SPS || eNalUnitType == NAL_UNIT_SUBSET_SPS)
    {
      SequenceParameterSet* pcSPS = NULL;
      RNOK( SequenceParameterSet::create  ( pcSPS   ) );
	  
      RNOK( pcSPS->read( m_pcUvlcReader, eNalUnitType ) );
      // Copy simple priority ID mapping from SPS
      uiSPSid = pcSPS->getSeqParameterSetId();
	  
      pcSPS->destroy();
    }
    // get the PPSid and the referenced SPSid
    else if( eNalUnitType == NAL_UNIT_PPS )
    {
      PictureParameterSet* pcPPS = NULL;
      RNOK( PictureParameterSet::create  ( pcPPS   ) );
      RNOK( pcPPS->read( m_pcUvlcReader, eNalUnitType ) );
      uiPPSid = pcPPS->getPicParameterSetId();
      uiSPSid = pcPPS->getSeqParameterSetId();

	  // FMO ROI ICU/ETRI
	  m_uiNumSliceGroupsMinus1 = pcPPS->getNumSliceGroupsMinus1();
   
	  for(UInt i=0; i<=m_uiNumSliceGroupsMinus1; i++)
	  {
		 uiaAddrFirstMBofROIs[uiPPSid ][i] = pcPPS->getTopLeft (i);
		 uiaAddrLastMBofROIs[uiPPSid ][i]  = pcPPS->getBottomRight (i);
	  }

      pcPPS->destroy();
      rcPacketDescription.SPSidRefByPPS[uiPPSid] = uiSPSid;
    }
    // get the PPSid and SPSid referenced by the slice header
    else if(  eNalUnitType == NAL_UNIT_CODED_SLICE              ||
              eNalUnitType == NAL_UNIT_CODED_SLICE_IDR          ||
              eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||
              eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE   )
    {
//BUG FIX Kai Zhang{
	    if(!(uiLayer == 0 && uiFGSLayer == 0 && m_bAVCCompatible&&
			    (eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE||eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE)))
	    {
          UInt uiTemp;
          //JVT-P031
        RNOK( m_pcUvlcReader->getUvlc( uiTemp,  "SH: first_mb_in_slice" ) );
	    // FMO ROI ICU/ETRI
  	    rcPacketDescription.uiFirstMb = uiTemp;
        RNOK( m_pcUvlcReader->getUvlc( uiTemp,  "SH: slice_type" ) );
    //JVT-T054{
		    rcPacketDescription.bEnableQLTruncation = false;
    //JVT-T054}
      
        if(uiFragmentOrder == 0)
        {
            RNOK( m_pcUvlcReader->getUvlc( uiPPSid, "SH: pic_parameter_set_id" ) );
            uiSPSid = rcPacketDescription.SPSidRefByPPS[uiPPSid];
        }     
    		
        //~JVT-P031
		    m_uiCurrPicLayer = (uiLayer << 4) + uiFGSLayer;
	      if(m_uiCurrPicLayer <= m_uiPrevPicLayer && m_uiNonRequiredSeiFlag != 1)
	      {
		      m_pcNonRequiredSEI->destroy();
		      m_pcNonRequiredSEI = NULL;
	      }          
	      m_uiNonRequiredSeiFlag = 0;
	      m_uiPrevPicLayer = m_uiCurrPicLayer;
	    }
//BUG_FIX Kai Zhang}
    }
    m_pcNalUnitParser->closeNalUnit();
  }

  rcPacketDescription.NalUnitType   = eNalUnitType;
  rcPacketDescription.SPSid         = uiSPSid;
  rcPacketDescription.PPSid         = uiPPSid;

  rcPacketDescription.Scalable      = bScalable;
  rcPacketDescription.ParameterSet  = bParameterSet;
  rcPacketDescription.Layer         = uiLayer;
  rcPacketDescription.FGSLayer      = uiFGSLayer;
  rcPacketDescription.Level         = uiLevel;
  rcPacketDescription.ApplyToNext   = bApplyToNext;
  rcPacketDescription.uiPId         = uiSimplePriorityId;
  rcPacketDescription.bDiscardable  = bDiscardableFlag;//JVT-P031
  rcPacketDescription.bFragmentedFlag   = bFragmentedFlag;//JVT-P031
  rcPacketDescription.NalRefIdc     = eNalRefIdc;
//SEI {
  rcPacketDescription.bAnchorPicFlag = bAnchorPicFlag;
  rcPacketDescription.bSvcMvcFlag   = bSvcMvcFlag;
  rcPacketDescription.ViewId		= uiViewId;
//SEI }
  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::create( H264AVCPacketAnalyzer*& rpcH264AVCPacketAnalyzer )
{
  rpcH264AVCPacketAnalyzer = new H264AVCPacketAnalyzer;
  ROT( NULL == rpcH264AVCPacketAnalyzer );
  RNOK( rpcH264AVCPacketAnalyzer->xCreate() )
  return Err::m_nOK;
}
ErrVal     
H264AVCPacketAnalyzer::isMVCProfile ( BinData*				pcBinData, Bool& b )// fix Nov. 30
{
  UChar       ucByte        = (pcBinData->data())[0];
  NalUnitType eNalUnitType  = NalUnitType ( ucByte  & 0x1F );
  
  if ( eNalUnitType != NAL_UNIT_SPS && eNalUnitType != NAL_UNIT_SUBSET_SPS )
	return Err::m_nERR; 

  UInt32*  pulData = (UInt32*)( pcBinData->data() + 1 );
  UInt    uiSize  =     8 * ( pcBinData->size() - 1 ) - 1;
  RNOK( m_pcBitReadBuffer->initPacket( pulData, uiSize ) );

  uiSize = pcBinData->byteSize();
  BinData cBinData( new UChar[uiSize], uiSize );
  memcpy( cBinData.data(), pcBinData->data(), uiSize );
  BinDataAccessor cBinDataAccessor;
  cBinData.setMemAccessor( cBinDataAccessor );
  m_pcNalUnitParser->setCheckAllNALUs(true); //JVT-P031
  UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC

  RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor, NULL, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC

  m_pcNalUnitParser->setCheckAllNALUs(false);//JVT-P031

  SequenceParameterSet* pcSPS = NULL;
  RNOK( SequenceParameterSet::create  ( pcSPS   ) );

  RNOK( pcSPS->read( m_pcUvlcReader, eNalUnitType ) );

  b= (MULTI_VIEW_PROFILE == pcSPS->getProfileIdc()
	  || STEREO_HIGH_PROFILE == pcSPS->getProfileIdc());
  
  pcSPS->destroy();
  
  return Err::m_nOK;
}

ErrVal
H264AVCPacketAnalyzer::processSEIAndMVC( BinData*				pcBinData,
								   SEI::ViewScalabilityInfoSei*&		pcSEIMessage )
{
  UChar       ucByte        = (pcBinData->data())[0];
  NalUnitType eNalUnitType  = NalUnitType ( ucByte  & 0x1F );
  Bool        bApplyToNext  = false;

  if( eNalUnitType == NAL_UNIT_SEI )
  {
    UInt32*  pulData = (UInt32*)( pcBinData->data() + 1 );
    UInt    uiSize  =     8 * ( pcBinData->size() - 1 ) - 1;
    RNOK( m_pcBitReadBuffer->initPacket( pulData, uiSize ) );

    uiSize = pcBinData->byteSize();
    BinData cBinData( new UChar[uiSize], uiSize );
    memcpy( cBinData.data(), pcBinData->data(), uiSize );
    BinDataAccessor cBinDataAccessor;
    cBinData.setMemAccessor( cBinDataAccessor );

    UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
    RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor, NULL, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC
    SEI::MessageList cMessageList;
    RNOK( SEI::read( m_pcUvlcReader, cMessageList ) );

    SEI::MessageList::iterator iter = cMessageList.begin();
    while( ! cMessageList.empty() )
    {
      SEI::SEIMessage* pcSEIMessage1 = cMessageList.popBack();

      switch( pcSEIMessage1->getMessageType() )
      {
	  case SEI::VIEW_SCALABILITY_INFO_SEI:
      {
		pcSEIMessage = (SEI::ViewScalabilityInfoSei* )pcSEIMessage1;

        bApplyToNext = true;
        break;
      }
      default:
        {
          delete pcSEIMessage1;
        }
      }
    }
    m_pcNalUnitParser->closeNalUnit();
  }
  //else if( eNalUnitType == NAL_UNIT_SPS && pcSEIMessage )
  else if( eNalUnitType == NAL_UNIT_SUBSET_SPS && pcSEIMessage )
  {
    UInt32*  pulData = (UInt32*)( pcBinData->data() + 1 );
    UInt    uiSize  =     8 * ( pcBinData->size() - 1 ) - 1;
    RNOK( m_pcBitReadBuffer->initPacket( pulData, uiSize ) );

    uiSize = pcBinData->byteSize();
    BinData cBinData( new UChar[uiSize], uiSize );
    memcpy( cBinData.data(), pcBinData->data(), uiSize );
    BinDataAccessor cBinDataAccessor;
    cBinData.setMemAccessor( cBinDataAccessor );
    m_pcNalUnitParser->setCheckAllNALUs(true); //JVT-P031
	  UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC

  	RNOK( m_pcNalUnitParser->initNalUnit( &cBinDataAccessor, NULL, uiNumBytesRemoved ) ); //FIX_FRAG_CAVLC

    m_pcNalUnitParser->setCheckAllNALUs(false);//JVT-P031
  
    SequenceParameterSet* pcSPS = NULL;
    RNOK( SequenceParameterSet::create  ( pcSPS   ) );
	pcSPS->SpsMVC = new SpsMvcExtension;	
 
	RNOK( pcSPS->read( m_pcUvlcReader, eNalUnitType ) );
	//modify the view scalability information sei message
	UInt uiNumAllViews = pcSPS->SpsMVC->getNumViewMinus1() + 1;
	UInt* ViewOp;
	ViewOp = new UInt [uiNumAllViews];
	UInt uiNumOp = pcSEIMessage->getNumOperationPointsMinus1();
	UInt i;
	for( i = 0; i <= uiNumOp; i++ )
	{
		UInt uiNumViews = pcSEIMessage->getNumTargetOutputViewsMinus1( i );//SEI JJ
		for( UInt j = 0; j <= uiNumViews; j++ )
		{
			UInt uiViewId = pcSEIMessage->getViewId(i, j); 
			ViewOp[uiViewId] = i;
		}
	}

	for( i = 0; i <= uiNumOp; i++ )
	{
		UInt uiNumViews = pcSEIMessage->getNumTargetOutputViewsMinus1( i );//SEI JJ
		for( UInt j = 0; j <= uiNumViews; j++ )
		{
			UInt uiViewId = pcSEIMessage->getViewId(i, j); 

			UInt uiNumRef = pcSPS->SpsMVC->getNumRefsForListX( uiViewId, LIST_0, true ) 
				+ pcSPS->SpsMVC->getNumRefsForListX( uiViewId, LIST_1, true );
			UInt uiNumRef1 = pcSPS->SpsMVC->getNumRefsForListX( uiViewId, LIST_0, false ) 
				+ pcSPS->SpsMVC->getNumRefsForListX( uiViewId, LIST_1, false );
			if( uiNumRef1 > uiNumRef )
				uiNumRef = uiNumRef1;

			if( uiNumRef == 0 )
				continue;

			if( !pcSEIMessage->getViewDependencyInfoPresentFlag(i) && !pcSEIMessage->getViewDependencyInfoSrcOpId(i))//SEI 
			{
				pcSEIMessage->setNumDirectlyDependentViews(i, uiNumRef );//SEI JJ
				pcSEIMessage->setViewDependencyInfoPresentFlag(i, true);//SEI JJ 
			}
			else if( pcSEIMessage->getNumDirectlyDependentViews(i) < uiNumRef )//SEI JJ
				pcSEIMessage->setNumDirectlyDependentViews(i, uiNumRef );//SEI JJ 

			for( UInt ui = 0; ui < uiNumRef; ui++ )
			{
				UInt uiRef;
				if( ui< pcSPS->SpsMVC->getNumRefsForListX( uiViewId, LIST_0, true ) )
					uiRef = pcSPS->SpsMVC->getAnchorRefForListX( uiViewId, ui, LIST_0 ); 
				else
					uiRef = pcSPS->SpsMVC->getAnchorRefForListX( uiViewId, ui-pcSPS->SpsMVC->getNumRefsForListX(uiViewId,LIST_0,true), LIST_1 );
				if( i > ViewOp[uiRef] )
					pcSEIMessage->setDirectlyDependentViewId( i, ui, i - ViewOp[uiRef] - 1 );//SEI JJ
			}
		}
	}

	ViewOp = NULL;
	pcSPS->destroy();
  }

  return Err::m_nOK;
}


ErrVal
H264AVCPacketAnalyzer::xCreate()
{
  RNOK( BitReadBuffer::create( m_pcBitReadBuffer ) );
  RNOK( UvlcReader   ::create( m_pcUvlcReader    ) );
  RNOK( NalUnitParser::create( m_pcNalUnitParser  ) );

  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::destroy()
{
  RNOK( m_pcBitReadBuffer ->destroy() );
  RNOK( m_pcUvlcReader    ->destroy() );
  RNOK( m_pcNalUnitParser ->destroy() );
  
  delete this;

  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::init()
{
  RNOK( m_pcBitReadBuffer ->init() );
  RNOK( m_pcUvlcReader    ->init( m_pcBitReadBuffer ) );
  RNOK( m_pcNalUnitParser ->init( m_pcBitReadBuffer ) );

  ::memset( m_auiDecompositionStages, 0x00, MAX_LAYERS*sizeof(UInt) );

  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::uninit()
{
  RNOK( m_pcBitReadBuffer ->uninit() );
  RNOK( m_pcUvlcReader    ->uninit() );

  return Err::m_nOK;
}


// JVT-Q054 Red. Picture {
Bool
CreaterH264AVCDecoder::isRedundantPic()
{
  return m_pcH264AVCDecoder->isRedundantPic();
}


ErrVal
CreaterH264AVCDecoder::checkRedundantPic()
{
  return m_pcH264AVCDecoder->checkRedundantPic();
}


// JVT-Q054 Red. Picture }

H264AVC_NAMESPACE_END

#include <cstdio>
#include "H264AVCEncoderLibTest.h"
#include "H264AVCEncoderTest.h"
#include "EncoderCodingParameter.h"


H264AVCEncoderTest::H264AVCEncoderTest() :
  m_pcH264AVCEncoder        ( NULL ),
  m_pcWriteBitstreamToFile  ( NULL ),
  m_pcEncoderCodingParameter( NULL )
{
  ::memset( m_apcReadYuv,   0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_apcWriteYuv,  0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_auiLumOffset, 0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiCbOffset,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiCrOffset,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiHeight,    0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiWidth,     0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiStride,    0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_aauiCropping, 0x00, MAX_LAYERS*sizeof(UInt)*4);
}


H264AVCEncoderTest::~H264AVCEncoderTest()
{
}


ErrVal
H264AVCEncoderTest::create( H264AVCEncoderTest*& rpcH264AVCEncoderTest )
{
  rpcH264AVCEncoderTest = new H264AVCEncoderTest;

  ROT( NULL == rpcH264AVCEncoderTest );
  
  return Err::m_nOK;
}



ErrVal H264AVCEncoderTest::init( Int    argc,
                                 Char** argv )
{
  //===== create and read encoder parameters =====
  RNOK( EncoderCodingParameter::create( m_pcEncoderCodingParameter ) );
  if( Err::m_nOK != m_pcEncoderCodingParameter->init( argc, argv, m_cEncoderIoParameter.cBitstreamFilename ) )
  {
    m_pcEncoderCodingParameter->printHelpMVC(argc, argv);
    return -3;
  }
  m_cEncoderIoParameter.nResult = -1;


  //===== init instances for reading and writing yuv data =====
  UInt uiNumberOfLayers = m_pcEncoderCodingParameter->getMVCmode() ? 1 : m_pcEncoderCodingParameter->getNumberOfLayers();
  for( UInt uiLayer = 0; uiLayer < uiNumberOfLayers; uiLayer++ )
  {
    h264::LayerParameters&  rcLayer = m_pcEncoderCodingParameter->getLayerParameters( uiLayer );

    RNOKS( WriteYuvToFile::create( m_apcWriteYuv[uiLayer], rcLayer.getOutputFilename() ) );
    RNOKS( ReadYuvFile   ::create( m_apcReadYuv [uiLayer] ) );  

    RNOKS( m_apcReadYuv[uiLayer]->init( rcLayer.getInputFilename(),
                                        rcLayer.getFrameHeight  (),
                                        rcLayer.getFrameWidth   () ) ); 
  }


  //===== init bitstream writer =====
  if( m_pcEncoderCodingParameter->getMVCmode() )
  {
  //SEI {
	if( m_pcEncoderCodingParameter->getViewScalInfoSEIEnable() )
	{
    m_cWriteToBitFileTempName                 = m_cEncoderIoParameter.cBitstreamFilename + ".temp";
    m_cWriteToBitFileName                     = m_cEncoderIoParameter.cBitstreamFilename;
    m_cEncoderIoParameter.cBitstreamFilename  = m_cWriteToBitFileTempName;
	}
  //SEI }
    RNOKS( WriteBitstreamToFile::create   ( m_pcWriteBitstreamToFile ) );
    RNOKS( m_pcWriteBitstreamToFile->init ( m_cEncoderIoParameter.cBitstreamFilename ) );  
  }
  else
  {
    m_cWriteToBitFileTempName                 = m_cEncoderIoParameter.cBitstreamFilename + ".temp";
    m_cWriteToBitFileName                     = m_cEncoderIoParameter.cBitstreamFilename;
    m_cEncoderIoParameter.cBitstreamFilename  = m_cWriteToBitFileTempName;
    RNOKS( WriteBitstreamToFile::create   ( m_pcWriteBitstreamToFile ) );
    RNOKS( m_pcWriteBitstreamToFile->init ( m_cEncoderIoParameter.cBitstreamFilename ) );  
  }

  //===== create encoder instance =====
  RNOK( h264::CreaterH264AVCEncoder::create( m_pcH264AVCEncoder ) );


  //===== set start code =====
  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset ();
  m_cBinDataStartCode.set   ( m_aucStartCodeBuffer, 4 );

  // Extended NAL unit priority is enabled by default, since 6-bit short priority
  // is incompatible with extended 4CIF Palma test set.  Change value to false
  // to enable short ID.
  m_pcEncoderCodingParameter->setExtendedPriorityId( true );

  // Example priority ID assignment: (a) spatial, (b) temporal, (c) quality
  // Other priority assignments can be created by adjusting the mapping table.
  // (J. Ridge, Nokia)
  if ( !m_pcEncoderCodingParameter->getExtendedPriorityId() )
  {
    UInt  uiPriorityId = 0;
    for( UInt uiLayer = 0; uiLayer < m_pcEncoderCodingParameter->getNumberOfLayers(); uiLayer++ )
    {
        UInt uiBitplanes;
        if ( m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getFGSMode() > 0 )
        {
          uiBitplanes = MAX_QUALITY_LEVELS - 1;  
        } else {
          uiBitplanes = (UInt) m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getNumFGSLayers();
          if ( m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getNumFGSLayers() > (Double) uiBitplanes)
          {
            uiBitplanes++;
          }
        }
 /*       for ( UInt uiTempLevel = 0; uiTempLevel <= m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getDecompositionStages(); uiTempLevel++ )
        {
            for ( UInt uiQualLevel = 0; uiQualLevel <= uiBitplanes; uiQualLevel++ )
            {
                m_pcEncoderCodingParameter->setSimplePriorityMap( uiPriorityId++, uiTempLevel, uiLayer, uiQualLevel );
                AOF( uiPriorityId > ( 1 << PRI_ID_BITS ) );
            }
        }
 JVT-S036  */
    }

    m_pcEncoderCodingParameter->setNumSimplePris( uiPriorityId );
  }

  return Err::m_nOK;
}




ErrVal
H264AVCEncoderTest::destroy()
{
  m_cBinDataStartCode.reset();

  if( m_pcH264AVCEncoder )       
  {
    RNOK( m_pcH264AVCEncoder->uninit() );       
    RNOK( m_pcH264AVCEncoder->destroy() );       
  }

  for( UInt ui = 0; ui < MAX_LAYERS; ui++ )
  {
    if( m_apcWriteYuv[ui] )              
    {
      RNOK( m_apcWriteYuv[ui]->destroy() );  
    }

    if( m_apcReadYuv[ui] )              
    {
      RNOK( m_apcReadYuv[ui]->uninit() );  
      RNOK( m_apcReadYuv[ui]->destroy() );  
    }
  }

  RNOK( m_pcEncoderCodingParameter->destroy());

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    AOF( m_acActivePicBufferList[uiLayer].empty() );
    
    //===== delete picture buffer =====
    PicBufferList::iterator iter;
    for( iter = m_acUnusedPicBufferList[uiLayer].begin(); iter != m_acUnusedPicBufferList[uiLayer].end(); iter++ )
    {
      delete (*iter)->getBuffer();
      delete (*iter);
    }
    for( iter = m_acActivePicBufferList[uiLayer].begin(); iter != m_acActivePicBufferList[uiLayer].end(); iter++ )
    {
      delete (*iter)->getBuffer();
      delete (*iter);
    }
  }

  delete this;
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xGetNewPicBuffer ( PicBuffer*&  rpcPicBuffer,
                                       UInt         uiLayer,
                                       UInt         uiSize )
{
  if( m_acUnusedPicBufferList[uiLayer].empty() )
  {
    rpcPicBuffer = new PicBuffer( new UChar[ uiSize ] );
  }
  else
  {
    rpcPicBuffer = m_acUnusedPicBufferList[uiLayer].popFront();
  }

  m_acActivePicBufferList[uiLayer].push_back( rpcPicBuffer );

  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xRemovePicBuffer( PicBufferList&  rcPicBufferUnusedList,
                                      UInt            uiLayer )
{
  while( ! rcPicBufferUnusedList.empty() )
  {
    PicBuffer* pcBuffer = rcPicBufferUnusedList.popFront();

    if( NULL != pcBuffer )
    {
      PicBufferList::iterator begin = m_acActivePicBufferList[uiLayer].begin();
      PicBufferList::iterator end   = m_acActivePicBufferList[uiLayer].end  ();
      PicBufferList::iterator iter  = std::find( begin, end, pcBuffer );

      ROT( iter == end ); // there is something wrong if the address is not in the active list

      AOT_DBG( (*iter)->isUsed() );
      m_acUnusedPicBufferList[uiLayer].push_back( *iter );
      m_acActivePicBufferList[uiLayer].erase    (  iter );
    }
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xWrite( PicBufferList&  rcPicBufferList,
                            UInt            uiLayer )
{
  while( ! rcPicBufferList.empty() )
  {
    PicBuffer* pcBuffer = rcPicBufferList.popFront();

    Pel* pcBuf = pcBuffer->getBuffer();
    RNOK( m_apcWriteYuv[uiLayer]->writeFrame( pcBuf + m_auiLumOffset[uiLayer], 
                                              pcBuf + m_auiCbOffset [uiLayer],
                                              pcBuf + m_auiCrOffset [uiLayer],
                                              m_auiHeight           [uiLayer],
                                              m_auiWidth            [uiLayer],
                                              m_auiStride           [uiLayer] ) );
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xRelease( PicBufferList&  rcPicBufferList,
                              UInt            uiLayer )
{
  RNOK( xRemovePicBuffer( rcPicBufferList, uiLayer ) );
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xWrite( ExtBinDataAccessorList& rcList,
                            UInt&                   ruiBytesInFrame )
{
  while( rcList.size() )
  {
    ruiBytesInFrame += rcList.front()->size() + 4;
    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstreamToFile->writePacket( rcList.front() ) );
    delete[] rcList.front()->data();
    delete   rcList.front();
    rcList.pop_front();
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xRelease( ExtBinDataAccessorList& rcList )
{
  while( rcList.size() )
  {
    delete[] rcList.front()->data();
    delete   rcList.front();
    rcList.pop_front();
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::go()
{
  UInt                    uiWrittenBytes          = 0;
  const UInt              uiMaxFrame              = m_pcEncoderCodingParameter->getTotalFrames();
  UInt                    uiNumLayers             = ( m_pcEncoderCodingParameter->getMVCmode() ? 1 : m_pcEncoderCodingParameter->getNumberOfLayers() );
  UInt                    uiFrame;
  UInt                    uiLayer;
  UInt                    auiMbX                  [MAX_LAYERS];
  UInt                    auiMbY                  [MAX_LAYERS];
  UInt                    auiPicSize              [MAX_LAYERS];
  PicBuffer*              apcOriginalPicBuffer    [MAX_LAYERS];//original pic
  PicBuffer*              apcReconstructPicBuffer [MAX_LAYERS];//rec pic
  PicBufferList           acPicBufferOutputList   [MAX_LAYERS];
  PicBufferList           acPicBufferUnusedList   [MAX_LAYERS];
  ExtBinDataAccessorList  cOutExtBinDataAccessorList;
  Bool                    bMoreSets;

  
  //===== initialization =====
  RNOK( m_pcH264AVCEncoder->init( m_pcEncoderCodingParameter ) ); 


  //===== write parameter sets =====
  for( bMoreSets = true; bMoreSets;  )
  {
    UChar   aucParameterSetBuffer[1000];
    BinData cBinData;
    cBinData.reset();
    cBinData.set( aucParameterSetBuffer, 1000 );

    ExtBinDataAccessor cExtBinDataAccessor;
    cBinData.setMemAccessor( cExtBinDataAccessor );

    RNOK( m_pcH264AVCEncoder      ->writeParameterSets( &cExtBinDataAccessor, bMoreSets) );
		if( m_pcH264AVCEncoder->getScalableSeiMessage() )
		{		
    RNOK( m_pcWriteBitstreamToFile->writePacket       ( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstreamToFile->writePacket       ( &cExtBinDataAccessor ) );
    
    uiWrittenBytes += 4 + cExtBinDataAccessor.size();
		}
    cBinData.reset();
  }

//JVT-W080, PDS SEI message
	if( m_pcEncoderCodingParameter->getMVCmode() && m_pcEncoderCodingParameter->getPdsEnable() )
	{
		//write SEI message
	  UChar   aucParameterSetBuffer[1000];
    BinData cBinData;
    cBinData.reset();
    cBinData.set( aucParameterSetBuffer, 1000 );

    ExtBinDataAccessor cExtBinDataAccessor;
    cBinData.setMemAccessor( cExtBinDataAccessor );

		const UInt uiSPSId = 0; //currently only one SPS with SPSId = 0
		UInt uiNumView       = m_pcEncoderCodingParameter->SpsMVC.getNumViewMinus1()+1;
		UInt* num_refs_list0_anc = new UInt [uiNumView];
		UInt* num_refs_list1_anc = new UInt [uiNumView];
		UInt* num_refs_list0_nonanc = new UInt [uiNumView];
		UInt* num_refs_list1_nonanc = new UInt [uiNumView];

		for( UInt i = 0; i < uiNumView; i++ )
		{
			num_refs_list0_anc[i]    = m_pcEncoderCodingParameter->SpsMVC.getNumAnchorRefsForListX( m_pcEncoderCodingParameter->SpsMVC.getViewCodingOrder()[i], 0 );
			num_refs_list1_anc[i]    = m_pcEncoderCodingParameter->SpsMVC.getNumAnchorRefsForListX( m_pcEncoderCodingParameter->SpsMVC.getViewCodingOrder()[i], 1 );
			num_refs_list0_nonanc[i] = m_pcEncoderCodingParameter->SpsMVC.getNumNonAnchorRefsForListX( m_pcEncoderCodingParameter->SpsMVC.getViewCodingOrder()[i], 0 );
			num_refs_list1_nonanc[i] = m_pcEncoderCodingParameter->SpsMVC.getNumNonAnchorRefsForListX( m_pcEncoderCodingParameter->SpsMVC.getViewCodingOrder()[i], 1 );		  
		}
//#define HELP_INFOR
#ifdef  HELP_INFOR
		printf("\n");
		for( UInt i = 0; i < uiNumView; i++ )
		{
			printf(" num_refs_list0_anchor: %d\tnum_refs_list0_nonanchor: %d\n num_refs_list1_anchor: %d\tnum_refs_list1_nonanchor: %d\n", num_refs_list0_anc[i], num_refs_list1_anc[i], num_refs_list0_nonanc[i], num_refs_list1_nonanc[i] );
		}
#endif

    UInt uiInitialPDIDelayAnc = m_pcEncoderCodingParameter->getPdsInitialDelayAnc();
    UInt uiInitialPDIDelayNonAnc = m_pcEncoderCodingParameter->getPdsInitialDelayNonAnc();
		if( uiInitialPDIDelayAnc < 2 )
			uiInitialPDIDelayAnc  = 2;
		if( uiInitialPDIDelayNonAnc < 2 )
			uiInitialPDIDelayNonAnc  = 2;
		RNOK( m_pcH264AVCEncoder->writePDSSEIMessage( &cExtBinDataAccessor
			                                           , uiSPSId
			                                           , uiNumView
			                                           , num_refs_list0_anc
																								 , num_refs_list1_anc
			                                           , num_refs_list0_nonanc
																								 , num_refs_list1_nonanc
																								 , uiInitialPDIDelayAnc
																								 , uiInitialPDIDelayNonAnc
																								) 
				);

		delete[] num_refs_list0_anc;
		delete[] num_refs_list1_anc;
		delete[] num_refs_list0_nonanc;
		delete[] num_refs_list1_nonanc;
		num_refs_list0_anc = NULL;
		num_refs_list1_anc = NULL;
		num_refs_list0_nonanc = NULL;
		num_refs_list1_nonanc = NULL;
	  if( m_pcEncoderCodingParameter->getCurentViewId() == m_pcEncoderCodingParameter->SpsMVC.m_uiViewCodingOrder[0] )
		{
			RNOK( m_pcWriteBitstreamToFile->writePacket       ( &m_cBinDataStartCode ) );
			RNOK( m_pcWriteBitstreamToFile->writePacket       ( &cExtBinDataAccessor ) );
			uiWrittenBytes += 4 + cExtBinDataAccessor.size();
		}

		cBinData.reset();
	}
//~JVT-W080
  //SEI {
  if( m_pcEncoderCodingParameter->getMultiviewSceneInfoSEIEnable() ) // SEI JVT-W060
  {
	  // Multiview scene information sei message
	  UChar aucParameterSetBuffer[1000];
      BinData cBinData;
      cBinData.reset();
      cBinData.set( aucParameterSetBuffer, 1000 );
      ExtBinDataAccessor cExtBinDataAccessor;
      cBinData.setMemAccessor( cExtBinDataAccessor );
	  RNOK( m_pcH264AVCEncoder ->writeMultiviewSceneInfoSEIMessage( &cExtBinDataAccessor ) );
	  RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	  RNOK( m_pcWriteBitstreamToFile->writePacket( &cExtBinDataAccessor ) );
	  uiWrittenBytes += 4 + cExtBinDataAccessor.size();
	  cBinData.reset();
  }
  if( m_pcEncoderCodingParameter->getMultiviewAcquisitionInfoSEIEnable() ) // SEI JVT-W060
  {
	  // Multiview acquisition information sei message
	  UChar aucParameterSetBuffer[1000];
      BinData cBinData;
      cBinData.reset();
      cBinData.set( aucParameterSetBuffer, 1000 );
      ExtBinDataAccessor cExtBinDataAccessor;
      cBinData.setMemAccessor( cExtBinDataAccessor );
	  RNOK( m_pcH264AVCEncoder ->writeMultiviewAcquisitionInfoSEIMessage( &cExtBinDataAccessor ) );
	  RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	  RNOK( m_pcWriteBitstreamToFile->writePacket( &cExtBinDataAccessor ) );
	  uiWrittenBytes += 4 + cExtBinDataAccessor.size();
	  cBinData.reset();
  }
  if( m_pcEncoderCodingParameter->getNestingSEIEnable() && m_pcEncoderCodingParameter->getSnapshotEnable() 
	  && m_pcEncoderCodingParameter->getCurentViewId() == 0 )
  {
   // add nesting sei message for view0
      UChar aucParameterSetBuffer[1000];
      BinData cBinData;
      cBinData.reset();
      cBinData.set( aucParameterSetBuffer, 1000 );
      ExtBinDataAccessor cExtBinDataAccessor;
      cBinData.setMemAccessor( cExtBinDataAccessor );
	  RNOK( m_pcH264AVCEncoder ->writeNestingSEIMessage( &cExtBinDataAccessor ) );
	  RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	  RNOK( m_pcWriteBitstreamToFile->writePacket( &cExtBinDataAccessor ) );
	  uiWrittenBytes += 4 + cExtBinDataAccessor.size();
	  cBinData.reset();
  }
//SEI }

  //===== determine parameters for required frame buffers =====
  for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
  {
    //auiMbX        [uiLayer] = m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getFrameWidth () >> 4;
    //auiMbY        [uiLayer] = m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getFrameHeight() >> 4;
    auiMbX        [uiLayer] = m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getFrameWidthInMbs();
    auiMbY        [uiLayer] = m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getFrameHeightInMbs();
    m_aauiCropping[uiLayer][0]     = 0;
    m_aauiCropping[uiLayer][1]     = m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getHorPadding      ();
    m_aauiCropping[uiLayer][2]     = 0;
    m_aauiCropping[uiLayer][3]     = m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getVerPadding      ();
    m_apcWriteYuv[uiLayer]->setCrop(m_aauiCropping[uiLayer]);

    UInt  uiSize            = ((auiMbY[uiLayer]<<4)+2*YUV_Y_MARGIN)*((auiMbX[uiLayer]<<4)+2*YUV_X_MARGIN);
    auiPicSize    [uiLayer] = ((auiMbX[uiLayer]<<4)+2*YUV_X_MARGIN)*((auiMbY[uiLayer]<<4)+2*YUV_Y_MARGIN)*3/2;
    m_auiLumOffset[uiLayer] = ((auiMbX[uiLayer]<<4)+2*YUV_X_MARGIN)* YUV_Y_MARGIN   + YUV_X_MARGIN;  
    m_auiCbOffset [uiLayer] = ((auiMbX[uiLayer]<<3)+  YUV_X_MARGIN)* YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + uiSize; 
    m_auiCrOffset [uiLayer] = ((auiMbX[uiLayer]<<3)+  YUV_X_MARGIN)* YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + 5*uiSize/4;
    m_auiHeight   [uiLayer] =   auiMbY[uiLayer]<<4;
    m_auiWidth    [uiLayer] =   auiMbX[uiLayer]<<4;
    m_auiStride   [uiLayer] =  (auiMbX[uiLayer]<<4)+ 2*YUV_X_MARGIN;
  }

  //===== loop over frames =====
  for( uiFrame = 0; uiFrame < uiMaxFrame; uiFrame++ )
  {
    //===== get picture buffers and read original pictures =====
    for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
    {
      UInt  uiSkip = ( 1 << m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getTemporalResolution() );

      if( uiFrame % uiSkip == 0 )
      {
        RNOK( xGetNewPicBuffer( apcReconstructPicBuffer [uiLayer], uiLayer, auiPicSize[uiLayer] ) );
        RNOK( xGetNewPicBuffer( apcOriginalPicBuffer    [uiLayer], uiLayer, auiPicSize[uiLayer] ) );

        RNOK( m_apcReadYuv[uiLayer]->readFrame( *apcOriginalPicBuffer[uiLayer] + m_auiLumOffset[uiLayer],
                                                *apcOriginalPicBuffer[uiLayer] + m_auiCbOffset [uiLayer],
                                                *apcOriginalPicBuffer[uiLayer] + m_auiCrOffset [uiLayer],
                                                m_auiHeight [uiLayer],
                                                m_auiWidth  [uiLayer],
                                                m_auiStride [uiLayer] ) );
      }
      else
      {
        apcReconstructPicBuffer [uiLayer] = 0;
        apcOriginalPicBuffer    [uiLayer] = 0;
      }
    }

    //===== call encoder =====
    RNOK( m_pcH264AVCEncoder->process( cOutExtBinDataAccessorList,
                                       apcOriginalPicBuffer,
                                       apcReconstructPicBuffer,
                                       acPicBufferOutputList,
                                       acPicBufferUnusedList ) );

    //===== write and release NAL unit buffers =====
    UInt  uiBytesUsed = 0;
    RNOK( xWrite  ( cOutExtBinDataAccessorList, uiBytesUsed ) );
    uiWrittenBytes   += uiBytesUsed;
    
    //===== write and release reconstructed pictures =====
    for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
    {
      RNOK( xWrite  ( acPicBufferOutputList[uiLayer], uiLayer ) );
      RNOK( xRelease( acPicBufferUnusedList[uiLayer], uiLayer ) );
    }
  }

  //===== finish encoding =====
  UInt  uiNumCodedFrames = 0;
  Double  dHighestLayerOutputRate = 0.0;
  RNOK( m_pcH264AVCEncoder->finish( cOutExtBinDataAccessorList,
                                    acPicBufferOutputList,
                                    acPicBufferUnusedList,
                                    uiNumCodedFrames,
                                    dHighestLayerOutputRate ) );


  //===== write and release NAL unit buffers =====
  RNOK( xWrite  ( cOutExtBinDataAccessorList, uiWrittenBytes ) );

  //===== write and release reconstructed pictures =====
  for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
  {
    RNOK( xWrite  ( acPicBufferOutputList[uiLayer], uiLayer ) );
    RNOK( xRelease( acPicBufferUnusedList[uiLayer], uiLayer ) );
  }


  //===== set parameters and output summary =====
  m_cEncoderIoParameter.nFrames = uiFrame;
  m_cEncoderIoParameter.nResult = 0;

  if( ! m_pcEncoderCodingParameter->getMVCmode() )
	{
		UChar   aucParameterSetBuffer[1000];
		BinData cBinData;
		cBinData.reset();
		cBinData.set( aucParameterSetBuffer, 1000 );

		ExtBinDataAccessor cExtBinDataAccessor;
		cBinData.setMemAccessor( cExtBinDataAccessor );
		m_pcH264AVCEncoder->SetVeryFirstCall();
		RNOK( m_pcH264AVCEncoder      ->writeParameterSets( &cExtBinDataAccessor, bMoreSets) );
		RNOK( m_pcWriteBitstreamToFile->writePacket       ( &m_cBinDataStartCode ) );
		RNOK( m_pcWriteBitstreamToFile->writePacket       ( &cExtBinDataAccessor ) );
		uiWrittenBytes += 4 + cExtBinDataAccessor.size();
		cBinData.reset();
	}
//SEI {
  if( m_pcEncoderCodingParameter->getViewScalInfoSEIEnable() )
  {
    //view scalability information sei message
     UChar   aucParameterSetBuffer[1000];
     BinData cBinData;
     cBinData.reset();
     cBinData.set( aucParameterSetBuffer, 1000 );

     ExtBinDataAccessor cExtBinDataAccessor;
     cBinData.setMemAccessor( cExtBinDataAccessor );
     RNOK( m_pcH264AVCEncoder->writeViewScalInfoSEIMessage( &cExtBinDataAccessor ) );
     RNOK( m_pcWriteBitstreamToFile->writePacket       ( &m_cBinDataStartCode ) );
     RNOK( m_pcWriteBitstreamToFile->writePacket       ( &cExtBinDataAccessor ) );
     uiWrittenBytes += 4 + cExtBinDataAccessor.size();
     cBinData.reset();

  }
//SEI }
  if( m_pcWriteBitstreamToFile )
  {
    RNOK( m_pcWriteBitstreamToFile->uninit() );  
    RNOK( m_pcWriteBitstreamToFile->destroy() );  
  }

//SEI {
  if( m_pcEncoderCodingParameter->getViewScalInfoSEIEnable() )
  {
    RNOK    ( ViewScalableDealing() );
  }
//SEI }
  if( ! m_pcEncoderCodingParameter->getMVCmode() )
  {
	RNOK	( ScalableDealing() );
  }

  return Err::m_nOK;
}

ErrVal
H264AVCEncoderTest::ScalableDealing()
{
  FILE *ftemp = fopen( m_cWriteToBitFileTempName.c_str(), "rb" );
  FILE *f     = fopen( m_cWriteToBitFileName.c_str    (), "wb" );

	UChar pvBuffer[4];

	fseek( ftemp, SEEK_SET, SEEK_END );
	long lFileLength = ftell( ftemp );

	long lpos = 0;
	long loffset = -5;	//start offset from end of file
	Bool bMoreSets = true;
	do {
		fseek( ftemp, loffset, SEEK_END);
		fread( pvBuffer, 1, 4, ftemp );
		if( pvBuffer[0] == 0 && pvBuffer[1] == 0 && pvBuffer[2] == 0 && pvBuffer[3] == 1)
		{
			bMoreSets = false;
			lpos = abs( loffset );
		}
		else
		{
			loffset --;
		}
	} while( bMoreSets );

	fseek( ftemp, loffset, SEEK_END );

	UChar *pvChar = new UChar[lFileLength];
	fread( pvChar, 1, lpos, ftemp );
	fseek( ftemp, 0, SEEK_SET );
	fread( pvChar+lpos, 1, lFileLength-lpos, ftemp);
	fclose(ftemp);
	fflush(ftemp);
	fwrite( pvChar, 1, lFileLength, f);	
	delete pvChar;
	fclose(f);
	fflush(f);
	RNOK( remove( m_cWriteToBitFileTempName.c_str() ) ); 

	return Err::m_nOK;
}


//SEI {
ErrVal
H264AVCEncoderTest::ViewScalableDealing()
{
  FILE *ftemp = fopen( m_cWriteToBitFileTempName.c_str(), "rb" );
  FILE *f     = fopen( m_cWriteToBitFileName.c_str    (), "wb" );

	UChar pvBuffer[4];

	fseek( ftemp, SEEK_SET, SEEK_END );
	long lFileLength = ftell( ftemp );

	long lpos = 0;
	long loffset = -5;	//start offset from end of file
	Bool bMoreSets = true;
	do {
		fseek( ftemp, loffset, SEEK_END);
		fread( pvBuffer, 1, 4, ftemp );
		if( pvBuffer[0] == 0 && pvBuffer[1] == 0 && pvBuffer[2] == 0 && pvBuffer[3] == 1)
		{
			bMoreSets = false;
			lpos = abs( loffset );
		}
		else
		{
			loffset --;
		}
	} while( bMoreSets );

	fseek( ftemp, loffset, SEEK_END );

	UChar *pvChar = new UChar[lFileLength];
	fread( pvChar, 1, lpos, ftemp );
	fseek( ftemp, 0, SEEK_SET );
	fread( pvChar+lpos, 1, lFileLength-lpos, ftemp);
	fclose(ftemp);
	fflush(ftemp);
	fwrite( pvChar, 1, lFileLength, f);	
	delete pvChar;
	fclose(f);
	fflush(f);
	RNOK( remove( m_cWriteToBitFileTempName.c_str() ) ); 

	return Err::m_nOK;
}

//SEI }

#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/Sei.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/TraceFile.h"



H264AVC_NAMESPACE_BEGIN




ErrVal
//SEI::read( HeaderSymbolReadIf* pcReadIf,
//           MessageList&        rcSEIMessageList ) 
SEI::read( HeaderSymbolReadIf* pcReadIf,
           MessageList&        rcSEIMessageList /*, UInt NumViewsMinus1*/ )  // SEI JVT-W060 Nov. 30
{
  ROT( NULL == pcReadIf);

  while( pcReadIf->moreRBSPData() )
  {
    SEIMessage* pcActualSEIMessage = NULL;
	
	//    RNOK( xRead( pcReadIf, pcActualSEIMessage ));
	RNOK( xRead( pcReadIf, pcActualSEIMessage /*, NumViewsMinus1 */)); // SEI JVT-W060 Nov. 30


    rcSEIMessageList.push_back( pcActualSEIMessage );
	//JVT-AB025{{ omit parsing
	if( pcActualSEIMessage->getMessageType()  == VIEW_DEPENDENCY_STRUCTURE_SEI )
	{
		return Err::m_nOK;
	}
	//JVT-AB025 }}
//JVT-W080, stop parsing any more
		if( pcActualSEIMessage->getMessageType() == PARALLEL_DEC_SEI )
			return Err::m_nOK;
//~JVT-W080
  }
  return Err::m_nOK;
}


ErrVal
SEI::write( HeaderSymbolWriteIf*  pcWriteIf,
            HeaderSymbolWriteIf*  pcWriteTestIf,
            MessageList*          rpcSEIMessageList )
{
  ROT( NULL == pcWriteIf);
  ROT( NULL == pcWriteTestIf);
  ROT( NULL == rpcSEIMessageList);

  //===== NAL unit header =====
  ETRACE_DECLARE( Bool m_bTraceEnable = true );
  g_nLayer = 0;
  ETRACE_LAYER(0);
  ETRACE_HEADER( "SEI MESSAGE" );
  RNOK  ( pcWriteIf->writeFlag( 0,                "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( 0, 2,             "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( NAL_UNIT_SEI, 5,  "NALU HEADER: nal_unit_type" ) );

  while( rpcSEIMessageList->size() )
  {
    RNOK( xWrite( pcWriteIf, pcWriteTestIf, rpcSEIMessageList->front() ) );
    SEIMessage* pcTop = rpcSEIMessageList->front();
    rpcSEIMessageList->pop_front();
    delete pcTop;
  }
  return Err::m_nOK;
}

//SEI LSJ{
ErrVal
SEI::writeNesting( HeaderSymbolWriteIf*  pcWriteIf,
                   HeaderSymbolWriteIf*  pcWriteTestIf,
                   MessageList*          rpcSEIMessageList )
{
    ROT( NULL == pcWriteIf);
	ROT( NULL == pcWriteTestIf);
	ROT( NULL == rpcSEIMessageList);
    
    SEIMessage* pcTop = rpcSEIMessageList->front();
    rpcSEIMessageList->pop_front();
	SEIMessage* pcBottom = rpcSEIMessageList->front();
	rpcSEIMessageList->pop_front();

	//===== NAL unit header =====
	ETRACE_DECLARE( Bool m_bTraceEnable = true );
	g_nLayer = 0;
	ETRACE_LAYER(0);
	ETRACE_HEADER( "SEI MESSAGE" );
	RNOK  ( pcWriteIf->writeFlag( 0,                "NALU HEADER: forbidden_zero_bit"  ) );
	RNOK  ( pcWriteIf->writeCode( 0, 2,             "NALU HEADER: nal_ref_idc" ) );
	RNOK  ( pcWriteIf->writeCode( NAL_UNIT_SEI, 5,  "NALU HEADER: nal_unit_type" ) );

	//first write testing SEI message to get payload size
	UInt uiBits = 0;
	UInt uiSecondSEILength = 0;

	UInt uiStart = pcWriteTestIf->getNumberOfWrittenBits();
	//take scene info as example, 	//can be changed here
	switch( pcBottom->getMessageType() )
	{
		case FULLFRAME_SNAPSHOT_SEI:
	    {
			SEI::FullframeSnapshotSei* pcSnapShotSei = (SEI::FullframeSnapshotSei*) pcBottom;
			UInt uiSnapShotId = 1;
			pcSnapShotSei->setSnapShotId( uiSnapShotId );

			RNOK( pcWriteTestIf->writeCode( FULLFRAME_SNAPSHOT_SEI, 8, "SEI type" ) );
			RNOK( pcWriteTestIf->writeCode( 1, 8, "SEI payload size " ) ); //currently size equals to 1
			RNOK( xWriteNesting( pcWriteIf, pcWriteTestIf, pcBottom, uiBits ) );
			uiBits -= uiStart;
			uiSecondSEILength = (uiBits-16+7)/8;
			break;
	    }
		//more case: added here
		default: break;
	}
    RNOK  ( xWriteNesting( pcWriteIf, pcWriteTestIf, pcTop, uiBits ) );

	uiBits -= uiStart;
	//Then write actual SEI message
	UInt uiSize = (uiBits+7)/8;
    RNOK( xWritePayloadHeader( pcWriteIf, SCALABLE_NESTING_SEI, uiSize ) );
    RNOK( pcTop->write( pcWriteIf ) );
	RNOK( xWritePayloadHeader( pcWriteIf, FULLFRAME_SNAPSHOT_SEI, uiSecondSEILength ) );
	RNOK( pcBottom->write( pcWriteIf ) );
    UInt uiAlignedBits = 8 - (uiBits&7);
    if( uiAlignedBits != 0 && uiAlignedBits != 8)
    {
        RNOK( pcWriteIf->writeCode( 1<<(uiAlignedBits-1), uiAlignedBits, "SEI: alignment_bits" ) );
    }
  return Err::m_nOK;
}

ErrVal
SEI::xWriteNesting( HeaderSymbolWriteIf* pcWriteIf,
				   HeaderSymbolWriteIf*  pcWriteTestIf,
				   SEIMessage*           pcSEIMessage,
				   UInt&                 uiBits )
{
	UInt uiStart = uiBits;
	if( uiBits == 0 ) // write the following SEI message
	{
		uiBits += 16; //including SEI_type and SEI_payload_size bits, can be changed here for type>0xff
		RNOK( pcSEIMessage->write( pcWriteTestIf ) );
		uiBits = pcWriteTestIf->getNumberOfWrittenBits() - uiStart;
		if( uiBits/8 >0xff )
			uiBits += 8;
		return Err::m_nOK;
	}
	else
	{
		RNOK( pcSEIMessage->write( pcWriteTestIf ) );
		uiBits = pcWriteTestIf->getNumberOfWrittenBits();
		return Err::m_nOK;
	}
}
//JVT-W035 LSJ}
//JVT-W037 LSJ{
//////////////////////////////////////////////////////////////////////////
//
//			VIEW SCALABILITY INFORMATION SEI
//
//////////////////////////////////////////////////////////////////////////
SEI::ViewScalabilityInfoSei::ViewScalabilityInfoSei()
:SEIMessage			( VIEW_SCALABILITY_INFO_SEI )
,m_uiNumOperationPointsMinus1( 0 )
{
	::memset( m_uiOperationPointId, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiPriorityId, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiTemporalId, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiNumTargetOutputViewsMinus1, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );//SEI JJ
	::memset( m_uiViewId, 0x00, MAX_OPERATION_POINTS*sizeof(UInt*) );
	::memset( m_bProfileLevelInfoPresentFlag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );
	::memset( m_bBitRateInfoPresentFlag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );
	::memset( m_bFrmRateInfoPresentFlag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );
	::memset( m_bViewDependencyInfoPresentFlag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );//SEI JJ 
	::memset( m_bParameterSetsInfoPresentFlag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );//SEI JJ 
	::memset( m_uiOpProfileLevelIdc, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );//SEI JJ
	::memset( m_bOpConstraintSet0Flag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );
	::memset( m_bOpConstraintSet1Flag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );
	::memset( m_bOpConstraintSet2Flag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );
	::memset( m_bOpConstraintSet3Flag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );
//bug_fix_chenlulu{
	::memset( m_bOpConstraintSet4Flag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );
	::memset( m_bOpConstraintSet5Flag, 0x00, MAX_OPERATION_POINTS*sizeof(Bool) );
//bug_fix_chenlulu}

	::memset( m_uiOpLevelIdc, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );

	::memset( m_uiProfileLevelInfoSrcOpIdDelta, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiAvgBitrate, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiMaxBitrate, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiMaxBitrateCalcWindow, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiConstantFrmRateIdc, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiAvgFrmRate, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiFrmRateInfoSrcOpIdDela, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );
	::memset( m_uiNumDirectlyDependentViews, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );//SEI JJ 
	::memset( m_uiDirectlyDependentViewId, 0x00, MAX_OPERATION_POINTS*sizeof(UInt*) );//SEI JJ 
	::memset( m_uiViewDependencyInfoSrcOpId, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );//SEI JJ 
	::memset( m_uiNumSeqParameterSetMinus1, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );//SEI JJ 
	::memset( m_uiSeqParameterSetIdDelta, 0x00, MAX_OPERATION_POINTS*sizeof(UInt*) );//SEI JJ 
	::memset( m_uiNumPicParameterSetMinus1, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );//SEI JJ 
	::memset( m_uiPicParameterSetIdDelta, 0x00, MAX_OPERATION_POINTS*sizeof(UInt*) );//SEI JJ 
	::memset( m_uiParameterSetsInfoSrcOpId, 0x00, MAX_OPERATION_POINTS*sizeof(UInt) );//SEI JJ 
}

SEI::ViewScalabilityInfoSei::~ViewScalabilityInfoSei()
{
	UInt i;
	for( i = 0; i < MAX_OPERATION_POINTS; i++ )
	{
	  if( m_uiViewId[i] != NULL )
	  {
	    free( m_uiViewId[i] );
		m_uiViewId[i] = NULL;
	  }
	  //{{SEI JJ
	  if( m_uiDirectlyDependentViewId[i] != NULL )
	  {
	    free( m_uiDirectlyDependentViewId[i] );
		m_uiDirectlyDependentViewId[i] = NULL;
	  }

	  if( m_uiSeqParameterSetIdDelta[i] != NULL )
	  {
	    free( m_uiSeqParameterSetIdDelta[i] );
		m_uiSeqParameterSetIdDelta[i] = NULL;
	  }

	  if( m_uiPicParameterSetIdDelta[i] != NULL )
	  {
	    free( m_uiPicParameterSetIdDelta[i] );
		m_uiPicParameterSetIdDelta[i] = NULL;
	  }
	  //}}SEI JJ
	}
}

ErrVal
SEI::ViewScalabilityInfoSei::create( ViewScalabilityInfoSei*& rpcSeiMessage )
{
	rpcSeiMessage = new ViewScalabilityInfoSei();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}

ErrVal
SEI::ViewScalabilityInfoSei::destroy()
{
	delete this;
	return Err::m_nOK;
}

ErrVal
SEI::ViewScalabilityInfoSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
  UInt i=0,j=0;

  RNOK( pcWriteIf->writeUvlc( m_uiNumOperationPointsMinus1, "ViewScalabilityInfoSei: num_operation_points_minus1" ) );

  for( i = 0; i <= m_uiNumOperationPointsMinus1; i++ )
  {
	RNOK( pcWriteIf->writeUvlc( m_uiOperationPointId[i], "ViewScalabilityInfoSei: operation_point_id" ) );
	RNOK( pcWriteIf->writeCode( m_uiPriorityId[i], 5, "ViewScalabilityInfoSei: priority_id" ) );
	RNOK( pcWriteIf->writeCode( m_uiTemporalId[i], 3, "ViewScalabilityInfoSei: temporal_id" ) );
	RNOK( pcWriteIf->writeUvlc( m_uiNumTargetOutputViewsMinus1[i], "ViewScalabilityInfoSei: num_target_output_views_minus1" ) );//SEI JJ 

	for( j = 0; j <= m_uiNumTargetOutputViewsMinus1[i]; j++ )//SEI JJ 
	{
	  RNOK( pcWriteIf->writeUvlc( m_uiViewId[i][j], "ViewScalabilityInfoSei: view_id" ) );
	}

	RNOK( pcWriteIf->writeFlag( m_bProfileLevelInfoPresentFlag[i], "ViewScalabilityInfoSei: profile_level_info_present_flag" ) );
	RNOK( pcWriteIf->writeFlag( m_bBitRateInfoPresentFlag[i], "ViewScalabilityInfoSei: bitrate_info_present_flag" ) );
	RNOK( pcWriteIf->writeFlag( m_bFrmRateInfoPresentFlag[i], "ViewScalabilityInfoSei: frm_rate_info_present_flag" ) );
	//SEI JJ{{
	if ( !m_uiNumTargetOutputViewsMinus1[i] )
	{
		RNOK( pcWriteIf->writeFlag( m_bViewDependencyInfoPresentFlag[i], "ViewScalabilityInfoSei: view_dependency_info_present_flag" ) );
	}//SEI JJ}}
	RNOK( pcWriteIf->writeFlag( m_bParameterSetsInfoPresentFlag[i], "ViewScalabilityInfoSei: parameter_sets_info_present_flag" ) );//SEI JJ
	RNOK( pcWriteIf->writeFlag( m_bBitstreamRestrictionInfoPresentFlag[i], "ViewScalabilityInfoSei: bitstream_restriction_info_present_flag" ) );//SEI JJ
 
	if( m_bProfileLevelInfoPresentFlag[i] )
	{
	  RNOK( pcWriteIf->writeCode( m_uiOpProfileLevelIdc[i], 8, "ViewScalabilityInfoSei: op_profile_level_idc" ) );//SEI JJ
	  RNOK( pcWriteIf->writeFlag( m_bOpConstraintSet0Flag[i], "ViewScalabilityInfoSei: op_constraint_set0_flag" ) );
	  RNOK( pcWriteIf->writeFlag( m_bOpConstraintSet1Flag[i], "ViewScalabilityInfoSei: op_constraint_set1_flag" ) );
	  RNOK( pcWriteIf->writeFlag( m_bOpConstraintSet2Flag[i], "ViewScalabilityInfoSei: op_constraint_set2_flag" ) );
	  RNOK( pcWriteIf->writeFlag( m_bOpConstraintSet3Flag[i], "ViewScalabilityInfoSei: op_constraint_set3_flag" ) );
	  //bug_fix_chenlulu{
	  RNOK( pcWriteIf->writeFlag( m_bOpConstraintSet4Flag[i], "ViewScalabilityInfoSei: op_constraint_set4_flag" ) );
	  RNOK( pcWriteIf->writeFlag( m_bOpConstraintSet5Flag[i], "ViewScalabilityInfoSei: op_constraint_set5_flag" ) );	  
	  RNOK( pcWriteIf->writeCode( 0, 2, "ViewScalabilityInfoSei: reserved_zero_2bits" ) );
	  //RNOK( pcWriteIf->writeCode( 0, 4, "ViewScalabilityInfoSei: reserved_zero_4bits" ) );
	  //bug_fix_chenlulu} 
	  RNOK( pcWriteIf->writeCode( m_uiOpLevelIdc[i], 8, "ViewScalabilityInfoSei: op_level_idc" ) );
	}
	
	if( m_bBitRateInfoPresentFlag[i] )
	{
	  RNOK( pcWriteIf->writeCode( m_uiAvgBitrate[i], 16, "ViewScalabilityInfoSei: avg_bitrate" ) );
	  RNOK( pcWriteIf->writeCode( m_uiMaxBitrate[i], 16, "ViewScalabilityInfoSei: max_bitrate" ) );
	  RNOK( pcWriteIf->writeCode( m_uiMaxBitrateCalcWindow[i], 16, "ViewScalabilityInfoSei: max_bitrate_calc_window" ) );
	}

	if( m_bFrmRateInfoPresentFlag[i] )
	{
	  RNOK( pcWriteIf->writeCode( m_uiConstantFrmRateIdc[i], 2, "ViewScalabilityInfoSei: constant_frm_rate_idc" ) );
	  RNOK( pcWriteIf->writeCode( m_uiAvgFrmRate[i], 16, "ViewScalabilityInfoSei: avg_frm_rate" ) );
	}
	//{{SEI JJ
	if( m_bViewDependencyInfoPresentFlag[i] )
	{
	  RNOK( pcWriteIf->writeUvlc( m_uiNumDirectlyDependentViews[i], "ViewScalabilityInfoSei: num_directly_dependent_ops" ) );
	  for( j = 0; j < m_uiNumDirectlyDependentViews[i]; j++ )
	    RNOK( pcWriteIf->writeUvlc( m_uiDirectlyDependentViewId[i][j], "ViewScalabilityInfoSei: directly_dependent_view_id" ) );
	}
	else
	  RNOK( pcWriteIf->writeUvlc( m_uiViewDependencyInfoSrcOpId[i], "ViewScalabilityInfoSei: view_dependency_info_src_op_id" ) ); 

	if( m_bParameterSetsInfoPresentFlag[i] )
	{
	  RNOK( pcWriteIf->writeUvlc( m_uiNumSeqParameterSetMinus1[i], "ViewScalabilityInfoSei: num_seq_parameter_set_minus1" ) );
	  for( j = 0; j <= m_uiNumSeqParameterSetMinus1[i]; j++ )
	    RNOK( pcWriteIf->writeUvlc( m_uiSeqParameterSetIdDelta[i][j], "ViewScalabilityInfoSei: seq_parameter_set_id_delta" ) );
       
	  RNOK( pcWriteIf->writeUvlc( m_uiNumSubsetSeqParameterSetMinus1[i], "ViewScalabilityInfoSei: num_subset_seq_parameter_set_minus1" ) );
	  for ( j = 0; j <= m_uiNumSubsetSeqParameterSetMinus1[ i ]; j++)
	  {
		  RNOK( pcWriteIf->writeUvlc( m_uiSubsetSeqParameterSetIdDelta[i][j], "ViewScalabilityInfoSei: subset_seq_parameter_set_id_delta" ) );
	  }
	  
	  RNOK( pcWriteIf->writeUvlc( m_uiNumPicParameterSetMinus1[i], "ViewScalabilityInfoSei: num_pic_parameter_set_minus1" ) ); 
	  for( j = 0; j <= m_uiNumPicParameterSetMinus1[i]; j++ )
	    RNOK( pcWriteIf->writeUvlc( m_uiPicParameterSetIdDelta[i][j], "ViewScalabilityInfoSei: pic_parameter_set_id_delta" ) );
	}
	else
	  RNOK( pcWriteIf->writeUvlc( m_uiParameterSetsInfoSrcOpId[i], "ViewScalabilityInfoSei: parameter_sets_info_src_op_id" ) );
	
	if( m_bBitstreamRestrictionInfoPresentFlag[ i ] ) 
	{
		RNOK	( pcWriteIf->writeFlag( m_bMotionVectorsOverPicBoundariesFlag[i],"ScalableSEI:motion_vectors_over_pic_boundaries_flag"	) );
		RNOK	( pcWriteIf->writeUvlc( m_uiMaxBytesPerPicDenom[i],		"ScalableSEI:max_bytes_per_pic_denom" ) );
		RNOK	( pcWriteIf->writeUvlc( m_uiMaxBitsPerMbDenom[i],		"ScalableSEI:max_bits_per_mb_denom" ) );
		RNOK	( pcWriteIf->writeUvlc( m_uiLog2MaxMvLengthHorizontal[i],		"ScalableSEI:log2_max_mv_length_horizontal" ) );
		RNOK	( pcWriteIf->writeUvlc( m_uiLog2MaxMvLengthVertical[i],		"ScalableSEI:og2_max_mv_length_vertical" ) );
		RNOK	( pcWriteIf->writeUvlc( m_uiNumReorderFrames[i],		"ScalableSEI:num_reorder_frames" ) );
		RNOK	( pcWriteIf->writeUvlc( m_uiMaxDecFrameBuffering[i],		"ScalableSEI:max_dec_frame_buffering" ) );
	}
	//}}SEI JJ, Ying
  }
  return Err::m_nOK;

}

ErrVal
SEI::ViewScalabilityInfoSei::read( HeaderSymbolReadIf *pcReadIf )
{
  UInt i=0,j=0;
  UInt ReservedBit;

  RNOK( pcReadIf->getUvlc( m_uiNumOperationPointsMinus1, "ViewScalabilityInfoSei: num_operation_points_minus1" ) );

  for( i = 0; i <= m_uiNumOperationPointsMinus1; i++ )
  {
	RNOK( pcReadIf->getUvlc( m_uiOperationPointId[i], "ViewScalabilityInfoSei: operation_point_id" ) );
	RNOK( pcReadIf->getCode( m_uiPriorityId[i], 5, "ViewScalabilityInfoSei: priority_id" ) );
	RNOK( pcReadIf->getCode( m_uiTemporalId[i], 3, "ViewScalabilityInfoSei: temporal_id" ) );
	RNOK( pcReadIf->getUvlc( m_uiNumTargetOutputViewsMinus1[i], "ViewScalabilityInfoSei: num_target_output_views_minus1" ) );//SEI JJ 

	if( m_uiViewId[i] != NULL )
	  free( m_uiViewId[i] );
	m_uiViewId[i] = (UInt*)malloc( ( m_uiNumTargetOutputViewsMinus1[i]+1 )*sizeof(UInt) );//SEI JJ 

	for( j = 0; j <= m_uiNumTargetOutputViewsMinus1[i]; j++ )//SEI JJ 
	{
	  RNOK( pcReadIf->getUvlc( m_uiViewId[i][j], "ViewScalabilityInfoSei: view_id" ) );
	}

	RNOK( pcReadIf->getFlag( m_bProfileLevelInfoPresentFlag[i], "ViewScalabilityInfoSei: profile_level_info_present_flag" ) );
	RNOK( pcReadIf->getFlag( m_bBitRateInfoPresentFlag[i], "ViewScalabilityInfoSei: bitrate_info_present_flag" ) );
	RNOK( pcReadIf->getFlag( m_bFrmRateInfoPresentFlag[i], "ViewScalabilityInfoSei: frm_rate_info_present_flag" ) );
	//{{SEI JJ 
	if ( !m_uiNumTargetOutputViewsMinus1[i] )
	{
		RNOK( pcReadIf->getFlag( m_bViewDependencyInfoPresentFlag[i], "ViewScalabilityInfoSei: view_dependency_info_present_flag" ) );
	}
	RNOK( pcReadIf->getFlag( m_bParameterSetsInfoPresentFlag[i], "ViewScalabilityInfoSei: parameter_sets_info_present_flag" ) ); 
	RNOK( pcReadIf->getFlag( m_bBitstreamRestrictionInfoPresentFlag[i], "ViewScalabilityInfoSei: bitstream_restriction_info_present_flag" ) );
    //}}SEI JJ
	if( m_bProfileLevelInfoPresentFlag[i] )
	{
	  RNOK( pcReadIf->getCode( m_uiOpProfileLevelIdc[i], 8, "ViewScalabilityInfoSei: op_profile_level_idc" ) );//SEI JJ
	  RNOK( pcReadIf->getFlag( m_bOpConstraintSet0Flag[i], "ViewScalabilityInfoSei: op_constraint_set0_flag" ) );
	  RNOK( pcReadIf->getFlag( m_bOpConstraintSet1Flag[i], "ViewScalabilityInfoSei: op_constraint_set1_flag" ) );
	  RNOK( pcReadIf->getFlag( m_bOpConstraintSet2Flag[i], "ViewScalabilityInfoSei: op_constraint_set2_flag" ) );
	  RNOK( pcReadIf->getFlag( m_bOpConstraintSet3Flag[i], "ViewScalabilityInfoSei: op_constraint_set3_flag" ) );
	  //bug_fix_chenlulu{
	  RNOK( pcReadIf->getFlag( m_bOpConstraintSet4Flag[i], "ViewScalabilityInfoSei: op_constraint_set4_flag" ) );
	  RNOK( pcReadIf->getFlag( m_bOpConstraintSet5Flag[i], "ViewScalabilityInfoSei: op_constraint_set5_flag" ) );	  
	  RNOK( pcReadIf->getCode( ReservedBit, 2, "ViewScalabilityInfoSei: reserved_zero_2bits" ) );
	  //RNOK( pcReadIf->getCode( ReservedBit, 4, "ViewScalabilityInfoSei: reserved_zero_4bits" ) );
	  //bug_fix_chenlulu}
	  RNOK( pcReadIf->getCode( m_uiOpLevelIdc[i], 8, "ViewScalabilityInfoSei: op_level_idc" ) );
	}
	if( m_bBitRateInfoPresentFlag[i] )
	{
	  RNOK( pcReadIf->getCode( m_uiAvgBitrate[i], 16, "ViewScalabilityInfoSei: avg_bitrate" ) );
	  RNOK( pcReadIf->getCode( m_uiMaxBitrate[i], 16, "ViewScalabilityInfoSei: max_bitrate" ) );
	  RNOK( pcReadIf->getCode( m_uiMaxBitrateCalcWindow[i], 16, "ViewScalabilityInfoSei: max_bitrate_calc_window" ) );
	}

	if( m_bFrmRateInfoPresentFlag[i] )
	{
	  RNOK( pcReadIf->getCode( m_uiConstantFrmRateIdc[i], 2, "ViewScalabilityInfoSei: constant_frm_rate_idc" ) );
	  RNOK( pcReadIf->getCode( m_uiAvgFrmRate[i], 16, "ViewScalabilityInfoSei: avg_frm_rate" ) );
	}
    //{{SEI JJ
	if( m_bViewDependencyInfoPresentFlag[i] )
	{
	  RNOK( pcReadIf->getUvlc( m_uiNumDirectlyDependentViews[i], "ViewScalabilityInfoSei: num_directly_dependent_views" ) );

	  if( m_uiDirectlyDependentViewId[i] != NULL )
	    free( m_uiDirectlyDependentViewId[i] );
	  m_uiDirectlyDependentViewId[i] = (UInt*)malloc( ( m_uiNumDirectlyDependentViews[i]+1 ) * sizeof(UInt) );

	  for( j = 0; j < m_uiNumDirectlyDependentViews[i]; j++ )
	    RNOK( pcReadIf->getUvlc( m_uiDirectlyDependentViewId[i][j], "ViewScalabilityInfoSei: directly_dependent_view_id" ) );
	}
	else
	  RNOK( pcReadIf->getUvlc( m_uiViewDependencyInfoSrcOpId[i], "ViewScalabilityInfoSei: view_dependency_info_src_op_id" ) );

	if( m_bParameterSetsInfoPresentFlag[i] )
	{
	  RNOK( pcReadIf->getUvlc( m_uiNumSeqParameterSetMinus1[i], "ViewScalabilityInfoSei: num_seq_parameter_set_minus1" ) );

	  if( m_uiSeqParameterSetIdDelta[i] != NULL )
	    free( m_uiSeqParameterSetIdDelta[i] );
	  m_uiSeqParameterSetIdDelta[i] = (UInt*)malloc( ( m_uiNumSeqParameterSetMinus1[i]+1 ) * sizeof(UInt) );

	  for( j = 0; j <= m_uiNumSeqParameterSetMinus1[i]; j++ )
	    RNOK( pcReadIf->getUvlc( m_uiSeqParameterSetIdDelta[i][j], "ViewScalabilityInfoSei: init_seq_parameter_set_id_delta" ) );
      
	  RNOK( pcReadIf->getUvlc( m_uiNumSubsetSeqParameterSetMinus1[i], "ViewScalabilityInfoSei: num_subset_seq_parameter_set_minus1" ) );
	  for ( j = 0; j <= m_uiNumSubsetSeqParameterSetMinus1[ i ]; j++)
	  {
		  RNOK( pcReadIf->getUvlc( m_uiSubsetSeqParameterSetIdDelta[i][j], "ViewScalabilityInfoSei: subset_seq_parameter_set_id_delta" ) );
	  }
	  
	  RNOK( pcReadIf->getUvlc( m_uiNumPicParameterSetMinus1[i], "ViewScalabilityInfoSei: num_pic_parameter_set_minus1" ) );

	  if( m_uiPicParameterSetIdDelta[i] != NULL )
	    free( m_uiPicParameterSetIdDelta[i] );
	  m_uiPicParameterSetIdDelta[i] = (UInt*)malloc( ( m_uiNumPicParameterSetMinus1[i]+1 ) * sizeof(UInt) );

	  for( j = 0; j <= m_uiNumPicParameterSetMinus1[i]; j++ ) 
	    RNOK( pcReadIf->getUvlc( m_uiPicParameterSetIdDelta[i][j], "ViewScalabilityInfoSei: init_pic_parameter_set_id_delta" ) );
	}
	else
	  RNOK( pcReadIf->getUvlc( m_uiParameterSetsInfoSrcOpId[i], "ViewScalabilityInfoSei: parameter_sets_info_src_op_id" ) );
	
	if( m_bBitstreamRestrictionInfoPresentFlag[ i ] ) 
	{
		RNOK	( pcReadIf->getFlag( m_bMotionVectorsOverPicBoundariesFlag[i],"ScalableSEI:motion_vectors_over_pic_boundaries_flag"	) );
		RNOK	( pcReadIf->getUvlc( m_uiMaxBytesPerPicDenom[i],		"ScalableSEI:max_bytes_per_pic_denom" ) );
		RNOK	( pcReadIf->getUvlc( m_uiMaxBitsPerMbDenom[i],		"ScalableSEI:max_bits_per_mb_denom" ) );
		RNOK	( pcReadIf->getUvlc( m_uiLog2MaxMvLengthHorizontal[i],		"ScalableSEI:log2_max_mv_length_horizontal" ) );
		RNOK	( pcReadIf->getUvlc( m_uiLog2MaxMvLengthVertical[i],		"ScalableSEI:log2_max_mv_length_vertical" ) );
		RNOK	( pcReadIf->getUvlc( m_uiNumReorderFrames[i],		"ScalableSEI:num_reorder_frames" ) );
		RNOK	( pcReadIf->getUvlc( m_uiMaxDecFrameBuffering[i],		"ScalableSEI:max_dec_frame_buffering" ) );
	}
	//}}SEI JJ
  }
  return Err::m_nOK;

}
//JVT-W035 {
//////////////////////////////////////////////////////////////////////////
//
//			SCALABLE NESTING SEI
//
//////////////////////////////////////////////////////////////////////////
ErrVal
SEI::ScalableNestingSei::create( ScalableNestingSei* &rpcSeiMessage )
{
    rpcSeiMessage = new ScalableNestingSei();
	ROT( NULL == rpcSeiMessage );
	return Err::m_nOK;
}
ErrVal
SEI::ScalableNestingSei::destroy()
{
    delete this;
	return Err::m_nOK;
}

ErrVal
SEI::ScalableNestingSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
    UInt uiStartBits  = pcWriteIf->getNumberOfWrittenBits();
	UInt uiPayloadSize = 0;
	UInt uiIndex;
	RNOK( pcWriteIf->writeFlag( m_bAllPicturesInAuFlag, " ScalableNestingSei:AllPicturesInAuFlag " ) );
	if( m_bAllPicturesInAuFlag == 0 )
	{
		RNOK( pcWriteIf->writeUvlc( m_uiNumPicturesMinus1, "ScalableNestingSei:NumPicturesMinus1" ) );
		for( uiIndex = 0; uiIndex <= m_uiNumPicturesMinus1; uiIndex++ )
		{
			RNOK( pcWriteIf->writeCode( m_uiPicId[uiIndex], 10, " ScalableNestingSei:uiPicId " ) );
		}
		RNOK( pcWriteIf->writeCode( m_uiTemporalId, 3, " ScalableNestingSei:uiTemporalId " ) );
	}
	UInt uiBits = pcWriteIf->getNumberOfWrittenBits()-uiStartBits;
	UInt uiBitsMod8 = uiBits%8;
	if( uiBitsMod8 )
	{
		RNOK( pcWriteIf->writeCode(0, 8-uiBitsMod8, "SeiNestingZeroBits" ) );
	}
	uiBits = pcWriteIf->getNumberOfWrittenBits();
	uiPayloadSize = (uiBits+7)/8;

	return Err::m_nOK;
}

ErrVal
SEI::ScalableNestingSei::read( HeaderSymbolReadIf *pcReadIf )
{
	RNOK( pcReadIf->getFlag( m_bAllPicturesInAuFlag, " ScalableNestingSei:AllPicturesInAuFlag " ) );
	if( m_bAllPicturesInAuFlag == 0 )
	{
		RNOK( pcReadIf->getUvlc( m_uiNumPicturesMinus1, "ScalableNestingSei:NumPicturesMinus1" ) );
		for( UInt uiIndex = 0; uiIndex <= m_uiNumPicturesMinus1; uiIndex++ )
		{
			RNOK( pcReadIf->getCode( m_uiPicId[uiIndex], 10, " ScalableNestingSei:uiPicId " ) );
		}	
    	RNOK( pcReadIf->getCode( m_uiTemporalId, 3, " ScalableNestingSei:uiTemporalId " ) );
	}
	RNOK( pcReadIf->readZeroByteAlign() ); //nesting_zero_bit

	//Read the following SEI message
	UInt uiType, uiPayloadSize;
	while(1)
	{
		RNOK( pcReadIf->getCode( uiType, 8, " ScalableNestingSei:SEI type" ) );
		if( uiType != 0xff )
			break;
	}
	while(1)
	{
		RNOK( pcReadIf->getCode( uiPayloadSize, 8, " ScalableNestingSei:SEI payloadSize" ) );
		if( uiPayloadSize != 0xff )
			break;
	}
	switch( uiType )
	{
	case FULLFRAME_SNAPSHOT_SEI:
		{
			SEI::FullframeSnapshotSei* pcFullframeSnapshotSei;
			RNOK( SEI::FullframeSnapshotSei::create(pcFullframeSnapshotSei) );
			RNOK( pcFullframeSnapshotSei->read(pcReadIf) );			
			m_pcSEIMessage = pcFullframeSnapshotSei;
		//	RNOK( pcFullframeSnapshotSei->destroy() );  
			break;
		}
	//more case can be added here
	default:
		break;
	}

	return Err::m_nOK;
}

//Full Frame Snapshot sei message, simplified
ErrVal
SEI::FullframeSnapshotSei::create( FullframeSnapshotSei* &rpcSeiMessage )
{
    rpcSeiMessage = new FullframeSnapshotSei();
	ROT( NULL == rpcSeiMessage );
	return Err::m_nOK;
}

ErrVal
SEI::FullframeSnapshotSei::destroy()
{
    delete this;
	return Err::m_nOK;
}

ErrVal
SEI::FullframeSnapshotSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
	UInt uiStart = pcWriteIf->getNumberOfWrittenBits();
	UInt uiPayloadSize = 0;
	// bug fix for LSJ, ying
	RNOK( pcWriteIf->writeUvlc( getSnapShotId(), "FullframeSnapshotSei: SnapshotId" ) );

	uiPayloadSize = ( pcWriteIf->getNumberOfWrittenBits() - uiStart + 7 )/8;

	return Err::m_nOK;
}

ErrVal
SEI::FullframeSnapshotSei::read( HeaderSymbolReadIf *pcReadIf )
{
	RNOK( pcReadIf->getUvlc( m_uiSnapShotId, "FullframeSnapshotSei: SnapShotId" ) );

	return Err::m_nOK;
}
//////////////////////////////////////////////////////////////////////////
//
//			MULTIVIEW_SCENE_INFO SEI MESSAGE // SEI JVT-W060
//
//////////////////////////////////////////////////////////////////////////
ErrVal
SEI::MultiviewSceneInfoSei::create( MultiviewSceneInfoSei*& rpcSeiMessage )
{
    rpcSeiMessage = new MultiviewSceneInfoSei();
	ROT( NULL == rpcSeiMessage );
	return Err::m_nOK;
}
ErrVal
SEI::MultiviewSceneInfoSei::destroy()
{
    delete this;
	return Err::m_nOK;
}

ErrVal
SEI::MultiviewSceneInfoSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
    UInt uiStartBits  = pcWriteIf->getNumberOfWrittenBits();
	UInt uiPayloadSize = 0;
		
	RNOK( pcWriteIf->writeUvlc( m_uiMaxDisparity, "MultiviewSceneInfoSei:MaxDisparity" ) );
		
	UInt uiBits = pcWriteIf->getNumberOfWrittenBits()-uiStartBits;

	uiPayloadSize = (uiBits+7)/8;

	return Err::m_nOK;
}

ErrVal
SEI::MultiviewSceneInfoSei::read( HeaderSymbolReadIf *pcReadIf )
{
	RNOK( pcReadIf->getUvlc( m_uiMaxDisparity, "MultiviewSceneInfoSei:MaxDisparity" ) );
	
	return Err::m_nOK;
}
//////////////////////////////////////////////////////////////////////////
//
//			MULTIVIEW_ACQUISITION_INFO SEI MESSAGE // SEI JVT-W060
//
//////////////////////////////////////////////////////////////////////////
ErrVal
SEI::MultiviewAcquisitionInfoSei::create( MultiviewAcquisitionInfoSei*& rpcSeiMessage )
{
    rpcSeiMessage = new MultiviewAcquisitionInfoSei();
	ROT( NULL == rpcSeiMessage );
	return Err::m_nOK;
}
ErrVal
SEI::MultiviewAcquisitionInfoSei::destroy()
{
    delete this;
	return Err::m_nOK;
}


ErrVal
SEI::MultiviewAcquisitionInfoSei::write( HeaderSymbolWriteIf *pcWriteIf )// JVT-Z038
{
    UInt uiStartBits  = pcWriteIf->getNumberOfWrittenBits();
	UInt uiPayloadSize = 0;
	UInt uiIndex;
	UInt var_length=0;
	int i,j;
	
	RNOK( pcWriteIf->writeUvlc( m_uiNumViewMinus1, " MultiviewAcquisitionInfoSei:NumViewMinus1 " ) );	
	RNOK( pcWriteIf->writeFlag( m_bIntrinsicParamFlag, " MultiviewAcquisitionInfoSei:IntrinsicParamFlag " ) );
	if ( m_bIntrinsicParamFlag ) {
		RNOK( pcWriteIf->writeFlag( m_bIntrinsicParamsEqual, " MultiviewAcquisitionInfoSei:IntrinsicParamsEqual " ) );
		RNOK( pcWriteIf->writeUvlc( m_uiPrecFocalLength, " MultiviewAcquisitionInfoSei:PrecFocalLength " ) );
		RNOK( pcWriteIf->writeUvlc( m_uiPrecPrincipalPoint, " MultiviewAcquisitionInfoSei:PrecPrincipalPoint " ) );
		RNOK( pcWriteIf->writeUvlc( m_uiPrecRadialDistortion, " MultiviewAcquisitionInfoSei:PrecRadialDistortion " ) );
		for( uiIndex = 0; uiIndex <= m_uiNumViewMinus1; uiIndex++ )
		{ 
			RNOK( pcWriteIf->writeFlag( m_bSignFocalLengthX[uiIndex], " MultiviewAcquisitionInfoSei:SignFocalLengthX " ) );
			RNOK( pcWriteIf->writeCode(m_uiExponentFocalLengthX[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentFocalLengthX " ) );
			var_length = getVarLength(m_uiExponentFocalLengthX[uiIndex],m_uiPrecFocalLength);
			RNOK( pcWriteIf->writeCode(m_uiMantissaFocalLengthX[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaFocalLengthX " ) );
			RNOK( pcWriteIf->writeFlag( m_bSignFocalLengthY[uiIndex], " MultiviewAcquisitionInfoSei:SignFocalLengthY " ) );
			RNOK( pcWriteIf->writeCode(m_uiExponentFocalLengthY[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentFocalLengthY " ) );
			var_length = getVarLength(m_uiExponentFocalLengthY[uiIndex],m_uiPrecFocalLength);
			RNOK( pcWriteIf->writeCode(m_uiMantissaFocalLengthY[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaFocalLengthY " ) );
			
			RNOK( pcWriteIf->writeFlag( m_bSignPrincipalPointX[uiIndex], " MultiviewAcquisitionInfoSei:SignPrincipalPointX " ) );
			RNOK( pcWriteIf->writeCode(m_uiExponentPrincipalPointX[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentPrincipalPointX " ) );
			var_length = getVarLength(m_uiExponentPrincipalPointX[uiIndex],m_uiPrecPrincipalPoint);
			RNOK( pcWriteIf->writeCode(m_uiMantissaPrincipalPointX[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaPrincipalPointX " ) );
			RNOK( pcWriteIf->writeFlag( m_bSignPrincipalPointY[uiIndex], " MultiviewAcquisitionInfoSei:SignPrincipalPointY " ) );
			RNOK( pcWriteIf->writeCode(m_uiExponentPrincipalPointY[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentPrincipalPointY " ) );
			var_length = getVarLength(m_uiExponentPrincipalPointY[uiIndex],m_uiPrecPrincipalPoint);
			RNOK( pcWriteIf->writeCode(m_uiMantissaPrincipalPointY[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaPrincipalPointY " ) );

			RNOK( pcWriteIf->writeFlag( m_bSignRadialDistortion[uiIndex], " MultiviewAcquisitionInfoSei:SignRadialDistortion " ) );
			RNOK( pcWriteIf->writeCode(m_uiExponentRadialDistortion[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentRadialDistortion " ) );
			var_length = getVarLength(m_uiExponentRadialDistortion[uiIndex],m_uiPrecRadialDistortion);
			RNOK( pcWriteIf->writeCode(m_uiMantissaRadialDistortion[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaRadialDistortion " ) );
		}
	} 

	RNOK( pcWriteIf->writeFlag( m_bExtrinsicParamFlag, " MultiviewAcquisitionInfoSei:ExtrinsicParamFlag " ) );
	if (m_bExtrinsicParamFlag) {
		RNOK( pcWriteIf->writeUvlc( m_uiPrecRotationParam, " MultiviewAcquisitionInfoSei:PrecRotationParam " ) );
		RNOK( pcWriteIf->writeUvlc( m_uiPrecTranslationParam, " MultiviewAcquisitionInfoSei:PrecTranslationParam " ) );

		for( uiIndex = 0; uiIndex <= m_uiNumViewMinus1; uiIndex++ )
		{			
			for (i=0;i<3;i++)
			{
				RNOK( pcWriteIf->writeFlag( m_bSignTranslationParam[uiIndex][i], " MultiviewAcquisitionInfoSei:SignTranslationParam " ) );
				RNOK( pcWriteIf->writeCode(m_uiExponentTranslationParam[uiIndex][i],6, " MultiviewAcquisitionInfoSei:ExponentTranslationParam " ) );
				var_length = getVarLength(m_uiExponentTranslationParam[uiIndex][i],m_uiPrecTranslationParam);
				RNOK( pcWriteIf->writeCode(m_uiMantissaTranslationParam[uiIndex][i],var_length, " MultiviewAcquisitionInfoSei:MantissaTranslationParam " ) );
				
				for (j=0;j<3;j++) {
					RNOK( pcWriteIf->writeFlag( m_bSignRotationParam[uiIndex][i][j], " MultiviewAcquisitionInfoSei:SignRotationParam " ) );
					RNOK( pcWriteIf->writeCode(m_uiExponentRotationParam[uiIndex][i][j],6, " MultiviewAcquisitionInfoSei:ExponentRotationParam " ) );
					var_length = getVarLength(m_uiExponentRotationParam[uiIndex][i][j],m_uiPrecRotationParam);
					RNOK( pcWriteIf->writeCode(m_uiMantissaRotationParam[uiIndex][i][j],var_length, " MultiviewAcquisitionInfoSei:MantissaRotationParam " ) );				
				}
			}
			
		}	
	}
	
	UInt uiBits = pcWriteIf->getNumberOfWrittenBits()-uiStartBits;

	uiPayloadSize = (uiBits+7)/8;

	return Err::m_nOK;
}
ErrVal
SEI::MultiviewAcquisitionInfoSei::read( HeaderSymbolReadIf *pcReadIf )// JVT-Z038
{
		
	UInt uiIndex;
	UInt var_length;
	int i,j;
		
	
	RNOK( pcReadIf->getUvlc( m_uiNumViewMinus1, " MultiviewAcquisitionInfoSei:NumViewsMinus1 " ) );
	this->initialize_memory( m_uiNumViewMinus1+1);
		
	RNOK( pcReadIf->getFlag( m_bIntrinsicParamFlag, " MultiviewAcquisitionInfoSei:IntrinsicParamFlag " ) );
	if ( m_bIntrinsicParamFlag ) {
		RNOK( pcReadIf->getFlag( m_bIntrinsicParamsEqual, " MultiviewAcquisitionInfoSei:IntrinsicParamsEqual " ) );
		RNOK( pcReadIf->getUvlc( m_uiPrecFocalLength, " MultiviewAcquisitionInfoSei:PrecFocalLength " ) );
		RNOK( pcReadIf->getUvlc( m_uiPrecPrincipalPoint, " MultiviewAcquisitionInfoSei:PrecPrincipalPoint " ) );
		RNOK( pcReadIf->getUvlc( m_uiPrecRadialDistortion, " MultiviewAcquisitionInfoSei:PrecRadialDistortion " ) );	
		for( uiIndex = 0; uiIndex <= m_uiNumViewMinus1; uiIndex++ )
		{
			RNOK( pcReadIf->getFlag( m_bSignFocalLengthX[uiIndex], " MultiviewAcquisitionInfoSei:SignFocalLengthX " ) );
			RNOK( pcReadIf->getCode(m_uiExponentFocalLengthX[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentFocalLengthX " ) );
			var_length = getVarLength(m_uiExponentFocalLengthX[uiIndex],m_uiPrecFocalLength);
			RNOK( pcReadIf->getCode(m_uiMantissaFocalLengthX[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaFocalLengthX " ) );
			RNOK( pcReadIf->getFlag( m_bSignFocalLengthY[uiIndex], " MultiviewAcquisitionInfoSei:SignFocalLengthY " ) );
			RNOK( pcReadIf->getCode(m_uiExponentFocalLengthY[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentFocalLengthY " ) );
			var_length = getVarLength(m_uiExponentFocalLengthY[uiIndex],m_uiPrecFocalLength);
			RNOK( pcReadIf->getCode(m_uiMantissaFocalLengthY[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaFocalLengthY " ) );
			
			RNOK( pcReadIf->getFlag( m_bSignPrincipalPointX[uiIndex], " MultiviewAcquisitionInfoSei:SignPrincipalPointX " ) );
			RNOK( pcReadIf->getCode(m_uiExponentPrincipalPointX[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentPrincipalPointX " ) );
			var_length = getVarLength(m_uiExponentPrincipalPointX[uiIndex],m_uiPrecPrincipalPoint);
			RNOK( pcReadIf->getCode(m_uiMantissaPrincipalPointX[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaPrincipalPointX " ) );
			RNOK( pcReadIf->getFlag( m_bSignPrincipalPointY[uiIndex], " MultiviewAcquisitionInfoSei:SignPrincipalPointY " ) );
			RNOK( pcReadIf->getCode(m_uiExponentPrincipalPointY[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentPrincipalPointY " ) );
			var_length = getVarLength(m_uiExponentPrincipalPointY[uiIndex],m_uiPrecPrincipalPoint);
			RNOK( pcReadIf->getCode(m_uiMantissaPrincipalPointY[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaPrincipalPointY " ) );

			RNOK( pcReadIf->getFlag( m_bSignRadialDistortion[uiIndex], " MultiviewAcquisitionInfoSei:SignRadialDistortion " ) );
			RNOK( pcReadIf->getCode(m_uiExponentRadialDistortion[uiIndex],6, " MultiviewAcquisitionInfoSei:ExponentRadialDistortion " ) );
			var_length = getVarLength(m_uiExponentRadialDistortion[uiIndex],m_uiPrecRadialDistortion);
			RNOK( pcReadIf->getCode(m_uiMantissaRadialDistortion[uiIndex],var_length, " MultiviewAcquisitionInfoSei:MantissaRadialDistortion " ) );
		}
	}
	RNOK( pcReadIf->getFlag( m_bExtrinsicParamFlag, " MultiviewAcquisitionInfoSei:ExtrinsicParamFlag " ) );
	if (m_bExtrinsicParamFlag) {
		RNOK( pcReadIf->getUvlc( m_uiPrecRotationParam, " MultiviewAcquisitionInfoSei:PrecRotationParam " ) );
		RNOK( pcReadIf->getUvlc( m_uiPrecTranslationParam, " MultiviewAcquisitionInfoSei:PrecTranslationParam " ) );

		for( uiIndex = 0; uiIndex <= m_uiNumViewMinus1; uiIndex++ )
		{			
			for (i=0; i<3; i++) {
				RNOK( pcReadIf->getFlag( m_bSignTranslationParam[uiIndex][i], " MultiviewAcquisitionInfoSei:SignTranslationParam " ) );
				RNOK( pcReadIf->getCode(m_uiExponentTranslationParam[uiIndex][i],6, " MultiviewAcquisitionInfoSei:ExponentTranslationParam " ) );
				var_length = getVarLength(m_uiExponentTranslationParam[uiIndex][i],m_uiPrecTranslationParam);
				RNOK( pcReadIf->getCode(m_uiMantissaTranslationParam[uiIndex][i],var_length, " MultiviewAcquisitionInfoSei:MantissaTranslationParam " ) );
				
				for (j=0; j<3; j++) {
					RNOK( pcReadIf->getFlag( m_bSignRotationParam[uiIndex][i][j], " MultiviewAcquisitionInfoSei:SignRotationParam " ) );
					RNOK( pcReadIf->getCode(m_uiExponentRotationParam[uiIndex][i][j],6, " MultiviewAcquisitionInfoSei:ExponentRotationParam " ) );
					var_length = getVarLength(m_uiExponentRotationParam[uiIndex][i][j],m_uiPrecRotationParam);
					RNOK( pcReadIf->getCode(m_uiMantissaRotationParam[uiIndex][i][j],var_length, " MultiviewAcquisitionInfoSei:MantissaRotationParam " ) );				
				}

			}

		}	
	}
	
	return Err::m_nOK;
}

ErrVal
//SEI::xRead( HeaderSymbolReadIf* pcReadIf,
//            SEIMessage*&        rpcSEIMessage) 
SEI::xRead( HeaderSymbolReadIf* pcReadIf,
            SEIMessage*&        rpcSEIMessage /*, UInt NumViewsMinus1 */) // SEI JVT-W060 Nov. 30
{
  MessageType eMessageType = RESERVED_SEI;
  UInt        uiSize       = 0;

  RNOK( xReadPayloadHeader( pcReadIf, eMessageType, uiSize) );

  RNOK( xCreate( rpcSEIMessage, eMessageType, uiSize ) )
	  
//  rpcSEIMessage->NumOfViewMinus1 = NumViewsMinus1; // SEI JVT-W060 Nov. 30
//JVT-AB025{{ omit parsing
if( eMessageType == VIEW_DEPENDENCY_STRUCTURE_SEI )
{
	return Err::m_nOK;
}
//JVT-AB025 }}
//JVT-W080, omit parsing
  if( eMessageType == PARALLEL_DEC_SEI )
	{
//		ParallelDecodingSEI* pcPdSEI = (ParallelDecodingSEI*)rpcSEIMessage;
// fix for Liu Hui, Ying

		return Err::m_nOK;   
	}
//~JVT-W080
 
  RNOK( rpcSEIMessage->read( pcReadIf ) );
  RNOK( pcReadIf->readByteAlign() );
  return Err::m_nOK;
}


ErrVal
SEI::xWrite( HeaderSymbolWriteIf* pcWriteIf,
             HeaderSymbolWriteIf* pcWriteTestIf,
             SEIMessage*          pcSEIMessage )
{

  UInt uiStart  = pcWriteTestIf->getNumberOfWrittenBits();
  RNOK( pcSEIMessage->write( pcWriteTestIf ) );
  UInt uiBits = pcWriteTestIf->getNumberOfWrittenBits() - uiStart;

  UInt uiSize = (uiBits+7)/8;

  RNOK( xWritePayloadHeader( pcWriteIf, pcSEIMessage->getMessageType(), uiSize ) );
  RNOK( pcSEIMessage->write( pcWriteIf ) );
  UInt uiAlignedBits = 8 - (uiBits&7);
  if( uiAlignedBits != 0 && uiAlignedBits != 8)
  {
    RNOK( pcWriteIf->writeCode( 1<<(uiAlignedBits-1), uiAlignedBits, "SEI: alignment_bits" ) );
  }
  return Err::m_nOK;
}



ErrVal
SEI::xReadPayloadHeader( HeaderSymbolReadIf*  pcReadIf,
                         MessageType&         reMessageType,
                         UInt&                ruiSize)
{
  { // type
    UInt uiTemp = 0xFF;
    UInt uiSum  = 0;
    while( 0xFF == uiTemp )
    {
      RNOK( pcReadIf->getCode( uiTemp, 8, "SEI: payload type") );
      uiSum += uiTemp;
    }
    reMessageType = (RESERVED_SEI <= uiSum ) ? RESERVED_SEI : MessageType( uiSum );
  }

  { // size
    UInt uiTemp  = 0xFF;
    UInt uiSum  = 0;

    while( 0xFF == uiTemp )
    {
      RNOK( pcReadIf->getCode( uiTemp, 8, "SEI: payload size") );
      uiSum += uiTemp;
    }
    ruiSize = uiSum;
  }
  return Err::m_nOK;
}



ErrVal
SEI::xCreate( SEIMessage*&  rpcSEIMessage,
              MessageType   eMessageType,
              UInt          uiSize ) 
{
  switch( eMessageType )
  {
    case SUB_SEQ_INFO:  return SubSeqInfo ::create( (SubSeqInfo*&)  rpcSEIMessage );
    case SCALABLE_SEI:  return ScalableSei::create( (ScalableSei*&) rpcSEIMessage );
    case SUB_PIC_SEI:   return SubPicSei::create	( (SubPicSei*&)		rpcSEIMessage );
	case MOTION_SEI:	return MotionSEI::create( (MotionSEI*&) rpcSEIMessage );
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    case QUALITYLEVEL_SEI: return QualityLevelSEI::create((QualityLevelSEI*&) rpcSEIMessage);
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
  	case NON_REQUIRED_SEI: return NonRequiredSei::create((NonRequiredSei*&) rpcSEIMessage); 
	// JVT-S080 LMI {
    case SCALABLE_SEI_LAYERS_NOT_PRESENT:  return ScalableSeiLayersNotPresent::create( (ScalableSeiLayersNotPresent*&) rpcSEIMessage );
	case SCALABLE_SEI_DEPENDENCY_CHANGE:   return ScalableSeiDependencyChange::create( (ScalableSeiDependencyChange*&) rpcSEIMessage );
	// JVT-S080 LMI }
	case SCALABLE_NESTING_SEI: return ScalableNestingSei::create( (ScalableNestingSei*&) rpcSEIMessage ); // SEI
	case VIEW_SCALABILITY_INFO_SEI: return ViewScalabilityInfoSei ::create( (ViewScalabilityInfoSei*&) rpcSEIMessage ); //SEI
	case MULTIVIEW_SCENE_INFO_SEI:  return MultiviewSceneInfoSei::create( (MultiviewSceneInfoSei*&) rpcSEIMessage ); //SEI JVT-W060
	case MULTIVIEW_ACQUISITION_INFO_SEI:  return MultiviewAcquisitionInfoSei::create( (MultiviewAcquisitionInfoSei*&) rpcSEIMessage ); //SEI JVT-W060

//JVT-W080
		case PARALLEL_DEC_SEI: return ParallelDecodingSEI::create(( ParallelDecodingSEI*&) rpcSEIMessage );
//~JVT-W080
    case NON_REQ_VIEW_INFO_SEI: return NonReqViewInfoSei::create((NonReqViewInfoSei*&)rpcSEIMessage);   //JVT-AB025
    case VIEW_DEPENDENCY_STRUCTURE_SEI: return ViewDependencyStructureSei::create((ViewDependencyStructureSei*&)rpcSEIMessage); //JVT-AB025
    case OP_NOT_PRESENT_SEI: return OPNotPresentSei:: create((OPNotPresentSei*&) rpcSEIMessage);//JVT-AB025
		default :           return ReservedSei::create( (ReservedSei*&) rpcSEIMessage, uiSize );
  }
  //return Err::m_nOK;
}


ErrVal
SEI::xWritePayloadHeader( HeaderSymbolWriteIf*  pcWriteIf,
                          MessageType           eMessageType,
                          UInt                  uiSize )
{
  { // type
    UInt uiTemp = eMessageType;
    UInt uiByte = 0xFF;

    while( 0xFF == uiByte )
    {
      uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
      uiTemp -= 0xFF;
      RNOK( pcWriteIf->writeCode( uiByte, 8, "SEI: payload type") );
    }
  }

  { // size
    UInt uiTemp = uiSize;
    UInt uiByte = 0xFF;

    while( 0xFF == uiByte )
    {
      uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
      uiTemp -= 0xFF;
      RNOK( pcWriteIf->writeCode( uiByte, 8, "SEI: payload size") );
    }
  }

  return Err::m_nOK;
}





ErrVal
SEI::ReservedSei::create( ReservedSei*& rpcReservedSei,
                          UInt          uiSize )
{
  rpcReservedSei = new ReservedSei (uiSize);
  ROT( NULL == rpcReservedSei );
  return Err::m_nOK;
}


ErrVal
SEI::ReservedSei::write( HeaderSymbolWriteIf* pcWriteIf )
{
  AF();
  return Err::m_nOK;
}


ErrVal
SEI::ReservedSei::read( HeaderSymbolReadIf* pcReadIf )
{
  AF();
  return Err::m_nOK;
}





//////////////////////////////////////////////////////////////////////////
// 
//      S U B S E Q U E N C E     S E I
//
//////////////////////////////////////////////////////////////////////////

ErrVal
SEI::SubSeqInfo::create( SubSeqInfo*& rpcSEIMessage )
{
  SubSeqInfo* pcSubSeqInfo = new SubSeqInfo();
  rpcSEIMessage = pcSubSeqInfo;
  ROT( NULL == rpcSEIMessage )
  return Err::m_nOK;
}


ErrVal
SEI::SubSeqInfo::write( HeaderSymbolWriteIf* pcWriteIf )
{
  RNOK(   pcWriteIf->writeUvlc( m_uiSubSeqLayerNum,       "SubSeqSEI: sub_seq_layer_num") );
  RNOK(   pcWriteIf->writeUvlc( m_uiSubSeqId,             "SubSeqSEI: sub_seq_layer_id") );
  RNOK(   pcWriteIf->writeFlag( m_bFirstRefPicFlag,       "SubSeqSEI: first_ref_pic_flag" ) );
  RNOK(   pcWriteIf->writeFlag( m_bLeadingNonRefPicFlag,  "SubSeqSEI: leading_non_ref_pic_flag" ) );
  RNOK(   pcWriteIf->writeFlag( m_bLastPicFlag,           "SubSeqSEI: last_pic_flag" ) );
  RNOK(   pcWriteIf->writeFlag( m_bSubSeqFrameNumFlag,    "SubSeqSEI: sub_seq_frame_num_flag" ) );
  if( m_bSubSeqFrameNumFlag )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiSubSeqFrameNum,       "SubSeqSEI: sub_seq_frame_num") );
  }
  return Err::m_nOK;
}


ErrVal
SEI::SubSeqInfo::read ( HeaderSymbolReadIf* pcReadIf )
{
  RNOK(   pcReadIf->getUvlc( m_uiSubSeqLayerNum,       "SubSeqSEI: sub_seq_layer_num") );
  RNOK(   pcReadIf->getUvlc( m_uiSubSeqId,             "SubSeqSEI: sub_seq_layer_id") );
  RNOK(   pcReadIf->getFlag( m_bFirstRefPicFlag,       "SubSeqSEI: first_ref_pic_flag" ) );
  RNOK(   pcReadIf->getFlag( m_bLeadingNonRefPicFlag,  "SubSeqSEI: leading_non_ref_pic_flag" ) );
  RNOK(   pcReadIf->getFlag( m_bLastPicFlag,           "SubSeqSEI: last_pic_flag" ) );
  RNOK(   pcReadIf->getFlag( m_bSubSeqFrameNumFlag,    "SubSeqSEI: sub_seq_frame_num_flag" ) );
  if( m_bSubSeqFrameNumFlag )
  {
    RNOK( pcReadIf->getUvlc( m_uiSubSeqFrameNum,       "SubSeqSEI: sub_seq_frame_num") );
  }
  return Err::m_nOK;
}


ErrVal
SEI::SubSeqInfo::init( UInt  uiSubSeqLayerNum,
                       UInt  uiSubSeqId,
                       Bool  bFirstRefPicFlag,
                       Bool  bLeadingNonRefPicFlag,
                       Bool  bLastPicFlag,
                       Bool  bSubSeqFrameNumFlag,
                       UInt  uiSubSeqFrameNum ) 

{
  m_uiSubSeqLayerNum      = uiSubSeqLayerNum;
  m_uiSubSeqId            = uiSubSeqId;
  m_bFirstRefPicFlag      = bFirstRefPicFlag;
  m_bLeadingNonRefPicFlag = bLeadingNonRefPicFlag;
  m_bLastPicFlag          = bLastPicFlag;
  m_bSubSeqFrameNumFlag   = bSubSeqFrameNumFlag;
  m_uiSubSeqFrameNum      = uiSubSeqFrameNum;
  return Err::m_nOK;
}





//////////////////////////////////////////////////////////////////////////
// 
//      S C A L A B L E     S E I
//
//////////////////////////////////////////////////////////////////////////

SEI::ScalableSei::ScalableSei	() 
: SEIMessage									( SCALABLE_SEI )
, m_num_layers_minus1					( 0	)
{	
	::memset( m_layer_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_fgs_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );  //JVT-S036 
	::memset( m_sub_pic_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_sub_region_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_iroi_slice_division_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) ); //JVT-S036 
	::memset( m_profile_level_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_bitrate_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_frm_rate_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_frm_size_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_dependency_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_init_parameter_sets_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );

	::memset( m_exact_interlayer_pred_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );  //JVT-S036 

	::memset( m_layer_profile_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_layer_constraint_set0_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_constraint_set1_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_constraint_set2_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_constraint_set3_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_level_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

//JVT-S036  start
	::memset( m_profile_level_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//

	::memset( m_simple_priority_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
    ::memset( m_discardable_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );//
	::memset( m_temporal_level, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_dependency_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_quality_level, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_avg_bitrate, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_max_bitrate_layer, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
	::memset( m_max_bitrate_decoded_picture, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
	::memset( m_max_bitrate_calc_window, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//

	::memset( m_constant_frm_rate_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_avg_frm_rate, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_frm_rate_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//

	::memset( m_frm_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_frm_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_frm_size_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//

	::memset( m_base_region_layer_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_dynamic_rect_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_horizontal_offset, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_vertical_offset, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_region_width, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_region_height, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_sub_region_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //

	::memset( m_roi_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_iroi_slice_division_type, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_grid_slice_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_grid_slice_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_num_slice_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_first_mb_in_slice, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_slice_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_slice_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_slice_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );

	::memset( m_num_directly_dependent_layers, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_directly_dependent_layer_id_delta_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) ); //

	::memset( m_layer_dependency_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //
//JVT-S036  end

	::memset( m_num_init_seq_parameter_set_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_init_seq_parameter_set_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_num_init_pic_parameter_set_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_init_pic_parameter_set_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );

	::memset( m_init_parameter_sets_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //JVT-S036 

}


SEI::ScalableSei::~ScalableSei()
{
	// JVT-S054 (ADD) ->
	UInt i;
	for( i = 0; i < MAX_SCALABLE_LAYERS; i++ )
	{
		if ( m_first_mb_in_slice[i] != NULL )
		{
			free( m_first_mb_in_slice[i] );
			m_first_mb_in_slice[i] = NULL;
		}
		if ( m_slice_width_in_mbs_minus1[i] != NULL )
		{
			free( m_slice_width_in_mbs_minus1[i] );
			m_slice_width_in_mbs_minus1[i] = NULL;
		}
		if ( m_slice_height_in_mbs_minus1[i] != NULL )
		{
			free( m_slice_height_in_mbs_minus1[i] );
			m_slice_height_in_mbs_minus1[i] = NULL;
		}
		if ( m_slice_id[i] != NULL )
		{
			free( m_slice_id[i] );
			m_slice_id[i] = NULL;
		}
	}
	// JVT-S054 (ADD) <-
}

ErrVal
SEI::ScalableSei::create( ScalableSei*& rpcSeiMessage )
{
	rpcSeiMessage = new ScalableSei();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}


ErrVal
SEI::ScalableSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
  UInt i=0, j=0;

	ROF( m_num_layers_minus1+1 );
	RNOK		( pcWriteIf->writeUvlc(m_num_layers_minus1,													"ScalableSEI: num_layers_minus1"											) );

	for( i = 0; i <= m_num_layers_minus1; i++ )
	{
		if (0 < m_aiNumRoi[m_dependency_id[i]])
		{
			m_sub_pic_layer_flag[i] = true;
			m_roi_id[i]				= m_aaiRoiID[m_dependency_id[i]][0];
		}		

		RNOK	( pcWriteIf->writeCode( m_layer_id[i],												8,		"ScalableSEI: layer_id"															) );
	//JVT-S036  start
		//RNOK	( pcWriteIf->writeFlag( m_fgs_layer_flag[i],												"ScalableSEI: fgs_layer_flag"												) );
		RNOK	( pcWriteIf->writeCode( m_simple_priority_id[i],					6,		"ScalableSEI: simple_priority_id"										) ); 
		RNOK    ( pcWriteIf->writeFlag( m_discardable_flag[i],								"ScalableSEI: discardable_flag"											) );  
		RNOK	( pcWriteIf->writeCode( m_temporal_level[i],								3,		"ScalableSEI: temporal_level"												) );
		RNOK	( pcWriteIf->writeCode( m_dependency_id[i],							3,		"ScalableSEI: dependency_level"											) );
		RNOK	( pcWriteIf->writeCode( m_quality_level[i],									2,		"ScalableSEI: quality_level"													) );
	
		RNOK	( pcWriteIf->writeFlag( m_sub_pic_layer_flag[i],										"ScalableSEI: sub_pic_layer_flag"										) );
		RNOK	( pcWriteIf->writeFlag( m_sub_region_layer_flag[i],									"ScalableSEI: sub_region_layer_flag"									) );
		RNOK	( pcWriteIf->writeFlag( m_iroi_slice_division_info_present_flag[i],					"ScalableSEI: iroi_slice_division_info_present_flag"					) ); 
		RNOK	( pcWriteIf->writeFlag( m_profile_level_info_present_flag[i],				"ScalableSEI: profile_level_info_present_flag"				) );
	//JVT-S036  end
		RNOK	( pcWriteIf->writeFlag( m_bitrate_info_present_flag[i],							"ScalableSEI: bitrate_info_present_flag"							) );
		RNOK	( pcWriteIf->writeFlag( m_frm_rate_info_present_flag[i],						"ScalableSEI: frm_rate_info_present_flag"						) );
		RNOK	( pcWriteIf->writeFlag( m_frm_size_info_present_flag[i],						"ScalableSEI: frm_size_info_present_flag"						) );
		RNOK	( pcWriteIf->writeFlag( m_layer_dependency_info_present_flag[i],		"ScalableSEI: layer_dependency_info_present_flag"		) );
		RNOK	( pcWriteIf->writeFlag( m_init_parameter_sets_info_present_flag[i],	"ScalableSEI: init_parameter_sets_info_present_flag" ) );
		RNOK	( pcWriteIf->writeFlag( m_exact_interlayer_pred_flag[i],						"ScalableSEI: exact_interlayer_pred_flag"                      ) );//JVT-S036  

		if ( m_profile_level_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_layer_profile_idc[i],							8,		"ScalableSEI: layer_profile_idc"											) );
			RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set0_flag[i],					"ScalableSEI: layer_constraint_set0_flag"						) );
			RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set1_flag[i],					"ScalableSEI: layer_constraint_set1_flag"						) );
			RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set2_flag[i],					"ScalableSEI: layer_constraint_set2_flag"						) );
			RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set3_flag[i],					"ScalableSEI: layer_constraint_set3_flag"						) );
			RNOK	( pcWriteIf->writeCode( 0,																	4,		"ScalableSEI: reserved_zero_4bits"										) );
			RNOK	( pcWriteIf->writeCode( m_layer_level_idc[i],								8,		"ScalableSEI: layer_level_idc"												) );
		}

		else
		{//JVT-S036 
			RNOK	( pcWriteIf->writeUvlc( m_profile_level_info_src_layer_id_delta[i], "ScalableSEI: profile_level_info_src_layer_id_delta"      ) ); 
		}

/*		if ( m_decoding_dependency_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_simple_priority_id[i],					6,		"ScalableSEI: simple_priority_id"										) ); 
			RNOK    ( pcWriteIf->writeFlag( m_discardable_flag[i],								"ScalableSEI: discardable_flag"											) );  
			RNOK	( pcWriteIf->writeCode( m_temporal_level[i],								3,		"ScalableSEI: temporal_level"												) );
			RNOK	( pcWriteIf->writeCode( m_dependency_id[i],							3,		"ScalableSEI: dependency_level"											) );
			RNOK	( pcWriteIf->writeCode( m_quality_level[i],									2,		"ScalableSEI: quality_level"													) );
		}
JVT-S036  */
		if ( m_bitrate_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_avg_bitrate[i],										16,		"ScalableSEI: avg_bitrate"														) );
//JVT-S036  start
			RNOK	( pcWriteIf->writeCode( m_max_bitrate_layer[i],										16,		"ScalableSEI: max_bitrate_layer"											) );
			RNOK	( pcWriteIf->writeCode( m_max_bitrate_decoded_picture[i],										16,		"ScalableSEI: max_bitrate_decoded_picture"						) );
			RNOK	( pcWriteIf->writeCode( m_max_bitrate_calc_window[i],							16,		"ScalableSEI: max_bitrate_calc_window"											) );
//JVT-S036  end
		}
		if ( m_frm_rate_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_constant_frm_rate_idc[i],			2,		"ScalableSEI: constant_frm_bitrate_idc"							) );
			RNOK	( pcWriteIf->writeCode( m_avg_frm_rate[i],									16,		"ScalableSEI: avg_frm_rate"													) );
		}
		else
		{
	//JVT-S036 
			RNOK	(pcWriteIf->writeUvlc( m_frm_rate_info_src_layer_id_delta[i],	"ScalableSEI: frm_rate_info_src_layer_id_delta"			) );
		}

		if ( m_frm_size_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeUvlc( m_frm_width_in_mbs_minus1[i],							"ScalableSEI: frm_width_in_mbs_minus1"								) );
			RNOK	( pcWriteIf->writeUvlc( m_frm_height_in_mbs_minus1[i],						"ScalableSEI: frm_height_in_mbs_minus1"							) );
		}
		else
		{//JVT-S036 
			RNOK	(pcWriteIf->writeUvlc( m_frm_size_info_src_layer_id_delta[i],	"ScalableSEI: frm_size_info_src_layer_id_delta"			) );
		}

		if ( m_sub_region_layer_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode ( m_base_region_layer_id[i],					8,		"ScalableSEI: base_region_layer_id"									) );
			RNOK	( pcWriteIf->writeFlag ( m_dynamic_rect_flag[i],									"ScalableSEI: dynamic_rect_flag"											) );
			if ( m_dynamic_rect_flag[i] )
			{
				RNOK	( pcWriteIf->writeCode ( m_horizontal_offset[i],					16,		"ScalableSEI: horizontal_offset"											) );
				RNOK	( pcWriteIf->writeCode ( m_vertical_offset[i],						16,		"ScalableSEI: vertical_offset"												) );
				RNOK	( pcWriteIf->writeCode ( m_region_width[i],								16,		"ScalableSEI: region_width"													) );
				RNOK	( pcWriteIf->writeCode ( m_region_height[i],							16,		"ScalableSEI: region_height"													) );
			}
		}
		else
		{//JVT-S036 
			RNOK	( pcWriteIf->writeUvlc( m_sub_region_info_src_layer_id_delta[i],		"ScalableSEI: sub_region_info_src_layer_id_delta"				) );
		}

		if( m_sub_pic_layer_flag[i] )
		{//JVT-S036 
			RNOK	( pcWriteIf->writeCode( m_roi_id[i],		3,								"Scalable: roi_id"					) );
		}

	//JVT-S036  start
		if ( m_iroi_slice_division_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_iroi_slice_division_type[i],		2,		"ScalableSEI:iroi_slice_division_type" ) );
			if( m_iroi_slice_division_type[i] == 0 )
			{
				RNOK	( pcWriteIf->writeUvlc( m_grid_slice_width_in_mbs_minus1[i],    "ScalableSEI:grid_slice_width_in_mbs_minus1" ) );
				RNOK	( pcWriteIf->writeUvlc( m_grid_slice_height_in_mbs_minus1[i],    "ScalableSEI:grid_slice_height_in_mbs_minus1" ) );
			}
			else if( m_iroi_slice_division_type[i] == 1 )
			{
				RNOK	( pcWriteIf->writeUvlc( m_num_slice_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
				for ( j = 0; j <= m_num_slice_minus1[i]; j++ )
				{
					RNOK	( pcWriteIf->writeUvlc( m_first_mb_in_slice[i][j],				"ScalableSEI: first_mb_in_slice" ) );
					RNOK	( pcWriteIf->writeUvlc( m_slice_width_in_mbs_minus1[i][j],		"ScalableSEI:slice_width_in_mbs_minus1" ) );
					RNOK	( pcWriteIf->writeUvlc( m_slice_height_in_mbs_minus1[i][j],		"ScalableSEI:slice_height_in_mbs_minus1" ) );
				}
			}
      // JVT-S054 (REPLACE): Typo error
			//else if ( m_iroi_slice_division_type[1] == 2 )
			else if ( m_iroi_slice_division_type[i] == 2 )
			{
				RNOK	( pcWriteIf->writeUvlc( m_num_slice_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
    		// JVT-S054 (REPLACE) ->
        /*
				UInt uiFrameHeightInMb = m_slice_height_in_mbs_minus1[i][j] + 1;
				UInt uiFrameWidthInMb  = m_slice_width_in_mbs_minus1[i][j] + 1;
				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
				for ( j = 0; j < uiPicSizeInMbs; j++ )
				{
					RNOK	( pcWriteIf->writeUvlc( m_slice_id[i][j],		"ScalableSEI:slice_id"		   ) );
				}
        */
				UInt uiFrameHeightInMb = m_slice_height_in_mbs_minus1[i][j] + 1;
				UInt uiFrameWidthInMb  = m_slice_width_in_mbs_minus1[i][j] + 1;
				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
			  UInt uiWriteBits = (UInt) ceil( log( (double) (m_num_slice_minus1[i] + 1) ) / log(2.) );
				if (uiWriteBits == 0)
					uiWriteBits = 1;
				for ( j = 0; j < uiPicSizeInMbs; j++ )
				{
					RNOK	( pcWriteIf->writeCode ( m_slice_id[i][j],	uiWriteBits,		"ScalableSEI: slice_id") );
				}
    		// JVT-S054 (REPLACE) <-
			}
		}
	//JVT-S036  end
		if ( m_layer_dependency_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeUvlc ( m_num_directly_dependent_layers[i],			"ScalableSEI: num_directly_dependent_layers"					) );
// BUG_FIX liuhui{
		    for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
		    {
		      RNOK( pcWriteIf->writeUvlc (m_directly_dependent_layer_id_delta_minus1[i][j],      "ScalableSEI: directly_dependent_layers_id_delta"		) );  //JVT-S036 
		    }
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 
			RNOK( pcWriteIf->writeUvlc(m_layer_dependency_info_src_layer_id_delta[i],	 "ScalableSEI: layer_dependency_info_src_layer_id_delta"     ) );
		}

		if ( m_init_parameter_sets_info_present_flag[i] ) 
		{

// BUG_FIX liuhui{
			RNOK	( pcWriteIf->writeUvlc ( m_num_init_seq_parameter_set_minus1[i],	"ScalableSEI: num_init_seq_parameter_set_minus1"			) );
			for( j = 0; j <= m_num_init_seq_parameter_set_minus1[i]; j++ )
			{
			  RNOK( pcWriteIf->writeUvlc( m_init_seq_parameter_set_id_delta[i][j],  "ScalableSEI: init_seq_parameter_set_id_delta"				    ) );
			}

			RNOK    ( pcWriteIf->writeUvlc ( m_num_init_pic_parameter_set_minus1[i],	"ScalableSEI: num_init_pic_parameter_set_minus1"			) );
			for( j = 0; j <= m_num_init_pic_parameter_set_minus1[i]; j++ )
			{
			  RNOK ( pcWriteIf->writeUvlc ( m_init_pic_parameter_set_id_delta[i][j],"ScalableSEI: init_pic_parameter_set_id_delta"				    ) );
			}
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036  
			RNOK	(pcWriteIf->writeUvlc( m_init_parameter_sets_info_src_layer_id_delta[i], "ScalableSEI: init_parameter_sets_info_src_layer_id_delta"	) );
		}
	}

	return Err::m_nOK;
}
ErrVal
SEI::ScalableSei::read ( HeaderSymbolReadIf *pcReadIf )
{
  UInt i, j=0;
  UInt rl;//JVT-S036  

	RNOK	( pcReadIf->getUvlc( m_num_layers_minus1 ,																""	) );

	for ( i = 0; i <= m_num_layers_minus1; i++ )
	{
		RNOK	( pcReadIf->getCode( m_layer_id[i],																	8,			""	) );
	//JVT-S036  start
//		RNOK	( pcReadIf->getFlag( m_fgs_layer_flag[i],																""	) ); 
		RNOK	( pcReadIf->getCode( m_simple_priority_id[i],													6,		"" ) );	
		RNOK	( pcReadIf->getFlag( m_discardable_flag[i],													"" ) );     
		RNOK	( pcReadIf->getCode( m_temporal_level[i],													3,		""	) );
		RNOK	( pcReadIf->getCode( m_dependency_id[i],												3,		""	) );
		RNOK	( pcReadIf->getCode( m_quality_level[i],													2,		""	) );
	
		RNOK	( pcReadIf->getFlag( m_sub_pic_layer_flag[i],														""	) );
		RNOK	( pcReadIf->getFlag( m_sub_region_layer_flag[i],													""	) );
		RNOK	( pcReadIf->getFlag( m_iroi_slice_division_info_present_flag[i],						""  ) );  
		RNOK	( pcReadIf->getFlag( m_profile_level_info_present_flag[i],								""	) );
   //JVT-S036  end	
		RNOK	( pcReadIf->getFlag( m_bitrate_info_present_flag[i],											""	) );
		RNOK	( pcReadIf->getFlag( m_frm_rate_info_present_flag[i],											""	) );
		RNOK	( pcReadIf->getFlag( m_frm_size_info_present_flag[i],											""	) );
		RNOK	( pcReadIf->getFlag( m_layer_dependency_info_present_flag[i],								""	) );
		RNOK	( pcReadIf->getFlag( m_init_parameter_sets_info_present_flag[i],						""	) );
		RNOK	( pcReadIf->getFlag( m_exact_interlayer_pred_flag[i],											""  ) );	//JVT-S036 

		if( m_profile_level_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_layer_profile_idc[i],											8,		""	) );
			RNOK	( pcReadIf->getFlag( m_layer_constraint_set0_flag[i],										""	) );
			RNOK	( pcReadIf->getFlag( m_layer_constraint_set1_flag[i],										""	) );
			RNOK	( pcReadIf->getFlag( m_layer_constraint_set2_flag[i],										""	) );
			RNOK	( pcReadIf->getFlag( m_layer_constraint_set3_flag[i],										""	) );
			UInt uiReserved;
			RNOK	( pcReadIf->getCode( uiReserved,																	4,		""	) );
			RNOK	( pcReadIf->getCode( m_layer_level_idc[i],												8,		""	) );
		}
		else
		{//JVT-S036 
			RNOK	( pcReadIf->getUvlc( m_profile_level_info_src_layer_id_delta[i],						""  ) );
			rl = m_layer_id[i] - m_profile_level_info_src_layer_id_delta[i];
			m_layer_profile_idc[i] = m_layer_profile_idc[rl];
			m_layer_constraint_set0_flag[i] = m_layer_constraint_set0_flag[rl];
			m_layer_constraint_set1_flag[i] = m_layer_constraint_set1_flag[rl];
			m_layer_constraint_set2_flag[i] = m_layer_constraint_set2_flag[rl];
			m_layer_constraint_set3_flag[i] = m_layer_constraint_set3_flag[rl];
			m_layer_level_idc[i] = m_layer_level_idc[rl];
		}

	/*	if( m_decoding_dependency_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_simple_priority_id[i],													6,		"" ) );	
			RNOK	( pcReadIf->getFlag( m_discardable_flag[i],													"" ) );     
			RNOK	( pcReadIf->getCode( m_temporal_level[i],													3,		""	) );
			RNOK	( pcReadIf->getCode( m_dependency_id[i],												3,		""	) );
			RNOK	( pcReadIf->getCode( m_quality_level[i],													2,		""	) );
		}
JVT-S036  */
		if( m_bitrate_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_avg_bitrate[i],														16,		""	) );
	//JVT-S036  start
			RNOK	( pcReadIf->getCode( m_max_bitrate_layer[i],														16,		""	) );
			RNOK	( pcReadIf->getCode( m_max_bitrate_decoded_picture[i],														16,		""	) );
			RNOK	( pcReadIf->getCode( m_max_bitrate_calc_window[i],											16,		""	) );
	//JVT-S036  end
		}

		if( m_frm_rate_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_constant_frm_rate_idc[i],									2,		""	) );
			RNOK	( pcReadIf->getCode( m_avg_frm_rate[i],														16,		""	) );
		}
		else
		{//JVT-S036 
			RNOK	( pcReadIf->getUvlc( m_frm_rate_info_src_layer_id_delta[i],							"" ) );
			rl = m_layer_id[i] - m_frm_rate_info_src_layer_id_delta[i];
			m_constant_frm_rate_idc[i] = m_constant_frm_rate_idc[rl];
			m_avg_frm_rate[i] = m_avg_frm_rate[rl];
		}

		if( m_frm_size_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getUvlc( m_frm_width_in_mbs_minus1[i],											""	) );
			RNOK	( pcReadIf->getUvlc( m_frm_height_in_mbs_minus1[i],											""	) );
		}
		else
		{//JVT-S036  
			RNOK	( pcReadIf->getUvlc( m_frm_size_info_src_layer_id_delta[i],							""  ) );
			rl = m_layer_id[i] - m_frm_size_info_src_layer_id_delta[i];
			m_frm_width_in_mbs_minus1[i] = m_frm_width_in_mbs_minus1[rl];
			m_frm_width_in_mbs_minus1[i] = m_frm_width_in_mbs_minus1[rl];
		}

		if( m_sub_region_layer_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_base_region_layer_id[i],										8,		""	) );
			RNOK	( pcReadIf->getFlag( m_dynamic_rect_flag[i],														""	) );
			if( m_dynamic_rect_flag[i] )
			{
				RNOK( pcReadIf->getCode( m_horizontal_offset[i],											16,		""	) );
				RNOK( pcReadIf->getCode( m_vertical_offset[i],												16,		""	) );
				RNOK( pcReadIf->getCode( m_region_width[i],														16,		""	) );
				RNOK( pcReadIf->getCode( m_region_height[i],													16,		""	) );
			}
		}
		else
		{//JVT-S036  
			RNOK	( pcReadIf->getUvlc( m_sub_region_info_src_layer_id_delta[i],						""  ) );
			rl = m_layer_id[i] - m_sub_region_info_src_layer_id_delta[i];
			m_base_region_layer_id[i] = m_base_region_layer_id[rl];
			m_dynamic_rect_flag[i] = m_dynamic_rect_flag[rl];
			if( m_dynamic_rect_flag[i] )
			{
				 m_horizontal_offset[i] = m_horizontal_offset[rl];
				 m_vertical_offset[i] = m_vertical_offset[rl];
				 m_region_width[i] = m_region_width[rl];
				 m_region_height[i] = m_region_height[rl];
			}

		}

	//JVT-S036  start
		if( m_sub_pic_layer_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_roi_id[i],		3,								""					) );
		}
		if ( m_iroi_slice_division_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_iroi_slice_division_type[i],		2,		"ScalableSEI:iroi_slice_division_type" ) );
			if( m_iroi_slice_division_type[i] == 0 )
			{
				RNOK	( pcReadIf->getUvlc( m_grid_slice_width_in_mbs_minus1[i],    "ScalableSEI:grid_slice_width_in_mbs_minus1" ) );
				RNOK	( pcReadIf->getUvlc( m_grid_slice_height_in_mbs_minus1[i],    "ScalableSEI:grid_slice_height_in_mbs_minus1" ) );
			}
			else if( m_iroi_slice_division_type[i] == 1 )
			{
				RNOK	( pcReadIf->getUvlc( m_num_slice_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
    		// JVT-S054 (ADD) ->
				if ( m_first_mb_in_slice[i] != NULL )
					free( m_first_mb_in_slice[i] );
				m_first_mb_in_slice[i] = (UInt*)malloc( m_num_slice_minus1[i]*sizeof(UInt) );
				if ( m_slice_width_in_mbs_minus1[i] != NULL )
					free( m_slice_width_in_mbs_minus1[i] );
				m_slice_width_in_mbs_minus1[i] = (UInt*)malloc( m_num_slice_minus1[i]*sizeof(UInt) );
				if ( m_slice_height_in_mbs_minus1[i] != NULL )
					free( m_slice_height_in_mbs_minus1[i] );
				m_slice_height_in_mbs_minus1[i] = (UInt*)malloc( m_num_slice_minus1[i]*sizeof(UInt) );
    		// JVT-S054 (ADD) <-
				for ( j = 0; j <= m_num_slice_minus1[i]; j++ )
				{
					RNOK	( pcReadIf->getUvlc( m_first_mb_in_slice[i][j],				"ScalableSEI: first_mb_in_slice" ) );
					RNOK	( pcReadIf->getUvlc( m_slice_width_in_mbs_minus1[i][j],		"ScalableSEI:slice_width_in_mbs_minus1" ) );
					RNOK	( pcReadIf->getUvlc( m_slice_height_in_mbs_minus1[i][j],		"ScalableSEI:slice_height_in_mbs_minus1" ) );
				}
			}
			else if ( m_iroi_slice_division_type[i] == 2 )
			{
    		// JVT-S054 (REPLACE) ->
				RNOK	( pcReadIf->getUvlc( m_num_slice_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
        /*
				UInt uiFrameHeightInMb = m_slice_height_in_mbs_minus1[i][j] + 1;
				UInt uiFrameWidthInMb  = m_slice_width_in_mbs_minus1[i][j] + 1;
				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
				for ( j = 0; j < uiPicSizeInMbs; j++ )
				{
					RNOK	( pcReadIf->getUvlc( m_slice_id[i][j],		"ScalableSEI:slice_id"		   ) );
				}
        */
				UInt uiFrameHeightInMb = m_slice_height_in_mbs_minus1[i][j] + 1;
				UInt uiFrameWidthInMb  = m_slice_width_in_mbs_minus1[i][j] + 1;
				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
			  UInt uiReadBits = (UInt)ceil( log( (double) (m_num_slice_minus1[i] + 1) ) / log(2.) );
				if (uiReadBits == 0)
					uiReadBits = 1;
				if ( m_slice_id[i] != NULL )
					free( m_slice_id[i] );
				m_slice_id[i] = (UInt*)malloc( uiPicSizeInMbs*sizeof(UInt) );
				for ( j = 0; j < uiPicSizeInMbs; j++ )
				{
					RNOK	( pcReadIf->getCode( m_slice_id[i][j],	uiReadBits,		""	) );
				}
    		// JVT-S054 (REPLACE) <-
			}
		}
   //JVT-S036  end

		if( m_layer_dependency_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getUvlc( m_num_directly_dependent_layers[i],								""	) );
// BUG_FIX liuhui{
			for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
			{
				RNOK  ( pcReadIf->getUvlc( m_directly_dependent_layer_id_delta_minus1[i][j],				""  ) );//JVT-S036 
			}
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 
			RNOK	( pcReadIf->getUvlc( m_layer_dependency_info_src_layer_id_delta[i],				""  ) );
			rl = m_layer_id[i] - m_layer_dependency_info_src_layer_id_delta[i];
			m_num_directly_dependent_layers[i] = m_num_directly_dependent_layers[rl];
			for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
			{
				m_directly_dependent_layer_id_delta_minus1[i][j] = m_directly_dependent_layer_id_delta_minus1[rl][j];
			}
		}

		if( m_init_parameter_sets_info_present_flag[i] )
		{
// BUG_FIX liuhui{
			RNOK    ( pcReadIf->getUvlc( m_num_init_seq_parameter_set_minus1[i],                        ""  ) );
			for( j = 0; j <= m_num_init_seq_parameter_set_minus1[i]; j++ )
			{
			    RNOK	( pcReadIf->getUvlc( m_init_seq_parameter_set_id_delta[i][j],					""	) );
			}
			RNOK	( pcReadIf->getUvlc( m_num_init_pic_parameter_set_minus1[i],						""	) );
			for( j = 0; j <= m_num_init_pic_parameter_set_minus1[i]; j++ )
			{
				RNOK	( pcReadIf->getUvlc( m_init_pic_parameter_set_id_delta[i][j],					""	) );
			}
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 
			RNOK	( pcReadIf->getUvlc( m_init_parameter_sets_info_src_layer_id_delta[i],		"" ) );
			rl = m_layer_id[i] - m_init_parameter_sets_info_src_layer_id_delta[i];
			m_num_init_seq_parameter_set_minus1[i] = m_num_init_seq_parameter_set_minus1[rl];
			for( j = 0; j <= m_num_init_seq_parameter_set_minus1[i]; j++ )
			{
			    m_init_seq_parameter_set_id_delta[i][j] = m_init_seq_parameter_set_id_delta[rl][j];
			}
			m_num_init_pic_parameter_set_minus1[i] = m_num_init_pic_parameter_set_minus1[rl];
			for( j = 0; j <= m_num_init_pic_parameter_set_minus1[i]; j++ )
			{
				m_init_pic_parameter_set_id_delta[i][j] = m_init_pic_parameter_set_id_delta[rl][j];
			}
		}
	}	

	return Err::m_nOK;
}

//////////////////////////////////////////////////////////////////////////
//
//			SUB-PICTURE SCALABLE LAYER SEI
//
//////////////////////////////////////////////////////////////////////////

SEI::SubPicSei::SubPicSei ()
: SEIMessage		( SUB_PIC_SEI ),
m_uiLayerId			( 0 )
{
}

SEI::SubPicSei::~SubPicSei ()
{
}

ErrVal
SEI::SubPicSei::create( SubPicSei*& rpcSeiMessage)
{
	rpcSeiMessage = new SubPicSei();
	ROT( NULL == rpcSeiMessage );
	return Err::m_nOK;
}

ErrVal
SEI::SubPicSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
	RNOK	( pcWriteIf->writeUvlc( m_uiLayerId, "Sub-picture scalable SEI: m_uiLayerId" ) );
	return Err::m_nOK;
}

ErrVal
SEI::SubPicSei::read( HeaderSymbolReadIf *pcReadIf )
{
	RNOK	( pcReadIf->getUvlc( m_uiLayerId, "Sub-picture scalable SEI: m_uiLayerd" ) );
	return Err::m_nOK;
}



//////////////////////////////////////////////////////////////////////////
// 
//      MOTION     S E I  FOR  ROI
//
//////////////////////////////////////////////////////////////////////////

SEI::MotionSEI::MotionSEI     ()
 : SEIMessage                     ( MOTION_SEI ),
 m_num_slice_groups_in_set_minus1(0),
 m_exact_sample_value_match_flag(true),
 m_pan_scan_rect_flag(false)
{
}

SEI::MotionSEI::~MotionSEI()
{
}

ErrVal
SEI::MotionSEI::create( MotionSEI*& rpcSeiMessage )
{
  rpcSeiMessage = new MotionSEI();
  ROT( NULL == rpcSeiMessage )
  return Err::m_nOK;
}

ErrVal
SEI::MotionSEI::write( HeaderSymbolWriteIf* pcWriteIf )
{

  RNOK  ( pcWriteIf->writeUvlc( m_num_slice_groups_in_set_minus1,               "Motion Constrainted SEI: Num_slice_groups_in_set_minus1"   ) );

  for(UInt i = 0; i <= m_num_slice_groups_in_set_minus1; i++)
  {    
    RNOK  ( pcWriteIf->writeUvlc( m_slice_group_id[ i ],               "Motion Constrainted SEI: slice_group_id[ i ]"   ) );
  }
  
    
  RNOK  ( pcWriteIf->writeFlag(m_exact_sample_value_match_flag           ,     "Motion Constrainted SEI: exact_sample_value_match_flag"            ) );
  RNOK  ( pcWriteIf->writeFlag(m_pan_scan_rect_flag                      ,     "Motion Constrainted SEI: frm_rate_info_present_flag"           ) );

  return Err::m_nOK;
}

ErrVal
SEI::MotionSEI::read ( HeaderSymbolReadIf* pcReadIf )
{
  RNOK  ( pcReadIf->getUvlc( m_num_slice_groups_in_set_minus1,               "Motion Constrainted SEI: Num_slice_groups_in_set_minus1"   ) );

  for(UInt i = 0; i <= m_num_slice_groups_in_set_minus1; i++)
  {    
    RNOK  ( pcReadIf->getUvlc( m_slice_group_id[ i ],               "Motion Constrainted SEI: slice_group_id[ i ]"   ) );
  }
    
  RNOK  ( pcReadIf->getFlag(m_exact_sample_value_match_flag           ,     "Motion Constrainted SEI: exact_sample_value_match_flag"            ) );
  RNOK  ( pcReadIf->getFlag(m_pan_scan_rect_flag                      ,     "Motion Constrainted SEI: frm_rate_info_present_flag"           ) );

  assert(m_exact_sample_value_match_flag==true);
  assert(m_pan_scan_rect_flag ==false);

  return Err::m_nOK;
}

ErrVal        
SEI::MotionSEI::setSliceGroupId(UInt id)
{
  m_slice_group_id[0] = id;    
  return Err::m_nOK;
};


//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
//////////////////////////////////////////////////////////////////////////
// 
//      QUALITY LEVEL     S E I
//
//////////////////////////////////////////////////////////////////////////

SEI::QualityLevelSEI::QualityLevelSEI     ()
 : SEIMessage                     ( QUALITYLEVEL_SEI ),
 m_uiNumLevels         ( 0 ),
 m_uiDependencyId      ( 0 )
{
  ::memset( m_auiQualityLevel,  0x00, MAX_NUM_RD_LEVELS*sizeof(UInt) );
  ::memset( m_auiDeltaBytesRateOfLevel, 0x00, MAX_NUM_RD_LEVELS*sizeof(UInt) );
}


SEI::QualityLevelSEI::~QualityLevelSEI()
{
}


ErrVal
SEI::QualityLevelSEI::create( QualityLevelSEI*& rpcSeiMessage )
{
  rpcSeiMessage = new QualityLevelSEI();
  ROT( NULL == rpcSeiMessage )
  return Err::m_nOK;
}


ErrVal
SEI::QualityLevelSEI::write( HeaderSymbolWriteIf* pcWriteIf )
{
  RNOK  ( pcWriteIf->writeCode( m_uiDependencyId, 3,"QualityLevelSEI: DependencyId"   ) );
  RNOK  ( pcWriteIf->writeCode( m_uiNumLevels, 4,"QualityLevelSEI: NumLevels"   ) );
  for(UInt ui = 0; ui < m_uiNumLevels; ui++)
  {
	RNOK  ( pcWriteIf->writeCode( m_auiQualityLevel[ui], 8,"QualityLevelSEI: QualityLevel"   ) );
	RNOK  ( pcWriteIf->writeUvlc( m_auiDeltaBytesRateOfLevel[ui],"QualityLevelSEI: DeDeltaBytesRateOfLevellta"   ) );
  }

  return Err::m_nOK;
}


ErrVal
SEI::QualityLevelSEI::read ( HeaderSymbolReadIf* pcReadIf )
{
  RNOK  ( pcReadIf->getCode( m_uiDependencyId, 3,"QualityLevelSEI: DependencyId"   ) );
  RNOK  ( pcReadIf->getCode( m_uiNumLevels, 4,"QualityLevelSEI: NumLevels"   ) );
  for(UInt ui = 0; ui < m_uiNumLevels; ui++)
  {
	RNOK  ( pcReadIf->getCode( m_auiQualityLevel[ui], 8,"QualityLevelSEI: QualityLevel"   ) );
	RNOK  ( pcReadIf->getUvlc( m_auiDeltaBytesRateOfLevel[ui],"QualityLevelSEI: DeltaBytesRateOfLevel"   ) );
  }
  return Err::m_nOK;
}

//}}Quality level estimation and modified truncation- JVTO044 and m12007

//NonRequired JVT-Q066 (06-04-08){{
SEI::NonRequiredSei::NonRequiredSei	()
: SEIMessage						( NON_REQUIRED_SEI )
, m_uiNumInfoEntriesMinus1			(MSYS_UINT_MAX)
{
	::memset( m_uiEntryDependencyId,			MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*sizeof(UInt) );
	::memset( m_uiNumNonRequiredPicsMinus1,		MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*sizeof(UInt) );
	::memset( m_uiNonRequiredPicDependencyId,	MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
	::memset( m_uiNonRequiredPicQulityLevel,	MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
	::memset( m_uiNonRequiredPicFragmentOrder,  MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
}
/*
SEI::NonRequiredSei::NonRequiredSei	()
: SEIMessage						( NON_REQUIRED_SEI )
, m_uiNumInfoEntriesMinus1			(0)
{
::memset( m_uiEntryDependencyId,			0x00, MAX_NUM_INFO_ENTRIES*sizeof(UInt) );
::memset( m_uiNumNonRequiredPicsMinus1,		0x00, MAX_NUM_INFO_ENTRIES*sizeof(UInt) );
::memset( m_uiNonRequiredPicDependencyId,	0x00, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
::memset( m_uiNonRequiredPicQulityLevel,	0x00, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
::memset( m_uiNonRequiredPicFragmentOrder,  0x00, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
}*/
//NonRequired JVT-Q066 (06-04-08)}}

SEI::NonRequiredSei::~NonRequiredSei ()
{
}

ErrVal
SEI::NonRequiredSei::create ( NonRequiredSei*& rpcSeiMessage )
{
	rpcSeiMessage = new NonRequiredSei();
	ROT( NULL == rpcSeiMessage)
		return Err::m_nOK;
}

ErrVal
SEI::NonRequiredSei::destroy() 
{
	delete this;
	return Err::m_nOK;
}

ErrVal
SEI::NonRequiredSei::write( HeaderSymbolWriteIf* pcWriteIf )
{
	RNOK	(pcWriteIf->writeUvlc( m_uiNumInfoEntriesMinus1,	"NonRequiredSEI: NumInfoEntriesMinus1"	));
	for( UInt uiLayer = 0; uiLayer <= m_uiNumInfoEntriesMinus1; uiLayer++)
	{
		RNOK(pcWriteIf->writeCode( m_uiEntryDependencyId[uiLayer],	3,	"NonRequiredSEI: EntryDependencyId"		));
		RNOK(pcWriteIf->writeUvlc( m_uiNumNonRequiredPicsMinus1[uiLayer],	"NonRequiredSEI: NumNonRequiredPicsMinus1"	));
		for( UInt NonRequiredLayer = 0; NonRequiredLayer <= m_uiNumNonRequiredPicsMinus1[uiLayer]; NonRequiredLayer++)
		{
			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicDependencyId[uiLayer][NonRequiredLayer],	3,	"NonRequiredSEI: NonRequiredPicDependencyId"));
			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicQulityLevel[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredPicQulityLevel"	));
			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicFragmentOrder[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredFragmentOrder"	));
		}
	}
	return Err::m_nOK;
}

ErrVal
SEI::NonRequiredSei::read( HeaderSymbolReadIf* pcReadIf )
{
	RNOK	(pcReadIf->getUvlc( m_uiNumInfoEntriesMinus1,	"NonRequiredSEI: NumInfoEntriesMinus1"	));
	for( UInt uiLayer = 0; uiLayer <= m_uiNumInfoEntriesMinus1; uiLayer++)
	{
		RNOK(pcReadIf->getCode( m_uiEntryDependencyId[uiLayer],	3,	"NonRequiredSEI: EntryDependencyId"		));
		RNOK(pcReadIf->getUvlc( m_uiNumNonRequiredPicsMinus1[uiLayer],	"NonRequiredSEI: NumNonRequiredPicsMinus1"	));
		for( UInt NonRequiredLayer = 0; NonRequiredLayer <= m_uiNumNonRequiredPicsMinus1[uiLayer]; NonRequiredLayer++)
		{
			RNOK(pcReadIf->getCode( m_uiNonRequiredPicDependencyId[uiLayer][NonRequiredLayer],	3,	"NonRequiredSEI: NonRequiredPicDependencyId"));
			RNOK(pcReadIf->getCode( m_uiNonRequiredPicQulityLevel[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredPicQulityLevel"	));
			RNOK(pcReadIf->getCode( m_uiNonRequiredPicFragmentOrder[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredFragmentOrder"	));
		}
	}
	return Err::m_nOK;
}
// JVT-S080 LMI {
//////////////////////////////////////////////////////////////////////////
//
//			SCALABLE SEI LAYERS NOT PRESENT
//
//////////////////////////////////////////////////////////////////////////

UInt SEI::ScalableSeiLayersNotPresent::m_uiLeftNumLayers = 0;
UInt SEI::ScalableSeiLayersNotPresent::m_auiLeftLayerId[MAX_SCALABLE_LAYERS];

SEI::ScalableSeiLayersNotPresent::ScalableSeiLayersNotPresent (): SEIMessage		( SCALABLE_SEI_LAYERS_NOT_PRESENT )
{
}

SEI::ScalableSeiLayersNotPresent::~ScalableSeiLayersNotPresent ()
{
}

ErrVal
SEI::ScalableSeiLayersNotPresent::create( ScalableSeiLayersNotPresent*& rpcSeiMessage )
{
	rpcSeiMessage = new ScalableSeiLayersNotPresent();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}

ErrVal
SEI::ScalableSeiLayersNotPresent::write( HeaderSymbolWriteIf *pcWriteIf )
{
  UInt i;

	RNOK ( pcWriteIf->writeUvlc(m_uiNumLayers,													"ScalableSEILayersNotPresent: num_layers"											) );
	for( i = 0; i < m_uiNumLayers; i++ )
	{
		RNOK ( pcWriteIf->writeCode( m_auiLayerId[i],												8,		"ScalableSEILayersNotPresent: layer_id"															) );
	}
	return Err::m_nOK;
}

ErrVal
SEI::ScalableSeiLayersNotPresent::read ( HeaderSymbolReadIf *pcReadIf )
{
	UInt i;
	RNOK ( pcReadIf->getUvlc( m_uiNumLayers ,																"ScalableSEILayersNotPresent: num_layers"	) );
	for ( i = 0; i < m_uiNumLayers; i++ )
	{
		RNOK ( pcReadIf->getCode( m_auiLayerId[i],																	8,			"ScalableSEILayersNotPresent: layer_id"	) );
	}
	return Err::m_nOK;
}

//////////////////////////////////////////////////////////////////////////
//
//			SCALABLE SEI DEPENDENCY CHANGE
//
//////////////////////////////////////////////////////////////////////////

SEI::ScalableSeiDependencyChange::ScalableSeiDependencyChange (): SEIMessage		( SCALABLE_SEI_DEPENDENCY_CHANGE )
{
}

SEI::ScalableSeiDependencyChange::~ScalableSeiDependencyChange ()
{
}

ErrVal
SEI::ScalableSeiDependencyChange::create( ScalableSeiDependencyChange*& rpcSeiMessage )
{
	rpcSeiMessage = new ScalableSeiDependencyChange();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}

ErrVal
SEI::ScalableSeiDependencyChange::write( HeaderSymbolWriteIf *pcWriteIf )
{
  UInt i, j;

	ROF( m_uiNumLayersMinus1+1 );
	RNOK		( pcWriteIf->writeUvlc(m_uiNumLayersMinus1,													"ScalableSeiDependencyChange: num_layers_minus1"											) );
	for( i = 0; i <= m_uiNumLayersMinus1; i++ )
	{
		RNOK	( pcWriteIf->writeCode( m_auiLayerId[i],												8,		"ScalableSeiDependencyChange: layer_id"															) );
		RNOK	( pcWriteIf->writeFlag( m_abLayerDependencyInfoPresentFlag[i],		"ScalableSeiDependencyChange: layer_dependency_info_present_flag"															) );		
		if( m_abLayerDependencyInfoPresentFlag[i] )
		{
	       RNOK		( pcWriteIf->writeUvlc(m_auiNumDirectDependentLayers[i],													"ScalableSeiDependencyChange: num_directly_dependent_layers"											) );
	       for ( j = 0; j < m_auiNumDirectDependentLayers[i]; j++)
	            RNOK( pcWriteIf->writeUvlc(m_auiDirectDependentLayerIdDeltaMinus1[i][j],													"ScalableSeiDependencyChange: directly_dependent_layer_id_delta_minus1"											) );
		}
		else
	            RNOK	( pcWriteIf->writeUvlc(m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[i],													"ScalableSeiDependencyChange: layer_dependency_info_src_layer_id_delta_minus1"											) );
	}
	return Err::m_nOK;
}

ErrVal
SEI::ScalableSeiDependencyChange::read ( HeaderSymbolReadIf *pcReadIf )
{
  UInt i, j;

	RNOK		( pcReadIf->getUvlc(m_uiNumLayersMinus1,													"ScalableSeiDependencyChange: num_layers_minus1"											) );
	for( i = 0; i <= m_uiNumLayersMinus1; i++ )
	{
		RNOK	( pcReadIf->getCode( m_auiLayerId[i],												8,		"ScalableSeiDependencyChange: layer_id"															) );
		RNOK	( pcReadIf->getFlag( m_abLayerDependencyInfoPresentFlag[i],		"ScalableSeiDependencyChange: layer_dependency_info_present_flag"															) );		
		if( m_abLayerDependencyInfoPresentFlag[i] )
		{
	       RNOK		( pcReadIf->getUvlc(m_auiNumDirectDependentLayers[i],													"ScalableSeiDependencyChange: num_directly_dependent_layers"											) );
	       for ( j = 0; j < m_auiNumDirectDependentLayers[i]; j++)
	            RNOK		( pcReadIf->getUvlc(m_auiDirectDependentLayerIdDeltaMinus1[i][j],													"ScalableSeiDependencyChange: directly_dependent_layer_id_delta_minus1"											) );
		}
		else
	            RNOK		( pcReadIf->getUvlc(m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[i],													"ScalableSeiDependencyChange: layer_dependency_info_src_layer_id_delta_minus1"											) );
	}
	return Err::m_nOK;
}
// JVT-S080 LMI }

//JVT-W080
//////////////////////////////////////////////////////////////////////////
// 
//      PARALLEL DECODING SEI
//
//////////////////////////////////////////////////////////////////////////
SEI::ParallelDecodingSEI::ParallelDecodingSEI()
: SEIMessage                   ( PARALLEL_DEC_SEI )
, m_uiSPSId                    ( 0 )
, m_uiNumView                  ( 0 )
, m_puiNumRefAnchorFramesL0    ( 0 )
, m_puiNumRefAnchorFramesL1    ( 0 )
, m_puiNumNonRefAnchorFramesL0 ( 0 )
, m_puiNumNonRefAnchorFramesL1 ( 0 )
, m_ppuiPDIInitDelayMinus2L0Anc( 0 )
, m_ppuiPDIInitDelayMinus2L1Anc( 0 )
, m_ppuiPDIInitDelayMinus2L0NonAnc( 0 )
, m_ppuiPDIInitDelayMinus2L1NonAnc( 0 )
{}

SEI::ParallelDecodingSEI::~ParallelDecodingSEI()
{
	if( m_uiNumView ) //has been created
	{
		for( UInt uiNumView = 0; uiNumView < m_uiNumView; uiNumView++ )
		{
			if( m_puiNumRefAnchorFramesL0[uiNumView] )
			{
				delete[] m_ppuiPDIInitDelayMinus2L0Anc[uiNumView];
				m_ppuiPDIInitDelayMinus2L0Anc[uiNumView] = NULL;
			}
			if( m_puiNumRefAnchorFramesL1[uiNumView] )
			{
				delete[] m_ppuiPDIInitDelayMinus2L1Anc[uiNumView];
				m_ppuiPDIInitDelayMinus2L1Anc[uiNumView] = NULL;
			}	
			if( m_puiNumNonRefAnchorFramesL0[uiNumView] )
			{
				delete[] m_ppuiPDIInitDelayMinus2L0NonAnc[uiNumView];
				m_ppuiPDIInitDelayMinus2L0NonAnc[uiNumView] = NULL;
			}
			if( m_puiNumNonRefAnchorFramesL1[uiNumView] )
			{
				delete[] m_ppuiPDIInitDelayMinus2L1NonAnc[uiNumView];
				m_ppuiPDIInitDelayMinus2L1NonAnc[uiNumView] = NULL;
			}
		}
		delete[] m_ppuiPDIInitDelayMinus2L0Anc;
		delete[] m_ppuiPDIInitDelayMinus2L1Anc;
		delete[] m_ppuiPDIInitDelayMinus2L0NonAnc;
		delete[] m_ppuiPDIInitDelayMinus2L1NonAnc;

		m_ppuiPDIInitDelayMinus2L0Anc = NULL;
		m_ppuiPDIInitDelayMinus2L1Anc = NULL;
		m_ppuiPDIInitDelayMinus2L0NonAnc = NULL;
		m_ppuiPDIInitDelayMinus2L1NonAnc = NULL;

		delete[] m_puiNumRefAnchorFramesL0;
		delete[] m_puiNumRefAnchorFramesL1;
		delete[] m_puiNumNonRefAnchorFramesL0;
		delete[] m_puiNumNonRefAnchorFramesL1;

		m_puiNumRefAnchorFramesL0 = NULL;
		m_puiNumRefAnchorFramesL1 = NULL;
		m_puiNumNonRefAnchorFramesL0 = NULL;
		m_puiNumNonRefAnchorFramesL1 = NULL;
		m_uiNumView = 0;
	}
}

ErrVal
SEI::ParallelDecodingSEI::create( ParallelDecodingSEI*& rpcSeiMessage )
{
	rpcSeiMessage = new ParallelDecodingSEI();
	ROT( NULL == rpcSeiMessage )
	return Err::m_nOK;
}

ErrVal       
SEI::ParallelDecodingSEI::init( UInt uiSPSId
															, UInt uiNumView
													    , UInt* num_refs_list0_anc
													    , UInt* num_refs_list1_anc
													    , UInt* num_refs_list0_nonanc
													    , UInt* num_refs_list1_nonanc
														  , UInt  PDSInitialDelayAnc
														  , UInt  PDSInitialDelayNonAnc
														  )
{
	m_uiSPSId = uiSPSId;
  m_uiNumView = uiNumView;
	m_puiNumRefAnchorFramesL0 = new UInt[uiNumView];
	m_puiNumRefAnchorFramesL1 = new UInt[uiNumView];
  m_puiNumNonRefAnchorFramesL0 = new UInt[uiNumView];
	m_puiNumNonRefAnchorFramesL1 = new UInt[uiNumView];
	m_ppuiPDIInitDelayMinus2L0Anc = new UInt*[uiNumView];
	m_ppuiPDIInitDelayMinus2L1Anc = new UInt*[uiNumView];
  m_ppuiPDIInitDelayMinus2L0NonAnc = new UInt*[uiNumView];
  m_ppuiPDIInitDelayMinus2L1NonAnc = new UInt*[uiNumView];

	for( UInt i = 0; i < uiNumView; i++ )
	{
    m_puiNumRefAnchorFramesL0[i] = num_refs_list0_anc[i];
    m_puiNumRefAnchorFramesL1[i] = num_refs_list1_anc[i];
		m_puiNumNonRefAnchorFramesL0[i] = num_refs_list0_nonanc[i];
		m_puiNumNonRefAnchorFramesL1[i] = num_refs_list1_nonanc[i];
		if( num_refs_list0_anc[i] )
		{
      m_ppuiPDIInitDelayMinus2L0Anc[i] = new UInt [num_refs_list0_anc[i]];
			for( UInt j = 0; j < num_refs_list0_anc[i]; j++ )
				m_ppuiPDIInitDelayMinus2L0Anc[i][j] = PDSInitialDelayAnc-2;
		}
		if( num_refs_list1_anc[i] )
		{
      m_ppuiPDIInitDelayMinus2L1Anc[i] = new UInt [num_refs_list1_anc[i]];
      for( UInt j = 0; j < num_refs_list1_anc[i]; j++ )
				m_ppuiPDIInitDelayMinus2L1Anc[i][j] = PDSInitialDelayNonAnc-2;
		}	
		if( num_refs_list0_nonanc[i] )
		{
      m_ppuiPDIInitDelayMinus2L0NonAnc[i] = new UInt [num_refs_list0_nonanc[i]];
			for( UInt j = 0; j < num_refs_list0_anc[i]; j++ )
				m_ppuiPDIInitDelayMinus2L0NonAnc[i][j] = PDSInitialDelayNonAnc-2;
		}
		if( num_refs_list1_nonanc[i] )
		{
      m_ppuiPDIInitDelayMinus2L1NonAnc[i] = new UInt [num_refs_list1_nonanc[i]];
			for( UInt j = 0; j < num_refs_list1_anc[i]; j++ )
				m_ppuiPDIInitDelayMinus2L1NonAnc[i][j] = PDSInitialDelayNonAnc-2;
		}
	}

	return Err::m_nOK;
}


ErrVal
SEI::ParallelDecodingSEI::write( HeaderSymbolWriteIf *pcWriteIf )
{
	UInt uiNumViewMinus1 = m_uiNumView-1;
	UInt j;
	pcWriteIf->writeUvlc( m_uiSPSId, "SEI:PDSEI: PdsSeqParameterSetId");
	for( UInt uiNumView = 0; uiNumView <= uiNumViewMinus1; uiNumView++ )
	{
		for( j = 1; j < m_puiNumRefAnchorFramesL0[uiNumView]; j++ )//SEI JJ
	  {
			pcWriteIf->writeUvlc( m_ppuiPDIInitDelayMinus2L0Anc[uiNumView][j], "SEI:PDSEI: PdsInitialDelayMinus2L0Anc");
		}
		for( j = 0; j < m_puiNumRefAnchorFramesL1[uiNumView]; j++ )
		{
			pcWriteIf->writeUvlc( m_ppuiPDIInitDelayMinus2L1Anc[uiNumView][j], "SEI:PDSEI: PdsInitialDelayMinus2L1Anc");
		}
		for( j = 0; j < m_puiNumNonRefAnchorFramesL0[uiNumView]; j++ )
		{
			pcWriteIf->writeUvlc( m_ppuiPDIInitDelayMinus2L0NonAnc[uiNumView][j], "SEI:PDSEI: PdsInitialDelayMinus2L0NonAnc");
		}
		for( j = 0; j < m_puiNumNonRefAnchorFramesL1[uiNumView]; j++ )
		{
			pcWriteIf->writeUvlc( m_ppuiPDIInitDelayMinus2L1NonAnc[uiNumView][j], "SEI:PDSEI: PdsInitialDelayMinus2L1NonAnc");
		}   		  
	}
	return Err::m_nOK;
}

ErrVal
SEI::ParallelDecodingSEI::read ( HeaderSymbolReadIf *pcReadIf )
{
/* unnecessary for Parallel SEI to be parsed, omitted here
*/
	return Err::m_nOK;
}
//~JVT-W080


//JVT-AB025 {{
////////////////////////////////////////////////////////////////////////////////
//
//  Nonrequired View Component  Info //JVT-AB025 
//
///////////////////////////////////////////////////////////////////////////////
SEI::NonReqViewInfoSei::NonReqViewInfoSei()
:SEIMessage(NON_REQ_VIEW_INFO_SEI)
,m_uiNumOfTargetViewMinus1( 0 )
,m_puiViewOrderIndex( 0 )
,m_puiNumNonReqViewCopMinus1( 0 )
,m_ppuiNonReqViewOrderIndex(0)
,m_ppuiIndexDeltaMinus1( 0 )
{}
SEI::NonReqViewInfoSei::~NonReqViewInfoSei()
{
  if( m_uiNumOfTargetViewMinus1 >= 0)
  {
    for (UInt i=0; i<= m_uiNumOfTargetViewMinus1; i++)
    {
      delete []m_ppuiNonReqViewOrderIndex[i];
      m_ppuiNonReqViewOrderIndex[i] = NULL;
      delete []m_ppuiIndexDeltaMinus1[i];
      m_ppuiIndexDeltaMinus1[i] = NULL;
    }
    delete []m_ppuiIndexDeltaMinus1;
    delete []m_ppuiNonReqViewOrderIndex;
    delete []m_puiNumNonReqViewCopMinus1;
    delete []m_puiViewOrderIndex;

    m_ppuiIndexDeltaMinus1 = NULL;
    m_ppuiNonReqViewOrderIndex = NULL;
    m_puiNumNonReqViewCopMinus1 = NULL;
    m_puiViewOrderIndex = NULL;
    m_uiNumOfTargetViewMinus1 = 0;
  }
}
ErrVal
SEI::NonReqViewInfoSei::create(NonReqViewInfoSei*& rpcSeiMessage)
{
  rpcSeiMessage = new NonReqViewInfoSei();
  ROT( NULL == rpcSeiMessage);
  return Err::m_nOK;
}
ErrVal
SEI::NonReqViewInfoSei::init( UInt uiNumOfTargetViewMinus1, UInt* puiViewOrderIndex)
{
  UInt NumOfTargetView = uiNumOfTargetViewMinus1 + 1;
  m_uiNumOfTargetViewMinus1 = uiNumOfTargetViewMinus1;
  m_puiViewOrderIndex = new UInt[NumOfTargetView];
  m_puiNumNonReqViewCopMinus1 = new UInt[NumOfTargetView];
  m_ppuiNonReqViewOrderIndex = new UInt*[NumOfTargetView];
  m_ppuiIndexDeltaMinus1 = new UInt*[NumOfTargetView];
  for (UInt i = 0; i<= uiNumOfTargetViewMinus1; i++)
  {
    m_puiViewOrderIndex[i] = puiViewOrderIndex[2*i+1];  // designed for View Order 0-2-1-4-3-6-5..
    m_puiNumNonReqViewCopMinus1[i] = m_puiViewOrderIndex[i]-1;
    m_ppuiNonReqViewOrderIndex[i] = new UInt[m_puiNumNonReqViewCopMinus1[i] + 1];
    m_ppuiIndexDeltaMinus1[i] = new UInt[m_puiNumNonReqViewCopMinus1[i] + 1];
    for (UInt j = 0; j<= m_puiNumNonReqViewCopMinus1[i]; j++)
    {
      m_ppuiNonReqViewOrderIndex[i][j] = j;
      m_ppuiIndexDeltaMinus1[i][j] = m_puiViewOrderIndex[i] - m_ppuiNonReqViewOrderIndex[i][j] - 1;
    }
  }
  return Err::m_nOK;
}
ErrVal
SEI::NonReqViewInfoSei::write( HeaderSymbolWriteIf* pcWriteIf )
{
  pcWriteIf->writeUvlc( m_uiNumOfTargetViewMinus1, "NonReqViewInfoSei:uiNumOfTargetViewMinus1");
  for( UInt i = 0; i<= m_uiNumOfTargetViewMinus1; i++)
  {
    pcWriteIf->writeUvlc(m_puiViewOrderIndex[i],"NonReqViewInfoSei:TargetViewOrderIndex");
    pcWriteIf->writeUvlc(m_puiNumNonReqViewCopMinus1[i],"NonReqViewInfoSei:NumNonReqViewComponentMinus1");
    for(UInt j = 0; j<= m_puiNumNonReqViewCopMinus1[i]; j++)
    {
      pcWriteIf->writeUvlc(m_ppuiIndexDeltaMinus1[i][j],"NonReqViewInfoSei:DeltaViewOrderIndexMinus1");
    }
  }
  return Err::m_nOK;
}
ErrVal
SEI::NonReqViewInfoSei::read( HeaderSymbolReadIf* pcReadIf)
{
  pcReadIf->getUvlc( m_uiNumOfTargetViewMinus1, "NonReqViewInfoSei:uiNumOfTargetViewMinus1");
  UInt NumOfTargetView = m_uiNumOfTargetViewMinus1 + 1;
  m_puiViewOrderIndex = new UInt[NumOfTargetView];
  m_puiNumNonReqViewCopMinus1 = new UInt[NumOfTargetView];
  m_ppuiNonReqViewOrderIndex = new UInt*[NumOfTargetView];
  m_ppuiIndexDeltaMinus1 = new UInt*[NumOfTargetView];
  for( UInt i = 0; i< NumOfTargetView; i++)
  {
    pcReadIf->getUvlc(m_puiViewOrderIndex[i],"NonReqViewInfoSei:TargetViewOrderIndex");
    pcReadIf->getUvlc(m_puiNumNonReqViewCopMinus1[i],"NonReqViewInfoSei:NumNonReqViewComponentMinus1");
    m_ppuiIndexDeltaMinus1[i] = new UInt[m_puiNumNonReqViewCopMinus1[i]+1];
    m_ppuiNonReqViewOrderIndex[i] = new UInt[m_puiNumNonReqViewCopMinus1[i] + 1];
    for(UInt j = 0; j<= m_puiNumNonReqViewCopMinus1[i]; j++)
    {
      pcReadIf->getUvlc(m_ppuiIndexDeltaMinus1[i][j],"NonReqViewInfoSei:DeltaViewOrderIndexMinus1");
      m_ppuiNonReqViewOrderIndex[i][j] = m_puiViewOrderIndex[i] - m_ppuiIndexDeltaMinus1[i][j];
    }
  }
  return Err::m_nOK;
}
////////////////////////////////////////////////////////////////////////////////
//
//  View Dependency Structure 
//
///////////////////////////////////////////////////////////////////////////////
SEI::ViewDependencyStructureSei::ViewDependencyStructureSei()
:SEIMessage(VIEW_DEPENDENCY_STRUCTURE_SEI)
,m_uiNumOfViews(0)
,m_puiNumAnchorL0Refs(NULL)
,m_puiNumAnchorL1Refs(NULL)
,m_puiNumNonAnchorL0Refs(NULL)
,m_puiNumNonAnchorL1Refs(NULL)
,m_bAnchorUpdateFlag(false)
,m_bNonAnchorUpdateFlag(false)
,m_uiSeqParameterSetId(0)  //SEI JJ
,m_ppbAnchorRefL0Flag(NULL)
,m_ppbAnchorRefL1Flag(NULL)
,m_ppbNonAnchorRefL0Flag(NULL)
,m_ppbNonAnchorRefL1Flag(NULL)
{}
SEI::ViewDependencyStructureSei::~ViewDependencyStructureSei()
{
  if (m_bAnchorUpdateFlag)
  {
    for( UInt i = 0; i<m_uiNumOfViews; i++)
    {
      if (m_puiNumAnchorL0Refs[i])
      {
        delete []m_ppbAnchorRefL0Flag[i];
        m_ppbAnchorRefL0Flag[i] = NULL;
      }
      if (m_puiNumAnchorL1Refs[i])
      {
        delete []m_ppbAnchorRefL0Flag[i];
        m_ppbAnchorRefL0Flag[i] = NULL;
      }
    }
    delete []m_puiNumAnchorL0Refs;
    delete []m_ppbAnchorRefL0Flag;
    m_puiNumAnchorL0Refs = NULL;
    m_ppbAnchorRefL0Flag = NULL;
    delete []m_puiNumAnchorL1Refs;
    delete []m_ppbAnchorRefL1Flag;
    m_puiNumAnchorL1Refs = NULL;
    m_ppbAnchorRefL1Flag = NULL;
  }
  if (m_bNonAnchorUpdateFlag)
  {
    for( UInt i = 0; i<m_uiNumOfViews; i++)
    {
      if (m_puiNumNonAnchorL0Refs[i])
      {
        delete []m_ppbNonAnchorRefL0Flag[i];
        m_ppbNonAnchorRefL0Flag[i] = NULL;
      }
      if (m_puiNumNonAnchorL1Refs[i])
      {
        delete []m_ppbNonAnchorRefL0Flag[i];
        m_ppbNonAnchorRefL0Flag[i] = NULL;
      }
    }
    delete []m_ppbNonAnchorRefL0Flag;
    delete []m_puiNumNonAnchorL0Refs;
    m_ppbNonAnchorRefL0Flag = NULL;
    m_puiNumNonAnchorL0Refs = NULL;
    delete []m_ppbNonAnchorRefL1Flag;
    delete []m_puiNumNonAnchorL1Refs;
    m_ppbNonAnchorRefL1Flag = NULL;
    m_puiNumNonAnchorL1Refs = NULL;
  }
}
ErrVal
SEI::ViewDependencyStructureSei::create(ViewDependencyStructureSei*& rpcViewDepStruSei)
{
  rpcViewDepStruSei = new ViewDependencyStructureSei();
  ROT( NULL == rpcViewDepStruSei);
  return Err::m_nOK;
}
ErrVal
SEI::ViewDependencyStructureSei::init( UInt uiNumOfViews 
                                      ,UInt* puinum_refs_list0_anc 
                                      ,UInt* puinum_refs_list1_anc 
                                      ,UInt* puinum_refs_list0_nonanc 
                                      ,UInt* puinum_refs_list1_nonanc
                                      ,Bool  bEnc_Dec_Flag)
{
  m_uiNumOfViews = uiNumOfViews;
  m_puiNumAnchorL0Refs = new UInt[uiNumOfViews];
  m_puiNumAnchorL1Refs = new UInt[uiNumOfViews];
  m_puiNumNonAnchorL1Refs = new UInt[uiNumOfViews];
  m_puiNumNonAnchorL0Refs = new UInt[uiNumOfViews];
  m_ppbAnchorRefL0Flag = new Bool*[uiNumOfViews];
  m_ppbAnchorRefL1Flag = new Bool*[uiNumOfViews];
  m_ppbNonAnchorRefL0Flag = new Bool*[uiNumOfViews];
  m_ppbNonAnchorRefL1Flag = new Bool*[uiNumOfViews];
  if(bEnc_Dec_Flag)
  {
    if (m_bAnchorUpdateFlag)
    {
      for( UInt i = 0; i<uiNumOfViews; i++)
      {
        m_puiNumAnchorL0Refs[i] = puinum_refs_list0_anc[i];
        if (puinum_refs_list0_anc[i])
        {
          m_ppbAnchorRefL0Flag[i] = new Bool[puinum_refs_list0_anc[i]];
          for (UInt j = 0; j< puinum_refs_list0_anc[i]; j++)
          {
            m_ppbAnchorRefL0Flag[i][j] = 0;
          }
        }
        m_puiNumAnchorL1Refs[i] = puinum_refs_list1_anc[i];
        if (puinum_refs_list1_anc[i])
        {
          m_ppbAnchorRefL1Flag[i] = new Bool[puinum_refs_list1_anc[i]];
          for (UInt j = 0; j< puinum_refs_list1_anc[i]; j++)
          {
            m_ppbAnchorRefL0Flag[i][j] = 0;
          }
        }
      }
    }
    if (m_bNonAnchorUpdateFlag)
    {
      for( UInt i = 0; i<uiNumOfViews; i++)
      {
        m_puiNumNonAnchorL0Refs[i] = puinum_refs_list0_nonanc[i];
        if (puinum_refs_list0_nonanc[i])
        {
          m_ppbNonAnchorRefL0Flag[i] = new Bool[puinum_refs_list0_nonanc[i]];
          for (UInt j = 0; j< puinum_refs_list0_nonanc[i]; j++)
          {
            m_ppbNonAnchorRefL0Flag[i][j] = 0;
          }
        }
        m_puiNumNonAnchorL1Refs[i] = puinum_refs_list1_nonanc[i];
        if (puinum_refs_list1_nonanc[i])
        {
          m_ppbNonAnchorRefL1Flag[i] = new Bool[puinum_refs_list1_nonanc[i]];
          for (UInt j = 0; j< puinum_refs_list1_nonanc[i]; j++)
          {
            m_ppbNonAnchorRefL1Flag[i][j] = 0;
          }
        }
      }
    }
  }
  else
  {
	UInt i;
    for( i = 0; i<uiNumOfViews; i++)
    {
      m_puiNumAnchorL0Refs[i] = puinum_refs_list0_anc[i];
      m_puiNumAnchorL1Refs[i] = puinum_refs_list1_anc[i];
    }
    for( i = 0; i<uiNumOfViews; i++)
    {
      m_puiNumNonAnchorL0Refs[i] = puinum_refs_list0_nonanc[i];
      m_puiNumNonAnchorL1Refs[i] = puinum_refs_list1_nonanc[i];
    }
  }
  return Err::m_nOK;
}
ErrVal
SEI::ViewDependencyStructureSei::write(HeaderSymbolWriteIf* pcWriteIf )
{ 	
  RNOK(pcWriteIf->writeUvlc(m_uiSeqParameterSetId,"ViewDependencyStructureSei:seq_parameter_set_id" ) );  //SEI JJ	
  RNOK(pcWriteIf->writeFlag(m_bAnchorUpdateFlag,"ViewDependencyStructureSei: anchor_update_flag"));
  RNOK(pcWriteIf->writeFlag(m_bNonAnchorUpdateFlag, "ViewDependencyStructureSei: non_anchor_update_flag"));
  if (m_bAnchorUpdateFlag)
  {
    for ( UInt i = 1; i< m_uiNumOfViews; i++) // ying
    {
	  UInt j;	
      for ( j = 0; j< m_puiNumAnchorL0Refs[i]; j++)
      {
        pcWriteIf->writeFlag( m_ppbAnchorRefL0Flag[i][j], "ViewDependencyStructureSei: anchor_ref_l0_flag");
      }
      for ( j = 0; j< m_puiNumAnchorL1Refs[i]; j++)
      {
        pcWriteIf->writeFlag( m_ppbAnchorRefL1Flag[i][j], "ViewDependencyStructureSei: anchor_ref_l1_flag");
      }
    }
  }
  if (m_bNonAnchorUpdateFlag)
  {
    for ( UInt i = 1; i< m_uiNumOfViews; i++) // ying
    {
	  UInt j;	
      for ( j = 0; j< m_puiNumNonAnchorL0Refs[i]; j++)
      {
        pcWriteIf->writeFlag( m_ppbNonAnchorRefL0Flag[i][j], "ViewDependencyStructureSei: non_anchor_ref_l0_flag");
      }
      for ( j = 0; j< m_puiNumNonAnchorL1Refs[i]; j++)
      {
        pcWriteIf->writeFlag( m_ppbNonAnchorRefL1Flag[i][j], "ViewDependencyStructureSei: non_anchor_ref_l1_flag");
      }
    }
  }
  return Err::m_nOK;
}
ErrVal
SEI::ViewDependencyStructureSei::read(HeaderSymbolReadIf* pcReadIf)
{
  RNOK(pcReadIf->getUvlc(m_uiSeqParameterSetId, "ViewDependencyStructureSei:seq_parameter_set_id" ) );  //SEI JJ	
  RNOK(pcReadIf->getFlag(m_bAnchorUpdateFlag,"ViewDependencyStructureSei: anchor_update_flag"));
  RNOK(pcReadIf->getFlag(m_bNonAnchorUpdateFlag, "ViewDependencyStructureSei: non_anchor_update_flag"));
  if (m_bAnchorUpdateFlag)
  {
    for ( UInt i = 1; i< m_uiNumOfViews; i++) //ying
    {
      m_ppbAnchorRefL0Flag[i] = new Bool[m_puiNumAnchorL0Refs[i]];
      m_ppbAnchorRefL1Flag[i] = new Bool[m_puiNumAnchorL1Refs[i]];

	  UInt j;
      for ( j = 0; j< m_puiNumAnchorL0Refs[i]; j++)
      {
        pcReadIf->getFlag( m_ppbAnchorRefL0Flag[i][j], "ViewDependencyStructureSei: anchor_ref_l0_flag");
      }
      for ( j = 0; j< m_puiNumAnchorL1Refs[i]; j++)
      {
        pcReadIf->getFlag( m_ppbAnchorRefL1Flag[i][j], "ViewDependencyStructureSei: anchor_ref_l1_flag");
      }
    }
  }
  if (m_bNonAnchorUpdateFlag)
  {
    for ( UInt i = 1; i< m_uiNumOfViews; i++) //ying
    {
      m_ppbNonAnchorRefL0Flag[i] = new Bool[m_puiNumNonAnchorL0Refs[i]];
      m_ppbNonAnchorRefL1Flag[i] = new Bool[m_puiNumNonAnchorL1Refs[i]];

	  UInt j;
      for ( j = 0; j< m_puiNumNonAnchorL0Refs[i]; j++)
      {
        pcReadIf->getFlag( m_ppbNonAnchorRefL0Flag[i][j], "ViewDependencyStructureSei: non_anchor_ref_l0_flag");
      }
      for ( j = 0; j< m_puiNumNonAnchorL1Refs[i]; j++)
      {
        pcReadIf->getFlag( m_ppbNonAnchorRefL1Flag[i][j], "ViewDependencyStructureSei: non_anchor_ref_l1_flag");
      }
    }
  }
  return Err::m_nOK;
}
////////////////////////////////////////////////////////////////////////////////
//
//  Operation Point Not Present //JVT-AB025
//
///////////////////////////////////////////////////////////////////////////////
SEI::OPNotPresentSei::OPNotPresentSei()
:SEIMessage(OP_NOT_PRESENT_SEI)
,m_uiNumNotPresentOP(0)
,m_uiNotPresentOPId(NULL)
{}
SEI::OPNotPresentSei::~OPNotPresentSei()
{
  delete []m_uiNotPresentOPId;
  m_uiNotPresentOPId = NULL;
}
ErrVal
SEI::OPNotPresentSei::destroy()
{
  delete this;
  return Err::m_nOK;
}
ErrVal
SEI::OPNotPresentSei::create(OPNotPresentSei*& rpcSeiMessage)
{
  rpcSeiMessage = new OPNotPresentSei();
  ROT(NULL == rpcSeiMessage);
  return Err::m_nOK;
}
ErrVal
SEI::OPNotPresentSei::init( UInt uiNum)
{
  m_uiNotPresentOPId = new UInt[uiNum];
  return Err::m_nOK;
}
ErrVal
SEI::OPNotPresentSei::write(HeaderSymbolWriteIf * pcWriteIf)
{
  pcWriteIf->writeUvlc( m_uiNumNotPresentOP, "OPNotPresentSei: uiNumNotPresentOP");
  for ( UInt i = 0; i < m_uiNumNotPresentOP; i++ )
  {
    pcWriteIf->writeUvlc( m_uiNotPresentOPId[i], "OPNotPresentSei: NotPresentOPId");
  }
  return Err::m_nOK;
}
ErrVal
SEI::OPNotPresentSei::read(HeaderSymbolReadIf* pcReadIf)
{
  pcReadIf->getUvlc( m_uiNumNotPresentOP, "OPNotPresentSei: uiNumNotPresentOP");
  m_uiNotPresentOPId = new UInt[m_uiNumNotPresentOP];
  for ( UInt i = 0; i < m_uiNumNotPresentOP; i++ )
  {
    pcReadIf->getUvlc( m_uiNotPresentOPId[i], "OPNotPresentSei: NotPresentOPId");
  }
  return Err::m_nOK;
}
//JVT-AB025 }}
H264AVC_NAMESPACE_END

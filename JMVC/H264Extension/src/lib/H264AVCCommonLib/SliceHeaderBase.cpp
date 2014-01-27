#include "H264AVCCommonLib.h"

#include "H264AVCCommonLib/SliceHeaderBase.h"
#include "H264AVCCommonLib/TraceFile.h"

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN


Int iRandom( Int iMin, Int iMax )
{
  Int iRange = iMax - iMin + 1;
  Int iValue = rand() % iRange;
  return iMin + iValue;
}

ErrVal
SliceHeaderBase::PredWeight::createRandomParameters()
{
  setLumaWeightFlag  ( 0 == ( rand() & 1 ) );
  setChromaWeightFlag( 0 == ( rand() & 1 ) );

  if( getLumaWeightFlag() )
  {
    setLumaOffset   (     iRandom( -64, 63 ) );
    setLumaWeight   (     iRandom( -64, 63 ) );
  }
  if( getChromaWeightFlag() )
  {
    setChromaOffset ( 0,  iRandom( -64, 63 ) );
    setChromaWeight ( 0,  iRandom( -64, 63 ) );
    setChromaOffset ( 1,  iRandom( -64, 63 ) );
    setChromaWeight ( 1,  iRandom( -64, 63 ) );
  }
  return Err::m_nOK;
}

ErrVal
SliceHeaderBase::PredWeight::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK(   pcWriteIf->writeFlag( getLumaWeightFlag(),    "PWT: luma_weight_flag" ) );
  if( getLumaWeightFlag() )
  {
    RNOK( pcWriteIf->writeSvlc( getLumaWeight(),        "PWT: luma_weight" ) );
    RNOK( pcWriteIf->writeSvlc( getLumaOffset(),        "PWT: luma_offset" ) );
  }

  RNOK(   pcWriteIf->writeFlag( getChromaWeightFlag(),  "PWT: chroma_weight_flag" ) );
  if( getChromaWeightFlag() )
  {
    RNOK( pcWriteIf->writeSvlc( getChromaWeight( 0 ),   "PWT: cr_weight" ) );
    RNOK( pcWriteIf->writeSvlc( getChromaOffset( 0 ),   "PWT: cr_offset" ) );
    RNOK( pcWriteIf->writeSvlc( getChromaWeight( 1 ),   "PWT: cb_weight" ) );
    RNOK( pcWriteIf->writeSvlc( getChromaOffset( 1 ),   "PWT: cb_offset" ) );
  }

  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::PredWeight::read( HeaderSymbolReadIf* pcReadIf )
{
  RNOK(   pcReadIf->getFlag( m_bLumaWeightFlag,     "PWT: luma_weight_flag" ) );
  if( getLumaWeightFlag() )
  {
    RNOK( pcReadIf->getSvlc( m_iLumaWeight,         "PWT: luma_weight" ) );
    RNOK( pcReadIf->getSvlc( m_iLumaOffset,         "PWT: luma_offset" ) );
    ROTR( (-128 > m_iLumaWeight) || (127 < m_iLumaWeight), Err::m_nInvalidParameter );
    ROTR( (-128 > m_iLumaOffset) || (127 < m_iLumaOffset), Err::m_nInvalidParameter );
  }

  RNOK(   pcReadIf->getFlag( m_bChromaWeightFlag,   "PWT: chroma_weight_flag" ) );
  if( getChromaWeightFlag() )
  {
    RNOK( pcReadIf->getSvlc( m_aiChromaWeight[0],   "PWT: cr_weight" ) );
    RNOK( pcReadIf->getSvlc( m_aiChromaOffset[0],   "PWT: cr_offset" ) );
    RNOK( pcReadIf->getSvlc( m_aiChromaWeight[1],   "PWT: cb_weight" ) );
    RNOK( pcReadIf->getSvlc( m_aiChromaOffset[1],   "PWT: cb_offset" ) );
    ROTR( (-128 > m_aiChromaWeight[0]) || (127 < m_aiChromaWeight[0]), Err::m_nInvalidParameter );
    ROTR( (-128 > m_aiChromaOffset[0]) || (127 < m_aiChromaOffset[0]), Err::m_nInvalidParameter );
    ROTR( (-128 > m_aiChromaWeight[1]) || (127 < m_aiChromaWeight[1]), Err::m_nInvalidParameter );
    ROTR( (-128 > m_aiChromaOffset[1]) || (127 < m_aiChromaOffset[1]), Err::m_nInvalidParameter );
  }
  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::PredWeightTable::initDefaults( UInt uiLumaWeightDenom, UInt uiChromaWeightDenom )
{
  const Int iLumaWeight   = 1 << uiLumaWeightDenom;
  const Int iChromaWeight = 1 << uiChromaWeightDenom;

//TMM_WP
  const Int iLumaOffset = 0;
  const Int iChromaOffset = 0;
//TMM_WP

  for( UInt ui = 0; ui < size(); ui++ )
  {
    RNOK( get( ui ).init( iLumaWeight, iChromaWeight, iChromaWeight ) );

//TMM_WP
    RNOK( get( ui ).initOffsets( iLumaOffset, iChromaOffset, iChromaOffset ) );
//TMM_WP
  }
  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::createRandomParameters()
{
  ROT( 0 == size() );

  for( UInt n = 0; n < size(); n++ )
  {
    RNOK( get(n).createRandomParameters( ) );
  }
  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::PredWeightTable::write( HeaderSymbolWriteIf* pcWriteIf, UInt uiNumber ) const
{
  for( UInt ui = 0; ui < uiNumber; ui++ )
  {
    RNOK( get( ui ).write( pcWriteIf ) );
  }
  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::read( HeaderSymbolReadIf* pcReadIf, UInt uiNumber )
{
  for( UInt ui = 0; ui < uiNumber; ui++ )
  {
    RNOK( get( ui ).read( pcReadIf ) );
  }
  return Err::m_nOK;
}

ErrVal
SliceHeaderBase::PredWeightTable::copy( const PredWeightTable& rcPredWeightTable )
{
  UInt uiCopySize = min( m_uiBufferSize, rcPredWeightTable.m_uiBufferSize );
  for( UInt ui = 0; ui < uiCopySize; ui++ )
  {
    m_pT[ui].copy( rcPredWeightTable.m_pT[ui] );
  }
  return Err::m_nOK;
}





SliceHeaderBase::SliceHeaderBase( const SequenceParameterSet& rcSPS,
                                  const PictureParameterSet&  rcPPS )
: m_rcPPS                             ( rcPPS )
, m_rcSPS                             ( rcSPS )
, m_eNalRefIdc                        ( NAL_REF_IDC_PRIORITY_LOWEST )
, m_eNalUnitType                      ( NAL_UNIT_EXTERNAL )
, m_uiLayerId                         ( 0 )
, m_uiTemporalLevel                   ( 0 )
, m_uiQualityLevel                    ( 0 )
, m_uiFirstMbInSlice                  ( 0 )
, m_eSliceType                        ( B_SLICE )
, m_uiPicParameterSetId               ( rcPPS.getPicParameterSetId() )
, m_uiFrameNum                        ( 0 )
, m_uiNumMbsInSlice                   ( 0 )
, m_bFgsComponentSep                  ( 0 )
, m_uiIdrPicId                        ( 0 )
, m_uiPicOrderCntLsb                  ( 0 )
, m_iDeltaPicOrderCntBottom           ( 0 )
, m_bFieldPicFlag                     ( false )
, m_bBottomFieldFlag                  ( false )
, m_bBasePredWeightTableFlag          ( false )
, m_uiLumaLog2WeightDenom             ( 5 )
, m_uiChromaLog2WeightDenom           ( 5 )
, m_bDirectSpatialMvPredFlag          ( true )
, m_uiSnapShotId					  ( -1 )  //SEI
, m_bKeyPictureFlag                   ( false )
, m_bKeyPicFlagScalable               ( false )  //JVT-S036 lsj
, m_uiBaseLayerId                     ( MSYS_UINT_MAX )
, m_uiBaseQualityLevel                ( 0 )
, m_uiBaseFragmentOrder				        ( 0 )
, m_bAdaptivePredictionFlag           ( false )
, m_bNumRefIdxActiveOverrideFlag      ( false )
, m_bNoOutputOfPriorPicsFlag          ( true  )
, m_bAdaptiveRefPicBufferingModeFlag  ( false )
, m_bAdaptiveRefPicMarkingModeFlag	  ( false )    //JVT-S036 
, m_uiCabacInitIdc                    ( 0 )
, m_iSliceQpDelta                     ( 0 )
, m_pcFMO                             ( 0 ) //--ICU/ETRI FMO Implementation
//TMM_ESS_UNIFIED {
, m_iScaledBaseLeftOffset             ( 0 ) 
, m_iScaledBaseTopOffset              ( 0 ) 
, m_iScaledBaseRightOffset            ( 0 ) 
, m_iScaledBaseBottomOffset           ( 0 ) 
//TMM_ESS_UNIFIED }
, m_uiBaseChromaPhaseXPlus1           ( 0 ) // TMM_ESS
, m_uiBaseChromaPhaseYPlus1           ( 1 ) // TMM_ESS
, m_bArFgsUsageFlag                   ( false )
, m_uiLowPassFgsMcFilter              ( AR_FGS_DEFAULT_FILTER )
, m_uiBaseWeightZeroBaseBlock         ( AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_BLOCK )
, m_uiBaseWeightZeroBaseCoeff         ( AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_COEFF )
, m_uiFragmentOrder                   ( 0 )
, m_uiRedundantPicCnt                 ( 0 ) //JVT-Q054 Red. Picture
, m_uiSliceGroupChangeCycle           ( 0 ) 
, m_eErrorConceal                     ( EC_NONE ) 
//JVT-T054{
, m_uiLayerCGSSNR                     ( 0 )
, m_uiQualityLevelCGSSNR              ( 0 )
, m_uiBaseLayerCGSSNR                 ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR          ( 0 )
//JVT-T054}


, m_svc_mvc_flag                      (false)
, m_bAVCFlag                          (false)
, m_anchor_pic_flag                   (false)
, m_view_id                           (0) 
, m_bNonIDRFlag                       (true)
, m_reserved_zero_bits                (0)
, m_reserved_one_bit                  (1)     // bug fix: prefix NAL (NTT)
, m_bInterViewRef                     (false) // maybe shall be replaced by inter_view_flag
{
  ::memset( m_auiNumRefIdxActive        , 0x00, 2*sizeof(UInt) );
  ::memset( m_aauiNumRefIdxActiveUpdate , 0x00, 2*sizeof(UInt)*MAX_TEMP_LEVELS );
  ::memset( m_aiDeltaPicOrderCnt,         0x00, 2*sizeof(Int) );

}


SliceHeaderBase::~SliceHeaderBase()
{
  if(m_pcFMO)
  //manu.mathew@samsung : memory leak fix
  {
    delete m_pcFMO; m_pcFMO = NULL;
  }
  //--
  ANOK( m_acPredWeightTable[LIST_0].uninit() );
  ANOK( m_acPredWeightTable[LIST_1].uninit() );
}


ErrVal
SliceHeaderBase::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||  
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_PREFIX       )
  {
    return xWriteScalable         ( pcWriteIf );
  }
  else
  {
    return xWriteH264AVCCompatible( pcWriteIf );
  }
}



ErrVal
SliceHeaderBase::xWriteScalable( HeaderSymbolWriteIf* pcWriteIf ) const
{
  //===== NAL unit header =====
  RNOK  ( pcWriteIf->writeFlag( 0,                                              "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalRefIdc,   2,                              "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalUnitType, 5,                              "NALU HEADER: nal_unit_type" ) );
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_PREFIX       )
  {

	RNOK  ( pcWriteIf->writeCode( this->getSvcMvcFlag(),   1,                              "NALU HEADER: svc_mvc_flag" ) );		
	if (this->getSvcMvcFlag()!=0) {              

    //{{Variable Lengh NAL unit header data with priority and dead substream flag
    //France Telecom R&D- (nathalie.cammas@francetelecom.com)
    RNOK (pcWriteIf->writeCode( m_uiSimplePriorityId,  6,                       "NALU HEADER: simple_priority_id"));
    RNOK (pcWriteIf->writeFlag( m_bDiscardableFlag,                             "NALU HEADER: discardable_flag"));
  	RNOK (pcWriteIf->writeFlag( m_bReservedZeroBit,                             "NALU HEADER: reserved_zero_bit"));
   	RNOK (pcWriteIf->writeCode( m_uiTemporalLevel,   3,                         "NALU HEADER: temporal_level"));
    if( m_eSliceType != F_SLICE )
    {
      RNOK( pcWriteIf->writeCode( m_uiLayerCGSSNR,         3,                       "NALU HEADER: dependency_id" ) );
      RNOK( pcWriteIf->writeCode( m_uiQualityLevelCGSSNR,    2,                       "NALU HEADER: quality_level" ) );
    }
    else
    {
      RNOK( pcWriteIf->writeCode( m_uiLayerId,         3,                       "NALU HEADER: dependency_id" ) );
      RNOK( pcWriteIf->writeCode( m_uiQualityLevel,    2,                       "NALU HEADER: quality_level" ) );
    }

	} else {
	  RNOK  ( pcWriteIf->writeCode( this->getNonIDRFlag(),                 1,                              "NALU HEADER: non_idr_flag" ) );
	  RNOK (  pcWriteIf->writeCode( m_uiSimplePriorityId,               6,                              "NALU HEADER: priority_id"));
	  RNOK  ( pcWriteIf->writeCode( this->getViewId(),                  10,                             "NALU HEADER: view_id" ) );
		RNOK (  pcWriteIf->writeCode( m_uiTemporalLevel,                  3,                              "NALU HEADER: temporal_id"));
		RNOK  ( pcWriteIf->writeCode( this->getAnchorPicFlag(),           1,                              "NALU HEADER: anchor_pic_flag" ) );
		RNOK  ( pcWriteIf->writeCode( this->getInterViewFlag(),		    		1,														  "NALU HEADER: inter_view_flag") ); 
    RNOK  ( pcWriteIf->writeCode( this->getReservedOneBit(),          1,                              "NALU HEADER: reserved_one_bit" ) ); // bug fix: prefix NAL (NTT)
	
		return Err::m_nOK;

	}




  }


//JVT-S036  start
 if(m_uiLayerId == 0 && m_uiQualityLevel == 0 && m_eAVCCompatible )
 {
	  if( m_eNalRefIdc != 0)
	  {
		  RNOK (pcWriteIf->writeFlag( m_bKeyPictureFlag,					"SH: key_pic_flag"));
		  
		  if( m_bKeyPictureFlag && m_eNalUnitType != 21)
		  {
			   RNOK(pcWriteIf->writeFlag( m_bAdaptiveRefPicMarkingModeFlag,			"DRPM: adaptive_ref_pic_marking_mode_flag"));
			   if(m_bAdaptiveRefPicMarkingModeFlag)
			   {
				   RNOK( getMmcoBaseBuffer().write( pcWriteIf ) );
		       }
		  }
	  }
 }

 else
 {
//JVT-S036  end
	//===== slice header =====
	RNOK(     pcWriteIf->writeUvlc( m_uiFirstMbInSlice,                           "SH: first_mb_in_slice" ) );
	  
	UInt  uiSliceType = ( m_eSliceType == B_SLICE ? 0 : m_eSliceType == P_SLICE ? 1 : UInt(m_eSliceType) );
	RNOK(     pcWriteIf->writeUvlc( uiSliceType,                                  "SH: slice_type" ) );
	  
	//JVT-P031
	if(uiSliceType == F_SLICE)
	{
		RNOK( pcWriteIf    ->writeFlag( m_bFragmentedFlag,  "SH: fgs_frag_flag" ) );
		if(m_bFragmentedFlag)
		{
			RNOK( pcWriteIf    ->writeUvlc( m_uiFragmentOrder,  "SH: fgs_frag_order" ) );
			if(m_uiFragmentOrder!=0)
			{
			RNOK( pcWriteIf    ->writeFlag( m_bLastFragmentFlag,  "SH: fgs_last_frag_flag" ) );
			}
		}
	}
	if(m_uiFragmentOrder == 0)
	{
	//~JVT-P031
		if( m_eSliceType == F_SLICE ) 
		{
			RNOK(   pcWriteIf->writeUvlc( m_uiNumMbsInSlice,                            "SH: num_mbs_in_slice" ) );
			RNOK(   pcWriteIf->writeFlag( m_bFgsComponentSep,                           "SH: fgs_comp_sep" ) );
		}
	RNOK(     pcWriteIf->writeUvlc( m_uiPicParameterSetId,                        "SH: pic_parameter_set_id" ) );
	RNOK(     pcWriteIf->writeCode( m_uiFrameNum,
									getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );

	if(!getSPS().getFrameMbsOnlyFlag())
	{
		RNOK(   pcWriteIf->writeFlag( m_bFieldPicFlag,                                 "SH: field_pic_flag" ) );
		if(m_bFieldPicFlag)
			RNOK(   pcWriteIf->writeFlag( m_bBottomFieldFlag,                                 "SH: bottom_field_flag" ) );
	}

	//if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ) // fix Ying @QCT
	if(!getNonIDRFlag())//lufeng
	{
		RNOK(   pcWriteIf->writeUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
	}
	  
	if( getSPS().getPicOrderCntType() == 0 )
	{
	RNOK(     pcWriteIf->writeCode( m_uiPicOrderCntLsb,
									getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
	if( getPPS().getPicOrderPresentFlag() &&  m_bFieldPicFlag  )
	{
		RNOK( pcWriteIf->writeSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
		}
	}


	if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
	{
		RNOK(   pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
		if( getPPS().getPicOrderPresentFlag() &&   ! m_bFieldPicFlag )
		{
		RNOK( pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
		}
	}
//JVT-S036  start
	}
	if ( m_eNalRefIdc != 0 )
	{
		RNOK( pcWriteIf->writeFlag( m_bKeyPictureFlag,    						"SH: key_pic_flag"));
	}
	if( m_uiFragmentOrder == 0 )
	{
//JVT-S036  end
	//JVT-Q054 Red. Picture {
	if ( m_eSliceType != F_SLICE )
	{
		if ( getPPS().getRedundantPicCntPresentFlag() )
		{
		RNOK( pcWriteIf->writeUvlc( m_uiRedundantPicCnt,                          "SH: redundant_pic_cnt") );
		}
	}
	//JVT-Q054 Red. Picture }
	  
	if( m_eSliceType == B_SLICE )
	{
		RNOK(   pcWriteIf->writeFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
	}
	  
	if( m_eSliceType != F_SLICE )
	{
		UInt  uiBaseLayerIdPlus1;
		if( m_uiBaseLayerId == MSYS_UINT_MAX )
		uiBaseLayerIdPlus1 = 0;
		else
//JVT-T054{
    {
      if(m_uiQualityLevelCGSSNR != m_uiQualityLevel || m_uiLayerCGSSNR != m_uiLayerId)
      {
        uiBaseLayerIdPlus1 = ( (m_uiBaseLayerCGSSNR << 4) + ((m_uiBaseQualityLevelCGSSNR) << 2) + m_uiBaseFragmentOrder ) + 1;
      }
      else
      {
        // one example (m_uiBaseLayerId, m_uiBaseQualityLevel) -> uiBaseLayerIdPlus1 mapping
	      uiBaseLayerIdPlus1 = ( (m_uiBaseLayerId << 4) + (m_uiBaseQualityLevel << 2) + m_uiBaseFragmentOrder ) + 1;
      }
    }
//JVT-T054}
		RNOK(   pcWriteIf->writeUvlc( uiBaseLayerIdPlus1,                           "SH: base_id_plus1" ) );
		if( uiBaseLayerIdPlus1 )
		{
		RNOK( pcWriteIf->writeFlag( m_bAdaptivePredictionFlag,                    "SH: adaptive_prediction_flag" ) );
		}
	    
		if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
		{
		RNOK( pcWriteIf->writeFlag( m_bNumRefIdxActiveOverrideFlag,               "SH: num_ref_idx_active_override_flag" ) );
		if( m_bNumRefIdxActiveOverrideFlag )
		{
			RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_0]-1,             "SH: num_ref_idx_l0_active_minus1" ) );
			if( m_eSliceType == B_SLICE )
			{
			RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_1]-1,           "SH: num_ref_idx_l1_active_minus1" ) );
			}
		}
		}


		if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
		{
		RNOK( getRplrBuffer( LIST_0 ).write( pcWriteIf ) );
		}
		if( m_eSliceType == B_SLICE )
		{
		RNOK( getRplrBuffer( LIST_1 ).write( pcWriteIf ) );
		}

		if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
			( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
		{
		if( m_bAdaptivePredictionFlag ) 
		{
			RNOK( pcWriteIf->writeFlag( m_bBasePredWeightTableFlag,                "PWT: base_pred_weight_table_flag" ) );
		}
		if( ! m_bBasePredWeightTableFlag )
		{
			RNOK( pcWriteIf->writeUvlc( m_uiLumaLog2WeightDenom,                   "PWT: luma_log_weight_denom" ) );
			RNOK( pcWriteIf->writeUvlc( m_uiChromaLog2WeightDenom,                 "PWT: chroma_log_weight_denom" ) );

			RNOK( m_acPredWeightTable[LIST_0].write( pcWriteIf, getNumRefIdxActive( LIST_0 ) ) );
			if( m_eSliceType == B_SLICE )
			{
			RNOK( m_acPredWeightTable[LIST_1].write( pcWriteIf, getNumRefIdxActive( LIST_1) ) );
			}
		}
		}

		if( getNalRefIdc() )
		{
		if( isIdrNalUnit() )
		{
			RNOK( pcWriteIf->writeFlag( m_bNoOutputOfPriorPicsFlag,                 "DRPM: no_output_of_prior_pics_flag" ) );
			RNOK( pcWriteIf->writeFlag( false,                                      "DRPM: long_term_reference_flag" ) );
		}
		else
		{
			RNOK( pcWriteIf->writeFlag( m_bAdaptiveRefPicBufferingModeFlag,         "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
			if( m_bAdaptiveRefPicBufferingModeFlag )
			{
			RNOK( getMmcoBuffer().write( pcWriteIf ) );
			}
		}
	//JVT-S036  start
		if(getKeyPictureFlag() && getNalUnitType() !=21)
		{
			RNOK(pcWriteIf->writeFlag( m_bAdaptiveRefPicMarkingModeFlag,			"DRPM: adaptive_ref_pic_marking_mode_flag"));
			if(m_bAdaptiveRefPicMarkingModeFlag)
			{		
				RNOK( getMmcoBaseBuffer().write( pcWriteIf ) );
			}			  
		}
	//JVT-S036  end
		}

		if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
		{
		RNOK( pcWriteIf->writeUvlc( m_uiCabacInitIdc,                             "SH: cabac_init_idc" ) );
		}
	}


	RNOK( pcWriteIf->writeSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
	  
	if( getPPS().getDeblockingFilterParametersPresentFlag() )
	{
		RNOK( getDeblockingFilterParameter().write( pcWriteIf ) );
	}

	if(getPPS().getNumSliceGroupsMinus1()>0 && getPPS().getSliceGroupMapType() >=3 && getPPS().getSliceGroupMapType() <= 5)
	{    
		RNOK(     pcWriteIf->writeCode( m_uiSliceGroupChangeCycle, getPPS().getLog2MaxSliceGroupChangeCycle(getSPS().getMbInFrame()) ,                "SH: slice_group_change_cycle" ) );
	}

	// TMM_ESS {
	if ((m_eSliceType != F_SLICE) && (getSPS().getExtendedSpatialScalability() > ESS_NONE))
	{
		//if ( 1 /* chroma_format_idc */ > 0 )
		{
		RNOK( pcWriteIf->writeCode( m_uiBaseChromaPhaseXPlus1, 2,                  "SH: BaseChromaPhaseXPlus1" ) );
		RNOK( pcWriteIf->writeCode( m_uiBaseChromaPhaseYPlus1, 2,                  "SH: BaseChromaPhaseXPlus1" ) );
		}
	    
		if (getSPS().getExtendedSpatialScalability() == ESS_PICT)
		{
		RNOK( pcWriteIf->writeSvlc( m_iScaledBaseLeftOffset,                       "SH: ScaledBaseLeftOffset" ) );
		RNOK( pcWriteIf->writeSvlc( m_iScaledBaseTopOffset,                        "SH: ScaledBaseTopOffset" ) );
		RNOK( pcWriteIf->writeSvlc( m_iScaledBaseRightOffset,                      "SH: ScaledBaseRightOffset" ) );
		RNOK( pcWriteIf->writeSvlc( m_iScaledBaseBottomOffset,                     "SH: ScaledBaseBottomOffset" ) );
		}
	}
	// TMM_ESS }

	if( m_eSliceType == F_SLICE )
	{
		RNOK( pcWriteIf->writeFlag( m_bArFgsUsageFlag,                               "SH: base_layer_key_pic_flag" ) );
		if( m_bArFgsUsageFlag )
		{
		// send other information conditionally
		UInt uiWeight;

		// AR_FGS_MAX_BASE_WEIGHT - 1 is not allowed
		uiWeight = ( m_uiBaseWeightZeroBaseBlock <= 1 ) ? 0 : ( m_uiBaseWeightZeroBaseBlock - 1 );
		RNOK( pcWriteIf->writeCode( uiWeight, 5,                                   "SH: base_ref_weight_for_zero_base_block" ) );

		// AR_FGS_MAX_BASE_WEIGHT - 1 is not allowed
		uiWeight = ( m_uiBaseWeightZeroBaseCoeff <= 1 ) ? 0 : ( m_uiBaseWeightZeroBaseCoeff - 1 );
		RNOK( pcWriteIf->writeCode( uiWeight, 5,                                   "SH: base_ref_weight_for_zero_base_coeff" ) );

		RNOK( pcWriteIf->writeFlag( m_bFgsEntropyOrderFlag,                               "SH: fgs_order_flag" ) );
		}
		RNOK( pcWriteIf->writeFlag( m_bAdaptivePredictionFlag,                       "SH: motion_refinement_flag" ) );
	}
} //JVT-P031
}//JVT-S036 

  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::xWriteH264AVCCompatible( HeaderSymbolWriteIf* pcWriteIf ) const
{
  //===== NAL unit header =====

	enum NalUnitType NNalUnitType = m_eNalUnitType; 
	if ( !this->getAVCFlag() ) //JVT-W035
	{
  	NNalUnitType =NAL_UNIT_CODED_SLICE_SCALABLE;

    
	}
	
	RNOK  ( pcWriteIf->writeFlag( 0,                                              "NALU HEADER: forbidden_zero_bit" ) );
	RNOK  ( pcWriteIf->writeCode( m_eNalRefIdc,   2,                              "NALU HEADER: nal_ref_idc" ) );
	RNOK  ( pcWriteIf->writeCode( NNalUnitType, 5,                              "NALU HEADER: nal_unit_type" ) );

    
	
	if ( !this->getAVCFlag() ) //JVT-W035
	{
	  RNOK  ( pcWriteIf->writeCode( this->getSvcMvcFlag(),          1,                                "NALU HEADER: svc_mvc_flag" ) );
		RNOK  ( pcWriteIf->writeCode( this->getNonIDRFlag(),                 1,                       "NALU HEADER: non_idr_flag" ) );
	  RNOK (  pcWriteIf->writeCode( m_uiSimplePriorityId,               6,                              "NALU HEADER: priority_id"));
	  RNOK  ( pcWriteIf->writeCode( this->getViewId(),                  10,                             "NALU HEADER: view_id" ) );
		RNOK (  pcWriteIf->writeCode( m_uiTemporalLevel,                  3,                              "NALU HEADER: temporal_id"));
		RNOK  ( pcWriteIf->writeCode( this->getAnchorPicFlag(),           1,                              "NALU HEADER: anchor_pic_flag" ) );
		RNOK  ( pcWriteIf->writeCode( this->getInterViewFlag(),		    		1,														  "NALU HEADER: inter_view_flag") ); 
    RNOK  ( pcWriteIf->writeCode( this->getReservedOneBit(),          1,                              "NALU HEADER: reserved_one_bit" ) ); // bug fix: prefix NAL (NTT)
		
	}
	

  //===== slice header =====
  RNOK(     pcWriteIf->writeUvlc( m_uiFirstMbInSlice,                           "SH: first_mb_in_slice" ) );
  RNOK(     pcWriteIf->writeUvlc( m_eSliceType,                                 "SH: slice_type" ) );
  RNOK(     pcWriteIf->writeUvlc( m_uiPicParameterSetId,                        "SH: pic_parameter_set_id" ) );
  RNOK(     pcWriteIf->writeCode( m_uiFrameNum,
                                  getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );

  //lufeng: write field_pic_flag, bottom_field_flag in SH
  	if(!getSPS().getFrameMbsOnlyFlag() )
	{
		RNOK(   pcWriteIf->writeFlag( m_bFieldPicFlag,                                 "SH: field_pic_flag" ) );
		if(m_bFieldPicFlag)
			RNOK(   pcWriteIf->writeFlag( m_bBottomFieldFlag,                                 "SH: bottom_field_flag" ) );
	}
  //if( NNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  if(!this->getNonIDRFlag())//lufeng
  {
    RNOK(   pcWriteIf->writeUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( getSPS().getPicOrderCntType() == 0 )
  {
  RNOK(     pcWriteIf->writeCode( m_uiPicOrderCntLsb,
                                  getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
    if( getPPS().getPicOrderPresentFlag() &&  ! m_bFieldPicFlag  )
    {
      RNOK( pcWriteIf->writeSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }
  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
	if( getPPS().getPicOrderPresentFlag() && ! m_bFieldPicFlag )
    {
      RNOK( pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
  //JVT-Q054 Red. Picture {
  if ( getPPS().getRedundantPicCntPresentFlag() )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiRedundantPicCnt,                            "SH: redundant_pic_cnt") );
  }
  //JVT-Q054 Red. Picture }
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcWriteIf->writeFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( pcWriteIf->writeFlag( m_bNumRefIdxActiveOverrideFlag,                 "SH: num_ref_idx_active_override_flag" ) );
    if( m_bNumRefIdxActiveOverrideFlag )
    {
      RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_0]-1,               "SH: num_ref_idx_l0_active_minus1" ) );
      if( m_eSliceType == B_SLICE )
      {
        RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_1]-1,             "SH: num_ref_idx_l1_active_minus1" ) );
      }
    }
  }


  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_0 ).write( pcWriteIf ) );
  }
  if( m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_1 ).write( pcWriteIf ) );
  }

  if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
      ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiLumaLog2WeightDenom,                      "PWT: luma_log_weight_denom" ) );
    RNOK( pcWriteIf->writeUvlc( m_uiChromaLog2WeightDenom,                    "PWT: chroma_log_weight_denom" ) );

    RNOK( m_acPredWeightTable[LIST_0].write( pcWriteIf, getNumRefIdxActive( LIST_0 ) ) );
    if( m_eSliceType == B_SLICE )
    {
      RNOK( m_acPredWeightTable[LIST_1].write( pcWriteIf, getNumRefIdxActive( LIST_1) ) );
    }
  }

  if( getNalRefIdc() )
  {
    if( isIdrNalUnit() )
    {
      RNOK( pcWriteIf->writeFlag( m_bNoOutputOfPriorPicsFlag,                   "DRPM: no_output_of_prior_pics_flag" ) );
      RNOK( pcWriteIf->writeFlag( false,                                        "DRPM: long_term_reference_flag" ) );
    }
    else
    {
      RNOK( pcWriteIf->writeFlag( m_bAdaptiveRefPicBufferingModeFlag,           "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
      if( m_bAdaptiveRefPicBufferingModeFlag )
      {
        RNOK( getMmcoBuffer().write( pcWriteIf ) );
      }
    }
  }

  if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiCabacInitIdc,                               "SH: cabac_init_idc" ) );
  }


  RNOK( pcWriteIf->writeSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    RNOK( getDeblockingFilterParameter().write( pcWriteIf ) );
  }

  if(getPPS().getNumSliceGroupsMinus1()>0 && getPPS().getSliceGroupMapType() >=3 && getPPS().getSliceGroupMapType() <= 5)
  {    
    RNOK(     pcWriteIf->writeCode( m_uiSliceGroupChangeCycle, getPPS().getLog2MaxSliceGroupChangeCycle(getSPS().getMbInFrame()) ,                "SH: slice_group_change_cycle" ) );
  }



  return Err::m_nOK;
}




ErrVal
SliceHeaderBase::read( HeaderSymbolReadIf* pcReadIf )
{

    if( (m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE || 
         m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ) && !m_svc_mvc_flag)
    {
        return xReadMVCCompatible ( pcReadIf );
    }
    else
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE || 
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE       )
  {
    if      ( m_eSliceType == B_SLICE ) m_eSliceType = P_SLICE;
    else if ( m_eSliceType == P_SLICE ) m_eSliceType = B_SLICE;

    return xReadScalable          ( pcReadIf );
  }
  else
  {
    return xReadH264AVCCompatible ( pcReadIf );
  }



}



ErrVal
SliceHeaderBase::xReadScalable( HeaderSymbolReadIf* pcReadIf )
{
  Bool  bTmp;
  UInt  uiTmp;


  RNOK(     pcReadIf->getCode( m_uiFrameNum,
                               getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
	if(!getSPS().getFrameMbsOnlyFlag())
	{
		RNOK(   pcReadIf->getFlag( m_bFieldPicFlag,                                 "SH: field_pic_flag" ) );
		if(m_bFieldPicFlag)
			RNOK(   pcReadIf->getFlag( m_bBottomFieldFlag,                                 "SH: bottom_field_flag" ) );
	}

  if( !getNonIDRFlag()) //JVT-W035  
  {
    RNOK(   pcReadIf->getUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( getSPS().getPicOrderCntType() == 0 )
  {
  RNOK(     pcReadIf->getCode( m_uiPicOrderCntLsb,
                               getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
  if( getPPS().getPicOrderPresentFlag() &&  ! m_bFieldPicFlag )
    {
      RNOK( pcReadIf->getSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }

  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
	if( getPPS().getPicOrderPresentFlag() &&  ! m_bFieldPicFlag  )
    {
      RNOK( pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
//JVT-S036  start
  if( getNalRefIdc() )
  {
	  RNOK( pcReadIf->getFlag( m_bKeyPicFlagScalable,							 "SH: key_pic_flag"));
  }
//JVT-S036  end
  //JVT-Q054 Red. Picture {
  if ( m_eSliceType != F_SLICE )
  {
    if ( getPPS().getRedundantPicCntPresentFlag())
    {
      RNOK( pcReadIf->getUvlc( m_uiRedundantPicCnt,                            "SH: redundant_pic_cnt") );
    }
  }
  //JVT-Q054 Red. Picture }
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcReadIf->getFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }
  
  if( m_eSliceType != F_SLICE )
  {
    RNOK(   pcReadIf->getUvlc( uiTmp,                                        "SH: base_id_plus1" ) );
    m_uiBaseLayerId = uiTmp - 1;
    if( m_uiBaseLayerId != MSYS_UINT_MAX )
    {
	    m_uiBaseFragmentOrder = m_uiBaseLayerId & 0x03;
      m_uiBaseQualityLevel = (m_uiBaseLayerId >> 2) & 0x03;
	    m_uiBaseLayerId = m_uiBaseLayerId >> 4;
    }
    else
    {
      m_uiBaseQualityLevel = 0;
    }

    if( m_uiBaseLayerId != MSYS_UINT_MAX )
    {
      RNOK( pcReadIf->getFlag( m_bAdaptivePredictionFlag,                    "SH: adaptive_prediction_flag" ) );
    }
    
    if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
    {
      RNOK( pcReadIf->getFlag( m_bNumRefIdxActiveOverrideFlag,               "SH: num_ref_idx_active_override_flag" ) );
      if( m_bNumRefIdxActiveOverrideFlag )
      {
        RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_0],               "SH: num_ref_idx_l0_active_minus1" ) );
        m_auiNumRefIdxActive[LIST_0]++;
        if( m_eSliceType == B_SLICE )
        {
          RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_1],             "SH: num_ref_idx_l1_active_minus1" ) );
          m_auiNumRefIdxActive[LIST_1]++;
        }
      }
    }

    if( m_eSliceType != B_SLICE )
    {
      m_auiNumRefIdxActive[LIST_1] = 0;
    }

    if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
    {
      RNOK( getRplrBuffer( LIST_0 ).read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
    }
    if( m_eSliceType == B_SLICE )
    {
      RNOK( getRplrBuffer( LIST_1 ).read( pcReadIf, getNumRefIdxActive( LIST_1 ) ) );
    }

    RNOK( m_acPredWeightTable[LIST_0].init( 64 ) );
    RNOK( m_acPredWeightTable[LIST_1].init( 64 ) );

    if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
        ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
    {
      if( m_bAdaptivePredictionFlag)//m_uiBaseLayerId != MSYS_UINT_MAX )
      {
        RNOK( pcReadIf->getFlag( m_bBasePredWeightTableFlag,                "PWT: base_pred_weight_table_flag" ) );
      }
      else
      {
          if(m_uiBaseLayerId != MSYS_UINT_MAX)
              m_bBasePredWeightTableFlag = true;
          else
              m_bBasePredWeightTableFlag = false;
      }

      if( ! m_bBasePredWeightTableFlag )
      {
        RNOK( pcReadIf->getUvlc( m_uiLumaLog2WeightDenom,                   "PWT: luma_log_weight_denom" ) );
        RNOK( pcReadIf->getUvlc( m_uiChromaLog2WeightDenom,                 "PWT: chroma_log_weight_denom" ) );
        ROTR( m_uiLumaLog2WeightDenom   > 7, Err::m_nInvalidParameter );
        ROTR( m_uiChromaLog2WeightDenom > 7, Err::m_nInvalidParameter );

        RNOK( m_acPredWeightTable[LIST_0].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
        RNOK( m_acPredWeightTable[LIST_0].read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
        if( m_eSliceType == B_SLICE )
        {
          RNOK( m_acPredWeightTable[LIST_1].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
          RNOK( m_acPredWeightTable[LIST_1].read( pcReadIf, getNumRefIdxActive( LIST_1) ) );
        }
      }
    }

    if( getNalRefIdc() )
    {
      if( isIdrNalUnit() )
      {
        RNOK( pcReadIf->getFlag( m_bNoOutputOfPriorPicsFlag,                 "DRPM: no_output_of_prior_pics_flag" ) );
        RNOK( pcReadIf->getFlag( bTmp,                                       "DRPM: long_term_reference_flag" ) );
        ROT ( bTmp );
      }
      else
      {
        RNOK( pcReadIf->getFlag( m_bAdaptiveRefPicBufferingModeFlag,         "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
        if( m_bAdaptiveRefPicBufferingModeFlag )
        {
          RNOK( getMmcoBuffer().read( pcReadIf ) );
        }
      }
	//JVT-S036  start
	  if( getKeyPicFlagScalable() && getNalUnitType() != 21)
	  {
		  RNOK(pcReadIf->getFlag( m_bAdaptiveRefPicMarkingModeFlag,			"DRPM: adaptive_ref_pic_marking_mode_flag"));
		  if(m_bAdaptiveRefPicMarkingModeFlag)
		  {
			  RNOK( getMmcoBaseBuffer().read( pcReadIf ) );
		  }		  
	  }
	//JVT-S036  end
    }

    if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
    {
      RNOK( pcReadIf->getUvlc( m_uiCabacInitIdc,                             "SH: cabac_init_idc" ) );
    }
  }


  RNOK( pcReadIf->getSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    RNOK( getDeblockingFilterParameter().read( pcReadIf ) );
  }

  UInt uiSliceGroupChangeCycle;
  if( getPPS().getNumSliceGroupsMinus1()> 0  && getPPS().getSliceGroupMapType() >= 3  &&  getPPS().getSliceGroupMapType() <= 5)
  {
	  UInt pictureSizeInMB = getSPS().getFrameHeightInMbs()*getSPS().getFrameWidthInMbs();
	  RNOK(     pcReadIf->getCode( uiSliceGroupChangeCycle, getLog2MaxSliceGroupChangeCycle(pictureSizeInMB), "SH: slice_group_change_cycle" ) ); 
	  setSliceGroupChangeCycle(uiSliceGroupChangeCycle);
  }

// TMM_ESS {
  if ((m_eSliceType != F_SLICE) && (getSPS().getExtendedSpatialScalability() > ESS_NONE))
  {
    //if ( 1 /* chroma_format_idc */ > 0 )
    {
      RNOK( pcReadIf->getCode( m_uiBaseChromaPhaseXPlus1, 2,                 "SH: BaseChromaPhaseXPlus1" ) );
      RNOK( pcReadIf->getCode( m_uiBaseChromaPhaseYPlus1, 2,                 "SH: BaseChromaPhaseYPlus1" ) );
    }

    if (getSPS().getExtendedSpatialScalability() == ESS_PICT)
    {
      RNOK( pcReadIf->getSvlc( m_iScaledBaseLeftOffset,                          "SH: ScaledBaseLeftOffset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseTopOffset,                           "SH: ScaledBaseTopOffset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseRightOffset,                         "SH: ScaledBaseRightOffset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseBottomOffset,                        "SH: ScaledBaseBottomOffset" ) );
    }
  }
// TMM_ESS }

  if( m_eSliceType == F_SLICE )
  {
    RNOK( pcReadIf->getFlag( m_bArFgsUsageFlag,                               "SH: base_layer_key_pic_flag" ) );
    if( m_bArFgsUsageFlag ) 
    {
      // send other information conditionally
      RNOK( pcReadIf->getCode( m_uiBaseWeightZeroBaseBlock, 5,                "SH: base_ref_weight_for_zero_base_block" ) );
      if( m_uiBaseWeightZeroBaseBlock != 0 )
        m_uiBaseWeightZeroBaseBlock += 1;

      RNOK( pcReadIf->getCode( m_uiBaseWeightZeroBaseCoeff, 5,                "SH: base_ref_weight_for_zero_base_coeff" ) );
      if( m_uiBaseWeightZeroBaseCoeff != 0 )
        m_uiBaseWeightZeroBaseCoeff += 1;

      RNOK( pcReadIf->getFlag( m_bFgsEntropyOrderFlag,                               "SH: fgs_order_flag" ) );
    }
    else
    {
      m_uiBaseWeightZeroBaseBlock = AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_BLOCK;
      m_uiBaseWeightZeroBaseCoeff = AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_COEFF;
      m_bFgsEntropyOrderFlag = 0;
    }
    RNOK( pcReadIf->getFlag( m_bAdaptivePredictionFlag,                       "SH: motion_refinement_flag" ) );
  }

  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::xReadH264AVCCompatible( HeaderSymbolReadIf* pcReadIf )
{
  Bool  bTmp;
 
  RNOK(     pcReadIf->getCode( m_uiFrameNum,
                               getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
	if(!getSPS().getFrameMbsOnlyFlag())
	{
		RNOK(   pcReadIf->getFlag( m_bFieldPicFlag,                                 "SH: field_pic_flag" ) );
		if(m_bFieldPicFlag)
			RNOK(   pcReadIf->getFlag( m_bBottomFieldFlag,                                 "SH: bottom_field_flag" ) );
	}
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    RNOK(   pcReadIf->getUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( getSPS().getPicOrderCntType() == 0 )
  {
  RNOK(     pcReadIf->getCode( m_uiPicOrderCntLsb,
                               getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
  if( getPPS().getPicOrderPresentFlag() &&! m_bFieldPicFlag )
    {
      RNOK( pcReadIf->getSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }
  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
	if( getPPS().getPicOrderPresentFlag() &&  ! m_bFieldPicFlag )
  {
      RNOK( pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
  //JVT-Q054 Red. Picture {
  if ( getPPS().getRedundantPicCntPresentFlag())
  {
    RNOK( pcReadIf->getUvlc( m_uiRedundantPicCnt,                            "SH: redundant_pic_cnt") );
  }
  //JVT-Q054 Red. Picture }
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcReadIf->getFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( pcReadIf->getFlag( m_bNumRefIdxActiveOverrideFlag,                 "SH: num_ref_idx_active_override_flag" ) );
    if( m_bNumRefIdxActiveOverrideFlag )
    {
      RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_0],                 "SH: num_ref_idx_l0_active_minus1" ) );
      m_auiNumRefIdxActive[LIST_0]++;
      if( m_eSliceType == B_SLICE )
      {
        RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_1],               "SH: num_ref_idx_l1_active_minus1" ) );
        m_auiNumRefIdxActive[LIST_1]++;
      }
    }
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_0 ).read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
  }
  if( m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_1 ).read( pcReadIf, getNumRefIdxActive( LIST_1 ) ) );
  }

  RNOK( m_acPredWeightTable[LIST_0].init( 64 ) );
  RNOK( m_acPredWeightTable[LIST_1].init( 64 ) );

  if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
      ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
  {
    RNOK( pcReadIf->getUvlc( m_uiLumaLog2WeightDenom,                     "PWT: luma_log_weight_denom" ) );
    RNOK( pcReadIf->getUvlc( m_uiChromaLog2WeightDenom,                   "PWT: chroma_log_weight_denom" ) );
    ROTR( m_uiLumaLog2WeightDenom   > 7, Err::m_nInvalidParameter );
    ROTR( m_uiChromaLog2WeightDenom > 7, Err::m_nInvalidParameter );

    RNOK( m_acPredWeightTable[LIST_0].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
    RNOK( m_acPredWeightTable[LIST_0].read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
    if( m_eSliceType == B_SLICE )
    {
      RNOK( m_acPredWeightTable[LIST_1].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
      RNOK( m_acPredWeightTable[LIST_1].read( pcReadIf, getNumRefIdxActive( LIST_1) ) );
    }
  }

  if( getNalRefIdc() )
  {
    if( isIdrNalUnit() )
    {
      RNOK( pcReadIf->getFlag( m_bNoOutputOfPriorPicsFlag,                   "DRPM: no_output_of_prior_pics_flag" ) );
      RNOK( pcReadIf->getFlag( bTmp,                                         "DRPM: long_term_reference_flag" ) );
      ROT ( bTmp );
    }
    else
    {
      RNOK( pcReadIf->getFlag( m_bAdaptiveRefPicBufferingModeFlag,           "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
      if( m_bAdaptiveRefPicBufferingModeFlag )
      {
        RNOK( getMmcoBuffer().read( pcReadIf ) );
      }
    }
  }

  if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
  {
    RNOK( pcReadIf->getUvlc( m_uiCabacInitIdc,                               "SH: cabac_init_idc" ) );
  }


  RNOK( pcReadIf->getSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    RNOK( getDeblockingFilterParameter().read( pcReadIf ) );
  }

  //--ICU/ETRI FMO Implementation
  UInt uiSliceGroupChangeCycle;
  if( getPPS().getNumSliceGroupsMinus1()> 0  && getPPS().getSliceGroupMapType() >= 3  &&  getPPS().getSliceGroupMapType() <= 5)
  {
	  UInt pictureSizeInMB = getSPS().getFrameHeightInMbs()*getSPS().getFrameWidthInMbs();

	  RNOK(     pcReadIf->getCode( uiSliceGroupChangeCycle, getLog2MaxSliceGroupChangeCycle(pictureSizeInMB), "SH: slice_group_change_cycle" ) );
 
	   
	  setSliceGroupChangeCycle(uiSliceGroupChangeCycle);
  }


  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::DeblockingFilterParameter::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK( pcWriteIf->writeUvlc( getDisableDeblockingFilterIdc(),  "SH: disable_deblocking_filter_idc" ) );
  ROTRS( 1 == getDisableDeblockingFilterIdc(), Err::m_nOK );

  RNOK( pcWriteIf->writeSvlc( getSliceAlphaC0Offset() >> 1,     "SH: slice_alpha_c0_offset_div2" ) );
  RNOK( pcWriteIf->writeSvlc( getSliceBetaOffset() >> 1,        "SH: slice_beta_offset_div2" ) );
  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::DeblockingFilterParameter::read( HeaderSymbolReadIf* pcReadIf )
{
  RNOK( pcReadIf->getUvlc( m_uiDisableDeblockingFilterIdc,      "SH: disable_deblocking_filter_idc" ) );
  ROT ( m_uiDisableDeblockingFilterIdc > 2 );
  ROTRS( 1 == getDisableDeblockingFilterIdc(), Err::m_nOK );

  Int iTmp;
  RNOK( pcReadIf->getSvlc( iTmp,                                "SH: slice_alpha_c0_offset_div2" ) );
  ROT( (iTmp < -6) || (iTmp >  6) );
  setSliceAlphaC0Offset( iTmp << 1);

  RNOK( pcReadIf->getSvlc( iTmp,                                "SH: slice_beta_offset_div2" ) );
  ROT( (iTmp < -6) || (iTmp >  6) );
  setSliceBetaOffset( iTmp << 1 );

  return Err::m_nOK;
}


//--ICU/ETRI FMO Implementation
ErrVal 
SliceHeaderBase::FMOInit()
{
		
	if(m_pcFMO == NULL)
		m_pcFMO = new FMO();
	else
	{
    //manu.mathew@samsung : memory leak fix
    if( m_pcFMO ) 
    {
      delete m_pcFMO; m_pcFMO = NULL; 
    }
    //--
		m_pcFMO = new FMO();
	}

	const SequenceParameterSet* pcSPS = &(getSPS());
	const PictureParameterSet* pcPPS = &(getPPS());
	m_pcFMO->img_.field_pic_flag = getFieldPicFlag();

	m_pcFMO->pps_.num_slice_groups_minus1 = pcPPS->getNumSliceGroupsMinus1();
	m_pcFMO->pps_.slice_group_map_type = pcPPS->getSliceGroupMapType();
	m_pcFMO->img_.PicHeightInMapUnits = pcSPS->getFrameHeightInMbs(); 
	m_pcFMO->img_.PicWidthInMbs = pcSPS->getFrameWidthInMbs();
	m_pcFMO->img_.PicSizeInMbs = pcSPS->getFrameHeightInMbs()*pcSPS->getFrameWidthInMbs();
	m_pcFMO->img_.slice_group_change_cycle = getSliceGroupChangeCycle();
	m_pcFMO->pps_.num_slice_group_map_units_minus1 = pcPPS->getNumSliceGroupMapUnitsMinus1();	  
	m_pcFMO->pps_.copy_run_length_minus1(pcPPS->getArrayRunLengthMinus1());
	m_pcFMO->pps_.copy_top_left(pcPPS->getArrayTopLeft());
	m_pcFMO->pps_.copy_bottom_right(pcPPS->getArrayBottomRight());
	m_pcFMO->pps_.slice_group_change_direction_flag = pcPPS->getSliceGroupChangeDirection_flag();
	m_pcFMO->pps_.slice_group_change_rate_minus1 = pcPPS->getSliceGroupChangeRateMinus1();
	m_pcFMO->pps_.copy_slice_group_id(pcPPS->getArraySliceGroupId());
	m_pcFMO->sps_.pic_height_in_map_units_minus1 = ( pcSPS->getFrameMbsOnlyFlag()? pcSPS->getFrameHeightInMbs() : pcSPS->getFrameHeightInMbs() >>1 ) -1;
	m_pcFMO->sps_.pic_width_in_mbs_minus1 = pcSPS->getFrameWidthInMbs()-1;
	m_pcFMO->sps_.frame_mbs_only_flag = pcSPS->getFrameMbsOnlyFlag();
	m_pcFMO->sps_.mb_adaptive_frame_field_flag = pcSPS->getMbAdaptiveFrameFieldFlag();
	m_pcFMO->img_.PicHeightInMapUnits = m_pcFMO->sps_.pic_height_in_map_units_minus1+1;//pcSPS->getFrameHeightInMbs(); //by lf
	m_pcFMO->img_.PicWidthInMbs = pcSPS->getFrameWidthInMbs();
	m_pcFMO->img_.PicSizeInMbs = ( pcSPS->getFrameHeightInMbs() >> (UChar)getFieldPicFlag() ) *pcSPS->getFrameWidthInMbs();
	m_pcFMO->img_.slice_group_change_cycle = getSliceGroupChangeCycle();
	m_pcFMO->init(&(m_pcFMO->pps_),&(m_pcFMO->sps_));

	m_pcFMO->StartPicture();

	return Err::m_nOK;
}

Int SliceHeaderBase::getNumMbInSlice()
{  
	Int SliceID =m_pcFMO->getSliceGroupId(getFirstMbInSlice());
	return m_pcFMO->getNumMbInSliceGroup(SliceID);
}



//TMM_WP
ErrVal SliceHeaderBase::PredWeight::setPredWeightsAndFlags( const Int iLumaScale, 
                                                            const Int iChromaScale, 
                                                            const Double *pfWeight, 
                                                            Double fDiscardThr )
{
  const Double *pW = pfWeight;
  const Int iLScale = iLumaScale;
  const Int iCScale = iChromaScale;
  const Int iLumW = (Int)pW[0];
  const Int iCbW = (Int)pW[1];
  const Int iCrW = (Int)pW[2];

  RNOK( init( iLumW, iCbW, iCrW ) );

  setLumaWeightFlag  ( (iLumW != iLScale) );
  setChromaWeightFlag( (iCbW != iCScale) || (iCrW != iCScale) );

  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeight::getPredWeights( Double *afWeight)
{    
    afWeight[0] = (Double) getLumaWeight();
    afWeight[1] = (Double) getChromaWeight(0);
    afWeight[2] = (Double) getChromaWeight(1);
    
    return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::setPredWeightsAndFlags( const Int iLumaScale, const Int iChromaScale, const Double(*pafWeight)[3], Double fDiscardThr )
{
  ROT( 0 == size() );

  for( UInt n = 0; n < size(); n++ )
  {
    RNOK( get(n).setPredWeightsAndFlags( iLumaScale, iChromaScale, pafWeight[n], fDiscardThr ) );
  }
  return Err::m_nOK;
}

ErrVal SliceHeaderBase::copyWeightedPred(PredWeightTable& pcPredWeightTable, UInt uiLumaLogWeightDenom,
                                         UInt uiChromaWeightDenom, ListIdx eListIdx, Bool bDecoder)
{
    m_uiLumaLog2WeightDenom = uiLumaLogWeightDenom;
    m_uiChromaLog2WeightDenom = uiChromaWeightDenom;
    Int iLumaScale = 1 << uiLumaLogWeightDenom;
    Int iChromaScale = 1 << uiChromaWeightDenom;
    Double afWeights[3];
    /* Disable this for now since offsets are not supported for SVC. Enabling this will result in mismatch*/ 
    //Double afOffsets[3];

    if(!bDecoder)
    {
        RNOK( getPredWeightTable(eListIdx).uninit() );
        RNOK( getPredWeightTable(eListIdx).init( getNumRefIdxActive( eListIdx) ) );
    }
        
    for( UInt n = 0; n < pcPredWeightTable.size(); n++ )
    {
        RNOK( pcPredWeightTable.get(n).getPredWeights( afWeights) );
        m_acPredWeightTable[eListIdx].get(n).setPredWeightsAndFlags( iLumaScale, iChromaScale, afWeights, false );

        /* Disable this for now since offsets are not supported for SVC. Enabling this will result in mismatch*/ 
//        RNOK( pcPredWeightTable.get(n).getOffsets( afOffsets) );
//        m_acPredWeightTable[eListIdx].get(n).setOffsets(afOffsets);
    }

    return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeight::setOffsets( const Double *pfOffsets)
{
  const Double *pW = pfOffsets;

  const Int iLumO = (Int)pW[0];
  const Int iCbO = (Int)pW[1];
  const Int iCrO = (Int)pW[2];

  setLumaWeightFlag  ( (iLumO != 0) );
  setChromaWeightFlag( (iCbO != 0) || (iCrO != 0) );

  RNOK( initOffsets( iLumO, iCbO, iCrO ) );

  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeight::getOffsets( Double *afOffset)
{    
    afOffset[0] = (Double) getLumaOffset();
    afOffset[1] = (Double) getChromaOffset(0);
    afOffset[2] = (Double) getChromaOffset(1);
    
    return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::setOffsets(  const Double(*pafOffsets)[3] )
{
  ROT( 0 == size() );

  for( UInt n = 0; n < size(); n++ )
  {
    RNOK( get(n).setOffsets( pafOffsets[n] ) );
  }
  return Err::m_nOK;
}
//TMM_WP


ErrVal
SliceHeaderBase::xReadMVCCompatible( HeaderSymbolReadIf* pcReadIf )
{
  Bool  bTmp;
 
  RNOK(     pcReadIf->getCode( m_uiFrameNum,
                               getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
  	if(!getSPS().getFrameMbsOnlyFlag())
	{
		RNOK(   pcReadIf->getFlag( m_bFieldPicFlag,                                 "SH: field_pic_flag" ) );
		if(m_bFieldPicFlag)
			RNOK(   pcReadIf->getFlag( m_bBottomFieldFlag,                                 "SH: bottom_field_flag" ) );
	}
//  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR) // JVT-W035 
  if(!getNonIDRFlag())//lufeng
  {
    RNOK(   pcReadIf->getUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( getSPS().getPicOrderCntType() == 0 )
  {
  RNOK(     pcReadIf->getCode( m_uiPicOrderCntLsb,
                               getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
  if( getPPS().getPicOrderPresentFlag() && ! m_bFieldPicFlag )
    {
      RNOK( pcReadIf->getSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }
  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
	if( getPPS().getPicOrderPresentFlag() &&  ! m_bFieldPicFlag )
  {
      RNOK( pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
  //JVT-Q054 Red. Picture {
  if ( getPPS().getRedundantPicCntPresentFlag())
  {
    RNOK( pcReadIf->getUvlc( m_uiRedundantPicCnt,                            "SH: redundant_pic_cnt") );
  }
  //JVT-Q054 Red. Picture }
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcReadIf->getFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( pcReadIf->getFlag( m_bNumRefIdxActiveOverrideFlag,                 "SH: num_ref_idx_active_override_flag" ) );
    if( m_bNumRefIdxActiveOverrideFlag )
    {
      RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_0],                 "SH: num_ref_idx_l0_active_minus1" ) );
      m_auiNumRefIdxActive[LIST_0]++;
      if( m_eSliceType == B_SLICE )
      {
        RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_1],               "SH: num_ref_idx_l1_active_minus1" ) );
        m_auiNumRefIdxActive[LIST_1]++;
      }
    }
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_0 ).read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
  }
  if( m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_1 ).read( pcReadIf, getNumRefIdxActive( LIST_1 ) ) );
  }

  RNOK( m_acPredWeightTable[LIST_0].init( 64 ) );
  RNOK( m_acPredWeightTable[LIST_1].init( 64 ) );

  if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
      ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
  {
    RNOK( pcReadIf->getUvlc( m_uiLumaLog2WeightDenom,                     "PWT: luma_log_weight_denom" ) );
    RNOK( pcReadIf->getUvlc( m_uiChromaLog2WeightDenom,                   "PWT: chroma_log_weight_denom" ) );
    ROTR( m_uiLumaLog2WeightDenom   > 7, Err::m_nInvalidParameter );
    ROTR( m_uiChromaLog2WeightDenom > 7, Err::m_nInvalidParameter );

    RNOK( m_acPredWeightTable[LIST_0].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
    RNOK( m_acPredWeightTable[LIST_0].read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
    if( m_eSliceType == B_SLICE )
    {
      RNOK( m_acPredWeightTable[LIST_1].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
      RNOK( m_acPredWeightTable[LIST_1].read( pcReadIf, getNumRefIdxActive( LIST_1) ) );
    }
  }

  if( getNalRefIdc() )
  {
    if( isIdrNalUnit() )
    {
      RNOK( pcReadIf->getFlag( m_bNoOutputOfPriorPicsFlag,                   "DRPM: no_output_of_prior_pics_flag" ) );
      RNOK( pcReadIf->getFlag( bTmp,                                         "DRPM: long_term_reference_flag" ) );
      ROT ( bTmp );
    }
    else
    {
      RNOK( pcReadIf->getFlag( m_bAdaptiveRefPicBufferingModeFlag,           "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
      if( m_bAdaptiveRefPicBufferingModeFlag )
      {
        RNOK( getMmcoBuffer().read( pcReadIf ) );
      }
    }
  }

  if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
  {
    RNOK( pcReadIf->getUvlc( m_uiCabacInitIdc,                               "SH: cabac_init_idc" ) );
  }


  RNOK( pcReadIf->getSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    RNOK( getDeblockingFilterParameter().read( pcReadIf ) );
  }

  //--ICU/ETRI FMO Implementation
  UInt uiSliceGroupChangeCycle;
  if( getPPS().getNumSliceGroupsMinus1()> 0  && getPPS().getSliceGroupMapType() >= 3  &&  getPPS().getSliceGroupMapType() <= 5)
  {
	  UInt pictureSizeInMB = getSPS().getFrameHeightInMbs()*getSPS().getFrameWidthInMbs();

	  RNOK(     pcReadIf->getCode( uiSliceGroupChangeCycle, getLog2MaxSliceGroupChangeCycle(pictureSizeInMB), "SH: slice_group_change_cycle" ) );
 
	   
	  setSliceGroupChangeCycle(uiSliceGroupChangeCycle);
  }


  return Err::m_nOK;
}




H264AVC_NAMESPACE_END


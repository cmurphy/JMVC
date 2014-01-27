#include <cstdio>
#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/SequenceParameterSet.h"
#include "H264AVCCommonLib/TraceFile.h"
#include <cmath>


H264AVC_NAMESPACE_BEGIN

const SequenceParameterSet::LevelLimit SequenceParameterSet::m_aLevelLimit[52] =
{
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //0
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //1
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //2
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //3
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //4
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //5
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //6
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //7
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //8
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //9
  { 1,   1485,    99,    297*1024,     64,    175,  256, 2, MSYS_UINT_MAX },     //10
  { 1,   3000,   396,    675*1024,    192,    500,  512, 2, MSYS_UINT_MAX },     //11
  { 1,   6000,   396,   1782*1024,    384,   1000,  512, 2, MSYS_UINT_MAX },     //12
  { 1,  11880,   396,   1782*1024,    768,   2000,  512, 2, MSYS_UINT_MAX },     //13
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //14
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //15
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //16
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //17
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //18
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //19
  { 1,  11880,   396,   1782*1024,   2000,   2000,  512, 2, MSYS_UINT_MAX },     //20
  { 1,  19800,   792,   3564*1024,   4000,   4000, 1024, 2, MSYS_UINT_MAX },     //21
  { 1,  20250,  1620,   6075*1024,   4000,   4000, 1024, 2, MSYS_UINT_MAX },     //22
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //23
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //24
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //25
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //26
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //27
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //28
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //29
  { 1,  40500,  1620,   6075*1024,  10000,  10000, 1024, 2, 32 },                //30
  { 1, 108000,  3600,  13500*1024,  14000,  14000, 2048, 4, 16 },                //31
  //{ 1, 216000,  5120,  15360*1024,  20000,  20000, 2048, 4, 16 },                //32
  { 1, 216000,  5120,  13500*1024,  20000,  20000, 2048, 4, 16 },                //32
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //33
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //34
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //35
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //36
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //37
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //38
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //39
  { 1, 245760,  8192,  24576*1024,  20000,  25000, 2048, 4, 16 },                //40
  { 1, 245760,  8192,  24576*1024,  50000,  62500, 2048, 2, 16 },                //41
  //{ 1, 491520,  8192,  24576*1024,  50000,  62500, 2048, 2, 16 },                //42
  { 1, 491520,  8192,  26112*1024,  50000,  62500, 2048, 2, 16 },                //42
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //43
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //44
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //45
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //46
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //47
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //48
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //49
  //{ 1, 589824, 22080,  82620*1024, 135000, 135000, 2048, 2, 16 },                //50
  { 1, 589824, 22080,  75300*1024, 135000, 135000, 2048, 2, 16 },                //50
  { 1, 983040, 36864, 138240*1024, 240000, 240000, 2048, 2, 16 }                 //51
};






SequenceParameterSet::SequenceParameterSet  ()
: m_bInitDone                               ( false )
, m_eNalUnitType                            ( NAL_UNIT_EXTERNAL )
, m_uiLayerId                               ( 0 )
, m_eProfileIdc                             ( SCALABLE_PROFILE )
, m_bConstrainedSet0Flag                    ( false )
, m_bConstrainedSet1Flag                    ( false )
, m_bConstrainedSet2Flag                    ( false )
, m_bConstrainedSet3Flag                    ( false )
, m_bConstrainedSet4Flag                    ( false )
, m_bConstrainedSet5Flag                    ( false )
, m_uiLevelIdc                              ( 0 )
, m_uiSeqParameterSetId                     ( MSYS_UINT_MAX )
//, m_bNalUnitExtFlag                         ( true  )  //JVT-S036 lsj
, m_bSeqScalingMatrixPresentFlag            ( false )
, m_uiLog2MaxFrameNum                       ( 0 )
, m_uiPicOrderCntType                       ( 0 )
, m_uiLog2MaxPicOrderCntLsb                 ( 4 )
, m_bDeltaPicOrderAlwaysZeroFlag            ( false )
, m_iOffsetForNonRefPic                     ( 0 )
, m_iOffsetForTopToBottomField              ( 0 )
, m_uiNumRefFramesInPicOrderCntCycle        ( 0 )
, m_uiNumRefFrames                          ( 0 )
, m_bRequiredFrameNumUpdateBehaviourFlag    ( false )
, m_uiFrameWidthInMbs                       ( 0 )
, m_uiFrameHeightInMbs                      ( 0 )
, m_bDirect8x8InferenceFlag                 ( false )
,m_uiExtendedSpatialScalability             ( ESS_NONE ) // TMM_ESS
,m_uiChromaPhaseXPlus1                      ( 0 ) // TMM_ESS
,m_uiChromaPhaseYPlus1                      ( 1 )// TMM_ESS
, m_bFGSCodingMode                          ( false )
, m_uiGroupingSize                          ( 1 )
, m_bFrameMbsOnlyFlag                       ( true )
, m_bMbAdaptiveFrameFieldFlag               ( false ) 
{
	m_auiNumRefIdxUpdateActiveDefault[LIST_0]=1;// VW
	m_auiNumRefIdxUpdateActiveDefault[LIST_1]=1;// VW

  ::memset( m_aiOffsetForRefFrame, 0x00, 64*sizeof(Int) );
  ::memset( m_uiPosVect,           0x00, 16*sizeof(UInt));
  m_frame_crop_offset[0] =0;
  m_frame_crop_offset[1] =0;
  m_frame_crop_offset[2] =0;
  m_frame_crop_offset[3] =0;
}

SequenceParameterSet::~SequenceParameterSet()
{
}



ErrVal
SequenceParameterSet::create( SequenceParameterSet*& rpcSPS )
{
  rpcSPS = new SequenceParameterSet;
  ROT( NULL == rpcSPS);

  return Err::m_nOK;
}


ErrVal
SequenceParameterSet::destroy()
{
// previous implementation has memory leak! // ying
	if ( m_eProfileIdc == MULTI_VIEW_PROFILE || m_eProfileIdc == STEREO_HIGH_PROFILE ) // 
	{
		SpsMVC->releaseViewSPSMemory_num_refs_for_lists();
		if (SpsMVC->getNumViewMinus1()>0) // this is necessary due to JVT-Y061
			SpsMVC->releaseViewSPSMemory_ref_for_lists();
		SpsMVC->releaseViewSPSMemory_num_level_related_memory_2D();
		SpsMVC->releaseViewSPSMemory_num_level_related_memory_3D();
		SpsMVC->releaseViewSPSMemory_num_level_related_memory();
		if (SpsMVC!=NULL)
			delete SpsMVC; 
	}

  delete this;
  return Err::m_nOK;
}

SequenceParameterSet& SequenceParameterSet::operator = ( const SequenceParameterSet& rcSPS )
{
    m_eNalUnitType                      = rcSPS.m_eNalUnitType;
    m_uiLayerId                         = rcSPS.m_uiLayerId;
    m_eProfileIdc                       = rcSPS.m_eProfileIdc;
    m_bConstrainedSet0Flag              = rcSPS.m_bConstrainedSet0Flag;
    m_bConstrainedSet1Flag              = rcSPS.m_bConstrainedSet1Flag;
    m_bConstrainedSet2Flag              = rcSPS.m_bConstrainedSet2Flag;
    m_bConstrainedSet3Flag              = rcSPS.m_bConstrainedSet3Flag;
    m_uiLevelIdc                        = rcSPS.m_uiLevelIdc;
    m_uiSeqParameterSetId               = rcSPS.m_uiSeqParameterSetId;
    m_uiLog2MaxFrameNum                 = rcSPS.m_uiLog2MaxFrameNum;
    m_uiPicOrderCntType                 = rcSPS.m_uiPicOrderCntType;
    m_uiLog2MaxPicOrderCntLsb           = rcSPS.m_uiLog2MaxPicOrderCntLsb;
    m_bDeltaPicOrderAlwaysZeroFlag      = rcSPS.m_bDeltaPicOrderAlwaysZeroFlag;
    m_iOffsetForNonRefPic               = rcSPS.m_iOffsetForNonRefPic;
    m_iOffsetForTopToBottomField        = rcSPS.m_iOffsetForTopToBottomField;
    m_uiNumRefFramesInPicOrderCntCycle  = rcSPS.m_uiNumRefFramesInPicOrderCntCycle;
//    m_piOffsetForRefFrame               = rcSPS.m_piOffsetForRefFrame;
    m_uiNumRefFrames                    = rcSPS.m_uiNumRefFrames;
//    m_bGapsInFrameNumValueAllowedFlag   = rcSPS.m_bGapsInFrameNumValueAllowedFlag;
    m_uiFrameWidthInMbs                 = rcSPS.m_uiFrameWidthInMbs;
    m_uiFrameHeightInMbs                = rcSPS.m_uiFrameHeightInMbs;
    m_bFrameMbsOnlyFlag                 = rcSPS.m_bFrameMbsOnlyFlag;
    m_bMbAdaptiveFrameFieldFlag         = rcSPS.m_bMbAdaptiveFrameFieldFlag;
    m_bDirect8x8InferenceFlag           = rcSPS.m_bDirect8x8InferenceFlag;
//    m_bNalUnitExtFlag                   = rcSPS.m_bNalUnitExtFlag;
//    m_uiNumSimplePriIdVals              = rcSPS.m_uiNumSimplePriIdVals;
//#if MULTIPLE_LOOP_DECODING
//    m_bAlwaysDecodeBaseLayer            = rcSPS.m_bAlwaysDecodeBaseLayer;
//#endif
    m_uiExtendedSpatialScalability      = rcSPS.m_uiExtendedSpatialScalability;
    m_uiChromaPhaseXPlus1               = rcSPS.m_uiChromaPhaseXPlus1;
    m_uiChromaPhaseYPlus1               = rcSPS.m_uiChromaPhaseYPlus1;
    m_iScaledBaseLeftOffset             = rcSPS.m_iScaledBaseLeftOffset;
    m_iScaledBaseTopOffset              = rcSPS.m_iScaledBaseTopOffset;
    m_iScaledBaseRightOffset            = rcSPS.m_iScaledBaseRightOffset;
    m_iScaledBaseBottomOffset           = rcSPS.m_iScaledBaseBottomOffset;
    m_bInitDone                         = rcSPS.m_bInitDone;

    return *this;
}

UInt
SequenceParameterSet::getMaxDPBSize(UInt mvcScaleFactor) const
{
  const LevelLimit* pcLevelLimit = 0;
  UInt              uiFrameSize = 384*getMbInFrame();
  ANOK( xGetLevelLimit( pcLevelLimit, getLevelIdc() ) );
  return mvcScaleFactor*pcLevelLimit->uiMaxDPBSizeX2 / ( 2*uiFrameSize );
}


ErrVal
SequenceParameterSet::xGetLevelLimit( const LevelLimit*& rpcLevelLimit, Int iLevelIdc )
{
  ROT ( iLevelIdc > 51 )
  rpcLevelLimit = &m_aLevelLimit[iLevelIdc];
  ROFS( rpcLevelLimit->bValid )
  return Err::m_nOK;
}


UInt
SequenceParameterSet::getLevelIdc( UInt uiMbY, UInt uiMbX, UInt uiOutFreq, UInt uiMvRange, UInt uiNumRefPic, int Num_Views  )
{

 
  UInt mvcScaleFactor = Num_Views > 1 ? 2 : 1;	
  UInt uiFrameSize = uiMbY * uiMbX;
  UInt uiMbPerSec  = uiFrameSize * uiOutFreq * Num_Views;
  UInt uiDPBSizeX2 = (uiFrameSize*16*16*3/2) * uiNumRefPic * 2;
  UInt uiMaxDPBSizeX2_B = (uiFrameSize*16*16*3/2) * (max(1,(UInt)ceil((double)log((double)Num_Views)/log(2.)))*16) * 2;

  for( Int n = 0; n < 52; n++ )
  {
    const LevelLimit* pcLevelLimit;
    
    if( Err::m_nOK == xGetLevelLimit( pcLevelLimit, n ) )
    {
      UInt  uiMbPerLine  = (UInt)sqrt( (Double) pcLevelLimit->uiMaxFrameSize * 8 );
      if( ( uiMbPerLine                   >= uiMbX        ) &&
          ( uiMbPerLine                   >= uiMbY        ) &&
          ( mvcScaleFactor * pcLevelLimit->uiMaxMbPerSec   >= uiMbPerSec   ) &&
          ( pcLevelLimit->uiMaxFrameSize  >= uiFrameSize  ) &&         
		  (	min (mvcScaleFactor*pcLevelLimit->uiMaxDPBSizeX2, uiMaxDPBSizeX2_B) >= uiDPBSizeX2	) &&
          ( pcLevelLimit->uiMaxVMvRange   >= uiMvRange    )    )
      {
        return n;
      }
    }
  }
  return MSYS_UINT_MAX;
}


ErrVal
SequenceParameterSet::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  //===== NAL unit header =====
  ETRACE_DECLARE( Bool m_bTraceEnable = true );
	g_nLayer = m_uiLayerId;
  ETRACE_LAYER(m_uiLayerId);
  ETRACE_VIEWID(m_uiCurrentViewId);
  ETRACE_HEADER( "SEQUENCE PARAMETER SET" );
  RNOK  ( pcWriteIf->writeFlag( 0,                                        "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( 3, 2,                                     "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalUnitType, 5,                        "NALU HEADER: nal_unit_type" ) );

  //===== Sequence parameter set =====
  RNOK  ( pcWriteIf->writeCode( getProfileIdc(),                  8,      "SPS: profile_idc" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet0Flag,                   "SPS: constrained_set0_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet1Flag,                   "SPS: constrained_set1_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet2Flag,                   "SPS: constrained_set2_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet3Flag,                   "SPS: constrained_set3_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet4Flag,                   "SPS: constrained_set4_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet5Flag,                   "SPS: constrained_set5_flag" ) );
  RNOK  ( pcWriteIf->writeCode( 0,                                2,      "SPS: reserved_zero_2bits" ) );
  RNOK  ( pcWriteIf->writeCode( getLevelIdc(),                    8,      "SPS: level_idc" ) );
  RNOK  ( pcWriteIf->writeUvlc( getSeqParameterSetId(),                   "SPS: seq_parameter_set_id" ) );
  

  

  //--- fidelity range extension syntax ---
  RNOK  ( xWriteFrext( pcWriteIf ) );
  
  UInt    uiTmp = getLog2MaxFrameNum();
  ROF   ( uiTmp >= 4 );
  RNOK  ( pcWriteIf->writeUvlc( uiTmp - 4,                                "SPS: log2_max_frame_num_minus_4" ) );
  RNOK  ( pcWriteIf->writeUvlc( getPicOrderCntType(),                     "SPS: pic_order_cnt_type" ) );
  if( getPicOrderCntType() == 0 )
  {
  RNOK  ( pcWriteIf->writeUvlc( getLog2MaxPicOrderCntLsb() - 4,           "SPS: log2_max_pic_order_cnt_lsb_minus4" ) );
  }
  else if( getPicOrderCntType() == 1 )
  {
    RNOK( pcWriteIf->writeFlag( getDeltaPicOrderAlwaysZeroFlag(),         "SPS: delta_pic_order_always_zero_flag" ) );
    RNOK( pcWriteIf->writeSvlc( getOffsetForNonRefPic(),                  "SPS: offset_for_non_ref_pic" ) );
    RNOK( pcWriteIf->writeSvlc( getOffsetForTopToBottomField(),           "SPS: offset_for_top_to_bottom_field" ) );
    RNOK( pcWriteIf->writeUvlc( getNumRefFramesInPicOrderCntCycle(),      "SPS: num_ref_frames_in_pic_order_cnt_cycle" ) );
    for( UInt uiIndex = 0; uiIndex < getNumRefFramesInPicOrderCntCycle(); uiIndex++ )
    {
      RNOK( pcWriteIf->writeSvlc( getOffsetForRefFrame( uiIndex ),        "SPS: offset_for_ref_frame" ) );
    }
  }
  RNOK  ( pcWriteIf->writeUvlc( getNumRefFrames(),                        "SPS: num_ref_frames" ) );
  RNOK  ( pcWriteIf->writeFlag( getRequiredFrameNumUpdateBehaviourFlag(), "SPS: required_frame_num_update_behaviour_flag" ) );

//wirte SequenceParameterSet
  if( !getFrameMbsOnlyFlag() )
  {
      RNOK  ( pcWriteIf->writeUvlc( getFrameWidthInMbs  () - 1,               "SPS: pic_width_in_mbs_minus_1" ) );
      RNOK  ( pcWriteIf->writeUvlc( (getFrameHeightInMbs()-1) >> 1,            "SPS: pic_height_in_map_units_minus_1" ) );
      RNOK  ( pcWriteIf->writeFlag( false,                                    "SPS: frame_mbs_only_flag" ) );
      RNOK  ( pcWriteIf->writeFlag( getMbAdaptiveFrameFieldFlag(),                                     "SPS: mb_adaptive_frame_field_flag"));
  }
  else
  {
      RNOK  ( pcWriteIf->writeUvlc( getFrameWidthInMbs  () - 1,               "SPS: pic_width_in_mbs_minus_1" ) );
      RNOK  ( pcWriteIf->writeUvlc( getFrameHeightInMbs () - 1,               "SPS: pic_height_in_map_units_minus_1" ) );
      RNOK  ( pcWriteIf->writeFlag( true,                                     "SPS: frame_mbs_only_flag" ) );
  }


  RNOK  ( pcWriteIf->writeFlag( getDirect8x8InferenceFlag(),              "SPS: direct_8x8_inference_flag" ) );

  if( m_frame_crop_offset[0] || m_frame_crop_offset[1] || m_frame_crop_offset[2] || m_frame_crop_offset[3] )
  {
    RNOK( pcWriteIf->writeFlag( true,                                     "SPS: frame_cropping_flag"      ) );
    RNOK( pcWriteIf->writeUvlc( m_frame_crop_offset[0],                  "SPS: frame_crop_left_offset"   ) );
    RNOK( pcWriteIf->writeUvlc( m_frame_crop_offset[1],                 "SPS: frame_crop_right_offset"  ) );
    RNOK( pcWriteIf->writeUvlc( m_frame_crop_offset[2],                   "SPS: frame_crop_top_offset"    ) );
    RNOK( pcWriteIf->writeUvlc( m_frame_crop_offset[3],                "SPS: frame_crop_bottom_offset" ) );
  }
  else
  {
    RNOK( pcWriteIf->writeFlag( false,                                    "SPS: frame_cropping_flag" ) );
  }

  RNOK  ( pcWriteIf->writeFlag( false,                                  "SPS: vui_parameters_present_flag" ) );

  if (m_eNalUnitType == NAL_UNIT_SUBSET_SPS )
  {
	if( m_eProfileIdc == MULTI_VIEW_PROFILE || m_eProfileIdc == STEREO_HIGH_PROFILE) 
	{

		RNOK  ( pcWriteIf->writeFlag(true,                      "SUBSET SPS: bit_equal_to_one" ) );

		// seq_parameter_set_mvc_extension()

		RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_num_views_minus_1,                   "SPS: num_views_minus_1" ) ); // ue(v)
		
		int i,j,k;
		//JVT-V054
		printf("ViewCodingOrder: ");
		for (i=0;i<= SpsMVC->m_num_views_minus_1; i++)
		{
			printf("%d ", SpsMVC->m_uiViewCodingOrder[i]);
			RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_uiViewCodingOrder[i], "SPS: view_id[i]")); //ue(v)
		}	  
		printf ("\n");

		for (i=1;i<= SpsMVC->m_num_views_minus_1; i++)  //JVT-Y061
		{
			//vcOrder = SpsMVC->m_uiViewCodingOrder[i];

			RNOK  ( pcWriteIf->writeUvlc( (UInt)SpsMVC->m_num_anchor_refs_list0[i],                   "SPS: num_anchor_refs_l0[i]" ) ); // ue(v)
			for (j=0; j<SpsMVC->m_num_anchor_refs_list0[i]; j++)
				RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_anchor_ref_list0[i][j],      "SPS: anchor_ref_l0[i][j]" ) ); // ue(v)
					
			RNOK  ( pcWriteIf->writeUvlc( (UInt)SpsMVC->m_num_anchor_refs_list1[i],                   "SPS: num_anchor_refs_l1[i]" ) ); // ue(v)
			for (j=0; j<SpsMVC->m_num_anchor_refs_list1[i]; j++)
				RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_anchor_ref_list1[i][j],      "SPS: anchor_ref_l1[i][j]" ) ); // ue(v)
		}

		for (i=1;i<= SpsMVC->m_num_views_minus_1; i++)  //JVT-Y061
		{
			//vcOrder = SpsMVC->m_uiViewCodingOrder[i];
			RNOK  ( pcWriteIf->writeUvlc( (UInt)SpsMVC->m_num_non_anchor_refs_list0[i],                   "SPS: num_non_anchor_refs_l0[i]" ) ); // ue(v)
			for (j=0; j<SpsMVC->m_num_non_anchor_refs_list0[i]; j++)
				RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_non_anchor_ref_list0[i][j],      "SPS: non_anchor_ref_l0[i][j]" ) ); // ue(v)

			RNOK  ( pcWriteIf->writeUvlc( (UInt)SpsMVC->m_num_non_anchor_refs_list1[i],                   "SPS: num_non_anchor_refs_l1[i]" ) ); // ue(v)
			for (j=0; j<SpsMVC->m_num_non_anchor_refs_list1[i]; j++)
				RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_non_anchor_ref_list1[i][j],      "SPS: non_anchor_ref_l1[i][j]" ) ); // ue(v)

		}	

		RNOK  ( pcWriteIf->writeUvlc( (UInt &)SpsMVC->m_num_level_values_signalled_minus1,                   "SPS: num_level_values_signalled_minus1" ) ); // ue(v)

		for (i=0;i<= SpsMVC->m_num_level_values_signalled_minus1; i++)  
		{
			RNOK  ( pcWriteIf->writeCode( SpsMVC->m_ui_level_idc[i] ,                               8,      "SPS: level_idc[i]" ) );
			RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_ui_num_applicable_ops_minus1[i],                   "SPS: num_applicable_ops_minus1[i]" ) ); // ue(v)
			for (j=0; j<=(int)SpsMVC->m_ui_num_applicable_ops_minus1[i];j++)
			{
				RNOK  ( pcWriteIf->writeCode( SpsMVC->m_ui_applicable_op_temporal_id[i][j] ,                               3,      "SPS: applicable_op_temporal_id[i][j]" ) );
				RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_ui_applicable_op_num_target_views_minus1[i][j],                   "SPS: applicable_op_num_target_views_minus1[i][j]" ) ); // ue(v)
				for (k=0; k<= (int)SpsMVC->m_ui_applicable_op_num_target_views_minus1[i][j];k++)
					RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_ui_applicable_op_target_view_id[i][j][k],                    "SPS: applicable_op_num_target_view_id[i][j][k]" ) ); // ue(v)
				
				RNOK  ( pcWriteIf->writeUvlc( SpsMVC->m_ui_applicable_op_num_views_minus1[i][j],                   "SPS: num_applicable_op_num_views_minus1[i][j]" ) ); // ue(v)			
			}		
		}

	}
	RNOK  ( pcWriteIf->writeFlag(false,                      "SUBSET SPS: mvc_vui_parameters_present_flag" ) );	
	RNOK  ( pcWriteIf->writeFlag( false,                      "SUBSET SPS: Additional_extension2_flag" ) );
	
  }

  return Err::m_nOK;
}


ErrVal
SequenceParameterSet::read( HeaderSymbolReadIf* pcReadIf,
                            NalUnitType         eNalUnitType )
{
  m_uiExtendedSpatialScalability = ESS_NONE;

  //===== NAL unit header =====
  setNalUnitType    ( eNalUnitType );

  Bool  bTmp;
  UInt  uiTmp;

  //===== Sequence parameter set =====
  RNOK  ( pcReadIf->getCode( uiTmp,                               8,      "SPS: profile_idc" ) );
  m_eProfileIdc  = Profile ( uiTmp );
  ROT   ( m_eProfileIdc != BASELINE_PROFILE &&
          m_eProfileIdc != MAIN_PROFILE  &&
          m_eProfileIdc != EXTENDED_PROFILE  &&
          m_eProfileIdc != HIGH_PROFILE  &&
          m_eProfileIdc != MULTI_VIEW_PROFILE &&
		  m_eProfileIdc != STEREO_HIGH_PROFILE &&
          m_eProfileIdc != SCALABLE_PROFILE );

  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet0Flag,                      "SPS: constrained_set0_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet1Flag,                      "SPS: constrained_set1_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet2Flag,                      "SPS: constrained_set2_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet3Flag,                      "SPS: constrained_set3_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet4Flag,                      "SPS: constrained_set4_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet5Flag,                      "SPS: constrained_set5_flag" ) );
  RNOK  ( pcReadIf->getCode( uiTmp,                               2,      "SPS: reserved_zero_2bits" ) );
  ROT   ( uiTmp );
  RNOK  ( pcReadIf->getCode( m_uiLevelIdc,                        8,      "SPS: level_idc" ) );
  RNOK  ( pcReadIf->getUvlc( m_uiSeqParameterSetId,                       "SPS: seq_parameter_set_id" ) );


  


  //--- fidelity range extension syntax ---  
  RNOK  ( xReadFrext( pcReadIf ) );

  RNOK  ( pcReadIf->getUvlc( uiTmp,                                       "SPS: log2_max_frame_num_minus_4" ) );
  ROT   ( uiTmp > 12 );
  setLog2MaxFrameNum( uiTmp + 4 );

  RNOK  ( pcReadIf->getUvlc( m_uiPicOrderCntType,                         "SPS: pic_order_cnt_type" ) );
  if( m_uiPicOrderCntType == 0 )
  {
      RNOK( pcReadIf->getUvlc( uiTmp, "SPS: log2_max_pic_order_cnt_lsb_minus4" ));
      setLog2MaxPicOrderCntLsb( 4+uiTmp );
  }
  else if( getPicOrderCntType() == 1 )
  {
    RNOK( pcReadIf->getFlag( m_bDeltaPicOrderAlwaysZeroFlag,              "SPS: delta_pic_order_always_zero_flag" ) );
    RNOK( pcReadIf->getSvlc( m_iOffsetForNonRefPic,                       "SPS: offset_for_non_ref_pic" ) );
    RNOK( pcReadIf->getSvlc( m_iOffsetForTopToBottomField,                "SPS: offset_for_top_to_bottom_field" ) );
    RNOK( pcReadIf->getUvlc( m_uiNumRefFramesInPicOrderCntCycle,          "SPS: num_ref_frames_in_pic_order_cnt_cycle" ) );
      RNOK( initOffsetForRefFrame( m_uiNumRefFramesInPicOrderCntCycle ) );
   
            for( UInt i = 0; i < m_uiNumRefFramesInPicOrderCntCycle; i++)
      {
          Int  iTmp;
          RNOK( pcReadIf->getSvlc( iTmp, "SPS: offset_for_ref_frame" ) );
          setOffsetForRefFrame( i, iTmp );
      }
  }

  RNOK( pcReadIf->getUvlc( m_uiNumRefFrames,                              "SPS: num_ref_frames" ) );
  RNOK( pcReadIf->getFlag( m_bRequiredFrameNumUpdateBehaviourFlag,        "SPS: required_frame_num_update_behaviour_flag" ) );
  RNOK( pcReadIf->getUvlc( uiTmp,                                         "SPS: pic_width_in_mbs_minus_1" ) );
  setFrameWidthInMbs ( 1 + uiTmp );
  RNOK( pcReadIf->getUvlc( uiTmp,                                         "SPS: pic_height_in_map_units_minus_1" ) );

  RNOK( pcReadIf->getFlag( m_bFrameMbsOnlyFlag,                           "SPS: frame_mbs_only_flag" ) );
  if( getFrameMbsOnlyFlag() )
  {
      setFrameHeightInMbs( uiTmp+1 );
      setMbAdaptiveFrameFieldFlag( false );
  }
  else
  {
      setFrameHeightInMbs( (uiTmp+1)<<1 );
      RNOK( pcReadIf->getFlag( m_bMbAdaptiveFrameFieldFlag,                 "SPS: mb_adaptive_frame_field_flag"));
  }

  RNOK( pcReadIf->getFlag( m_bDirect8x8InferenceFlag,                     "SPS: direct_8x8_inference_flag" ) );
  RNOK( pcReadIf->getFlag( bTmp,                                          "SPS: frame_cropping_flag" ) );
//  ROT ( bTmp ); // always set to 0 in jmvc

  if(bTmp)//lufeng: support frame cropping
  {
	  RNOK( pcReadIf->getUvlc( m_frame_crop_offset[0],                                         "SPS: frame_crop_left_offset" ) );
	  RNOK( pcReadIf->getUvlc( m_frame_crop_offset[1],                                         "SPS: frame_crop_right_offset" ) );
      RNOK( pcReadIf->getUvlc( m_frame_crop_offset[2],                                         "SPS: frame_crop_top_offset" ) );
	  RNOK( pcReadIf->getUvlc( m_frame_crop_offset[3],                                         "SPS: frame_crop_bottom_offset" ) );
  }

  RNOK( pcReadIf->getFlag( bTmp,                                          "SPS: vui_parameters_present_flag" ) );
  ROT ( bTmp ); // always set to 0 in jmvc

  if (eNalUnitType == NAL_UNIT_SUBSET_SPS )
  {
	SpsMVC = NULL; 
	if( m_eProfileIdc == MULTI_VIEW_PROFILE || m_eProfileIdc == STEREO_HIGH_PROFILE) 
	{
		RNOK  ( pcReadIf->getFlag( bTmp,                      "SUBSET SPS: bit_equal_to_one" ) );
		ROF ( bTmp ); // always shoule be set to 1 

		SpsMVC = new SpsMvcExtension;	
		int i,j,k,vcOrder;
		
		// seq_parameter_set_mvc_extension()
		RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_num_views_minus_1,                   "SPS: num_views_minus_1" ) ); // ue(v)
		SpsMVC->initViewSPSMemory_num_refs_for_lists(SpsMVC->getNumViewMinus1());

		//JVT-V054
		SpsMVC->setInitDone(false);
		printf("ViewCodingOrder: ");
		for (i=0;i<= SpsMVC->m_num_views_minus_1; i++)
		{
		RNOK  ( pcReadIf->getUvlc( SpsMVC->m_uiViewCodingOrder[i], "SPS: view_id[i]")); //ue(v)
		printf("%d ", SpsMVC->m_uiViewCodingOrder[i]);
		}	  
		printf ("\n");

		for (i=1;i<= SpsMVC->m_num_views_minus_1; i++)  //JVT-Y061
		{
		vcOrder = SpsMVC->m_uiViewCodingOrder[i];

		RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_num_anchor_refs_list0[i],                   "SPS: num_anchor_refs_l0[i]" ) ); // ue(v)
		SpsMVC->initViewSPSMemory_ref_for_lists(SpsMVC->getNumViewMinus1(),vcOrder,0,0);
		for (j=0; j<SpsMVC->m_num_anchor_refs_list0[i]; j++)
			RNOK  ( pcReadIf->getUvlc( SpsMVC->m_anchor_ref_list0[i][j],      "SPS: anchor_ref_l0[i][j]" ) ); // ue(v)
				
		RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_num_anchor_refs_list1[i],                   "SPS: num_anchor_refs_l1[i]" ) ); // ue(v)
		SpsMVC->initViewSPSMemory_ref_for_lists(SpsMVC->getNumViewMinus1(),vcOrder,1,0);
		for (j=0; j<SpsMVC->m_num_anchor_refs_list1[i]; j++)
			RNOK  ( pcReadIf->getUvlc( SpsMVC->m_anchor_ref_list1[i][j],      "SPS: anchor_ref_l1[i][j]" ) ); // ue(v)
		}

		for (i=1;i<= SpsMVC->m_num_views_minus_1; i++)  //JVT-Y061
		{
		vcOrder = SpsMVC->m_uiViewCodingOrder[i];
		RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_num_non_anchor_refs_list0[i],                   "SPS: num_non_anchor_refs_l0[i]" ) ); // ue(v)
		SpsMVC->initViewSPSMemory_ref_for_lists(SpsMVC->getNumViewMinus1(),vcOrder,0,1);
		for (j=0; j<SpsMVC->m_num_non_anchor_refs_list0[i]; j++)
			RNOK  ( pcReadIf->getUvlc( SpsMVC->m_non_anchor_ref_list0[i][j],      "SPS: non_anchor_ref_l0[i][j]" ) ); // ue(v)

		RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_num_non_anchor_refs_list1[i],                   "SPS: num_non_anchor_refs_l1[i]" ) ); // ue(v)
		SpsMVC->initViewSPSMemory_ref_for_lists(SpsMVC->getNumViewMinus1(),vcOrder,1,1);
		for (j=0; j<SpsMVC->m_num_non_anchor_refs_list1[i]; j++)
			RNOK  ( pcReadIf->getUvlc( SpsMVC->m_non_anchor_ref_list1[i][j],      "SPS: non_anchor_ref_l1[i][j]" ) ); // ue(v)

		}

		RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_num_level_values_signalled_minus1,                   "SPS: num_level_values_signalled_minus1" ) ); // ue(v)
		SpsMVC->initViewSPSMemory_num_level_related_memory(SpsMVC->getNumLevelValuesSignalledMinus1());
		for (i=0;i<= SpsMVC->m_num_level_values_signalled_minus1; i++)  
		{
			RNOK  ( pcReadIf->getCode( SpsMVC->m_ui_level_idc[i] ,                               8,      "SPS: level_idc[i]" ) );
			RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_ui_num_applicable_ops_minus1[i],                   "SPS: num_applicable_ops_minus1[i]" ) ); // ue(v)
			SpsMVC->initViewSPSMemory_num_level_related_memory_2D(SpsMVC->m_ui_num_applicable_ops_minus1[i],i);
			for (j=0; j<=(int)SpsMVC->m_ui_num_applicable_ops_minus1[i];j++)
			{
				RNOK  ( pcReadIf->getCode( SpsMVC->m_ui_applicable_op_temporal_id[i][j] ,                               3,      "SPS: applicable_op_temporal_id[i][j]" ) );
				RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_ui_applicable_op_num_target_views_minus1[i][j],                   "SPS: applicable_op_num_target_views_minus1[i][j]" ) ); // ue(v)
				SpsMVC->initViewSPSMemory_num_level_related_memory_3D(SpsMVC->m_ui_num_applicable_ops_minus1[i],SpsMVC->m_ui_applicable_op_num_target_views_minus1[i][j], i,j);
				for (k=0; k<= (int)SpsMVC->m_ui_applicable_op_num_target_views_minus1[i][j];k++)
					RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_ui_applicable_op_target_view_id[i][j][k],                    "SPS: applicable_op_num_target_view_id[i][j][k]" ) ); // ue(v)
				
				RNOK  ( pcReadIf->getUvlc( (UInt &)SpsMVC->m_ui_applicable_op_num_views_minus1[i][j],                   "SPS: num_applicable_op_num_views_minus1[i][j]" ) ); // ue(v)
			
			
			}
		
		}

	}
	
	RNOK  ( pcReadIf->getFlag( bTmp,                      "SUBSET SPS: mvc_vui_parameters_present_flag" ) );
//	ROT ( bTmp ); // always shoule be set to 0 

	if(bTmp)//lufeng: support uvi syntax element
	{
		UInt uiTemp;
		RNOK  ( pcReadIf->getUvlc(uiTemp,"SPS: vui_mvc_num_ops_minus1"));
		RNOK  ( pcReadIf->getCode(uiTemp,3,"SPS: vui_mvc_temporal_id[ 0 ]"));
		RNOK  ( pcReadIf->getUvlc(uiTemp,"SPS: vui_mvc_num_target_output_views_minus1[ 0 ]"));
		RNOK  ( pcReadIf->getUvlc(uiTemp,"SPS: vui_mvc_view_id[ 0 ][ 0 ]"));
		RNOK  ( pcReadIf->getFlag(bTmp,"SPS: vui_mvc_timing_info_present_flag[ 0 ]"));
		RNOK  ( pcReadIf->getFlag(bTmp,"SPS: vui_mvc_nal_hrd_parameters_present_flag[ 0 ]"));
		ROT ( bTmp ); // always shoule be set to 0
		RNOK  ( pcReadIf->getFlag(bTmp,"SPS: vui_mvc_vcl_hrd_parameters_present_flag[ 0 ]"));
		ROT ( bTmp ); // always shoule be set to 0
		RNOK  ( pcReadIf->getFlag(bTmp,"SPS: vui_mvc_pic_struct_present_flag[ 0 ]"));
		ROT ( bTmp ); // always shoule be set to 0
	}

	RNOK  ( pcReadIf->getFlag( bTmp,                      "SUBSET SPS: Additional_extension2_flag" ) );
//	ROT ( bTmp ); // always shoule be set to 0 
  }
  return Err::m_nOK;
}


ErrVal
SequenceParameterSet::xWriteFrext( HeaderSymbolWriteIf* pcWriteIf ) const
{
  ROTRS( m_eProfileIdc != HIGH_PROFILE      &&
         m_eProfileIdc != HIGH_10_PROFILE   &&
         m_eProfileIdc != HIGH_422_PROFILE  &&
         m_eProfileIdc != HIGH_444_PROFILE  &&
         m_eProfileIdc != MULTI_VIEW_PROFILE &&
		 m_eProfileIdc != STEREO_HIGH_PROFILE &&
         m_eProfileIdc != SCALABLE_PROFILE, Err::m_nOK );

  RNOK  ( pcWriteIf->writeUvlc( 1,                              "SPS: chroma_format_idc" ) );
  RNOK  ( pcWriteIf->writeUvlc( 0,                              "SPS: bit_depth_luma_minus8" ) );
  RNOK  ( pcWriteIf->writeUvlc( 0,                              "SPS: bit_depth_chroma_minus8" ) );
  RNOK  ( pcWriteIf->writeFlag( false,                          "SPS: qpprime_y_zero_transform_bypass_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bSeqScalingMatrixPresentFlag, "SPS: seq_scaling_matrix_present_flag"  ) );
  
  ROTRS ( ! m_bSeqScalingMatrixPresentFlag, Err::m_nOK );
  RNOK  ( m_cSeqScalingMatrix.write( pcWriteIf, true ) );

  return Err::m_nOK;
}


ErrVal
SequenceParameterSet::xReadFrext( HeaderSymbolReadIf* pcReadIf )
{
  ROTRS( m_eProfileIdc != HIGH_PROFILE      &&
         m_eProfileIdc != HIGH_10_PROFILE   &&
         m_eProfileIdc != HIGH_422_PROFILE  &&
         m_eProfileIdc != HIGH_444_PROFILE  &&
         m_eProfileIdc != MULTI_VIEW_PROFILE &&
		 m_eProfileIdc != STEREO_HIGH_PROFILE &&
         m_eProfileIdc != SCALABLE_PROFILE, Err::m_nOK );

  UInt  uiTmp;
  Bool  bTmp;
  RNOK( pcReadIf->getUvlc( uiTmp,                               "SPS: chroma_format_idc" ) );
  ROF ( uiTmp == 1 );
  RNOK( pcReadIf->getUvlc( uiTmp,                               "SPS: bit_depth_luma_minus8" ) );
  ROF ( uiTmp == 0 );
  RNOK( pcReadIf->getUvlc( uiTmp,                               "SPS: bit_depth_chroma_minus8" ) );
  ROF ( uiTmp == 0 );
  RNOK( pcReadIf->getFlag( bTmp,                                "SPS: qpprime_y_zero_transform_bypass_flag" ) );
  ROT ( bTmp )
  RNOK( pcReadIf->getFlag( m_bSeqScalingMatrixPresentFlag,      "SPS: seq_scaling_matrix_present_flag") );
  
  ROTRS ( ! m_bSeqScalingMatrixPresentFlag, Err::m_nOK );
  RNOK  ( m_cSeqScalingMatrix.read( pcReadIf, true ) );

  return Err::m_nOK;
}


// TMM_ESS {
Void SequenceParameterSet::setResizeParameters ( const ResizeParameters * params )
{
  m_uiExtendedSpatialScalability = (UInt)params->m_iExtendedSpatialScalability;

  m_uiChromaPhaseXPlus1 = (UInt)(params->m_iChromaPhaseX+1);
  m_uiChromaPhaseYPlus1 = (UInt)(params->m_iChromaPhaseY+1);

  if (m_uiExtendedSpatialScalability == ESS_SEQ)
  {
    m_iScaledBaseLeftOffset   = params->m_iPosX /2;
    m_iScaledBaseTopOffset    = params->m_iPosY /2;
    m_iScaledBaseRightOffset  = (params->m_iGlobWidth - params->m_iPosX - params->m_iOutWidth) /2;
    m_iScaledBaseBottomOffset = (params->m_iGlobHeight - params->m_iPosY - params->m_iOutHeight) /2;
    printf("offset: %d,%d %d,%d\n", m_iScaledBaseLeftOffset<<1, m_iScaledBaseTopOffset<<1, m_iScaledBaseRightOffset<<1, m_iScaledBaseBottomOffset<<1);
  }
  else
  {
    m_iScaledBaseBottomOffset = 0;
    m_iScaledBaseLeftOffset = 0;
    m_iScaledBaseRightOffset = 0;
    m_iScaledBaseTopOffset = 0;
  }
 }

Void SequenceParameterSet::getResizeParameters ( ResizeParameters * params ) const
{
  params->m_iExtendedSpatialScalability = m_uiExtendedSpatialScalability;

  params->m_bCrop = (m_uiExtendedSpatialScalability != ESS_NONE);

  int w = m_uiFrameWidthInMbs * 16;
  int h = m_uiFrameHeightInMbs * 16;
  params->m_iGlobWidth  = w;
  params->m_iGlobHeight = h;

  params->m_iChromaPhaseX = (Int)m_uiChromaPhaseXPlus1 - 1;
  params->m_iChromaPhaseY = (Int)m_uiChromaPhaseYPlus1 - 1;


  if (m_uiExtendedSpatialScalability == ESS_SEQ)
  {
    params->m_iPosX       = m_iScaledBaseLeftOffset *2;
    params->m_iPosY       = m_iScaledBaseTopOffset *2;
    params->m_iOutWidth   = w - params->m_iPosX - (m_iScaledBaseRightOffset *2);
    params->m_iOutHeight  = h - params->m_iPosY - (m_iScaledBaseBottomOffset *2);
  }
  else
  {
    params->m_iOutWidth   = w;
    params->m_iOutHeight  = h;
    params->m_iPosX       = 0;
    params->m_iPosY       = 0;
  }
}
// TMM_ESS }


H264AVC_NAMESPACE_END

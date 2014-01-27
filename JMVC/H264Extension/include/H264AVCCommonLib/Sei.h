#if !defined(AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_)
#define AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "H264AVCCommonLib/CommonBuffers.h"
#include "H264AVCCommonLib/HeaderSymbolReadIf.h"
#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"
#include <list>

#define MAX_NUM_LAYER 6



H264AVC_NAMESPACE_BEGIN



class ParameterSetMng;


#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API SEI
{

public:
  enum MessageType
  {
	  SUB_SEQ_INFO                          = 10,
	  MOTION_SEI                            = 18,
	  //{{SEI JJ
	  SCALABLE_SEI                          = 24,  
	  SUB_PIC_SEI							= 25,
	  //{{Quality level estimation and modified truncation- JVTO044 and m12007
	  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
	  QUALITYLEVEL_SEI                      = 45,   
	  //}}Quality level estimation and modified truncation- JVTO044 and m12007
	  // JVT-S080 LMI {
	  SCALABLE_SEI_LAYERS_NOT_PRESENT       = 28,
	  SCALABLE_SEI_DEPENDENCY_CHANGE        = 29,
	  //SEI LSJ{
	  SCALABLE_NESTING_SEI                  = 30, 
	  FULLFRAME_SNAPSHOT_SEI                = 15,
	  //ACTIVE_VIEWINFO_SEI                   = 20, 
	  VIEW_SCALABILITY_INFO_SEI             = 38, 
	  //SEI }
	  MULTIVIEW_SCENE_INFO_SEI			    = 39, // SEI JVT-W060 
	  MULTIVIEW_ACQUISITION_INFO_SEI		= 40, // SEI JVT-W060 
	  RESERVED_SEI                          = 46, //JVT-AB025
	  //JVT-W080
	  PARALLEL_DEC_SEI                      =36,    
	  /* RESERVED_SEI                          = 30, */ // SEI JVT-W060
	  /*RESERVED_SEI                          = 28,*/
	  //~JVT-W080   

	  NON_REQ_VIEW_INFO_SEI                 = 41,  //JVT-AB025 
	  VIEW_DEPENDENCY_STRUCTURE_SEI         = 42,  //JVT-AB025 
	  OP_NOT_PRESENT_SEI                    = 43,  //JVT-AB025 
	  // JVT-S080 LMI }
	  NON_REQUIRED_SEI					    = 26
	  //}}SEI JJ
  };


  class H264AVCCOMMONLIB_API SEIMessage
  {
  public:
    virtual ~SEIMessage()                                                       {}
    MessageType     getMessageType()                                      const { return m_eMessageType; }
    virtual ErrVal  write         ( HeaderSymbolWriteIf* pcWriteIf ) = 0;
    virtual ErrVal  read          ( HeaderSymbolReadIf*   pcReadIf ) = 0;
	UInt NumOfViewMinus1; // SEI JVT-W060

  protected:
    SEIMessage( MessageType eMessageType) : m_eMessageType( eMessageType ) {}

  private:
    MessageType m_eMessageType;	
  };



  class H264AVCCOMMONLIB_API ReservedSei : public SEIMessage
  {
  protected:
    ReservedSei( UInt uiSize = 0 ) : SEIMessage(RESERVED_SEI), m_uiSize(uiSize) {}

  public:
    static ErrVal create( ReservedSei*&         rpcReservedSei,
                          UInt                  uiSize );
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
  
  private:
    UInt m_uiSize;
  };


  class H264AVCCOMMONLIB_API SubSeqInfo : public SEIMessage
  {
  protected:
    SubSeqInfo()
      : SEIMessage(SUB_SEQ_INFO)
      , m_uiSubSeqLayerNum      (0)
	    , m_uiSubSeqId            (0)
	    , m_bFirstRefPicFlag      (false)
	    , m_bLeadingNonRefPicFlag (false)
	    , m_bLastPicFlag          (false)
	    , m_bSubSeqFrameNumFlag   (false)
      , m_uiSubSeqFrameNum      (0)
    {}

  public:
    static ErrVal create( SubSeqInfo*&          rpcSEIMessage );
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        init  ( UInt                  uiSubSeqLayerNum,
	                        UInt                  uiSubSeqId,
	                        Bool                  bFirstRefPicFlag,
	                        Bool                  bLeadingNonRefPicFlag,
	                        Bool                  bLastPicFlag        = false,
	                        Bool                  bSubSeqFrameNumFlag = false,
                          UInt                  uiSubSeqFrameNum    = 0 );

    UInt getSubSeqId      ()  const { return m_uiSubSeqId; }
    UInt getSubSeqLayerNum()  const { return m_uiSubSeqLayerNum; }

  private:
	  UInt  m_uiSubSeqLayerNum;
	  UInt  m_uiSubSeqId;
	  Bool  m_bFirstRefPicFlag;
	  Bool  m_bLeadingNonRefPicFlag;
	  Bool  m_bLastPicFlag;
	  Bool  m_bSubSeqFrameNumFlag;
    UInt  m_uiSubSeqFrameNum;
  };

	class H264AVCCOMMONLIB_API ScalableSei: public SEIMessage
	{
	protected:
		ScalableSei ();
		~ScalableSei();

	public:
		static ErrVal create ( ScalableSei*&			rpcSeiMessage);
		ErrVal write				 ( HeaderSymbolWriteIf	*pcWriteIf);
		ErrVal read					 ( HeaderSymbolReadIf		*pcReadIf);

		Void setNumLayersMinus1( UInt ui )																				{ m_num_layers_minus1 = ui;	}
		Void setLayerId ( UInt uilayer, UInt uiId )																{ m_layer_id															[uilayer] = uiId; }
	//JVT-S036 lsj start
//		Void setFGSlayerFlag ( UInt uilayer, Bool bFlag )													{ m_fgs_layer_flag												[uilayer] = bFlag; }   
		Void setSimplePriorityId ( UInt uilayer, UInt uiLevel )										{ m_simple_priority_id										[uilayer] = uiLevel; }
		Void setDiscardableFlag	(UInt uilayer, Bool bFlag)												{ m_discardable_flag											[uilayer] = bFlag; }
		Void setTemporalLevel ( UInt uilayer, UInt uiLevel )											{ m_temporal_level												[uilayer] = uiLevel; }
		Void setDependencyId ( UInt uilayer, UInt uiId )													{ m_dependency_id													[uilayer] = uiId; }
		Void setQualityLevel ( UInt uilayer, UInt uiLevel )												{ m_quality_level													[uilayer] = uiLevel; }
	
		Void setSubPicLayerFlag ( UInt uilayer, Bool bFlag)												{ m_sub_pic_layer_flag[uilayer] = bFlag; }
		Void setSubRegionLayerFlag ( UInt uilayer, Bool bFlag)										{ m_sub_region_layer_flag									[uilayer] = bFlag; }
		Void setIroiSliceDivisionInfoPresentFlag ( UInt uilayer, Bool bFlag )				{ m_iroi_slice_division_info_present_flag		[uilayer] = bFlag; } 
		Void setProfileLevelInfoPresentFlag ( UInt uilayer, Bool bFlag)						{ m_profile_level_info_present_flag				[uilayer] = bFlag; }
	//JVT-S036  end

		Void setBitrateInfoPresentFlag ( UInt uilayer, Bool bFlag )								{ m_bitrate_info_present_flag							[uilayer] = bFlag; }
		Void setFrmRateInfoPresentFlag ( UInt uilayer, Bool bFlag )								{ m_frm_rate_info_present_flag						[uilayer] = bFlag; }
		Void setFrmSizeInfoPresentFlag ( UInt uilayer, Bool bFlag )								{ m_frm_size_info_present_flag						[uilayer] = bFlag; }
		Void setLayerDependencyInfoPresentFlag ( UInt uilayer, Bool bFlag )				{ m_layer_dependency_info_present_flag		[uilayer] = bFlag; }
		Void setInitParameterSetsInfoPresentFlag ( UInt uilayer, Bool bFlag )			{ m_init_parameter_sets_info_present_flag	[uilayer] = bFlag; }
		Void setExactInterlayerPredFlag ( UInt uilayer, Bool bFlag )			{ m_exact_interlayer_pred_flag  [uilayer] = bFlag; }				//JVT-S036 
		Void setLayerProfileIdc ( UInt uilayer, UInt uiIdc )											{ m_layer_profile_idc											[uilayer] = uiIdc; }
		Void setLayerConstraintSet0Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set0_flag						[uilayer] = bFlag; }
		Void setLayerConstraintSet1Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set1_flag						[uilayer] = bFlag; }
		Void setLayerConstraintSet2Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set2_flag						[uilayer] = bFlag; }
		Void setLayerConstraintSet3Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set3_flag						[uilayer] = bFlag; }
		Void setLayerLevelIdc ( UInt uilayer, UInt uiIdc )												{ m_layer_level_idc												[uilayer] = uiIdc; }
		
	//JVT-S036  start
		Void setProfileLevelInfoSrcLayerIdDelta ( UInt uilayer, UInt uiIdc ) { m_profile_level_info_src_layer_id_delta [uilayer] = uiIdc; } 
	
		Void setAvgBitrate ( UInt uilayer, UInt uiBitrate )												{ m_avg_bitrate										[uilayer] = uiBitrate; }
		Void setMaxBitrateLayer ( UInt uilayer, UInt uiBitrate )										{ m_max_bitrate_layer								[uilayer] = uiBitrate; }
		Void setMaxBitrateDecodedPicture ( UInt uilayer, UInt uiBitrate )								{ m_max_bitrate_decoded_picture						[uilayer] = uiBitrate; }		
		Void setMaxBitrateCalcWindow ( UInt uilayer, UInt uiBitrate )									{ m_max_bitrate_calc_window							[uilayer] = uiBitrate; }
	//JVT-S036  end
		
		
		Void setConstantFrmRateIdc ( UInt uilayer, UInt uiFrmrate )								{ m_constant_frm_rate_idc									[uilayer] = uiFrmrate; }
		Void setAvgFrmRate ( UInt uilayer, UInt uiFrmrate )												{ m_avg_frm_rate													[uilayer] = uiFrmrate; }
		Void setFrmRateInfoSrcLayerIdDelta( UInt uilayer, UInt uiFrmrate)					{ m_frm_rate_info_src_layer_id_delta			[uilayer] = uiFrmrate; } //JVT-S036 
		Void setFrmWidthInMbsMinus1 ( UInt uilayer, UInt uiWidth )								{ m_frm_width_in_mbs_minus1								[uilayer] = uiWidth; }
		Void setFrmHeightInMbsMinus1 ( UInt uilayer, UInt uiHeight )							{ m_frm_height_in_mbs_minus1							[uilayer] = uiHeight; }
		Void setFrmSizeInfoSrcLayerIdDelta ( UInt uilayer, UInt uiFrmsize)					{ m_frm_size_info_src_layer_id_delta			[uilayer] = uiFrmsize; } //JVT-S036 
		Void setBaseRegionLayerId ( UInt uilayer, UInt uiId )											{ m_base_region_layer_id									[uilayer] = uiId; }
		Void setDynamicRectFlag ( UInt uilayer, Bool bFlag )											{ m_dynamic_rect_flag											[uilayer] = bFlag; }
		Void setHorizontalOffset ( UInt uilayer, UInt uiOffset )									{ m_horizontal_offset											[uilayer] = uiOffset; }
		Void setVerticalOffset ( UInt uilayer, UInt uiOffset )										{ m_vertical_offset												[uilayer] = uiOffset; }
		Void setRegionWidth ( UInt uilayer, UInt uiWidth )												{ m_region_width													[uilayer] = uiWidth; }
		Void setRegionHeight ( UInt uilayer, UInt uiHeight )											{ m_region_height													[uilayer] = uiHeight; }
		Void setSubRegionInfoSrcLayerIdDelta ( UInt uilayer, UInt uiSubRegion )					{ m_sub_region_info_src_layer_id_delta						[uilayer] = uiSubRegion; } //JVT-S036 
	//JVT-S036  start
		Void setRoiId ( UInt uilayer, UInt RoiId )												{ m_roi_id[uilayer]	= RoiId; } 
		Void setIroiSliceDivisionType ( UInt uilayer, UInt bType )								{ m_iroi_slice_division_type[uilayer] = bType; }
		Void setGridSliceWidthInMbsMinus1 ( UInt uilayer, UInt bWidth )							{ m_grid_slice_width_in_mbs_minus1[uilayer] = bWidth; }
		Void setGridSliceHeightInMbsMinus1 ( UInt uilayer, UInt bHeight )						{ m_grid_slice_height_in_mbs_minus1[uilayer] = bHeight; }

		Void setROINum(UInt iDependencyId, UInt iNumROI)  		{ m_aiNumRoi[iDependencyId] = iNumROI; }
		Void setROIID(UInt iDependencyId, UInt* iROIId)
		{
			for (UInt i =0; i < m_aiNumRoi[iDependencyId]; ++i)
			{
				m_aaiRoiID[iDependencyId][i] = iROIId[i];
			}
		}
		Void setSGID(UInt iDependencyId, UInt* iSGId)
		{
			for (UInt i =0; i < m_aiNumRoi[iDependencyId]; ++i)
			{
				m_aaiSGID[iDependencyId][i] = iSGId[i];
			}
		}
		Void setSLID(UInt iDependencyId, UInt* iSGId)
		{
			for (UInt i =0; i < m_aiNumRoi[iDependencyId]; ++i)
			{
				m_aaiSLID[iDependencyId][i] = iSGId[i];
			}
		}

		// JVT-S054 (REPLACE) ->
		//Void setNumSliceMinus1 ( UInt uilayer, UInt bNum ) 										{ m_num_slice_minus1[uilayer] = bNum; }
    Void setNumSliceMinus1 ( UInt uilayer, UInt bNum )
    {
      if ( m_num_slice_minus1[uilayer] != bNum )
      {
        if ( m_first_mb_in_slice[uilayer] != NULL )
        {
          free(m_first_mb_in_slice[uilayer]);
          m_first_mb_in_slice[uilayer] = NULL;
        }
        if ( m_slice_width_in_mbs_minus1[uilayer] != NULL )
        {
          free(m_slice_width_in_mbs_minus1[uilayer]);
          m_slice_width_in_mbs_minus1[uilayer] = NULL;
        }
        if ( m_slice_height_in_mbs_minus1[uilayer] != NULL )
        {
          free(m_slice_height_in_mbs_minus1[uilayer]);
          m_slice_height_in_mbs_minus1[uilayer] = NULL;
        }
      }

      m_num_slice_minus1[uilayer] = bNum;

      if ( m_first_mb_in_slice[uilayer] == NULL )
        m_first_mb_in_slice[uilayer] = (UInt*)malloc((bNum+1)*sizeof(UInt));

      if ( m_slice_width_in_mbs_minus1[uilayer] == NULL )
        m_slice_width_in_mbs_minus1[uilayer] = (UInt*)malloc((bNum+1)*sizeof(UInt));

      if ( m_slice_height_in_mbs_minus1[uilayer] == NULL )
        m_slice_height_in_mbs_minus1[uilayer] = (UInt*)malloc((bNum+1)*sizeof(UInt));

      if ( sizeof(m_slice_id[uilayer]) != (m_frm_width_in_mbs_minus1[uilayer]+1)*(m_frm_height_in_mbs_minus1[uilayer]+1)*sizeof(UInt) )
      {
        free(m_slice_id[uilayer]);
        m_slice_id[uilayer] = NULL;
      }
      if ( m_slice_id[uilayer] == NULL )
        m_slice_id[uilayer] = (UInt*)malloc((m_frm_width_in_mbs_minus1[uilayer]+1)*(m_frm_height_in_mbs_minus1[uilayer]+1)*sizeof(UInt));
    }
		// JVT-S054 (REPLACE) <-

		Void setFirstMbInSlice ( UInt uilayer, UInt uiTar, UInt bNum )							{ m_first_mb_in_slice[uilayer][uiTar] = bNum; }
		Void setSliceWidthInMbsMinus1 ( UInt uilayer, UInt uiTar, UInt bWidth )					{ m_slice_width_in_mbs_minus1[uilayer][uiTar] = bWidth; }
		Void setSliceHeightInMbsMinus1 ( UInt uilayer, UInt uiTar, UInt bHeight )				{ m_slice_height_in_mbs_minus1[uilayer][uiTar] = bHeight; }
		Void setSliceId ( UInt uilayer, UInt uiTar, UInt bId )									{ m_slice_id[uilayer][uiTar] = bId; }
    //JVT-S036  end	
		Void setNumDirectlyDependentLayers ( UInt uilayer, UInt uiNum )						{ m_num_directly_dependent_layers					[uilayer] = uiNum; }
		Void setDirectlyDependentLayerIdDeltaMinus1( UInt uilayer, UInt uiTar, UInt uiDelta ) { m_directly_dependent_layer_id_delta_minus1[uilayer][uiTar] = uiDelta;} ///JVT-S036 
		Void setLayerDependencyInfoSrcLayerIdDelta( UInt uilayer, UInt uiDelta )		  { m_layer_dependency_info_src_layer_id_delta	    [uilayer] = uiDelta;} //JVT-S036 
		Void setNumInitSeqParameterSetMinus1 ( UInt uilayer, UInt uiNum )					{ m_num_init_seq_parameter_set_minus1			[uilayer] = uiNum; }
		Void setInitSeqParameterSetIdDelta ( UInt uilayer, UInt uiSPS, UInt uiTar){ m_init_seq_parameter_set_id_delta				[uilayer][uiSPS] = uiTar;	}
		Void setNumInitPicParameterSetMinus1 ( UInt uilayer, UInt uiNum )					{ m_num_init_pic_parameter_set_minus1			[uilayer] = uiNum; }
		Void setInitPicParameterSetIdDelta ( UInt uilayer, UInt uiPPS, UInt uiTar){ m_init_pic_parameter_set_id_delta				[uilayer][uiPPS] = uiTar; }
		Void setInitParameterSetsInfoSrcLayerIdDelta (UInt uilayer, UInt uiDelta)	{ m_init_parameter_sets_info_src_layer_id_delta[uilayer] = uiDelta; } //JVT-S036 
// BUG_FIX liuhui{
		Void setStdAVCOffset( UInt uiOffset )                                     { m_std_AVC_Offset = uiOffset;}
		UInt getStdAVCOffset()const { return m_std_AVC_Offset; }
// BUG_FIX liuhui}

		UInt getNumLayersMinus1() const {return m_num_layers_minus1;}
		UInt getLayerId ( UInt uilayer ) const { return m_layer_id[uilayer]; }
	 //JVT-S036  start
//		Bool getFGSLayerFlag ( UInt uilayer ) const { return m_fgs_layer_flag[uilayer]; } 
		UInt getSimplePriorityId ( UInt uilayer ) const { return  m_simple_priority_id [uilayer]; }
		Bool getDiscardableFlag	(UInt uilayer) const { return  m_discardable_flag [uilayer]; }
		UInt getTemporalLevel ( UInt uilayer ) const { return m_temporal_level[uilayer]; }
		UInt getDependencyId ( UInt uilayer ) const { return m_dependency_id[uilayer]; }
		UInt getQualityLevel ( UInt uilayer ) const { return m_quality_level[uilayer]; }
	
		Bool getSubPicLayerFlag ( UInt uilayer ) { return m_sub_pic_layer_flag[uilayer]; }
		Bool getSubRegionLayerFlag ( UInt uilayer ) const { return m_sub_region_layer_flag[uilayer]; }
		Bool getIroiSliceDivisionInfoPresentFlag ( UInt uilayer ) const { return m_iroi_slice_division_info_present_flag[uilayer]; } 
		Bool getProfileLevelInfoPresentFlag ( UInt uilayer ) const { return m_profile_level_info_present_flag[uilayer]; }
   //JVT-S036  end
		Bool getBitrateInfoPresentFlag ( UInt uilayer ) const { return m_bitrate_info_present_flag[uilayer]; }
		Bool getFrmRateInfoPresentFlag ( UInt uilayer ) const { return m_frm_rate_info_present_flag[uilayer]; }
		Bool getFrmSizeInfoPresentFlag ( UInt uilayer ) const { return m_frm_size_info_present_flag[uilayer]; }
		Bool getLayerDependencyInfoPresentFlag ( UInt uilayer ) const { return m_layer_dependency_info_present_flag[uilayer]; }
		Bool getInitParameterSetsInfoPresentFlag ( UInt uilayer ) const { return m_init_parameter_sets_info_present_flag[uilayer]; }

		Bool getExactInterlayerPredFlag ( UInt uilayer )	const { return m_exact_interlayer_pred_flag  [uilayer]; }				//JVT-S036 

		UInt getLayerProfileIdc ( UInt uilayer ) const { return m_layer_profile_idc[uilayer]; }
		Bool getLayerConstraintSet0Flag ( UInt uilayer ) const { return m_layer_constraint_set0_flag[uilayer]; }
		Bool getLayerConstraintSet1Flag ( UInt uilayer ) const { return m_layer_constraint_set1_flag[uilayer]; }
		Bool getLayerConstraintSet2Flag ( UInt uilayer ) const { return m_layer_constraint_set2_flag[uilayer]; }
		Bool getLayerConstraintSet3Flag ( UInt uilayer ) const { return m_layer_constraint_set3_flag[uilayer]; }
		UInt getLayerLevelIdc ( UInt uilayer ) const { return m_layer_level_idc[uilayer]; }

	//JVT-S036  start
		UInt getProfileLevelInfoSrcLayerIdDelta ( UInt uilayer) const { return m_profile_level_info_src_layer_id_delta [uilayer];} 
		
		UInt getAvgBitrate ( UInt uilayer ) const { return m_avg_bitrate[uilayer]; }
		UInt getMaxBitrateLayer ( UInt uilayer ) const { return m_max_bitrate_layer[uilayer]; }
		UInt getMaxBitrateDecodedPicture ( UInt uilayer ) const { return m_max_bitrate_decoded_picture[uilayer]; }		
		UInt getMaxBitrateCalcWindow ( UInt uilayer ) const { return m_max_bitrate_calc_window[uilayer]; }
	//JVT-S036  end

		
		UInt getConstantFrmRateIdc ( UInt uilayer ) const { return m_constant_frm_rate_idc[uilayer]; }
		UInt getAvgFrmRate ( UInt uilayer ) const { return m_avg_frm_rate[uilayer]; }
		UInt getFrmRateInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_frm_rate_info_src_layer_id_delta[uilayer]; } //JVT-S036 
		UInt getFrmWidthInMbsMinus1 ( UInt uilayer ) const { return m_frm_width_in_mbs_minus1[uilayer]; }
		UInt getFrmHeightInMbsMinus1 ( UInt uilayer ) const { return m_frm_height_in_mbs_minus1[uilayer]; }
		UInt getFrmSizeInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_frm_size_info_src_layer_id_delta[uilayer]; } //JVT-S036 
		UInt getBaseRegionLayerId ( UInt uilayer ) const { return m_base_region_layer_id[uilayer]; }
		Bool getDynamicRectFlag ( UInt uilayer ) const { return m_dynamic_rect_flag[uilayer]; }
		UInt getHorizontalOffset ( UInt uilayer ) const { return m_horizontal_offset[uilayer]; }
		UInt getVerticalOffset ( UInt uilayer ) const { return m_vertical_offset[uilayer]; }
		UInt getRegionWidth ( UInt uilayer ) const { return m_region_width[uilayer]; }
		UInt getRegionHeight ( UInt uilayer ) const { return m_region_height[uilayer]; }
		UInt getSubRegionInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_sub_region_info_src_layer_id_delta[uilayer]; } ///JVT-S036 
	//JVT-S036  start
		UInt getRoiId ( UInt uilayer ) const { return m_roi_id[uilayer]; } 
		UInt getIroiSliceDivisionType ( UInt uilayer ) const { return m_iroi_slice_division_type[uilayer]; }
		UInt getGridSliceWidthInMbsMinus1 ( UInt uilayer ) const { return m_grid_slice_width_in_mbs_minus1[uilayer]; }
		UInt getGridSliceHeightInMbsMinus1 ( UInt uilayer ) const { return m_grid_slice_height_in_mbs_minus1[uilayer]; }
		UInt getNumSliceMinus1 ( UInt uilayer ) const { return m_num_slice_minus1[uilayer]; }
		UInt getFirstMbInSlice ( UInt uilayer, UInt uiTar )	const { return m_first_mb_in_slice[uilayer][uiTar]; }
		UInt getSliceWidthInMbsMinus1 ( UInt uilayer, UInt uiTar ) const { return m_slice_width_in_mbs_minus1[uilayer][uiTar]; }
		UInt getSliceHeightInMbsMinus1 ( UInt uilayer, UInt uiTar ) const { return m_slice_height_in_mbs_minus1[uilayer][uiTar]; }
		UInt getSliceId ( UInt uilayer, UInt uiTar ) const { return m_slice_id[uilayer][uiTar]; }
	//JVT-S036  end

		UInt getNumDirectlyDependentLayers ( UInt uilayer ) const { return m_num_directly_dependent_layers[uilayer]; }
// BUG_FIX liuhui{
		UInt getNumDirectlyDependentLayerIdDeltaMinus1( UInt uilayer, UInt uiIndex ) const { return m_directly_dependent_layer_id_delta_minus1[uilayer][uiIndex]; } //JVT-S036 
// BUG_FIX liuhui}
		UInt getLayerDependencyInfoSrcLayerIdDelta( UInt uilayer ) const { return m_layer_dependency_info_src_layer_id_delta[uilayer];} //JVT-S036 
		//
		UInt getNumInitSPSMinus1 ( UInt uilayer ) const { return m_num_init_seq_parameter_set_minus1[uilayer]; }
		UInt getNumInitPPSMinus1 ( UInt uilayer ) const { return m_num_init_pic_parameter_set_minus1[uilayer]; }
// BUG_FIX liuhui{
		UInt getInitSPSIdDelta ( UInt uilayer, UInt uiIndex ) const { return m_init_seq_parameter_set_id_delta[uilayer][uiIndex]; }
		UInt getInitPPSIdDelta ( UInt uilayer, UInt uiIndex ) const { return m_init_pic_parameter_set_id_delta[uilayer][uiIndex]; }
// BUG_FIX liuhui}
		UInt getInitParameterSetsInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_init_parameter_sets_info_src_layer_id_delta[uilayer]; } //JVT-S036 

	private:
// BUG_FIX liuhui{
		UInt m_std_AVC_Offset;
// BUG_FIX liuhui}
		UInt m_num_layers_minus1;
		UInt m_layer_id[MAX_SCALABLE_LAYERS];
	//JVT-S036  start
		//Bool m_fgs_layer_flag[MAX_SCALABLE_LAYERS];  
		UInt m_simple_priority_id[MAX_SCALABLE_LAYERS];  
		Bool m_discardable_flag[MAX_SCALABLE_LAYERS];
		UInt m_temporal_level[MAX_SCALABLE_LAYERS];
		UInt m_dependency_id[MAX_SCALABLE_LAYERS];
		UInt m_quality_level[MAX_SCALABLE_LAYERS];

		Bool m_sub_pic_layer_flag[MAX_SCALABLE_LAYERS];
		Bool m_sub_region_layer_flag[MAX_SCALABLE_LAYERS];
		Bool m_iroi_slice_division_info_present_flag[MAX_SCALABLE_LAYERS]; 
		Bool m_profile_level_info_present_flag[MAX_SCALABLE_LAYERS];
	//JVT-S036  end
		Bool m_bitrate_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_frm_rate_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_frm_size_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_dependency_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_init_parameter_sets_info_present_flag[MAX_SCALABLE_LAYERS];

		Bool m_exact_interlayer_pred_flag[MAX_SCALABLE_LAYERS];  //JVT-S036 

		UInt m_layer_profile_idc[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set0_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set1_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set2_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set3_flag[MAX_SCALABLE_LAYERS];
		UInt m_layer_level_idc[MAX_SCALABLE_LAYERS];
        
	//JVT-S036  start
		UInt m_profile_level_info_src_layer_id_delta[MAX_SCALABLE_LAYERS]; //
	


		UInt m_avg_bitrate[MAX_SCALABLE_LAYERS];
		UInt m_max_bitrate_layer[MAX_SCALABLE_LAYERS];//
		UInt m_max_bitrate_decoded_picture[MAX_SCALABLE_LAYERS];//
		UInt m_max_bitrate_calc_window[MAX_SCALABLE_LAYERS];//

		UInt m_constant_frm_rate_idc[MAX_SCALABLE_LAYERS];
		UInt m_avg_frm_rate[MAX_SCALABLE_LAYERS];

		UInt m_frm_rate_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//
	
		UInt m_frm_width_in_mbs_minus1[MAX_SCALABLE_LAYERS];
		UInt m_frm_height_in_mbs_minus1[MAX_SCALABLE_LAYERS];

		UInt m_frm_size_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//

		UInt m_base_region_layer_id[MAX_SCALABLE_LAYERS];
		Bool m_dynamic_rect_flag[MAX_SCALABLE_LAYERS];
		UInt m_horizontal_offset[MAX_SCALABLE_LAYERS];
		UInt m_vertical_offset[MAX_SCALABLE_LAYERS];
		UInt m_region_width[MAX_SCALABLE_LAYERS];
		UInt m_region_height[MAX_SCALABLE_LAYERS];

		UInt m_sub_region_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//

		UInt m_roi_id[MAX_SCALABLE_LAYERS]; //

		UInt m_iroi_slice_division_type[MAX_SCALABLE_LAYERS]; //
		UInt m_grid_slice_width_in_mbs_minus1[MAX_SCALABLE_LAYERS]; //
		UInt m_grid_slice_height_in_mbs_minus1[MAX_SCALABLE_LAYERS]; //
		UInt m_num_slice_minus1[MAX_SCALABLE_LAYERS];//
		// JVT-S054 (REPLACE) ->
    /*
		UInt m_first_mb_in_slice[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
		UInt m_slice_width_in_mbs_minus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
		UInt m_slice_height_in_mbs_minus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
		UInt m_slice_id[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
    */
		UInt* m_first_mb_in_slice[MAX_SCALABLE_LAYERS];//
		UInt* m_slice_width_in_mbs_minus1[MAX_SCALABLE_LAYERS];//
		UInt* m_slice_height_in_mbs_minus1[MAX_SCALABLE_LAYERS];//
		UInt* m_slice_id[MAX_SCALABLE_LAYERS];//
		// JVT-S054 (REPLACE) <-
// BUG_FIX liuhui{
		UInt m_num_directly_dependent_layers[MAX_SCALABLE_LAYERS];
		UInt m_directly_dependent_layer_id_delta_minus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//

		UInt m_layer_dependency_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//

		UInt m_num_init_seq_parameter_set_minus1[MAX_SCALABLE_LAYERS];
		UInt m_init_seq_parameter_set_id_delta[MAX_SCALABLE_LAYERS][32];
		UInt m_num_init_pic_parameter_set_minus1[MAX_SCALABLE_LAYERS];
		UInt m_init_pic_parameter_set_id_delta[MAX_SCALABLE_LAYERS][256];
// BUG_FIX liuhui}
		UInt m_init_parameter_sets_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//
	//JVT-S036  end

		UInt m_aiNumRoi[MAX_SCALABLE_LAYERS];
		UInt m_aaiRoiID[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
		UInt m_aaiSGID[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
		UInt m_aaiSLID[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
	};

	class H264AVCCOMMONLIB_API SubPicSei : public SEIMessage
	{
	protected:
		SubPicSei ();
		~SubPicSei();

	public:
		static ErrVal create	( SubPicSei*&				rpcSeiMessage );
		ErrVal				write		( HeaderSymbolWriteIf*	pcWriteIf );
		ErrVal				read		( HeaderSymbolReadIf*		pcReadIf  );	

		UInt getLayerId	()					const	{ return m_uiLayerId;				}
		Void setLayerId ( UInt uiLayerId) { m_uiLayerId = uiLayerId;	}

	private:
		UInt m_uiLayerId;
	};

  class H264AVCCOMMONLIB_API MotionSEI : public SEIMessage
  {

  protected:
    MotionSEI();
    ~MotionSEI();

  public:

    UInt m_num_slice_groups_in_set_minus1;
    UInt m_slice_group_id[8];
    Bool m_exact_sample_value_match_flag;
    Bool m_pan_scan_rect_flag;

    static ErrVal create  ( MotionSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        setSliceGroupId(UInt id);
	UInt          getSliceGroupId(){return m_slice_group_id[0];}
  };
  
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  class H264AVCCOMMONLIB_API QualityLevelSEI : public SEIMessage
  {
	protected:
    QualityLevelSEI ();
    ~QualityLevelSEI();

  public:
    static ErrVal create  ( QualityLevelSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
	
	UInt		 getNumLevel() { return m_uiNumLevels;}
	Void		 setNumLevel(UInt ui) { m_uiNumLevels = ui;}
	UInt		 getDeltaBytesRateOfLevel(UInt ui) { return m_auiDeltaBytesRateOfLevel[ui];}
	Void		 setDeltaBytesRateOfLevel(UInt uiIndex, UInt ui) { m_auiDeltaBytesRateOfLevel[uiIndex] = ui;}
	UInt		 getQualityLevel(UInt ui) { return m_auiQualityLevel[ui];}
	Void		 setQualityLevel(UInt uiIndex, UInt ui) { m_auiQualityLevel[uiIndex] = ui;}
	UInt		 getDependencyId() { return m_uiDependencyId;}
	Void		 setDependencyId( UInt ui) { m_uiDependencyId = ui;}

  private:
	  UInt m_auiQualityLevel[MAX_NUM_RD_LEVELS];
	  UInt m_auiDeltaBytesRateOfLevel[MAX_NUM_RD_LEVELS];
	  UInt m_uiNumLevels;
	  UInt m_uiDependencyId;
  };
  //}}Quality level estimation and modified truncation- JVTO044 and m12007


  class H264AVCCOMMONLIB_API NonRequiredSei : public SEIMessage
  {
  protected:
	  NonRequiredSei ();
	  ~NonRequiredSei();

  public:
	  static ErrVal create	(NonRequiredSei*&			rpcSeiMessage);
	  ErrVal		destroy ();  
	  ErrVal		write	(HeaderSymbolWriteIf*		pcWriteIf);
	  ErrVal		read	(HeaderSymbolReadIf*		pcReadIf);

	  UInt			getNumInfoEntriesMinus1()					const{ return m_uiNumInfoEntriesMinus1;}
	  UInt			getEntryDependencyId(UInt uiLayer)			const{ return m_uiEntryDependencyId[uiLayer];}
	  UInt			getNumNonRequiredPicsMinus1(UInt uiLayer)	const{ return m_uiNumNonRequiredPicsMinus1[uiLayer];}
	  UInt			getNonRequiredPicDependencyId(UInt uiLayer, UInt uiNonRequiredLayer)	const{ return m_uiNonRequiredPicDependencyId[uiLayer][uiNonRequiredLayer];}
	  UInt			getNonRequiredPicQulityLevel(UInt uiLayer, UInt uiNonRequiredLayer)		const{ return m_uiNonRequiredPicQulityLevel[uiLayer][uiNonRequiredLayer];}
	  UInt			getNonRequiredPicFragmentOrder(UInt uiLayer, UInt uiNonRequiredLayer)	const{ return m_uiNonRequiredPicFragmentOrder[uiLayer][uiNonRequiredLayer];}


	  Void			setNumInfoEntriesMinus1(UInt ui)					{ m_uiNumInfoEntriesMinus1 = ui;}
	  Void			setEntryDependencyId(UInt uiLayer, UInt ui)			{ m_uiEntryDependencyId[uiLayer] = ui;}
	  Void			setNumNonRequiredPicsMinus1(UInt uiLayer, UInt ui)	{ m_uiNumNonRequiredPicsMinus1[uiLayer] = ui;}
	  Void			setNonNonRequiredPicDependencyId(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)		{m_uiNonRequiredPicDependencyId[uiLayer][uiNonRequiredLayer] = ui;}
	  Void			setNonNonRequiredPicQulityLevel(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)			{m_uiNonRequiredPicQulityLevel[uiLayer][uiNonRequiredLayer] = ui;}
	  Void			setNonNonRequiredPicFragmentOrder(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)		{m_uiNonRequiredPicFragmentOrder[uiLayer][uiNonRequiredLayer] = ui;}


  private:
	  UInt		m_uiNumInfoEntriesMinus1;
	  UInt		m_uiEntryDependencyId[MAX_NUM_INFO_ENTRIES];
	  UInt		m_uiNumNonRequiredPicsMinus1[MAX_NUM_INFO_ENTRIES];
	  UInt		m_uiNonRequiredPicDependencyId[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
	  UInt		m_uiNonRequiredPicQulityLevel[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
	  UInt		m_uiNonRequiredPicFragmentOrder[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
  };//shenqiu 05-09-15

  // JVT-S080 LMI {
  class H264AVCCOMMONLIB_API ScalableSeiLayersNotPresent: public SEIMessage
  {
  protected:
      ScalableSeiLayersNotPresent ();
	 ~ScalableSeiLayersNotPresent();

  public:
      static ErrVal create ( ScalableSeiLayersNotPresent*&			rpcSeiMessage);
      ErrVal write				 ( HeaderSymbolWriteIf	*pcWriteIf);
      ErrVal read					 ( HeaderSymbolReadIf		*pcReadIf);
      Void setNumLayers( UInt ui )																				{ m_uiNumLayers = ui;	}
      Void setLayerId ( UInt uiLayer, UInt uiId )																{ m_auiLayerId															[uiLayer] = uiId; }
	  Void setOutputFlag ( Bool bFlag )  { m_bOutputFlag = bFlag; }

      UInt getNumLayers() const {return m_uiNumLayers;}
      UInt getLayerId ( UInt uiLayer ) const { return m_auiLayerId[uiLayer]; }
	  Bool getOutputFlag ( ) const { return m_bOutputFlag; }
      static UInt m_uiLeftNumLayers;
      static UInt m_auiLeftLayerId[MAX_SCALABLE_LAYERS];

  private:
      UInt m_uiNumLayers;
      UInt m_auiLayerId[MAX_SCALABLE_LAYERS];
	  Bool m_bOutputFlag;

	};

  class H264AVCCOMMONLIB_API ScalableSeiDependencyChange: public SEIMessage
  {
  protected:
      ScalableSeiDependencyChange ();
	 ~ScalableSeiDependencyChange();

  public:
      static ErrVal create ( ScalableSeiDependencyChange*&			rpcSeiMessage);
      ErrVal write				 ( HeaderSymbolWriteIf	*pcWriteIf);
      ErrVal read					 ( HeaderSymbolReadIf		*pcReadIf);
      Void setNumLayersMinus1( UInt ui )																				{ m_uiNumLayersMinus1 = ui;	}
      Void setLayerId ( UInt uiLayer, UInt uiId )																{ m_auiLayerId															[uiLayer] = uiId; }
	  Void setLayerDependencyInfoPresentFlag ( UInt uiLayer, Bool bFlag ) { m_abLayerDependencyInfoPresentFlag[uiLayer] = bFlag; }
      Void setNumDirectDependentLayers ( UInt uiLayer, UInt ui ) { m_auiNumDirectDependentLayers[uiLayer] = ui; }
	  Void setDirectDependentLayerIdDeltaMinus1( UInt uiLayer, UInt uiDirectLayer, UInt uiIdDeltaMinus1 )  { m_auiDirectDependentLayerIdDeltaMinus1[uiLayer][uiDirectLayer] = uiIdDeltaMinus1; }
	  Void setLayerDependencyInfoSrcLayerIdDeltaMinus1 ( UInt uiLayer, UInt uiIdDeltaMinus1 ) { m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[uiLayer] = uiIdDeltaMinus1; }
	  Void setOutputFlag ( Bool bFlag )  { m_bOutputFlag = bFlag; }

      UInt getNumLayersMinus1() const {return m_uiNumLayersMinus1;}
      UInt getLayerId ( UInt uiLayer ) const { return m_auiLayerId[uiLayer]; }
      UInt getNumDirectDependentLayers ( UInt uiLayer ) const { return m_auiNumDirectDependentLayers[uiLayer]; }
	  UInt getDirectDependentLayerIdDeltaMinus1( UInt uiLayer, UInt uiDirectLayer ) const { return m_auiDirectDependentLayerIdDeltaMinus1[uiLayer][uiDirectLayer]; }
	  UInt getLayerDependencyInfoSrcLayerIdDeltaMinus1 ( UInt uiLayer ) const { return m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[uiLayer]; }
	  Bool getLayerDependencyInfoPresentFlag ( UInt uiLayer ) const { return m_abLayerDependencyInfoPresentFlag[uiLayer]; }
	  Bool getOutputFlag ( ) const { return m_bOutputFlag; }

  private:
      UInt m_uiNumLayersMinus1;
      UInt m_auiLayerId[MAX_SCALABLE_LAYERS];
      UInt m_auiNumDirectDependentLayers[MAX_SCALABLE_LAYERS];
      UInt m_auiDirectDependentLayerIdDeltaMinus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
      UInt m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[MAX_SCALABLE_LAYERS];
	  Bool m_abLayerDependencyInfoPresentFlag[MAX_SCALABLE_LAYERS];
	  Bool m_bOutputFlag;
	};

  // JVT-S080 LMI }
//JVT-W080
	class H264AVCCOMMONLIB_API ParallelDecodingSEI : public SEIMessage
	{
	protected:
    ParallelDecodingSEI();
    ~ParallelDecodingSEI();

  public:

    static ErrVal create  ( ParallelDecodingSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
		ErrVal        init    ( UInt uiSPSId
			                    , UInt uiNumView
													, UInt* num_refs_list0_anc
													, UInt* num_refs_list1_anc
													, UInt* num_refs_list0_nonanc
													, UInt* num_refs_list1_nonanc
													, UInt  PDSInitialDelayAnc
													, UInt  PDSInitialDelayNonAnc
													);
    //ErrVal        uninit  ();

	public:
		UInt          getSPSId()                   const { return m_uiSPSId; }
		UInt          getNumView()                 const { return m_uiNumView; }
		UInt*         getNumRefAnchorFramesL0()    const { return m_puiNumRefAnchorFramesL0; }		
		UInt*         getNumRefAnchorFramesL1()    const { return m_puiNumRefAnchorFramesL1; }
		UInt*         getNumNonRefAnchorFramesL0() const { return m_puiNumNonRefAnchorFramesL0; }		
		UInt*         getNumNonRefAnchorFramesL1() const { return m_puiNumNonRefAnchorFramesL1; }
		UInt**        getPDIInitDelayMinus2L0Anc() const { return m_ppuiPDIInitDelayMinus2L0Anc; }
		UInt**        getPDIInitDelayMinus2L1Anc() const { return m_ppuiPDIInitDelayMinus2L1Anc; }
		UInt**        getPDIInitDelayMinus2L0NonAnc() const { return m_ppuiPDIInitDelayMinus2L0NonAnc; }
		UInt**        getPDIInitDelayMinus2L1NonAnc() const { return m_ppuiPDIInitDelayMinus2L1NonAnc; }

		Void          setSPSId( UInt uiSPSId )              { m_uiSPSId = uiSPSId; }
		Void          setNumView( UInt uiNumView )          { m_uiNumView = uiNumView; }
		Void          setNumRefAnchorFramesL0( UInt* ui)    { m_puiNumRefAnchorFramesL0 = ui; }
		Void          setNumRefAnchorFramesL1( UInt* ui)    { m_puiNumRefAnchorFramesL1 = ui; }
		Void          setNumNonRefAnchorFramesL0( UInt* ui) { m_puiNumNonRefAnchorFramesL0 = ui; }
		Void          setNumNonRefAnchorFramesL1( UInt* ui) { m_puiNumNonRefAnchorFramesL1 = ui; }
		Void          setPDIInitDelayMinus2L0Anc( UInt** ui){ m_ppuiPDIInitDelayMinus2L0Anc = ui; }
		Void          setPDIInitDelayMinus2L1Anc( UInt** ui){ m_ppuiPDIInitDelayMinus2L1Anc = ui; }
		Void          setPDIInitDelayMinus2L0NonAnc( UInt** ui){ m_ppuiPDIInitDelayMinus2L0NonAnc = ui; }
		Void          setPDIInitDelayMinus2L1NonAnc( UInt** ui){ m_ppuiPDIInitDelayMinus2L1NonAnc = ui; }
	
	private:
		UInt          m_uiSPSId;
		UInt          m_uiNumView;
		UInt*         m_puiNumRefAnchorFramesL0;
		UInt*         m_puiNumRefAnchorFramesL1;
		UInt*         m_puiNumNonRefAnchorFramesL0;
		UInt*         m_puiNumNonRefAnchorFramesL1;
    UInt**        m_ppuiPDIInitDelayMinus2L0Anc;
		UInt**        m_ppuiPDIInitDelayMinus2L1Anc;
    UInt**        m_ppuiPDIInitDelayMinus2L0NonAnc;
		UInt**        m_ppuiPDIInitDelayMinus2L1NonAnc;
	};
//~JVT-W080

//SEI LSJ{
#define MAX_OPERATION_POINTS 10
  class H264AVCCOMMONLIB_API ViewScalabilityInfoSei : public SEIMessage
  {
  protected:
	ViewScalabilityInfoSei ();
	~ViewScalabilityInfoSei();

  public:
	static ErrVal create ( ViewScalabilityInfoSei*&		rpcSeiMessage);
	ErrVal destroy		 ();
	ErrVal write		 ( HeaderSymbolWriteIf	*pcWriteIf);
	ErrVal read			 ( HeaderSymbolReadIf		*pcReadIf);

	UInt getNumOperationPointsMinus1()			const { return m_uiNumOperationPointsMinus1; }
	UInt getOperationPointId( UInt index )			  { return m_uiOperationPointId[index]; }
	UInt getPriorityId( UInt index )				  { return m_uiPriorityId[index]; }
	UInt getTemporalId( UInt index )				  { return m_uiTemporalId[index]; }
	UInt getNumTargetOutputViewsMinus1( UInt index )		  { return m_uiNumTargetOutputViewsMinus1[index]; }//SEI JJ
	UInt getViewId( UInt i, UInt j )				  { return m_uiViewId[i][j]; }
	Bool getProfileLevelInfoPresentFlag( UInt index ) { return m_bProfileLevelInfoPresentFlag[index]; }
	Bool getBitRateInfoPresentFlag( UInt index )	  { return m_bBitRateInfoPresentFlag[index]; }
	Bool getFrmRateInfoPresentFlag( UInt index )      { return m_bFrmRateInfoPresentFlag[index]; }
	Bool getViewDependencyInfoPresentFlag( UInt index ) { return m_bViewDependencyInfoPresentFlag[index]; }//SEI JJ
	Bool getParameterSetsInfoPresentFlag(UInt i)  { return m_bParameterSetsInfoPresentFlag[i]; }//SEI JJ
	UInt getOpProfileLevelIdc( UInt index )				  { return m_uiOpProfileLevelIdc[index]; }//SEI JJ
	Bool getOpConstraintSet0Flag( UInt index )		  { return m_bOpConstraintSet0Flag[index]; }
	Bool getOpConstraintSet1Flag( UInt index )		  { return m_bOpConstraintSet1Flag[index]; }
	Bool getOpConstraintSet2Flag( UInt index )		  { return m_bOpConstraintSet2Flag[index]; }
	Bool getOpConstraintSet3Flag( UInt index )		  { return m_bOpConstraintSet3Flag[index]; }
//bug_fix_chenlulu{
	Bool getOpConstraintSet4Flag( UInt index )		  { return m_bOpConstraintSet4Flag[index]; }
	Bool getOpConstraintSet5Flag( UInt index )		  { return m_bOpConstraintSet5Flag[index]; }
//bug_fix_chenlulu}
	UInt getOpLevelIdc( UInt index )			      { return m_uiOpLevelIdc[index]; }
	UInt getProfileLevelInfoSrcOpIdDelta( UInt i )    { return m_uiProfileLevelInfoSrcOpIdDelta[i]; }
	UInt getAvgBitrate( UInt index )				  { return m_uiAvgBitrate[index]; }
	UInt getMaxBitrate( UInt index )				  { return m_uiMaxBitrate[index]; }
	UInt getMaxBitrateCalcWindow( UInt index )		  { return m_uiMaxBitrateCalcWindow[index]; }
	UInt getConstantFrmRateIdc( UInt index )		  { return m_uiConstantFrmRateIdc[index]; }
	UInt getAvgFrmRate( UInt index )				  { return m_uiAvgFrmRate[index]; }
	UInt getFrmRateInfoSrcOpIdDela( UInt index )      { return m_uiFrmRateInfoSrcOpIdDela[index]; }
	UInt getNumDirectlyDependentViews( UInt index )	  { return m_uiNumDirectlyDependentViews[index]; }//SEI JJ 
	UInt getDirectlyDependentViewId( UInt i, UInt j ) { return m_uiDirectlyDependentViewId[i][j]; }//SEI JJ
	UInt getViewDependencyInfoSrcOpId( UInt index ) { return m_uiViewDependencyInfoSrcOpId[index]; }//SEI JJ
	UInt getNumSeqParameterSetMinus1( UInt index ) { return m_uiNumSeqParameterSetMinus1[index]; }//SEI JJ
	UInt getSeqParameterSetIdDelta( UInt i, UInt j ) { return m_uiSeqParameterSetIdDelta[i][j]; } //SEI JJ
	UInt getNumPicParameterSetMinus1( UInt index ) { return m_uiNumPicParameterSetMinus1[index]; }//SEI JJ
	UInt getPicParameterSetIdDelta( UInt i, UInt j){ return m_uiPicParameterSetIdDelta[i][j]; }//SEI JJ 
	UInt getParameterSetsInfoSrcOpId( UInt index) { return m_uiParameterSetsInfoSrcOpId[index]; }//SEI JJ
    //{{SEI JJ
	UInt getNumSubsetSeqParameterSetMinus1(UInt index )	{ return m_uiNumSubsetSeqParameterSetMinus1[index];}
	UInt getSubsetSeqParameterSetIdDelta( UInt i,UInt j )  { return m_uiSubsetSeqParameterSetIdDelta[i][j];}
	Bool getBitstreamRestrictionInfoPresentFlag(UInt index ) {  return m_bBitstreamRestrictionInfoPresentFlag[index];}
	Bool getMotionVectorsOverPicBoundariesFlag(UInt index )  { return m_bMotionVectorsOverPicBoundariesFlag[index];}
	UInt getMaxBytesPerPicDenom(UInt index ) { return m_uiMaxBytesPerPicDenom[index];}
	UInt getMaxBitsPerMbDenom(UInt index ) { return m_uiMaxBitsPerMbDenom[index];}
	UInt getLog2MaxMvLengthHorizontal(UInt index ) { return m_uiLog2MaxMvLengthHorizontal[index];}
	UInt getLog2MaxMvLengthVertical(UInt index ) { return m_uiLog2MaxMvLengthVertical[index];}
	UInt getNumReorderFrames(UInt index ) { return m_uiNumReorderFrames[index];}
	UInt getMaxDecFrameBuffering(UInt index ) { return m_uiMaxDecFrameBuffering[index];}
	//}}SEI JJ

	Void setNumOperationPointsMinus1( UInt ui )		  { m_uiNumOperationPointsMinus1 = ui; }
	Void setOperationPointId( UInt index, UInt ui )	  { m_uiOperationPointId[index] = ui; }
	Void setPriorityId( UInt index, UInt ui )		  { m_uiPriorityId[index] = ui; }
	Void setTemporalId( UInt index, UInt ui )		  { m_uiTemporalId[index] = ui; }
//	Void setNumTargetOutputViewsMinus1( UInt index, UInt ui )  { m_uiNumTargetOutputViewsMinus1[index] = ui; }//SEI JJ 
	Void setViewId( UInt i, UInt j, UInt uiId )		  { m_uiViewId[i][j] = uiId; }
	Void setProfileLevelInfoPresentFlag( UInt index, Bool b ) { m_bProfileLevelInfoPresentFlag[index] = b; }
	Void setBitRateInfoPresentFlag( UInt index, Bool b )	  { m_bBitRateInfoPresentFlag[index] = b; }
	Void setFrmRateInfoPresentFlag( UInt index, Bool b )      { m_bFrmRateInfoPresentFlag[index] = b; }
	Void setViewDependencyInfoPresentFlag( UInt index, Bool b ) { m_bViewDependencyInfoPresentFlag[index] = b; }//SEI JJ 
	Void setParameterSetsInfoPresentFlag(UInt i, Bool b ) { m_bParameterSetsInfoPresentFlag[i] = b; }//SEI JJ 
	Void setOpProfileLevelIdc( UInt index, UInt ui )				  { m_uiOpProfileLevelIdc[index] = ui; }//SEI JJ
	Void setOpConstraintSet0Flag( UInt index, Bool b )		  { m_bOpConstraintSet0Flag[index] = b; }
	Void setOpConstraintSet1Flag( UInt index, Bool b )		  { m_bOpConstraintSet1Flag[index] = b; }
	Void setOpConstraintSet2Flag( UInt index, Bool b )		  { m_bOpConstraintSet2Flag[index] = b; }
	Void setOpConstraintSet3Flag( UInt index, Bool b )  	  { m_bOpConstraintSet3Flag[index] = b; }
//bug_fix_chenlulu{
	Void setOpConstraintSet4Flag( UInt index, Bool b )		  { m_bOpConstraintSet4Flag[index] = b; }
	Void setOpConstraintSet5Flag( UInt index, Bool b )		  { m_bOpConstraintSet5Flag[index] = b; }
//bug_fix_chenlulu}
	Void setOpLevelIdc( UInt index, UInt ui )			      { m_uiOpLevelIdc[index] = ui; }
	Void setProfileLevelInfoSrcOpIdDelta( UInt i, UInt ui )   { m_uiProfileLevelInfoSrcOpIdDelta[i] = ui; }
	Void setAvgBitrate( UInt index, UInt ui )				  { m_uiAvgBitrate[index] = ui; }
	Void setMaxBitrate( UInt index, UInt ui )				  { m_uiMaxBitrate[index] = ui; }
	Void setMaxBitrateCalcWindow( UInt index, UInt ui )		  { m_uiMaxBitrateCalcWindow[index] = ui; }
	Void setConstantFrmRateIdc( UInt index, UInt ui )		  { m_uiConstantFrmRateIdc[index] = ui; }
	Void setAvgFrmRate( UInt index, UInt ui )				  { m_uiAvgFrmRate[index] = ui; }
	Void setFrmRateInfoSrcOpIdDela( UInt index, UInt ui )     { m_uiFrmRateInfoSrcOpIdDela[index] = ui; }
//	Void setNumDirectlyDependentViews( UInt index, UInt ui )	  { m_uiNumDirectlyDependentViews[index] = ui; }//SEI JJ 
	Void setDirectlyDependentViewId( UInt i, UInt j, UInt ui ) { m_uiDirectlyDependentViewId[i][j] = ui; }//SEI JJ 
	Void setViewDependencyInfoSrcOpId( UInt index, UInt ui )     { m_uiViewDependencyInfoSrcOpId[index] = ui; }//SEI JJ
//	Void setNumSeqParameterSetMinus1( UInt index, UInt ui )     { m_uiNumSeqParameterSetMinus1[index] = ui; }//SEI JJ 
	Void setSeqParameterSetIdDelta( UInt i, UInt j, UInt ui )   { m_uiSeqParameterSetIdDelta[i][j] = ui; } //SEI JJ 
//	Void setNumPicParameterSetMinus1( UInt index, UInt ui )     { m_uiNumPicParameterSetMinus1[index] = ui; }//SEI JJ 
	Void setPicParameterSetIdDelta( UInt i, UInt j, UInt ui)    { m_uiPicParameterSetIdDelta[i][j] = ui; }//SEI JJ 
	Void setParameterSetsInfoSrcOpId( UInt index, UInt ui)		{ m_uiParameterSetsInfoSrcOpId[index] = ui; }//SEI JJ 
    //{{SEI JJ
	Void setNumSubsetSeqParameterSetMinus1(UInt index, UInt ui)	{ m_uiNumSubsetSeqParameterSetMinus1[index]=ui;}
	Void setSubsetSeqParameterSetIdDelta( UInt i,UInt j, UInt ui )  { m_uiSubsetSeqParameterSetIdDelta[i][j]=ui;}
	Void setBitstreamRestrictionInfoPresentFlag(UInt index,Bool b) {  m_bBitstreamRestrictionInfoPresentFlag[index]=b;}
	Void setMotionVectorsOverPicBoundariesFlag(UInt index,Bool b)  { m_bMotionVectorsOverPicBoundariesFlag[index]=b;}
	Void setMaxBytesPerPicDenom(UInt index,UInt ui) { m_uiMaxBytesPerPicDenom[index]=ui;}
	Void setMaxBitsPerMbDenom(UInt index,UInt ui) {m_uiMaxBitsPerMbDenom[index]=ui;}
	Void setLog2MaxMvLengthHorizontal(UInt index,UInt ui) { m_uiLog2MaxMvLengthHorizontal[index]=ui;}
	Void setLog2MaxMvLengthVertical(UInt index,UInt ui) { m_uiLog2MaxMvLengthVertical[index]=ui;}
	Void setNumReorderFrames(UInt index,UInt ui) { m_uiNumReorderFrames[index]=ui;}
	Void setMaxDecFrameBuffering(UInt index,UInt ui) {m_uiMaxDecFrameBuffering[index]=ui;}
	//}}SEI JJ

	Void setNumTargetOutputViewsMinus1( UInt index, UInt ui )  //SEI JJ
	{ 

  	  m_uiNumTargetOutputViewsMinus1[index] = ui; //SEI JJ
	  if( m_uiViewId[index] == NULL )
		m_uiViewId[index] = (UInt*)malloc((ui+1)*sizeof(UInt));
	}
	//{{SEI JJ
	Void setNumDirectlyDependentViews( UInt index, UInt ui )	 
	{ 
	  m_uiNumDirectlyDependentViews[index] = ui;
	  if( m_uiDirectlyDependentViewId[index] == NULL )
		  m_uiDirectlyDependentViewId[index] = (UInt*)malloc(ui*sizeof(UInt)); 
	}

	Void setNumSeqParameterSetMinus1( UInt index, UInt ui )   
	{ 
	  m_uiNumSeqParameterSetMinus1[index] = ui; 
	  if( m_uiSeqParameterSetIdDelta[index] == NULL )
	    m_uiSeqParameterSetIdDelta[index] = (UInt*)malloc((ui+1)*sizeof(UInt));
	}
	
	Void setNumPicParameterSetMinus1( UInt index, UInt ui ) 
	{ 
	  m_uiNumPicParameterSetMinus1[index] = ui; 
	  if( m_uiPicParameterSetIdDelta[index] == NULL )
	    m_uiPicParameterSetIdDelta[index] = (UInt*)malloc((ui+1)*sizeof(UInt));
	}
	//}}SEI JJ
  private:
	UInt			m_uiNumOperationPointsMinus1;

	UInt			m_uiOperationPointId[MAX_OPERATION_POINTS];
	UInt			m_uiPriorityId[MAX_OPERATION_POINTS];
	UInt			m_uiTemporalId[MAX_OPERATION_POINTS];
	UInt			m_uiNumTargetOutputViewsMinus1[MAX_OPERATION_POINTS];//SEI JJ

	UInt*			m_uiViewId[MAX_OPERATION_POINTS];
	Bool			m_bProfileLevelInfoPresentFlag[MAX_OPERATION_POINTS];
	Bool			m_bBitRateInfoPresentFlag[MAX_OPERATION_POINTS];
	Bool			m_bFrmRateInfoPresentFlag[MAX_OPERATION_POINTS];
	Bool			m_bViewDependencyInfoPresentFlag[MAX_OPERATION_POINTS];//SEI JJ 
	Bool			m_bParameterSetsInfoPresentFlag[MAX_OPERATION_POINTS];//SEI JJ 

	UInt			m_uiOpProfileLevelIdc[MAX_OPERATION_POINTS];//SEI JJ
	Bool			m_bOpConstraintSet0Flag[MAX_OPERATION_POINTS];
	Bool			m_bOpConstraintSet1Flag[MAX_OPERATION_POINTS];
	Bool			m_bOpConstraintSet2Flag[MAX_OPERATION_POINTS];
	Bool			m_bOpConstraintSet3Flag[MAX_OPERATION_POINTS];
//bug_fix_chenlulu{
	Bool			m_bOpConstraintSet4Flag[MAX_OPERATION_POINTS];
	Bool			m_bOpConstraintSet5Flag[MAX_OPERATION_POINTS];
//bug_fix_chenlulu}
	UInt			m_uiOpLevelIdc[MAX_OPERATION_POINTS];

	UInt			m_uiProfileLevelInfoSrcOpIdDelta[MAX_OPERATION_POINTS];

	UInt			m_uiAvgBitrate[MAX_OPERATION_POINTS];
	UInt			m_uiMaxBitrate[MAX_OPERATION_POINTS];
	UInt			m_uiMaxBitrateCalcWindow[MAX_OPERATION_POINTS];

	UInt			m_uiConstantFrmRateIdc[MAX_OPERATION_POINTS];
	UInt			m_uiAvgFrmRate[MAX_OPERATION_POINTS];

	UInt			m_uiFrmRateInfoSrcOpIdDela[MAX_OPERATION_POINTS];
    //{{SEI JJ
	UInt			m_uiNumDirectlyDependentViews[MAX_OPERATION_POINTS]; 
	UInt*			m_uiDirectlyDependentViewId[MAX_OPERATION_POINTS]; 

	UInt			m_uiViewDependencyInfoSrcOpId[MAX_OPERATION_POINTS]; 

	UInt			m_uiNumSeqParameterSetMinus1[MAX_OPERATION_POINTS];
	UInt*			m_uiSeqParameterSetIdDelta[MAX_OPERATION_POINTS]; 
	UInt			m_uiNumPicParameterSetMinus1[MAX_OPERATION_POINTS]; 
	UInt*			m_uiPicParameterSetIdDelta[MAX_OPERATION_POINTS]; 

	UInt			m_uiParameterSetsInfoSrcOpId[MAX_OPERATION_POINTS]; 

	UInt			m_uiNumSubsetSeqParameterSetMinus1[MAX_OPERATION_POINTS];
	UInt*           m_uiSubsetSeqParameterSetIdDelta[MAX_OPERATION_POINTS];
	Bool m_bBitstreamRestrictionInfoPresentFlag[MAX_SCALABLE_LAYERS]; 
	Bool m_bMotionVectorsOverPicBoundariesFlag[MAX_SCALABLE_LAYERS];
	UInt m_uiMaxBytesPerPicDenom[MAX_SCALABLE_LAYERS];
	UInt m_uiMaxBitsPerMbDenom[MAX_SCALABLE_LAYERS];
	UInt m_uiLog2MaxMvLengthHorizontal[MAX_SCALABLE_LAYERS];
	UInt m_uiLog2MaxMvLengthVertical[MAX_SCALABLE_LAYERS];
	UInt m_uiNumReorderFrames[MAX_SCALABLE_LAYERS];
	UInt m_uiMaxDecFrameBuffering[MAX_SCALABLE_LAYERS];
	//}}SEI JJ
  };

#define MAX_PICTURES_IN_ACCESS_UNIT 50
  class H264AVCCOMMONLIB_API ScalableNestingSei : public SEIMessage
  {
  protected:
    ScalableNestingSei()
      : SEIMessage(SCALABLE_NESTING_SEI)
      , m_bAllPicturesInAuFlag  (0)
    , m_uiNumPicturesMinus1     (0)
	, m_uiTemporalId		    (0)
    , m_pcSEIMessage          (NULL)
    {}

  public:
    static ErrVal create( ScalableNestingSei*&  rpcSEIMessage );
    ErrVal      destroy();
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        init  ( Bool                  m_bAllPicturesInAuFlag,
                        UInt                  m_uiNumPicturesMinus1,
						UInt				  m_uiTemporalId,
                        UInt*                 m_uiPicId
            );

    Bool getAllPicturesInAuFlag()  const { return m_bAllPicturesInAuFlag; }
    UInt getNumPicturesMinus1()          const { return m_uiNumPicturesMinus1; }
    UInt getPicId( UInt uiIndex ) { return m_uiPicId[uiIndex]; }
	UInt getTemporalId()		const { return m_uiTemporalId; }
	SEIMessage *getSEIMessage()   { return m_pcSEIMessage;}
  
    Void setAllPicturesInAuFlag( Bool bFlag ) { m_bAllPicturesInAuFlag = bFlag; }
    Void setNumPicturesMinus1( UInt uiNum ) { m_uiNumPicturesMinus1 = uiNum; }
    Void setPicId( UInt uiIndex, UInt uiValue ) { m_uiPicId[uiIndex] = uiValue; }
    Void setTemporalId( UInt uiValue ) { m_uiTemporalId= uiValue; }
 
  private:
    Bool  m_bAllPicturesInAuFlag;
    UInt  m_uiNumPicturesMinus1;
	UInt  m_uiTemporalId;
    UInt  m_uiPicId[MAX_PICTURES_IN_ACCESS_UNIT];
    SEIMessage *m_pcSEIMessage;
  };
  //full-frame snapshot SEI message is taken as en example
  class H264AVCCOMMONLIB_API FullframeSnapshotSei : public SEIMessage
  {
  protected:
    FullframeSnapshotSei() : SEIMessage(FULLFRAME_SNAPSHOT_SEI)
    {}
  public:
    static ErrVal create( FullframeSnapshotSei*& rpcFullframeSnapshotSei );
    ErrVal    destroy ();
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf);
      ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );

    UInt getSnapShotId()              const { return m_uiSnapShotId; }
    Void setSnapShotId( UInt uiSnapShotId )                   { m_uiSnapShotId = uiSnapShotId; }
  private:
    UInt m_uiSnapShotId;
  };


 class H264AVCCOMMONLIB_API MultiviewSceneInfoSei : public SEIMessage // SEI JVT-W060
 {
 protected:
   MultiviewSceneInfoSei() 
	 : SEIMessage(MULTIVIEW_SCENE_INFO_SEI)
	 , m_uiMaxDisparity	   (0)
	 {}
 public:
   static ErrVal create( MultiviewSceneInfoSei*& rpcSeiMessage );
   ErrVal    destroy ();
   ErrVal	 write ( HeaderSymbolWriteIf* pcWriteIf );
   ErrVal	 read ( HeaderSymbolReadIf*   pcReadIf );
   
   UInt getMaxDisparity()		const { return m_uiMaxDisparity; }
 
   Void setMaxDisparity( UInt uiId )		{ m_uiMaxDisparity = uiId; }
 
 private:
   UInt	    m_uiMaxDisparity;
 };

class H264AVCCOMMONLIB_API MultiviewAcquisitionInfoSei : public SEIMessage // SEI JVT-W060
 {
 protected:
   MultiviewAcquisitionInfoSei()
   : SEIMessage(MULTIVIEW_ACQUISITION_INFO_SEI)
	,m_bIntrinsicParamFlag(false)
	,m_bIntrinsicParamsEqual(false)
	,m_bExtrinsicParamFlag(false)
   {}
	~MultiviewAcquisitionInfoSei()	 
	{
		release_memory();

	}
 public:
   static ErrVal create( MultiviewAcquisitionInfoSei*& rpcSeiMessage );
   ErrVal    destroy ();
   ErrVal	 write ( HeaderSymbolWriteIf* pcWriteIf );
   ErrVal	 read ( HeaderSymbolReadIf*   pcReadIf );
   
   
   void initialize_memory(UInt num_of_views)
   {
	   UInt i,j;

		m_bSignFocalLengthX = new Bool[num_of_views];
		m_bSignFocalLengthY = new Bool[num_of_views];
		m_bSignPrincipalPointX = new Bool[num_of_views];
		m_bSignPrincipalPointY = new Bool[num_of_views];
		m_bSignRadialDistortion = new Bool[num_of_views];

		m_uiExponentFocalLengthX = new UInt[num_of_views];
		m_uiExponentFocalLengthY = new UInt[num_of_views];
		m_uiExponentPrincipalPointX = new UInt[num_of_views];
		m_uiExponentPrincipalPointY = new UInt[num_of_views];
		m_uiExponentRadialDistortion = new UInt[num_of_views];

		m_uiMantissaFocalLengthX = new UInt[num_of_views];
		m_uiMantissaFocalLengthY = new UInt[num_of_views];
		m_uiMantissaPrincipalPointX = new UInt[num_of_views];
		m_uiMantissaPrincipalPointY = new UInt[num_of_views];
		m_uiMantissaRadialDistortion = new UInt[num_of_views];

		m_bSignRotationParam = new Bool**[num_of_views];		
		for (i=0;i<num_of_views;i++) {
			m_bSignRotationParam[i] = new Bool*[3];							
			for (j=0;j<3;j++)
				m_bSignRotationParam[i][j]= new Bool[3];
		}
		m_bSignTranslationParam= new Bool*[num_of_views];
		for (i=0;i<num_of_views;i++) 
			m_bSignTranslationParam[i] = new Bool[3];										

		
		m_uiExponentRotationParam = new UInt**[num_of_views];		
		for (i=0;i<num_of_views;i++) {
			m_uiExponentRotationParam[i] = new UInt*[3];							
			for (j=0;j<3;j++)
				m_uiExponentRotationParam[i][j]= new UInt[3];
		}

		m_uiExponentTranslationParam= new UInt*[num_of_views];
		for (i=0;i<num_of_views;i++) 
			m_uiExponentTranslationParam[i] = new UInt[3];	

		
		m_uiMantissaRotationParam = new UInt**[num_of_views];		
		for (i=0;i<num_of_views;i++) {
			m_uiMantissaRotationParam[i] = new UInt*[3];							
			for (j=0;j<3;j++)
				m_uiMantissaRotationParam[i][j]= new UInt[3];
		}

		m_uiMantissaTranslationParam= new UInt*[num_of_views];
		for (i=0;i<num_of_views;i++) 
			m_uiMantissaTranslationParam[i] = new UInt[3];	


				
		m_uiNumViewMinus1=num_of_views-1;
   }
   void release_memory()
   {
	    int i,j;
		int num_of_views=m_uiNumViewMinus1+1;

		delete [] m_bSignFocalLengthX;
		delete [] m_bSignFocalLengthY;
		delete [] m_bSignPrincipalPointX;
		delete [] m_bSignPrincipalPointY;
		delete [] m_bSignRadialDistortion;

		delete [] m_uiExponentFocalLengthX;
		delete [] m_uiExponentFocalLengthY;
		delete [] m_uiExponentPrincipalPointX;
		delete [] m_uiExponentPrincipalPointY;
		delete [] m_uiExponentRadialDistortion;

		delete [] m_uiMantissaFocalLengthX;
		delete [] m_uiMantissaFocalLengthY;
		delete [] m_uiMantissaPrincipalPointX;
		delete [] m_uiMantissaPrincipalPointY;
		delete [] m_uiMantissaRadialDistortion;
			
		for (i=0;i<num_of_views;i++) {									
			for (j=0;j<3;j++)
				delete [] m_bSignRotationParam[i][j];
			delete [] m_bSignRotationParam[i];
		}
		delete [] m_bSignRotationParam;
		
		for (i=0;i<num_of_views;i++) 
			delete [] m_bSignTranslationParam[i];
		delete [] m_bSignTranslationParam;


		for (i=0;i<num_of_views;i++) {									
			for (j=0;j<3;j++)
				delete [] m_uiExponentRotationParam[i][j];
			delete [] m_uiExponentRotationParam[i];
		}
		delete [] m_uiExponentRotationParam;		

		for (i=0;i<num_of_views;i++) 
			delete [] m_uiExponentTranslationParam[i];
		delete [] m_uiExponentTranslationParam;

		for (i=0;i<num_of_views;i++) {									
			for (j=0;j<3;j++)
				delete [] m_uiMantissaRotationParam[i][j];
			delete [] m_uiMantissaRotationParam[i];
		}
		delete [] m_uiMantissaRotationParam;
		
		for (i=0;i<num_of_views;i++) 
			delete [] m_uiMantissaTranslationParam[i];
		delete [] m_uiMantissaTranslationParam;
			
   }

   UInt getVarLength(UInt E, UInt Prec_SE) const {
		if (E==0)
			return max(0, -30+Prec_SE);
		else 
			return max(0, E-31+Prec_SE);
   }

   ErrVal getReconstSceneAcqSEI(UInt E, UInt Sign, UInt Bin_mantissa, UInt V, double *Recon)
   {
		int i,result=0;
		int sgn= (Sign==0) ? 1 : -1;

		*Recon=0.0;
		double factor=(double)(1.0)/(double)(1<<V);
		for (i=0;i<(int)V;i++) {
			*Recon += factor*((Bin_mantissa>>i)&0x01);
			factor *= (double)2.0;
		}
		if (E>0 && E<63)
			*Recon = (double)sgn*(double)pow((double)2,(double)E-31)*(1.0+*Recon);
		else if (E==63)
			result= -1; // invalid entry
		else if (E==0)
			*Recon = (double)sgn*(double)pow((double)2,(double)-30)*(*Recon);	
		
		return result;

   }
   UInt getNumViewMinus1() const {return m_uiNumViewMinus1;}
   Bool getIntrinsicParamFlag()	const {return m_bIntrinsicParamFlag;}
   Bool getIntrinsicParamsEqual()	const {return m_bIntrinsicParamsEqual;}
   UInt getPrecFocalLength()	const {return m_uiPrecFocalLength;}	
   UInt getPrecPrincipalPoint()	const {return m_uiPrecPrincipalPoint;}
   UInt getPrecRadialDistortion()	const {return m_uiPrecRadialDistortion;}
   UInt getPrecRotationParam()	const {return m_uiPrecRotationParam;}	
   UInt getPrecTranslationParam()	const {return m_uiPrecTranslationParam;}


   Bool getSignFocalLengthX(UInt uiIndex) const {return m_bSignFocalLengthX[uiIndex];}
   Bool getSignFocalLengthY(UInt uiIndex) const {return m_bSignFocalLengthY[uiIndex];}
   Bool getSignPrincipalPointX(UInt uiIndex) const {return m_bSignPrincipalPointX[uiIndex];}
   Bool getSignPrincipalPointY(UInt uiIndex) const {return m_bSignPrincipalPointY[uiIndex];}
   Bool getSignRadialDistortion(UInt uiIndex) const {return m_bSignRadialDistortion[uiIndex];}

   UInt getExponentFocalLengthX(UInt uiIndex) const {return m_uiExponentFocalLengthX[uiIndex];}
   UInt getExponentFocalLengthY(UInt uiIndex) const {return m_uiExponentFocalLengthY[uiIndex];}
   UInt getExponentPrincipalPointX(UInt uiIndex) const {return m_uiExponentPrincipalPointX[uiIndex];}
   UInt getExponentPrincipalPointY(UInt uiIndex) const {return m_uiExponentPrincipalPointY[uiIndex];}
   UInt getExponentRadialDistortion(UInt uiIndex) const {return m_uiExponentRadialDistortion[uiIndex];}

   UInt getMantissaFocalLengthX(UInt uiIndex) const {return m_uiMantissaFocalLengthX[uiIndex];}
   UInt getMantissaFocalLengthY(UInt uiIndex) const {return m_uiMantissaFocalLengthY[uiIndex];}
   UInt getMantissaPrincipalPointX(UInt uiIndex) const {return m_uiMantissaPrincipalPointX[uiIndex];}
   UInt getMantissaPrincipalPointY(UInt uiIndex) const {return m_uiMantissaPrincipalPointY[uiIndex];}
   UInt getMantissaRadialDistortion(UInt uiIndex) const {return m_uiMantissaRadialDistortion[uiIndex];}

   Bool getExtrinsicParamFlag()	const {return m_bExtrinsicParamFlag;}
   Bool getSignRotationParam(UInt uiIndex, UInt RowIdx, UInt ColIdx) const { return m_bSignRotationParam[uiIndex][RowIdx][ColIdx];}
   UInt getExponentRotationParam(UInt uiIndex, UInt RowIdx, UInt ColIdx) const { return m_uiExponentRotationParam[uiIndex][RowIdx][ColIdx];}
   UInt getMantissaRotationParam(UInt uiIndex, UInt RowIdx, UInt ColIdx) const { return m_uiMantissaRotationParam[uiIndex][RowIdx][ColIdx];}
     
   Bool getSignTranslationParam(UInt uiIndex, UInt RowIdx) const { return m_bSignTranslationParam[uiIndex][RowIdx];}
   UInt getExponentTranslationParam(UInt uiIndex, UInt RowIdx) const { return m_uiExponentTranslationParam[uiIndex][RowIdx];}
   UInt getMantissaTranslationParam(UInt uiIndex, UInt RowIdx) const { return m_uiMantissaTranslationParam[uiIndex][RowIdx];}
   
   	   			
   void setIntrinsicParamFlag(Bool IntParamFlag)	{ m_bIntrinsicParamFlag=IntParamFlag;}
   void setIntrinsicParamsEqual(Bool IntParamEqual)	{ m_bIntrinsicParamsEqual=IntParamEqual;}
   void setPrecFocalLength(UInt PrecFocalLength)	{ m_uiPrecFocalLength=PrecFocalLength;}
   void setPrecPrincipalPoint(UInt PrecPrincipalPoint)	{ m_uiPrecPrincipalPoint=PrecPrincipalPoint;}
   void setPrecRadialDistortion(UInt PrecRadialDistortion)	{ m_uiPrecRadialDistortion=PrecRadialDistortion;}
   void setPrecRotationParam(UInt PrecRotationParam)	{ m_uiPrecRotationParam=PrecRotationParam;}
   void setPrecTranslationParam(UInt PrecTranslationParam)	{ m_uiPrecTranslationParam=PrecTranslationParam;}

   void setSignFocalLengthX(UInt uiIndex, Bool value) {m_bSignFocalLengthX[uiIndex]=value;}
   void setSignFocalLengthY(UInt uiIndex, Bool value) {m_bSignFocalLengthY[uiIndex]=value;}
   void setSignPrincipalPointX(UInt uiIndex, Bool value) { m_bSignPrincipalPointX[uiIndex]=value;}
   void setSignPrincipalPointY(UInt uiIndex, Bool value){ m_bSignPrincipalPointY[uiIndex]=value;}
   void setSignRadialDistortion(UInt uiIndex, Bool value){ m_bSignRadialDistortion[uiIndex]=value;}

   void setExponentFocalLengthX(UInt uiIndex, UInt value) {m_uiExponentFocalLengthX[uiIndex]=value;}
   void setExponentFocalLengthY(UInt uiIndex, UInt value) {m_uiExponentFocalLengthY[uiIndex]=value;}
   void setExponentPrincipalPointX(UInt uiIndex, UInt value) { m_uiExponentPrincipalPointX[uiIndex]=value;}
   void setExponentPrincipalPointY(UInt uiIndex, UInt value){ m_uiExponentPrincipalPointY[uiIndex]=value;}
   void setExponentRadialDistortion(UInt uiIndex, UInt value){ m_uiExponentRadialDistortion[uiIndex]=value;}

   void setMantissaFocalLengthX(UInt uiIndex, UInt value) {m_uiMantissaFocalLengthX[uiIndex]=value;}
   void setMantissaFocalLengthY(UInt uiIndex, UInt value) {m_uiMantissaFocalLengthY[uiIndex]=value;}
   void setMantissaPrincipalPointX(UInt uiIndex, UInt value) { m_uiMantissaPrincipalPointX[uiIndex]=value;}
   void setMantissaPrincipalPointY(UInt uiIndex, UInt value){ m_uiMantissaPrincipalPointY[uiIndex]=value;}
   void setMantissaRadialDistortion(UInt uiIndex, UInt value){ m_uiMantissaRadialDistortion[uiIndex]=value;}

   void setExtrinsicParamFlag(Bool ExtParamFlag)	{ m_bExtrinsicParamFlag=ExtParamFlag;}
   void setSignRotationParam(UInt uiIndex, UInt RowIdx, UInt ColIdx, Bool value) {m_bSignRotationParam[uiIndex][RowIdx][ColIdx]=value;}
   void setExponentRotationParam(UInt uiIndex, UInt RowIdx, UInt ColIdx, UInt value) {m_uiExponentRotationParam[uiIndex][RowIdx][ColIdx]=value;}
   void setMantissaRotationParam(UInt uiIndex, UInt RowIdx, UInt ColIdx, UInt value) {m_uiMantissaRotationParam[uiIndex][RowIdx][ColIdx]=value;}
   
   void setSignTranslationParam(UInt uiIndex, UInt RowIdx, Bool value) {m_bSignTranslationParam[uiIndex][RowIdx]=value;}
   void setExponentTranslationParam(UInt uiIndex, UInt RowIdx,UInt value) {m_uiExponentTranslationParam[uiIndex][RowIdx]=value;}
   void setMantissaTranslationParam(UInt uiIndex, UInt RowIdx,UInt value) {m_uiMantissaTranslationParam[uiIndex][RowIdx]=value;}   	

   
 private:
   UInt		m_uiNumViewMinus1;	
   Bool		m_bIntrinsicParamFlag;
   Bool		m_bIntrinsicParamsEqual;
   UInt		m_uiPrecFocalLength;
   UInt		m_uiPrecPrincipalPoint;
   UInt		m_uiPrecRadialDistortion;
   UInt		m_uiPrecRotationParam;
   UInt		m_uiPrecTranslationParam;
   
   Bool	    *m_bSignFocalLengthX;
   Bool	    *m_bSignFocalLengthY;
   Bool	    *m_bSignPrincipalPointX;
   Bool	    *m_bSignPrincipalPointY;
   Bool	    *m_bSignRadialDistortion;

   UInt	    *m_uiExponentFocalLengthX;
   UInt	    *m_uiExponentFocalLengthY;
   UInt	    *m_uiExponentPrincipalPointX;
   UInt	    *m_uiExponentPrincipalPointY;
   UInt	    *m_uiExponentRadialDistortion;

   UInt	    *m_uiMantissaFocalLengthX;
   UInt	    *m_uiMantissaFocalLengthY;
   UInt	    *m_uiMantissaPrincipalPointX;
   UInt	    *m_uiMantissaPrincipalPointY;
   UInt	    *m_uiMantissaRadialDistortion;
   Bool		m_bExtrinsicParamFlag;

   Bool		***m_bSignRotationParam;
   Bool		**m_bSignTranslationParam;

   UInt		***m_uiExponentRotationParam;
   UInt		**m_uiExponentTranslationParam;

   UInt		***m_uiMantissaRotationParam;
   UInt		**m_uiMantissaTranslationParam;	

   
 };


 //JVT-AB025 {{
 class H264AVCCOMMONLIB_API NonReqViewInfoSei:public SEIMessage
 {
 protected:
   NonReqViewInfoSei();
   ~NonReqViewInfoSei();
 public:
   static ErrVal create( NonReqViewInfoSei*& rpcNonReqViewInfoSei);
   ErrVal write( HeaderSymbolWriteIf* pcWriteIf);
   ErrVal read ( HeaderSymbolReadIf* pcReadIf);
   ErrVal init ( UInt uiNumOfTargetViewMinus1
     ,UInt* puiViewOrderIndex);
   UInt   getNumTargetViewMinus1()  const { return m_uiNumOfTargetViewMinus1;}
   UInt*  getTargetViewOrderIndex() const { return m_puiViewOrderIndex;}
   UInt*  getNumNonReqViewCopMinus1() const { return m_puiNumNonReqViewCopMinus1;}
   UInt** getindexDeltaMinus1()     const { return m_ppuiIndexDeltaMinus1;}

 private:
   UInt m_uiNumOfTargetViewMinus1;
   UInt *m_puiViewOrderIndex;
   UInt *m_puiNumNonReqViewCopMinus1;
   UInt **m_ppuiNonReqViewOrderIndex;
   UInt **m_ppuiIndexDeltaMinus1;
 };
 class H264AVCCOMMONLIB_API ViewDependencyStructureSei:public SEIMessage
 {
 protected:
   ViewDependencyStructureSei();
   ~ViewDependencyStructureSei();
 public:
   static ErrVal create( ViewDependencyStructureSei*& rpcViewDepStruSei);
   ErrVal write( HeaderSymbolWriteIf* pcWriteIf );
   ErrVal read ( HeaderSymbolReadIf*  pcReadIf);
   ErrVal init ( UInt uiNumOfViews
     ,UInt* puinum_refs_list0_anc
     ,UInt* puinum_refs_list1_anc
     ,UInt* puinum_refs_list0_nonanc
     ,UInt* puinum_refs_list1_nonanc
     ,Bool bEnc_Dec_Flag); // True:encder False:decoder

   UInt  getSeqParameterSetId() { return m_uiSeqParameterSetId;}//SEI JJ
   Bool  getAnchorUpdateFlag()  const { return m_bAnchorUpdateFlag;}
   Bool  getNonAnchorUpdateFlag()  const { return m_bNonAnchorUpdateFlag;}
   Void  setSeqParameterSetId( UInt i ) { m_uiSeqParameterSetId=i; }  //SEI JJ
   Void  setAnchorUpdateFlag( Bool flag ){ m_bAnchorUpdateFlag = flag;}
   Void  setNonAnchorUpdateFlag( Bool flag){ m_bNonAnchorUpdateFlag = flag;}
   Bool** getAnchorRefL0Flag()  const { return m_ppbAnchorRefL0Flag;}
   Bool** getAnchorRefL1Flag()  const { return m_ppbAnchorRefL1Flag;}
   Bool** getNonAnchorRefL0Flag() const{ return m_ppbNonAnchorRefL0Flag;}
   Bool** getNonAnchorRefL1Flag() const{ return m_ppbNonAnchorRefL1Flag;}
 private:
   UInt m_uiNumOfViews;
   UInt *m_puiNumAnchorL0Refs;
   UInt *m_puiNumAnchorL1Refs;
   UInt *m_puiNumNonAnchorL0Refs;
   UInt *m_puiNumNonAnchorL1Refs;
   Bool m_bAnchorUpdateFlag;
   Bool m_bNonAnchorUpdateFlag;
   UInt m_uiSeqParameterSetId;//SEI JJ
   Bool **m_ppbAnchorRefL0Flag;
   Bool **m_ppbAnchorRefL1Flag;
   Bool **m_ppbNonAnchorRefL0Flag;
   Bool **m_ppbNonAnchorRefL1Flag;
 };
 class H264AVCCOMMONLIB_API OPNotPresentSei : public SEIMessage
 {
 protected:
   OPNotPresentSei();
   ~OPNotPresentSei();
 public:
   static ErrVal create( OPNotPresentSei*& rpcSeiMessage);
   ErrVal destroy();
   ErrVal write( HeaderSymbolWriteIf * pcWriteIf);
   ErrVal read( HeaderSymbolReadIf* pcReadIf);
   ErrVal init( UInt uiNum );
   Void   setNumNotPresentOP( UInt uiNumNotPresentOP ) { m_uiNumNotPresentOP = uiNumNotPresentOP;}
   UInt   getNumNotPresentOP() { return m_uiNumNotPresentOP;}
   Void   setNotPresentOPId( UInt* OperationPointId)
   {
     for (UInt i = 0; i< m_uiNumNotPresentOP; i++)
     {
       m_uiNotPresentOPId[i] = OperationPointId[i];
     }
   }
   UInt*  getNotPresentOPId(){ return m_uiNotPresentOPId;}
 private:
   UInt  m_uiNumNotPresentOP;
   UInt* m_uiNotPresentOPId;
 };
 //JVT-AB025 }}
  typedef MyList<SEIMessage*> MessageList;
  static ErrVal writeNesting        ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      MessageList*          rpcSEIMessageList );
  static ErrVal xWriteNesting       ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      SEIMessage*           pcSEIMessage,
                    UInt&                 uiBits );

  
 // static ErrVal read  ( HeaderSymbolReadIf*   pcReadIf,
 //                       MessageList&          rcSEIMessageList );
  static ErrVal read  ( HeaderSymbolReadIf*   pcReadIf,
                        MessageList&          rcSEIMessageList /*, UInt NumViewsMinus1 =0 */ ); // SEI JVT-W060 Nov. 30
  static ErrVal write ( HeaderSymbolWriteIf*  pcWriteIf,
                        HeaderSymbolWriteIf*  pcWriteTestIf,
                        MessageList*          rpcSEIMessageList );

protected:
  //static ErrVal xRead               ( HeaderSymbolReadIf*   pcReadIf,
  //                                    SEIMessage*&          rpcSEIMessage ); 
  static ErrVal xRead               ( HeaderSymbolReadIf*   pcReadIf,
                                      SEIMessage*&          rpcSEIMessage /*, UInt NumViewsMinus1*/ );  // SEI JVT-W060 Nov. 30
  static ErrVal xWrite              ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      SEIMessage*           pcSEIMessage );
  static ErrVal xWritePayloadHeader ( HeaderSymbolWriteIf*  pcWriteIf,
                                      MessageType           eMessageType,
                                      UInt                  uiSize );
  static ErrVal xReadPayloadHeader  ( HeaderSymbolReadIf*   pcReadIf,
                                      MessageType&          reMessageType,
                                      UInt&                 ruiSize);
  static ErrVal xCreate             ( SEIMessage*&          rpcSEIMessage,
                                      MessageType           eMessageType,
                                      UInt                  uiSize ); 
public:


};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

H264AVC_NAMESPACE_END


#endif // !defined(AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_)

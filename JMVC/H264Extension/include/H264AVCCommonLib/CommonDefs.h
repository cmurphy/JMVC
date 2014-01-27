#if !defined(AFX_COMMONDEFS_H__4CE634CE_B48D_4812_8098_9CAEA258BAA2__INCLUDED_)
#define AFX_COMMONDEFS_H__4CE634CE_B48D_4812_8098_9CAEA258BAA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _JSVM_VERSION_ "6.5" //added by jerome.vieron@thomson.net
#define _JMVM_VERSION_ "7.0" //added by purvin.pandit@thomson.net
#define _JMVC_VERSION_ "8.5" //added by ying.chen@tut.fi, cheny@qualcomm.com

#define MB_BUFFER_WIDTH 24
#define MB_BASE_WIDTH   16

#define DC_PRED         2
#define OUTSIDE         -1
#define DOUBLE_MAX      1.7e+308


#define NO_LEFT_REF        1
#define NO_ABOVE_REF       2
#define NO_ABOVELEFT_REF   4
#define NO_ABOVERIGHT_REF  8


H264AVC_NAMESPACE_BEGIN

enum PicType
{
	NOT_SPECIFIED   = 0x00,
	TOP_FIELD       = 0x01,
	BOT_FIELD       = 0x02,
	FRAME           = 0x03,
	MAX_FRAME_TYPE  = 0x04
};

#if JM_MVC_COMPATIBLE
#define DELTA_POC 0//lufeng: offset for negtive poc
#endif

enum ParIdx16x16
{
  PART_16x16   = 0x00
};
enum ParIdx16x8
{
  PART_16x8_0   = 0x00,
  PART_16x8_1   = 0x08
};
enum ParIdx8x16
{
  PART_8x16_0   = 0x00,
  PART_8x16_1   = 0x02
};
enum Par8x8
{
  B_8x8_0    = 0x00,
  B_8x8_1    = 0x01,
  B_8x8_2    = 0x02,
  B_8x8_3    = 0x03
};
enum ParIdx8x8
{
  PART_8x8_0    = 0x00,
  PART_8x8_1    = 0x02,
  PART_8x8_2    = 0x08,
  PART_8x8_3    = 0x0A
};
enum SParIdx8x8
{
  SPART_8x8   = 0x00
};
enum SParIdx8x4
{
  SPART_8x4_0   = 0x00,
  SPART_8x4_1   = 0x04
};
enum SParIdx4x8
{
  SPART_4x8_0   = 0x00,
  SPART_4x8_1   = 0x01
};
enum SParIdx4x4
{
  SPART_4x4_0   = 0x00,
  SPART_4x4_1   = 0x01,
  SPART_4x4_2   = 0x04,
  SPART_4x4_3   = 0x05
};

enum NeighbourBlock
{
  CURR_MB_LEFT_NEIGHBOUR   = -1,
  LEFT_MB_LEFT_NEIGHBOUR   = +3,
  CURR_MB_ABOVE_NEIGHBOUR  = -4,
  ABOVE_MB_ABOVE_NEIGHBOUR = +12,
  CURR_MB_RIGHT_NEIGHBOUR  = +1,
  RIGHT_MB_RIGHT_NEIGHBOUR = -3
};
enum ListIdx
{
  LIST_0 = 0x00,
  LIST_1 = 0x01
};

enum ProcessingState
{
  PRE_PROCESS     = 0,
  PARSE_PROCESS   = 1,
  DECODE_PROCESS  = 2,
  ENCODE_PROCESS  = 3,
  POST_PROCESS    = 4
};


enum SliceType
{
  P_SLICE  = 0,
  B_SLICE  = 1,
  I_SLICE  = 2,
  F_SLICE  = 3
};


enum NalRefIdc
{
  NAL_REF_IDC_PRIORITY_LOWEST  = 0,
  NAL_REF_IDC_PRIORITY_LOW     = 1,
  NAL_REF_IDC_PRIORITY_HIGH    = 2,
  NAL_REF_IDC_PRIORITY_HIGHEST = 3
};

enum NalUnitType
{
  NAL_UNIT_EXTERNAL                 = 0,
  NAL_UNIT_CODED_SLICE              = 1,
  NAL_UNIT_CODED_SLICE_DATAPART_A   = 2,
  NAL_UNIT_CODED_SLICE_DATAPART_B   = 3,
  NAL_UNIT_CODED_SLICE_DATAPART_C   = 4,
  NAL_UNIT_CODED_SLICE_IDR          = 5,
  NAL_UNIT_SEI                      = 6,
  NAL_UNIT_SPS                      = 7,
  NAL_UNIT_PPS                      = 8,
  NAL_UNIT_ACCESS_UNIT_DELIMITER    = 9,
  NAL_UNIT_END_OF_SEQUENCE          = 10,
  NAL_UNIT_END_OF_STREAM            = 11,
  NAL_UNIT_FILLER_DATA              = 12,
  NAL_UNIT_SUBSET_SPS               = 15,

  NAL_UNIT_CODED_SLICE_PREFIX       = 14,

  NAL_UNIT_CODED_SLICE_SCALABLE     = 20,
  NAL_UNIT_CODED_SLICE_IDR_SCALABLE = 21  
};


enum MbMode
{
  MODE_SKIP         = 0,
  MODE_16x16        = 1,
  MODE_16x8         = 2,
  MODE_8x16         = 3,
  MODE_8x8          = 4,
  MODE_8x8ref0      = 5,
  INTRA_4X4         = 6,
  MODE_PCM          = 25+6,
  INTRA_BL          = 36
};

enum BlkMode
{
  BLK_8x8   = 8,
  BLK_8x4   = 9,
  BLK_4x8   = 10,
  BLK_4x4   = 11,
  BLK_SKIP  = 0
};

enum Profile
{
  BASELINE_PROFILE  = 66,
  MAIN_PROFILE      = 77,
  EXTENDED_PROFILE  = 88,

  HIGH_PROFILE      = 100,
  HIGH_10_PROFILE   = 110,
  HIGH_422_PROFILE  = 122,
  HIGH_444_PROFILE  = 144,
  HIGH_444_INTRA_PROFILE = 244,

  MULTI_VIEW_PROFILE= 118,
  STEREO_HIGH_PROFILE= 128,

  SCALABLE_PROFILE  = 83
};

enum DFunc
{
  DF_SAD      = 0,
  DF_SSD      = 1,
  DF_HADAMARD = 2,
  DF_YUV_SAD  = 3
};

enum SearchMode
{
  BLOCK_SEARCH  = 0,
  SPIRAL_SEARCH = 1,
  LOG_SEARCH    = 2,
  FAST_SEARCH   = 3
};


H264AVC_NAMESPACE_END



#define MIN_QP              0
#define MAX_QP              51
#define QP_BITS             15
#define QP_SHIFT1           12
#define MAX_FRAME_NUM_LOG2  9

#define YUV_X_MARGIN        32

#define YUV_Y_MARGIN        128

#define MAX_LAYERS          8
#define MAX_TEMP_LEVELS     8
#define MAX_QUALITY_LEVELS  4
#define MAX_FGS_LAYERS      3
#define MAX_DSTAGES         6
#define MAX_DSTAGES_MVC		10	//SEI LSJ
#define	MAX_FRAMERATE		120  //SEI LSJ
#define LOG2_GOP_ID_WRAP    4
#define PRI_ID_BITS         6
#define MAX_SCALABLE_LAYERS MAX_LAYERS * MAX_TEMP_LEVELS * MAX_QUALITY_LEVELS

//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
#define MAX_NUM_RD_LEVELS      50
//}}Quality level estimation and modified truncation- JVTO044 and m12007
#define MAX_SIZE_PID 64

// heiko.schwarz@hhi.fhg.de: Hack for ensuring that the scaling factors
// work with the closed-loop config files
// and the other available config files
// SHOULD BE REMOVED in the future
#define SCALING_FACTOR_HACK 1

#define MAX_NUM_INFO_ENTRIES 8
#define MAX_NUM_NON_REQUIRED_PICS 32

#define AR_FGS_MAX_BASE_WEIGHT                        32
#define AR_FGS_BASE_WEIGHT_SHIFT_BITS                 5

// default values
#define AR_FGS_DEFAULT_LOW_PASS_ENH_REF               0.0
#define AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_BLOCK         0
#define AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_COEFF         0
#define AR_FGS_DEFAULT_ENC_STRUCTURE                  1

#define AR_FGS_MC_INTERP_AVC                          0
#define AR_FGS_MC_INTERP_BILINEAR                     1
#define AR_FGS_MC_INTERP_4_TAP                        2
#define AR_FGS_DEFAULT_FILTER                         AR_FGS_MC_INTERP_BILINEAR
#define AR_FGS_COMPENSATE_SIGNED_FRAME                1

#define MVC_PROFILE                                   MULTI_VIEW_PROFILE  // ( MULTI_VIEW_PROFILE or HIGH_PROFILE )
#define WEIGHTED_PRED_FLAG                            0                   // (0:no weighted prediction, 1:random weights)
#define WEIGHTED_BIPRED_IDC                           0                   // (0:no weighted bi-prediction, 1:random weights, 2:implicit weights)
#define INFER_ELAYER_PRED_WEIGHTS                     0                   // (0:BL weights are not used, 1:infer enhancement layer prediction weights)

//TMM_EC {{
typedef	enum
{
	EC_NONE												=	100,
  EC_BLSKIP,
	EC_RECONSTRUCTION_UPSAMPLE,
	EC_FRAME_COPY,
	EC_TEMPORAL_DIRECT,
	EC_INTRA_COPY
}	ERROR_CONCEAL;
//TMM_EC }}

#define REDUCE_MAX_FRM_DPB 0

#endif // !defined(AFX_COMMONDEFS_H__4CE634CE_B48D_4812_8098_9CAEA258BAA2__INCLUDED_)

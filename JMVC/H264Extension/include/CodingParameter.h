#if !defined(AFX_CODINGPARAMETER_H__8403A680_A65D_466E_A411_05C3A7C0D59F__INCLUDED_)
#define AFX_CODINGPARAMETER_H__8403A680_A65D_466E_A411_05C3A7C0D59F__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "YUVFileParams.h"
using namespace std;


H264AVC_NAMESPACE_BEGIN


#include "H264AVCCommonLib/SequenceParameterSet.h"


#if defined( MSYS_WIN32 )
# pragma warning( disable: 4275 )
#endif

// TMM_ESS 
#include "ResizeParameters.h"

#define MAX_CONFIG_PARAMS 256

class H264AVCENCODERLIB_API EncoderConfigLineBase
{
protected:
  EncoderConfigLineBase(const Char* pcTag, UInt uiType ) : m_cTag( pcTag ), m_uiType( uiType ) {}
  EncoderConfigLineBase() {}
public:
  virtual ~EncoderConfigLineBase() {}
  std::string&  getTag () { return m_cTag; }
  virtual Void  setVar ( std::string& rcValue ) = 0;
protected:
  std::string m_cTag;
  UInt m_uiType;
};

class H264AVCENCODERLIB_API MotionVectorSearchParams
{
public:
  MotionVectorSearchParams() :  m_eSearchMode(FAST_SEARCH),  m_eFullPelDFunc(DF_SAD), m_eSubPelDFunc(DF_SAD), m_uiSearchRange(64), m_uiDirectMode(0) {}

  ErrVal check() const;

  const SearchMode getSearchMode()                const { return m_eSearchMode; }
  const DFunc getFullPelDFunc()                   const { return m_eFullPelDFunc; }
  const DFunc getSubPelDFunc()                    const { return m_eSubPelDFunc; }
  const UInt getSearchRange()                     const { return m_uiSearchRange; }
  UInt        getNumMaxIter     ()                const { return m_uiNumMaxIter; }
  UInt        getIterSearchRange()                const { return m_uiIterSearchRange; }
  const UInt getDirectMode()                      const { return m_uiDirectMode; }

  Void setSearchMode( UInt uiSearchMode )               { m_eSearchMode = SearchMode(uiSearchMode); }
  Void setFullPelDFunc( UInt uiFullPelDFunc )           { m_eFullPelDFunc = DFunc(uiFullPelDFunc); }
  Void setSubPelDFunc( UInt uiSubPelDFunc )             { m_eSubPelDFunc = DFunc(uiSubPelDFunc); }
  Void setSearchRange ( UInt uiSearchRange)             { m_uiSearchRange = uiSearchRange; }
  Void setNumMaxIter        ( UInt uiNumMaxIter      )  { m_uiNumMaxIter      = uiNumMaxIter;       }
  Void setIterSearchRange   ( UInt uiIterSearchRange )  { m_uiIterSearchRange = uiIterSearchRange;  }
  Void setDirectMode( UInt uiDirectMode)                { m_uiDirectMode = uiDirectMode; }

public:
  SearchMode  m_eSearchMode;
  DFunc       m_eFullPelDFunc;
  DFunc       m_eSubPelDFunc;
  UInt        m_uiSearchRange;   // no limit
  UInt        m_uiNumMaxIter;
  UInt        m_uiIterSearchRange;
  UInt        m_uiDirectMode;    // 0 temporal, 1 spatial
};



class H264AVCENCODERLIB_API LoopFilterParams
{
public:
  LoopFilterParams() : m_uiFilterIdc( 0 ),  m_iAlphaOffset( 0 ), m_iBetaOffset( 0 ) {}

  ErrVal check() const ;

  Bool  isDefault()                     const { return m_uiFilterIdc == 0 && m_iAlphaOffset == 0 && m_iBetaOffset == 0;}
  const UInt getFilterIdc()             const { return m_uiFilterIdc; }
  const Int getAlphaOffset()            const { return m_iAlphaOffset; }
  const Int getBetaOffset()             const { return m_iBetaOffset; }
  Void setAlphaOffset( Int iAlphaOffset )     { m_iAlphaOffset = iAlphaOffset; }
  Void setBetaOffset( Int iBetaOffset )       { m_iBetaOffset = iBetaOffset; }
  Void setFilterIdc( UInt uiFilterIdc)        { m_uiFilterIdc = uiFilterIdc; }

public:
  UInt m_uiFilterIdc;   // 0: Filter All Edges, 1: Filter No Edges, 2: Filter All Edges But Slice Boundaries
  UInt m_iAlphaOffset;
  UInt m_iBetaOffset;
};


const unsigned CodParMAXNumSliceGroupsMinus1 =8; // the same as MAXNumSliceGroupsMinus1 in pictureParameter.h






class H264AVCENCODERLIB_API LayerParameters
{
public:
  LayerParameters()
    : m_uiLayerId                         (0)
    , m_uiFrameWidth                      (352)
    , m_uiFrameHeight                     (288)
    , m_dInputFrameRate                   (7.5)
    , m_dOutputFrameRate                  (7.5)
    , m_cInputFilename                    ("none")
    , m_cOutputFilename                   ("none")
    , m_uiEntropyCodingModeFlag           (1)
    , m_uiClosedLoop                      (0)
    , m_uiAdaptiveTransform               (0)
    , m_uiMaxAbsDeltaQP                   (1)
    , m_dBaseQpResidual                   (26.0)
    , m_dNumFGSLayers                     (0)
    , m_uiInterLayerPredictionMode        (0)
    , m_uiMotionInfoMode                  (0)
    , m_cMotionInfoFilename               ("none")
    , m_uiFGSMode                         (0)
    , m_cFGSRateFilename                  ("none")
    , m_dFGSRate                          (0)
    , m_uiDecompositionStages             (0)
    , m_uiNotCodedMCTFStages              (0)
    , m_uiTemporalResolution              (0)
    , m_uiFrameDelay                      (0)
    , m_uiBaseQualityLevel                (3)
    , m_bConstrainedIntraPredForLP        (false)
    , m_uiForceReorderingCommands         (0)
    , m_uiBaseLayerId                     (MSYS_UINT_MAX)
    , m_dLowPassEnhRef                    ( AR_FGS_DEFAULT_LOW_PASS_ENH_REF )
    , m_uiBaseWeightZeroBaseBlock         ( AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_BLOCK )
    , m_uiBaseWeightZeroBaseCoeff         ( AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_COEFF )
    , m_uiFgsEncStructureFlag             ( AR_FGS_DEFAULT_ENC_STRUCTURE )
    , m_bUseDiscardable                   (false) //JVT-P031
    , m_dPredFGSRate                      (0.0) //JVT-P031
    , m_uiUseRedundantSlice               (0)   //JVT-Q054 Red. Picture
// JVT-Q065 EIDR{
	  , m_iIDRPeriod						  (0)
	  , m_bBLSkipEnable					  ( false )
  // JVT-Q065 EIDR}
    , m_uiFGSCodingMode                      ( 0 )
    , m_uiGroupingSize                       ( 1 )
    , m_dQpModeDecisionLP ( 0.00 )
    , m_uiNumSliceGroupMapUnitsMinus1 ( 0 ) 
    // JVT-S054 (ADD) ->
    , m_uiNumSliceMinus1 (0)
    , m_bSliceDivisionFlag (false)
    , m_uiSliceDivisionType (0)
    , m_puiGridSliceWidthInMbsMinus1 (0)
    , m_puiGridSliceHeightInMbsMinus1 (0)
    , m_puiFirstMbInSlice (0)
    , m_puiLastMbInSlice (0)
    , m_puiSliceId (0)
    // JVT-S054 (ADD) <-

	//S051{
	, m_cOutSIPFileName					("none")
	, m_cInSIPFileName					("none")
	, m_uiAnaSIP						(0)
	, m_bEncSIP							(false)
	//S051}
//JVT-T054{
    , m_uiLayerCGSSNR         ( 0 )
    , m_uiQualityLevelCGSSNR  ( 0 )
    , m_uiBaseLayerCGSSNR     ( 0 )
    , m_uiBaseQualityLevelCGSSNR ( 0 )
    , m_bDiscardable          ( false )
//JVT-T054}
  {
    for( UInt ui = 0; ui < MAX_DSTAGES; ui++ ) m_adQpModeDecision[ui] = 0.00;
    ::memset( m_uiPosVect, 0x00, 16*sizeof(UInt) );
  }

  virtual ~LayerParameters()
  {
    // JVT-S054 (ADD) ->
    if (m_puiGridSliceWidthInMbsMinus1 != NULL)
    {
      free(m_puiGridSliceWidthInMbsMinus1);
      m_puiGridSliceWidthInMbsMinus1 = NULL;
    }
    if (m_puiGridSliceHeightInMbsMinus1 != NULL)
    {
      free(m_puiGridSliceHeightInMbsMinus1);
      m_puiGridSliceHeightInMbsMinus1 = NULL;
    }
    if (m_puiFirstMbInSlice != NULL)
    {
      free(m_puiFirstMbInSlice);
      m_puiFirstMbInSlice = NULL;
    }
    if (m_puiLastMbInSlice != NULL)
    {
      free(m_puiLastMbInSlice);
      m_puiLastMbInSlice = NULL;
    }
    if (m_puiSliceId != NULL)
    {
      free(m_puiSliceId);
      m_puiSliceId = NULL;
    }
    // JVT-S054 (ADD) <-

  }


  //===== get =====
  UInt                            getLayerId                        () const {return m_uiLayerId; }
  UInt                            getFrameWidth                     () const {return m_uiFrameWidth; }
  UInt                            getFrameHeight                    () const {return m_uiFrameHeight; }
  Double                          getInputFrameRate                 () const {return m_dInputFrameRate; }
  Double                          getOutputFrameRate                () const {return m_dOutputFrameRate; }
  const std::string&              getInputFilename                  () const {return m_cInputFilename; }
  const std::string&              getOutputFilename                 () const {return m_cOutputFilename; }
  UInt                            getClosedLoop                     () const {return m_uiClosedLoop; }
  Bool                            getEntropyCodingModeFlag          () const {return m_uiEntropyCodingModeFlag == 1; }
  UInt                            getAdaptiveTransform              () const {return m_uiAdaptiveTransform; }
  UInt                            getMaxAbsDeltaQP                  () const {return m_uiMaxAbsDeltaQP; }
  Double                          getBaseQpResidual                 () const {return m_dBaseQpResidual; }
  Double                          getNumFGSLayers                   () const {return m_dNumFGSLayers; }
  Double                          getQpModeDecision          (UInt ui) const {return m_adQpModeDecision[ui]; }
  Double                          getQpModeDecisionLP               () const {return m_dQpModeDecisionLP; }
  UInt                            getInterLayerPredictionMode       () const {return m_uiInterLayerPredictionMode; }
  UInt                            getBaseQualityLevel               () const {return m_uiBaseQualityLevel; }
  UInt                            getMotionInfoMode                 () const {return m_uiMotionInfoMode; }
  const std::string&              getMotionInfoFilename             () const {return m_cMotionInfoFilename; }
  UInt                            getFGSMode                        () const {return m_uiFGSMode; }
  const std::string&              getFGSFilename                    () const {return m_cFGSRateFilename; }
  Double                          getFGSRate                        () const {return m_dFGSRate; }
  UInt                            getFrameWidthInMbs                () const {return ( m_uiFrameWidth + 15 ) >> 4; }
  UInt                            getFrameHeightInMbs               () const {return ( m_uiFrameHeight + 15 ) >> 4; }
  UInt                            getHorPadding                     () const {return 16*getFrameWidthInMbs () - getFrameWidth (); }
  UInt                            getVerPadding                     () const {return 16*getFrameHeightInMbs() - getFrameHeight(); }
  
  UInt                            getDecompositionStages            () const {return m_uiDecompositionStages; }
  UInt                            getNotCodedMCTFStages             () const {return m_uiNotCodedMCTFStages; }
  UInt                            getTemporalResolution             () const {return m_uiTemporalResolution; }
  UInt                            getFrameDelay                     () const {return m_uiFrameDelay; }

  UInt                            getBaseLayerSpatRes               () const {return m_uiBaseLayerSpatRes; }
  UInt                            getBaseLayerTempRes               () const {return m_uiBaseLayerTempRes; }
  Bool                            getContrainedIntraForLP           () const {return m_bConstrainedIntraPredForLP; }
  UInt                            getForceReorderingCommands        () const {return m_uiForceReorderingCommands; }
  UInt                            getBaseLayerId                    () const {return m_uiBaseLayerId; }

  Bool                            getUseDiscardable                 () const {return m_bUseDiscardable;} //JVT-P031
  Double                          getPredFGSRate                    () const {return m_dPredFGSRate;} //JVT-P031
  Bool                            getUseRedundantSliceFlag          () const {return m_uiUseRedundantSlice == 1; }  //JVT-Q054 Red. Picture
  
  //--ICU/ETRI FMO Implementation :  FMO start 
  UInt          getNumSliceGroupsMinus1() const {return m_uiNumSliceGroupsMinus1;}  //for test
  UInt          getSliceGroupMapType() const {return  m_uiSliceGroupMapType;  }
  Bool          getSliceGroupChangeDirection_flag () const {return m_bSliceGroupChangeDirection_flag;}
  UInt          getSliceGroupChangeRateMinus1 () const {return m_uiSliceGroupChangeRateMinus1;}
  UInt          getNumSliceGroupMapUnitsMinus1() const {return m_uiNumSliceGroupMapUnitsMinus1;}
  UInt          getSliceGroupId(Int i) const {return m_uiSliceGroupId[i];}
  UInt          getSliceMode() const {return m_uiSliceMode;}
  UInt          getSliceArgument() const { return m_uiSliceArgument ;}
  const std::string&   getSliceGroupConfigFileName() const{ return m_cSliceGroupConfigFileName;}
  UInt          getUseRedundantSlice() const { return m_uiUseRedundantSlice;}
  UInt*         getArrayRunLengthMinus1 () const {return (UInt*)m_uiRunLengthMinus1;}  
  UInt*         getArrayTopLeft () const {return (UInt*)m_uiTopLeft;}
  UInt*         getArrayBottomRight () const {return (UInt*)m_uiBottomRight;}
  UInt*         getArraySliceGroupId() const {return (UInt*)m_uiSliceGroupId;}
  //--ICU/ETRI FMO Implementation : FMO end

  //<-- consider ROI Extraction ICU/ETRI DS
  const std::string&   getROIConfigFileName() const{ return m_cROIConfigFileName;}
  UInt          getNumROI() const {return m_uiNumROI;}  //for test

  UInt*         getROIID () const {return (UInt*)m_uiROIID;}
  UInt*         getSGID () const {return (UInt*)m_uiSGID;}
  UInt*         getSLID () const {return (UInt*)m_uiSLID;}
  //--> consider ROI Extraction ICU/ETRI DS
  
  UInt getFGSCodingMode                  ()    { return m_uiFGSCodingMode; }
  UInt getGroupingSize                   ()    { return m_uiGroupingSize; }
  UInt getPosVect                        (UInt uiNum) {return m_uiPosVect[uiNum];} 

  //===== set =====
  Void setLayerId                         (UInt   p) { m_uiLayerId                        = p; }
  Void setFrameWidth                      (UInt   p) { m_uiFrameWidth                     = p; }
  Void setFrameHeight                     (UInt   p) { m_uiFrameHeight                    = p; }
  Void setInputFrameRate                  (Double p) { m_dInputFrameRate                  = p; }
  Void setOutputFrameRate                 (Double p) { m_dOutputFrameRate                 = p; }
  Void setInputFilename                   (Char*  p) { m_cInputFilename                   = p; }
  Void setOutputFilename                  (Char*  p) { m_cOutputFilename                  = p; }
  Void setClosedLoop                      (UInt   p) { m_uiClosedLoop                     = p; }
  Void setEntropyCodingModeFlag           (Bool   p) { m_uiEntropyCodingModeFlag          = p; }
  Void setAdaptiveTransform               (UInt   p) { m_uiAdaptiveTransform              = p; }
  Void setMaxAbsDeltaQP                   (UInt   p) { m_uiMaxAbsDeltaQP                  = p; }
  Void setBaseQpResidual                  (Double p) { m_dBaseQpResidual                  = p; }
  Void setNumFGSLayers                    (Double p) { m_dNumFGSLayers                    = p; }
  Void setQpModeDecision                  (UInt   n,
                                           Double p) { m_adQpModeDecision             [n] = p; }
  Void setQpModeDecisionLP                (Double p) { m_dQpModeDecisionLP                = p; }
  Void setInterLayerPredictionMode        (UInt   p) { m_uiInterLayerPredictionMode       = p; }
  Void setMotionInfoMode                  (UInt   p) { m_uiMotionInfoMode                 = p; }
  Void setMotionInfoFilename              (Char*  p) { m_cMotionInfoFilename              = p; }
  Void setFGSMode                         (UInt   p) { m_uiFGSMode                        = p; }
  Void setFGSFilename                     (Char*  p) { m_cFGSRateFilename                 = p; }
  Void setFGSRate                         (Double p) { m_dFGSRate                         = p; }
  
  Void setDecompositionStages             (UInt   p) { m_uiDecompositionStages            = p; }
  Void setNotCodedMCTFStages              (UInt   p) { m_uiNotCodedMCTFStages             = p; }
  Void setTemporalResolution              (UInt   p) { m_uiTemporalResolution             = p; }
  Void setFrameDelay                      (UInt   p) { m_uiFrameDelay                     = p; }

  Void setBaseLayerSpatRes                (UInt   p) { m_uiBaseLayerSpatRes               = p; }
  Void setBaseLayerTempRes                (UInt   p) { m_uiBaseLayerTempRes               = p; }
  Void setBaseQualityLevel                (UInt   p) { m_uiBaseQualityLevel               = p; }
  Void setContrainedIntraForLP            ()         { m_bConstrainedIntraPredForLP       = true; }
  Void setForceReorderingCommands         (UInt   p) { m_uiForceReorderingCommands        = p; }
  Void setBaseLayerId                     (UInt   p) { m_uiBaseLayerId                    = p; }

  Void setUseDiscardable                 (Bool b)     {m_bUseDiscardable                  = b;} //JVT-P031
  Void setPredFGSRate                    (Double d)   {m_dPredFGSRate                     = d;} //JVT-P031
  Void setFGSCodingMode                   ( UInt ui )                { m_uiFGSCodingMode  = ui;      }
  Void setGroupingSize                    ( UInt ui )                { m_uiGroupingSize   = ui;     }
  Void setPosVect                         ( UInt uiNum, UInt uiVect) { m_uiPosVect[uiNum] = uiVect; } 
// TMM_ESS {
  int                 getExtendedSpatialScalability     () { return m_ResizeParameter.m_iExtendedSpatialScalability; }
  int                 getSpatialScalabilityType         () { return m_ResizeParameter.m_iSpatialScalabilityType; }
  Void                setResizeParameters      (ResizeParameters *p) { memcpy(&m_ResizeParameter, p, sizeof(ResizeParameters)); }
  ResizeParameters*   getResizeParameters      () {return &m_ResizeParameter; }
// TMM_ESS }

// JVT-Q065 EIDR{
  Int				  getIDRPeriod			   () { return m_iIDRPeriod; }
  Bool				  getBLSkipEnable		   () { return m_bBLSkipEnable; }
  Void				  setBLSkipEnable( Bool b )   { m_bBLSkipEnable = b; }
// JVT-Q065 EIDR}

  UInt                getPLR                   () { return m_uiPLR; } //JVT-R057 LA-RDO

  //===== check =====
  ErrVal  check();

//--ICU/ETRI FMO Implementation
  Void setSliceGroupId(int i, UInt value) { m_uiSliceGroupId[i] = value;}

  Void                            setLowPassEnhRef        ( Double d )   
  {
      m_dLowPassEnhRef = ( d < 0.0 ) ? 0.0 : ( ( d > 1.0 ) ? 1.0 : d );
  }

  Double                          getLowPassEnhRef        ()            { return m_dLowPassEnhRef;        }
  Void                            setAdaptiveRefFGSWeights( UInt  uiBlock, UInt  uiCoeff )
  {
    // do not allow 1, to store it in 5-bit fixed-length
    AOT( uiBlock > AR_FGS_MAX_BASE_WEIGHT );
    m_uiBaseWeightZeroBaseBlock = (uiBlock <= 1) ? 0 : uiBlock; 

    // do not allow 1, to store it in 5-bit fixed-length
    AOT( uiCoeff > AR_FGS_MAX_BASE_WEIGHT );
    m_uiBaseWeightZeroBaseCoeff = (uiCoeff <= 1) ? 0 : uiCoeff;
  }
  Void                            getAdaptiveRefFGSWeights( UInt& uiBlock, UInt& uiCoeff )
  { 
    uiBlock = m_uiBaseWeightZeroBaseBlock; 
    uiCoeff = m_uiBaseWeightZeroBaseCoeff;
  }

  Void                            setFgsEncStructureFlag( UInt  flag )
  {
    m_uiFgsEncStructureFlag = flag;
  }
  UInt                            getFgsEncStructureFlag( )
  { 
    return m_uiFgsEncStructureFlag; 
  }

  UInt                            getFGSMotionMode() { return m_uiFGSMotionMode;  }
  Void                            setFGSMotionMode( UInt uiFGSMotionMode ) { m_uiFGSMotionMode = uiFGSMotionMode; }
  Void                            setUseRedundantSliceFlag(Bool   b) { m_uiUseRedundantSlice = b; }  // JVT-Q054 Red. Picture

  //S051{
  const std::string&              getInSIPFileName             () const { return m_cInSIPFileName; }
  const std::string&              getOutSIPFileName            () const { return m_cOutSIPFileName; }
  Void							  setInSIPFileName			   (Char* p) { m_cInSIPFileName=p; }
  Void							  setOutSIPFileName			   (Char* p) { m_cOutSIPFileName=p; }
  Void							  setAnaSIP					   (UInt	uiAnaSIP){ m_uiAnaSIP = uiAnaSIP;}
  Void						      setEncSIP					   (Bool	bEncSIP){ m_bEncSIP = bEncSIP;}
  UInt							  getAnaSIP					   (){ return m_uiAnaSIP; }
  Bool							  getEncSIP					   (){ return m_bEncSIP; }
  //S051}
//JVT-T054{
  UInt getLayerCGSSNR                    ()    { return m_uiLayerCGSSNR;}
  UInt getQualityLevelCGSSNR             ()    { return m_uiQualityLevelCGSSNR;}
  UInt getBaseLayerCGSSNR                    ()    { return m_uiBaseLayerCGSSNR;}
  UInt getBaseQualityLevelCGSSNR             ()    { return m_uiBaseQualityLevelCGSSNR;}
  Void setLayerCGSSNR                    (UInt ui)    { m_uiLayerCGSSNR                   = ui;}
  Void setQualityLevelCGSSNR             (UInt ui)    { m_uiQualityLevelCGSSNR            = ui;}
  Void setBaseLayerCGSSNR                    (UInt ui)    { m_uiBaseLayerCGSSNR                   = ui;}
  Void setBaseQualityLevelCGSSNR             (UInt ui)    { m_uiBaseQualityLevelCGSSNR            = ui;}
  Bool isDiscardable                      ()          { return m_bDiscardable;}
//JVT-T054}
public:
  UInt                      m_uiLayerId;
  UInt                      m_uiFrameWidth;
  UInt                      m_uiFrameHeight;
  Double                    m_dInputFrameRate;
  Double                    m_dOutputFrameRate;
  std::string               m_cInputFilename;
  std::string               m_cOutputFilename;

  UInt                      m_uiClosedLoop;
  UInt                      m_uiEntropyCodingModeFlag;
  UInt                      m_uiAdaptiveTransform;

  UInt                      m_uiMaxAbsDeltaQP;
  Double                    m_dBaseQpResidual;
  Double                    m_dNumFGSLayers;
  
  Double                    m_adQpModeDecision[MAX_DSTAGES];
  Double                    m_dQpModeDecisionLP;
  UInt                      m_uiInterLayerPredictionMode;
  Bool                      m_bConstrainedIntraPredForLP;
  UInt                      m_uiForceReorderingCommands;
  UInt                      m_uiBaseLayerId;

  UInt                      m_uiBaseQualityLevel;

  UInt                      m_uiMotionInfoMode;
  std::string               m_cMotionInfoFilename;

  UInt                      m_uiFGSMode;
  std::string               m_cFGSRateFilename;
  Double                    m_dFGSRate;
  
  //----- derived parameters -----
  UInt                      m_uiDecompositionStages;
  UInt                      m_uiNotCodedMCTFStages;
  UInt                      m_uiTemporalResolution;
  UInt                      m_uiFrameDelay;

  //----- for scalable SEI ----
  UInt                      m_uiBaseLayerSpatRes;
  UInt                      m_uiBaseLayerTempRes;

  //----- ESS ---- 
  ResizeParameters          m_ResizeParameter;

  Double                    m_dLowPassEnhRef;
  UInt                      m_uiBaseWeightZeroBaseBlock;
  UInt                      m_uiBaseWeightZeroBaseCoeff;
  UInt                      m_uiFgsEncStructureFlag;

  //--ICU/ETRI FMO Implementation : FMO start
  UInt         m_uiNumSliceGroupsMinus1;  
  UInt         m_uiSliceGroupMapType;  
  UInt         m_uiRunLengthMinus1[CodParMAXNumSliceGroupsMinus1];  
  UInt         m_uiTopLeft[CodParMAXNumSliceGroupsMinus1];
  UInt         m_uiBottomRight[CodParMAXNumSliceGroupsMinus1];
  Bool         m_bSliceGroupChangeDirection_flag;
  UInt         m_uiSliceGroupChangeRateMinus1;
  UInt         m_uiNumSliceGroupMapUnitsMinus1;
  UInt         m_uiSliceGroupId[CodParMAXNumSliceGroupsMinus1];
  UInt         m_uiSliceMode;
  UInt         m_uiSliceArgument;
  std::string  m_cSliceGroupConfigFileName;

  std::string  m_cROIConfigFileName;
  UInt		   m_uiNumROI;
  UInt		   m_uiROIID[CodParMAXNumSliceGroupsMinus1];
  UInt		   m_uiSGID[CodParMAXNumSliceGroupsMinus1];
  UInt		   m_uiSLID[CodParMAXNumSliceGroupsMinus1];

  //--ICU/ETRI FMO Implementation : FMO end
  UInt         m_uiUseRedundantSlice;   // JVT-Q054 Red. Picture

  // JVT-S054 (ADD) ->
  Bool         m_bSliceDivisionFlag;
  UInt         m_uiNumSliceMinus1;
  UInt         m_uiSliceDivisionType;
  UInt*        m_puiGridSliceWidthInMbsMinus1;
  UInt*        m_puiGridSliceHeightInMbsMinus1;
  UInt*        m_puiFirstMbInSlice;
  UInt*        m_puiLastMbInSlice;
  UInt*        m_puiSliceId;
  // JVT-S054 (ADD) <-

  //JVT-P031
  Bool                      m_bUseDiscardable; //indicate if discardable stream is coded for this layer 
                                                //discardable stream should not be used for inter-layer prediction
  Double                    m_dPredFGSRate; //rate use for inter-layer prediction (after that rate, stream is discardable)

  UInt                      m_uiFGSMotionMode;

// JVT-Q065 EIDR{
  Int						m_iIDRPeriod;
  Bool						m_bBLSkipEnable;
// JVT-Q065 EIDR}

  UInt               m_uiPLR; //JVT-R057 LA-RDO
  UInt       m_uiFGSCodingMode;
  UInt       m_uiGroupingSize;
  UInt       m_uiPosVect[16];

  //S051{
  std::string    m_cOutSIPFileName;
  std::string	 m_cInSIPFileName;
  UInt			 m_uiAnaSIP;
  Bool			 m_bEncSIP;
  //S051}
//JVT-T054{
  UInt                      m_uiLayerCGSSNR;
  UInt                      m_uiQualityLevelCGSSNR;
  UInt                      m_uiBaseLayerCGSSNR;
  UInt                      m_uiBaseQualityLevelCGSSNR;
  Bool                      m_bDiscardable;
//JVT-T054}
};


//TMM_WP
class H264AVCENCODERLIB_API SampleWeightingParams
{
  public:
    SampleWeightingParams() : m_uiIPMode(0), m_uiBMode(0), m_uiLumaDenom(5), m_uiChromaDenom(5), m_fDiscardThr(1) { }
        ErrVal check() const ;
        ErrVal checkForValidChanges( const SampleWeightingParams& rcSW )const;

        Bool operator == ( const SampleWeightingParams& rcSWP ) const ;
        Bool operator != ( const SampleWeightingParams& rcSWP ) const { return !((*this) == rcSWP); }
        ErrVal writeBinary( BinDataAccessor& rcBinDataAccessor )  const;
        ErrVal readBinary( BinDataAccessor& rcBinDataAccessor );

        UInt getIPMode()                  const { return m_uiIPMode; }
        UInt getBMode()                   const { return m_uiBMode; }
        UInt getLumaDenom()               const { return m_uiLumaDenom; }
        UInt getChromaDenom()             const { return m_uiChromaDenom; }
        Float getDiscardThr()             const { return m_fDiscardThr; }
        Void setIPMode( UInt uiIPMode )         { m_uiIPMode      = uiIPMode; }
        Void setBMode( UInt uiBMode )           { m_uiBMode       = uiBMode; }
        Void setLumaDenom( UInt uiDenom )       { m_uiLumaDenom   = uiDenom; }
        Void setChromaDenom( UInt uiDenom )     { m_uiChromaDenom = uiDenom; }
        Void setDiscardThr( Float fDiscardThr ) { m_fDiscardThr = fDiscardThr; }

  protected:
        UInt m_uiIPMode;      // 0 off, 1 on, 2 random
        UInt m_uiBMode;       // 0 off, 1 explicit, 2 implicit, 3 random
        UInt m_uiLumaDenom;   // 0-7
        UInt m_uiChromaDenom; // 0-7
        Float m_fDiscardThr;
};
//TMM_WP



class H264AVCENCODERLIB_API CodingParameter
{
public:
  CodingParameter()
    : m_dMaximumFrameRate                 ( 0.0 )
    , m_dMaximumDelay                     ( 1e6 )
    , m_uiTotalFrames                     ( 0 )
    , m_uiGOPSize                         ( 0 )
    , m_uiDecompositionStages             ( 0 )
    , m_uiIntraPeriod                     ( 0 )
    , m_uiIntraPeriodLowPass              ( 0 )
    , m_uiNumRefFrames                    ( 0 )
    , m_uiBaseLayerMode                   ( 0 )
    , m_uiNumberOfLayers                  ( 0 )
    , m_bExtendedPriorityId               ( 0 )
    , m_uiNumSimplePris                   (0)
    , m_dLowPassEnhRef                    ( -1.0 )
    , m_uiBaseWeightZeroBaseBlock         ( MSYS_UINT_MAX )
    , m_uiBaseWeightZeroBaseCoeff         ( MSYS_UINT_MAX )
    , m_uiFgsEncStructureFlag             ( MSYS_UINT_MAX )
    , m_uiLowPassFgsMcFilter              ( AR_FGS_DEFAULT_FILTER )
    , m_uiMVCmode                         ( 0 )
    , m_uiFrameWidth                      ( 0 )
    , m_uiFrameHeight                     ( 0 )
    , m_uiSymbolMode                      ( 0 )
    , m_ui8x8Mode                         ( 0 )
    , m_dBasisQp                          ( 0 )
    , m_uiDPBSize                         ( 0 )
    , m_uiNumDPBRefFrames                 ( 0 )
    , m_uiLog2MaxFrameNum                 ( 0 )
	, m_uiPicOrderCntType                 ( 0 )//Poc Type
    , m_uiLog2MaxPocLsb                   ( 0 )
    , m_cSequenceFormatString             ()
    , m_uiMaxRefIdxActiveBL0              ( 0 )
    , m_uiMaxRefIdxActiveBL1              ( 0 )
    , m_uiMaxRefIdxActiveP                ( 0 )
//TMM_WP
      , m_uiIPMode                       (0)
      , m_uiBMode                        (0)
//TMM_WP
	  , m_bNonRequiredEnable				( 0 ) //NonRequired JVT-Q066
	  , m_uiLARDOEnable                  (0)      //JVT-R057 LA-RDO
	  , m_uiSuffixUnitEnable			  (0)  //JVT-S036 lsj
	  , m_uiMMCOBaseEnable					  ( 0 ) //JVT-S036 
//SEI {
	  , m_uiNestingSEIEnable              ( 0 ) 
	  , m_uiSnapShotEnable                ( 0 )
	  , m_uiActiveViewSEIEnable			  ( 0 )
	  , m_uiViewScalInfoSEIEnable		  ( 0 )
      , m_uiMultiviewSceneInfoSEIEnable			  ( 0 ) // SEI JVT-W060		
	  , m_uiMultiviewAcquisitionInfoSEIEnable			  ( 0 ) // SEI JVT-W060
//SEI  }	  
//JVT-T054{
    , m_uiCGSSNRRefinementFlag             ( 0 )
    , m_uiMaxLayerCGSSNR                ( 0 )
    , m_uiMaxQualityLevelCGSSNR         ( 0 )
//JVT-T054}
    , m_uiInterPredPicsFirst            ( 0 )
//JVT-W080
		, m_uiPdsEnable                       ( 0 ) 
		, m_uiPdsInitialDelayAnc              ( 0 )
		, m_uiPdsInitialDelayNonAnc           ( 0 )
		, m_uiPdsBlockSize                    ( 0 )
		, m_ppuiPdsInitialDelayMinus2L0Anc    ( 0 )
		, m_ppuiPdsInitialDelayMinus2L1Anc    ( 0 )
		, m_ppuiPdsInitialDelayMinus2L0NonAnc ( 0 )
		, m_ppuiPdsInitialDelayMinus2L1NonAnc ( 0 )
		, m_uiDPBConformanceCheck (1)
        ,m_uiMbAff (0)
        ,m_uiPAff(0)

//~JVT-W080
	{
    for( UInt uiLayer = 0; uiLayer < 6; uiLayer++ )
    {
      m_adDeltaQpLayer[uiLayer] = 0;
    }
  }
	virtual ~CodingParameter()
  {
//JVT-W080
		if( m_uiMVCmode && m_uiPdsEnable )
	  {
			UInt uiViewNum = (UInt)SpsMVC.m_num_views_minus_1+1;
      for( UInt i = 0; i < uiViewNum; i++ )
			{
	      delete[] m_ppuiPdsInitialDelayMinus2L0Anc[i];
	      delete[] m_ppuiPdsInitialDelayMinus2L1Anc[i];
	      delete[] m_ppuiPdsInitialDelayMinus2L0NonAnc[i];
	      delete[] m_ppuiPdsInitialDelayMinus2L1NonAnc[i];
			}
			delete[] m_ppuiPdsInitialDelayMinus2L0Anc;
			delete[] m_ppuiPdsInitialDelayMinus2L1Anc;
			delete[] m_ppuiPdsInitialDelayMinus2L0NonAnc;
			delete[] m_ppuiPdsInitialDelayMinus2L1NonAnc;
      m_ppuiPdsInitialDelayMinus2L0Anc = NULL;
			m_ppuiPdsInitialDelayMinus2L1Anc = NULL;
      m_ppuiPdsInitialDelayMinus2L0NonAnc = NULL;
			m_ppuiPdsInitialDelayMinus2L1NonAnc = NULL;
	  }
//~JVT-W080
  }

public:
  const MotionVectorSearchParams& getMotionVectorSearchParams       () const {return m_cMotionVectorSearchParams; }
  const LoopFilterParams&         getLoopFilterParams               () const {return m_cLoopFilterParams; }

  MotionVectorSearchParams&       getMotionVectorSearchParams       ()       {return m_cMotionVectorSearchParams; }
  LoopFilterParams&               getLoopFilterParams               ()       {return m_cLoopFilterParams; }

  const LayerParameters&          getLayerParameters  ( UInt    n )   const   { return m_acLayerParameters[n]; }
  LayerParameters&                getLayerParameters  ( UInt    n )           { return m_acLayerParameters[n]; }

//TMM_WP
  SampleWeightingParams&           getSampleWeightingParams(UInt uiLayerId)  {return m_cSampleWeightingParams[uiLayerId];}
//TMM_WP
//JVT-W080
	ErrVal savePDSParameters( UInt   uiNumView
											  	, UInt*  num_refs_list0_anc
											  	, UInt*  num_refs_list1_anc
											  	, UInt*  num_refs_list0_nonanc
											  	, UInt*  num_refs_list1_nonanc
											  	, UInt** PDIInitialDelayMinus2L0Anc
											  	, UInt** PDIInitialDelayMinus2L1Anc
											  	, UInt** PDIInitialDelayMinus2L0NonAnc
											  	, UInt** PDIInitialDelayMinus2L1NonAnc
											  	);
//~JVT-W080 
  
  const std::string&              getInputFile            ()              const   { return m_cInputFile; }
  Double                          getMaximumFrameRate     ()              const   { return m_dMaximumFrameRate; }
  Double                          getMaximumDelay         ()              const   { return m_dMaximumDelay; }
  UInt                            getTotalFrames          ()              const   { return m_uiTotalFrames; }
  UInt                            getGOPSize              ()              const   { return m_uiGOPSize; }  
  UInt                            getDecompositionStages  ()              const   { return m_uiDecompositionStages; }
  UInt                            getIntraPeriod          ()              const   { return m_uiIntraPeriod; }
  UInt                            getIntraPeriodLowPass   ()              const   { return m_uiIntraPeriodLowPass; }
  UInt                            getNumRefFrames         ()              const   { return m_uiNumRefFrames; }
  UInt                            getBaseLayerMode        ()              const   { return m_uiBaseLayerMode; }
  UInt                            getNumberOfLayers       ()              const   { return m_uiNumberOfLayers; }
  Bool                            getExtendedPriorityId   ()              const   { return m_bExtendedPriorityId; }
  UInt                            getNumSimplePris        ()              const   { return m_uiNumSimplePris; }
/*  Void                            getSimplePriorityMap    ( UInt uiSimplePri, UInt& uiTemporalLevel, UInt& uiLayer, UInt& uiQualityLevel )
                                                                          { uiTemporalLevel = m_uiTemporalLevelList[uiSimplePri];
                                                                            uiLayer         = m_uiDependencyIdList [uiSimplePri];
                                                                            uiQualityLevel  = m_uiQualityLevelList [uiSimplePri];
                                                                          }
 JVT-S036  */
//TMM_WP
  UInt getIPMode()                  const { return m_uiIPMode; }
  UInt getBMode()                   const { return m_uiBMode; }
//TMM_WP

  UInt                            getMVCmode              ()              const   { return m_uiMVCmode; }
  UInt                            getInterPredPicsFirst   ()              const   { return m_uiInterPredPicsFirst; } // JVT-V043 enc

  UInt                            getFrameWidth           ()              const   { return m_uiFrameWidth; }
  UInt                            getFrameHeight          ()              const   { return m_uiFrameHeight; }
  UInt                            getSymbolMode           ()              const   { return m_uiSymbolMode; }
  UInt                            get8x8Mode              ()              const   { return m_ui8x8Mode; }
  Double                          getBasisQp              ()              const   { return m_dBasisQp; }
  UInt                            getDPBSize              ()              const   { return m_uiDPBSize; }
  UInt                            getNumDPBRefFrames      ()              const   { return m_uiNumDPBRefFrames; }
  UInt                            getLog2MaxFrameNum      ()              const   { return m_uiLog2MaxFrameNum; }
  UInt                            getPicOrderCntType      ()              const   { return m_uiPicOrderCntType; } //Poc Type
  UInt                            getLog2MaxPocLsb        ()              const   { return m_uiLog2MaxPocLsb; }
  std::string                     getSequenceFormatString ()              const   { return m_cSequenceFormatString; }
  Double                          getDeltaQpLayer         ( UInt ui )     const   { return m_adDeltaQpLayer[ui]; }
  UInt                            getMaxRefIdxActiveBL0   ()              const   { return m_uiMaxRefIdxActiveBL0; }
  UInt                            getMaxRefIdxActiveBL1   ()              const   { return m_uiMaxRefIdxActiveBL1; }
  UInt                            getMaxRefIdxActiveP     ()              const   { return m_uiMaxRefIdxActiveP; }
  UInt                            getDPBConformanceCheck              ()              const   { return m_uiDPBConformanceCheck; }
  UInt                              getMbAff            ( )    const   { return m_uiMbAff; }
  UInt                              getPAff             ( )    const   { return m_uiPAff; }
  Bool                              isInterlaced        ( )    const   { return ( m_uiMbAff != 0 || m_uiPAff != 0 ); }
//JVT-W080
	UInt                            getPdsEnable            ()              const   { return m_uiPdsEnable; } 
	UInt                            getPdsInitialDelayAnc   ()              const   { return m_uiPdsInitialDelayAnc; } 
	UInt                            getPdsInitialDelayNonAnc()              const   { return m_uiPdsInitialDelayNonAnc; } 
	UInt**                          getPdsInitialDelayMinus2L0Anc()         const   { return m_ppuiPdsInitialDelayMinus2L0Anc; }
	UInt**                          getPdsInitialDelayMinus2L1Anc()         const   { return m_ppuiPdsInitialDelayMinus2L1Anc; }
	UInt**                          getPdsInitialDelayMinus2L0NonAnc()      const   { return m_ppuiPdsInitialDelayMinus2L0NonAnc; }
	UInt**                          getPdsInitialDelayMinus2L1NonAnc()      const   { return m_ppuiPdsInitialDelayMinus2L1NonAnc; }
	UInt                            getPdsBlockSize         ()              const   { return m_uiPdsBlockSize; } 
	Void                            setPdsEnable            ( UInt    n )   { m_uiPdsEnable           = n; }
//~JVT-W080
	UInt                            getMultiviewSceneInfoEnable            ()              const   { return m_uiMultiviewSceneInfoSEIEnable; } // SEI JVT-W060
	UInt							getMaxDisparity () const { return m_uiMaxDisparity;} // SEI JVT-W060
	UInt							getMultiviewAcquisitionInfoEnable            ()              const   { return m_uiMultiviewAcquisitionInfoSEIEnable; } // SEI JVT-W060
  UInt                            getFrameWidthInMbs                () const {return ( m_uiFrameWidth + 15 ) >> 4; }
  UInt                            getFrameHeightInMbs               () const {return ( m_uiFrameHeight + 15 ) >> 4; }
    UInt                            getHorPadding                     () const {return 16*getFrameWidthInMbs () - getFrameWidth (); }
    UInt                            getVerPadding                     () const {return 16*getFrameHeightInMbs() - getFrameHeight(); }


  Void                            setInputFile            ( Char*   p )   { m_cInputFile            = p; }

  UInt                            getLARDOEnable          ()              const   { return m_uiLARDOEnable;} //JVT-R057 LA-RDO
  UInt							  getSuffixUnitEnable	  ()		      const	  { return m_uiSuffixUnitEnable;} //JVT-S036 
  UInt							  getMMCOBaseEnable		  ()			  const	  { return m_uiMMCOBaseEnable; } //JVT-S036 
   //SEI {
  UInt                            getNestingSEIEnable     ()              const   { return m_uiNestingSEIEnable; }
  UInt                            getSnapshotEnable       ()              const   { return m_uiSnapShotEnable; }
  UInt                            getActiveViewSEIEnable  ()              const   { return m_uiActiveViewSEIEnable; } 
  UInt							  getViewScalInfoSEIEnable()			  const   { return m_uiViewScalInfoSEIEnable; }
  //SEI }
  UInt                            getMultiviewSceneInfoSEIEnable  ()              const   { return m_uiMultiviewSceneInfoSEIEnable; }   // SEI JVT-W060
  UInt                            getMultiviewAcquisitionInfoSEIEnable  ()              const   { return m_uiMultiviewAcquisitionInfoSEIEnable; }   // SEI JVT-W060
  Void                            setMaximumFrameRate     ( Double  d )   { m_dMaximumFrameRate     = d; }
  Void                            setMaximumDelay         ( Double  d )   { m_dMaximumDelay         = d; }
  Void                            setTotalFrames          ( UInt    n )   { m_uiTotalFrames         = n; }
  Void                            setGOPSize              ( UInt    n )   { m_uiGOPSize             = n; }
  Void                            setDecompositionStages  ( UInt    n )   { m_uiDecompositionStages = n; }
  Void                            setIntraPeriod          ( UInt    n )   { m_uiIntraPeriod         = n; }
  Void                            setIntraPeriodLowPass   ( UInt    n )   { m_uiIntraPeriodLowPass  = n; }
  Void                            setNumRefFrames         ( UInt    n )   { m_uiNumRefFrames        = n; }
  Void                            setBaseLayerMode        ( UInt    n )   { m_uiBaseLayerMode       = n; }
  Void                            setNumberOfLayers       ( UInt    n )   { m_uiNumberOfLayers      = n; }
  Void                            setExtendedPriorityId   ( Bool    b )   { m_bExtendedPriorityId   = b; }
  Void                            setNumSimplePris        ( UInt    n )   { m_uiNumSimplePris       = n; }
 /* Void                            setSimplePriorityMap ( UInt uiSimplePri, UInt uiTemporalLevel, UInt uiLayer, UInt uiQualityLevel )
                                                                          { m_uiTemporalLevelList[uiSimplePri] = uiTemporalLevel;
                                                                            m_uiDependencyIdList [uiSimplePri] = uiLayer;
                                                                            m_uiQualityLevelList [uiSimplePri] = uiQualityLevel;
                                                                          }
 JVT-S036  */

  Void                            setMVCmode              ( UInt    p )   { m_uiMVCmode             = p; }

  Void                            setInterPredPicsFirst   ( UInt    p)    { m_uiInterPredPicsFirst  = p; } // JVT-V043 enc
  Void                            setFrameWidth           ( UInt    p )   { m_uiFrameWidth          = p; }
  Void                            setFrameHeight          ( UInt    p )   { m_uiFrameHeight         = p; }
  Void                            setSymbolMode           ( UInt    p )   { m_uiSymbolMode          = p; }
  Void                            set8x8Mode              ( UInt    p )   { m_ui8x8Mode             = p; }
  Void                            setBasisQp              ( Double  p )   { m_dBasisQp              = p; }
  Void                            setDPBSize              ( UInt    p )   { m_uiDPBSize             = p; }
  Void                            setNumDPBRefFrames      ( UInt    p )   { m_uiNumDPBRefFrames     = p; }
  Void                            setLog2MaxFrameNum      ( UInt    p )   { m_uiLog2MaxFrameNum     = p; }
  Void                            setPicOrderCntType      ( UInt    p )   { m_uiPicOrderCntType     = p; } //Poc Type
  Void                            setLog2MaxPocLsb        ( UInt    p )   { m_uiLog2MaxPocLsb       = p; }
  Void                            setSequenceFormatString ( Char*   p )   { m_cSequenceFormatString = p; }
  Void                            setDeltaQpLayer         ( UInt    n,
                                                            Double  p )   { m_adDeltaQpLayer[n]     = p; }
  Void                            setMaxRefIdxActiveBL0   ( UInt    p )   { m_uiMaxRefIdxActiveBL0  = p; }
  Void                            setMaxRefIdxActiveBL1   ( UInt    p )   { m_uiMaxRefIdxActiveBL1  = p; }
  Void                            setMaxRefIdxActiveP     ( UInt    p )   { m_uiMaxRefIdxActiveP    = p; }

  ErrVal                          check                   ();
  
  // TMM_ESS 
  ResizeParameters*               getResizeParameters  ( UInt    n )    { return m_acLayerParameters[n].getResizeParameters(); }

  Void                            setLowPassEnhRef        ( Double d )   
  { 
    m_dLowPassEnhRef = ( d < 0.0 ) ? 0.0 : ( ( d > 1.0 ) ? 1.0 : d );
  }

  Double                          getLowPassEnhRef        ()            { return m_dLowPassEnhRef;        }
  Void                            setAdaptiveRefFGSWeights( UInt  uiBlock, UInt  uiCoeff )
  {
    // do not allow 1, to store it in 5-bit fixed-length
    AOT( uiBlock > AR_FGS_MAX_BASE_WEIGHT );
    m_uiBaseWeightZeroBaseBlock = (uiBlock <= 1) ? 0 : uiBlock; 

    AOT( uiCoeff > AR_FGS_MAX_BASE_WEIGHT );
    m_uiBaseWeightZeroBaseCoeff = (uiCoeff <= 1) ? 0 : uiCoeff;
  }
  Void                            getAdaptiveRefFGSWeights( UInt& uiBlock, UInt& uiCoeff )
  { 
    uiBlock = m_uiBaseWeightZeroBaseBlock; 
    uiCoeff = m_uiBaseWeightZeroBaseCoeff;
  }

  Void                            setFgsEncStructureFlag( UInt  flag )
  {
    m_uiFgsEncStructureFlag = flag;
  }
  UInt                            getFgsEncStructureFlag( )
  { 
    return m_uiFgsEncStructureFlag; 
  }

  Void                            setLowPassFgsMcFilter   ( UInt ui )   { m_uiLowPassFgsMcFilter  = ui;   }
  UInt                            getLowPassFgsMcFilter   ()            { return m_uiLowPassFgsMcFilter;  }

  UInt  getVFramePeriod() const {return m_vFramePeriod;}
  unsigned int	getCurentViewId() const {return m_CurrentViewId;}
  Bool  getAVCFlag() const {return m_bAVCFlag;}
  Bool getSvcMvcFlag() const { return m_SvcMvcFlag;}
  SpsMvcExtension * getSpsMVC() {return &SpsMVC;} // 
  SpsMvcExtension SpsMVC;

  Int							  getNonRequiredEnable    ()			{ return m_bNonRequiredEnable; }  //NonRequired JVT-Q066 (06-04-08)
//JVT-T054{
  UInt                            getCGSSNRRefinement     ()              const   { return m_uiCGSSNRRefinementFlag;}
  UInt                            getMaxLayerCGSSNR       ()              const   { return m_uiMaxLayerCGSSNR;}
  UInt                            getMaxQualityLevelCGSSNR ()             const   { return m_uiMaxQualityLevelCGSSNR;}
  Void                            setCGSSNRRefinement     ( UInt    b )   { m_uiCGSSNRRefinementFlag = b; }
  Void                            setMaxLayerCGSSNR       ( UInt    ui )  { m_uiMaxLayerCGSSNR       = ui; }
  Void                            setMaxQualityLevelCGSSNR( UInt    ui )  { m_uiMaxQualityLevelCGSSNR= ui; }
//JVT-T054}
public: //  
  UInt                            getLogFactor            ( Double  r0,
                                                            Double  r1 );

  /** SEI JVT-W060 **/
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

		m_uiNumViewMinus1 = num_of_views -1;
   }
   void release_memory()
   {
	   int i,j;
	   int num_of_views = m_uiNumViewMinus1+1;

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
   int getNumViewMinus1() const {return m_uiNumViewMinus1;}
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

/******************/
protected:
  std::string               m_cInputFile;
//  std::string               m_cBitstreamFile;
  std::string               m_cViewOrder; 
  UInt                      m_uiBaseViewId;

  Double                    m_dMaximumFrameRate;
  Double                    m_dMaximumDelay;
  UInt                      m_uiTotalFrames;

  UInt                      m_uiGOPSize;
  UInt                      m_uiDecompositionStages;
  UInt                      m_uiIntraPeriod;
  UInt                      m_uiIntraPeriodLowPass;
  UInt                      m_uiNumRefFrames;
  UInt                      m_uiBaseLayerMode;

  MotionVectorSearchParams  m_cMotionVectorSearchParams;
  LoopFilterParams          m_cLoopFilterParams;

  UInt                      m_uiNumberOfLayers;
  LayerParameters           m_acLayerParameters[MAX_LAYERS];
  Bool                      m_bExtendedPriorityId;
  UInt                      m_uiNumSimplePris;

  EncoderConfigLineBase*    m_pEncoderLines[MAX_CONFIG_PARAMS];
  EncoderConfigLineBase*    m_pLayerLines  [MAX_CONFIG_PARAMS];

  Double                    m_dLowPassEnhRef;
  UInt                      m_uiBaseWeightZeroBaseBlock;
  UInt                      m_uiBaseWeightZeroBaseCoeff;
  UInt                      m_uiFgsEncStructureFlag;
  UInt                      m_uiLowPassFgsMcFilter;

  UInt                      m_uiMVCmode;
  UInt                      m_uiInterPredPicsFirst; // JVT-V043 enc

  UInt                      m_uiFrameWidth;
  UInt                      m_uiFrameHeight;
  UInt                      m_uiSymbolMode;
  UInt                      m_ui8x8Mode;
  Double                    m_dBasisQp;
  UInt                      m_uiDPBSize;
  UInt                      m_uiNumDPBRefFrames;
  UInt                      m_uiLog2MaxFrameNum;
  UInt                      m_uiPicOrderCntType; //Poc Type
  UInt                      m_uiLog2MaxPocLsb;
  std::string               m_cSequenceFormatString;
  Double                    m_adDeltaQpLayer[6];
  UInt                      m_uiMaxRefIdxActiveBL0;
  UInt                      m_uiMaxRefIdxActiveBL1;
  UInt                      m_uiMaxRefIdxActiveP;

  UInt          m_vFramePeriod;
  unsigned int	m_CurrentViewId;
  Bool          m_bAVCFlag;
  Bool          m_SvcMvcFlag;


//JVT-W080
	UInt                      m_uiPdsEnable; 
	UInt                      m_uiPdsInitialDelayAnc; 
	UInt                      m_uiPdsInitialDelayNonAnc; 
	UInt                      m_uiPdsBlockSize;
	UInt**                    m_ppuiPdsInitialDelayMinus2L0Anc;
	UInt**                    m_ppuiPdsInitialDelayMinus2L1Anc;
	UInt**                    m_ppuiPdsInitialDelayMinus2L0NonAnc;
	UInt**                    m_ppuiPdsInitialDelayMinus2L1NonAnc;
//~JVT-W080
	UInt                      m_uiMaxDisparity; // SEI JVT-W060
//TMM_WP
  UInt m_uiIPMode;
  UInt m_uiBMode;

  SampleWeightingParams m_cSampleWeightingParams[MAX_LAYERS];
//TMM_WP

  Int						m_bNonRequiredEnable; //NonRequired JVT-Q066
  UInt                       m_uiLARDOEnable; //JVT-R057 LA-RDO

  UInt						m_uiSuffixUnitEnable; //JVT-S036 
  UInt						m_uiMMCOBaseEnable;  //JVT-S036 
//JVT-T054{
  UInt                      m_uiCGSSNRRefinementFlag;
  UInt                      m_uiMaxLayerCGSSNR;
  UInt                      m_uiMaxQualityLevelCGSSNR;
//JVT-T054}
//SEI {
  UInt                      m_uiNestingSEIEnable;
  UInt                      m_uiSnapShotEnable;
  UInt                      m_uiActiveViewSEIEnable; 
  UInt						m_uiViewScalInfoSEIEnable;
//SEI }  
  UInt                      m_uiMultiviewSceneInfoSEIEnable;  // SEI JVT-W060
  UInt                      m_uiMultiviewAcquisitionInfoSEIEnable;  // SEI JVT-W060
  /* SEI JVT-W060 */
   int		m_uiNumViewMinus1;
   Bool		m_bIntrinsicParamFlag; 
   Bool		m_bIntrinsicParamsEqual;
   UInt		m_uiPrecFocalLength;
   UInt		m_uiPrecPrincipalPoint;
   UInt		m_uiPrecRadialDistortion;
   UInt		m_uiPrecRotationParam;
   UInt		m_uiPrecTranslationParam;

   Bool	    *m_bSignFocalLengthX;
   Bool     *m_bSignFocalLengthY;
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


   ////////////lufeng: modify .cfg
   UInt                      m_uiMbAff;
   UInt                      m_uiPAff;
/**********************/
   UInt		m_uiDPBConformanceCheck;
public:
	std::vector<YUVFileParams> m_MultiviewReferenceFileParams;

};

#if defined( MSYS_WIN32 )
# pragma warning( default: 4275 )
#endif


H264AVC_NAMESPACE_END


#endif // !defined(AFX_CODINGPARAMETER_H__8403A680_A65D_466E_A411_05C3A7C0D59F__INCLUDED_)

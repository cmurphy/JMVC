#if !defined(AFX_MOTIONESTIMATION_H__00B6343B_CCFC_466D_9F60_04EB29EE8E56__INCLUDED_)
#define AFX_MOTIONESTIMATION_H__00B6343B_CCFC_466D_9F60_04EB29EE8E56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "MotionEstimationCost.h"
#include "Distortion.h"
#include "CodingParameter.h"

#include "H264AVCCommonLib/MotionCompensation.h"



H264AVC_NAMESPACE_BEGIN


class MotionVectorCalculation;
class RateDistortionIf;
class CodingParameter;
class QuarterPelFilter;


class MotionEstimation
: public MotionCompensation
, public MotionEstimationCost
{
public:
  class MEBiSearchParameters
  {
  public:
    IntYuvMbBuffer* pcAltRefPelData;    // the prediction signal for the opposite list (not weighted)
    UInt            uiL1Search;         // 1 if current search is L1 search, else false
    const IntFrame* pcAltRefFrame;      // the reference frame of the opposite list
    const PW*       apcWeight[2];       // { list 0 prediction weight, list 1 prediction weight }
  };

protected:
  typedef struct
  {
    Void init( Short sLimit, Mv& rcMvPel, Mv cMin, Mv cMax)
    {
      cMin >>= 2;
      cMax >>= 2;
      Short sPosV = cMax.getVer() - rcMvPel.getVer();
      Short sNegV = rcMvPel.getVer() - cMin.getVer();
      iNegVerLimit = min( sLimit, sNegV) - rcMvPel.getVer();
      iPosVerLimit = min( sLimit, sPosV) + rcMvPel.getVer();

      Short sPosH = cMax.getHor() - rcMvPel.getHor();
      Short sNegH = rcMvPel.getHor() - cMin.getHor();
      iNegHorLimit = min( sLimit, sNegH) - rcMvPel.getHor();
      iPosHorLimit = min( sLimit, sPosH) + rcMvPel.getHor();
    }
    Int iNegVerLimit;
    Int iPosVerLimit;
    Int iNegHorLimit;
    Int iPosHorLimit;
  }
  SearchRect;

  typedef struct
  {
    XPel* pucYRef;
    XPel* pucURef;
    XPel* pucVRef;
    Int   iYStride;
    Int   iCStride;
    Int   iBestX;
    Int   iBestY;
    UInt  uiBestRound;
    UInt  uiBestDistance;
    UInt  uiBestSad;
    UChar ucPointNr;
  }
  IntTZSearchStrukt;

protected:
	MotionEstimation();
	virtual ~MotionEstimation();

public:
  ErrVal destroy();
  virtual ErrVal uninit();

  SampleWeighting* getSW() { return m_pcSampleWeighting; }

  ErrVal initMb( UInt uiMbPosY, UInt uiMbPosX, MbDataAccess& rcMbDataAccess );

  virtual ErrVal init(  XDistortion*  pcXDistortion,
                        CodingParameter* pcCodingParameter,
                        RateDistortionIf* pcRateDistortionIf,
                        QuarterPelFilter* pcQuarterPelFilter,
                        Transform*        pcTransform,
                        SampleWeighting* pcSampleWeighting);

  UInt    getRateCost           ( UInt                  uiBits,
                                  Bool                  bSad  )            { xGetMotionCost( bSad, 0 ); return xGetCost( uiBits ); }
  ErrVal  estimateBlockWithStart( const MbDataAccess&   rcMbDataAccess,
                                  const IntFrame&       rcRefFrame,
                                  Mv&                   rcMv,         // <-- MVSTART / --> MV
                                  Mv&                   rcMvPred,
                                  UInt&                 ruiBits,
                                  UInt&                 ruiCost,
                                  UInt                  uiBlk,
                                  UInt                  uiMode,
                                  Bool                  bQPelRefinementOnly,
                                  UInt                  uiSearchRange,
                                  const PW*             pcPW,
                                  MEBiSearchParameters* pcBSP = 0 );

	Bool          OmitPDISearch   ( Int x, Int y, Bool bQPel ); //JVT-W080 BUG_FIX

//JVT-W080
	Void          setPdsEnable        ( UInt uiValue ) { m_uiPdsEnable         = uiValue; }
	Void          setConstrainedMBNum ( UInt uiValue ) { m_uiConstrainedMBNum  = uiValue; }
	Void          setFrameWidthInMbs  ( UInt uiValue ) { m_uiFrameWidthInMbs   = uiValue; }
	Void          setCurrMBX          ( UInt uiValue ) { m_uiCurrMBX           = uiValue; }
	Void          setCurrMBY          ( UInt uiValue ) { m_uiCurrMBY           = uiValue; }
	UInt          getPdsEnable        ()         const { return m_uiPdsEnable;            }
	UInt          getConstrainedMBNum ()         const { return m_uiConstrainedMBNum;     }
	UInt          getFrameWidthInMbs  ()         const { return m_uiFrameWidthInMbs;      }
	UInt          getCurrMBX          ()         const { return m_uiCurrMBX;              }
	UInt          getCurrMBY          ()         const { return m_uiCurrMBY;              }
//~JVT-W080 
  virtual ErrVal compensateBlock( IntYuvMbBuffer *pcRecPelData, UInt uiBlk, UInt uiMode, IntYuvMbBuffer *pcRefPelData2 = NULL ) = 0;

  DFunc getDistortionFunction() { return m_cParams.getSubPelDFunc(); }

//TMM_WP
  Void setLstIdx( ListIdx eLstIdx) {m_eLstIdx = eLstIdx;}
  ListIdx getLstIdx() {return m_eLstIdx;}
//TMM_WP

protected:

  Void          xTZSearch             ( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, UInt uiSearchRange = 0 );
  __inline Void xTZSearchHelp         ( IntTZSearchStrukt& rcStrukt, const Int iSearchX, const Int iSearchY, const UChar ucPointNr, const UInt uiDistance );
  __inline Void xTZ2PointSearch       ( IntTZSearchStrukt& rcStrukt, SearchRect rcSearchRect );
  __inline Void xTZ8PointSquareSearch ( IntTZSearchStrukt& rcStrukt, SearchRect rcSearchRect, const Int iStartX, const Int iStartY, const Int iDist );
  __inline Void xTZ8PointDiamondSearch( IntTZSearchStrukt& rcStrukt, SearchRect rcSearchRect, const Int iStartX, const Int iStartY, const Int iDist );

  Void          xPelBlockSearch ( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD,                              UInt uiSearchRange = 0 );
  Void          xPelSpiralSearch( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD,                              UInt uiSearchRange = 0 );
  Void          xPelLogSearch   ( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, Bool bFme,  UInt uiStep = 4, UInt uiSearchRange = 0 );
  virtual Void  xSubPelSearch   ( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, UInt uiBlk, UInt uiMode,     Bool bQPelOnly ) = 0;

//TMM_WP
   Int getDistScaleFactor(Int iCurrPoc, Int iL0Poc, Int iL1Poc ) const;
//TMM_WP

protected:
  QuarterPelFilter* m_pcQuarterPelFilter;
  MotionVectorSearchParams m_cParams;
  Int m_iMaxLogStep;
  Mv *m_pcMvSpiralSearch;
  UInt m_uiSpiralSearchEntries;
  Mv m_cLastPelMv;
  Mv  m_acMvPredictors[3];

  XDistortion*      m_pcXDistortion;
  XDistSearchStruct m_cXDSS;
//JVT-W080
	UInt m_uiPdsEnable;
	UInt m_uiConstrainedMBNum;  
  UInt m_uiFrameWidthInMbs; 
	UInt m_uiCurrMBX;
	UInt m_uiCurrMBY;
//~JVT-W080
//TMM_WP
  ListIdx m_eLstIdx;
//TMM_WP
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_MOTIONESTIMATION_H__00B6343B_CCFC_466D_9F60_04EB29EE8E56__INCLUDED_)

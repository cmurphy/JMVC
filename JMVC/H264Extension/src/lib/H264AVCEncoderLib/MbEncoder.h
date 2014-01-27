#if !defined(AFX_MBENCODER_H__F725C8AD_2589_44AD_B904_62FE2A7F7D8D__INCLUDED_)
#define AFX_MBENCODER_H__F725C8AD_2589_44AD_B904_62FE2A7F7D8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/FrameMng.h"
#include "DistortionIf.h"
#include "RateDistortionIf.h"
#include "MotionEstimation.h"
#include "MbTempData.h"
#include "MbCoder.h"
#include "BitCounter.h"
#include "UvlcWriter.h"
#include "H264AVCCommonLib/Quantizer.h"


H264AVC_NAMESPACE_BEGIN

class Transform;
class FrameMng;
class IntraPredictionSearch;
class CodingParameter;

//TMM_WP
#define MAX_REF_FRAMES 64
//TMM_WP


class MbEncoder
: protected MbCoder
, public UvlcWriter
, protected BitCounter
{
protected:
	MbEncoder();
	virtual ~MbEncoder();

public:
  static ErrVal create( MbEncoder*& rpcMbEncoder );
  ErrVal destroy();

  ErrVal init(  Transform* pcTransform,
                IntraPredictionSearch* pcIntraPrediction,
                MotionEstimation *pcMotionEstimation,
                CodingParameter* pcCodingParameter,
                RateDistortionIf* pcRateDistortionIf,
                XDistortion* pcXDistortion
                );

  ErrVal uninit();
  ErrVal initSlice( const SliceHeader& rcSH);

  IntMbTempData* getBestIntData() {return m_pcIntMbBestData; }

  ErrVal  encodeIntra         ( MbDataAccess&   rcMbDataAccess,
                                MbDataAccess*   pcMbDataAccessBase,                                  
                                IntFrame*       pcFrame,
                                IntFrame*       pcRecSubband,
                                IntFrame*       pcBaseLayer,
                                IntFrame*       pcPredSignal,
                                Double          dLambda );
  ErrVal  encodeResidual      ( MbDataAccess&   rcMbDataAccess, 
                                IntFrame*       pcFrame,
                                IntFrame*       pcResidual,
                                IntFrame*       pcBaseSubband,
																IntFrame*				pcSRFrame, // JVT-R091
                                Bool&           rbCoded,
                                Double          dLambda,
                                Int             iMaxDeltaQp );
  ErrVal  encodeResidual      ( MbDataAccess&   rcMbDataAccess,
                                MbDataAccess&   rcMbDataAccessBL,
                                IntFrame*       pcResidual,
                                Double          dLambda,
                                Bool            bLowPass,
                                Int             iMaxQpDelta );

  ErrVal  compensatePrediction( MbDataAccess&   rcMbDataAccess,
                                IntFrame*       pcMCFrame,
                                RefFrameList&   rcRefFrameList0,
                                RefFrameList&   rcRefFrameList1,
                                Bool            bCalcMv,
                                Bool            bFaultTolerant );

  ErrVal  compensateUpdate(      MbDataAccess&   rcMbDataAccess,
                                 IntFrame*       pcMCFrame,
                                 Int             iRefIdx,
                                 ListIdx         eListPrd,
                                 IntFrame*       pcPrdFrame);


  ErrVal  encodeMacroblock    ( MbDataAccess&  rcMbDataAccess,
	  IntFrame*      pcFrame,
	  IntFrame*      pcOrigFrame,
	  RefFrameList&  rcList0,
	  RefFrameList&  rcList1,
	  UInt           uiNumMaxIter,
	  UInt           uiIterSearchRange,
	  Double         dLambda, 
      Double&       rdCost
      );

    ErrVal  encodeMacroblock    ( MbDataAccess&  rcMbDataAccess,
	  IntFrame*      pcFrame,
	  IntFrame*      pcOrigFrame,
	  RefFrameList&  rcList0,
	  RefFrameList&  rcList1,
	  UInt           uiNumMaxIter,
	  UInt           uiIterSearchRange,
	  Double         dLambda, 
      Double&       rdCost,
	  Bool            bSkipModeAllowed
      );
//TMM_WP
  ErrVal getPredWeights( SliceHeader& rcSH, ListIdx eLstIdx, 
                         Double(*pafWeight)[3], IntFrame* pOrgFrame,
                         RefFrameList& rcRefFrameListX);


  ErrVal getPredOffsets( SliceHeader& rcSH, ListIdx eLstIdx, 
                         Double(*pafOffsets)[3], IntFrame* pOrgFrame,
                         RefFrameList& rcRefFrameListX);

//TMM_WP


  //JVT-R057 LA-RDO{
  Void setLARDOEnable( Bool bLARDO)  { m_bLARDOEnable= bLARDO; }
 
  Void setLayerID (UInt uiLayer)     { m_uiLayerID=uiLayer;}
 
  Void setPLR( UInt auiPLR[5])       { for(UInt i=0;i<5;i++) m_auiPLR[i] = auiPLR[i];}

  Void setRatio( Double adRatio[5][2])
  { 
	  for(UInt i=0;i<5;i++)
		  for(UInt j=0;j<2;j++)
			  m_aadRatio[i][j] = adRatio[i][j];
  }
 Void setMBSSD      ( UInt uiSSD)      { m_uiMBSSD=uiSSD; }
 
  Bool getLARDOEnable(){ return m_bLARDOEnable;}
 
 Void setFrameEcEp  ( IntFrame* p1)    { m_pcFrameEcEp=p1; }

  Int  GetEC_REC            ( IntYuvPicBuffer* pPic1,
                              IntYuvPicBuffer* pPic2,
                              Int              blockX, 
                              Int              blockY);

  Void  getChannelDistortion( MbDataAccess&    rcMbDataAccess,
	  IntFrame&       rcRefFrame,
	                            Int              *distortion,
	                            Int              iMvX,
	                            Int              iMvY,
	                            Int              startX,
	                            Int              startY,
	                            Int              blockX,
	                            Int              blockY,
	                            Bool             bSpatial=false);
  
  Int getEpRef() { return m_iEpRef; }

  Void setEpRef(Int iRef)   { m_iEpRef=iRef; }
  
  Void  getDistortion       (Int              iDList0, 
                             Int              iDList1,
                             SampleWeighting* pcSampleWeighting,
                             MbDataAccess&    rcMbDataAccess);
  //JVT-R057 LA-RDO}

  //S051{
  Void		setUseBDir	( Bool bUse){ m_bUseBDir = bUse;}
  //S051}
//JVT-W080
	Void  setPdsEnable              ( UInt uiValue )   { m_uiPdsEnable                 = uiValue; }
	Void  setConstrainedMBNum       ( UInt uiValue )   { m_uiConstrainedMBNum          = uiValue; }
	Void  setFrameWidthInMbs        ( UInt uiValue )   { m_uiFrameWidthInMbs           = uiValue; }
	Void  setPdsBlockSize           ( UInt uiValue )   { m_uiPdsBlockSize              = uiValue; }
	Void  setCurrMBX                ( UInt uiValue )   { m_uiCurrMBX                   = uiValue; }
  Void  setCurrMBY                ( UInt uiValue )   { m_uiCurrMBY                   = uiValue; }
	Void  setPdsInitialDelayMinus2L0( UInt* uiValue )  { m_puiPdsInitialDelayMinus2L0  = uiValue; }
	Void  setPdsInitialDelayMinus2L1( UInt* uiValue )  { m_puiPdsInitialDelayMinus2L1  = uiValue; }
	UInt  getPdsEnable         ()         const { return m_uiPdsEnable;            }
	UInt  getFrameWidthInMbs   ()         const { return m_uiFrameWidthInMbs;      }
	UInt  getConstrainedMBNum  ()         const { return m_uiConstrainedMBNum;     }
	UInt  getPdsBlockSize      ()         const { return m_uiPdsBlockSize;         }
	UInt  getCurrMBX           ()         const { return m_uiCurrMBX;              }
	UInt  getCurrMBY           ()         const { return m_uiCurrMBY;              }
	UInt* getPdsInitialDelayMinus2L0 ()   const { return m_puiPdsInitialDelayMinus2L0; }
	UInt* getPdsInitialDelayMinus2L1 ()   const { return m_puiPdsInitialDelayMinus2L1; }
	//JVT-W080 BUG_FIX
  Bool  SkipPDISearch    ( const RefFrameList& rcRefFrameList, Int iRefIdx, IntMbTempData*& rpcMbTempData, Int x, Int y, UInt dir );
  Void  setPDIParameters( const RefFrameList& rcRefFrameList, Int iRefIdx, IntMbTempData*& rpcMbTempData, UInt dir );
	ErrVal xCheckSkipSearch( const RefFrameList& rcRefFrameList,
		                       Int                 iRefIdx, 
													 IntMbTempData*&     rpcMbTempData, 
													 Int x, Int y, UInt dir ); 
	//~JVT-W080 BUG_FIX

//~JVT-W080

protected:

  ErrVal  xScale4x4Block        ( TCoeff*            piCoeff,
                                  const UChar*       pucScale,
                                  UInt               uiStart,
                                  const QpParameter& rcQP );
 
  ErrVal  xScale8x8Block        ( TCoeff*            piCoeff,
                                  const UChar*       pucScale,
                                  const QpParameter& rcQP );
  ErrVal  xScaleTCoeffs         ( MbDataAccess&      rcMbDataAccess,
                                  MbTransformCoeffs& rcTCoeffs );

  ErrVal  xSetRdCostIntraMb     ( IntMbTempData&    rcMbTempData,
                                  UInt              uiCoeffBits,
                                  Bool              bBSlice,
                                  Bool              bBLSkip );
  
  ErrVal  xSetRdCostInterMb     ( IntMbTempData&    rcMbTempData,
                                  MbDataAccess*     pcMbDataAccessBase,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBLSkip          = false,
                                  UInt              uiAdditionalBits = 0,
                                  Bool              bSkipMCPrediction = false );

	//-- JVT-R012
  ErrVal  xSetRdCostInterMbSR   ( IntMbTempData&    rcMbTempData,
                                  MbDataAccess*     pcMbDataAccessBase,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
																	IntFrame*					pcBaseLayerSbb,
                                  Bool              bBLSkip          = false,
                                  UInt              uiAdditionalBits = 0 );
	//--
  ErrVal  xSetRdCost8x8InterMb  ( IntMbTempData&    rcMbTempData,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBLSkip          = false,
                                  UInt              uiAdditionalBits = 0,
                                  Bool              bSkipMCPrediction = false );

	//-- JVT-R012
  ErrVal  xSetRdCost8x8InterMbSR( IntMbTempData&    rcMbTempData,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
																	IntFrame*					pcBaseLayerSbb,
                                  Bool              bBLSkip          = false,
                                  UInt              uiAdditionalBits = 0 );
	//--
  ErrVal  xSetRdCostInterSubMb  ( IntMbTempData&    rcMbTempData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  B8x8Idx           c8x8Idx,
                                  Bool              bTrafo8x8,
                                  UInt              uiAddBits );

  ErrVal  xEncodeChromaIntra        ( IntMbTempData& rcMbTempData, UInt& ruiExtCbp, UInt& ruiBits );

  ErrVal  xEncode4x4IntraBlock      ( IntMbTempData& rcMbTempData, LumaIdx cIdx,     UInt& ruiBits, UInt& ruiExtCbp );
  ErrVal  xEncode4x4InterBlock      ( IntMbTempData& rcMbTempData, LumaIdx cIdx,     UInt& ruiBits, UInt& ruiExtCbp );
  ErrVal  xEncode8x8InterBlock      ( IntMbTempData& rcMbTempData, B8x8Idx c8x8Idx,  UInt& ruiBits, UInt& ruiExtCbp );
  ErrVal  xEncode8x8IntraBlock      ( IntMbTempData& rcMbTempData, B8x8Idx cIdx,     UInt& ruiBits, UInt& ruiExtCbp );
  ErrVal  xEncodeChromaTexture      ( IntMbTempData& rcMbTempData, UInt& ruiExtCbp, UInt& ruiBits );

  Void    xReStoreParameter     ( MbDataAccess&     rcMbDataAccess, IntMbTempData& rcMbBestData );
  Void    xUpDateBest           ( IntMbTempData&    rcMbTempData );

  ErrVal  xCheckInterMbMode8x8  ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  IntMbTempData*    pcMbRefData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  MbDataAccess*     pcMbDataAccessBaseMotion );

	//-- JVT-R012
  ErrVal  xCheckInterMbMode8x8SR( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  IntMbTempData*    pcMbRefData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
																	IntFrame*					pcBaseLayerSbb,
                                  MbDataAccess*     pcMbDataAccessBaseMotion );
	//--

  ErrVal  xEstimateMbIntraBL    ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  const IntFrame*   pcBaseLayerRec,
                                  Bool              bBSlice,
                                  MbDataAccess*     pcMbDataAccessBase );
  ErrVal  xEstimateMbIntraBL8x8 ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  const IntFrame*   pcBaseLayerRec,
                                  Bool              bBSlice,
                                  Bool              bBLSkip );
  ErrVal  xEstimateMbIntra16    ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  Bool              bBSlice  );
  ErrVal  xEstimateMbIntra8     ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  Bool              bBSlice  );
  ErrVal  xEstimateMbIntra4     ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  Bool              bBSlice  );
  ErrVal  xEstimateMbPCM        ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  Bool              bBSlice  );
  
  ErrVal  xEstimateMbSkip       ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1 );
  ErrVal  xEstimateMbBLSkip     ( IntMbTempData*&   rpcIntMbTempData,
                                  IntMbTempData*&   rpcIntMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  const IntFrame*   pcBaseLayerRec,
                                  Bool              bBSlice,
                                  Int				iSpatialScalabilityType,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  Bool              bResidualPred );
	//-- JVT-R091
  ErrVal  xEstimateMbSR					( IntMbTempData*&   rpcIntMbTempData,
                                  IntMbTempData*&   rpcIntMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  const IntFrame*   pcBaseLayerSbb,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  Bool              bResidualPred );
	//--

  ErrVal  xEstimateMbDirect     ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  Bool              bResidualPred );
  ErrVal xEstimateMbDirect( IntMbTempData*&  rpcMbTempData,
                              IntMbTempData*&  rpcMbBestData,
                              RefFrameList&    rcRefFrameList0,
                              RefFrameList&    rcRefFrameList1,
                              MbDataAccess*    pcMbDataAccessBaseMotion,
                              Bool             bResidualPred,
							  Bool             bSkipModeAllowed);
  ErrVal  xEstimateMb16x16      ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBiPredOnly,
                                  UInt              uiNumMaxIter,
                                  UInt              uiIterSearchRange,
                                  Bool              bQPelRefinementOnly,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  Bool              bResidualPred );
  ErrVal  xEstimateMb16x8       ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBiPredOnly,
                                  UInt              uiNumMaxIter,
                                  UInt              uiIterSearchRange,
                                  Bool              bQPelRefinementOnly,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  Bool              bResidualPred );
  ErrVal  xEstimateMb8x16       ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBiPredOnly,
                                  UInt              uiNumMaxIter,
                                  UInt              uiIterSearchRange,
                                  Bool              bQPelRefinementOnly,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  Bool              bResidualPred );
  ErrVal  xEstimateMb8x8        ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBiPredOnly,
                                  UInt              uiNumMaxIter,
                                  UInt              uiIterSearchRange,
                                  Bool              bQPelRefinementOnly,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  Bool              bResidualPred );
  ErrVal  xEstimateMb8x8Frext   ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBiPredOnly,
                                  UInt              uiNumMaxIter,
                                  UInt              uiIterSearchRange,
                                  Bool              bQPelRefinementOnly,
                                  MbDataAccess*     pcMbDataAccessBaseMotion,
                                  Bool              bResidualPred );
  ErrVal  xEstimateSubMbDirect  ( Par8x8            ePar8x8,
                                  IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bTrafo8x8,
                                  UInt              uiAddBits,
                                  MbDataAccess*     pcMbDataAccessBaseMotion );
  ErrVal  xEstimateSubMb8x8     ( Par8x8            ePar8x8,
                                  IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bTrafo8x8,
                                  Bool              bBiPredOnly,
                                  UInt              uiNumMaxIter,
                                  UInt              uiIterSearchRange,
                                  UInt              uiAddBits,
                                  Bool              bQPelRefinementOnly,
                                  MbDataAccess*     pcMbDataAccessBaseMotion );
  ErrVal  xEstimateSubMb8x4     ( Par8x8            ePar8x8,
                                  IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBiPredOnly,
                                  UInt              uiNumMaxIter,
                                  UInt              uiIterSearchRange,
                                  UInt              uiAddBits,
                                  Bool              bQPelRefinementOnly,
                                  MbDataAccess*     pcMbDataAccessBaseMotion );
  ErrVal  xEstimateSubMb4x8     ( Par8x8            ePar8x8,
                                  IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBiPredOnly,
                                  UInt              uiNumMaxIter,
                                  UInt              uiIterSearchRange,
                                  UInt              uiAddBits,
                                  Bool              bQPelRefinementOnly,
                                  MbDataAccess*     pcMbDataAccessBaseMotion );
  ErrVal  xEstimateSubMb4x4     ( Par8x8            ePar8x8,
                                  IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bBiPredOnly,
                                  UInt              uiNumMaxIter,
                                  UInt              uiIterSearchRange,
                                  UInt              uiAddBits,
                                  Bool              bQPelRefinementOnly,
                                  MbDataAccess*     pcMbDataAccessBaseMotion );
  
  ErrVal  xCheckBestEstimation  ( IntMbTempData*&   rpcMbTempData,
                                  IntMbTempData*&   rpcMbBestData );

  Void    xStoreEstimation      ( MbDataAccess&     rcMbDataAccess,
                                  IntMbTempData&    rcMbBestData,
                                  IntFrame*         pcRecSubband,
                                  IntFrame*         pcPredSignal,
                                  Bool              bMotionFieldEstimation,
                                  IntYuvMbBuffer*   pcBaseLayerBuffer );
  Bool    xCheckUpdate          ( IntYuvMbBuffer&   rcPredBuffer,
                                  IntYuvMbBuffer&   rcOrigBuffer,
                                  LumaIdx           cIdx,
                                  Int               iXSize,
                                  Int               iYSize );


  ErrVal  xEncode16x16ResidualMB( IntMbTempData&    rcMbTempData,
                                  UInt&             ruiBits,
                                  UInt&             ruiExtCbp );


  UInt  xCalcMbCbp    ( UInt uiExtCbp );

private:
  UChar xGetFrameBits ( ListIdx eLstIdx, Int iRefPic );

protected:
  CodingParameter* m_pcCodingParameter;
  Transform*   m_pcTransform;
  IntraPredictionSearch*    m_pcIntraPrediction;
  MotionEstimation *m_pcMotionEstimation;
  RateDistortionIf* m_pcRateDistortionIf;
  XDistortion*  m_pcXDistortion;
  Bool bInitDone;
  Bool m_bISlice;
  Bool m_bBSlice;
  Bool m_bCabac;
  IntMbTempData  m_acIntMbTempData[5];
  IntMbTempData* m_pcIntMbBestData;
  IntMbTempData* m_pcIntMbTempData;
  IntMbTempData* m_pcIntMbBest8x8Data;
  IntMbTempData* m_pcIntMbTemp8x8Data;
  IntMbTempData* m_pcIntMbBestIntraChroma;

  IntYuvMbBuffer  *m_pcIntOrgMbPelData;
  IntYuvPicBuffer *m_pcIntPicBuffer;
	  IntYuvPicBuffer *m_pcIntPicBufferOrig;
  IntYuvPicBuffer *m_pcIntraPredPicBuffer;

  UInt m_uiMaxRefFrames[2];
  UInt m_uiMaxRefPics[2];
//JVT-W080
	UInt   m_uiPdsEnable;
	UInt   m_uiConstrainedMBNum;  
	UInt   m_uiFrameWidthInMbs; 
	UInt   m_uiCurrMBX;
	UInt   m_uiCurrMBY;
	UInt*  m_puiPdsInitialDelayMinus2L0;
	UInt*  m_puiPdsInitialDelayMinus2L1;
	UInt   m_uiPdsBlockSize;
//~JVT-W080
  BitWriteBufferIf* m_BitCounter;
  //JVT-R057 LA-RDO{
  Bool m_bLARDOEnable;
  UInt m_uiLayerID;
  UInt m_auiPLR[5];
  Double m_aadRatio[5][2];
  UInt m_uiMBSSD;
  IntFrame* m_pcFrameEcEp;
  Int  m_iEpRef;
  Double m_dWr0;
  Double m_dWr1;
  //JVT-R057 LA-RDO}

  //S051{
  Bool		m_bUseBDir;
  //S051}

};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_MBENCODER_H__F725C8AD_2589_44AD_B904_62FE2A7F7D8D__INCLUDED_)

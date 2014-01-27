#if !defined(AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_)
#define AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/Mv.h"
#include "H264AVCCommonLib/MbMvData.h"
#include "H264AVCCommonLib/MbTransformCoeffs.h"
#include "H264AVCCommonLib/MbDataStruct.h"


H264AVC_NAMESPACE_BEGIN


class H264AVCCOMMONLIB_API MbData :
public MbDataStruct
{
public:
  MbData()
  : m_pcMbTCoeffs         ( 0 )
  , m_bHasMotionRefinement( false )
  {
    m_apcMbMvdData   [ LIST_0 ]  = NULL;
    m_apcMbMvdData   [ LIST_1 ]  = NULL;
    m_apcMbMotionData[ LIST_0 ]  = NULL;
    m_apcMbMotionData[ LIST_1 ]  = NULL;

  }

  ~MbData()
  {
  }

  Void init(  MbTransformCoeffs*  pcMbTCoeffs,
              MbMvData*           pcMbMvdDataList0,
              MbMvData*           pcMbMvdDataList1,
              MbMotionData*       pcMbMotionDataList0,
              MbMotionData*       pcMbMotionDataList1,
              MbMotionData*       pcMbMotionDataBaseList0 = NULL,
              MbMotionData*       pcMbMotionDataBaseList1 = NULL)
  {
    AOT( m_bHasMotionRefinement );
    m_pcMbTCoeffs           = pcMbTCoeffs;
    m_apcMbMvdData[0]       = pcMbMvdDataList0;
    m_apcMbMvdData[1]       = pcMbMvdDataList1;
    m_apcMbMotionData[0]    = pcMbMotionDataList0;
    m_apcMbMotionData[1]    = pcMbMotionDataList1;
    m_apcMbMotionDataBase[0] = pcMbMotionDataBaseList0;
    m_apcMbMotionDataBase[1] = pcMbMotionDataBaseList1;

  }

public:
  MbTransformCoeffs&        getMbTCoeffs    ()                          { return *m_pcMbTCoeffs; }
  MbMvData&                 getMbMvdData    ( ListIdx eListIdx )        { return *m_apcMbMvdData   [ eListIdx ]; }
  MbMotionData&             getMbMotionData ( ListIdx eListIdx )        { return *m_apcMbMotionData[ eListIdx ]; }

  const MbTransformCoeffs&  getMbTCoeffs    ()                    const { return *m_pcMbTCoeffs; }
  const MbMvData&           getMbMvdData    ( ListIdx eListIdx )  const { return *m_apcMbMvdData   [ eListIdx ]; }
  const MbMotionData&       getMbMotionData ( ListIdx eListIdx )  const { return *m_apcMbMotionData[ eListIdx ]; }

  MbMotionData&             getMbMotionDataBase ( ListIdx eListIdx )        { return m_bHasMotionRefinement ? *m_apcMbMotionDataBase[eListIdx] : *m_apcMbMotionData[ eListIdx ]; }
  const MbMotionData&       getMbMotionDataBase ( ListIdx eListIdx )  const { return m_bHasMotionRefinement ? *m_apcMbMotionDataBase[eListIdx] : *m_apcMbMotionData[ eListIdx ]; }

  Void                      switchMotionRefinement();
  Void                      activateMotionRefinement();
  Void                      deactivateMotionRefinement()                    { m_bHasMotionRefinement = false; }

  operator MbTransformCoeffs& ()                                        { return *m_pcMbTCoeffs; }

  Void copy( const MbData& rcMbData );

  ErrVal  loadAll( FILE* pFile );
  ErrVal  saveAll( FILE* pFile );

  ErrVal  copyMotion    ( MbData& rcMbData, UInt    uiSliceId = MSYS_UINT_MAX );
  ErrVal  copyMotionBL  ( MbData& rcMbData, Bool bDirect8x8, UInt    uiSliceId = MSYS_UINT_MAX );
  ErrVal  upsampleMotion( MbData& rcMbData, Par8x8  ePar8x8, Bool bDirect8x8   );

	// TMM_ESS {
  ErrVal upsampleMotionESS( MbData* pcBaseMbData,
                            const UInt uiBaseMbStride,
                            const Int aiPelOrig[2],
                            const Bool bDirect8x8,
                            ResizeParameters* pcParameters);
  ErrVal  noUpsampleMotion(); 
  // TMM_ESS }
//JVT-T054{
  ErrVal  copyMbCbp( MbData& rcMbData );
  ErrVal  initMbCbp();
//JVT-T054}


protected:
  MbTransformCoeffs*  m_pcMbTCoeffs;
  MbMvData*           m_apcMbMvdData[2];
  MbMotionData*       m_apcMbMotionData[2];

  MbMode              m_eMbModeBase;
  BlkMode             m_aBlkModeBase[4];
  MbMotionData*       m_apcMbMotionDataBase[2];
  Bool                m_bHasMotionRefinement;

  static const UChar		    m_aucPredictor  [2][4];
  static const Char         aaacGetPartInfo [7][4][16];
 
  MbData*                   m_apcMbData     [4];
  SChar                     m_ascBl4x4RefIdx[2][16];// ref index of list_0/1 for each 4x4 blk
  Mv                        m_acBl4x4Mv	    [2][16];// motion vectors of list_0/1 for each 4x4 blk

  ErrVal xInitESS             ( );

 ErrVal  xFillBaseMbData( MbData*           pcBaseMbData,
                          const UInt        uiBaseMbStride,
                          const Int         aiPelOrig[2],
                          const Bool        bDirect8x8,
                          ResizeParameters* pcParameters,
                          MbMode            aeMbMode [4],
                          BlkMode           aeBlkMode[4][4],
                          UInt&             uiMbBaseOrigX,
                          UInt&             uiMbBaseOrigY);
 
 ErrVal xBuildPartInfo(   const Int aiPelOrig[2],
                          ResizeParameters* pcParameters,
                          const MbMode      aeMbMode[4],
                          const  BlkMode     aeBlkMode[4][4],  
                          UInt          aui4x4Idx[4][4],
                          UInt          auiMbIdx [4][4], 
                          Int          aaiPartInfo[4][4],
                          Bool          abBl8x8Intra[4],
                          const UInt    uiMbBaseOrigX,
                          const UInt    uiMbBaseOrigY );

  ErrVal xInheritMbMotionData ( const Int        aaiPartInfo[4][4]	);

  ErrVal xInherit8x8MotionData( const UInt        aui4x4Idx  [4][4],
                                const UInt        auiMbIdx	  [4][4], 
                                const Int        aaiPartInfo[4][4]);

  ErrVal xFillMbMvData		  ( ResizeParameters* pcParameters );
							  
  ErrVal xMergeBl8x8MvAndRef( const UInt uiBlIdx	);

  ErrVal xFillMvandRefBl4x4	( const UInt uiBlIdx, const UChar* pucWhich, const UInt uiList, const SChar* psChosenRefIdx );

  ErrVal xRemoveIntra8x8(const UInt uiBlIdx, 
                          const Bool* abBl8x8Intra,
                          UInt  aui4x4Idx[4][4],
                          UInt  auiMbIdx[4][4],
                          Int   aaiPartInfo[4][4]);

  ErrVal  xCopyBl8x8(const UInt uiBlIdx,
                      const UInt uiBlIdxCopy,
                      UInt  aui4x4Idx[4][4],
                      UInt  auiMbIdx[4][4],
                      Int  aaiPartInfo[4][4]);

  ErrVal xRemoveIntra4x4(const UInt uiBlIdx,
                          UInt  aui4x4Idx[4],
                          UInt  auiMbIdx[4],
                          Int  aaiPartInfo[4]);
};

class MbDataBuffer : public MbData
{
public :
    MbDataBuffer()
    {
        MbData::init( &m_cMbTransformCoeffs, m_acMbMvData, m_acMbMvData+1, m_acMbMotionData, m_acMbMotionData + 1);
    }
    virtual ~MbDataBuffer() {}

    MbTransformCoeffs m_cMbTransformCoeffs;
    MbMvData          m_acMbMvData[2];
    MbMotionData      m_acMbMotionData[2];
};

H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_)

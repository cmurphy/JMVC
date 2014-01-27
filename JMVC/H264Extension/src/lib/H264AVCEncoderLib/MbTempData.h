#if !defined(AFX_MBTEMPDATA_H__2D7469B9_B5A8_44FE_AA77_6BA0269C8F37__INCLUDED_)
#define AFX_MBTEMPDATA_H__2D7469B9_B5A8_44FE_AA77_6BA0269C8F37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"


H264AVC_NAMESPACE_BEGIN



class IntMbTempData :
public CostData
, public MbData
, public MbTransformCoeffs
, public IntYuvMbBuffer
{
public:
	IntMbTempData               ();
	virtual ~IntMbTempData      ();

  ErrVal  init                ( MbDataAccess& rcMbDataAccess );
  ErrVal  uninit              ();

  Void    clear               ();
  Void    clearCost           ();

  UInt&   cbp                 ()                { return m_uiMbCbp; }

  Void    copyTo              ( MbDataAccess&   rcMbDataAccess );
  Void    loadChromaData      ( IntMbTempData&  rcMbTempData );

  Void    copyResidualDataTo  ( MbDataAccess&   rcMbDataAccess );

  MbMotionData&       getMbMotionData   ( ListIdx eLstIdx ) { return m_acMbMotionData [ eLstIdx ]; }
  MbMvData&           getMbMvdData      ( ListIdx eLstIdx ) { return m_acMbMvdData    [ eLstIdx ]; }
  IntYuvMbBuffer&     getTempYuvMbBuffer()                  { return m_cTempYuvMbBuffer; }
  MbDataAccess&       getMbDataAccess   ()                  { AOF_DBG(m_pcMbDataAccess); return *m_pcMbDataAccess; }
  const SliceHeader&  getSH             ()            const { AOF_DBG(m_pcMbDataAccess); return m_pcMbDataAccess->getSH(); }
  const CostData&     getCostData       ()            const { return *this; }
  operator MbDataAccess&                ()                  { AOF_DBG(m_pcMbDataAccess); return *m_pcMbDataAccess; }
  operator IntYuvMbBuffer*              ()                  { return this; }

protected:
  MbDataAccess*       m_pcMbDataAccess;
  MbMvData            m_acMbMvdData[2];
  MbMotionData        m_acMbMotionData[2];
  IntYuvMbBuffer      m_cTempYuvMbBuffer;


};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBTEMPDATA_H__2D7469B9_B5A8_44FE_AA77_6BA0269C8F37__INCLUDED_)

#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/MotionVectorCalculation.h"



H264AVC_NAMESPACE_BEGIN


MotionVectorCalculation::MotionVectorCalculation() :
  m_uiMaxBw           ( 0 ),
  m_bSpatialDirectMode( false )
{
}


MotionVectorCalculation::~MotionVectorCalculation()
{
}


ErrVal MotionVectorCalculation::initSlice( const SliceHeader& rcSH )
{
  m_bSpatialDirectMode  = rcSH.getDirectSpatialMvPredFlag();
  m_uiMaxBw             = rcSH.isInterB() ? 2 : 1;

  return Err::m_nOK;
}


ErrVal MotionVectorCalculation::uninit()
{
  return Err::m_nOK;
}

Void MotionVectorCalculation::xCalcSDirect( MbDataAccess& rcMbDataAccess,
                                            MbDataAccess* pcMbDataAccessBase )
{
  Mv    cMv;
  SChar scRefPic;

  MbDataAccess* pTmp = rcMbDataAccess.getMbDataAccessBase();
  rcMbDataAccess.setMbDataAccessBase( NULL );

  for( UInt uiBw = 0; uiBw < 2; uiBw++ )
  {
    ListIdx       eListIdx        = ListIdx( uiBw );
    MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx() ) )
    {
      if( rcMbMotionData.getMotPredFlag() )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv();
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx );
      }

      rcMbMotionData.setAllMv( cMv );
    }
  }
  rcMbDataAccess.setMbDataAccessBase( pTmp );
}



Void MotionVectorCalculation::xCalc16x16( MbDataAccess& rcMbDataAccess,
                                          MbDataAccess* pcMbDataAccessBase )
{
  Mv    cMv;
  SChar scRefPic;

  for( UInt uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
  {
    ListIdx       eListIdx        = ListIdx( uiBw );
    MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
    MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx() ) )
    {
      if( rcMbMotionData.getMotPredFlag() )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv();
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx );
      }
      cMv += rcMbMvdData.getMv();

      rcMbMotionData.setAllMv( cMv );
    }
  }
}


Void MotionVectorCalculation::xCalc16x8( MbDataAccess&  rcMbDataAccess,
                                         MbDataAccess*  pcMbDataAccessBase )
{
  Mv    cMv;
  SChar scRefPic;

  for( UInt uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
  {
    ListIdx       eListIdx        = ListIdx( uiBw );
    MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
    MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx( PART_16x8_0 ) ) )
    {
      if( rcMbMotionData.getMotPredFlag( PART_16x8_0 ) )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( PART_16x8_0 );
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, PART_16x8_0 );
      }
      cMv += rcMbMvdData.getMv( PART_16x8_0 );

      rcMbMotionData.setAllMv( cMv, PART_16x8_0 );
    }
    if( 0 < (scRefPic = rcMbMotionData.getRefIdx( PART_16x8_1 ) ) )
    {
      if( rcMbMotionData.getMotPredFlag( PART_16x8_1 ) )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( PART_16x8_1 );
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, PART_16x8_1 );
      }
      cMv += rcMbMvdData.getMv( PART_16x8_1 );

      rcMbMotionData.setAllMv( cMv, PART_16x8_1 );
    }
  }
}


Void MotionVectorCalculation::xCalc8x16( MbDataAccess&  rcMbDataAccess,
                                         MbDataAccess*  pcMbDataAccessBase )
{
  Mv    cMv;
  SChar scRefPic;

  for( UInt uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
  {
    ListIdx       eListIdx        = ListIdx( uiBw );
    MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
    MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx( PART_8x16_0 ) ) )
    {
      if( rcMbMotionData.getMotPredFlag( PART_8x16_0 ) )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( PART_8x16_0 );
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, PART_8x16_0 );
      }
      cMv += rcMbMvdData.getMv( PART_8x16_0 );

      rcMbMotionData.setAllMv( cMv, PART_8x16_0 );
    }

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx( PART_8x16_1 ) ) )
    {
      if( rcMbMotionData.getMotPredFlag( PART_8x16_1 ) )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( PART_8x16_1 );
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, PART_8x16_1 );
      }
      cMv += rcMbMvdData.getMv( PART_8x16_1 );

      rcMbMotionData.setAllMv( cMv, PART_8x16_1 );
    }
  }
}



Void MotionVectorCalculation::xCalc8x8( B8x8Idx       c8x8Idx,
                                        MbDataAccess& rcMbDataAccess,
                                        MbDataAccess* pcMbDataAccessBase,
                                        Bool          bFaultTolerant )
{
  Mv    cMv;
  SChar scRefPic;
  UInt  uiBw;

  ParIdx8x8 eParIdx = c8x8Idx.b8x8();

  switch( rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) )
  {
    case BLK_SKIP:
    {
      MbDataAccess* pTmp = rcMbDataAccess.getMbDataAccessBase();
      rcMbDataAccess.setMbDataAccessBase( NULL );
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx );
          }
          rcMbMotionData.setAllMv( cMv, eParIdx );
        }
      }
      rcMbDataAccess.setMbDataAccessBase( pTmp );
      break;
    }
    case BLK_8x8:
    {
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
        MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx );
          }
          cMv += rcMbMvdData.getMv( eParIdx );
          rcMbMotionData.setAllMv( cMv, eParIdx );
        }
      }
      break;
    }
    case BLK_8x4:
    {
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
        MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_8x4_0 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_8x4_0 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_8x4_0 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_8x4_0 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_8x4_1 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_8x4_1 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_8x4_1 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_8x4_1 );
        }
      }
      break;
    }
    case BLK_4x8:
    {
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
        MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x8_0 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x8_0 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x8_0 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x8_0 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x8_1 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x8_1 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x8_1 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x8_1 );
        }
      }
      break;
    }
    case BLK_4x4:
    {
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
        MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_0 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_0 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_0 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_0 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_1 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_1 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_1 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_1 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_2 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_2 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_2 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_2 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_3 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_3 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_3 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_3 );
        }
      }
      break;
    }
    default:
    {
      AF();
      break;
    }
  }
}




Void MotionVectorCalculation::xCalc8x8( MbDataAccess& rcMbDataAccess,
                                        MbDataAccess* pcMbDataAccessBase,
                                        Bool          bFaultTolerant )
{
  Mv    cMv;
  SChar scRefPic;
  UInt  uiBw;

  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    ParIdx8x8 eParIdx = c8x8Idx.b8x8();

    switch( rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) )
    {
      case BLK_SKIP:
      {
        Bool bOneMv;
        AOF( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ) );
        break;
      }
      case BLK_8x8:
      {
        for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
        {
          ListIdx       eListIdx        = ListIdx( uiBw );
          MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
          MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

          if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
          {
            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx );
            }
            cMv += rcMbMvdData.getMv( eParIdx );

            rcMbMotionData.setAllMv( cMv, eParIdx );
          }
        }
        break;
      }
      case BLK_8x4:
      {
        for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
        {
          ListIdx       eListIdx        = ListIdx( uiBw );
          MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
          MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

          if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
          {
            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_8x4_0 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_8x4_0 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_8x4_0 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_8x4_0 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_8x4_1 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_8x4_1 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_8x4_1 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_8x4_1 );
          }
        }
        break;
      }
      case BLK_4x8:
      {
        for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
        {
          ListIdx       eListIdx        = ListIdx( uiBw );
          MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
          MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

          if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
          {
            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x8_0 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x8_0 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x8_0 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x8_0 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x8_1 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x8_1 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x8_1 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x8_1 );
          }
        }
        break;
      }
      case BLK_4x4:
      {
        for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
        {
          ListIdx       eListIdx        = ListIdx( uiBw );
          MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
          MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

          if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
          {
            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_0 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_0 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_0 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_0 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_1 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_1 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_1 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_1 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_2 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_2 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_2 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_2 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_3 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_3 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_3 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_3 );
          }
        }
        break;
      }
      default:
      {
        AF();
        break;
      }
    }
  }
}


H264AVC_NAMESPACE_END



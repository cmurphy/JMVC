#include "H264AVCCommonLib.h"

#include "H264AVCCommonLib/SliceHeader.h"
#include "H264AVCCommonLib/PictureParameterSet.h"
#include "H264AVCCommonLib/IntFrame.h"



H264AVC_NAMESPACE_BEGIN



SliceHeader::SliceHeader( const SequenceParameterSet& rcSPS,
                          const PictureParameterSet&  rcPPS )
: SliceHeaderBase   ( rcSPS, rcPPS ),
  m_uiLastMbInSlice ( 0 ), //--ICU/ETRI FMO Implementation
  m_pcFrameUnit     ( 0 ),
  m_bList1ShortTerm (true)
{
  m_auiNumRefIdxActive[LIST_0] = m_rcPPS.getNumRefIdxActive( LIST_0 );
  m_auiNumRefIdxActive[LIST_1] = m_rcPPS.getNumRefIdxActive( LIST_1 );

  ANOK( xInitScalingMatrix() );
}




ErrVal
SliceHeader::xInitScalingMatrix()
{
  if( ! m_rcSPS.getSeqScalingMatrixPresentFlag() && ! m_rcPPS.getPicScalingMatrixPresentFlag() )
  {
    m_acScalingMatrix.setAll( NULL );
    return Err::m_nOK;
  }

  for( UInt n = 0; n < m_acScalingMatrix.size(); n++ )
  {
    const UChar* puc = m_rcPPS.getPicScalingMatrix().get(n);
    if( puc == NULL )
    {
      puc = m_rcSPS.getSeqScalingMatrix().get(n);
    }
    if( puc == NULL &&  ( 1 == n || 2 == n || 4 == n || 5 == n ) )
    {
      puc = m_acScalingMatrix.get( n - 1);
    }
    if( puc == NULL )
    {
      switch(n)
      {
      case 0 : puc = g_aucScalingMatrixDefault4x4Intra; break;
      case 3 : puc = g_aucScalingMatrixDefault4x4Inter; break;
      case 6 : puc = g_aucScalingMatrixDefault8x8Intra; break;
      case 7 : puc = g_aucScalingMatrixDefault8x8Inter; break;
      default:
        AF()
      }
    }
    m_acScalingMatrix.set( n, puc );
  }

  return Err::m_nOK;
}

Void SliceHeader::getMbPositionFromAddress( UInt& ruiMbY, UInt& ruiMbX, const UInt uiMbAddress ) const 
{
    const UInt uiMbsInRow = getSPS().getFrameWidthInMbs();
    if( isMbAff() )
    {
        ruiMbY = ( 2*(uiMbAddress/(2*uiMbsInRow))+uiMbAddress%2);
        ruiMbX = ( (uiMbAddress/2) % uiMbsInRow);
    }
    else
    {
        ruiMbY = ( uiMbAddress / uiMbsInRow );
        ruiMbX = ( uiMbAddress % uiMbsInRow );
    }
}



Void SliceHeader::getMbPositionFromAddress( UInt& ruiMbY, UInt& ruiMbX, UInt& ruiMbIndex, const UInt uiMbAddress ) const 
{
    const UInt uiMbsInRow = getSPS().getFrameWidthInMbs();
    if( isMbAff() )
    {
        ruiMbY = ( 2*(uiMbAddress/(2*uiMbsInRow))+uiMbAddress%2);
        ruiMbX = ( (uiMbAddress/2) % uiMbsInRow);
        ruiMbIndex = uiMbsInRow * ruiMbY + ruiMbX;
    }
    else
    {
        ruiMbY = ( uiMbAddress / uiMbsInRow );
        ruiMbX = ( uiMbAddress % uiMbsInRow );
        ruiMbIndex = uiMbAddress;
    }
}
UInt SliceHeader::getMbIndexFromAddress( UInt uiMbAddress ) const
{
    if( isMbAff() )
    {
        UInt uiMbIndex;
        const UInt uiMbsInRow = getSPS().getFrameWidthInMbs();
        uiMbIndex = uiMbsInRow*(2*(uiMbAddress/(2*uiMbsInRow))+uiMbAddress%2) + (uiMbAddress/2) % uiMbsInRow;
        return uiMbIndex;
    }
    return uiMbAddress;
}

SliceHeader::~SliceHeader()
{
}



ErrVal
SliceHeader::compare( const SliceHeader* pcSH,
                     Bool&              rbNewPic,
                     Bool&              rbNewFrame ) const
{
    rbNewPic = rbNewFrame = true;

    if( isIdrNalUnit() )
    {
        ROTRS( NULL == pcSH,                                    Err::m_nOK ); //very first frame
        ROTRS( ! pcSH->isIdrNalUnit(),                          Err::m_nOK ); //previous no idr
        ROTRS( getIdrPicId() != pcSH->getIdrPicId(),            Err::m_nOK );
    }

    ROTRS( NULL == pcSH,                                      Err::m_nOK );
    ROTRS( isNalRefIdc() != pcSH->isNalRefIdc(),              Err::m_nOK );
    ROTRS( getFrameNum() != pcSH->getFrameNum(),              Err::m_nOK );

	ROTRS( getViewId() != pcSH->getViewId(),                  Err::m_nOK );

    ROTRS( getFieldPicFlag() != pcSH->getFieldPicFlag(),      Err::m_nOK );

    const Bool bSameParity = ( ! getFieldPicFlag() || ( getBottomFieldFlag() == pcSH->getBottomFieldFlag() ) );
    rbNewFrame = bSameParity;

    if( getSPS().getPicOrderCntType() == 0 )
    {
        ROTRS( getPicOrderCntLsb() != pcSH->getPicOrderCntLsb(), Err::m_nOK );
        ROTRS( ! getFieldPicFlag() && (getDeltaPicOrderCntBottom() != pcSH->getDeltaPicOrderCntBottom()), Err::m_nOK );
    }

    if( getSPS().getPicOrderCntType() == 1 )
    {
        ROTRS( getDeltaPicOrderCnt( 0 ) != pcSH->getDeltaPicOrderCnt( 0 ), Err::m_nOK );
        ROTRS( ! getFieldPicFlag() && (getDeltaPicOrderCnt( 1 ) != pcSH->getDeltaPicOrderCnt( 1 )), Err::m_nOK );
    }

    ROTRS( ! getFieldPicFlag() && (getDeltaPicOrderCntBottom() != pcSH->getDeltaPicOrderCntBottom()), Err::m_nOK );

    rbNewFrame = false;

    if( isIdrNalUnit() )
    {
        ROTRS( ! pcSH->isIdrNalUnit(),                          Err::m_nOK ); //prev no idr
    }
    ROTRS( ! bSameParity,  Err::m_nOK ); // differ

    rbNewPic = false;

    return Err::m_nOK;
}


Int
SliceHeader::getDistScaleFactor( PicType eMbPicType,
                                SChar sL0RefIdx,
                                SChar sL1RefIdx ) const
{
    const Frame*  pcFrameL0 = getRefPic( sL0RefIdx, eMbPicType, LIST_0 ).getFrame();
    const Frame*  pcFrameL1 = getRefPic( sL1RefIdx, eMbPicType, LIST_1 ).getFrame();
    Int           iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
    if( iDiffPocD == 0 )
    {
        return 1024;
    }
    else
    {
        Int iDiffPocB = getPoc( eMbPicType ) - pcFrameL0->getPOC();
        Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
        Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
    Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
        Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
        return iScale;
    }
}

//TMM_EC {{

Int
SliceHeader::getDistScaleFactorVirtual( PicType eMbPicType,
                                SChar sL0RefIdx,
                                SChar sL1RefIdx,
                                RefFrameList& rcRefFrameListL0, 
                                RefFrameList& rcRefFrameListL1 ) const
{

    const IntFrame*  pcFrameL0 = rcRefFrameListL0[sL0RefIdx];

    const IntFrame*  pcFrameL1 = rcRefFrameListL1[sL1RefIdx];
  Int           iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
  if( iDiffPocD == 0 )
  {
    return 1024;
  }
  else
  {
    Int iDiffPocB = getPoc( eMbPicType ) - pcFrameL0->getPOC();
    Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
    Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
    Int iX        = (0x4000 + (iTDD>>1)) / iTDD;
    Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
    return iScale;
  }
}
//TMM_EC }}

Int
SliceHeader::getDistScaleFactorScal( PicType eMbPicType,
                                    SChar sL0RefIdx,
                                    SChar sL1RefIdx ) const
{
    IntFrame* pcFrameL0     = getRefFrameList( eMbPicType, LIST_0 )->getEntry( sL0RefIdx-1 );
    IntFrame* pcFrameL1     = getRefFrameList( eMbPicType, LIST_1 )->getEntry( sL1RefIdx-1 );
    Int       iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
    if( iDiffPocD == 0 )
    {
        return 1024;
    }
    else
    {
        Int iDiffPocB = getPoc() - pcFrameL0->getPOC();
        Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
        Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
		Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
        Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
        return iScale;
    }
}


Int
SliceHeader::getDistScaleFactorWP( const Frame* pcFrameL0, const Frame* pcFrameL1 ) const
{
  Int iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
  if( iDiffPocD == 0 )
  {
    return 1024;
  }
  Int iDiffPocB = getPoc() - pcFrameL0->getPOC();
  Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
  Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
  Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
  Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
  return iScale;
}


Int
SliceHeader::getDistScaleFactorWP( const IntFrame* pcFrameL0, const IntFrame* pcFrameL1 ) const
{
  Int iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
  if( iDiffPocD == 0 )
  {
    return 1024;
  }
  Int iDiffPocB = getPoc() - pcFrameL0->getPOC();
  Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
  Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
  Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
  Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
  return iScale;
}


// JVT-Q054 Red. Picture {
ErrVal
SliceHeader::compareRedPic( const SliceHeader* pcSH,
                           Bool&              rbNewFrame ) const
{
  rbNewFrame = true;

  ROTRS( NULL == pcSH,                                          Err::m_nOK );
  ROTRS( getIdrPicId() != pcSH->getIdrPicId(),                  Err::m_nOK );
  ROTRS( getFrameNum() != pcSH->getFrameNum(),                  Err::m_nOK );
  ROTRS( getViewId() != pcSH->getViewId(),                      Err::m_nOK );
  ROTRS( getLayerId() != pcSH->getLayerId(),                    Err::m_nOK );
  ROTRS( getQualityLevel() != pcSH->getQualityLevel(),          Err::m_nOK );
  ROTRS( getFirstMbInSlice() != pcSH->getFirstMbInSlice(),      Err::m_nOK );
  ROTRS( (getNalRefIdc() == 0 )&&(pcSH->getNalRefIdc() != 0),   Err::m_nOK );
  ROTRS( (getNalRefIdc() != 0 )&&(pcSH->getNalRefIdc() == 0),   Err::m_nOK );
  ROTRS( getPicOrderCntLsb() != pcSH->getPicOrderCntLsb(),      Err::m_nOK );

  rbNewFrame = false;

  return Err::m_nOK;
}


ErrVal
SliceHeader::sliceHeaderBackup ( SliceHeader*                pcSH )
{
  pcSH->setIdrPicId       ( getIdrPicId()       );
  pcSH->setFrameNum       ( getFrameNum()       );
  pcSH->setLayerId        ( getLayerId()        );
  pcSH->setQualityLevel   ( getQualityLevel()   );
  pcSH->setFirstMbInSlice ( getFirstMbInSlice() );
  pcSH->setNalRefIdc      ( getNalRefIdc()      );
  pcSH->setPicOrderCntLsb ( getPicOrderCntLsb() );
  pcSH->setViewId         ( getViewId()        );

  return Err::m_nOK;
}



// JVT-Q054 Red. Picture }

H264AVC_NAMESPACE_END

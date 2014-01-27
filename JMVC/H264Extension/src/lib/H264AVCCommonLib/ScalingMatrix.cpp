#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/ScalingMatrix.h"



H264AVC_NAMESPACE_BEGIN



ScalingMatrix::ScalingMatrix()
{
}


const UChar*
ScalingMatrix::get( UInt uiMatrix ) const
{
  if( uiMatrix < 6)
  {
    return  m_acScalingMatrix4x4.get(uiMatrix  ).getMatrix();
  }
  return    m_acScalingMatrix8x8.get(uiMatrix-6).getMatrix();
}


ErrVal
ScalingMatrix::write( HeaderSymbolWriteIf*  pcWriteIf,
                      Bool                  bWrite8x8 ) const
{
  for( Int i4x4 = 0; i4x4 < 6; i4x4++ ) 
  {
    RNOK( m_acScalingMatrix4x4[i4x4].write( pcWriteIf, g_aucFrameScan   ) );
  }
  ROTRS( ! bWrite8x8, Err::m_nOK );

  for( Int i8x8 = 0; i8x8 < 2; i8x8++ ) 
  {
    RNOK( m_acScalingMatrix8x8[i8x8].write( pcWriteIf, g_aucFrameScan64 ) );
  }

  return Err::m_nOK;
}


ErrVal
ScalingMatrix::read( HeaderSymbolReadIf*  pcReadIf,
                     Bool                 bRead8x8 )
{
  for( Int i4x4 = 0; i4x4 < 6; i4x4++ ) 
  {
    RNOK( m_acScalingMatrix4x4.get(i4x4).read( pcReadIf, g_aucFrameScan   ) );
  }
  ROTRS( ! bRead8x8, Err::m_nOK );

  RNOK  ( m_acScalingMatrix8x8.get(0   ).read( pcReadIf, g_aucFrameScan64 ) );
  RNOK  ( m_acScalingMatrix8x8.get(1   ).read( pcReadIf, g_aucFrameScan64 ) );

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END




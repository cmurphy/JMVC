#if !defined(AFX_GLOBALFUNCTIONS_H__85D028A1_590E_423B_9072_AB351037B1E8__INCLUDED_)
#define AFX_GLOBALFUNCTIONS_H__85D028A1_590E_423B_9072_AB351037B1E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))


H264AVC_NAMESPACE_BEGIN

__inline
const Int gClip( const Int iX )
{
  const Int i2 = (iX & 0xFF);
  if( i2 == iX )  { return iX; }
  if( iX < 0 )    { return 0x00; }
  else            { return 0xFF; }
}


__inline
const Int gClipMinMax( const Int iX, const Int iMin, const Int iMax )
{
  return max( min( iX, iMax ), iMin );
}

//TMM_WP
__inline
Int gIntRandom(const Int iMin, const Int iMax)
{
    Double fRange = (Double)(iMax - iMin + 1);
    Int iValue = (Int)(fRange*rand()/(RAND_MAX+1.0));

    AOT_DBG( (iValue + iMin)> iMax );
    return iValue + iMin;
}
//TMM_WP

H264AVC_NAMESPACE_END


#endif // !defined(AFX_GLOBALFUNCTIONS_H__85D028A1_590E_423B_9072_AB351037B1E8__INCLUDED_)

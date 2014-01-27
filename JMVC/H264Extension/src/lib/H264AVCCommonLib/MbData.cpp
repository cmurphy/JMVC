#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/MbData.h"



H264AVC_NAMESPACE_BEGIN

Void MbData::copy( const MbData& rcMbData )
{
    MbDataStruct::          copy    ( rcMbData );
    m_pcMbTCoeffs         ->copyFrom( *rcMbData.m_pcMbTCoeffs );
    m_apcMbMotionData [0] ->copyFrom( *rcMbData.m_apcMbMotionData [0] );
    m_apcMbMvdData    [0] ->copyFrom( *rcMbData.m_apcMbMvdData    [0] );
    m_apcMbMotionData [1] ->copyFrom( *rcMbData.m_apcMbMotionData [1] );
    m_apcMbMvdData    [1] ->copyFrom( *rcMbData.m_apcMbMvdData    [1] );
}

ErrVal
MbData::saveAll( FILE* pFile )
{
  RNOK( MbDataStruct::          save( pFile ) );
  RNOK( m_pcMbTCoeffs         ->save( pFile ) );
  RNOK( m_apcMbMotionData [0] ->save( pFile ) );
  RNOK( m_apcMbMvdData    [0] ->save( pFile ) );
  RNOK( m_apcMbMotionData [1] ->save( pFile ) );
  RNOK( m_apcMbMvdData    [1] ->save( pFile ) );

  return Err::m_nOK;
}


ErrVal
MbData::loadAll( FILE* pFile )
{
  RNOK( MbDataStruct::          load( pFile ) );
  RNOK( m_pcMbTCoeffs         ->load( pFile ) );
  RNOK( m_apcMbMotionData [0] ->load( pFile ) );
  RNOK( m_apcMbMvdData    [0] ->load( pFile ) );
  RNOK( m_apcMbMotionData [1] ->load( pFile ) );
  RNOK( m_apcMbMvdData    [1] ->load( pFile ) );

  return Err::m_nOK;
}

ErrVal
MbData::copyMotion( MbData& rcMbData,
                    UInt    uiSliceId )
{
  m_uiSliceId   = ( uiSliceId != MSYS_UINT_MAX ? uiSliceId : m_uiSliceId );
  m_bBLSkipFlag = rcMbData.m_bBLSkipFlag;
  m_bSkipFlag   = rcMbData.m_bSkipFlag;
  m_eMbMode     = rcMbData.m_eMbMode;
  m_aBlkMode[0] = rcMbData.m_aBlkMode[0];
  m_aBlkMode[1] = rcMbData.m_aBlkMode[1];
  m_aBlkMode[2] = rcMbData.m_aBlkMode[2];
  m_aBlkMode[3] = rcMbData.m_aBlkMode[3]; 
  m_usFwdBwd    = rcMbData.m_usFwdBwd;
  m_bFieldFlag  = rcMbData.m_bFieldFlag; //th060201

  m_apcMbMotionData[0]->copyFrom( *rcMbData.m_apcMbMotionData[0] );
  m_apcMbMotionData[1]->copyFrom( *rcMbData.m_apcMbMotionData[1] );

  m_apcMbMvdData[0]->copyFrom( *rcMbData.m_apcMbMvdData[0] );
  m_apcMbMvdData[1]->copyFrom( *rcMbData.m_apcMbMvdData[1] );

  return Err::m_nOK;
}


ErrVal
MbData::copyMotionBL( MbData& rcMbData,
                      Bool    bDirect8x8,
                      UInt    uiSliceId )
{
  m_uiSliceId   = ( uiSliceId != MSYS_UINT_MAX ? uiSliceId : m_uiSliceId );
  m_bBLSkipFlag = false;
  m_eMbMode     = ( rcMbData.m_eMbMode     == MODE_SKIP ? MODE_8x8   : rcMbData.m_eMbMode     );
  m_aBlkMode[0] = ( rcMbData.m_aBlkMode[0] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[0] );
  m_aBlkMode[1] = ( rcMbData.m_aBlkMode[1] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[1] );
  m_aBlkMode[2] = ( rcMbData.m_aBlkMode[2] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[2] );
  m_aBlkMode[3] = ( rcMbData.m_aBlkMode[3] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[3] );

  if( rcMbData.m_eMbMode == MODE_SKIP || rcMbData.m_eMbMode == MODE_8x8)
  {
    UInt uiFwdBwd = 0;
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbData.m_apcMbMotionData[0]->getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbData.m_apcMbMotionData[1]->getRefIdx( Par8x8(n) )) ? 2:0;
    }
    m_usFwdBwd = uiFwdBwd;
  }
  else
  {
    m_usFwdBwd    = rcMbData.m_usFwdBwd;
  }

  m_apcMbMotionData[0]->copyFrom( *rcMbData.m_apcMbMotionData[0] );
  m_apcMbMotionData[1]->copyFrom( *rcMbData.m_apcMbMotionData[1] );

  m_apcMbMvdData[0]->copyFrom( *rcMbData.m_apcMbMvdData[0] );
  m_apcMbMvdData[1]->copyFrom( *rcMbData.m_apcMbMvdData[1] );

  return Err::m_nOK;
}

//JVT-T054{
ErrVal
MbData::copyMbCbp( MbData& rcMbData )
{
  m_uiMbCbp = rcMbData.m_uiMbCbp;
  return Err::m_nOK;
}
ErrVal
MbData::initMbCbp()
{
  m_uiMbCbp = 0;
  return Err::m_nOK;
}
//JVT-T054}


ErrVal
MbData::upsampleMotion( MbData& rcMbData, Par8x8 ePar8x8, Bool bDirect8x8 )
{
  RNOK( MbDataStruct::upsampleMotion( rcMbData, ePar8x8, bDirect8x8 ) );
  m_bBLSkipFlag = false;
  
  RNOK( m_apcMbMotionData[0]->upsampleMotion( *rcMbData.m_apcMbMotionData[0], ePar8x8 ) );
  RNOK( m_apcMbMotionData[1]->upsampleMotion( *rcMbData.m_apcMbMotionData[1], ePar8x8 ) );

  UInt uiFwdBwd = 0;
  uiFwdBwd += (0 < m_apcMbMotionData[0]->getRefIdx( B_8x8_0 )) ? 1:0;
  uiFwdBwd += (0 < m_apcMbMotionData[1]->getRefIdx( B_8x8_0 )) ? 2:0;
  m_usFwdBwd = (uiFwdBwd<<12)|(uiFwdBwd<<8)|(uiFwdBwd<<4)|(uiFwdBwd);

  return Err::m_nOK;
}


Void
MbData::switchMotionRefinement()
{
  ROFVS( m_bHasMotionRefinement );

  // switch mb_type
  MbMode  eMbModeTemp = m_eMbMode;
  m_eMbMode = m_eMbModeBase;
  m_eMbModeBase = eMbModeTemp;

  // switch sub-mb_type
  BlkMode aBlkModeTemp[4];  ::memcpy( aBlkModeTemp, m_aBlkMode, sizeof( m_aBlkMode ) );
  ::memcpy( m_aBlkMode, m_aBlkModeBase, sizeof( m_aBlkMode ) );
  ::memcpy( m_aBlkModeBase, aBlkModeTemp, sizeof( m_aBlkMode ) );

  // switch motion vectors
  for( UInt ui = 0; ui < 2; ui++ )
  {
    MbMotionData cMbMotionDataTemp;
    cMbMotionDataTemp.copyFrom( *m_apcMbMotionData[ui] );
    m_apcMbMotionData[ui]->copyFrom( *m_apcMbMotionDataBase[ui] );
    m_apcMbMotionDataBase[ui]->copyFrom( cMbMotionDataTemp );
  }
}

Void
MbData::activateMotionRefinement()
{
  AOT( m_bHasMotionRefinement );
  m_bHasMotionRefinement = true;
  m_eMbModeBase          = m_eMbMode;
  ::memcpy( m_aBlkModeBase, m_aBlkMode, sizeof( m_aBlkMode ) );
  m_apcMbMotionDataBase[0]->copyFrom( *m_apcMbMotionData[0] );
  m_apcMbMotionDataBase[1]->copyFrom( *m_apcMbMotionData[1] );
}

ErrVal
MbData::noUpsampleMotion()
{
    clear();
    m_apcMbMotionData[0]->clear(BLOCK_NOT_AVAILABLE) ;
    m_apcMbMotionData[1]->clear(BLOCK_NOT_AVAILABLE) ;
    return Err::m_nOK;      
}


const UChar MbData::m_aucPredictor[2][4]= {{1,0,3,2},{2,3,0,1}};
//Indexes: MBMode Mode8x8%8 b4x4idx
const Char MbData::aaacGetPartInfo[7][4][16]={
  //MODE_SKIP 
  {{0,0,4,4,0,0,4,4,8,8,12,12,8,8,12,12},//BLK_8x8
  {0,0,4,4,1,1,5,5,8,8,12,12,9,9,13,13},//BLK_8x4
  {0,1,4,5,0,1,4,5,8,9,12,13,8,9,12,13},//BLK_4x8
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},//BLK_4x4
  //MODE_16x16
  {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//BLK_8x8
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//BLK_8x4
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//BLK_4x8
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},//BLK_4x4
  //MODE_16x8
  {{0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4},//BLK_8x8
  {0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4},//BLK_8x4
  {0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4},//BLK_4x8
  {0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4}},//BLK_4x4
  //MODE_8x16
  {{0,0,4,4,0,0,4,4,0,0,4,4,0,0,4,4},//BLK_8x8
  {0,0,4,4,0,0,4,4,0,0,4,4,0,0,4,4}, //BLK_8x4
  {0,0,4,4,0,0,4,4,0,0,4,4,0,0,4,4}, //BLK_4x8
  {0,0,4,4,0,0,4,4,0,0,4,4,0,0,4,4}},//BLK_4x4
  //MODE_8x8
  {{0,0,4,4,0,0,4,4,8,8,12,12,8,8,12,12},//BLK_8x8
   {0,0,4,4,1,1,5,5,8,8,12,12,9,9,13,13},//BLK_8x4
   {0,1,4,5,0,1,4,5,8,9,12,13,8,9,12,13},//BLK_4x8
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},//BLK_4x4
  //MODE_8x8ref0
  {{0,0,4,4,0,0,4,4,8,8,12,12,8,8,12,12},//BLK_8x8
   {0,0,4,4,1,1,5,5,8,8,12,12,9,9,13,13},//BLK_8x4
   {0,1,4,5,0,1,4,5,8,9,12,13,8,9,12,13},//BLK_4x8
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},//BLK_4x4
  //INTRA_4X4_
  {{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}} 
};

ErrVal
MbData::upsampleMotionESS (MbData* pcBaseMbData,
                           const UInt uiBaseMbStride,
                           const Int aiPelOrig[2], 
                           const Bool bDirect8x8,
                           ResizeParameters* pcParameters)
{
    Bool        abBl8x8Intra  [4]= {false,false,false,false}; 
    MbMode      aeMbMode      [4]= {MODE_16x16,MODE_16x16,MODE_16x16,MODE_16x16};
    BlkMode     aeBlkMode     [4][4];  
    UInt        auiMbIdx	    [4][4]; 
    UInt        aui4x4Idx	    [4][4];
    Int         aaiPartInfo   [4][4];
    UInt        uiMbBaseOrigX = 0, uiMbBaseOrigY = 0;    
  
    // initializing
    //-------------
     xInitESS();
    // Fill Base idx
    //-----------------
    xFillBaseMbData(pcBaseMbData, 
                    uiBaseMbStride,
                    aiPelOrig, 
                    bDirect8x8, 
                    pcParameters, 
                    aeMbMode,
                    aeBlkMode,
                    uiMbBaseOrigX,
                    uiMbBaseOrigY);

  if( m_eMbMode!=INTRA_4X4)
  {
    // Build PartInfo and inherited base idx filling
    //----------------------------------------------
    xBuildPartInfo(aiPelOrig, 
                  pcParameters,
                  aeMbMode,
                  aeBlkMode,  
                  aui4x4Idx, 
                  auiMbIdx,
                  aaiPartInfo,
                  abBl8x8Intra,
                  uiMbBaseOrigX,
                  uiMbBaseOrigY);

    //--- Remove 4x4 INTRA blocks
    //--------------------------
    UInt uiB8x8=0,uiB4x4=0;
    for ( uiB8x8=0 ; uiB8x8<4 ; uiB8x8++)
      if(!abBl8x8Intra[uiB8x8]) 
      {
        for (uiB4x4=0 ; uiB4x4<4 ; uiB4x4++)
          if(aaiPartInfo[uiB8x8][uiB4x4]==-1) xRemoveIntra4x4(uiB4x4,aui4x4Idx[uiB8x8],auiMbIdx[uiB8x8],aaiPartInfo[uiB8x8]);
      }
      
    //--- Remove 8x8 INTRA blocks
    //----------------------------    
      for ( uiB8x8=0 ; uiB8x8<4 ; uiB8x8++) 
        if(abBl8x8Intra[uiB8x8]) xRemoveIntra8x8(uiB8x8,abBl8x8Intra,aui4x4Idx,auiMbIdx,aaiPartInfo);

    // 8x8 blocks partitioning
    //------------------------
    xInherit8x8MotionData (aui4x4Idx,auiMbIdx,aaiPartInfo);

    // macroblock mode choice
    //-----------------------
    xInheritMbMotionData (aaiPartInfo);
  }

  // Transfer in MB structure
  //-------------------------
  RNOK( xFillMbMvData(pcParameters ) );

  //--- Set fwd/bwd 
  UInt uiFwdBwd = 0;
  for( Int n = 3; n >= 0; n--)
  {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < m_apcMbMotionData[0]->getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < m_apcMbMotionData[1]->getRefIdx( Par8x8(n) )) ? 2:0;
  }
   m_usFwdBwd = uiFwdBwd;

   return Err::m_nOK;
}

ErrVal
MbData::xInitESS()
{
    //--- Initialisation
    ///////////////////////
    m_bBLSkipFlag = false;

    m_uiMbCbp = 0;
    m_eMbMode = MODE_16x16;
    m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] =BLK_8x8;

    Mv* pcMv0=m_acBl4x4Mv[0];
    Mv* pcMv1=m_acBl4x4Mv[1];
    SChar* pscRefIdx0=m_ascBl4x4RefIdx[0];
    SChar* pscRefIdx1=m_ascBl4x4RefIdx[1];
    for(UInt  uiBl4x4Idx=0 ; uiBl4x4Idx < 16 ;uiBl4x4Idx++,pcMv0++,pcMv1++,pscRefIdx0++,pscRefIdx1++)
    {
        pcMv0->setZero();
        pcMv1->setZero();
        *pscRefIdx0=*pscRefIdx1=BLOCK_NOT_PREDICTED;
    }

    return Err::m_nOK;   
}

ErrVal
MbData::xFillBaseMbData(  MbData* pcBaseMbData,
                          const UInt uiBaseMbStride,
                          const Int aiPelOrig[2],
                          const Bool bDirect8x8,
                          ResizeParameters* pcParameters,
                          MbMode      aeMbMode	  [4],
                          BlkMode     aeBlkMode	  [4][4],
                          UInt&       uiMbBaseOrigX,
                          UInt&       uiMbBaseOrigY)
{
    //--- Base MB association + Fill in Base MbMode + aeBlkMode
    ///////////////////////////////////////////////////////////
    const Int iBaseX0 = ((aiPelOrig[0]+1)*pcParameters->m_iInWidth + pcParameters->m_iOutWidth/2) / pcParameters->m_iOutWidth; 
    const Int iBaseY0 = ((aiPelOrig[1]+1)*pcParameters->m_iInHeight + pcParameters->m_iOutHeight/2) / pcParameters->m_iOutHeight; 
    const Int iBaseX1 = ((aiPelOrig[0]+13)*pcParameters->m_iInWidth  + pcParameters->m_iOutWidth /2) / pcParameters->m_iOutWidth; 
    const Int iBaseY1 = ((aiPelOrig[1]+13)*pcParameters->m_iInHeight + pcParameters->m_iOutHeight/2) / pcParameters->m_iOutHeight; 

    uiMbBaseOrigX=(iBaseX0>>4);
    uiMbBaseOrigY=(iBaseY0>>4);

    Int iMbBaseIdx =uiMbBaseOrigY*uiBaseMbStride + uiMbBaseOrigX;
    MbData* pcBaseMb0 = &(pcBaseMbData[iMbBaseIdx]);

    const Int iSMbWidth   = ( iBaseX1>>4 > iBaseX0>>4 ) ? 2 : 1;
    const Int iSMbHeight  = ( iBaseY1>>4 > iBaseY0>>4 ) ? 2 : 1;
    
    Bool bIsMbIntra=true;
    m_apcMbData[0]=m_apcMbData[1]=m_apcMbData[2]=m_apcMbData[3]=0;

    for( Int iBaseMbY = 0; iBaseMbY < iSMbHeight; iBaseMbY ++)
        for( Int iBaseMbX = 0; iBaseMbX < iSMbWidth; iBaseMbX ++)
        {  
            UInt uiIdxMb=(iBaseMbY<<1) + iBaseMbX;
            m_apcMbData[uiIdxMb]= pcBaseMb0 + iBaseMbY * uiBaseMbStride + iBaseMbX; 
          
            MbMode eMbMode= m_apcMbData[uiIdxMb]->m_eMbMode;  
             if(eMbMode>=INTRA_4X4 )  
             {
               eMbMode=INTRA_4X4;
               aeBlkMode[uiIdxMb][0]=aeBlkMode[uiIdxMb][1]=aeBlkMode[uiIdxMb][2]=aeBlkMode[uiIdxMb][3]=BLK_8x8; //bug fix jerome.vieron@thomson.net
             }
             else
             {
               if(eMbMode==MODE_SKIP) eMbMode = MODE_8x8;
               for(UInt uiB8x8Idx=0;uiB8x8Idx<4;uiB8x8Idx++)
               {
                 BlkMode eBlkMode=m_apcMbData[uiIdxMb]->m_aBlkMode[uiB8x8Idx];
                 if (eBlkMode == BLK_SKIP)
                   eBlkMode = (bDirect8x8) ? BLK_8x8 : BLK_4x4;

                 aeBlkMode[uiIdxMb][uiB8x8Idx]=eBlkMode;
               }
             }

             aeMbMode[uiIdxMb]= eMbMode;

             bIsMbIntra &= (eMbMode==INTRA_4X4);
        }

   
    if(bIsMbIntra) m_eMbMode=INTRA_4X4;

    return Err::m_nOK;   
}


ErrVal
MbData::xBuildPartInfo(const Int         aiPelOrig[2],
                       ResizeParameters* pcParameters,
                       const MbMode      aeMbMode[4],
                       const  BlkMode    aeBlkMode[4][4],  
                       UInt              aui4x4Idx[4][4],
                       UInt              auiMbIdx [4][4], 
                       Int              aaiPartInfo[4][4],
                       Bool              abBl8x8Intra[4],
                       const UInt        uiMbBaseOrigX,
                       const UInt        uiMbBaseOrigY )
{
   for( Int x=0;x<2;x++)
    {
        for( Int y=0;y<2;y++)
        {
            UInt uiB8x8Idx=(y<<1) + x;

            UInt uiBaseX0=((aiPelOrig[0]+x*8+1)*pcParameters->m_iInWidth  + pcParameters->m_iOutWidth/2)  / pcParameters->m_iOutWidth ;
            UInt uiBaseX1=((aiPelOrig[0]+x*8+5)*pcParameters->m_iInWidth  + pcParameters->m_iOutWidth/2)  / pcParameters->m_iOutWidth ;
            UInt uiBaseY0=((aiPelOrig[1]+y*8+1)*pcParameters->m_iInHeight + pcParameters->m_iOutHeight/2) / pcParameters->m_iOutHeight;
            UInt uiBaseY1=((aiPelOrig[1]+y*8+5)*pcParameters->m_iInHeight + pcParameters->m_iOutHeight/2) / pcParameters->m_iOutHeight;
            
            uiBaseX0 -= (uiMbBaseOrigX<<4);
            uiBaseX1 -= (uiMbBaseOrigX<<4);
            uiBaseY0 -= (uiMbBaseOrigY<<4);
            uiBaseY1 -= (uiMbBaseOrigY<<4);

            //Set Base Indexes
            ///////////////////
            aui4x4Idx[uiB8x8Idx][0]=(((uiBaseY0>>2)%4)<<2)+((uiBaseX0>>2)%4);
            aui4x4Idx[uiB8x8Idx][1]=(((uiBaseY0>>2)%4)<<2)+((uiBaseX1>>2)%4);
            aui4x4Idx[uiB8x8Idx][2]=(((uiBaseY1>>2)%4)<<2)+((uiBaseX0>>2)%4);
            aui4x4Idx[uiB8x8Idx][3]=(((uiBaseY1>>2)%4)<<2)+((uiBaseX1>>2)%4);

            auiMbIdx[uiB8x8Idx][0]=((uiBaseY0>>4)<<1)+(uiBaseX0>>4);
            auiMbIdx[uiB8x8Idx][1]=((uiBaseY0>>4)<<1)+(uiBaseX1>>4);
            auiMbIdx[uiB8x8Idx][2]=((uiBaseY1>>4)<<1)+(uiBaseX0>>4);
            auiMbIdx[uiB8x8Idx][3]=((uiBaseY1>>4)<<1)+(uiBaseX1>>4);
          
          //Set PartInfo
          /////////////// 
          aaiPartInfo[uiB8x8Idx][0]=aaacGetPartInfo[ aeMbMode[auiMbIdx[uiB8x8Idx][0]]] [(aeBlkMode[auiMbIdx[uiB8x8Idx][0]][g_aucConvertTo8x8Idx[aui4x4Idx[uiB8x8Idx][0]]])%8 ] [aui4x4Idx[uiB8x8Idx][0]];
          aaiPartInfo[uiB8x8Idx][1]=aaacGetPartInfo[ aeMbMode[auiMbIdx[uiB8x8Idx][1]]] [(aeBlkMode[auiMbIdx[uiB8x8Idx][1]][g_aucConvertTo8x8Idx[aui4x4Idx[uiB8x8Idx][1]]])%8 ] [aui4x4Idx[uiB8x8Idx][1]];
          aaiPartInfo[uiB8x8Idx][2]=aaacGetPartInfo[ aeMbMode[auiMbIdx[uiB8x8Idx][2]]] [(aeBlkMode[auiMbIdx[uiB8x8Idx][2]][g_aucConvertTo8x8Idx[aui4x4Idx[uiB8x8Idx][2]]])%8 ] [aui4x4Idx[uiB8x8Idx][2]];
          aaiPartInfo[uiB8x8Idx][3]=aaacGetPartInfo[ aeMbMode[auiMbIdx[uiB8x8Idx][3]]] [(aeBlkMode[auiMbIdx[uiB8x8Idx][3]][g_aucConvertTo8x8Idx[aui4x4Idx[uiB8x8Idx][3]]])%8 ] [aui4x4Idx[uiB8x8Idx][3]];
         
          aaiPartInfo[uiB8x8Idx][0]+=  (aaiPartInfo[uiB8x8Idx][0]==-1? 0:auiMbIdx[uiB8x8Idx][0]<<4);
          aaiPartInfo[uiB8x8Idx][1]+=  (aaiPartInfo[uiB8x8Idx][1]==-1? 0:auiMbIdx[uiB8x8Idx][1]<<4);
          aaiPartInfo[uiB8x8Idx][2]+=  (aaiPartInfo[uiB8x8Idx][2]==-1? 0:auiMbIdx[uiB8x8Idx][2]<<4);
          aaiPartInfo[uiB8x8Idx][3]+=  (aaiPartInfo[uiB8x8Idx][3]==-1? 0:auiMbIdx[uiB8x8Idx][3]<<4);
            
          abBl8x8Intra[uiB8x8Idx]=(aaiPartInfo[uiB8x8Idx][0]==-1)&&(aaiPartInfo[uiB8x8Idx][1]==-1)&&(aaiPartInfo[uiB8x8Idx][2]==-1)&&(aaiPartInfo[uiB8x8Idx][3]==-1);
        }
    }     

    return Err::m_nOK;
}

        
ErrVal 
MbData::xInherit8x8MotionData( const UInt        aui4x4Idx  [4][4],
                               const UInt        auiMbIdx	  [4][4], 
                               const Int         aaiPartInfo[4][4])
 {
   UInt uiB8x8Idx=0;
   for(uiB8x8Idx=0 ; uiB8x8Idx<4 ; uiB8x8Idx++)
   {
     //Compute  m_aBlkMode
     //-------------------
     Bool bR1 = aaiPartInfo[uiB8x8Idx][0]==aaiPartInfo[uiB8x8Idx][1];
     Bool bR2 = aaiPartInfo[uiB8x8Idx][2]==aaiPartInfo[uiB8x8Idx][3];
     Bool bC1 = aaiPartInfo[uiB8x8Idx][0]==aaiPartInfo[uiB8x8Idx][2];
     Bool bC2 = aaiPartInfo[uiB8x8Idx][1]==aaiPartInfo[uiB8x8Idx][3];

     //We assume that m_aBlkMode[uiB8x8Idx]has been initialized with BLK_8x8
     if( ! bC1 || ! bC2 )
     {
       m_aBlkMode[uiB8x8Idx]= BLK_8x4 ;
     }
     if( ! bR1 || ! bR2 )
     {
       m_aBlkMode[uiB8x8Idx]= ( m_aBlkMode[uiB8x8Idx] == BLK_8x8 ) ? BLK_4x8  : BLK_4x4;
     }
   
    // Inherit from base mv and RefIdx
    //---------------------------------
     const UInt*  pucMbIdx =auiMbIdx[uiB8x8Idx];
     const UInt*  puc4x4Idx=aui4x4Idx[uiB8x8Idx];
     const UChar* pucBlockOrder=&(g_aucConvertBlockOrder[uiB8x8Idx<<2]);

     for(UInt uiBl4x4Idx=0;uiBl4x4Idx<4;uiBl4x4Idx++,puc4x4Idx++,pucMbIdx++,pucBlockOrder++)
     {
       MbMotionData* pcmbMd0=m_apcMbData[*pucMbIdx]->m_apcMbMotionData[0];
       MbMotionData* pcmbMd1=m_apcMbData[*pucMbIdx]->m_apcMbMotionData[1];

       const UChar ucMapIdx=*puc4x4Idx;
       const UChar uc8x8Idx=g_aucConvertTo8x8Idx[ucMapIdx];
       m_acBl4x4Mv[0][*pucBlockOrder]     =pcmbMd0->getMv(B4x4Idx(ucMapIdx));
       m_ascBl4x4RefIdx[0][*pucBlockOrder]=pcmbMd0->getRefIdx(Par8x8(uc8x8Idx));	
       m_acBl4x4Mv[1][*pucBlockOrder]     =pcmbMd1->getMv(B4x4Idx(ucMapIdx));
       m_ascBl4x4RefIdx[1][*pucBlockOrder]=pcmbMd1->getRefIdx(Par8x8(uc8x8Idx));

       //fill cbp
       ///////////
       if((m_apcMbData[*pucMbIdx]->getMbExtCbp()>>ucMapIdx)&1)
         m_uiMbCbp|=(1<<(*pucBlockOrder));
     }
   }

   //Homogenization step
   ////////////////////////////////
   //--- Merge Ref Indexes and Mvs inside a given 8x8 block  
   for ( uiB8x8Idx=0 ; uiB8x8Idx<4 ; uiB8x8Idx++) 
     if(m_aBlkMode[uiB8x8Idx]!= BLK_8x8) xMergeBl8x8MvAndRef(uiB8x8Idx);

   return Err::m_nOK;
 }

ErrVal 
MbData::xInheritMbMotionData(const Int aaiPartInfo[4][4])
 {
     Bool bR1 = (m_aBlkMode[0]==m_aBlkMode[1]) && (m_aBlkMode[0]==BLK_8x8) && (aaiPartInfo[0][0]==aaiPartInfo[1][0]);
     Bool bR2 = (m_aBlkMode[2]==m_aBlkMode[3]) && (m_aBlkMode[2]==BLK_8x8) && (aaiPartInfo[2][0]==aaiPartInfo[3][0]);
     Bool bC1 = (m_aBlkMode[0]==m_aBlkMode[2]) && (m_aBlkMode[0]==BLK_8x8) && (aaiPartInfo[0][0]==aaiPartInfo[2][0]);
     Bool bC2 = (m_aBlkMode[1]==m_aBlkMode[3]) && (m_aBlkMode[1]==BLK_8x8) && (aaiPartInfo[1][0]==aaiPartInfo[3][0]);

      //We assume that m_eMbMode has been initialized with MODE_16x16
     if( ! bC1 || ! bC2 )
     {
       m_eMbMode = MODE_16x8 ;
     }
     if( ! bR1 || ! bR2 )
     {
        m_eMbMode = ( m_eMbMode == MODE_16x16 ) ? MODE_8x16  : MODE_8x8;
     }

     return Err::m_nOK;
 }

ErrVal
MbData::xMergeBl8x8MvAndRef(const UInt uiBlIdx)
{
  const UChar* pucWhich=&(g_aucConvertBlockOrder[uiBlIdx*4]);
  SChar ascChosenRefIdx[2]={MSYS_INT8_MAX,MSYS_INT8_MAX};
	UInt uiIdxstride=((m_aBlkMode[uiBlIdx]==BLK_8x4)?2:1);
	const UInt uiNbBl4x4=((m_aBlkMode[uiBlIdx]==BLK_4x4)?4:2);
	const UChar* pucMap=0;
	UInt uiList,Bl4x4Idx;

     for(uiList=0 ; uiList < 2 ; uiList++)
    {
        pucMap=pucWhich;

        for(Bl4x4Idx=0;Bl4x4Idx<uiNbBl4x4;Bl4x4Idx++,pucMap+=uiIdxstride)
        { 
            SChar scRefIdx=m_ascBl4x4RefIdx[uiList][*pucMap];
            if( scRefIdx>0) 
            {
                if(ascChosenRefIdx[uiList]>scRefIdx) ascChosenRefIdx[uiList]=scRefIdx;
            }
        }	
    }

    ascChosenRefIdx[0]=((ascChosenRefIdx[0]==MSYS_INT8_MAX)? -1:ascChosenRefIdx[0]);
    ascChosenRefIdx[1]=((ascChosenRefIdx[1]==MSYS_INT8_MAX)? -1:ascChosenRefIdx[1]);

    for(uiList=0 ; uiList < 2 ; uiList++)
    {
        if(ascChosenRefIdx[uiList]>0)
        {
            pucMap=pucWhich;
            for(Bl4x4Idx=0;Bl4x4Idx<4;Bl4x4Idx++,pucMap++)
            {
                if(m_ascBl4x4RefIdx[uiList][*pucMap]!=ascChosenRefIdx[uiList])	
                    xFillMvandRefBl4x4(Bl4x4Idx,pucWhich,uiList,ascChosenRefIdx);
            }
        }
        else
        {
            pucMap=pucWhich;
            for(Bl4x4Idx=0;Bl4x4Idx<4;Bl4x4Idx++,pucMap++)
            {
                m_ascBl4x4RefIdx[uiList][*pucMap]=-1;
                m_acBl4x4Mv[uiList][*pucMap].setZero();
            }
        }
    }

    return Err::m_nOK;
}

ErrVal
MbData::xFillMvandRefBl4x4(const UInt uiBlIdx,const UChar* pucWhich,const UInt uiList,const SChar* psChosenRefIdx)
{	
    UChar ucRealBlIdx=pucWhich[uiBlIdx];
    UChar ucPredIdx=m_aucPredictor[0][uiBlIdx];
    UChar ucRealPredIdx=pucWhich[ucPredIdx];

    if(m_ascBl4x4RefIdx[uiList][ucRealPredIdx]!=psChosenRefIdx[uiList])
    {
        ucPredIdx=m_aucPredictor[1][uiBlIdx];
        ucRealPredIdx=pucWhich[ucPredIdx];

        if(m_ascBl4x4RefIdx[uiList][ucRealPredIdx]!=psChosenRefIdx[uiList])		
            xFillMvandRefBl4x4(ucPredIdx,pucWhich,uiList,psChosenRefIdx);
    }	

    m_ascBl4x4RefIdx[uiList][ucRealBlIdx]=m_ascBl4x4RefIdx[uiList][ucRealPredIdx];
    m_acBl4x4Mv[uiList][ucRealBlIdx]=m_acBl4x4Mv[uiList][ucRealPredIdx];

    return Err::m_nOK;
}

ErrVal
MbData::xRemoveIntra8x8(const UInt uiBlIdx, 
                        const Bool* abBl8x8Intra,
                        UInt  aui4x4Idx[4][4],
                        UInt  auiMbIdx[4][4],
                        Int  aaiPartInfo[4][4])
                        
{	
    UChar ucPredIdx=m_aucPredictor[0][uiBlIdx] ;
    if(abBl8x8Intra[ucPredIdx])
    {
        ucPredIdx=m_aucPredictor[1][uiBlIdx] ;
        if(abBl8x8Intra[ucPredIdx])
            xRemoveIntra8x8(ucPredIdx,abBl8x8Intra,aui4x4Idx,auiMbIdx,aaiPartInfo);
    }
    xCopyBl8x8(ucPredIdx,uiBlIdx,aui4x4Idx,auiMbIdx,aaiPartInfo);

    return Err::m_nOK;
}

ErrVal
MbData::xCopyBl8x8(const UInt uiBlIdx,
                   const UInt uiBlIdxCopy,
                   UInt  aui4x4Idx[4][4],
                   UInt  auiMbIdx[4][4],
                   Int  aaiPartInfo[4][4])
{
    for( UInt uiB4x4Idx=0;uiB4x4Idx<4;uiB4x4Idx++)
    {
      aui4x4Idx[uiBlIdxCopy][uiB4x4Idx]=aui4x4Idx[uiBlIdx][uiB4x4Idx];
      auiMbIdx [uiBlIdxCopy][uiB4x4Idx]=auiMbIdx[uiBlIdx][uiB4x4Idx];
      aaiPartInfo[uiBlIdxCopy][uiB4x4Idx]=aaiPartInfo[uiBlIdx][uiB4x4Idx];
    }
    return Err::m_nOK;
}

 
         

ErrVal
MbData::xRemoveIntra4x4(const UInt uiBlIdx,
                        UInt  aui4x4Idx[4],
                        UInt  auiMbIdx[4],
                        Int  aaiPartInfo[4])
{	  
    UChar ucPredIdx=m_aucPredictor[0][uiBlIdx] ;
    
    if(aaiPartInfo[ucPredIdx]==-1)
    {
        ucPredIdx=m_aucPredictor[1][uiBlIdx] ;
        if(aaiPartInfo[ucPredIdx]==-1)
            xRemoveIntra4x4(ucPredIdx,aui4x4Idx,auiMbIdx,aaiPartInfo);
    }
    aui4x4Idx[uiBlIdx]  = aui4x4Idx[ucPredIdx];
    auiMbIdx[uiBlIdx]   = auiMbIdx [ucPredIdx];
    aaiPartInfo[uiBlIdx]= aaiPartInfo[ucPredIdx];

    return Err::m_nOK;
}




ErrVal
MbData::xFillMbMvData(ResizeParameters* pcParameters )
{
    //--- IF ESS PICTURE LEVEL
    if( pcParameters->m_iExtendedSpatialScalability == ESS_PICT )
    {
        Mv      deltaMv[4];
        Int     refFrNum, refPosX, refPosY;
        Int     curFrNum = pcParameters->getPOC();
        Int     curPosX = pcParameters->getCurrentPictureParameters(curFrNum)->m_iPosX;
        Int     curPosY = pcParameters->getCurrentPictureParameters(curFrNum)->m_iPosY;

        for (UInt uilist=0; uilist<2; uilist++)
        {	for (UInt uiB8x8Idx=0; uiB8x8Idx<4 ; uiB8x8Idx++)
        {
            int refidx=	 m_ascBl4x4RefIdx[uilist][g_aucConvertTo4x4Idx[uiB8x8Idx]];
            if(refidx <=0)  deltaMv[uiB8x8Idx].set(0,0) ;
            else
            {
                refFrNum = pcParameters->m_aiRefListPoc[uilist][refidx-1];
                refPosX = pcParameters->getCurrentPictureParameters(refFrNum)->m_iPosX;
                refPosY = pcParameters->getCurrentPictureParameters(refFrNum)->m_iPosY;
                deltaMv[uiB8x8Idx].set( 4*(refPosX-curPosX) , 4*(refPosY-curPosY) );
            }
        }
        m_apcMbMotionData[uilist]->upsampleMotionNonDyad( m_ascBl4x4RefIdx[uilist] , m_acBl4x4Mv[uilist] , pcParameters , deltaMv );
        }
    }
    else
    {
        m_apcMbMotionData[0]->upsampleMotionNonDyad( m_ascBl4x4RefIdx[0] , m_acBl4x4Mv[0] , pcParameters );
        m_apcMbMotionData[1]->upsampleMotionNonDyad( m_ascBl4x4RefIdx[1] , m_acBl4x4Mv[1] , pcParameters );
    }

    return Err::m_nOK;   
}

H264AVC_NAMESPACE_END

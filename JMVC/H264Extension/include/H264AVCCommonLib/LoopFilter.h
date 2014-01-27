#if !defined(AFX_LOOPFILTER_H__1860BB4C_C677_487A_A81F_0BD39DA40284__INCLUDED_)
#define AFX_LOOPFILTER_H__1860BB4C_C677_487A_A81F_0BD39DA40284__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN

class ControlMngIf;
class IntYuvPicBuffer;
class IntFrame;

class ReconstructionBypass;

class H264AVCCOMMONLIB_API LoopFilter
{
    enum Dir
    {
        VER = 0,
        HOR
    };

public:
    enum LFMode
    {
        LFM_DEFAULT_FILTER    = 0,
        LFM_NO_INTER_FILTER   = 1,
        LFM_NO_INTRAINTER     = 2,
        LFM_EXTEND_INTRA_SUR  = 4,
        LFM_NO_FILTER         = 8
    };

    typedef struct
    {
        UChar ucAlpha;
        UChar aucClip[5];
    }AlphaClip;

    typedef struct
    {
        Pel* pFlt;
        Int iOffset;
        Int iIndexA;
        Int iIndexB;
        UChar ucBs;
        Bool bLum;
    }FilterParameter;

    static const UChar g_aucBetaTab[52]; // leszek
    static const AlphaClip g_acAlphaClip[52]; // leszek

protected:
    LoopFilter();
    virtual ~LoopFilter();

public:
    static ErrVal create( LoopFilter*& rpcLoopFilter );
    ErrVal destroy();

    ErrVal process        ( SliceHeader& rcSH, IntFrame* pcIntFrame = NULL );
    ErrVal process        ( SliceHeader&        rcSH,
        IntFrame*           pcFrame,
        MbDataCtrl*         pcMbDataCtrlMot,
        MbDataCtrl*         pcMbDataCtrlRes,
        UInt                uiMbInRow,
        RefFrameList*       pcRefFrameList0,
        RefFrameList*       pcRefFrameList1,
        bool                spatial_scalable_flg);  // SSUN@SHARP

    ErrVal init( ControlMngIf*          pcControlMngIf,
        ReconstructionBypass*  pcReconstructionBypass );
    ErrVal uninit();

    Void setFilterMode( LFMode eLFMode = LFM_DEFAULT_FILTER ) { m_eLFMode = eLFMode; }

    // Hanke@RWTH
    Void setHighpassFramePointer( IntFrame* pcHighpassFrame = NULL ) { m_pcHighpassFrame = pcHighpassFrame; }

private:
    ErrVal xSetLF_INTERLACEFieldLists( RefFrameList& rcTopFieldList, RefFrameList& rcBotFieldList, RefFrameList* pcRefFrameList );//th

    UChar xCheckMvDataB( const MbData& rcQMbData, const LumaIdx cQIdx, const MbData& rcPMbData, const LumaIdx cPIdx, const Short sHorMvThr, const Short sVerMvThr );
    UChar xCheckMvDataP( const MbData& rcQMbData, const LumaIdx cQIdx, const MbData& rcPMbData, const LumaIdx cPIdx, const Short sHorMvThr, const Short sVerMvThr );

    __inline ErrVal xFilterMb( const MbDataAccess& rcMbDataAccess );
    __inline ErrVal xFilterMbFast( const MbDataAccess& rcMbDataAccess );
    ErrVal xGetFilterStrengthFast( const MbDataAccess& rcMbDataAccess, const Int iFilterIdc );
    __inline UInt xGetHorFilterStrengthFast( const MbData& rcMbDataCurr,
        const MbData& rcMbDataAbove,
        LumaIdx       cIdx,
        Bool          bAboveIntra,
        Bool          bCheckMv,
        Bool          bCoded,
        const Short   sVerMvThr,
        Int           iIntraStrength);
    __inline UInt xGetVerFilterStrengthFast( const MbData& rcMbDataCurr,
        const MbData& rcMbDataLeft,
        LumaIdx       cIdx,
        Bool          bLeftIntra,
        Bool          bCheckMv,
        Bool          bCoded,
        const Short   sVerMvThr);

    __inline Void xFilter( Pel* pFlt, const Int& iOffset, const Int& iIndexA, const Int& iIndexB, const UChar& ucBs, const Bool& bLum );

    __inline UInt xGetHorFilterStrength( const MbDataAccess& rcMbDataAccess, LumaIdx cIdx, Int iFilterIdc);
    __inline UInt xGetVerFilterStrength( const MbDataAccess& rcMbDataAccess, LumaIdx cIdx, Int iFilterIdc);
    __inline ErrVal xLumaHorFiltering  ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);
    __inline ErrVal xLumaVerFiltering  ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);
    __inline ErrVal xChromaHorFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);
    __inline ErrVal xChromaVerFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer);


    __inline Void   xFilter                       ( XPel*               pFlt,
        const Int&          iOffset,
        const Int&          iIndexA,
        const Int&          iIndexB,
        const UChar&        ucBs,
        const Bool&         bLum );
    __inline ErrVal xFilterMb                     ( MbDataAccess* rcMbDataAccessMot,
        MbDataAccess* rcMbDataAccessRes,
        IntYuvPicBuffer*    pcYuvBuffer,
        RefFrameList*       pcRefFrameList0,
        RefFrameList*       pcRefFrameList1,
        bool                spatial_scalable_flg);  // SSUN@SHARP

    __inline ErrVal xLumaHorFiltering             ( const MbDataAccess& rcMbDataAccessRes,
        const DFP&          rcDFP,
        IntYuvPicBuffer*    pcYuvBuffer );
    __inline ErrVal xLumaVerFiltering             ( const MbDataAccess& rcMbDataAccessRes,
        const DFP&          rcDFP,
        IntYuvPicBuffer*    pcYuvBuffer );
    __inline ErrVal xChromaHorFiltering           ( const MbDataAccess& rcMbDataAccessRes,
        const DFP&          rcDFP,
        IntYuvPicBuffer*    pcYuvBuffer );
    __inline ErrVal xChromaVerFiltering           ( const MbDataAccess& rcMbDataAccessRes,
        const DFP&          rcDFP,
        IntYuvPicBuffer*    pcYuvBuffer );

    UChar           xCheckMvDataP_RefIdx          ( const MbData&       rcQMbData,
        const LumaIdx       cQIdx,
        const MbData&       rcPMbData,
        const LumaIdx       cPIdx,
        const Short         sHorMvThr,
        const Short         sVerMvThr,
        RefFrameList&       rcRefFrameList0,
        RefFrameList&       rcRefFrameList1 );
    UChar           xCheckMvDataB_RefIdx          ( const MbData&       rcQMbData,
        const LumaIdx       cQIdx,
        const MbData&       rcPMbData,
        const LumaIdx       cPIdx,
        const Short         sHorMvThr,
        const Short         sVerMvThr,
        RefFrameList&       rcRefFrameList0,
        RefFrameList&       rcRefFrameList1 );
    __inline UInt   xGetHorFilterStrength_RefIdx  ( const MbDataAccess* pcMbDataAccessMot,
        const MbDataAccess* pcMbDataAccessRes,
        LumaIdx             cIdx,
        Int                 iFilterIdc,
        RefFrameList*       pcRefFrameList0,
        RefFrameList*       pcRefFrameList1,
        bool                spatial_scalable_flg);  // SSUN@SHARP
    __inline UInt   xGetVerFilterStrength_RefIdx  ( const MbDataAccess* pcMbDataAccessMot,
        const MbDataAccess* pcMbDataAccessRes,
        LumaIdx             cIdx,
        Int                 iFilterIdc,
        RefFrameList*       pcRefFrameList0,
        RefFrameList*       pcRefFrameList1,
        bool                spatial_scalable_flg);  // SSUN@SHARP

protected:
    // Hanke@RWTH
    IntFrame*        m_pcHighpassFrame;
    IntYuvPicBuffer* m_pcHighpassYuvBuffer;

    UChar m_aucBs[4];

    ControlMngIf*    m_pcControlMngIf;
    FrameUnit*       m_pcRecFrameUnit;
    UChar            m_aaaucBs[2][4][4];
    IntYuvPicBuffer* m_apcIntYuvBuffer[4];
    LFMode           m_eLFMode;
    ReconstructionBypass*         m_pcReconstructionBypass;
    UChar           m_aucBsHorTop[4];
    UChar           m_aucBsVerBot[4];
    Bool            m_bVerMixedMode;
    Bool            m_bHorMixedMode;
    Bool            m_bAddEdge;

protected:

    template <UInt uiLum> Void xFilterTempl( FilterParameter& rcFilterParameter )
    {
        const Int iAlpha = g_acAlphaClip[ rcFilterParameter.iIndexA ].ucAlpha;

        Int P0 = rcFilterParameter.pFlt[-rcFilterParameter.iOffset];
        Int Q0 = rcFilterParameter.pFlt[       0];

        Int iDelta = Q0 - P0;
        Int iAbsDelta  = abs( iDelta  );

        AOF_DBG( rcFilterParameter.ucBs );

        ROFVS( iAbsDelta < iAlpha );


        const Int iBeta = g_aucBetaTab [ rcFilterParameter.iIndexB ];

        Int P1  = rcFilterParameter.pFlt[-2*rcFilterParameter.iOffset];
        Int Q1  = rcFilterParameter.pFlt[   rcFilterParameter.iOffset];

        ROFVS( (abs(P0 - P1) < iBeta) && (abs(Q0 - Q1) < iBeta) );

        if( rcFilterParameter.ucBs < 4 )
        {
            Int C0 = g_acAlphaClip[ rcFilterParameter.iIndexA ].aucClip[rcFilterParameter.ucBs];

            if( uiLum )
            {
                Int P2 = rcFilterParameter.pFlt[-3*rcFilterParameter.iOffset] ;
                Int Q2 = rcFilterParameter.pFlt[ 2*rcFilterParameter.iOffset] ;
                Int aq = (( abs( Q2 - Q0 ) < iBeta ) ? 1 : 0 );
                Int ap = (( abs( P2 - P0 ) < iBeta ) ? 1 : 0 );

                if( ap )
                {
                    rcFilterParameter.pFlt[-2*rcFilterParameter.iOffset] = P1 + gClipMinMax((P2 + ((P0 + Q0 + 1)>>1) - (P1<<1)) >> 1, -C0, C0 );
                }

                if( aq  )
                {
                    rcFilterParameter.pFlt[   rcFilterParameter.iOffset] = Q1 + gClipMinMax((Q2 + ((P0 + Q0 + 1)>>1) - (Q1<<1)) >> 1, -C0, C0 );
                }

                C0 += ap + aq -1;
            }

            C0++;
            Int iDiff      = gClipMinMax(((iDelta << 2) + (P1 - Q1) + 4) >> 3, -C0, C0 ) ;
            rcFilterParameter.pFlt[-rcFilterParameter.iOffset] = gClip( P0 + iDiff );
            rcFilterParameter.pFlt[       0] = gClip( Q0 - iDiff );
            return;
        }

        if( ! uiLum )
        {
            rcFilterParameter.pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
            rcFilterParameter.pFlt[  -rcFilterParameter.iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
        }
        else
        {
            Int P2 = rcFilterParameter.pFlt[-3*rcFilterParameter.iOffset] ;
            Int Q2 = rcFilterParameter.pFlt[ 2*rcFilterParameter.iOffset] ;
            Bool bEnable  = (iAbsDelta < ((iAlpha >> 2) + 2));
            Bool aq       = bEnable & ( abs( Q2 - Q0 ) < iBeta );
            Bool ap       = bEnable & ( abs( P2 - P0 ) < iBeta );
            Int PQ0 = P0 + Q0;

            if( aq )
            {
                rcFilterParameter.pFlt[         0] = (P1 + ((Q1 + PQ0) << 1) +  Q2 + 4) >> 3;
                rcFilterParameter.pFlt[   rcFilterParameter.iOffset] = (PQ0 +Q1 + Q2 + 2) >> 2;
                rcFilterParameter.pFlt[ 2*rcFilterParameter.iOffset] = (((rcFilterParameter.pFlt[ 3*rcFilterParameter.iOffset] + Q2) <<1) + Q2 + Q1 + PQ0 + 4) >> 3;
            }
            else
            {
                rcFilterParameter.pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
            }

            if( ap )
            {
                rcFilterParameter.pFlt[  -rcFilterParameter.iOffset] = (Q1 + ((P1 + PQ0) << 1) +  P2 + 4) >> 3;
                rcFilterParameter.pFlt[-2*rcFilterParameter.iOffset] = (PQ0 +P1 + P2 + 2) >> 2;
                rcFilterParameter.pFlt[-3*rcFilterParameter.iOffset] = (((rcFilterParameter.pFlt[-4*rcFilterParameter.iOffset] + P2) <<1) + P2 + P1 + PQ0 + 4) >> 3;
            }
            else
            {
                rcFilterParameter.pFlt[  -rcFilterParameter.iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
            }
        }
    }
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_LOOPFILTER_H__1860BB4C_C677_487A_A81F_0BD39DA40284__INCLUDED_)

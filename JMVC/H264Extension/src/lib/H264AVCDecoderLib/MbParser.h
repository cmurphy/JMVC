#if !defined(AFX_MBPARSER_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_)
#define AFX_MBPARSER_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN

class Transform;
class MbSymbolReadIf;

class MbParser
{
protected:
	MbParser();
	virtual ~MbParser();

public:
  static ErrVal create    ( MbParser*&      rpcMbParser );
  ErrVal        destroy   ();

  ErrVal initSlice        ( MbSymbolReadIf* pcMbSymbolReadIf );
  ErrVal init             ( Transform*      pcTransform );
  ErrVal uninit           ();
  ErrVal process          ( MbDataAccess&   rcMbDataAccess, Bool& rbEndOfSlice);

  ErrVal read             ( MbDataAccess&   rcMbDataAccess,
                            MbDataAccess*   pcMbDataAccessBase,
                            Int             iSpatialScalabilityType,
                            Bool&           rbEndOfSlice);
  ErrVal readMotion       ( MbDataAccess&  rcMbDataAccess,
                            MbDataAccess*  pcMbDataAccessBase );
//	TMM_EC {{
  ErrVal readVirtual      ( MbDataAccess&   rcMbDataAccess,
                            MbDataAccess*   pcMbDataAccessBase,
                            Int             iSpatialScalabilityType,
                            Bool&           rbEndOfSlice,
														ERROR_CONCEAL   e_ErrorConceal);
//  TMM_EC }}

protected:
  ErrVal xSkipMb                      ( MbDataAccess& rcMbDataAccess );
  ErrVal xReadMbType                  ( MbDataAccess& rcMbDataAccess );
  ErrVal xReadIntraPredModes          ( MbDataAccess& rcMbDataAccess );

	//-- JVT-R091
	ErrVal xReadTextureInfo             ( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase, Bool bTrafo8x8Flag );
	//--

  ErrVal xScanChromaBlocks            ( MbDataAccess& rcMbDataAccess, UInt uiChromCbp );

  ErrVal xReadMotionVectors           ( MbDataAccess& rcMbDataAccess, MbMode eMbMode, ListIdx eLstIdx );
  ErrVal xReadReferenceFrames         ( MbDataAccess& rcMbDataAccess, MbMode eMbMode, ListIdx eLstIdx );

  ErrVal xReadReferenceFramesNoRefPic ( MbDataAccess& rcMbDataAccess, MbMode eMbMode, ListIdx eLstIdx );


  ErrVal xReadMotionPredFlags_FGS     ( MbDataAccess& rcMbDataAccess,
                                        MbDataAccess* pcMbDataAccessBaseMotion,
                                        MbMode        eMbMode,
                                        ListIdx       eLstIdx );
  ErrVal xReadMotionPredFlags         ( MbDataAccess& rcMbDataAccess,
                                        MbMode        eMbMode,
                                        ListIdx       eLstIdx );

  ErrVal xGet8x8BlockMv               ( MbDataAccess& rcMbDataAccess, B8x8Idx c8x8Idx, ListIdx eLstIdx );

protected:
  Transform*      m_pcTransform;
  MbSymbolReadIf* m_pcMbSymbolReadIf;
  Bool            m_bInitDone;
  Bool            m_bPrevIsSkipped;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBPARSER_H__C2625057_4318_4267_8A7A_8185BB75AA7F__INCLUDED_)

#if !defined(AFX_RECONSTRUCTIONBYPASS_H__A1FD51F1_158E_45AF_93DE_35EF82E2A708__INCLUDED_)
#define AFX_RECONSTRUCTIONBYPASS_H__A1FD51F1_158E_45AF_93DE_35EF82E2A708__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN

class IntFrame;
class IntYuvMbBufferExtension;
class MbDataCtrl;
class YuvBufferCtrl;

class ReconstructionBypass  
{
public:
  ReconstructionBypass();
  virtual ~ReconstructionBypass() {}

  static ErrVal create( ReconstructionBypass*& rpcReconstructionBypass );

  ErrVal destroy();
  ErrVal init();
  ErrVal uninit();

  ErrVal padRecFrame( IntFrame* pcIntFrame, const MbDataCtrl* pcMbDataCtrl, YuvBufferCtrl* pcYuvFullPelBufferCtrl, UInt uiFrameWidthInMb, UInt uiFrameHeightInMb );
  ErrVal padRecMb( IntYuvMbBufferExtension* pcBuffer, UInt uiMask );

};

H264AVC_NAMESPACE_END


#endif // !defined(AFX_RECONSTRUCTIONBYPASS_H__A1FD51F1_158E_45AF_93DE_35EF82E2A708__INCLUDED_)

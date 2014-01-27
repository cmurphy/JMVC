#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"


#include "H264AVCCommonLib/IntFrame.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"

H264AVC_NAMESPACE_BEGIN

ReconstructionBypass::ReconstructionBypass() 
{
}

ErrVal ReconstructionBypass::create( ReconstructionBypass*& rpcReconstructionBypass )
{
  rpcReconstructionBypass = new ReconstructionBypass;
  ROT( NULL == rpcReconstructionBypass ) ;
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::init()
{
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::uninit()
{
  return Err::m_nOK;
}
 
ErrVal ReconstructionBypass::padRecFrame( IntFrame* pcIntFrame, const MbDataCtrl* pcMbDataCtrl, YuvBufferCtrl* pcYuvFullPelBufferCtrl, UInt uiFrameWidthInMb, UInt uiFrameHeightInMb )
{
  IntYuvPicBuffer* pcIntYuvPicBuffer = pcIntFrame->getFullPelYuvBuffer();

  // loop over macroblocks
  for( UInt uiMbY = 0; uiMbY < uiFrameWidthInMb; uiMbY++ )
  {
    for( UInt uiMbX = 0; uiMbX < uiFrameHeightInMb; uiMbX++ )
    {
      UInt uiMask = 0;

      //===== init macroblock =====
      RNOK( pcMbDataCtrl   ->getBoundaryMask( uiMbY, uiMbX, uiMask ) );
      RNOK( pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX ) );

      if( uiMask )
      {
		    IntYuvMbBufferExtension cBuffer;
		    cBuffer.loadSurrounding( pcIntYuvPicBuffer );

        RNOK( padRecMb( &cBuffer, uiMask ) );

  	    pcIntYuvPicBuffer->loadBuffer( &cBuffer );
      }
    }
  }

  return Err::m_nOK;
}

ErrVal ReconstructionBypass::padRecMb( IntYuvMbBufferExtension* pcBuffer, UInt uiMask )
{

  Bool bAboveIntra      = 0 != (uiMask & 0x01);
  Bool bBelowIntra      = 0 != (uiMask & 0x10);
  Bool bLeftIntra       = 0 != (uiMask & 0x40);
  Bool bRightIntra      = 0 != (uiMask & 0x04);
  Bool bLeftAboveIntra  = 0 != (uiMask & 0x80);
  Bool bRightAboveIntra = 0 != (uiMask & 0x02);
  Bool bLeftBelowIntra  = 0 != (uiMask & 0x20);
  Bool bRightBelowIntra = 0 != (uiMask & 0x08);

  for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    switch( cIdx.b8x8Index() )
    {
    case 0:
      {
        if( bAboveIntra )
        {
          if( bLeftIntra )
          {
            pcBuffer->mergeFromLeftAbove( cIdx, bLeftAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bLeftIntra )
          {
            pcBuffer->copyFromLeft( cIdx );
          }
          else if( bLeftAboveIntra )
          {
            pcBuffer->copyFromLeftAbove( cIdx );
          }
        }
      }
      break;
    case 1:
      {
        if( bAboveIntra )
        {
          if( bRightIntra )
          {
            pcBuffer->mergeFromRightAbove( cIdx, bRightAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bRightIntra )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightAboveIntra )
          {
            pcBuffer->copyFromRightAbove( cIdx );
          }
        }
      }
      break;
    case 2:
      {
        if( bBelowIntra )
        {
          if( bLeftIntra )
          {
            pcBuffer->mergeLeftBelow( cIdx, bLeftBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bLeftIntra )
          {
            pcBuffer->copyFromLeft( cIdx );
          }
          else if( bLeftBelowIntra )
          {
            pcBuffer->copyFromLeftBelow( cIdx );
          }
        }
      }
      break;
    case 3:
      {
        if( bBelowIntra )
        {
          if( bRightIntra )
          {
            pcBuffer->mergeRightBelow( cIdx, bRightBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bRightIntra )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightBelowIntra )
          {
            pcBuffer->copyFromRightBelow( cIdx );
          }
        }
      }
      break;
    default:
      break;
    }
  }

  return Err::m_nOK;
}

H264AVC_NAMESPACE_END





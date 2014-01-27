#ifndef __H264AVCDECODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
#define __H264AVCDECODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79


#include "ReadBitstreamFile.h"
#include "WriteYuvToFile.h"

#define MAX_REFERENCE_FRAMES 15
#define MAX_B_FRAMES         15

#include <algorithm>
#include <list>

#include "DecoderParameter.h"


class H264AVCDecoderTest  
{
protected:
	H264AVCDecoderTest();
	virtual ~H264AVCDecoderTest();

public:
  static ErrVal create( H264AVCDecoderTest*& rpcH264AVCDecoderTest );
  ErrVal init( DecoderParameter *pcDecoderParameter, WriteYuvToFile *pcWriteYuv, ReadBitstreamFile *pcReadBitstreamFile );//TMM_EC
  ErrVal go();
  ErrVal destroy();
  ErrVal setec( UInt uiErrorConceal);//TMM_EC

  ErrVal setCrop();

protected:
  ErrVal xGetNewPicBuffer ( PicBuffer*& rpcPicBuffer, UInt uiSize );
  ErrVal xRemovePicBuffer ( PicBufferList& rcPicBufferUnusedList );

protected:
  h264::CreaterH264AVCDecoder*   m_pcH264AVCDecoder;
  h264::CreaterH264AVCDecoder*   m_pcH264AVCDecoderSuffix; //JVT-S036 lsj
  ReadBitstreamIf*            m_pcReadBitstream;
  WriteYuvIf*                 m_pcWriteYuv;
  WriteYuvIf*				  m_pcWriteSnapShot;   //SEI LSJ
  DecoderParameter*           m_pcParameter;
	
  PicBufferList               m_cActivePicBufferList;
  PicBufferList               m_cUnusedPicBufferList;
};

#endif //__H264AVCDECODERTEST_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79

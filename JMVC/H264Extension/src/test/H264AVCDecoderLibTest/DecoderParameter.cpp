#include <cstdio>
#include "H264AVCDecoderLibTest.h"
#include "DecoderParameter.h"

#ifndef MSYS_WIN32
#define stricmp strcasecmp
#endif


DecoderParameter::DecoderParameter()
{
}

DecoderParameter::~DecoderParameter()
{
}


ErrVal DecoderParameter::init(int argc, char** argv)
{

//  Char* pcCom;

  
  if (argc <4 || argc>5)
    RNOKS ( xPrintUsage(argv) );
  cBitstreamFile = argv[1]; // input bitstream
  cYuvFile       = argv[2]; // decoded output file

  uiNumOfViews   = atoi (argv[3]);
  if (argc==5) {
	  uiMaxPocDiff = (unsigned int) atoi( argv[4] );	
	  if (uiMaxPocDiff<=0)
		  uiMaxPocDiff= 1000; //MSYS_UINT_MAX;
  } else
	uiMaxPocDiff= 1000; //MSYS_UINT_MAX;
	
   

  return Err::m_nOK;
}



ErrVal DecoderParameter::xPrintUsage(char **argv)
{
	printf("usage: %s <BitstreamFile> <YuvOutputFile> <NumOfViews>  [<maxPodDiff>] \n\n", argv[0] );
	RERRS();
}

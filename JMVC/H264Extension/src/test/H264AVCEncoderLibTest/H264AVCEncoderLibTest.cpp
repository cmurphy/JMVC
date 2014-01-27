#include <cstdio>
#include "H264AVCEncoderLibTest.h"
#include "H264AVCEncoderTest.h"
#include "H264AVCCommonLib/CommonBuffers.h"


int
main( int argc, char** argv)   
{
  printf("JMVC %s Encoder (running on a %d-bit system)\n\n",_JMVC_VERSION_, sizeof(void*)*8);

  H264AVCEncoderTest*               pcH264AVCEncoderTest = NULL;
  RNOK( H264AVCEncoderTest::create( pcH264AVCEncoderTest ) );

  RNOKS( pcH264AVCEncoderTest->init   ( argc, argv ) );
  RNOK ( pcH264AVCEncoderTest->go     () );
  RNOK ( pcH264AVCEncoderTest->destroy() );

  return 0;
}

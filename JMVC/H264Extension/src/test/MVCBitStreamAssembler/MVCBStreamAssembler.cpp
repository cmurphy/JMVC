#include <cstdio>
#include "MVCBStreamAssembler.h"
#include "AssemblerParameter.h"
#include "Assembler.h"


int main( int argc, char** argv)
{
  Assembler*          pcAssembler = NULL;
  AssemblerParameter  cParameter;

  printf( "JMVM %s MVC BitStream Assembler \n\n", "1.0");

  RNOKRS( cParameter.init       ( argc, argv ),   -2 );

  
  for( Int n = 0; n < 1; n++ )
  {
    RNOKR( Assembler::create    ( pcAssembler ),  -3 );

    RNOKR( pcAssembler->init    ( &cParameter ),  -4 );

    RNOKR( pcAssembler->go      (),               -5 );

    RNOKR( pcAssembler->destroy (),               -6 );
  }

  return 0;
}

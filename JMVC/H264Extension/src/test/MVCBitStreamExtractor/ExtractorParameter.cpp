#include <cstdio>
#include "MVCBStreamExtractor.h"
#include "ExtractorParameter.h"


#ifndef MSYS_WIN32
#define stricmp strcasecmp
#endif

#define equal(a,b)  (!stricmp((a),(b)))




ExtractorParameter::ExtractorParameter()
: m_cInFile       ()
, m_cOutFile      ()
, m_iResult       ( -10 )
, m_bAnalysisOnly ( true )
{
}



ExtractorParameter::~ExtractorParameter()
{
}



ErrVal
ExtractorParameter::init( Int     argc,
                          Char**  argv )	
{
  Bool	bOpIdSpecified			  = false;

  
#define EXIT(x,m) {if(x){printf("\n%s\n",m);RNOKS(xPrintUsage(argv))}}

  //===== get file names and set parameter "AnalysisOnly" =====
 EXIT( argc < 2, "No arguments specified" );
  m_iResult       = 0;
  m_bAnalysisOnly = ( argc == 2 ? true : false );
  m_cInFile       = argv[1];
  ROTRS( m_bAnalysisOnly, Err::m_nOK );
  m_cOutFile      = argv[2];

  //===== process arguments =====
  for( Int iArg = 3; iArg < argc; iArg++ )
  {
	if( equal( "-op", argv[iArg] ) )
	{
      EXIT( iArg + 1 == argc,           "Option \"-viewid\" without argument specified" );
      EXIT( bOpIdSpecified,            "Multiple options \"-t\"" );
      m_uiOpId       = atoi( argv[ ++iArg ] );
      bOpIdSpecified = true;
      continue;	  
	}
    EXIT( true, "Unknown option specified" );
  }

  return Err::m_nOK;
#undef EXIT
}


ErrVal
ExtractorParameter::xPrintUsage( Char **argv )
{
  printf("\nUsage: %s InputStream [OutputStream | [-op] ]", argv[0] ); 
  printf("\noptions:\n");
  printf("\t-op Op -> extract the corresponding packets with operation point id = Op\n");
  printf("\n");
  RERRS();
}




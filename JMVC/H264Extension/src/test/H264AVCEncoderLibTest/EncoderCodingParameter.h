#if !defined(AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_)
#define AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
 
 
#include <cstdio>
#include <string> 
#include "CodingParameter.h"


#define MAX_NUM_VIEWS_MINUS_1  1023 
#define ROTREPORT(x,t) {if(x) {::printf("\n%s\n",t); assert(0); return Err::m_nInvalidParameter;} }

class EncoderConfigLineStr : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineStr( const Char* pcTag, std::string* pcPar, const Char* pcDefault ) : EncoderConfigLineBase( pcTag, 1 ), m_pcPar( pcPar )
  {
    *m_pcPar = pcDefault;
  };
  Void setVar( std::string& pvValue )
  {
    *m_pcPar = pvValue;
  };
protected:
  std::string* m_pcPar;
};

class EncoderConfigLineDbl : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineDbl( const Char* pcTag, Double* pdPar, Double pdDefault ) :  EncoderConfigLineBase( pcTag, 2 ), m_pdPar( pdPar ) 
  {
    *m_pdPar = pdDefault;
  };
  Void setVar( std::string& pvValue )
  {
    *m_pdPar = atof( pvValue.c_str() );
  };
protected:
  Double* m_pdPar;
};

class EncoderConfigLineInt : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineInt( const Char* pcTag, Int* piPar, Int piDefault ) : EncoderConfigLineBase( pcTag, 3 ), m_piPar( piPar )
  {
    *m_piPar = piDefault;
  };
  Void setVar( std::string& pvValue)
  {
    *m_piPar = atoi( pvValue.c_str() );
  };
protected:
  Int* m_piPar;
};

class EncoderConfigLineUInt : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineUInt( const Char* pcTag, UInt* puiPar, UInt puiDefault ) : EncoderConfigLineBase( pcTag, 4 ), m_puiPar( puiPar )
  {
    *m_puiPar = puiDefault;
  };
  Void setVar( std::string& pvValue)
  {
    *m_puiPar = atoi( pvValue.c_str() );
  };
protected:
  UInt* m_puiPar;
};

class EncoderConfigLineChar : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineChar( Char* pcTag, Char* pcPar, Char pcDefault ) : EncoderConfigLineBase( pcTag, 5 ), m_pcPar( pcPar )
  {
    *m_pcPar = pcDefault;
  };
  Void setVar( std::string& pvValue )
  {
    *m_pcPar = (Char)atoi( pvValue.c_str() );
  };
protected:
  Char* m_pcPar;
};



class EncoderCodingParameter : 
public h264::CodingParameter 
{
protected: 
  EncoderCodingParameter          (){}
  virtual ~EncoderCodingParameter (){}

public:
  static ErrVal create    ( EncoderCodingParameter*& rpcEncoderCodingParameter );
  ErrVal        destroy   ();
  ErrVal        init      ( Int     argc,
                            Char**  argv,
                            std::string&               rcBitstreamFile );

  Void          printHelp ();
  Void printHelpMVC(Int argc, Char**  argv);

protected:
  Bool    equals( const Char* str1, const Char* str2, UInt nLetter ) { return 0 == ::strncmp( str1, str2, nLetter); }


  Void    xAppendStringWithNO( std::string&  rcInStr, std::string&  rcoutStr, UInt uiV, const char * type)
  {
    char aAppendedID[10]="";

    rcoutStr=rcInStr; 
    rcoutStr.append("_");
    sprintf(aAppendedID, "%d", uiV);
    rcoutStr.append(aAppendedID);
    rcoutStr.append(type);
  }

  ErrVal  xReadFromFile      ( std::string&            rcFilename,
                               UInt                    uiViewId,
                               std::string&            rcBitstreamFile );
  //original xReadFromFile, xReadFromFile2 and xReadFromFile3 are deleted


  ErrVal  xReadFromFile_MVAcquisitionInfo      ( std::string&            rcFilename);	// SEI JVT-W060, JVT-Z038
  ErrVal  GetExponentMantissa_MVAcquisitionInfo (double Number, UInt Mant_Precision, UInt *Exponent, UInt *Mantissa );// JVT-Z038
  ErrVal  xReadLayerFromFile ( std::string&            rcFilename,
                               h264::LayerParameters&  rcLayer );
  ErrVal  xReadLine          ( FILE*                   hFile,
                               std::string*            pacTag );
  ErrVal xReadSliceGroupCfg(h264::LayerParameters&  rcLayer );
  ErrVal xReadROICfg(h264::LayerParameters&  rcLayer );
};




ErrVal EncoderCodingParameter::create( EncoderCodingParameter*& rpcEncoderCodingParameter )
{
  rpcEncoderCodingParameter = new EncoderCodingParameter;
  
  ROT( NULL == rpcEncoderCodingParameter );

  return Err::m_nOK;
}


ErrVal EncoderCodingParameter::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal EncoderCodingParameter::init( Int     argc,
                                     Char**  argv,
                                     std::string& rcBitstreamFile  )
{
  Char* pcCom;

  rcBitstreamFile = "";

  ROTS( argc < 2 )

  for( Int n = 1; n < argc; n++ )
  {
    pcCom = argv[n++];

    if( equals( pcCom, "-anafgs", 7 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt  uiLayer       = atoi( argv[n++] );
      UInt  uiNumLayers   = atoi( argv[n++] );
      ROT( CodingParameter::getLayerParameters( uiLayer ).getFGSMode() );
      CodingParameter::getLayerParameters( uiLayer ).setNumFGSLayers( uiNumLayers );
      CodingParameter::getLayerParameters( uiLayer ).setFGSFilename ( argv[n]     );
      CodingParameter::getLayerParameters( uiLayer ).setFGSMode     ( 1           );
      continue;
    }
    if( equals( pcCom, "-encfgs", 7 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt    uiLayer   = atoi( argv[n++] );
      Double  dFGSRate  = atof( argv[n++] );
      ROT( CodingParameter::getLayerParameters( uiLayer ).getFGSMode() );
      CodingParameter::getLayerParameters( uiLayer ).setFGSRate     ( dFGSRate    );
      CodingParameter::getLayerParameters( uiLayer ).setFGSFilename ( argv[n]     );
      CodingParameter::getLayerParameters( uiLayer ).setFGSMode     ( 2           );
      continue;
    }

    if( equals( pcCom, "-bf", 3 ) )
    {
      ROTS( NULL == argv[n] );
      rcBitstreamFile = argv[n];
      continue;
    }
    if( equals( pcCom, "-numl", 5 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiNumLayers = atoi( argv[n] );
      CodingParameter::setNumberOfLayers( uiNumLayers );
      continue;
    }
    if( equals( pcCom, "-rqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      Double  dResQp  = atof( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setBaseQpResidual( dResQp );
      printf("\n********** layer %1d - rqp = %f **********\n\n",uiLayer,dResQp);
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-mqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiStage = atoi( argv[n+1] );
      Double  dMotQp  = atof( argv[n+2] );
      CodingParameter::getLayerParameters( uiLayer ).setQpModeDecision( uiStage, dMotQp );
      n += 2;
      continue;      
    }
    if( equals( pcCom, "-lqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      Double  dQp     = atof( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setBaseQpResidual( dQp );
      for( UInt uiStage = 0; uiStage < MAX_DSTAGES; uiStage++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setQpModeDecision( uiStage, dQp );
      }
      CodingParameter::getLayerParameters( uiLayer ).setQpModeDecisionLP( dQp );
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-ilpred", 7 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiBLRes = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setInterLayerPredictionMode( uiBLRes );
      n += 1;
      continue;
    }
    if( equals( pcCom, "-mfile", 6 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiMode  = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setMotionInfoMode( uiMode );
      CodingParameter::getLayerParameters( uiLayer ).setMotionInfoFilename( argv[n+2] );
      n += 2;
      continue;
    }
    if( equals( pcCom, "-frms", 5 ) )
    {
      ROTS( NULL == argv[n] ); 
      UInt uiFrms = atoi( argv[n] );
      CodingParameter::setTotalFrames( uiFrms );
      continue;
    }
    if( equals( pcCom, "-bcip", 5 ) )
    {
      n--;
      ROTS( NULL == argv[n] );
      CodingParameter::getLayerParameters(0).setContrainedIntraForLP();
      continue;
    }
    if( equals( pcCom, "-cl", 3 ) )
    {
      ROTS( NULL == argv[n] );
      ROTS( NULL == argv[n+1] );
      UInt uiLayer = atoi( argv[n] );
      UInt uiCLoop = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setClosedLoop( uiCLoop );
      n += 1;
      continue;
    }
    if( equals( pcCom, "-ref", 4 ) )
    {
      ROTS( NULL == argv[n] );
      Double dLowPassEnhRef = atof( argv[n] );
      CodingParameter::setLowPassEnhRef( dLowPassEnhRef );
      continue;
    }
    if( equals( pcCom, "-ar", 3 ) )
    {
      ROTS( NULL == argv[n] );
      ROTS( NULL == argv[n + 1] );
      UInt uiBaseRefWeightZeroBlock = atoi( argv[n] );
      UInt uiBaseRefWeightZeroCoeff = atoi( argv[n + 1] );
      CodingParameter::setAdaptiveRefFGSWeights( uiBaseRefWeightZeroBlock, uiBaseRefWeightZeroCoeff );
      // skip two
      n += 1;
      continue;
    }
    if( equals( pcCom, "-fs", 3 ) )
    {
      ROTS( NULL == argv[n] );
      UInt flag = atoi( argv[n] );
      CodingParameter::setFgsEncStructureFlag( flag );
      continue;
    }
//  {{
  if( equals( pcCom, "-vf", 4) )
  {
    ROTS( NULL == argv[n] );
    ROTS( NULL == argv[n+1] );
    std::string cFilename = argv[n++];
    UInt uiViewid = atoi(argv[n]);
    RNOKS( xReadFromFile( cFilename, uiViewid, rcBitstreamFile ) );   // Read-in SPS parameters
	  continue;
  }
//  }}

//JVT-W080
	if( equals( pcCom, "-pdi", 4 ) )
	{
	  ROTS(NULL == argv[n] );
		ROTS(NULL == argv[n+1] );
		m_uiPdsEnable = 1;
		m_uiPdsInitialDelayAnc = atoi( argv[n++] );
		m_uiPdsInitialDelayNonAnc = atoi( argv[n] );
		continue;
	}
//~JVT-W080

// 
  /*
    if( equals( pcCom, "-pf", 3) )
    {
      ROTS( NULL == argv[n] );
      std::string cFilename = argv[n];
      RNOKS( xReadFromFile( cFilename, rcBitstreamFile ) );  
	  
      continue;
    }
  */
    //JVT-P031
    if( equals( pcCom, "-ds", 3) )
    {
     ROTS( NULL == argv[n] );
     ROTS( NULL == argv[n+1] );
     UInt uiLayer = atoi(argv[n]);
     CodingParameter::getLayerParameters(uiLayer).setUseDiscardable(true);
     Double dRate = atof(argv[n+1]);
     CodingParameter::getLayerParameters(uiLayer).setPredFGSRate(dRate);
     n+=1;
     continue;
    }
    //~JVT-P031
	
	//S051{
	if( equals( pcCom, "-encsip", 7 ) )
    {
		ROTS( NULL == argv[n  ] );
		ROTS( NULL == argv[n+1] );
		
		UInt    uiLayer = atoi( argv[n  ] );
		CodingParameter::getLayerParameters( uiLayer ).setEncSIP(true);
		CodingParameter::getLayerParameters( uiLayer ).setInSIPFileName(argv[n+1]);
		n += 1;
		continue;
    }
	if( equals( pcCom, "-anasip", 7 ) )
    {
		ROTS( NULL == argv[n  ] );
		ROTS( NULL == argv[n+1] );
		ROTS( NULL == argv[n+2] );
		
		UInt    uiLayer = atoi( argv[n  ] );
		UInt	uiMode = atoi( argv[n+1] );
		
		if(uiMode!=0)
			CodingParameter::getLayerParameters( uiLayer ).setAnaSIP(2);
		else
			CodingParameter::getLayerParameters( uiLayer ).setAnaSIP(1);
		
		CodingParameter::getLayerParameters( uiLayer ).setOutSIPFileName(argv[n+2]);
		n += 2;
		continue;
    }
	//S051}
	
    if( equals( pcCom, "-fgsmot", 7 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt uiLayer         = atoi( argv[n  ] );
      UInt uiFGSMotionMode = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setFGSMotionMode( uiFGSMotionMode );
      n += 1;
      continue;
    }

    if( equals( pcCom, "-org", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      ROF(    uiLayer < MAX_LAYERS );
      CodingParameter::getLayerParameters( uiLayer ).setInputFilename( argv[n+1] );
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-rec", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      ROF(    uiLayer < MAX_LAYERS );
      CodingParameter::getLayerParameters( uiLayer ).setOutputFilename( argv[n+1] );
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-ec", 3 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer  = atoi( argv[n  ] );
      ROF(    uiLayer < MAX_LAYERS );
      UInt    uiECmode = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setEntropyCodingModeFlag( uiECmode != 0 );
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-vlc", 4 ) )
    {
      n--;
      ROTS( NULL == argv[n] );
      for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setEntropyCodingModeFlag( false );
      }
      continue;
    }
    if( equals( pcCom, "-cabac", 6 ) )
    {
      n--;
      ROTS( NULL == argv[n] );
      for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setEntropyCodingModeFlag( true );
      }
      continue;
    }

    if( equals( pcCom, "-h", 2) )
    {
      printHelp();
      return Err::m_nOK;
    }

    return Err::m_nERR;
  }

//JVT-W080
	if( m_uiMVCmode && m_uiPdsEnable )
	{
		m_uiPdsBlockSize = m_uiFrameWidth/16;
		UInt uiViewNum = (UInt)SpsMVC.m_num_views_minus_1+1;
		UInt uiRefNum = m_uiNumRefFrames;
	  m_ppuiPdsInitialDelayMinus2L0Anc = new UInt* [uiViewNum];
		m_ppuiPdsInitialDelayMinus2L1Anc = new UInt* [uiViewNum];
	  m_ppuiPdsInitialDelayMinus2L0NonAnc = new UInt* [uiViewNum];
		m_ppuiPdsInitialDelayMinus2L1NonAnc = new UInt* [uiViewNum];
    for( UInt i = 0; i < uiViewNum; i++ )
		{
	    m_ppuiPdsInitialDelayMinus2L0Anc[i] = new UInt [uiRefNum];
			m_ppuiPdsInitialDelayMinus2L1Anc[i] = new UInt [uiRefNum];
	    m_ppuiPdsInitialDelayMinus2L0NonAnc[i] = new UInt [uiRefNum];
			m_ppuiPdsInitialDelayMinus2L1NonAnc[i] = new UInt [uiRefNum];
			for( UInt j = 0; j < uiRefNum; j++ )
			{
				m_ppuiPdsInitialDelayMinus2L0Anc[i][j] = 0;
				m_ppuiPdsInitialDelayMinus2L1Anc[i][j] = 0;
				m_ppuiPdsInitialDelayMinus2L0NonAnc[i][j] = 0;
				m_ppuiPdsInitialDelayMinus2L1NonAnc[i][j] = 0;
			}
		}
	}
//~JVT-W080

  RNOKS( check() );
  
  return Err::m_nOK;
}


Void EncoderCodingParameter::printHelpMVC(Int     argc,
                                          Char**  argv)
{
  printf("Usage: %s -vf <encoder.cfg> <view_id>\n\n", argv[0]);
  printf("\n supported options:\n\n");
  printf("  -vf     Parameter File Name\n\n");

  printf("  -h      Print Option List \n");
  printf("\n");
}

Void EncoderCodingParameter::printHelp()
{
  printf("\n supported options:\n\n");
  printf("  -pf     Parameter File Name\n\n");

  printf("  -bf     BitStreamFile\n");
  printf("  -frms   Number of total frames\n");
  printf("  -numl   Number Of Layers\n");
  printf("  -cabac  CABAC for all layers as entropy coding mode\n");
  printf("  -vlc    VLC for all layers as entropy coding mode\n");
  printf("  -org    (Layer) (original file)\n");
  printf("  -rec    (Layer) (reconstructed file)\n");
  printf("  -ec     (Layer) (entropy coding mode)\n");
  printf("  -rqp    (Layer) (ResidualQP)\n");
  printf("  -mqp    (Layer) (Stage) (MotionQP)\n");
  printf("  -lqp    (Layer) (ResidualAndMotionQP)\n");
  printf("  -ilpred (Layer) (InterLayerPredictionMode)\n");
  printf("  -mfile  (Layer) (Mode) (MotionInfoFile)\n");
  printf("  -anafgs (Layer) (NumFGSLayers) (File for storing FGS parameters)\n");
  printf("  -encfgs (Layer) (bit-rate in kbps) (File with stored FGS parameters)\n");
  printf("  -cl     (Layer) (ClosedLoopParameter)\n");
  printf("  -ds     (Layer) (Rate for inter-layer prediction)\n");
  printf("  -fgsmot (Layer) (FGSMotionRefinementMode) [0: no, 1: HP only, 2: all]\n");
  printf("  -lcupd  Update method [0 - original, 1 - low-complexity (default)]\n");
  printf("  -bcip   Constrained intra prediction for base layer (needed for single-loop) in scripts\n");
  //S051{
  printf("  -anasip (Layer) (SIP Analysis Mode)[0: persists all inter-predictions, 1: forbids all inter-prediction.] (File for storing bits information)\n");
  printf("  -encsip (Layer) (File with stored SIP information)\n");
  //S051}
  printf("  -h      Print Option List \n");
  printf("\n");
}


ErrVal EncoderCodingParameter::xReadLine( FILE* hFile, std::string* pacTag )
  {
  ROF( pacTag );

    Int  n;
    UInt uiTagNum = 0;
  Bool          bComment  = false;
  std::string*  pcTag     = &pacTag[0];

    for( n = 0; n < 4; n++ )
    {
      pacTag[n] = "";
    }

  for( n = 0; ; n++ )
  {
      Char cChar = (Char) fgetc( hFile );
    ROTRS( cChar == '\n' || feof( hFile ), Err::m_nOK );  // end of line
    if   ( cChar == '#' )
      {
      bComment = true;
      }
    if( ! bComment )
    {
      if ( cChar == '\t' || cChar == ' ' ) // white space
        {
			//ROTR( uiTagNum == 3, Err::m_nERR );
			if( ! pcTag->empty() )
			{
				ROTR( uiTagNum == 3, Err::m_nERR ); // SEI JVT-W060
				uiTagNum++;
				pcTag = &pacTag[uiTagNum]; 
			}
		}
      else
		{
        *pcTag += cChar;
		}
	}
  }

 }


//  
ErrVal  
EncoderCodingParameter::xReadFromFile  ( std::string&    rcFilename,
                                         UInt            uiViewId,
                                         std::string&    rcBitstreamFile)
{
  std::string acTags[4];
  UInt        uiParLnCount = 0;
  std::string cFileNameStringTemp, cInputFilename,  cInputFile, cReconFile;
  std::string cAcquisitionFile; // SEI JVT-W060

  Int i, j;

  FILE *f = fopen( rcFilename.c_str(), "r");
  if( NULL == f )
  { 
    printf( "failed to open %s parameter file\n", rcFilename.c_str() );
    return Err::m_nERR;
  } 

  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr ("OutputFile",              &rcBitstreamFile,                                      "test.264");
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("FrameRate",               &m_dMaximumFrameRate,                                  60.0      );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("MaxDelay",                &m_dMaximumDelay,                                      1200.0    );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("FramesToBeEncoded",       &m_uiTotalFrames,                                      1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("GOPSize",                 &m_uiGOPSize,                                          1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("IntraPeriod",             &m_uiIntraPeriod,                                      MSYS_UINT_MAX );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("NumberReferenceFrames",   &m_uiNumRefFrames,                                     1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("BaseLayerMode",           &m_uiBaseLayerMode,                                    3 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("InterPredPicsFirst",      &m_uiInterPredPicsFirst ,                              1 ); // JVT-V043
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("NumLayers",               &m_uiNumberOfLayers,                                   1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SearchRange",             &(m_cMotionVectorSearchParams.m_uiSearchRange),        96);
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("BiPredIter",              &(m_cMotionVectorSearchParams.m_uiNumMaxIter),         4 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("IterSearchRange",         &(m_cMotionVectorSearchParams.m_uiIterSearchRange),    8 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("LoopFilterDisable",       &(m_cLoopFilterParams.m_uiFilterIdc),                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("LoopFilterAlphaC0Offset", (Int*)&(m_cLoopFilterParams.m_iAlphaOffset),           0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("LoopFilterBetaOffset",    (Int*)&(m_cLoopFilterParams.m_iBetaOffset),            0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("SearchMode",              (Int*)&(m_cMotionVectorSearchParams.m_eSearchMode),    0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("SearchFuncFullPel",       (Int*)&(m_cMotionVectorSearchParams.m_eFullPelDFunc),  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("SearchFuncSubPel",        (Int*)&(m_cMotionVectorSearchParams.m_eSubPelDFunc),   0 );
// not need to be inputed
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("Log2MaxFrameNum",         &m_uiLog2MaxFrameNum,                                  12 ); // originally it was 9, this modification might not be needed
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("Log2MaxPocLsb",           &m_uiLog2MaxPocLsb,                                    12 ); // same as above: CY
//TMM_WP
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("WeightedPrediction",         &m_uiIPMode,                                     0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("WeightedBiprediction",       &m_uiBMode,                                      0 );  
  
//TMM_WP
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MVCMode",                 &m_uiMVCmode,                                          1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer0Quant",        &m_adDeltaQpLayer[0],                                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer1Quant",        &m_adDeltaQpLayer[1],                                  3 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer2Quant",        &m_adDeltaQpLayer[2],                                  4 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer3Quant",        &m_adDeltaQpLayer[3],                                  5 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer4Quant",        &m_adDeltaQpLayer[4],                                  6 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("DeltaLayer5Quant",        &m_adDeltaQpLayer[5],                                  7 );

  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr ("InputFile",               &cInputFile,                                           "in.yuv");
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr ("ReconFile",               &cReconFile,                                           "rec.yuv");
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceWidth",             &m_uiFrameWidth,                                       0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceHeight",            &m_uiFrameHeight,                                      0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SymbolMode",              &m_uiSymbolMode,                                       1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("FRExt",                   &m_ui8x8Mode,                                          1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("BasisQP",                 &m_dBasisQp,                                          26 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("PicOrderCntType",         &m_uiPicOrderCntType,                                  0 );//poc type 

  //SEI LSJ{
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("NestingSEI",              &m_uiNestingSEIEnable,                                 0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SnapShot",                &m_uiSnapShotEnable,                                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("ActiveViewSEI",           &m_uiActiveViewSEIEnable,                             0 ); 
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("ViewScalInfoSEI",		 &m_uiViewScalInfoSEIEnable,						   0 ); 
  //SEI LSJ

  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MultiviewSceneInfoSEI",           &m_uiMultiviewSceneInfoSEIEnable,                             0 );  // SEI JVT-W060
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MaxDisparity",           &m_uiMaxDisparity,                             0 );  // SEI JVT-W060
 
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MultiviewAcquisitionInfoSEI",           &m_uiMultiviewAcquisitionInfoSEIEnable,                             0 );  // SEI JVT-W060
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr("AcquisitionInfoFile",           &cAcquisitionFile,                             "cam.cfg" );  // SEI JVT-W060
 
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr ("ViewOrder",               &m_cViewOrder,                                      "");
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("AVCBaseView",             &m_uiBaseViewId,                                       0);
//JVT-W080
	m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("PDISEIMessage",           &m_uiPdsEnable,                                        0 );
	m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("PDIInitialDelayAnc",      &m_uiPdsInitialDelayAnc,                               0 );
	m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("PDIInitialDelayNonAnc",   &m_uiPdsInitialDelayNonAnc,                            0 );
//lufeng: read from .cfg
    m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("MbAff",      &m_uiMbAff,                               0 );
    m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("PAff",   &m_uiPAff,                            0 );
//~JVT-W080
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("DPBConformanceCheck",             &m_uiDPBConformanceCheck,                                       0);
  m_CurrentViewId = uiViewId; 
  m_bAVCFlag      = false;
  if ( uiViewId == m_uiBaseViewId ) m_bAVCFlag = true;

  m_pEncoderLines[uiParLnCount] = NULL;


  int cur_view_id=0, view_id, view_cnt=-1;
  int cur_level_id,level_cnt=-1;
  int num_of_ops=0,num_target_views_minus1,num_views_minus1,temporal_id,view_idx;
  int ref_idx,num_of_refs,temp;
  //int vcOrder;
  CodingParameter::SpsMVC.setNumViewsMinus1(0);

  //JVT-V054  
  CodingParameter::SpsMVC.setInitDone(false);
  

  while (!feof(f))
  {
    RNOK( xReadLine( f, acTags ) );
    if ( acTags[0].empty() )
    {
      continue;
    }
    for (UInt ui=0; m_pEncoderLines[ui] != NULL; ui++)
    {
      if( acTags[0] == m_pEncoderLines[ui]->getTag() )
      {
        m_pEncoderLines[ui]->setVar( acTags[1] );
        break;
      }
    }
	
    // view prediciton informaiton

    if (acTags[0] == "NumViewsMinusOne") 
    {
      AOF(( temp=atoi(acTags[1].c_str()))>=0);
      AOF(temp<=MAX_NUM_VIEWS_MINUS_1);
      CodingParameter::SpsMVC.setNumViewsMinus1(temp);
      CodingParameter::SpsMVC.initViewSPSMemory_num_refs_for_lists(CodingParameter::SpsMVC.getNumViewMinus1());
    }

	//JVT-V054  
	if (acTags[0] == "ViewOrder") {
		CodingParameter::SpsMVC.setViewCodingOrder(m_cViewOrder);
		UInt *ViewCodingOrder=CodingParameter::SpsMVC.getViewCodingOrder();
		for (i=0; i<= CodingParameter::SpsMVC.getNumViewMinus1(); i++)
			if (m_CurrentViewId == ViewCodingOrder[i])
				break;		
		if (i > CodingParameter::SpsMVC.getNumViewMinus1())	{
			printf("Error : View dependency information on the specified view_id is not specified\n\n");
			return -1;
		}
		

	}

    if (acTags[0] == "View_ID") 
    {
      AOF((cur_view_id=atoi(acTags[1].c_str()))>=0);      
      AOF(++view_cnt<=CodingParameter::SpsMVC.getNumViewMinus1());		
    }

    // JVT-W035 
    if (acTags[0] == "Fwd_NumAnchorRefs")
    {
      AOF((num_of_refs=atoi(acTags[1].c_str()))>=0); // the number should be nonnegative
      AOF(num_of_refs<=CodingParameter::SpsMVC.getNumViewMinus1());
      CodingParameter::SpsMVC.setNumAnchorRefsForListX(cur_view_id,0,num_of_refs);  // list0
      CodingParameter::SpsMVC.initViewSPSMemory_ref_for_lists(CodingParameter::SpsMVC.getNumViewMinus1(),cur_view_id,0,0);// list0,anchor	
    }

    if (acTags[0] == "Bwd_NumAnchorRefs")
    {
      AOF((num_of_refs=atoi(acTags[1].c_str()))>=0); // the number should be nonnegative
      AOF(num_of_refs<=CodingParameter::SpsMVC.getNumViewMinus1()); 
      CodingParameter::SpsMVC.setNumAnchorRefsForListX(cur_view_id,1,num_of_refs);  //  list1
      CodingParameter::SpsMVC.initViewSPSMemory_ref_for_lists(CodingParameter::SpsMVC.getNumViewMinus1(),cur_view_id,1,0);// list1,anchor	
    }


    if (acTags[0] == "Fwd_NumNonAnchorRefs")
    {
		  AOF((num_of_refs=atoi(acTags[1].c_str()))>=0); // the number should be nonnegative
		  AOF(num_of_refs<=CodingParameter::SpsMVC.getNumViewMinus1()); 
      CodingParameter::SpsMVC.setNumNonAnchorRefsForListX(cur_view_id,0,num_of_refs);  // list0
      CodingParameter::SpsMVC.initViewSPSMemory_ref_for_lists(CodingParameter::SpsMVC.getNumViewMinus1(),cur_view_id,0,1);// list0,non-anchor	
    }

    if (acTags[0] == "Bwd_NumNonAnchorRefs")
    {
		  AOF((num_of_refs=atoi(acTags[1].c_str()))>=0); // the number should be nonnegative
		  AOF(num_of_refs<=CodingParameter::SpsMVC.getNumViewMinus1()); 
      CodingParameter::SpsMVC.setNumNonAnchorRefsForListX(cur_view_id,1,num_of_refs);  // list1
      CodingParameter::SpsMVC.initViewSPSMemory_ref_for_lists(CodingParameter::SpsMVC.getNumViewMinus1(),cur_view_id,1,1);// list1,non-anchor	
  	}
	   
    if (acTags[0] == "Fwd_AnchorRefs")
    {
      AOF((ref_idx=atoi(acTags[1].c_str()))< (int)CodingParameter::SpsMVC.getNumAnchorRefsForListX(cur_view_id,0));      
	  AOF((view_id=atoi(acTags[2].c_str())) >= 0);      
      CodingParameter::SpsMVC.setAnchorRefForListX(cur_view_id,ref_idx,0,view_id);
     
      if(m_CurrentViewId==cur_view_id) 
      {
      
        xAppendStringWithNO ( cReconFile, cFileNameStringTemp, view_id, ".yuv");

		for (i=0; i<(int)m_MultiviewReferenceFileParams.size() ; i++) {
			if ( m_MultiviewReferenceFileParams[i]._view_id  == view_id) // file already present, so skip
				break;
		}
		if (i==	m_MultiviewReferenceFileParams.size())	
		m_MultiviewReferenceFileParams.push_back
				(YUVFileParams(cFileNameStringTemp,view_id,
				m_uiFrameHeight, CodingParameter::getVerPadding(), 
				m_uiFrameWidth, CodingParameter::getHorPadding()));
      }
    }

    if (acTags[0] == "Bwd_AnchorRefs")
    {
      AOF((ref_idx=atoi(acTags[1].c_str()))< (int)CodingParameter::SpsMVC.getNumAnchorRefsForListX(cur_view_id,1));      
	  AOF((view_id=atoi(acTags[2].c_str())) >= 0);    
      CodingParameter::SpsMVC.setAnchorRefForListX(cur_view_id,ref_idx,1,view_id);
     
      if(m_CurrentViewId==cur_view_id)
      {
      
        xAppendStringWithNO ( cReconFile, cFileNameStringTemp, view_id, ".yuv");

		for (i=0; i<(int)m_MultiviewReferenceFileParams.size() ; i++) {
			if ( m_MultiviewReferenceFileParams[i]._view_id  == view_id) // file already present, so skip
				break;
		}

		if (i==	m_MultiviewReferenceFileParams.size())	
			m_MultiviewReferenceFileParams.push_back
			(YUVFileParams(cFileNameStringTemp,view_id,m_uiFrameHeight, 
			CodingParameter::getVerPadding(), 
			m_uiFrameWidth, CodingParameter::getHorPadding()));
      }
    }
	
    if (acTags[0] == "Fwd_NonAnchorRefs")
    {
      AOF((ref_idx=atoi(acTags[1].c_str()))< (int)CodingParameter::SpsMVC.getNumNonAnchorRefsForListX(cur_view_id,0));      
	  AOF((view_id=atoi(acTags[2].c_str())) >= 0);    
      CodingParameter::SpsMVC.setNonAnchorRefForListX(cur_view_id,ref_idx,0,view_id);
	  
	  if(m_CurrentViewId==cur_view_id)
      {
      
        xAppendStringWithNO ( cReconFile, cFileNameStringTemp, view_id, ".yuv");

		for (i=0; i<(int)m_MultiviewReferenceFileParams.size() ; i++) {
			if ( m_MultiviewReferenceFileParams[i]._view_id  == view_id) // file already present, so skip
				break;
		}

		if (i==	m_MultiviewReferenceFileParams.size())	
			m_MultiviewReferenceFileParams.push_back
			(YUVFileParams(cFileNameStringTemp,view_id,
			m_uiFrameHeight, CodingParameter::getVerPadding(), 
			m_uiFrameWidth, CodingParameter::getHorPadding()));
      }	
    }

    if (acTags[0] == "Bwd_NonAnchorRefs")
    {
      AOF((ref_idx=atoi(acTags[1].c_str()))< (int)CodingParameter::SpsMVC.getNumNonAnchorRefsForListX(cur_view_id,1));      
	  AOF((view_id=atoi(acTags[2].c_str())) >= 0);    
      CodingParameter::SpsMVC.setNonAnchorRefForListX(cur_view_id,ref_idx,1,view_id);
	  if(m_CurrentViewId==cur_view_id)
      {
          
        xAppendStringWithNO ( cReconFile, cFileNameStringTemp, view_id, ".yuv");

		for (i=0; i<(int)m_MultiviewReferenceFileParams.size() ; i++) {
			if ( m_MultiviewReferenceFileParams[i]._view_id  == view_id) 
				break;
		}

		if (i==	m_MultiviewReferenceFileParams.size())	
			m_MultiviewReferenceFileParams.push_back
			(YUVFileParams(cFileNameStringTemp,view_id,
			m_uiFrameHeight, CodingParameter::getVerPadding(), 
			m_uiFrameWidth, CodingParameter::getHorPadding()));
      }

    }

	if (acTags[0] == "NumLevelValuesSignalledMinus1") 
    {
      AOF(( temp=atoi(acTags[1].c_str()))>=0);
      AOF(temp<=63); //hard-coded 
      CodingParameter::SpsMVC.setNumLevelValuesSignalledMinus1(temp);
      CodingParameter::SpsMVC.initViewSPSMemory_num_level_related_memory(CodingParameter::SpsMVC.getNumLevelValuesSignalledMinus1());
    }

	if (acTags[0] == "Level_IDC" && CodingParameter::SpsMVC.m_ui_level_idc!=NULL ) 
    {
      AOF((cur_level_id=atoi(acTags[1].c_str()))>=0);       
      AOF(++level_cnt<=CodingParameter::SpsMVC.getNumLevelValuesSignalledMinus1());		
	  CodingParameter::SpsMVC.m_ui_level_idc[level_cnt]=cur_level_id;

    }

	if (acTags[0] == "NumApplicableOpsMinus1" && CodingParameter::SpsMVC.m_ui_num_applicable_ops_minus1!=NULL)
    {
      AOF((num_of_ops=atoi(acTags[1].c_str()))>=0); 
      AOF(num_of_ops<=1023); // hard-coded
      CodingParameter::SpsMVC.m_ui_num_applicable_ops_minus1[level_cnt]=num_of_ops;
	  CodingParameter::SpsMVC.initViewSPSMemory_num_level_related_memory_2D(num_of_ops,level_cnt);      
	}

	if (acTags[0] == "ApplicableOpTemporalId" && CodingParameter::SpsMVC.m_ui_applicable_op_temporal_id!=NULL)
    {
      AOF((ref_idx=atoi(acTags[1].c_str()))<= (int)CodingParameter::SpsMVC.m_ui_num_applicable_ops_minus1[level_cnt]);
      temporal_id=atoi(acTags[2].c_str());
      CodingParameter::SpsMVC.m_ui_applicable_op_temporal_id[level_cnt][ref_idx]=temporal_id;
    }

	if (acTags[0] == "ApplicableOpNumTargetViewsMinus1"&& CodingParameter::SpsMVC.m_ui_applicable_op_num_target_views_minus1!=NULL)
    {
      AOF((ref_idx=atoi(acTags[1].c_str()))<= (int)CodingParameter::SpsMVC.m_ui_num_applicable_ops_minus1[level_cnt]);
      num_target_views_minus1=atoi(acTags[2].c_str());
      CodingParameter::SpsMVC.m_ui_applicable_op_num_target_views_minus1[level_cnt][ref_idx]=num_target_views_minus1;
	  CodingParameter::SpsMVC.initViewSPSMemory_num_level_related_memory_3D(num_of_ops,num_target_views_minus1,level_cnt,ref_idx);   
    }

	if (acTags[0] == "ApplicableOpNumViewsMinus1" && CodingParameter::SpsMVC.m_ui_applicable_op_num_views_minus1!=NULL)
    {
      AOF((ref_idx=atoi(acTags[1].c_str()))<= (int)CodingParameter::SpsMVC.m_ui_num_applicable_ops_minus1[level_cnt]);
      num_views_minus1=atoi(acTags[2].c_str());
      CodingParameter::SpsMVC.m_ui_applicable_op_num_views_minus1[level_cnt][ref_idx]=num_views_minus1;
    }

	if (acTags[0] == "ApplicableOpTargetViewId" && CodingParameter::SpsMVC.m_ui_applicable_op_target_view_id!=NULL)
    {
      AOF((ref_idx=atoi(acTags[1].c_str()))<= (int)CodingParameter::SpsMVC.m_ui_num_applicable_ops_minus1[level_cnt]);      
      AOF((view_idx=atoi(acTags[2].c_str()))<= (int)CodingParameter::SpsMVC.m_ui_applicable_op_num_target_views_minus1[level_cnt][ref_idx]);            
      CodingParameter::SpsMVC.m_ui_applicable_op_target_view_id[level_cnt][ref_idx][view_idx]=atoi(acTags[3].c_str());
	     
	}

	
  }

  /*if(m_uiPAff>0&&m_uiIntraPeriod<m_uiTotalFrames)//lufeng
	{
      fprintf(stderr, "anchor picture not supported in field coding, set IntraPeriod >= totalFrames\n");
      AF();
    }*/

  if((m_uiPAff>0)&&(CodingParameter::SpsMVC.getNumViewMinus1()>1)) // hwsun (add if)
	{
      fprintf(stderr, "field coding is used for 1/2 views only, set NumViewsMinusOne <= 1\n");
      AF();
    }

  if (m_uiMultiviewAcquisitionInfoSEIEnable==1) // SEI JVT-W060
  {
	RNOKS( xReadFromFile_MVAcquisitionInfo( cAcquisitionFile) ); 

  }


  //JVT-V054
//  CodingParameter::SpsMVC.setViewCodingOrder(m_cViewOrder);

  UInt *order = CodingParameter::SpsMVC.getViewCodingOrder();

  if(order[0] == m_CurrentViewId)
        m_bAVCFlag      = true;
  else   
        m_bAVCFlag      = false;

  /////////////////////////
    if ( view_cnt != CodingParameter::SpsMVC.getNumViewMinus1() )
    {
      fprintf(stderr, "Could not locate all view-dependency information: check config file syntax\n");
      AF();
    }
    ///////////////

	// Setting default values related to level
	if (CodingParameter::SpsMVC.m_ui_level_idc==NULL)
	{
		UInt temp_num_views;

		CodingParameter::SpsMVC.initViewSPSMemory_num_level_related_memory(CodingParameter::SpsMVC.getNumLevelValuesSignalledMinus1());
		CodingParameter::SpsMVC.m_ui_level_idc[0]=0;
		CodingParameter::SpsMVC.m_ui_num_applicable_ops_minus1[0]=0;
	    CodingParameter::SpsMVC.initViewSPSMemory_num_level_related_memory_2D(0,0);      
		CodingParameter::SpsMVC.m_ui_applicable_op_temporal_id[0][0]=0;
		CodingParameter::SpsMVC.m_ui_applicable_op_num_target_views_minus1[0][0]=(temp_num_views=CodingParameter::SpsMVC.getNumViewMinus1());
		CodingParameter::SpsMVC.initViewSPSMemory_num_level_related_memory_3D(0,temp_num_views,0,0);   
		CodingParameter::SpsMVC.m_ui_applicable_op_num_views_minus1[0][0]=temp_num_views;
		for (int k=0;k<=(int)temp_num_views;k++)
			CodingParameter::SpsMVC.m_ui_applicable_op_target_view_id[0][0][k]=order[k];	

	}
	////

      uiParLnCount = 0;
      while (m_pEncoderLines[uiParLnCount] != NULL)
      {
        delete m_pEncoderLines[uiParLnCount];
        m_pEncoderLines[uiParLnCount] = NULL;
        uiParLnCount++;
      }


      printf( "SPS: num_views_minus_1: %d\n",CodingParameter::SpsMVC.m_num_views_minus_1 ); // ue(v)

      for (i=0;i<= CodingParameter::SpsMVC.m_num_views_minus_1; i++)
      {
        //vcOrder = CodingParameter::SpsMVC.m_uiViewCodingOrder[i];       

        printf("SPS: num_anchor_refs_l0[%d]: %d\n", i, CodingParameter::SpsMVC.m_num_anchor_refs_list0[i]);
        for (j=0; j<CodingParameter::SpsMVC.m_num_anchor_refs_list0[i]; j++)
          printf("SPS: anchor_ref_l0[%d][%d]: %d\n",  i, j, CodingParameter::SpsMVC.m_anchor_ref_list0[i][j] );
		
        printf("SPS: num_anchor_refs_l1[%d]: %d\n", i, CodingParameter::SpsMVC.m_num_anchor_refs_list1[i]);
        for (j=0; j<CodingParameter::SpsMVC.m_num_anchor_refs_list1[i]; j++)
          printf("SPS: anchor_ref_l1[%d][%d]: %d\n",  i, j, CodingParameter::SpsMVC.m_anchor_ref_list1[i][j] );
      }

      for (i=0;i<= CodingParameter::SpsMVC.m_num_views_minus_1; i++)
      {
        //vcOrder = CodingParameter::SpsMVC.m_uiViewCodingOrder[i];       

        printf("SPS: num_non_anchor_refs_l0[%d]: %d\n", i, CodingParameter::SpsMVC.m_num_non_anchor_refs_list0[i] ); 
        for (j=0; j<CodingParameter::SpsMVC.m_num_non_anchor_refs_list0[i]; j++)
    			printf("SPS: non_anchor_ref_l0[%d][%d]: %d\n",i,j, CodingParameter::SpsMVC.m_non_anchor_ref_list0[i][j]);
    
        printf("SPS: num_non_anchor_refs_l1[%d]: %d\n", i, CodingParameter::SpsMVC.m_num_non_anchor_refs_list1[i] ); 
        for (j=0; j<CodingParameter::SpsMVC.m_num_non_anchor_refs_list1[i]; j++)
		    	printf("SPS: non_anchor_ref_l1[%d][%d]: %d\n", i,j, CodingParameter::SpsMVC.m_non_anchor_ref_list1[i][j]);
	  }	
      if( m_uiMVCmode )
      {
        m_uiNumberOfLayers = 0;
        xAppendStringWithNO ( cInputFile, cFileNameStringTemp, m_CurrentViewId, ".yuv");
        getLayerParameters(0).setInputFilename  ( (Char*)cFileNameStringTemp.c_str() );
        xAppendStringWithNO ( cReconFile, cFileNameStringTemp, m_CurrentViewId, ".yuv");
        getLayerParameters(0).setOutputFilename ( (Char*)cFileNameStringTemp.c_str() );

        getLayerParameters(0).setFrameWidth     ( m_uiFrameWidth );
        getLayerParameters(0).setFrameHeight    ( m_uiFrameHeight );
        xAppendStringWithNO ( rcBitstreamFile, rcBitstreamFile, m_CurrentViewId, ".264");

      }

      fclose( f );
      return Err::m_nOK;
}

ErrVal EncoderCodingParameter::GetExponentMantissa_MVAcquisitionInfo(double arg, UInt mant_prec, UInt *E, UInt *M)// JVT-Z038
{
	double log_val;
	int expo,v=0,i;
	double mant;

	/* Find expo & mant when arg=(-1)^sign*2^expo*(0.mant),*/
	if (arg==0.0){
		expo=0;
		mant=0;
	} else {
		log_val= (double)log((double)fabs((double)arg))/(double)log(2.0);
		expo = (int)floor((double)log_val);
		mant = (double)pow((double)2, (double)(log_val - (double)expo));
		if ((double)fabs((double)mant) > 1.0) {
			mant = mant / (double)2.0;
			expo= expo + 1;
		}		
	}

	/* Convert expo & mant so that X=(-1)^s*2^expo*(1.mant) */
	if (expo == 0 && mant ==0.0 ) {
       v=0;
	} else if (expo> -30) { 
		while ((double)fabs(mant)<(double)1.0) {
			mant=mant*2.0;
			expo=expo-1;
		}
        if (mant>=0.0)
			mant = mant-1.0;
        v= expo + mant_prec; // number of necessary mantissa bits in the case of truncation
		if (v<0) v=0;
	    expo += 31;
	} else if (expo == -30) {
		v= expo + mant_prec; // number of necessary mantissa bits in the case of truncation
		if (v<0) v=0;
        expo=0;
	}

	*E = (UInt)expo;

	/* Convert a float number mant (0<=mant<1) into a binary representation N which is a mantissa with v bits */
	*M=0;
	int bit;

	for (i=1; i<=v; i++ ){
		bit = (2.0*mant >= 1.0);
		mant = 2.0*mant - (double)bit;
		*M = (*M << 1 ) | bit ; // MSB of M corresponds to 1/2 and MSB-1 to 1/4 and so on.
	}

	return 0;
}

ErrVal EncoderCodingParameter::xReadFromFile_MVAcquisitionInfo( std::string& rcFilename) // SEI JVT-W060, JVT-Z038
{
  
  std::string acTags[4];
 
  
  FILE *f = fopen( rcFilename.c_str(), "r");
  if( NULL == f )
  { 
    printf( "failed to open %s parameter file\n", rcFilename.c_str() );
    return Err::m_nERR;
  } 

	  
	   
  
  UInt NumViewsMinus1=0;
  int view_cnt=0;
  int j,cur_view_id=-1;
  Bool IntrinsicFlag=false;
  Bool ExtrinsicFlag=false;
  UInt PrecFocalLength=0;
  UInt PrecPrincipalPoint=0;
  UInt PrecRadialDistortion=0;
  UInt PrecRotationParam=0;
  UInt PrecTranslationParam=0;

  while (!feof(f))
  {
    RNOK( xReadLine( f, acTags ) );
    if ( acTags[0].empty() )
    {
      continue;
    }

	if (acTags[0] == "NumViewsMinus1") { // This line should come first in the cfg file
		AOF((NumViewsMinus1=atoi(acTags[1].c_str()))>=0)
		CodingParameter::initialize_memory(NumViewsMinus1+1);				
		continue;
	}

	if ( acTags[0]== "IntrinsicParameterFlag" ) {
		CodingParameter::setIntrinsicParamFlag(IntrinsicFlag=(Bool)(atoi(acTags[1].c_str())!=0));
		continue;
	}
		
	if ( acTags[0]== "IntrinsicParametersEqual" ) {
		CodingParameter::setIntrinsicParamsEqual((Bool)(atoi(acTags[1].c_str())!=0));
		continue;
	}

	if ( acTags[0]== "Precision_FocalLength" ) {
		CodingParameter::setPrecFocalLength(PrecFocalLength=atoi(acTags[1].c_str()));
		continue;
	}

	if ( acTags[0]== "Precision_PrincipalPoint" ) {
		CodingParameter::setPrecPrincipalPoint(PrecPrincipalPoint=atoi(acTags[1].c_str()));
		continue;
	}

	if ( acTags[0]== "Precision_RadialDistortion" ) {
		CodingParameter::setPrecRadialDistortion(PrecRadialDistortion=atoi(acTags[1].c_str()));
		continue;
	}

	if ( acTags[0]== "ExtrinsicParameterFlag" ) {
		CodingParameter::setExtrinsicParamFlag(ExtrinsicFlag=(Bool)(atoi(acTags[1].c_str())!=0));
		continue;
	}

	if ( acTags[0]== "Precision_RotationParam" ) {
		CodingParameter::setPrecRotationParam(PrecRotationParam=atoi(acTags[1].c_str()));
		continue;
	}

	if ( acTags[0]== "Precision_TranslationParam" ) {
		CodingParameter::setPrecTranslationParam(PrecTranslationParam=atoi(acTags[1].c_str()));
		continue;
	}

	if (acTags[0] == "View_ID") {
		AOF((cur_view_id=atoi(acTags[1].c_str()))>=0)
		AOF(cur_view_id<=CodingParameter::SpsMVC.getNumViewMinus1())
		AOF(++view_cnt<=(int)(CodingParameter::SpsMVC.getNumViewMinus1()+1))		
		continue;
	}

	UInt tmp_exponent, tmp_mantissa;
	if (IntrinsicFlag && cur_view_id!=-1)
	{
		if ( acTags[0]== "FocalLengthX" ) {
			CodingParameter::setSignFocalLengthX(cur_view_id,atof(acTags[1].c_str())<0.0);
			GetExponentMantissa_MVAcquisitionInfo(atof(acTags[1].c_str()), PrecFocalLength, &tmp_exponent, &tmp_mantissa);
			CodingParameter::setExponentFocalLengthX(cur_view_id, tmp_exponent);
			CodingParameter::setMantissaFocalLengthX(cur_view_id, tmp_mantissa);
			continue;
		}
		if ( acTags[0]== "FocalLengthY" ) {
			CodingParameter::setSignFocalLengthY(cur_view_id,atof(acTags[1].c_str())<0.0);
			GetExponentMantissa_MVAcquisitionInfo(atof(acTags[1].c_str()), PrecFocalLength, &tmp_exponent, &tmp_mantissa);
			CodingParameter::setExponentFocalLengthY(cur_view_id, tmp_exponent);
			CodingParameter::setMantissaFocalLengthY(cur_view_id, tmp_mantissa);
			continue;
		}
		if ( acTags[0]== "PrincipalPointX" ) {
			CodingParameter::setSignPrincipalPointX(cur_view_id,atof(acTags[1].c_str())<0.0);
			GetExponentMantissa_MVAcquisitionInfo(atof(acTags[1].c_str()), PrecPrincipalPoint, &tmp_exponent, &tmp_mantissa);
			CodingParameter::setExponentPrincipalPointX(cur_view_id, tmp_exponent);
			CodingParameter::setMantissaPrincipalPointX(cur_view_id, tmp_mantissa);
			continue;
		}
		if ( acTags[0]== "PrincipalPointY" ) {
			CodingParameter::setSignPrincipalPointY(cur_view_id,atof(acTags[1].c_str())<0.0);
			GetExponentMantissa_MVAcquisitionInfo(atof(acTags[1].c_str()), PrecPrincipalPoint, &tmp_exponent, &tmp_mantissa);
			CodingParameter::setExponentPrincipalPointY(cur_view_id, tmp_exponent);
			CodingParameter::setMantissaPrincipalPointY(cur_view_id, tmp_mantissa);
			continue;
		}
		if ( acTags[0]== "RadialDistortion" ) {
			CodingParameter::setSignRadialDistortion(cur_view_id,atof(acTags[1].c_str())<0.0);
			GetExponentMantissa_MVAcquisitionInfo(atof(acTags[1].c_str()), PrecRadialDistortion, &tmp_exponent, &tmp_mantissa);
			CodingParameter::setExponentRadialDistortion(cur_view_id, tmp_exponent);
			CodingParameter::setMantissaRadialDistortion(cur_view_id, tmp_mantissa);
			continue;
		}
	}
	
	
	if (ExtrinsicFlag && cur_view_id!=-1 )
	{
		if ( acTags[0]== "R_1" ) {
			for (j=0;j<3;j++) {
				CodingParameter::setSignRotationParam(cur_view_id,0,j,atof(acTags[j+1].c_str())<0.0);
				GetExponentMantissa_MVAcquisitionInfo(atof(acTags[j+1].c_str()), PrecRotationParam, &tmp_exponent, &tmp_mantissa);
				CodingParameter::setExponentRotationParam(cur_view_id,0,j, tmp_exponent);
				CodingParameter::setMantissaRotationParam(cur_view_id,0,j, tmp_mantissa);				
			}
			continue;
		}
		if ( acTags[0]== "R_2" ) {
			for (j=0;j<3;j++) {
				CodingParameter::setSignRotationParam(cur_view_id,1,j,atof(acTags[j+1].c_str())<0.0);
				GetExponentMantissa_MVAcquisitionInfo(atof(acTags[j+1].c_str()), PrecRotationParam, &tmp_exponent, &tmp_mantissa);
				CodingParameter::setExponentRotationParam(cur_view_id,1,j, tmp_exponent);
				CodingParameter::setMantissaRotationParam(cur_view_id,1,j, tmp_mantissa);				
			}
			continue;
		}
		if ( acTags[0]== "R_3" ) {
			for (j=0;j<3;j++) {
				CodingParameter::setSignRotationParam(cur_view_id,2,j,atof(acTags[j+1].c_str())<0.0);
				GetExponentMantissa_MVAcquisitionInfo(atof(acTags[j+1].c_str()), PrecRotationParam, &tmp_exponent, &tmp_mantissa);
				CodingParameter::setExponentRotationParam(cur_view_id,2,j, tmp_exponent);
				CodingParameter::setMantissaRotationParam(cur_view_id,2,j, tmp_mantissa);				
			}
			continue;
		}
		if ( acTags[0]== "Translation" ) {
			for (j=0;j<3;j++) {
				CodingParameter::setSignTranslationParam(cur_view_id,j,atof(acTags[j+1].c_str())<0.0);
				GetExponentMantissa_MVAcquisitionInfo(atof(acTags[j+1].c_str()), PrecTranslationParam, &tmp_exponent, &tmp_mantissa);
				CodingParameter::setExponentTranslationParam(cur_view_id,j, tmp_exponent);
				CodingParameter::setMantissaTranslationParam(cur_view_id,j, tmp_mantissa);				
			}
			continue;
		}

	}

  }


  fclose( f );

  

  return Err::m_nOK;
}





ErrVal EncoderCodingParameter::xReadLayerFromFile ( std::string&            rcFilename,
                                                    h264::LayerParameters&  rcLayer )
{
  std::string acTags[4];
  std::string cInputFilename, cOutputFilename, cMotionFilename, cESSFilename;

  //S051{
  std::string cEncSIPFilename;
  //S051}
  
  UInt        uiParLnCount = 0;
  
  FILE *f = fopen( rcFilename.c_str(), "r");
  if( NULL == f )
  { 
    printf( "failed to open %s layer config file\n", rcFilename.c_str() );
    return Err::m_nERR;
  } 

  //--ICU/ETRI FMO Implementation
  UInt bSliceGroupChangeDirection_flag=0;

  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceWidth",    &(rcLayer.m_uiFrameWidth),               176       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceHeight",   &(rcLayer.m_uiFrameHeight),              352       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("FrameRateIn",    &(rcLayer.m_dInputFrameRate),            30        );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("FrameRateOut",   &(rcLayer.m_dOutputFrameRate),           30        );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("InputFile",      &cInputFilename,                         "test.yuv");
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("ReconFile",      &cOutputFilename,                        "rec.yuv" );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SymbolMode",     &(rcLayer.m_uiEntropyCodingModeFlag),    1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("ClosedLoop",     &(rcLayer.m_uiClosedLoop),               0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FRExt",          &(rcLayer.m_uiAdaptiveTransform),        0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MaxDeltaQP",     &(rcLayer.m_uiMaxAbsDeltaQP),            1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("QP",             &(rcLayer.m_dBaseQpResidual),            32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("NumFGSLayers",   &(rcLayer.m_dNumFGSLayers),              0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQPLP",         &(rcLayer.m_dQpModeDecisionLP),          -1.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP0",          &(rcLayer.m_adQpModeDecision[0]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP1",          &(rcLayer.m_adQpModeDecision[1]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP2",          &(rcLayer.m_adQpModeDecision[2]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP3",          &(rcLayer.m_adQpModeDecision[3]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP4",          &(rcLayer.m_adQpModeDecision[4]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP5",          &(rcLayer.m_adQpModeDecision[5]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("InterLayerPred", &(rcLayer.m_uiInterLayerPredictionMode), 0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("BaseQuality",    &(rcLayer.m_uiBaseQualityLevel),         3         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MotionInfoMode", &(rcLayer.m_uiMotionInfoMode),           0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("MotionInfoFile", &cMotionFilename,                        "test.mot");
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("UseESS",         &(rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability), 0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("ESSPicParamFile",&cESSFilename,                                              "ess.dat" );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSCropWidth",   &(rcLayer.m_ResizeParameter.m_iOutWidth),                   0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSCropHeight",  &(rcLayer.m_ResizeParameter.m_iOutHeight),                  0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSOriginX",     &(rcLayer.m_ResizeParameter.m_iPosX),                       0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSOriginY",     &(rcLayer.m_ResizeParameter.m_iPosY),                       0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSChromaPhaseX",&(rcLayer.m_ResizeParameter.m_iChromaPhaseX),              -1         );  // SSUN, Nov2005
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSChromaPhaseY",&(rcLayer.m_ResizeParameter.m_iChromaPhaseY),               0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSBaseChromaPhaseX",&(rcLayer.m_ResizeParameter.m_iBaseChromaPhaseX),      -1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSBaseChromaPhaseY",&(rcLayer.m_ResizeParameter.m_iBaseChromaPhaseY),       0         );  // SSUN, Nov2005
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("ForceReOrdering",&(rcLayer.m_uiForceReorderingCommands),  0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("BaseLayerId",    &(rcLayer.m_uiBaseLayerId),              MSYS_UINT_MAX );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("EnhRefME",       &(rcLayer.m_dLowPassEnhRef),              AR_FGS_DEFAULT_LOW_PASS_ENH_REF );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("WeightZeroBlock",&(rcLayer.m_uiBaseWeightZeroBaseBlock),   AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_BLOCK   );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("WeightZeroCoeff",&(rcLayer.m_uiBaseWeightZeroBaseCoeff),   AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_COEFF   );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FgsEncStructure",&(rcLayer.m_uiFgsEncStructureFlag),   AR_FGS_DEFAULT_ENC_STRUCTURE   );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SliceMode",      &(rcLayer.m_uiSliceMode),                             0       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SliceArgument",  &(rcLayer.m_uiSliceArgument),                        50       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("NumSlicGrpMns1", &(rcLayer.m_uiNumSliceGroupsMinus1),                  0       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SlcGrpMapType",  &(rcLayer.m_uiSliceGroupMapType),                     2       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SlcGrpChgDrFlag",&(bSliceGroupChangeDirection_flag),         0       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SlcGrpChgRtMus1",&(rcLayer.m_uiSliceGroupChangeRateMinus1),           85       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("SlcGrpCfgFileNm",&rcLayer.m_cSliceGroupConfigFileName,             "sgcfg.cfg" );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("NumROI", &(rcLayer.m_uiNumROI),                  0       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr ("ROICfgFileNm",&rcLayer.m_cROIConfigFileName,             "roicfg.cfg" );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSMotion",      &(rcLayer.m_uiFGSMotionMode),							0		);
// JVT-Q065 EIDR{
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("IDRPeriod",	  &(rcLayer.m_iIDRPeriod),								0		);
// JVT-Q065 EIDR}
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt ("PLR",	          &(rcLayer.m_uiPLR),								0		); //JVT-R057 LA-RDO
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("UseRedundantSlc",&(rcLayer.m_uiUseRedundantSlice), 0   );  //JVT-Q054 Red. Picture
  
  //S051{
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineStr( "EncSIPFile", &cEncSIPFilename, ""); 
  //S051}

  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVectorMode", &(rcLayer.m_uiFGSCodingMode), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSGroupingSize", &(rcLayer.m_uiGroupingSize), 1 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector0", &(rcLayer.m_uiPosVect[0]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector1", &(rcLayer.m_uiPosVect[1]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector2", &(rcLayer.m_uiPosVect[2]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector3", &(rcLayer.m_uiPosVect[3]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector4", &(rcLayer.m_uiPosVect[4]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector5", &(rcLayer.m_uiPosVect[5]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector6", &(rcLayer.m_uiPosVect[6]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector7", &(rcLayer.m_uiPosVect[7]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector8", &(rcLayer.m_uiPosVect[8]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector9", &(rcLayer.m_uiPosVect[9]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector10", &(rcLayer.m_uiPosVect[10]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector11", &(rcLayer.m_uiPosVect[11]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector12", &(rcLayer.m_uiPosVect[12]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector13", &(rcLayer.m_uiPosVect[13]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector14", &(rcLayer.m_uiPosVect[14]), 0 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FGSVector15", &(rcLayer.m_uiPosVect[15]), 0 );
  m_pLayerLines[uiParLnCount] = NULL;

  while (!feof(f))
  {
    RNOK( xReadLine( f, acTags ) );
    if ( acTags[0].empty() )
    {
      continue;
    }
    for (UInt ui=0; m_pLayerLines[ui] != NULL; ui++)
    {
      if( acTags[0] == m_pLayerLines[ui]->getTag() )
      {
        m_pLayerLines[ui]->setVar( acTags[1] );
        break;
      }
    }
  }

  //S051{
        if(cEncSIPFilename.length())
        {
      	  rcLayer.setEncSIP(true);
      	  rcLayer.setInSIPFileName( (char*) cEncSIPFilename.c_str());
        }  
  //S051}

  rcLayer.setInputFilename     ( (Char*)cInputFilename.c_str() );
  rcLayer.setOutputFilename    ( (Char*)cOutputFilename.c_str() );
  rcLayer.setMotionInfoFilename( (Char*)cMotionFilename.c_str() );

  uiParLnCount = 0;
  while (m_pLayerLines[uiParLnCount] != NULL)
  {
    delete m_pLayerLines[uiParLnCount];
    m_pLayerLines[uiParLnCount] = NULL;
    uiParLnCount++;
  }

// TMM_ESS {
  // default values
  rcLayer.m_ResizeParameter.m_iInWidth    = rcLayer.m_uiFrameWidth;
  rcLayer.m_ResizeParameter.m_iInHeight   = rcLayer.m_uiFrameHeight;
  rcLayer.m_ResizeParameter.m_iGlobWidth  = rcLayer.m_uiFrameWidth;
  rcLayer.m_ResizeParameter.m_iGlobHeight = rcLayer.m_uiFrameHeight;
  rcLayer.m_ResizeParameter.m_bCrop       = false;
  if(rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability)  
  {
    rcLayer.m_ResizeParameter.m_bCrop = true;        
    if(rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability==2)
    {
      rcLayer.m_ResizeParameter.m_pParamFile = fopen( cESSFilename.c_str(), "r");
      if( NULL == rcLayer.m_ResizeParameter.m_pParamFile )
      { 
        printf( "failed to open resize parameter file %s\n", cESSFilename.c_str() );
        return Err::m_nERR;
      }
      rcLayer.m_ResizeParameter.m_iSpatialScalabilityType = SST_RATIO_X;
    }
  } else {
    // default values
    rcLayer.m_ResizeParameter.m_iOutWidth   = rcLayer.m_uiFrameWidth;
    rcLayer.m_ResizeParameter.m_iOutHeight  = rcLayer.m_uiFrameHeight;
    rcLayer.m_ResizeParameter.m_iPosX       = 0;
    rcLayer.m_ResizeParameter.m_iPosY       = 0;
    rcLayer.m_ResizeParameter.m_iBaseChromaPhaseX = rcLayer.m_ResizeParameter.m_iChromaPhaseX;  // SSUN, Nov2005
    rcLayer.m_ResizeParameter.m_iBaseChromaPhaseY = rcLayer.m_ResizeParameter.m_iChromaPhaseY;
  }
// TMM_ESS }

  //--ICU/ETRI FMO Implementation : FMO stuff start
  rcLayer.m_bSliceGroupChangeDirection_flag = ( bSliceGroupChangeDirection_flag != 0 );
  RNOK( xReadSliceGroupCfg( rcLayer)); //Slice group configuration file
  //--ICU/ETRI FMO Implementation : FMO stuff end

  // ROI Config ICU/ETRI
  RNOK( xReadROICfg( rcLayer)); 

  ::fclose(f);

  return Err::m_nOK;
}

ErrVal EncoderCodingParameter::xReadSliceGroupCfg( h264::LayerParameters&  rcLayer )
{
	UInt mapunit_height;
	UInt mb_height;
	UInt i;
	UInt mb_width;
 	FILE* sgfile=NULL;

	if( (rcLayer.getNumSliceGroupsMinus1()!=0)&&
		((rcLayer.getSliceGroupMapType() == 0) || (rcLayer.getSliceGroupMapType() == 2) || (rcLayer.getSliceGroupMapType() == 6)) )
	{ 
    if ( ! rcLayer.getSliceGroupConfigFileName().empty() &&
         ( sgfile = fopen( rcLayer.getSliceGroupConfigFileName().c_str(), "r" ) ) == NULL )
		{
      printf("Error open file %s", rcLayer.getSliceGroupConfigFileName().c_str() );
		}
		else
		{
			if (rcLayer.getSliceGroupMapType() == 0) 
			{
				for(i=0;i<=rcLayer.getNumSliceGroupsMinus1();i++)
				{
					fscanf(sgfile,"%d",(rcLayer.getArrayRunLengthMinus1()+i));
					fscanf(sgfile,"%*[^\n]");

				}
			}
			else if (rcLayer.getSliceGroupMapType() == 2)
			{
				// every two lines contain 'top_left' and 'bottom_right' value
				for(i=0;i<rcLayer.getNumSliceGroupsMinus1();i++)
				{
					fscanf(sgfile,"%d",(rcLayer.getArrayTopLeft()+i));
					fscanf(sgfile,"%*[^\n]");
					fscanf(sgfile,"%d",(rcLayer.getArrayBottomRight()+i));
					fscanf(sgfile,"%*[^\n]");
				}

			}
			else if (rcLayer.getSliceGroupMapType()== 6)
			{
				//--ICU/ETRI
				//TODO : currently map type 6 is partially supported 
				// Assume that only frame mode(no interlaced mode) is available
				// Assume that Frame cropping is not avaliable

				Int tmp;

				/*
				frame_mb_only = !(input->getPicInterlace() || input->getMbInterlace());
				mb_width= (input->get_img_width()+img->get_auto_crop_right())/16;
				mb_height= (input->get_img_height()+img->get_auto_crop_bottom())/16;
				mapunit_height=mb_height/(2-frame_mb_only);
				*/

				
				mb_width= (rcLayer.getFrameWidth())/16;
				mb_height= (rcLayer.getFrameHeight())/16;
				mapunit_height=mb_height;


				// each line contains slice_group_id for one Macroblock
				for (i=0;i<mapunit_height*mb_width;i++)
				{
					fscanf(sgfile,"%d", &tmp);
					//input->set_slice_group_id_ith( i, (unsigned) tmp);
					rcLayer.setSliceGroupId(i,(UInt)tmp);
					assert(*(rcLayer.getArraySliceGroupId()+i) <= rcLayer.getNumSliceGroupsMinus1() );
					fscanf(sgfile,"%*[^\n]");
				}

			}
			fclose(sgfile);

		}
	}
	return Err::m_nOK;

}


// ROI Config Read ICU/ETRI
ErrVal EncoderCodingParameter::xReadROICfg( h264::LayerParameters&  rcLayer )
{
	UInt i;
 	FILE* roifile=NULL;

	if ( (0 < rcLayer.getNumROI()) )
	{
		if ( ! rcLayer.getROIConfigFileName().empty() &&
         ( roifile = fopen( rcLayer.getROIConfigFileName().c_str(), "r" ) ) == NULL )
		{
			printf("Error open file %s", rcLayer.getROIConfigFileName().c_str() );
		}

		else
		{
			// every two lines contain 'top_left' and 'bottom_right' value
			for(i=0;i<rcLayer.getNumROI(); i++)
			{
				fscanf(roifile, "%d",(rcLayer.getROIID()+i));
				fscanf(roifile, "%*[^\n]");
				fscanf(roifile, "%d",(rcLayer.getSGID()+i));
				fscanf(roifile, "%*[^\n]");
				fscanf(roifile, "%d",(rcLayer.getSLID()+i));
				fscanf(roifile, "%*[^\n]");
			}

			fclose(roifile);
		}

	}
	
	
	return Err::m_nOK;
}

#endif // !defined(AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_)

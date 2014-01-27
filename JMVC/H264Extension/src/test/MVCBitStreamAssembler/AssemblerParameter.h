#if !defined(AFX_ASSEMBLERPARAMETER_H__79149AEA_06A8_49CE_AB0A_7FC9ED7C05B4__INCLUDED_)
#define AFX_ASSEMBLERPARAMETER_H__79149AEA_06A8_49CE_AB0A_7FC9ED7C05B4__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MVCBStreamAssembler.h"

#define MAX_CONFIG_PARAMS 256
//configLine codes are borrowed from the encoder cheny
//using namespace std;
class ConfigLineBase
{
protected:
  ConfigLineBase(const Char* pcTag, UInt uiType ) : m_cTag( pcTag ), m_uiType( uiType ) {}
  ConfigLineBase() {}
public:
  virtual ~ConfigLineBase() {}
  std::string&  getTag () { return m_cTag; }
  virtual Void  setVar ( std::string& rcValue ) = 0;
protected:
  std::string m_cTag;
  UInt m_uiType;
};

class ConfigLineStr : public ConfigLineBase
{
public:
  ConfigLineStr( const Char* pcTag, std::string* pcPar, const Char* pcDefault ) : ConfigLineBase( pcTag, 1 ), m_pcPar( pcPar )
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

class ConfigLineDbl : public ConfigLineBase
{
public:
  ConfigLineDbl( const Char* pcTag, Double* pdPar, Double pdDefault ) :  ConfigLineBase( pcTag, 2 ), m_pdPar( pdPar ) 
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

class ConfigLineInt : public ConfigLineBase
{
public:
  ConfigLineInt( const Char* pcTag, Int* piPar, Int piDefault ) : ConfigLineBase( pcTag, 3 ), m_piPar( piPar )
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

class ConfigLineUInt : public ConfigLineBase
{
public:
  ConfigLineUInt( const Char* pcTag, UInt* puiPar, UInt puiDefault ) : ConfigLineBase( pcTag, 4 ), m_puiPar( puiPar )
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

class ConfigLineChar : public ConfigLineBase
{
public:
  ConfigLineChar( const Char* pcTag, Char* pcPar, Char pcDefault ) : ConfigLineBase( pcTag, 5 ), m_pcPar( pcPar )
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


class AssemblerParameter  
{

public:
	AssemblerParameter          ();
	virtual ~AssemblerParameter ();

  const std::string&    getInFile           (UInt view_id)            const { return m_pcInFile[view_id];}
  const std::string&    getOutFile          ()                        const { return m_cOutFile;        }
  Int                   getResult           ()                        const { return m_iResult;         }
  
  UInt                  getNumViews         ()                        const { return m_uiNumViews;      }
  Void                  setNumViews         (UInt view)                     { m_uiNumViews = view;      }
  UInt                  getViewId           ()                        const { return m_uiViewId;        }
  Void                  setViewId           (UInt viewid)                   { m_uiViewId  = viewid;     }
  UInt                  getSuffix           ()                        const { return m_uiSuffix;        }
  Void                  setSuffix           (UInt uiSuffix)                 { m_uiSuffix  = uiSuffix;   }
  
	Void                  setOutFileName      (std::string strName)           { m_cOutFile = strName;     }
	std::string           getOutFileName      ()                        const { return m_cOutFile;        } 

  Bool                  getTraceEnabled     ()            const { return m_bTraceFile;      }
  Bool                  getAssemblerTrace   ()            const { return m_bTraceAssembler;   }
  const std::string&    getTraceFile        ()            const { return m_cTraceFile;      }
  const std::string&    getAssemblerTraceFile()           const { return m_cTraceAssmblerFile;   }

  

  Void    setResult           ( Int     iResult )   { m_iResult = iResult;  }
  ErrVal          init( Int     argc,
                        Char**  argv);

  ErrVal            xReadLine     ( FILE* hFile, std::string* pacTag );
  ErrVal            xReadFromFile ( std::string& rcFilename );

//  UInt getSuffixUnitEnable()                                    { return m_uiSuffixUnitEnable;}
  Bool    equals( const Char* str1, const Char* str2, UInt nLetter ) { return 0 == ::strncmp( str1, str2, nLetter); }
protected:
  ErrVal  xPrintUsage         ( Char**  argv );

protected:
  UInt            m_uiNumViews;
  std::string    *m_pcInFile;
  std::string     m_cOutFile;
  UInt            m_uiViewId;
  Int             m_iResult;


  Bool            m_bTraceFile;
  Bool            m_bTraceAssembler;
  UInt            m_uiSuffix; 
  std::string     m_cTraceFile;
  std::string     m_cTraceAssmblerFile;

  ConfigLineBase*    m_pCfgLines[MAX_CONFIG_PARAMS];
};

#endif // !defined(AFX_ASSEMBLERPARAMETER_H__79149AEA_06A8_49CE_AB0A_7FC9ED7C05B4__INCLUDED_)


#if !defined(AFX_EXTRACTORPARAMETER_H__79149AEA_06A8_49CE_AB0A_7FC9ED7C05B5__INCLUDED_)
#define AFX_EXTRACTORPARAMETER_H__79149AEA_06A8_49CE_AB0A_7FC9ED7C05B5__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class ExtractorParameter  
{
public:
  class Point
  {
  public:
    Point():uiWidth(0),uiHeight(0),dFrameRate(0.0),dBitRate(0.0)
    {
    }
    UInt    uiWidth;
    UInt    uiHeight;
    Double  dFrameRate;
    Double  dBitRate;

//--TEST DJ 0602
		UInt            uiROI[5];
  };

  //JVT-S043
  enum QLExtractionMode
  {
    QL_EXTRACTOR_MODE_ORDERED=0,
    QL_EXTRACTOR_MODE_JOINT
  };

public:
	ExtractorParameter          ();
	virtual ~ExtractorParameter ();

  const std::string&    getInFile           ()            const { return m_cInFile;         }
  const std::string&    getOutFile          ()            const { return m_cOutFile;        }
  Bool                  getAnalysisOnly     ()            const { return m_bAnalysisOnly;   }
  UInt				    getOpId				()			  const { return m_uiOpId; }


  ErrVal  init                ( Int     argc,
                                Char**  argv );


protected:
  ErrVal  xPrintUsage         ( Char**  argv );

protected:
  std::string     m_cInFile;
  std::string     m_cOutFile;
  Int             m_iResult;
  UInt			  m_uiOpId;
  Bool            m_bAnalysisOnly;
};

#endif // !defined(AFX_EXTRACTORPARAMETER_H__79149AEA_06A8_49CE_AB0A_7FC9ED7C05B5__INCLUDED_)


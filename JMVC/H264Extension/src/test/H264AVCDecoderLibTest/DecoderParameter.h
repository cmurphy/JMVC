#if !defined(AFX_DECODERPARAMETER_H__79149AEA_06A8_49CE_AB0A_7FC9ED7C05B5__INCLUDED_)
#define AFX_DECODERPARAMETER_H__79149AEA_06A8_49CE_AB0A_7FC9ED7C05B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// If max and min are defined by the preprocessor, STL stuff breaks so
// undefine them.
#undef max
#undef min
#include <vector>
#include "YUVFileParams.h"
// now redefine them
#define max(x,y) (((x) < (y)) ? (y) : (x))
#define min(x,y) (((x) < (y)) ? (x) : (y))
using namespace std;


class DecoderParameter  
{
public:
	DecoderParameter();
	virtual ~DecoderParameter();

  ErrVal init(int argc, char** argv);

  std::string  cBitstreamFile;
  std::string  cYuvFile;
  Int          nResult; 
  UInt         nFrames;
  UInt         uiMaxPocDiff; // HS: decoder robustness
  UInt				 uiErrorConceal;

  UInt         uiNumOfViews;
  UInt getNumOfViews() { return uiNumOfViews;}


  Bool    equals( const Char* str1, const Char* str2, UInt nLetter ) { return 0 == ::strncmp( str1, str2, nLetter); }
  std::vector <YUVFileParams> m_MultiviewReferenceFileParams;
  Int			vFramePeriod;

protected:

	
  ErrVal xPrintUsage(char** argv);
};



#endif // !defined(AFX_DECODERPARAMETER_H__79149AEA_06A8_49CE_AB0A_7FC9ED7C05B5__INCLUDED_)

#if !defined(AFX_WRITEYUVTOFILE_H__B25F5BA6_A016_4457_A617_0FE895AA14EC__INCLUDED_)
#define AFX_WRITEYUVTOFILE_H__B25F5BA6_A016_4457_A617_0FE895AA14EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "WriteYuvIf.h"
#include "LargeFile.h"

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


class H264AVCVIDEOIOLIB_API WriteYuvToFile
: public WriteYuvIf
{
protected:
	WriteYuvToFile();
	virtual ~WriteYuvToFile();

public:
  static ErrVal create( WriteYuvIf*& rpcWriteYuv, const std::string& rcFileName );
  ErrVal destroy();

  ErrVal writeFrame( const UChar *pLum,
                     const UChar *pCb,
                     const UChar *pCr,
                     UInt uiLumHeight,
                     UInt uiLumWidth,
                     UInt uiLumStride );

  static ErrVal createMVC( WriteYuvIf*& rpcWriteYuv, 
                           const std::string& rcFileName,
                           UInt numOfViews);

  ErrVal destroyMVC(UInt numOfViews);

  ErrVal writeFrame( const UChar *pLum,
                     const UChar *pCb,
                     const UChar *pCr,
                     UInt uiLumHeight,
                     UInt uiLumWidth,
                     UInt uiLumStride, 
                     //UInt uiViewId,
					 UInt ViewCnt);

protected:
  ErrVal xWriteFrame    ( const UChar *pLum, const UChar *pCb, const UChar *pCr,
                          UInt uiHeight, UInt uiWidth, UInt uiStride );

  ErrVal xInit( const std::string& rcFileName );

  ErrVal xWriteFrame    ( const UChar *pLum, const UChar *pCb, const UChar *pCr,
                          UInt uiHeight, UInt uiWidth, UInt uiStride, UInt ViewCnt );

  ErrVal xInitMVC( const std::string& rcFileName, UInt *vcOrder, 
                   UInt uiNumOfViews ); // JVT-AB024
  //JVT-V054
  Bool getFileInitDone() {return m_bFileInitDone;}

  ErrVal setCrop(UInt *uiCrop);

protected:
  LargeFile m_cFile;
  LargeFile *m_cFileMVC;

  Bool  m_bInitDone;
  BinData m_cTempBuffer;
  UInt  m_uiWidth;
  UInt  m_uiHeight;

//JVT-V054
  Bool  m_bFileInitDone;

  UInt m_uiCrop[4];

};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

#endif // !defined(AFX_WRITEYUVTOFILE_H__B25F5BA6_A016_4457_A617_0FE895AA14EC__INCLUDED_)

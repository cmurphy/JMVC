#include "YUVFileParams.h"
#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/CommonDefs.h"
#include <iostream>


YUVFileParams::YUVFileParams(const std::string& fileName, const int view_id,
			     const int height, const int vertPadding, 
				 const int width, const int horPadding)
	:_fileName(fileName),_view_id(view_id), _height(height), _width(width), 
     _lumaSize( (_height + vertPadding + 2*YUV_Y_MARGIN)*(_width + horPadding + 2*YUV_X_MARGIN) ),
     _bufSize( 3 * _lumaSize / 2 ),
     _lumaOffset( (_width + horPadding + 2*YUV_X_MARGIN) * YUV_Y_MARGIN + YUV_X_MARGIN ),
     _cbOffset( (((_width+horPadding)/2) + YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 
		+ _lumaSize ),
     _crOffset( (((_width+horPadding)/2) + YUV_X_MARGIN) * YUV_Y_MARGIN/2 
		+ YUV_X_MARGIN/2 + 5*_lumaSize/4 ),
     _stride( (_width+horPadding) + 2*YUV_X_MARGIN ) {
  if (0 == height || 0 == width) {
    std::cerr 
      << "Attempted to construct a YUVFileParams object with " << std::endl
      << "either height or width of 0.  Did you make sure to " << std::endl
      << "put the AddViewRef keywords in the config file     " << std::endl
      << "AFTER the SourceWidth and SourceHeight keywords.   " << std::endl;
    abort();
  }
 
}

YUVFileParams::YUVFileParams(const YUVFileParams& other) {
  _fileName = other._fileName;
  _view_id = other._view_id;
  _height = other._height;       
  _width = other._width;
  _lumaSize = other._lumaSize;
  _bufSize = other._bufSize;
  _lumaOffset = other._lumaOffset;
  _cbOffset = other._cbOffset;
  _crOffset = other._crOffset;
  _stride = other._stride;
}

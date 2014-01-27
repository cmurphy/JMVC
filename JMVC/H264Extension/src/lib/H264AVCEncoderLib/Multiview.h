
#ifndef INCLUDE_MULTIVIEW_DOT_H
#define INCLUDE_MULTIVIEW_DOT_H

// If max and min are defined by the preprocessor, STL stuff breaks so
// undefine them.
//#undef max
//#undef min
#include <vector>
#include <string>

#include "H264AVCVideoIoLib.h"
#include "H264AVCEncoderLib.h"
#include "H264AVCCommonLib.h"
#include "RecPicBuffer.h"
#include "SliceEncoder.h"
#include "ReadYuvFile.h"
#include "ReadBitstreamFile.h"
#include "WriteBitstreamToFile.h"
#include "YUVFileParams.h"
#include "RecPicBuffer.h"

using namespace std;

H264AVC_NAMESPACE_BEGIN


// Note, enum MultiviewReferenceDirection is defined in RecPicBuffer.h


// ----------------------------------------------------------------------
//
// CLASS:	MultiviewReferencePictureManager
//
// PURPOSE:	This is the main class that manages inserting/deleting
//		multiview references from the decoded picture buffer.  
//		First, you initialize a MultiviewReferencePictureManager 
//		by telling it which files you want to use for reference
//		using the AddViewFileToUseAsReference member function,
//		or the AddVectorOfFileFilesToUseAsReference member
//		function, and then inside the coding loop, you call
//		AddMultiviewReferencesPicturesToBuffer before coding
//		and call RemoveMultiviewReferencesPicturesFromBuffer
//		after coding.
//
// MODIFIED:	Tue Mar 14, 2006
//
// ----------------------------------------------------------------------

class MultiviewReferencePictureManager {
  class MultiviewReferenceInfo; // helper class (see below for details)
public:
  MultiviewReferencePictureManager();

  ~MultiviewReferencePictureManager();

  
  void AddViewFileToUseAsReference
  (const YUVFileParams& paramsForMultiviewReference);

  void AddVectorOfFilesToUseAsReference
  (const vector<YUVFileParams>& vectorOfReferenceFiles);
  
  void AddMultiviewReferencesPicturesToBuffer
  (RecPicBuffer* m_pcRecPicBuffer, SliceHeader* pcSliceHeader,
   PicBufferList& rcOutputList, PicBufferList& rcUnusedList, 
   const SequenceParameterSet & pcSPS, const int pictureOrderCount, const Bool IsAnchor);

  void RemoveMultiviewReferencesPicturesFromBuffer
  (RecPicBuffer* m_pcRecPicBuffer);

  int CountNumMultiviewReferenceStreams() const;

  bool                               _verbose;
protected:
  // The _references field contains a MultiviewReferenceInfo object
  // for each reference stream.  These tell the 
  // MultiviewReferencePictureManager what multiview references to
  // insert when AddMultiviewReferencesPicturesToBuffer is called.

  vector<MultiviewReferenceInfo*>    _references;

private:
  MultiviewReferencePictureManager
  (const MultiviewReferencePictureManager& other) {
    abort(); // copying not implemented yet
  }
};


// ----------------------------------------------------------------------
//
// CLASS:	MultiviewReferencePictureManager::MultiviewReferenceInfo
//
// PURPOSE:	This class is designed to collect together and hold 
//		the parameters for a multiview reference stream.  It
//		is internal to MultiviewReferencePictureManager and
//		probably does not need to be used outside of it.
//
// ----------------------------------------------------------------------

class MultiviewReferencePictureManager::MultiviewReferenceInfo {
public:
  
  MultiviewReferenceInfo
  (const YUVFileParams& fileParams, ReadYuvFile*const reader);
  
  ~MultiviewReferenceInfo();


public:
  MultiviewReferenceDirection _referenceDirection;
  YUVFileParams               _fileParams;
  ReadYuvFile*                _fileReader;
  vector<RecPicBufUnit*>      _referencePicsToRemove;
  
};


H264AVC_NAMESPACE_END

#endif



#include<stdio.h>
#include<stdlib.h>

#include "Multiview.h"


H264AVC_NAMESPACE_BEGIN

static void ShowFrameInfo(ostream & outputStream, const char*const msg,
			  const RecPicBufUnit* const bufUnit) {
  outputStream << msg << " with frameNumber and poc (" 
	       << bufUnit->getFrameNum() 
	       << ", " << bufUnit->getPoc() << ") " << endl;
}


// ----------------------------------------------------------------------
//
// FUNCTION:	MultiviewReferenceInfo::MultiviewReferenceInfo
//
// INPUTS:	refDirection:  The reference direction to use 
//			       when adding the references.  FORWARD
//			       references are added to list0 for P/B
//			       frames while BACKWARD references are
//			       added to list 1 only for B frames.
//
//		fileParams: A YUVFileParams object describing the
//			    parameters of an existing file which will
//			    be used as a reference.
//
//		reader:	A valid ReadYuvFile object that has been
//			properly opened and is ready to read frames
//			from the stream to use as a reference.
//
// PURPOSE:	Collect and hold parameters for a multiview reference
//		stream.
//
//----------------------------------------------------------------------


MultiviewReferencePictureManager::MultiviewReferenceInfo::MultiviewReferenceInfo(
 const YUVFileParams& fileParams, ReadYuvFile*const reader) 
  : _fileParams(fileParams), 
    _fileReader(reader) {
}

MultiviewReferencePictureManager::MultiviewReferenceInfo::~MultiviewReferenceInfo() {
  _fileReader->uninit();
  _fileReader->destroy();

  while (! _referencePicsToRemove.empty()) {
    _referencePicsToRemove.back()->uninit();
    _referencePicsToRemove.back()->destroy();
    _referencePicsToRemove.pop_back();
  }
}

// ----------------------------------------------------------------------
//
// FUNCTION:	ReadMultiviewReferencePictureFromStreamIntoPicBuffer
//
// INPUTS:	reader: Pointer to a ReadYuvFile object that has
//			already been properly initialized.
//
//		fileParams: A properly initialized fileParams object
//			    containing appropriate buffer offsets.
//
//		pictureOrderCount: An integer specifying which frame
//				   number to read (starting from 0).
//
//		verbose: A flag indicating whether to print debug info.
//
// RETURNS:	A new'ly allocated PicBuffer object containing the
//		desired frame.  It is the caller's responsibility to
//		delete the PicBuffer when finished.
//
// PURPOSE:	Read a frame from a YUV file into a new PicBuffer.
//
// MODIFIED:	Tue Mar 14, 2006
//
// ----------------------------------------------------------------------


PicBuffer* ReadMultiviewReferencePictureFromStreamIntoPicBuffer
(ReadYuvFile* reader, const YUVFileParams& fileParams, 
 const int pictureOrderCount, const bool verbose) {

//  if (verbose) 
 //   cout << "Going to read frame "<< pictureOrderCount <<" for multivew ref\n";

  UChar* buffer = new UChar[ fileParams._bufSize ];
  PicBuffer* newPicture = new PicBuffer(buffer);

  reader->GoToFrame(pictureOrderCount);
  if( ERR_CLASS::m_nOK != 
      reader->readFrame(buffer + fileParams._lumaOffset, 
			buffer + fileParams._cbOffset, 
			buffer + fileParams._crOffset,
			fileParams._height, fileParams._width, 
			fileParams._stride) ) {
    fprintf(stderr,"readFrame failed in %s:%i.\nAbort.\n",
	    __FILE__,__LINE__);
    fflush(stderr);
    fprintf(stdout,"readFrame failed in %s:%i.\nAbort.\n",
	    __FILE__,__LINE__);
    fflush(stdout);
    abort();
  }

  return newPicture;
}



MultiviewReferencePictureManager::MultiviewReferencePictureManager()
  :_verbose(true) {
}

MultiviewReferencePictureManager::~MultiviewReferencePictureManager() {

  while (! _references.empty() ) {
    delete _references.back();
    _references.pop_back();
  }

}

// ----------------------------------------------------------------------
//
// FUNCTION:	AddViewFileToUseAsReference
//
// INPUTS:	paramsForMultiviewReference:  A YUVFileParams object
//					      describing a multiview 
//					      reference file.
//
//		refDirection:  The reference direction to use 
//			       when adding the references.  FORWARD
//			       references are added to list0 for P/B
//			       frames while BACKWARD references are
//			       added to list 1 only for B frames.
//		
//
// PURPOSE:	Tell the MultiviewReferencePictureManager that it should
//		insert frames from the file specified in
//		paramsForMultiviewReference when the 
//		AddMultiviewReferencesPicturesToBuffer method is called.
//
//
// MODIFIED:	Tue Mar 14, 2006
//
// ----------------------------------------------------------------------


void MultiviewReferencePictureManager::AddViewFileToUseAsReference
(const YUVFileParams& paramsForMultiviewReference) {
  
  ReadYuvFile* newYUVReader;
  ReadYuvFile::create(newYUVReader);
  newYUVReader->init(paramsForMultiviewReference._fileName,
		     paramsForMultiviewReference._height, 
		     paramsForMultiviewReference._width, 0,
#if 0 // hwsun, seems not a bug (disable)
		     paramsForMultiviewReference._height, //paramsForMultiviewReference._width //lufeng: terrible bug fix
#else
			 paramsForMultiviewReference._width,
#endif
		     ReadYuvFile::FILL_FRAME);


   _references.push_back(new MultiviewReferenceInfo
			(paramsForMultiviewReference,
			 newYUVReader));

}

// ----------------------------------------------------------------------
//
// FUNCTION:	AddVectorOfFilesToUseAsReference
//
// INPUTS:	vectorOfReferenceFiles:  A vector of YUVFileParams
//					 objects describing files to use
//					 as references in multiview coding.
//
//		refDirection:  The reference direction to use 
//			       when adding the references.  FORWARD
//			       references are added to list0 for P/B
//			       frames while BACKWARD references are
//			       added to list 1 only for B frames.
//		
// PURPOSE:	When you have a vector of YUVFileParams you can use this
//		function ot add the references all at once instead of
//		having to call AddViewFileToUseAsReference yourself for
//		each item in the vector.
//
// MODIFIED:	Wed Mar 15, 2006
//
// ----------------------------------------------------------------------


void MultiviewReferencePictureManager::AddVectorOfFilesToUseAsReference
(const vector<YUVFileParams>& vectorOfReferenceFiles) {
  
  UInt i;
  
  for (i=0; i < vectorOfReferenceFiles.size(); i++) 
    AddViewFileToUseAsReference(vectorOfReferenceFiles[i]);
}


// ----------------------------------------------------------------------
//
// FUNCTION:	AddMultiviewReferencesPicturesToBuffer
//
// INPUTS:	pcRecPicBuffer:  
//		pcSliceHeader:
//		rcOutputList:
//		rcUnusedList:
//		pcSPS: 
//
//		    The above are all parameters provided by the 
//		    PicEncoder object that we want to insert multiview
//		    references into.  At this point, I don't know much
//		    more about the arguments than that.
//
//		pictureOrderCount:  The frame number to insert.
//
// PURPOSE:	This method goes through each file that has been
//		registered via AddViewFileToUseAsReference, reads in
//		frame number pictureOrderCount, and inserts that into
//		the prediction buffer.
//
// MODIFIED:	Wed Mar 15, 2006
//
// ----------------------------------------------------------------------


void MultiviewReferencePictureManager::AddMultiviewReferencesPicturesToBuffer
(RecPicBuffer* pcRecPicBuffer,  SliceHeader* pcSliceHeader,
 PicBufferList& rcOutputList, PicBufferList& rcUnusedList, 
 const SequenceParameterSet & pcSPS, const int pictureOrderCount, const Bool IsAnchor) {

  UInt i,kk;

//  if (_verbose) cout << endl << "Preparing to add multiview refs" << endl;

  for (i=0; i < _references.size(); i++) {

	  
	Bool used_for_ref =false;

	if (IsAnchor)	  
	{
		for ( kk=0; kk < pcSPS.SpsMVC->getNumAnchorRefsForListX(pcSliceHeader->getViewId(),0) ; kk++) // list0, anchor
			if ( _references[i]->_fileParams._view_id  == pcSPS.SpsMVC->getAnchorRefForListX(pcSliceHeader->getViewId(),kk,0) )
			{
				used_for_ref =true;
				_references[i]->_referenceDirection = FORWARD;		
			}
		for ( kk=0; kk < pcSPS.SpsMVC->getNumAnchorRefsForListX(pcSliceHeader->getViewId(),1) ; kk++) // list1, anchor
			if ( _references[i]->_fileParams._view_id  == pcSPS.SpsMVC->getAnchorRefForListX(pcSliceHeader->getViewId(),kk,1) )
			{
				used_for_ref =true;
				_references[i]->_referenceDirection = BACKWARD;		
			}
	}
	else
	{
		for ( kk=0; kk < pcSPS.SpsMVC->getNumNonAnchorRefsForListX(pcSliceHeader->getViewId(),0) ; kk++) // list0, non-anchor
			if ( _references[i]->_fileParams._view_id  == pcSPS.SpsMVC->getNonAnchorRefForListX(pcSliceHeader->getViewId(),kk,0) )
			{
				used_for_ref = true;
				_references[i]->_referenceDirection = FORWARD;		
			}
		for ( kk=0; kk < pcSPS.SpsMVC->getNumNonAnchorRefsForListX(pcSliceHeader->getViewId(),1) ; kk++) // list1, non-anchor
			if ( _references[i]->_fileParams._view_id  == pcSPS.SpsMVC->getNonAnchorRefForListX(pcSliceHeader->getViewId(),kk,1) )
			{
				used_for_ref = true;
				_references[i]->_referenceDirection = BACKWARD;		
			}
	}
	if (used_for_ref == false)
		continue;

    RecPicBufUnit* newRecPicBufUnit;
    
    PicBuffer* newPic=ReadMultiviewReferencePictureFromStreamIntoPicBuffer
      (_references[i]->_fileReader, _references[i]->_fileParams, 
       pictureOrderCount, _verbose);
    
    pcSliceHeader->setNalUnitType(NAL_UNIT_CODED_SLICE);
    pcSliceHeader->setNalRefIdc(NAL_REF_IDC_PRIORITY_LOW);
    pcSliceHeader->setInterViewRef(true);
    pcRecPicBuffer->initCurrRecPicBufUnit
      ( newRecPicBufUnit, newPic, pcSliceHeader, rcOutputList, rcUnusedList,
	_references[i]->_referenceDirection);
    
    pcRecPicBuffer->store( newRecPicBufUnit, pcSliceHeader, 
			   rcOutputList, rcUnusedList, 
			   _references[i]->_referenceDirection);
    
     assert( newRecPicBufUnit->isNeededForRef() );
    
     newRecPicBufUnit->setViewId(_references[i]->_fileParams._view_id);
     newRecPicBufUnit->getRecFrame()->setViewId(_references[i]->_fileParams._view_id);

    _references[i]->_referencePicsToRemove.push_back(newRecPicBufUnit);
    
//    if (_verbose) ShowFrameInfo(cout,"Added frame", newRecPicBufUnit);
    
	delete [] newPic->getBuffer();
	delete newPic;
  }

  

 }

int MultiviewReferencePictureManager::CountNumMultiviewReferenceStreams()const
{
  UInt count=0;

  while (count < _references.size()) {
    count++;
  }

  return count;
}

// ----------------------------------------------------------------------
//
// FUNCTION:	RemoveMultiviewReferencesPicturesFromBuffer
//
// PURPOSE:	Remove the multiview reference pictures we have
//		inserted.
//
// MODIFIED:	Wed Mar 15, 2006
//
// ----------------------------------------------------------------------


void MultiviewReferencePictureManager::RemoveMultiviewReferencesPicturesFromBuffer(RecPicBuffer* pcRecPicBuffer) {

//  if (_verbose) cout << endl << "Preparing to remove multiview refs" << endl;

  for (UInt i=0; i < _references.size(); i++) {
    while (! _references[i]->_referencePicsToRemove.empty()) {
      RecPicBufUnit* unitToRemove = 
	_references[i]->_referencePicsToRemove.back();

//      if (_verbose) ShowFrameInfo(cout,"going to remove frame",unitToRemove);

      assert( unitToRemove->isNeededForRef() );
      assert( unitToRemove->IsMultiviewReference() );
      unitToRemove->markNonRef();
      unitToRemove->markOutputted();
      pcRecPicBuffer->RemoveMultiviewRef(unitToRemove);
      _references[i]->_referencePicsToRemove.pop_back();
    }
  }
}

H264AVC_NAMESPACE_END

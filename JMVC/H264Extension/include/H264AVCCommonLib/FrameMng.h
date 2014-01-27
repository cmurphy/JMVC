#if !defined(AFX_FRAMEMNG_H__FCFD4695_2766_4D95_BFD2_B2496827BC03__INCLUDED_)
#define AFX_FRAMEMNG_H__FCFD4695_2766_4D95_BFD2_B2496827BC03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/FrameUnit.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include <algorithm>
#include <list>



H264AVC_NAMESPACE_BEGIN

class QuarterPelFilter;

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API FrameMng
{
  enum OutputMode
  {
    OM_BUMPING = 0,
    OM_DELAYED,
    OM_DELAYED_LOCKED,
    OM_FLUSH
  };

  class FUList : public std::list<FrameUnit*>
  {
  public:
    FrameUnit* popBack()  { FrameUnit* pcFU = back();   pop_back();   return pcFU; }
    FrameUnit* popFront() { FrameUnit* pcFU = front();  pop_front();  return pcFU; }
// big clean up, and fix, add view id for all list getting. ying

  Void setRefPicList( RefPicList<RefPic>& rcRefPicList , SliceHeader& rcSH)
  {
    for( iterator iter = begin(); iter != end(); iter++ )
    {
        if( (*iter)->isUsed(FRAME) && (*iter)->getPicBuffer() && rcSH.getViewId() == (*iter)->getFrame().getViewId() ) 
	    {
		    rcRefPicList.next().setFrame( &( (*iter)->getFrame() ) );
	    }
    }
  }

  Void setRefPictureList( RefPicList<Frame*>& rcRefFrameList, SliceHeader& rcSH) const
  {
      for( const_iterator iter = begin(); iter != end(); iter++ )
      {
          if( (*iter)->isUsed( FRAME ) && (*iter)->getPicBuffer() && rcSH.getViewId() == (*iter)->getFrame().getViewId() )
          {
              rcRefFrameList.add( &( (*iter)->getFrame() ) );
          }
      }
  }

  Void setRefFrameList( RefPicList<Frame*>& rcRefFrameList , SliceHeader& rcSH) 
  {
    for( iterator iter = begin(); iter != end(); iter++ )
    {
        if( (*iter)->isUsed(FRAME) && (*iter)->getPicBuffer() && rcSH.getViewId() == (*iter)->getFrame().getViewId() ) 
      {
         rcRefFrameList.add( &( (*iter)->getFrame() ) );
      }
    }
  }

    Void setRefFrameUnitList( RefPicList<FrameUnit*>& rcRefFrameUnitList, SliceHeader& rcSH )
    {
      for( iterator iter = begin(); iter != end(); iter++ )
      {
		  FrameUnit* pcFU = (*iter);
          if( pcFU->isUsed(FRAME) && (*iter)->getPicBuffer() && rcSH.getViewId() == (*iter)->getFrame().getViewId() ) 
        {
            rcRefFrameUnitList.add( *iter );
        }
      }
    }

    iterator findShortTerm( UInt uiPicNum )
    {
      for( iterator iter = begin(); iter != end(); iter++ )
      {
				FrameUnit* pcFU = (*iter);
        if( pcFU->getFrameNumber() == uiPicNum )
        {
          return iter;
        }
      }
      return end();
    }
  

            iterator findShortTerm_iter( FrameUnit *FrameU )
            {
              for( iterator iter = begin(); iter != end(); iter++ )
              {
                if( (*iter) == FrameU )
                {
                  return iter;
                }
              }
              return end();
            }

            iterator findShortTermMVC( UInt uiPicNum, UInt uiViewId )
            {
              for( iterator iter = begin(); iter != end(); iter++ )
              {
                  FrameUnit* pcFU = (*iter);
                  if(( pcFU->getFrameNumber() == uiPicNum) && (pcFU->getFrame().getViewId() == uiViewId) )
                {
                  return iter;
                }
              }
              return end();
            }

//JVT-V043  {{
            iterator findInterView(SliceHeader& rcSH, UInt uiViewIdx, ListIdx eListIdx )
            {
              Bool bAnchor      = rcSH.getAnchorPicFlag();
              UInt targetViewId = rcSH.getSPS().getSpsMVC()->getViewIDByViewIndex( rcSH.getViewId(), uiViewIdx, eListIdx, bAnchor );
      
              for( iterator iter = begin(); iter != end(); iter++ )
              {
				FrameUnit* pcFU = (*iter);
			     if( (pcFU->getFrame().getViewId() == targetViewId && 
					 rcSH.getPoc() == pcFU->getFrame(rcSH.getPicType()).getPOC() ) )//SH get frame poc

                {
                  return iter;
                }
              }
              return end();
            }

            Void setRefFrameListFGSMVCView( RefPicList<Frame*>& rcRefFrameList, 
                                                 SliceHeader& rcSH, enum ListIdx eListIdx ) // memory 
            {
              Bool bAnchor =rcSH.getAnchorPicFlag() ;
              UInt n = rcSH.getSPS().getSpsMVC()->getNumRefsForListX(rcSH.getViewId(), eListIdx, bAnchor);
                      
              for( UInt ui = 0; ui < n; ui++)
              {
                for( iterator iter = begin(); iter != end(); iter++ )
                {
                  FrameUnit* pcFU = (*iter);
                  if( pcFU->isUsed(FRAME) && !pcFU->getBaseRep() ) //JVT-S036 lsj 
                  {
                    if((/*pcFU->getFGSPicBuffer() || */pcFU->getPicBuffer()) && 
                      rcSH.getSPS().getSpsMVC()->getViewIDByViewIndex(rcSH.getViewId(), ui, eListIdx, bAnchor)== pcFU->getFrame().getViewId() 
					  && rcSH.getPoc() == pcFU->getFrame(rcSH.getPicType()).getPOC() && pcFU->getFrame().getInterViewFlag()) //JVT-W056 
                    {
                      rcRefFrameList.add(  & pcFU->getFrame()  ); //				  rcRefFrameList.add(  &( pcFU->getFGSPicBuffer() ? pcFU->getFGSFrame() : pcFU->getFrame() ) );
                    }
                  }
                }
              }        
            }

            Void setRefFrameListFGSMVCViewNonRef(RefPicList<Frame*>& rcRefFrameList, 
                                                 SliceHeader& rcSH, enum ListIdx eListIdx ) //memory 
            {
              Bool bAnchor =rcSH.getAnchorPicFlag() ;
              UInt n = rcSH.getSPS().getSpsMVC()->getNumRefsForListX(rcSH.getViewId(), eListIdx, bAnchor);

              for( UInt ui = 0; ui < n; ui++)
              {
                for( iterator iter = begin(); iter != end(); iter++ )
                {
                  FrameUnit* pcFU = (*iter);
                  if((/*pcFU->getFGSPicBuffer() || */pcFU->getPicBuffer()) && 
                     rcSH.getSPS().getSpsMVC()->getViewIDByViewIndex(rcSH.getViewId(), ui, eListIdx, bAnchor) == pcFU->getFrame().getViewId() 
					 && rcSH.getPoc() == pcFU->getFrame(rcSH.getPicType()).getPOC() && pcFU->getFrame().getInterViewFlag() )  
                  {
                    //rcRefFrameList.add(  &( pcFU->getFGSPicBuffer() ? pcFU->getFGSFrame() : pcFU->getFrame() ) );
                    rcRefFrameList.add(  & pcFU->getFrame() ); // memory
                  }
                }
              }        
            }
//JVT-V043  }}

            iterator find ( const FrameUnit* & rcT ) 
            {
                return std::find( begin(), end(), rcT );
            }
          };

  typedef FUList::iterator          FUIter;
  typedef FUList::reverse_iterator  FURIter;

  class FrameUnitBuffer
  {
  public:
    FrameUnitBuffer();
    ~FrameUnitBuffer();
    ErrVal init( YuvBufferCtrl* pcYuvFullPelBufferCtrl, YuvBufferCtrl* pcYuvHalfPelBufferCtrl );
    ErrVal uninit();
    ErrVal getFrameUnit( FrameUnit*& rpcFrameUnit );
    ErrVal releaseFrameUnit( FrameUnit* pcFrameUnit );
    YuvBufferCtrl* getYuvFullPelBufferCtrl()  { return m_pcYuvFullPelBufferCtrl;  }

  protected:
    FUList            m_cFreeList;
    YuvBufferCtrl*    m_pcYuvFullPelBufferCtrl;
    YuvBufferCtrl*    m_pcYuvHalfPelBufferCtrl;
  };

  class PocOrder
  {
  public:
    Int operator() ( const Frame* pcFrame1, const Frame* pcFrame2 )
    {
      return pcFrame1->getPOC() < pcFrame2->getPOC();
    }
  };

protected:
  FrameMng              ();
	virtual ~FrameMng     ();

public:

          Int getMaxEntriesinDPB(); // hwsun, fix meomory for field coding

          FrameUnit*  getCurrentFrameUnit   () { return m_pcCurrentFrameUnit; }
          IntFrame*   getRefinementIntFrame () { return m_pcRefinementIntFrame; }
          IntFrame*   getRefinementIntFrame2() { return m_pcRefinementIntFrameSpatial; }
          IntFrame*   getPredictionIntFrame()  { return m_pcPredictionIntFrame; }
  
          ErrVal			xSlidingWindowUpdateBase    ( UInt mCurrFrameNum); //JVT-S036 
          ErrVal			xMMCOUpdateBase				( SliceHeader* rcSH ); //JVT-S036 

//          ErrVal initSlice( SliceHeader *rcSH );
          ErrVal initSlice( SliceHeader *rcSH , UInt NumOfViewsInTheStream);
          
          ErrVal initSPS( const SequenceParameterSet& rcSPS );


          ErrVal initFrame( SliceHeader& rcSH, PicBuffer* pcPicBuffer );

  
          ErrVal initPic( SliceHeader& rcSH );

          static  ErrVal  create          ( FrameMng*& rpcFrameMng );
          static  UInt    MaxRefFrames    ( UInt uiLevel, UInt uiNumMbs );
          ErrVal storePicture( const SliceHeader& rcSH );

//  ErrVal storeFGSPicture( PicBuffer* pcPicBuffer );
  ErrVal setRefPicLists( SliceHeader& rcSH, Bool bDoNotRemap );
  ErrVal  destroy                 ();
  ErrVal  init                    ( YuvBufferCtrl* pcYuvFullPelBufferCtrl, YuvBufferCtrl* pcYuvHalfPelBufferCtrl = NULL, QuarterPelFilter* pcQuarterPelFilter = NULL );
  ErrVal  uninit                  ();
  ErrVal  RefreshOrederedPOCList  (); //JVT-S036 
  ErrVal  setPicBufferLists       ( PicBufferList& rcPicBufferOutputList, PicBufferList& rcPicBufferUnusedList );
  ErrVal  outputAll               ();

  ErrVal  getRecYuvBuffer         ( YuvPicBuffer*& rpcRecYuvBuffer, PicType ePicType );
  FUList& getShortTermList        ()  { return m_cShortTermList; }

  FrameUnit*        getReconstructedFrameUnit( Int iPoc );

  YuvBufferCtrl*    getYuvFullPelBufferCtrl() { return m_cFrameUnitBuffer.getYuvFullPelBufferCtrl();  }
  //void    SetCodeAsVFrameFlag(const bool flag) { m_codeAsVFrame = flag; }
  //bool    CodeAsVFrameP() const {return m_codeAsVFrame; }


protected:
    ErrVal            xStoreShortTerm             ( FrameUnit* pcFrameUnit );
    ErrVal            xStoreNonRef                ( FrameUnit* pcFrameUnit );
    ErrVal            xStoreInOrderedPocList      ( FrameUnit* pcFrameUnit );
    Void              xSetIdentifier              ( UInt& uiNum, 
        PicType& rePicType, 
        const PicType eCurrPicType);
    ErrVal            xOutputPicture              ( FrameUnit* pcFrameUnit );
  ErrVal            xCheckMissingFrameNums( SliceHeader& rcSH );

  ErrVal            xSetReferenceLists          ( SliceHeader& rcSH );
  ErrVal            xSetReferenceListsMVC       ( SliceHeader& rcSH );
  ErrVal            xClearListsIDR              ( const SliceHeader& rcSH );
  ErrVal            xManageMemory               ( const SliceHeader& rcSH );
  ErrVal            xSlidingWindowUpdate        ();
  ErrVal            xStoreCurrentPicture        ( const SliceHeader& rcSH );                // MMCO 6
  ErrVal            xReferenceListRemapping     ( SliceHeader& rcSH, ListIdx eListIdx );
  ErrVal            xMmcoMarkShortTermAsUnused  ( const PicType eCurrPicType, const FrameUnit* pcCurrFrameUnit, UInt uiDiffOfPicNums );
  ErrVal            xMmcoMarkShortTermAsUnusedBase( const PicType eCurrPicType, const FrameUnit* pcCurrFrameUnit, UInt uiDiffOfPicNums ); //JVT-S036 

          ErrVal            xSetOutputListMVC              ( FrameUnit* pcFrameUnit, UInt uiNumOfViews );
	      ErrVal            xSetOutputListMVC              ( FrameUnit* pcFrameUnit, const SliceHeader& rcSH );		

          private:
          UInt              xSortPocOrderedList                 (RefPicList<Frame*,64>& rcRefPicFrameList, Int iCurrPoc);
          ErrVal            xSetInitialReferenceListPFrame      ( SliceHeader& rcSH );
          ErrVal            xSetInitialReferenceListBFrame      ( SliceHeader& rcSH );

          ErrVal            xSetInitialReferenceListPFields     ( SliceHeader& rcSH );
          ErrVal            xSetInitialReferenceListBFields     ( SliceHeader& rcSH );
          ErrVal            xSetInitialReferenceFieldList       ( SliceHeader& rcSH, ListIdx eListIdx );
          ErrVal            xSetMbaffFieldLists                 ( SliceHeader& rcSH, ListIdx eListIdx );

          __inline ErrVal   xRemoveFromRefList( FUList& rcFUList, FUIter iter );
          __inline ErrVal   xRemoveFromRefList( FUList& rcFUList );
          __inline ErrVal   xRemove           ( FrameUnit* pcFrameUnit );
          __inline ErrVal   xAddToFreeList    ( FrameUnit* pcFrameUnit );
          __inline ErrVal   xAddToFreeList    ( FUList& rcFUList );
          __inline Bool     xFindAndErase     ( FUList& rcFUList, FrameUnit* pcFrameUnit );

          ErrVal            xMmcoMarkShortTermAsUnusedMVC( const PicType eCurrPicType, const FrameUnit* pcCurrFrameUnit, 
              UInt uiDiffOfPicNums, UInt uiCurrViewId );
          ErrVal            xDumpRefList( ListIdx eListIdx, SliceHeader& rcSH );
          ErrVal            xSetBFrameListMVC ( SliceHeader& rcSH); 
          ErrVal            xSetPFrameListMVC ( SliceHeader& rcSH); 
private:
  Bool              m_bInitDone;
  QuarterPelFilter* m_pcQuarterPelFilter;
  UInt              m_uiPrecedingRefFrameNum;

  PicBufferList     m_cPicBufferOutputList;
  PicBufferList     m_cPicBufferUnusedList;
  FrameUnit*        m_pcOriginalFrameUnit;
  FrameUnit*        m_pcCurrentFrameUnit;
  FrameUnit*        m_pcCurrentViewFrameUnit[8];//lufeng: temp buf for undone frame of every view in field decoding
  FrameUnit*		m_pcCurrentFrameUnitBase; //JVT-S036 

  RefPicList<FrameUnit*> m_acTmpShortTermRefList[2];

  RefPicList<Frame*>     m_cPocOrderedFrameList;

  UInt               m_iEntriesInDPB;
  UInt               m_iMaxEntriesinDPB;

  UInt              m_uiNumRefFrames;
  UInt              m_uiMaxFrameNumCurr;
  UInt              m_uiMaxFrameNumPrev;
  FUList            m_cShortTermList;
  FUList            m_cNonRefList;
  FUList            m_cOrderedPOCList;

  FrameUnitBuffer   m_cFrameUnitBuffer;

  static UInt       m_uiDBPMemory[256];
  IntFrame*         m_pcRefinementIntFrame;
  IntFrame*         m_pcRefinementIntFrameSpatial;
  IntFrame*         m_pcPredictionIntFrame;
  Bool                m_codeAsVFrame;
  UInt              m_uiLastViewId;
        };

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif



H264AVC_NAMESPACE_END


#endif // !defined(AFX_FRAMEMNG_H__FCFD4695_2766_4D95_BFD2_B2496827BC03__INCLUDED_)

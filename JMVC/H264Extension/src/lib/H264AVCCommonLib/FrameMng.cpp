#include <cstdio>
#include "H264AVCCommonLib.h"

#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/FrameMng.h"



H264AVC_NAMESPACE_BEGIN



UInt FrameMng:: m_uiDBPMemory[256 ]  =
{
  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,152064 ,345600 ,912384 ,912384 ,0 ,0 ,0 ,0 ,0 ,0 ,912384 ,1824768 ,3110400 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,3110400 ,6912000 ,7864320 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,12582912 ,12582912 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,41656320 ,70778880 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0
};


FrameMng::FrameUnitBuffer::FrameUnitBuffer()
 :  m_pcYuvFullPelBufferCtrl( NULL )
 ,  m_pcYuvHalfPelBufferCtrl( NULL )
{
}

FrameMng::FrameUnitBuffer::~FrameUnitBuffer()
{
  AOF_DBG( m_cFreeList.empty() );
}

ErrVal FrameMng::FrameUnitBuffer::init( YuvBufferCtrl* pcYuvFullPelBufferCtrl, YuvBufferCtrl* pcYuvHalfPelBufferCtrl )
{
  ROT( NULL == pcYuvFullPelBufferCtrl );
  if( NULL == pcYuvHalfPelBufferCtrl )
  {
    m_pcYuvHalfPelBufferCtrl = pcYuvFullPelBufferCtrl;
  }
  else
  {
    m_pcYuvHalfPelBufferCtrl = pcYuvHalfPelBufferCtrl;
  }
  m_pcYuvFullPelBufferCtrl = pcYuvFullPelBufferCtrl;
  return Err::m_nOK;
}

ErrVal FrameMng::FrameUnitBuffer::uninit()
{
  FUIter iter;
  for( iter = m_cFreeList.begin(); iter != m_cFreeList.end(); iter++ )
  {
    RNOK( (*iter)->destroy() );
  }
  m_cFreeList.clear();

  m_pcYuvFullPelBufferCtrl = NULL;
  m_pcYuvHalfPelBufferCtrl = NULL;
  return Err::m_nOK;
}

ErrVal FrameMng::FrameUnitBuffer::getFrameUnit( FrameUnit*& rpcFrameUnit )
{
  if( m_cFreeList.empty() )
  {
    return FrameUnit::create( rpcFrameUnit, *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl );
  }

  rpcFrameUnit = m_cFreeList.popBack();
  return Err::m_nOK;
}

ErrVal FrameMng::FrameUnitBuffer::releaseFrameUnit( FrameUnit* pcFrameUnit )
{
  AOT_DBG( NULL == pcFrameUnit );
  m_cFreeList.push_back( pcFrameUnit );
  return Err::m_nOK;
}


FrameMng::FrameMng()
: m_bInitDone               ( false )
, m_pcQuarterPelFilter      ( NULL )
, m_pcOriginalFrameUnit     ( NULL )
, m_pcCurrentFrameUnit      ( NULL )
, m_codeAsVFrame (false)
, m_uiLastViewId (0)
{
  m_uiPrecedingRefFrameNum  = 0;
  m_iEntriesInDPB           = 0;
  m_iMaxEntriesinDPB        = 0;
  m_uiNumRefFrames          = 0;
  m_uiMaxFrameNumCurr       = 0;
  m_uiMaxFrameNumPrev       = 0;
  m_pcRefinementIntFrame    = 0;
  m_pcRefinementIntFrameSpatial = 0;
  m_pcPredictionIntFrame    = 0;
  for (UInt i=0;i<8;i++)
  {  
      m_pcCurrentViewFrameUnit[i]=NULL;
}
}

FrameMng::~FrameMng()
{
}

Int FrameMng::getMaxEntriesinDPB() { return m_iMaxEntriesinDPB;} // hwsun, fix meomory for field coding

ErrVal FrameMng::create( FrameMng*& rpcFrameMng )
{
  rpcFrameMng = new FrameMng;

  ROT( NULL == rpcFrameMng );
  return Err::m_nOK;
}


ErrVal FrameMng::destroy()
{
  delete this;
  return Err::m_nOK;
}


__inline Bool FrameMng::xFindAndErase( FUList& rcFUList, FrameUnit* pcFrameUnit )
{
  FUIter iter = std::find( rcFUList.begin(), rcFUList.end(), pcFrameUnit );

  ROTRS( rcFUList.end() == iter, false );

  rcFUList.erase( iter );
  return true;
}


__inline ErrVal FrameMng::xAddToFreeList( FrameUnit* pcFrameUnit )
{
  if( pcFrameUnit->getPicBuffer() )
  {
    pcFrameUnit->getPicBuffer()->setUnused();
    if( ! pcFrameUnit->getPicBuffer()->isUsed() )
    {
      m_cPicBufferUnusedList.push_back( pcFrameUnit->getPicBuffer() );
    }
  }


  RNOK( pcFrameUnit->uninit() );
  RNOK( m_cFrameUnitBuffer.releaseFrameUnit( pcFrameUnit ) );

  m_iEntriesInDPB--;
  AOT_DBG( m_iEntriesInDPB < 0 );
  return Err::m_nOK;
}


__inline ErrVal FrameMng::xAddToFreeList( FUList& rcFUList )
{
  for( FUIter iter = rcFUList.begin(); iter != rcFUList.end(); iter++ )
  {
    RNOK( xAddToFreeList( *iter ) );
  }
  rcFUList.clear();
  return Err::m_nOK;
}

__inline ErrVal FrameMng::xRemove( FrameUnit* pcFrameUnit )
{
    pcFrameUnit->setUnused( FRAME );

    if( pcFrameUnit->isOutputDone() )
    {
        RNOK( xAddToFreeList( pcFrameUnit ) );
        return Err::m_nOK;
    }

    m_cNonRefList.push_back( pcFrameUnit );
    return Err::m_nOK;
}


__inline ErrVal FrameMng::xRemoveFromRefList( FUList& rcFUList, FUIter iter )
{
  ROTRS( iter == rcFUList.end() ,Err::m_nOK );
  xRemove( *iter );
  rcFUList.erase( iter );
  return Err::m_nOK;
}


__inline ErrVal FrameMng::xRemoveFromRefList( FUList& rcFUList )
{
  for( FUIter iter = rcFUList.begin(); iter != rcFUList.end(); iter++ )
  {
    RNOK( xRemove( *iter ) );
  }
  rcFUList.clear();
  return Err::m_nOK;
}

//JVT-S036 lsj
ErrVal FrameMng::init( YuvBufferCtrl* pcYuvFullPelBufferCtrl, YuvBufferCtrl* pcYuvHalfPelBufferCtrl, QuarterPelFilter* pcQuarterPelFilter )
{

  ROT( m_bInitDone )

  m_uiPrecedingRefFrameNum  = 0;
  m_iEntriesInDPB           = 0;
  m_iMaxEntriesinDPB        = 0;
  m_uiNumRefFrames          = 0;
  m_uiMaxFrameNumCurr       = 0;
  m_uiMaxFrameNumPrev       = 0;
  m_bInitDone               = false;
  m_pcQuarterPelFilter      = NULL;
  m_pcOriginalFrameUnit     = NULL;
  m_pcCurrentFrameUnit      = NULL;
  m_pcCurrentFrameUnitBase  = NULL; //JVT-S036 
  m_codeAsVFrame			= false;	 


  m_pcQuarterPelFilter = pcQuarterPelFilter;

  RNOK( m_cFrameUnitBuffer.init( pcYuvFullPelBufferCtrl, pcYuvHalfPelBufferCtrl ) );
  RNOK( FrameUnit::create( m_pcOriginalFrameUnit, *pcYuvFullPelBufferCtrl, *pcYuvFullPelBufferCtrl, true ) );

  if( m_pcRefinementIntFrame == 0)
  {
    ROF( m_pcRefinementIntFrame = new IntFrame( *pcYuvFullPelBufferCtrl, *pcYuvFullPelBufferCtrl ) );
  }
  if( m_pcRefinementIntFrameSpatial == 0)
  {
    ROF( m_pcRefinementIntFrameSpatial = new IntFrame( *pcYuvFullPelBufferCtrl, *pcYuvFullPelBufferCtrl ) );
  }
  if( m_pcPredictionIntFrame == 0)
  {
    ROF( m_pcPredictionIntFrame = new IntFrame( *pcYuvFullPelBufferCtrl, *pcYuvFullPelBufferCtrl ) );
  }

  m_bInitDone = true;

  return Err::m_nOK;
}


ErrVal FrameMng::uninit()
{
  if( NULL != m_pcRefinementIntFrame )
  {
    m_pcRefinementIntFrame->uninit();
    delete m_pcRefinementIntFrame;
    m_pcRefinementIntFrame = NULL;
  }

  if( NULL != m_pcRefinementIntFrameSpatial )
  {
    m_pcRefinementIntFrameSpatial->uninit();
    delete m_pcRefinementIntFrameSpatial;
    m_pcRefinementIntFrameSpatial = NULL;
  }

  if( NULL != m_pcPredictionIntFrame )
  {
    m_pcPredictionIntFrame->uninit();
    delete m_pcPredictionIntFrame;
    m_pcPredictionIntFrame = NULL;
  }

  if( NULL != m_pcOriginalFrameUnit )
  {
    RNOK( m_pcOriginalFrameUnit->uninit() );
    RNOK( m_pcOriginalFrameUnit->destroy() );
    m_pcOriginalFrameUnit = NULL;
  }

  RNOK( m_cFrameUnitBuffer.uninit() );

  m_uiPrecedingRefFrameNum  = 0;
  m_iEntriesInDPB           = 0;
  m_iMaxEntriesinDPB        = 0;
  m_uiNumRefFrames          = 0;
  m_uiMaxFrameNumCurr       = 0;
  m_uiMaxFrameNumPrev       = 0;
  m_bInitDone               = false;
  m_pcQuarterPelFilter      = NULL;
  m_pcOriginalFrameUnit     = NULL;
  m_pcCurrentFrameUnit      = NULL;
  m_pcCurrentFrameUnitBase  = NULL; //JVT-S036 
  m_codeAsVFrame			= false;	 



  AOT( ! m_cShortTermList.empty() );
  AOT( ! m_cNonRefList.empty() );
  AOT( ! m_cOrderedPOCList.empty() );
  AOT( ! m_cPicBufferUnusedList.empty() );
  AOT( ! m_cPicBufferOutputList.empty() );

  m_cShortTermList.clear();
  m_cNonRefList.clear();
  m_cOrderedPOCList.clear();
  m_cPicBufferUnusedList.clear();
  m_cPicBufferOutputList.clear();

  return Err::m_nOK;
}

//JVT-S036 {
ErrVal FrameMng::RefreshOrederedPOCList()
{
  //===== store reference in ordered POC List =====
  FUIter  iter;
  for( iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
  {
    if( (*iter)->getMaxPOC() == m_pcCurrentFrameUnit->getMaxPOC() )
    {
      break;
    }
  }

  (*iter) = m_pcCurrentFrameUnit;

	return Err::m_nOK;
}
//JVT-S036 }

//ErrVal FrameMng::initSlice( SliceHeader *rcSH )
ErrVal FrameMng::initSlice( SliceHeader *rcSH , UInt NumOfViewsInTheStream)
{
   m_uiMaxFrameNumCurr = ( 1 << ( rcSH->getSPS().getLog2MaxFrameNum() ) );
  m_uiMaxFrameNumPrev = ( 1 << ( rcSH->getSPS().getLog2MaxFrameNum() ) );
  m_uiNumRefFrames    = rcSH->getSPS().getNumRefFrames();

  
  
  UInt Num_Views=1;
  //if (rcSH->getSPS().SpsMVC!=NULL)
  if ((rcSH->getSPS().SpsMVC!=NULL) && NumOfViewsInTheStream>1)
	  Num_Views = rcSH->getSPS().SpsMVC->getNumViewMinus1()+1;
  UInt mvcScaleFactor = Num_Views > 1 ? 2 : 1;

  m_iMaxEntriesinDPB = rcSH->getSPS().getMaxDPBSize(mvcScaleFactor);
#if REDUCE_MAX_FRM_DPB 
  //m_iMaxEntriesinDPB = min ( mvcScaleFactor*m_iMaxEntriesinDPB , (max(1,(UInt)ceil((double)log((double)Num_Views)/log(2.)))*16) );
  m_iMaxEntriesinDPB = min ( m_iMaxEntriesinDPB , (max(1,(UInt)ceil((double)log((double)Num_Views)/log(2.)))*16) );
#endif
  
  printf("MaxNumRefFrames=%d NumViews=%d dec DPB-size=%d\n",m_uiNumRefFrames,Num_Views,	m_iMaxEntriesinDPB);

  if( ! m_iMaxEntriesinDPB )
  {
    printf("WARNING: Size of Decoded picture buffer is less than 1 frame!");
  }

  if( rcSH->getSPS().getNalUnitType() == NAL_UNIT_CODED_SLICE || rcSH->getSPS().getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR)
  {
	if( m_pcRefinementIntFrame )
	{
		RNOK( m_pcRefinementIntFrame->init( false ) );
	}
	if( m_pcRefinementIntFrameSpatial )
	{
		RNOK( m_pcRefinementIntFrameSpatial->init( false ) );
	}
	if( m_pcPredictionIntFrame )
	{
		RNOK( m_pcPredictionIntFrame->init( false ) );
	}
  }

  return Err::m_nOK;
}

ErrVal FrameMng::initSPS( const SequenceParameterSet& rcSPS )
{
  m_uiMaxFrameNumCurr = ( 1 << ( rcSPS.getLog2MaxFrameNum() ) );
  m_uiMaxFrameNumPrev = ( 1 << ( rcSPS.getLog2MaxFrameNum() ) );
  m_uiNumRefFrames    = rcSPS.getNumRefFrames();

  if( rcSPS.getProfileIdc() == MULTI_VIEW_PROFILE || rcSPS.getProfileIdc() == STEREO_HIGH_PROFILE )
  {
    
	UInt Num_Views=1;
	if (rcSPS.SpsMVC!=NULL)
	  Num_Views = rcSPS.SpsMVC->getNumViewMinus1()+1;
	UInt mvcScaleFactor = Num_Views > 1 ? 2 : 1;

	m_iMaxEntriesinDPB = rcSPS.getMaxDPBSize(mvcScaleFactor);
	//m_iMaxEntriesinDPB = min ( mvcScaleFactor*m_iMaxEntriesinDPB , (max(1,(UInt)ceil((double)log((double)Num_Views)/log(2.)))*16) );
	m_iMaxEntriesinDPB = min ( m_iMaxEntriesinDPB , (max(1,(UInt)ceil((double)log((double)Num_Views)/log(2.)))*16) );
	

  }
  else
  {
    m_iMaxEntriesinDPB= min( 48, rcSPS.getMaxDPBSize(1) + 3 );
  }

  if( ! m_iMaxEntriesinDPB )
  {
    printf("WARNING: Size of Decoded picture buffer is less than 1 frame!");
  }

  if( m_pcRefinementIntFrame )
  {
    RNOK( m_pcRefinementIntFrame->init( false ) );
  }
  if( m_pcRefinementIntFrameSpatial )
  {
    RNOK( m_pcRefinementIntFrameSpatial->init( false ) );
  }
  if( m_pcPredictionIntFrame )
  {
    RNOK( m_pcPredictionIntFrame->init( false ) );
  }

  return Err::m_nOK;
}

ErrVal FrameMng::initPic( SliceHeader& rcSH )
{
    rcSH.setFrameUnit( m_pcCurrentFrameUnit );

    PicType eCurrPicType = rcSH.getPicType();

    if( NOT_SPECIFIED == m_pcCurrentFrameUnit->getAvailableStatus() )
    {
        m_pcCurrentFrameUnit->setFrameNumber( rcSH.getFrameNum() );
    }
    //===== init POC =====
    if( eCurrPicType & TOP_FIELD )
    {
        m_pcCurrentFrameUnit->setTopFieldPoc( rcSH.getTopFieldPoc() );
    }
    if( eCurrPicType & BOT_FIELD )
    {
        m_pcCurrentFrameUnit->setBotFieldPoc( rcSH.getBotFieldPoc() );
    }

    return Err::m_nOK;
}

//

ErrVal FrameMng::initFrame( SliceHeader& rcSH, PicBuffer* pcPicBuffer )
{
  ROF( m_bInitDone );

  //===== check frame numbers for reference pictures =====

  if( ! rcSH.isIdrNalUnit() )
  {
    RNOK( xCheckMissingFrameNums( rcSH ) );
  }

  if ( !m_pcCurrentViewFrameUnit[rcSH.getViewId()] 
        || m_pcCurrentViewFrameUnit[rcSH.getViewId()] ->getMbDataCtrl()->isFrameDone( rcSH )//lufeng: alloc new buf for new frame
			|| !m_pcCurrentViewFrameUnit[rcSH.getViewId()] ->getMbDataCtrl()->isInitDone()
			)
  {

      RNOK( m_cFrameUnitBuffer.getFrameUnit( m_pcCurrentViewFrameUnit[rcSH.getViewId()] ) );
      m_pcCurrentFrameUnit = m_pcCurrentViewFrameUnit[rcSH.getViewId()];//lufeng: different buf temps for views
      RNOK( m_pcCurrentFrameUnit->init( rcSH, pcPicBuffer ) );


      m_pcCurrentFrameUnit->getFrame().setViewId( rcSH.getViewId() );

      m_pcCurrentFrameUnit->getFrame().setInterViewFlag(rcSH.getInterViewFlag()); //JVT-W056	

      rcSH.setFrameUnit( m_pcCurrentFrameUnit );
  }
  else
  {
      m_pcCurrentFrameUnit = m_pcCurrentViewFrameUnit[rcSH.getViewId()];//lufeng: continue last incomplete view
  }

  return Err::m_nOK;
}

ErrVal FrameMng::xStoreInOrderedPocList( FrameUnit* pcFrameUnit )
{
    //===== store reference in ordered POC List =====
    const Int iMaxPoc = pcFrameUnit->getMaxPOC();
    FUIter  iter;
    for( iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
    {
        if( (*iter)->getMaxPOC() > iMaxPoc )
        {
            break;
        }
    }
    m_cOrderedPOCList.insert( iter, pcFrameUnit );

    m_iEntriesInDPB++;

    return Err::m_nOK;
}

ErrVal FrameMng::xStoreShortTerm( FrameUnit* pcFrameUnit )
{
    ROTRS( m_cShortTermList.size() && (m_cShortTermList.front() == pcFrameUnit), Err::m_nOK );
    ROTRS( m_cShortTermList.find( (const FrameUnit*&) pcFrameUnit) != m_cShortTermList.end() , Err::m_nOK );//lufeng: first field has been stored

    m_cShortTermList.push_front( pcFrameUnit );

    // this might happen if the corresponding field is marked
    // as unused for reference
    FUIter iterNR = m_cNonRefList.find( (const FrameUnit*&)pcFrameUnit );
    if( iterNR != m_cNonRefList.end() && (pcFrameUnit == *iterNR) )
    {
        m_cNonRefList.erase( iterNR );
    }
    else
    {
        RNOK( xStoreInOrderedPocList( pcFrameUnit ) );
    }

    AOT_DBG( m_cNonRefList.  end() != m_cNonRefList.  find( (const FrameUnit*&)pcFrameUnit ) );

    return Err::m_nOK;
}

ErrVal FrameMng::xStoreNonRef( FrameUnit* pcFrameUnit )
{
    ROTRS( m_cNonRefList.size() && m_cNonRefList.back() == pcFrameUnit, Err::m_nOK );
	ROTRS( m_cNonRefList.find( (const FrameUnit*&) pcFrameUnit) != m_cNonRefList.end() , Err::m_nOK );//lufeng: first field has been stored

    m_cNonRefList.push_back( pcFrameUnit );
    RNOK( xStoreInOrderedPocList( pcFrameUnit ) );

    AOT_DBG( m_cShortTermList.end() != m_cShortTermList.find( (const FrameUnit*&)pcFrameUnit ) );

    return Err::m_nOK;
}

Void FrameMng::xSetIdentifier( UInt& uiNum, PicType& rePicType, const PicType eCurrPicType)
{
    if( eCurrPicType == FRAME )
    {
        rePicType = FRAME;
    }
    else
    {
        if( uiNum % 2 ) 
        {
            rePicType = eCurrPicType;
        }
        else if( eCurrPicType == TOP_FIELD )
        {
            rePicType = BOT_FIELD;
        }
        else
        {
            rePicType = TOP_FIELD;
        }
        uiNum /= 2;
    }
}


ErrVal FrameMng::xCheckMissingFrameNums( SliceHeader& rcSH )
{
	//===== check frame numbers for reference pictures =====
  if( ( ( m_uiPrecedingRefFrameNum + 1 ) % m_uiMaxFrameNumCurr) != rcSH.getFrameNum() && 
      (m_uiLastViewId == rcSH.getViewId()) )
  {
    UInt  uiNumMissingPictures = rcSH.getFrameNum() - m_uiPrecedingRefFrameNum - 1;

    if( rcSH.getFrameNum() <= m_uiPrecedingRefFrameNum )
    {
      uiNumMissingPictures += m_uiMaxFrameNumCurr;
    }

    if( rcSH.getSPS().getRequiredFrameNumUpdateBehaviourFlag() )
    {
      for( UInt uiIndex = 1; uiIndex <= uiNumMissingPictures; uiIndex++ )
      {
        UInt        uiFrameNum  = ( m_uiPrecedingRefFrameNum + uiIndex ) % m_uiMaxFrameNumCurr;
        FrameUnit*  pcFrameUnit = 0;
        RNOK( m_cFrameUnitBuffer.getFrameUnit( pcFrameUnit ) );
		//JVT-S036 {
		if( !m_pcCurrentFrameUnit->getBaseRep() )
		{
			FUList::iterator iter = m_cShortTermList.begin();
			FUList::iterator end  = m_cShortTermList.end();
			Bool bFlag = false;
			for( ; iter != m_cShortTermList.end(); iter++ )
			{
				if( (*iter)->getBaseRep() && (*iter)->getFrameNumber() == m_pcCurrentFrameUnit->getFrameNumber())
				{
				    bFlag = true;
					break;
				}
			}
			if( bFlag )
			{
					FrameUnit* pcFrameUnitTemp = (*iter);
					RNOK(pcFrameUnit->init( rcSH, *pcFrameUnitTemp ));
			}
			else
			{
				RNOK( pcFrameUnit->init( rcSH, *m_pcCurrentFrameUnit ) );
			}
			
		}
		else
		//JVT-S036 }
		{
			RNOK( pcFrameUnit->init( rcSH, *m_pcCurrentFrameUnit ) ); // HS: decoder robustness
		}

        pcFrameUnit->setFrameNumber( uiFrameNum );
        m_cShortTermList.push_front( pcFrameUnit );
        m_iEntriesInDPB++;

		RNOK( xSlidingWindowUpdate() );

      }
    }
    else
    {
      printf("\n  LOST PICTURES: %d\n", uiNumMissingPictures );
      AF();
    }

    m_uiPrecedingRefFrameNum = ( m_uiPrecedingRefFrameNum + uiNumMissingPictures ) % m_uiMaxFrameNumCurr;
  }
  return Err::m_nOK;
}



ErrVal FrameMng::setPicBufferLists( PicBufferList& rcPicBufferOutputList, PicBufferList& rcPicBufferUnusedList )
{
  rcPicBufferUnusedList += m_cPicBufferUnusedList;
  m_cPicBufferUnusedList.clear();

  rcPicBufferOutputList += m_cPicBufferOutputList;
  m_cPicBufferOutputList.clear();

  return Err::m_nOK;
}


ErrVal FrameMng::storePicture( const SliceHeader& rcSH )
{
  //===== memory managment =====
#if JM_MVC_COMPATIBLE
  if( rcSH.getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR)//lufeng: clear DPB at one time
#else 
  if( rcSH.isIdrNalUnit() ) 
#endif
  {
    RNOK( xClearListsIDR( rcSH ) );
  }
  RNOK( xManageMemory( rcSH ) );

  //===== store current picture =====
  RNOK( xStoreCurrentPicture( rcSH ) );

  //===== set pictures for output =====
  RNOK( xSetOutputListMVC( m_pcCurrentFrameUnit, rcSH) );


  if( rcSH.getNalRefIdc() )
  {
    m_uiPrecedingRefFrameNum = m_pcCurrentFrameUnit->getFrameNumber();
  }

  return Err::m_nOK;
}


UInt FrameMng::xSortPocOrderedList( RefPicList<Frame*,64>& rcRefPicFrameList, Int iCurrPoc )
{
    UInt  uiFirstPosWithGreaterPoc;

    std::sort( rcRefPicFrameList.begin(), rcRefPicFrameList.end(), Frame::PocOrder() );
    for( uiFirstPosWithGreaterPoc = 0; uiFirstPosWithGreaterPoc < rcRefPicFrameList.size(); uiFirstPosWithGreaterPoc++ )
    {
        if( m_cPocOrderedFrameList.get( uiFirstPosWithGreaterPoc )->getPOC() > iCurrPoc )
        {
            break;
        }
    }
    return uiFirstPosWithGreaterPoc;
}

ErrVal FrameMng::xSetInitialReferenceListPFrame( SliceHeader& rcSH )
{
    RefPicList<RefPic>& rcList = rcSH.getRefPicList( FRAME, LIST_0 );
    m_cShortTermList.setRefPicList( rcList, rcSH);
  return Err::m_nOK;
}



ErrVal FrameMng::xSetInitialReferenceListBFrame( SliceHeader& rcSH )
{
  RefPicList<RefPic>& rcList0 = rcSH.getRefPicList( FRAME, LIST_0 );
  RefPicList<RefPic>& rcList1 = rcSH.getRefPicList( FRAME, LIST_1 );
  UInt                uiFirstPosWithGreaterPoc, uiPos;
  FUIter              iter;

  //====== set Poc ordered short-term list and get index with smallest Poc greater than current ======
  m_cPocOrderedFrameList.reset();
  m_cShortTermList.setRefFrameList( m_cPocOrderedFrameList , rcSH);

  PicType eCurrPicType = rcSH.getPicType();
  uiFirstPosWithGreaterPoc = xSortPocOrderedList( m_cPocOrderedFrameList, m_pcCurrentFrameUnit->getPic( eCurrPicType ).getPOC() );

  //===== set short term reference frames =====
  for( uiPos = uiFirstPosWithGreaterPoc - 1; uiPos != MSYS_UINT_MAX; uiPos-- )
  {
    rcList0.next().setFrame( m_cPocOrderedFrameList.get( uiPos ) );
  }
  for( uiPos = uiFirstPosWithGreaterPoc; uiPos != m_cPocOrderedFrameList.size(); uiPos++ )
  {
    rcList0.next().setFrame( m_cPocOrderedFrameList.get( uiPos ) );
    rcList1.next().setFrame( m_cPocOrderedFrameList.get( uiPos ) );
  }
  for( uiPos = uiFirstPosWithGreaterPoc - 1; uiPos != MSYS_UINT_MAX; uiPos-- )
  {
    rcList1.next().setFrame( m_cPocOrderedFrameList.get( uiPos ) );
  }

  return Err::m_nOK;
}

ErrVal FrameMng::xSetInitialReferenceListPFields( SliceHeader& rcSH )
{
    //----- initial frame list for short term pictures -----
    m_acTmpShortTermRefList[LIST_0].reset();
    getShortTermList().setRefFrameUnitList( m_acTmpShortTermRefList[LIST_0], rcSH );

    RNOK( xSetInitialReferenceFieldList( rcSH, LIST_0 ) );

    return Err::m_nOK;
}

ErrVal FrameMng::xSetInitialReferenceListBFields( SliceHeader& rcSH )
{
    UInt    uiFirstPosWithGreaterPoc, uiPos;

    //====== set Poc ordered short-term list and get index with smallest Poc greater than current ======
    m_cPocOrderedFrameList.reset();
    getShortTermList().setRefPictureList( m_cPocOrderedFrameList, rcSH );

    PicType eCurrPicType = rcSH.getPicType();
    uiFirstPosWithGreaterPoc = xSortPocOrderedList( m_cPocOrderedFrameList, m_pcCurrentFrameUnit->getPic( eCurrPicType ).getPOC() );

    //----- initial frame list for short term pictures -----
    m_acTmpShortTermRefList[LIST_0].reset();
    m_acTmpShortTermRefList[LIST_1].reset();
    for( uiPos = uiFirstPosWithGreaterPoc - 1; uiPos != MSYS_UINT_MAX; uiPos-- )
    {
        m_acTmpShortTermRefList[LIST_0].add( m_cPocOrderedFrameList.get( uiPos )->getFrameUnit() );
    }
    for( uiPos = uiFirstPosWithGreaterPoc; uiPos != m_cPocOrderedFrameList.size(); uiPos++ )
    {
        m_acTmpShortTermRefList[LIST_0].add( m_cPocOrderedFrameList.get( uiPos )->getFrameUnit() );
        m_acTmpShortTermRefList[LIST_1].add( m_cPocOrderedFrameList.get( uiPos )->getFrameUnit() );
    }
    for( uiPos = uiFirstPosWithGreaterPoc - 1; uiPos != MSYS_UINT_MAX; uiPos-- )
    {
        m_acTmpShortTermRefList[LIST_1].add( m_cPocOrderedFrameList.get( uiPos )->getFrameUnit() );
    }

    RNOK( xSetInitialReferenceFieldList( rcSH, LIST_0 ) );
    RNOK( xSetInitialReferenceFieldList( rcSH, LIST_1 ) );

    return Err::m_nOK;
}

ErrVal FrameMng::xSetInitialReferenceFieldList( SliceHeader& rcSH, ListIdx eListIdx )
{

    PicType               eCurrPicType      = rcSH.getPicType();
    PicType               eOppositePicture  = ( eCurrPicType == TOP_FIELD ? BOT_FIELD : TOP_FIELD );
    RefPicList<FrameUnit*>& rcShortList       = m_acTmpShortTermRefList          [eListIdx];
    RefPicList<RefPic>&     rcList            = rcSH.getRefPicList( eCurrPicType, eListIdx );
    FrameUnit*              pcFU;

    ROTRS( rcList.full(), Err::m_nOK );

    //----- initialize field list for short term pictures -----
    UInt  uiCurrentParityIndex  = 0;
    UInt  uiOppositeParityIndex = 0;

    while( uiCurrentParityIndex < rcShortList.size() || uiOppositeParityIndex < rcShortList.size() )
    {
        //--- current parity ---
        while( uiCurrentParityIndex < rcShortList.size() )
        {
            pcFU = rcShortList.get( uiCurrentParityIndex++ );

            if( pcFU->isUsed( eCurrPicType ) )
            {
                //if(! rcSH.getKeyPictureFlag() && pcFU->getFGSPicBuffer() )
                //    rcList.next().setFrame( &( pcFU->getFGSPic( eCurrPicType ) ) );
                //else
                    rcList.next().setFrame( &( pcFU->getPic( eCurrPicType ) ) );
                break;
            }
        }
        //--- opposite parity ---
        while( uiOppositeParityIndex < rcShortList.size() )
        {
            pcFU = rcShortList.get( uiOppositeParityIndex++ );

            if( pcFU->isUsed( eOppositePicture ) 
                && pcFU->getFrame().getViewId()==rcSH.getViewId()//only use same field type for interview ref
                )
            {
                //if(! rcSH.getKeyPictureFlag() && pcFU->getFGSPicBuffer() )
                //    rcList.next().setFrame( &( pcFU->getFGSPic( eOppositePicture ) ) );
                //else
                    rcList.next().setFrame( &( pcFU->getPic( eOppositePicture ) ) );
                break;
            }
        }
    }

    return Err::m_nOK;
}

ErrVal FrameMng::xSetMbaffFieldLists( SliceHeader& rcSH, ListIdx eListIdx )
{
    RefPicList<RefPic>& rcFrameList   = rcSH.getRefPicList(     FRAME, eListIdx );
    RefPicList<RefPic>& rcTopFldList  = rcSH.getRefPicList( TOP_FIELD, eListIdx );
    RefPicList<RefPic>& rcBotFldList  = rcSH.getRefPicList( BOT_FIELD, eListIdx );

    //===== generate list for field macroblocks =====
    for( UInt uiFrmIdx = 0; uiFrmIdx < rcFrameList.size(); uiFrmIdx++ )
    {
        //th
        const Frame* pcFrame = rcFrameList.get( uiFrmIdx ).getFrame();
        const Frame* pcTopFld = NULL;
        const Frame* pcBotFld = NULL;

        if( pcFrame == &rcFrameList.get( uiFrmIdx ).getFrame()->getFrameUnit()->getFrame() )
        {
            pcTopFld = &( pcFrame->getFrameUnit()->getTopField() );
            pcBotFld = &( pcFrame->getFrameUnit()->getBotField() );
        }
     
        //th
        //    const Frame*  pcTopFld = &( rcFrameList.get( uiFrmIdx ).getFrame()->getFrameUnit()->getTopField() );
        //    const Frame*  pcBotFld = &( rcFrameList.get( uiFrmIdx ).getFrame()->getFrameUnit()->getBotField() );

        rcTopFldList.next().setFrame( pcTopFld );
        rcTopFldList.next().setFrame( pcBotFld );

        rcBotFldList.next().setFrame( pcBotFld );
        rcBotFldList.next().setFrame( pcTopFld );
    }
    return Err::m_nOK;
}

ErrVal FrameMng::setRefPicLists( SliceHeader& rcSH, Bool bDoNotRemap )
{
  RNOK( xSetReferenceLists( rcSH) );

  if(rcSH.getNalUnitType() == NAL_UNIT_CODED_SLICE_SCALABLE)
    xSetReferenceListsMVC( rcSH );

  if( ! bDoNotRemap )
  {
    //===== remapping =====
    RNOK( xReferenceListRemapping( rcSH, LIST_0 ) );
    RNOK( xReferenceListRemapping( rcSH, LIST_1 ) );
  }

// cleanup //dump
#if 1
  if( rcSH.isInterP() )
  {
    xDumpRefList( LIST_0, rcSH );
  }
  else if (rcSH.isInterB())
  {
    xDumpRefList( LIST_0, rcSH );
    xDumpRefList( LIST_1, rcSH );
  }
#endif

	PicType eCurrPicType = rcSH.getPicType();
  //===== MBAFF field lists =====
  if( eCurrPicType == FRAME && rcSH.getSPS().getMbAdaptiveFrameFieldFlag() )
  {
    RNOK( xSetMbaffFieldLists( rcSH, LIST_0 ) );
    RNOK( xSetMbaffFieldLists( rcSH, LIST_1 ) );
  }

  return Err::m_nOK;
}



ErrVal FrameMng::xSetReferenceLists( SliceHeader& rcSH )
{
  rcSH.getRefPicList( FRAME    , LIST_0 ).reset( 0 );
  rcSH.getRefPicList( FRAME    , LIST_1 ).reset( 0 );
  rcSH.getRefPicList( TOP_FIELD, LIST_0 ).reset( 0 );
  rcSH.getRefPicList( TOP_FIELD, LIST_1 ).reset( 0 );
  rcSH.getRefPicList( BOT_FIELD, LIST_0 ).reset( 0 );
  rcSH.getRefPicList( BOT_FIELD, LIST_1 ).reset( 0 );

  if( rcSH.isIntra() )
  {
    return Err::m_nOK;
  }

  PicType eCurrPicType = rcSH.getPicType();
  rcSH.getRefPicList( eCurrPicType, LIST_0 ).reset();
  if( eCurrPicType == FRAME && rcSH.getSPS().getMbAdaptiveFrameFieldFlag() )
  {
      rcSH.getRefPicList( TOP_FIELD, LIST_0 ).reset();
      rcSH.getRefPicList( BOT_FIELD, LIST_0 ).reset();
  }

  if( rcSH.isInterB() )
  {
      rcSH.getRefPicList( eCurrPicType, LIST_1 ).reset();
      if( eCurrPicType == FRAME && rcSH.getSPS().getMbAdaptiveFrameFieldFlag() )
      {
          rcSH.getRefPicList( TOP_FIELD, LIST_1 ).reset();
          rcSH.getRefPicList( BOT_FIELD, LIST_1 ).reset();
      }
  }
  
  //===== initial lists =====
  if( ! rcSH.isInterB() )
  {
      if( eCurrPicType == FRAME )
      {
          RNOK( xSetInitialReferenceListPFrame( rcSH ) );
      }
      else
      {
          RNOK( xSetInitialReferenceListPFields( rcSH ) );
      }
      }
  else
  {
      if( eCurrPicType == FRAME )
      {
          RNOK( xSetInitialReferenceListBFrame( rcSH ) );
      }
      else
      {
          RNOK( xSetInitialReferenceListBFields( rcSH ) );
      }

      // Check if L0==L1 here. -Dong
      RefPicList<RefPic>&     rcList0 = rcSH.getRefPicList( eCurrPicType, LIST_0 );
      RefPicList<RefPic>&     rcList1 = rcSH.getRefPicList( eCurrPicType, LIST_1 );
      if( rcList1.size() >= 2 && rcList0.size() == rcList1.size() )
      {
          Bool bSwitch = true;
          for( UInt uiPos = 0; uiPos < rcList1.size(); uiPos++ )
          {
              if( rcList0.get(uiPos) != rcList1.get(uiPos) )
              {
                  bSwitch = false;
                  break;
              }
          }
          if( bSwitch )
          {
              rcList1.switchFirstEntries();
          }
      }
  }

  return Err::m_nOK;
}



ErrVal FrameMng::xClearListsIDR( const SliceHeader& rcSH  )
{
	FUIter iterTemp = m_cOrderedPOCList.begin();
	FUIter iter;
  //===== output =====
  for( ;;)//FUIter iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )//lufeng :bug fix
  {
	if(iterTemp==m_cOrderedPOCList.end())break;
	iter=iterTemp;
	iterTemp++;

	if(*iter==m_pcCurrentFrameUnit)continue;
#if JM_MVC_COMPATIBLE
    if(true)//lufeng: clear DPB at one time
#else
	if(rcSH.getViewId() == (*iter)->getFrame().getViewId() )
#endif
    {
      if( ! rcSH.getNoOutputOfPriorPicsFlag() )
      {
        if ((*iter)->getPicBuffer() )  //JVT-S036 
        {
          (*iter)->getPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
          m_cPicBufferOutputList.push_back( (*iter)->getPicBuffer() );
        }
      }

      (*iter)->setOutputDone();

      if( xFindAndErase( m_cNonRefList, *iter ) )
      {
        RNOK( xAddToFreeList( (*iter) ) );
      }
      m_cOrderedPOCList.erase((iter));
    }
  }

//  m_cOrderedPOCList.clear();

  iterTemp = m_cShortTermList.begin();
  for( ;; )//lufeng: clear short term list
  {
	if(iterTemp==m_cShortTermList.end())break;
	iter=iterTemp;
	iterTemp++;

	if(*iter==m_pcCurrentFrameUnit)continue;
#if JM_MVC_COMPATIBLE
    if(true)
#else
    if( m_pcCurrentFrameUnit->getFrame().getViewId() == (*iter)->getFrame().getViewId()) // fix
#endif
    {
      RNOK( xRemoveFromRefList( m_cShortTermList, iter ) );
    }
  }

  return Err::m_nOK;
}


UInt FrameMng::MaxRefFrames( UInt uiLevel, UInt uiNumMbs )
{
  return m_uiDBPMemory[ uiLevel ] / ( 384 * uiNumMbs );
}



ErrVal FrameMng::outputAll()
{
  FUIter  iter;
  //===== output =====
  for( iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
  {
/*    if( (*iter)->getFGSPicBuffer() )
    {
      (*iter)->getFGSPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
      m_cPicBufferOutputList.push_back( (*iter)->getFGSPicBuffer() );
    }
    else 
*/
    if ((*iter)->getPicBuffer() )  //JVT-S036 
    {
      (*iter)->getPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
      m_cPicBufferOutputList.push_back( (*iter)->getPicBuffer() );
    }
    (*iter)->setOutputDone();
  }
  m_cOrderedPOCList.erase( m_cOrderedPOCList.begin(), iter );

  RNOK( xAddToFreeList( m_cShortTermList ) );
  RNOK( xAddToFreeList( m_cNonRefList ) );

  return Err::m_nOK;
}





/*
ErrVal FrameMng::storeFGSPicture( PicBuffer* pcPicBuffer )
{
  UInt uiFGSReconCount = m_pcCurrentFrameUnit->getFGSReconCount();
  m_pcCurrentFrameUnit->getFGSReconstruction(uiFGSReconCount)->copyAll(m_pcCurrentFrameUnit->getFGSIntFrame());
  m_pcCurrentFrameUnit->setFGSReconCount(uiFGSReconCount + 1);

  m_pcCurrentFrameUnit->setFGS( pcPicBuffer );
  m_pcCurrentFrameUnit->getFGSIntFrame()->store( pcPicBuffer );

  m_pcCurrentFrameUnit->getFGSFrame().extendFrame( m_pcQuarterPelFilter );
  
  return Err::m_nOK;
}
*/

ErrVal FrameMng::xStoreCurrentPicture( const SliceHeader& rcSH )
{
	// Frame& cBaseFrame = m_pcCurrentFrameUnit->getFrame();
	Frame& cBaseFrame = m_pcCurrentFrameUnit->getPic( rcSH.getPicType() ); //th fix
  PicBuffer cTempPicBuffer(cBaseFrame.getFullPelYuvBuffer()->getBuffer());

  PicBuffer*  pcTempPicBuffer = m_pcCurrentFrameUnit->getPicBuffer();
  cBaseFrame.setViewId(rcSH.getViewId());
  pcTempPicBuffer->setViewId(rcSH.getViewId());
  m_uiLastViewId = rcSH.getViewId();

  PicType eCurrPicType = rcSH.getPicType();

  Bool bFieldCoded = rcSH.getFieldPicFlag()? true : rcSH.isMbAff();

  m_pcCurrentFrameUnit->addPic( eCurrPicType, bFieldCoded, rcSH.getIdrPicId() );

  if( rcSH.getNalRefIdc() )
  {
      RNOK( m_pcCurrentFrameUnit->getPic( eCurrPicType ).extendFrame( m_pcQuarterPelFilter, rcSH.getSPS().getFrameMbsOnlyFlag(), false ) );

      //===== store as short term picture =====
      m_pcCurrentFrameUnit->setFrameNumber( rcSH.getFrameNum() );
      m_pcCurrentFrameUnit->setShortTerm  ( eCurrPicType );

      RNOK( xStoreShortTerm( m_pcCurrentFrameUnit ) );
  }
  else
  {
	  RNOK( m_pcCurrentFrameUnit->getPic( eCurrPicType ).extendFrame( m_pcQuarterPelFilter, rcSH.getSPS().getFrameMbsOnlyFlag(), false ) );
      RNOK( xStoreNonRef( m_pcCurrentFrameUnit ) );
  }
  return Err::m_nOK;
}



ErrVal FrameMng::xMmcoMarkShortTermAsUnused( const PicType eCurrPicType, const FrameUnit* pcCurrFrameUnit, UInt uiDiffOfPicNums )
{
    UInt  uiCurrPicNum  = ( eCurrPicType == FRAME ? pcCurrFrameUnit->getFrameNumber()
        : pcCurrFrameUnit->getFrameNumber() * 2 + 1 );
    PicType ePicType;
    UInt  uiPicNumN     = uiCurrPicNum - uiDiffOfPicNums - 1;

    if( uiCurrPicNum <= uiDiffOfPicNums )
    {
        uiPicNumN += ( eCurrPicType == FRAME ? m_uiMaxFrameNumPrev : 2 * m_uiMaxFrameNumPrev );
    }

    xSetIdentifier( uiPicNumN, ePicType, eCurrPicType );

    FUIter iter = m_cShortTermList.findShortTerm( uiPicNumN );
    if( iter == m_cShortTermList.end() )
    {
        printf("\nMMCO not possible\n" );
        return Err::m_nOK; // HS: decoder robustness
    }

    FrameUnit* pcFrameUnit = (*iter);
    pcFrameUnit->setUnused( ePicType );
    if( ! pcFrameUnit->isUsed( FRAME ) )
    {
        RNOK( xRemoveFromRefList( m_cShortTermList, iter ) )
    }

    return Err::m_nOK;
}

//JVT-S036  start
ErrVal FrameMng::xMmcoMarkShortTermAsUnusedBase( const PicType eCurrPicType, const FrameUnit* pcCurrFrameUnit, UInt uiDiffOfPicNums )
{
    UInt  uiCurrPicNum  = ( eCurrPicType == FRAME ? pcCurrFrameUnit->getFrameNumber()
        : pcCurrFrameUnit->getFrameNumber() * 2 + 1 );
    PicType ePicType;
    UInt  uiPicNumN     = uiCurrPicNum - uiDiffOfPicNums - 1;

    if( uiCurrPicNum <= uiDiffOfPicNums )
    {
        uiPicNumN += ( eCurrPicType == FRAME ? m_uiMaxFrameNumPrev : 2 * m_uiMaxFrameNumPrev );
    }

    xSetIdentifier( uiPicNumN, ePicType, eCurrPicType );

    FUIter iter = m_cShortTermList.findShortTerm( uiPicNumN );
    if( iter == m_cShortTermList.end() )
    {
        printf("\nMMCO not possible\n" );
        return Err::m_nOK; // HS: decoder robustness
    }

    FrameUnit* pcFrameUnit = (*iter);
    if(pcFrameUnit->getBaseRep() )
    {
        pcFrameUnit->setUnused( ePicType );
        if( ! pcFrameUnit->isUsed( FRAME ) )
        {
            RNOK( xRemoveFromRefList( m_cShortTermList, iter ) )
        }
    }

    return Err::m_nOK;
}
//JVT-S036  end

ErrVal FrameMng::xManageMemory( const SliceHeader& rcSH )
{
  ROTRS( ! rcSH.getNalRefIdc(), Err::m_nOK );

  if( ! rcSH.getAdaptiveRefPicBufferingFlag() )
  {
#if JM_MVC_COMPATIBLE // hwsun, add #if
	if( !getCurrentFrameUnit()->isRefPic() )
#endif
    RNOK( xSlidingWindowUpdate() );
    return Err::m_nOK;
  }

  MmcoOp eMmcoOp;
  const MmcoBuffer& rcMmcoBuffer = rcSH.getMmcoBuffer();
  UInt uiVal1, uiVal2;
  Int iIndex = 0;
  while( MMCO_END != (eMmcoOp = rcMmcoBuffer.get( iIndex++ ).getCommand( uiVal1, uiVal2)) )
  {

    switch (eMmcoOp)
    {
    case MMCO_SHORT_TERM_UNUSED:
		RNOK( xMmcoMarkShortTermAsUnusedMVC(rcSH.getPicType(), m_pcCurrentFrameUnit, uiVal1, rcSH.getViewId() ) );

      break;
    case MMCO_RESET:
    case MMCO_MAX_LONG_TERM_IDX:
    case MMCO_ASSIGN_LONG_TERM:
    case MMCO_LONG_TERM_UNUSED:
    case MMCO_SET_LONG_TERM:
    default:AF();
      break;
    }
    
  }


  return Err::m_nOK;
}



ErrVal FrameMng::xSlidingWindowUpdate()
{
  UInt uiSV = 0;
  FUList::iterator iter = m_cShortTermList.begin();
  FUList::iterator end  = m_cShortTermList.end();

  FUList::iterator temp;
  FrameUnit * pMinFrameUnit = NULL;  
  UInt uiMinFrameNumWrap  = m_uiMaxFrameNumCurr;
  for( ; iter != m_cShortTermList.end(); iter++ )
  {
    if( m_pcCurrentFrameUnit->getFrame().getViewId() == (*iter)->getFrame().getViewId()) // fix
    {
      uiSV++; 
      if ( ((*iter)->getFrameNumber() % m_uiMaxFrameNumCurr) < uiMinFrameNumWrap)
      {
        pMinFrameUnit=(*iter);
        temp = iter;
      }
      
    }
  }
  if (uiSV >= m_uiNumRefFrames ) 
    RNOK( xRemoveFromRefList( m_cShortTermList, temp ) );

  return Err::m_nOK;
}

//JVT-S036  start
ErrVal FrameMng::xSlidingWindowUpdateBase( UInt mCurrFrameNum )  
{
	FUList::iterator iter = m_cShortTermList.begin();
	FUList::iterator end  = m_cShortTermList.end();
	FUList::iterator iiter;

	for( ; iter != m_cShortTermList.end(); iter++ )
   {
	   if( (*iter)->getBaseRep() && (*iter)->getFrameNumber() != mCurrFrameNum )
    {
		for( iiter = m_cShortTermList.begin(); iiter != m_cShortTermList.end(); iiter++ )
		{
			if ( (*iiter)->getFrameNumber() == (*iter)->getFrameNumber() && !(*iiter)->getBaseRep() )
			{
                (*iter)->setUnused(FRAME);
				RNOK( xRemoveFromRefList( m_cShortTermList, iter ) );
				return Err::m_nOK;
			}
		}
    }
  }
   return Err::m_nOK;
}

ErrVal FrameMng::xMMCOUpdateBase( SliceHeader* rcSH )
{

  MmcoOp            eMmcoOp;
  const MmcoBuffer& rcMmcoBaseBuffer = rcSH->getMmcoBaseBuffer();
  Int               iIndex        = 0;
  UInt              uiVal1, uiVal2;

  while( MMCO_END != (eMmcoOp = rcMmcoBaseBuffer.get( iIndex++ ).getCommand( uiVal1, uiVal2 ) ) )
 {
		switch( eMmcoOp )
		{
		case MMCO_SHORT_TERM_UNUSED:
            RNOK( xMmcoMarkShortTermAsUnusedBase( rcSH->getPicType(), m_pcCurrentFrameUnit, uiVal1 ) );
		break;
		case MMCO_RESET:
		case MMCO_MAX_LONG_TERM_IDX:
		case MMCO_ASSIGN_LONG_TERM:
		case MMCO_LONG_TERM_UNUSED:
		case MMCO_SET_LONG_TERM:
		default:
			fprintf( stderr,"\nERROR: MMCO COMMAND currently not supported in the software\n\n" );
		RERR();
		}
 }
	return Err::m_nOK;
}
//JVT-S036  end

ErrVal FrameMng::getRecYuvBuffer( YuvPicBuffer*& rpcRecYuvBuffer, PicType ePicType )
{
    ROT( NULL == m_pcCurrentFrameUnit );
    rpcRecYuvBuffer = m_pcCurrentFrameUnit->getPic( ePicType ).getFullPelYuvBuffer();
    return Err::m_nOK;
}



FrameUnit*
FrameMng::getReconstructedFrameUnit( Int iPoc )
{
  for( FUIter iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
  {
    if( (*iter)->getMaxPOC() == iPoc )
    {
      return *iter;
    }
  }
  return 0;
}

ErrVal FrameMng::xReferenceListRemapping( SliceHeader& rcSH, ListIdx eListIdx )
{
    PicType eCurrPicType   = rcSH.getPicType();
    RefPicList<RefPic>& rcList = rcSH.getRefPicList( eCurrPicType, eListIdx );
    ROTRS( 0 == rcList.bufSize(), Err::m_nOK );

    const RplrBuffer& rcRplrBuffer = rcSH.getRplrBuffer( eListIdx );

    ROTRS( ! rcRplrBuffer.getRefPicListReorderingFlag(), Err::m_nOK );

    UInt      uiPicNumPred  = ( eCurrPicType == FRAME ? rcSH.getFrameNum()
        : rcSH.getFrameNum() * 2 + 1 );
    UInt      uiMaxPicNum   = ( eCurrPicType == FRAME ? m_uiMaxFrameNumPrev : 2 * m_uiMaxFrameNumPrev );
    UInt      uiIndex       = 0;
    UInt      uiCommand;
    UInt      uiIdentifier;
// JVT-V043
  Int       iPicViewIdx   = -1; //JVT-AB204_r1, ying
  Int IndexSkipCount=0;

  while( RPLR_END != ( uiCommand = rcRplrBuffer.get( uiIndex ).getCommand( uiIdentifier ) ) )
  {
      PicType   ePicType;
    FUIter    iter;
    const Frame* pcFrame = NULL;
	Bool InterViewFlag=false;
    
    if( uiCommand == RPLR_LONG )
    //===== LONG TERM INDEX =====
    {
      AF();
    }
    else if ( uiCommand == RPLR_NEG || uiCommand == RPLR_POS) //JVT-V043 
    //===== SHORT TERM INDEX =====
    {
      UInt uiAbsDiff = uiIdentifier + 1;
      //---- set short term index ----
      if( uiCommand == RPLR_NEG )
      {
        if( uiPicNumPred < uiAbsDiff )
        {
          uiPicNumPred -= ( uiAbsDiff - uiMaxPicNum );
        }
        else
        {
          uiPicNumPred -= uiAbsDiff;
        }
      }
      else
      {
        if( uiPicNumPred + uiAbsDiff > uiMaxPicNum - 1 )
        {
          uiPicNumPred += ( uiAbsDiff - uiMaxPicNum );
        }
        else
        {
          uiPicNumPred += uiAbsDiff;
        }
      }
      uiIdentifier = uiPicNumPred;
      xSetIdentifier( uiIdentifier, ePicType, eCurrPicType );
      //---- search for short term picture ----
      iter = m_cShortTermList.findShortTermMVC( uiIdentifier, rcSH.getViewId() ); 

      //---- check ----
      if( iter == m_cShortTermList.end() || ! (*iter)->isShortTerm( ePicType ) )
      {
        return Err::m_nDataNotAvailable;
      }
      else
      { // everything is fine
        //---- set frame ----
          pcFrame = &((*iter)->getPic( ePicType ) );
      }
    }
    else // uiCommand == 4 or 5 JVT-V043
    {

		Int iAbsDiff = uiIdentifier + 1; 
				
// JVT-W066
        Int  iMaxRef = rcSH.getSPS().getSpsMVC()->getNumRefsForListX (rcSH.getViewId(), 
                                                                       eListIdx, 
                                                                       rcSH.getAnchorPicFlag());

        if( uiCommand == RPLR_VIEW_NEG )
        {
          if( iPicViewIdx < iAbsDiff )
          {
            iPicViewIdx -= ( iAbsDiff - iMaxRef );
           }
           else
           {
              iPicViewIdx -=   iAbsDiff;
           }
         }

         if( uiCommand == RPLR_VIEW_POS)
         {
           if( iPicViewIdx + iAbsDiff >= iMaxRef )
           {
             iPicViewIdx += ( iAbsDiff - iMaxRef );
           }
           else
           {
             iPicViewIdx +=   iAbsDiff;
           }
         }
// JVT-W066
      uiIdentifier = iPicViewIdx; 

      iter = m_cShortTermList.findInterView(rcSH, uiIdentifier, eListIdx);
      if( iter == m_cShortTermList.end()  ) 
      {
        iter = m_cNonRefList.findInterView(rcSH, uiIdentifier, eListIdx);
        //---- check ----
        if( iter == m_cNonRefList.end() )
        {
          return Err::m_nDataNotAvailable;
        }
      }

     //---- set frame ----
     InterViewFlag = (*iter)->getFrame().getInterViewFlag(); 
	  if (InterViewFlag == true)
		  pcFrame = &((*iter)->getFrame(rcSH.getPicType()) );

    }
    //---- find picture in reference list -----
    if ( (uiCommand == RPLR_VIEW_NEG || uiCommand == RPLR_VIEW_POS) && InterViewFlag == false )
	{
		uiIndex++;
		IndexSkipCount++;

	} else 
	{
		//---- find picture in reference list -----
	    UInt uiRemoveIndex = MSYS_UINT_MAX;
		if( NULL != pcFrame )
		{
			for( UInt uiPos = uiIndex; uiPos < rcList.size(); uiPos++ )
			{
				if( rcList.get( uiPos ).getFrame() == pcFrame )
				{
					uiRemoveIndex = uiPos;
					break;
				}
			}
		}
	    //----- reference list reordering ----- shift
		rcList.getElementAndRemove( uiIndex-IndexSkipCount, uiRemoveIndex ).setFrame( pcFrame );
		uiIndex++;
	}
  }

  return Err::m_nOK;
}

ErrVal FrameMng::xSetOutputListMVC( FrameUnit* pcFrameUnit, UInt uiNumOfViews )
{
  ROTRS( m_iEntriesInDPB <= m_iMaxEntriesinDPB, Err::m_nOK );

  UInt uiOutput = 0;

  for (UInt i = 0; i < uiNumOfViews; i++ )
  {
      //===== get minimum POC for output =====
      Int     iMinPOCtoOuput = MSYS_INT_MAX;
      FUIter  iter;
      for( iter = m_cNonRefList.begin(); iter != m_cNonRefList.end(); iter++ )
      {
          if( (*iter)->getMaxPOC() < iMinPOCtoOuput && ( (*iter) != pcFrameUnit) && (*iter)->getFrame().getViewId() == i)
          {
              iMinPOCtoOuput = (*iter)->getMaxPOC();
          }
      }

//      ROT( iMinPOCtoOuput == MSYS_INT_MAX );
      if(iMinPOCtoOuput == MSYS_INT_MAX)
          continue;
      
      uiOutput++;

      //===== output =====
      for( iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
      {
          if( (*iter)->getMaxPOC() <= iMinPOCtoOuput && (*iter)->getFrame().getViewId() == i )
          {
/*              if( (*iter)->getFGSPicBuffer() )
              {
                  (*iter)->getFGSPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
                  m_cPicBufferOutputList.push_back( (*iter)->getFGSPicBuffer() );
              }
              else 
*/
              if ((*iter)->getPicBuffer() )  //JVT-S036 
              {
                  (*iter)->getPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
                  m_cPicBufferOutputList.push_back( (*iter)->getPicBuffer() );
              }
              (*iter)->setOutputDone();
              if( xFindAndErase( m_cNonRefList, *iter ) )
              {
                  RNOK( xAddToFreeList( *iter ) );
              }
          }
          else
          {
              break;
          }
      }
      m_cOrderedPOCList.erase( m_cOrderedPOCList.begin(), iter );

  }

  ROF(uiOutput);

  return Err::m_nOK;
}

ErrVal FrameMng::xSetOutputListMVC( FrameUnit* pcFrameUnit, const SliceHeader& rcSH )
{
  ROTRS( m_iEntriesInDPB <= m_iMaxEntriesinDPB, Err::m_nOK );

  UInt uiNumOfViews = rcSH.getSPS().getSpsMVC()->getNumViewMinus1()+1;	
  UInt uiOutput = 0;

  for (UInt i = 0; i < uiNumOfViews; i++ )
  {
      //===== get minimum POC for output =====
      Int     iMinPOCtoOuput = MSYS_INT_MAX;
      FUIter  iter;
      for( iter = m_cNonRefList.begin(); iter != m_cNonRefList.end(); iter++ )
      {          
		  if( (*iter)->getMaxPOC() < iMinPOCtoOuput && ( (*iter) != pcFrameUnit) && (*iter)->getFrame().getViewId() == rcSH.getSPS().getSpsMVC()->m_uiViewCodingOrder[i])
			  iMinPOCtoOuput = (*iter)->getMaxPOC();
         
      }

     if(iMinPOCtoOuput == MSYS_INT_MAX)
          continue;
      
      uiOutput++;

      //===== output =====
      for( iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
      {
          if( (*iter)->getMaxPOC() <= iMinPOCtoOuput && (*iter)->getFrame().getViewId() == rcSH.getSPS().getSpsMVC()->m_uiViewCodingOrder[i] )
          {

              if ((*iter)->getPicBuffer() )  //JVT-S036 
              {
                  (*iter)->getPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
                  m_cPicBufferOutputList.push_back( (*iter)->getPicBuffer() );
              }
              (*iter)->setOutputDone();
              if( xFindAndErase( m_cNonRefList, *iter ) ) 
              {
                  RNOK( xAddToFreeList( *iter ) );
              }
          }
          else
          {
              break;
          }
      }
      m_cOrderedPOCList.erase( m_cOrderedPOCList.begin(), iter );

  }

  if (!uiOutput)
	  printf("Oops! Current prediction structure seems to require a larger DPB size than is allowed by the standard.\n");

  ROF(uiOutput);

  return Err::m_nOK;
}



ErrVal
FrameMng::xDumpRefList( ListIdx       eListIdx,
                        SliceHeader& rcSH )
{
#if 0 // NO_DEBUG
  return Err::m_nOK;
#endif
  RefPicList<RefPic>& rcList = rcSH.getRefPicList( rcSH.getPicType() ,eListIdx  );

#if JM_MVC_COMPATIBLE
#define DELTA_POCA  DELTA_POC
#else
#define DELTA_POCA  0
#endif

//  printf("List %d: ", eListIdx );
  //for( UInt uiIndex = 0; uiIndex < rcList.size(); uiIndex++ )
  for( UInt uiIndex = 0; uiIndex < rcSH.getNumRefIdxActive(eListIdx); uiIndex++ )
  {
    printf("%d/%d ", rcList.get(uiIndex).getFrame()->getViewId(), 
           rcList.get(uiIndex).getFrame()->getPOC()-DELTA_POCA );
  }
  printf("\n");
  return Err::m_nOK;
}

   ErrVal FrameMng::xMmcoMarkShortTermAsUnusedMVC( const PicType eCurrPicType, const FrameUnit* pcCurrFrameUnit, 
                                               UInt uiDiffOfPicNums, UInt uiCurrViewId )
{
  UInt  uiCurrPicNum  = ( eCurrPicType == FRAME ? pcCurrFrameUnit->getFrameNumber()
      : pcCurrFrameUnit->getFrameNumber() * 2 + 1 );
  PicType ePicType;

  UInt  uiPicNumN     = uiCurrPicNum - uiDiffOfPicNums - 1;

  if( uiCurrPicNum <= uiDiffOfPicNums )
  {
    uiPicNumN += ( eCurrPicType == FRAME ? m_uiMaxFrameNumPrev : 2 * m_uiMaxFrameNumPrev );
  }

  xSetIdentifier( uiPicNumN, ePicType, eCurrPicType );

  FUIter iter = m_cShortTermList.findShortTermMVC( uiPicNumN, uiCurrViewId );
  if( iter == m_cShortTermList.end() )
  {
    printf("\nMMCO not possible\n" );
    return Err::m_nOK; // HS: decoder robustness
  }

  FrameUnit* pcFrameUnit = (*iter);
  pcFrameUnit->setUnused( ePicType );
  if( ! pcFrameUnit->isUsed( FRAME ) )
  {
      RNOK( xRemoveFromRefList( m_cShortTermList, iter ) )
  }

  return Err::m_nOK;
}


ErrVal FrameMng::xSetReferenceListsMVC( SliceHeader& rcSH )
{
  if( rcSH.isIntra() )
  {
    return Err::m_nOK;
  }

  if( rcSH.isInterP() )
  {
    xSetPFrameListMVC(rcSH);
  }
  else
  {
    xSetBFrameListMVC(rcSH);
  }
// cleanup 
  return Err::m_nOK;
}

//  {{
ErrVal            
FrameMng::xSetPFrameListMVC ( SliceHeader& rcSH)
{
  RefPicList<RefPic>& rcList0 = rcSH.getRefPicList(  rcSH.getPicType() ,LIST_0 );

  UInt  uiInterViewPredRefNumFwd = ( rcSH.getAnchorPicFlag() ? rcSH.getSPS().SpsMVC->getNumAnchorRefsForListX(rcSH.getViewId(), LIST_0) : rcSH.getSPS().SpsMVC->getNumNonAnchorRefsForListX(rcSH.getViewId(), LIST_0));
  if (uiInterViewPredRefNumFwd ==0 ) 
    return Err::m_nOK;

  RefPicList<Frame*>      cTempFrameList0;
//  cTempFrameList0.reset(uiNumRefActive0-uiInterPredRefNumFwd);
    cTempFrameList0.reset(uiInterViewPredRefNumFwd);

  m_cShortTermList.setRefFrameListFGSMVCView( cTempFrameList0, rcSH, LIST_0 );
//  if(cTempFrameList0.size()!=uiNumRefActive0-uiInterPredRefNumFwd)
  if(cTempFrameList0.size()!= uiInterViewPredRefNumFwd )
    m_cNonRefList.setRefFrameListFGSMVCViewNonRef(cTempFrameList0, rcSH, LIST_0);


  UInt uiInterPredSize = rcList0.size();
  for(UInt uiIdx=0; uiIdx< uiInterViewPredRefNumFwd; uiIdx++)
  {
	  rcList0.next();
	  rcList0.getElementAndRemove(uiIdx + uiInterPredSize , uiIdx+uiInterPredSize ).setFrame( &cTempFrameList0.get(uiIdx)->getFrameUnit()->getPic(rcSH.getPicType()));
  }

  return Err::m_nOK;
}

ErrVal
FrameMng::xSetBFrameListMVC ( SliceHeader& rcSH)
{
  RefPicList<RefPic>& rcList0 = rcSH.getRefPicList(  rcSH.getPicType(),LIST_0  );
  RefPicList<RefPic>& rcList1 = rcSH.getRefPicList(  rcSH.getPicType() ,LIST_1 );

  UInt  uiInterViewPredRefNumFwd = ( rcSH.getAnchorPicFlag() ? rcSH.getSPS().SpsMVC->getNumAnchorRefsForListX(rcSH.getViewId(), LIST_0) : rcSH.getSPS().SpsMVC->getNumNonAnchorRefsForListX(rcSH.getViewId(), LIST_0));
  UInt  uiInterViewPredRefNumBwd = ( rcSH.getAnchorPicFlag() ? rcSH.getSPS().SpsMVC->getNumAnchorRefsForListX(rcSH.getViewId(), LIST_1) : rcSH.getSPS().SpsMVC->getNumNonAnchorRefsForListX(rcSH.getViewId(), LIST_1));
  
  if ( uiInterViewPredRefNumFwd ==0 && uiInterViewPredRefNumBwd ==0 ) 
    return Err::m_nOK;

  RefPicList<Frame*>     cTempFrameList0;
  RefPicList<Frame*>     cTempFrameList1;

  cTempFrameList0.reset(uiInterViewPredRefNumFwd);
  cTempFrameList1.reset(uiInterViewPredRefNumBwd);
    
  m_cShortTermList.setRefFrameListFGSMVCView( cTempFrameList0, rcSH, LIST_0 );
  m_cShortTermList.setRefFrameListFGSMVCView( cTempFrameList1, rcSH, LIST_1 );
  if(cTempFrameList0.size()!= uiInterViewPredRefNumFwd)
    m_cNonRefList.setRefFrameListFGSMVCViewNonRef(cTempFrameList0, rcSH, LIST_0);
  if(cTempFrameList1.size()!= uiInterViewPredRefNumBwd)
    m_cNonRefList.setRefFrameListFGSMVCViewNonRef(cTempFrameList1, rcSH, LIST_1);
  UInt uiIdx;
  UInt uiInterPredSizeL0 = rcList0.size();
  UInt uiInterPredSizeL1 = rcList1.size();
  
    //for( uiIdx=0; uiIdx< uiInterViewPredRefNumFwd; uiIdx++)
  for( uiIdx=0; uiIdx< cTempFrameList0.size(); uiIdx++)
  {
    rcList0.next();
    rcList0.getElementAndRemove(uiIdx + uiInterPredSizeL0 , uiIdx+uiInterPredSizeL0 ).setFrame( &cTempFrameList0.get(uiIdx)->getFrameUnit()->getPic(rcSH.getPicType()));
  }
  
  //for( uiIdx=0; uiIdx< uiInterViewPredRefNumBwd; uiIdx++)
  for( uiIdx=0; uiIdx< cTempFrameList1.size(); uiIdx++)
  {
    rcList1.next();
    rcList1.getElementAndRemove(uiIdx + uiInterPredSizeL1 , uiIdx+uiInterPredSizeL1 ).setFrame( &cTempFrameList1.get(uiIdx)->getFrameUnit()->getPic(rcSH.getPicType()));
  }
  return Err::m_nOK;
}  
//  }}

H264AVC_NAMESPACE_END


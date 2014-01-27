#if !defined(AFX_H264AVCCOMMONIF_H__625AA7B6_0241_4166_8D3A_BC831985BE5F__INCLUDED_)
#define AFX_H264AVCCOMMONIF_H__625AA7B6_0241_4166_8D3A_BC831985BE5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#if defined( WIN32 )
# if !defined( MSYS_WIN32 )
#   define MSYS_WIN32
# endif
#endif

#if defined( _DEBUG ) || defined( DEBUG )
# if !defined( _DEBUG )
#   define _DEBUG
# endif
# if !defined( DEBUG )
#   define DEBUG
# endif
#endif


typedef int ErrVal;

class Err  
{
public:
  static const ErrVal m_nOK;         
  static const ErrVal m_nERR;         
  static const ErrVal m_nEndOfStream;
  static const ErrVal m_nEndOfFile;
  static const ErrVal m_nEndOfBuffer;
  static const ErrVal m_nInvalidParameter;
  static const ErrVal m_nDataNotAvailable;
};

#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include "Typedefs.h"
#include "Macros.h"
#include "MemList.h"

#include <list>         // Move the two lines prior to min() and max() to avoid certain Linux compiling issue. -Dong
#include <algorithm>

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

typedef MemCont< UChar > BinData;
typedef MemList< UChar > BinDataList;
typedef MemAccessor< UChar > BinDataAccessor;

typedef UChar   Pel;
typedef Short   TCoeff;
typedef Short   XPel;
typedef Long    XXPel;

template< class T >
class MyList : public std::list< T >
{
public:
  typedef typename std::list<T>::iterator MyIterator;

  MyList& operator += ( const MyList& rcMyList) { if( ! rcMyList.empty() ) { insert( this->end(), rcMyList.begin(), rcMyList.end());} return *this; } // leszek
  T popBack()                           { T cT = this->back(); this->pop_back(); return cT;  }
  T popFront()                          { T cT = this->front(); this->pop_front(); return cT; }
  Void pushBack( const T& rcT )         { if( sizeof(T) == sizeof(void*)) { if( rcT != NULL ){ push_back( rcT);} } } // Fix crash with Linux 64 systems. -Dong
  Void pushFront( const T& rcT )        { if( sizeof(T) == sizeof(void*)) { if( rcT != NULL ){ push_front( rcT);} } }
  MyIterator find( const T& rcT ) {  return std::find( this->begin(), this->end(), rcT ); } // leszek
};

class ExtBinDataAccessor : public BinDataAccessor
{
public:
  ExtBinDataAccessor() : BinDataAccessor() , m_pcMediaPacket (NULL ){}
  ExtBinDataAccessor( BinDataAccessor& rcAccessor, Void* pcMediaPacket = NULL ) 
    :  BinDataAccessor( rcAccessor ) 
    ,  m_pcMediaPacket (pcMediaPacket ){}

  Void* getMediaPacket() { return m_pcMediaPacket; }
private:
  Void* m_pcMediaPacket;
};


typedef MyList< ExtBinDataAccessor* > ExtBinDataAccessorList;
typedef MyList< ExtBinDataAccessorList* > ExtBinDataAccessorListList;

enum PicStruct
{
    PS_NOT_SPECIFIED = -1,
    PS_FRAME         =  0, // frame	field_pic_flag shall be 0	1
    PS_TOP           =  1, // top field	field_pic_flag shall be 1, bottom_field_flag shall be 0 1
    PS_BOT           =  2, // bottom field field_pic_flag shall be 1, bottom_field_flag shall be 1 1
    PS_TOP_BOT       =  3, // top field, bottom field, in that order field_pic_flag shall be 0 2
    PS_BOT_TOP       =  4  // bottom field, top field, in that order field_pic_flag shall be 0 2
};

class PicBuffer
{
public:
    PicBuffer( Pel* pcBuffer = NULL, Void* pcMediaPacket  = NULL, UInt64 ui64Cts = 0) 
        : m_pcMediaPacket( pcMediaPacket )
        , m_pcBuffer     ( pcBuffer )
        , m_iInUseCout   ( 0 )
        , m_ui64Cts      ( ui64Cts )
        , m_ePicStruct   ( PS_NOT_SPECIFIED )
        , m_iTopPoc      ( 0 )
        , m_iBotPoc      ( 0 )
        , m_iFramePoc    ( 0 )
        , m_uiIdrPicId   ( 0 )
        , m_bFieldCoded  ( false )
    {}

    Void setUnused()           { m_iInUseCout--; }
    Void setUsed()             { m_iInUseCout++; }
    Bool isUsed()              { return 0 != m_iInUseCout; }
    Pel* getBuffer()           { return m_pcBuffer; }
    operator Pel*()            { return m_pcBuffer; }
    Void* getMediaPacket()     { return m_pcMediaPacket; }
    UInt64& getCts()           { return m_ui64Cts; }

    Void setPicStruct     ( PicStruct e  ) { m_ePicStruct  = e;  }
    Void setFieldCoding   ( Bool      b  ) { m_bFieldCoded = b;  }
    Void setIdrPicId      ( UInt      ui ) { m_uiIdrPicId  = ui; }
    Void setTopPOC        ( Int       i  ) { m_iTopPoc     = i;  }
    Void setBotPOC        ( Int       i  ) { m_iBotPoc     = i;  }
    Void setFramePOC      ( Int       i  ) { m_iFramePoc   = i;  }

    PicStruct getPicStruct() const  { return m_ePicStruct; }
    Bool isFieldCoded     () const  { return m_bFieldCoded; }
    UInt getIdrPicId      () const  { return m_uiIdrPicId; }
    Int  getTopPOC        () const  { return m_iTopPoc; }
    Int  getBotPOC        () const  { return m_iBotPoc; }
    Int  getFramePOC      () const  { return m_iFramePoc; }
    Void setCts( UInt64 ui64 ) { m_ui64Cts = ui64; } // HS: decoder robustness

    UInt          getViewId       ()  const { return m_uiViewId; }
    Void          setViewId       (UInt v_id)  { m_uiViewId = v_id; }

private:
    Void*  m_pcMediaPacket;
    Pel*   m_pcBuffer;
    Int    m_iInUseCout;
    UInt64 m_ui64Cts;

    PicStruct m_ePicStruct;
    Int       m_iTopPoc;
    Int       m_iBotPoc;
    Int       m_iFramePoc;
    UInt      m_uiIdrPicId;
    Bool      m_bFieldCoded;
    UInt          m_uiViewId;
};

typedef MyList< PicBuffer* > PicBufferList;


#endif // !defined(AFX_H264AVCCOMMONIF_H__625AA7B6_0241_4166_8D3A_BC831985BE5F__INCLUDED_)

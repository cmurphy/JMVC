#if !defined(AFX_TRACEFILE_H__B87E26CF_023E_4DC7_8F94_D3E38F59ABA1__INCLUDED_)
#define AFX_TRACEFILE_H__B87E26CF_023E_4DC7_8F94_D3E38F59ABA1__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#define ENCODER_TRACE     0
#define DECODER_TRACE     0


#define MAX_LINE_LENGTH 255


H264AVC_NAMESPACE_BEGIN


class H264AVCCOMMONLIB_API TraceFile
{
public:
	TraceFile         ();
	virtual ~TraceFile();

  static Bool IsInitialized() {return Initialized;}
  static ErrVal initTrace   (Bool b,  UInt uiNumOfViews);
  static ErrVal openTrace   ( Char* pucBaseFilename, UInt uiViewId );
  static ErrVal closeTrace  ();
  static ErrVal setLayer    ( UInt  uiLayerId );
  static ErrVal setViewId    ( UInt  uiViewId );


  static ErrVal startNalUnit();
  static ErrVal startFrame  ();
  static ErrVal startSlice  ();
  static ErrVal startMb     ( Int   iMbAddress  );

  static ErrVal printHeading( const Char* pcString    );

  static ErrVal countBits   ( UInt  uiBitCount );
  static ErrVal printPos    ();

  static ErrVal printString ( const Char* pcString );
  static ErrVal printVal    ( UInt  uiVal );
  static ErrVal printVal    ( Int   iVal );
  static ErrVal printXVal   ( UInt  uiVal );

  static ErrVal addBits     ( UInt  uiVal, UInt uiLength );
  static ErrVal printBits   ( UInt  uiVal, UInt uiLength );
  static ErrVal printCode   ( UInt  uiVal );
  static ErrVal printCode   ( Int   iVal );
  static ErrVal printType   ( Char* pcString);

  static ErrVal newLine();

protected:
  static UInt  sm_uiLayer;
  static UInt  sm_uiViewId;
  static Bool  sm_bEncoder;
  static UInt  sm_uiNumOfViews;
  static FILE **sm_fTrace;
  static UInt  *sm_uiFrameNum;
  static UInt  *sm_uiSliceNum;
  static UInt  *sm_uiPosCounter;
  static Char  sm_acLine      [MAX_LINE_LENGTH];
  static Char  sm_acType      [9];
  static Char  sm_acPos       [9];
  static Char  sm_acCode      [6];
  static Char  sm_acBits      [35];
  static Bool Initialized;	

};



H264AVC_NAMESPACE_END




#if ENCODER_TRACE
  #define INIT_ETRACE(x, y)     if( m_bTraceEnable ) TraceFile::initTrace   (x, y)
  #define OPEN_ETRACE(x)   if( m_bTraceEnable ) TraceFile::openTrace   ("TraceEncoder", x)
  #define CLOSE_ETRACE     if( m_bTraceEnable ) TraceFile::closeTrace  ()
  #define ETRACE_VIEWID(x) if( m_bTraceEnable ) TraceFile::setViewId   (x) 
  
  #define ETRACE_LAYER(x)  if( m_bTraceEnable ) TraceFile::setLayer    (x) 
  #define ETRACE_NEWFRAME  if( m_bTraceEnable ) TraceFile::startFrame  ()
  #define ETRACE_NEWSLICE  if( m_bTraceEnable ) TraceFile::startSlice  ()
  #define ETRACE_NEWMB(x)  if( m_bTraceEnable ) TraceFile::startMb     (x)
  #define ETRACE_HEADER(x)                      TraceFile::printHeading(x)

  #define ETRACE_POS       if( m_bTraceEnable ) TraceFile::printPos    ()
  #define ETRACE_COUNT(i)  if( m_bTraceEnable ) TraceFile::countBits   (i)

  #define ETRACE_BITS(v,l) if( m_bTraceEnable ) TraceFile::addBits     (v,l)
  #define ETRACE_CODE(v)   if( m_bTraceEnable ) TraceFile::printCode   (v)

  #define ETRACE_TH(t)     if( m_bTraceEnable ) TraceFile::printString (t)
  #define ETRACE_T(t)      if( m_bTraceEnable ) TraceFile::printString (t)
  #define ETRACE_TY(t)     if( m_bTraceEnable ) TraceFile::printType   (t)
  #define ETRACE_V(t)      if( m_bTraceEnable ) TraceFile::printVal    (t)
  #define ETRACE_X(t)      if( m_bTraceEnable ) TraceFile::printXVal   (t)

  #define ETRACE_N         if( m_bTraceEnable ) TraceFile::newLine     ()
  #define ETRACE_DO(x)     if( m_bTraceEnable ) x
  #define ETRACE_DECLARE(x) x
#else

  #define OPEN_ETRACE(x)
#define INIT_ETRACE(x,y)
  #define CLOSE_ETRACE
  #define ETRACE_VIEWID(x)
  #define ETRACE_LAYER(x)
  #define ETRACE_NEWFRAME
  #define ETRACE_NEWSLICE
  #define ETRACE_NEWMB(x)
  #define ETRACE_HEADER(x)

  #define ETRACE_POS
  #define ETRACE_COUNT(i)

  #define ETRACE_BITS(v,l)
  #define ETRACE_CODE(v)

  #define ETRACE_TH(t)
  #define ETRACE_T(t)
  #define ETRACE_TY(t)
  #define ETRACE_V(t)
  #define ETRACE_X(t)

  #define ETRACE_N
  #define ETRACE_DO(x)
  #define ETRACE_DECLARE(x) 
#endif

#if DECODER_TRACE
  #define INIT_DTRACE(x, y)   TraceFile::initTrace   (x, y)
  #define OPEN_DTRACE      TraceFile::openTrace   ("TraceDecoder", 0)
  #define CLOSE_DTRACE     TraceFile::closeTrace  ()
  
  #define DTRACE_LAYER(x)  TraceFile::setLayer    (x) 
  #define DTRACE_VIEWID(x) TraceFile::setViewId   (x) 
  #define DTRACE_NEWFRAME  TraceFile::startFrame  ()
  #define DTRACE_NEWSLICE  TraceFile::startSlice  ()
  #define DTRACE_NEWMB(x)  TraceFile::startMb     (x)
  #define DTRACE_HEADER(x) TraceFile::printHeading(x)

  #define DTRACE_POS       TraceFile::printPos    ()
  #define DTRACE_COUNT(i)  TraceFile::countBits   (i)

  #define DTRACE_BITS(v,l) TraceFile::addBits     (v,l)
  #define DTRACE_CODE(v)   TraceFile::printCode   (v)

  #define DTRACE_TH(t)     TraceFile::printString (t)
  #define DTRACE_T(t)      TraceFile::printString (t)
  #define DTRACE_TY(t)     TraceFile::printType   (t)
  #define DTRACE_V(t)      TraceFile::printVal    (t)
  #define DTRACE_X(t)      TraceFile::printXVal   (t)

  #define DTRACE_N         TraceFile::newLine     ()
  #define DTRACE_DO(x)     x
#else
  #define OPEN_DTRACE
  #define INIT_DTRACE(x,y)
  #define CLOSE_DTRACE

  #define DTRACE_LAYER(x)
  #define DTRACE_VIEWID(x) 
  #define DTRACE_NEWFRAME
  #define DTRACE_NEWSLICE
  #define DTRACE_NEWMB(x)
  #define DTRACE_HEADER(x)

  #define DTRACE_POS
  #define DTRACE_COUNT(i)

  #define DTRACE_BITS(v,l)
  #define DTRACE_CODE(v)

  #define DTRACE_TH(t)
  #define DTRACE_T(t)
  #define DTRACE_TY(t)
  #define DTRACE_V(t)
  #define DTRACE_X(t)

  #define DTRACE_N
  #define DTRACE_DO(x)
#endif

#endif // !defined(AFX_TRACEFILE_H__B87E26CF_023E_4DC7_8F94_D3E38F59ABA1__INCLUDED_)

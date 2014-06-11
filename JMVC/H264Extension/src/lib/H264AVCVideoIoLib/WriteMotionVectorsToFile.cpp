#include "WriteMotionVectorsToFile.h"

WriteMotionVectorsToFile::WriteMotionVectorsToFile()
{}

WriteMotionVectorsToFile::~WriteMotionVectorsToFile()
{}

ErrVal WriteMotionVectorsToFile::create( WriteMotionVectorsToFile *& rpcWriteMotionVectorsToFile, Int picOrder)
{
  char * pcMvFileName = new char[strlen("motionfile")+10];
  sprintf(pcMvFileName, "motionfile%d", picOrder);
  rpcWriteMotionVectorsToFile = new WriteMotionVectorsToFile;
  ROT ( NULL == rpcWriteMotionVectorsToFile );
  RNOKS ( rpcWriteMotionVectorsToFile->init(pcMvFileName) );
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::init( const std::string& rcFileName )
{
  if( ! (motionVectorFile = fopen(rcFileName.c_str(), "w")))
  {
    std::cerr << "Failed to create motion vector file " << rcFileName.data() << std::endl;
    return Err::m_nERR;
  }
  fprintf(motionVectorFile, "%-12s", "Mb Num");
  fprintf(motionVectorFile, "%-12s", "Mb Coord");
  fprintf(motionVectorFile, "%-12s", "Mb Type");
  fprintf(motionVectorFile, "%-36s", "List 0");
  fprintf(motionVectorFile, "%-36s\n", "List 1");
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::uninit()
{
  fclose(motionVectorFile);
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::destroy()
{
  RNOK ( uninit() );
  delete this;
  return Err::m_nOK;
}

// Write 16x16 macroblock vectors
ErrVal WriteMotionVectorsToFile::writeMotionVector(UInt mvId,
                                                   UInt mvX, UInt mvY,
                                                   const std::string& mbType,
                                                   Short mv_l0_hor,
                                                   Short mv_l0_ver,
                                                   Short mv_l1_hor,
                                                   Short mv_l1_ver)
{
  if ( 0 > fprintf(motionVectorFile, "%-12u%3u,%-8u%-12s%4hd,%-31hd%4hd,%-31hd\n", mvId, mvX, mvY, mbType.c_str(), mv_l0_hor, mv_l0_ver, mv_l1_hor, mv_l1_ver) ) {
    return Err::m_nERR;
  }
  return Err::m_nOK;
}

// Write 16x8 or 8x16 macroblock vectors
ErrVal WriteMotionVectorsToFile::writeMotionVector(UInt mvId,
                                                   UInt mvX, UInt mvY,
                                                   const std::string& mbType,
                                                   Short mv_l0_0_hor,
                                                   Short mv_l0_0_ver,
                                                   Short mv_l0_1_hor,
                                                   Short mv_l0_1_ver,
                                                   Short mv_l1_0_hor,
                                                   Short mv_l1_0_ver,
                                                   Short mv_l1_1_hor,
                                                   Short mv_l1_1_ver)
{
  if ( 0 > fprintf(motionVectorFile, "%-12u%3u,%-8u%-12s%4hd,%-4hd%4hd,%-22hd%4hd,%-4hd%4hd,%-22hd\n", mvId, mvX, mvY, mbType.c_str(), mv_l0_0_hor, mv_l0_0_ver, mv_l0_1_hor, mv_l0_1_ver, mv_l1_0_hor, mv_l1_0_ver, mv_l1_1_hor, mv_l1_1_ver) ) {
    return Err::m_nERR;
  }
  return Err::m_nOK;
}

// Write 8x8 macroblock vectors
ErrVal WriteMotionVectorsToFile::writeMotionVector(UInt mvId,
                                                   UInt mvX, UInt mvY,
                                                   const std::string& mbType,
                                                   Short mv_l0_0_hor,
                                                   Short mv_l0_0_ver,
                                                   Short mv_l0_1_hor,
                                                   Short mv_l0_1_ver,
                                                   Short mv_l0_2_hor,
                                                   Short mv_l0_2_ver,
                                                   Short mv_l0_3_hor,
                                                   Short mv_l0_3_ver,
                                                   Short mv_l1_0_hor,
                                                   Short mv_l1_0_ver,
                                                   Short mv_l1_1_hor,
                                                   Short mv_l1_1_ver,
                                                   Short mv_l1_2_hor,
                                                   Short mv_l1_2_ver,
                                                   Short mv_l1_3_hor,
                                                   Short mv_l1_3_ver)
{
  if ( 0 > fprintf(motionVectorFile, "%-12u%3u,%-8u%-12s%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd\n", mvId, mvX, mvY, mbType.c_str(), mv_l0_0_hor, mv_l0_0_ver, mv_l0_1_hor, mv_l0_1_ver, mv_l0_2_hor, mv_l0_2_ver, mv_l0_3_hor, mv_l0_3_ver, mv_l1_0_hor, mv_l1_0_ver, mv_l1_1_hor, mv_l1_1_ver, mv_l1_2_hor, mv_l1_2_ver, mv_l1_3_hor, mv_l1_3_ver) )
  {
    return Err::m_nERR;
  }
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::writeBlankMb(UInt mvId, UInt mvX, UInt mvY, const std::string& mbType)
{
  if ( 0 > fprintf(motionVectorFile, "%-12u%3u,%-8u%-12s\n", mvId, mvX, mvY, mbType.c_str() ))
  {
    return Err::m_nERR;
  }
  return Err::m_nOK;
}

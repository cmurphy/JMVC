#include "WriteMotionVectorsToFile.h"

H264AVC_NAMESPACE_BEGIN

WriteMotionVectorsToFile::WriteMotionVectorsToFile()
{}

WriteMotionVectorsToFile::~WriteMotionVectorsToFile()
{}

ErrVal WriteMotionVectorsToFile::create( WriteMotionVectorsToFile *& rpcWriteMotionVectorsToFile)
{
  rpcWriteMotionVectorsToFile = new WriteMotionVectorsToFile;
  ROT ( NULL == rpcWriteMotionVectorsToFile );
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::init( const std::string& rcFileName )
{
  if( ! (m_fMotionVectorFile = fopen(rcFileName.c_str(), "w")))
  {
    std::cerr << "Failed to create motion vector file " << rcFileName.data() << std::endl;
    return Err::m_nERR;
  }
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::uninit()
{
  fclose(m_fMotionVectorFile);
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::destroy()
{
  RNOK ( uninit() );
  delete this;
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::initSlice(Int iPoc, enum SliceType cSliceType)
{
  m_iPoc = iPoc;
  m_cSliceType = cSliceType;
  printColumnHeader();
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::printColumnHeader()
{
  fprintf(m_fMotionVectorFile, "%-12s", "Pic Ord");
  fprintf(m_fMotionVectorFile, "%-12s", "Type");
  fprintf(m_fMotionVectorFile, "%-12s", "Mb Num");
  fprintf(m_fMotionVectorFile, "%-12s", "Mb Coord");
  fprintf(m_fMotionVectorFile, "%-12s", "Mb Type");
  fprintf(m_fMotionVectorFile, "%-36s", "List 0");
  fprintf(m_fMotionVectorFile, "%-36s\n", "List 1");
  return Err::m_nOK;
}

Char WriteMotionVectorsToFile::sliceToChar(SliceType type)
{
  return (type == P_SLICE) ? 'P' : (type == B_SLICE) ? 'B' : 'I';
}

ErrVal WriteMotionVectorsToFile::writeMotionVector(MbData& rMacroblock, UInt uiMbId, UInt uiMbX, UInt uiMbY)
{
  MbMode mbmode = rMacroblock.getMbMode();
  if (mbmode == MODE_16x16) {
    Mv mv_l0_16x16 = rMacroblock.getMbMvdData(LIST_0).getMv();
    Mv mv_l1_16x16 = rMacroblock.getMbMvdData(LIST_1).getMv();
    writeFullBlockVector(uiMbId, uiMbX, uiMbY, "16x16", mv_l0_16x16, mv_l1_16x16);
  } else if (mbmode == MODE_16x8) {
    Mv mv_l0_16x8_0 = rMacroblock.getMbMvdData(LIST_0).getMv(PART_16x8_0);
    Mv mv_l0_16x8_1 = rMacroblock.getMbMvdData(LIST_0).getMv(PART_16x8_1);
    Mv mv_l1_16x8_0 = rMacroblock.getMbMvdData(LIST_1).getMv(PART_16x8_0);
    Mv mv_l1_16x8_1 = rMacroblock.getMbMvdData(LIST_1).getMv(PART_16x8_1);
    writeHalfBlockVector(uiMbId, uiMbX, uiMbY, "16x8", mv_l0_16x8_0, mv_l0_16x8_1, mv_l1_16x8_0, mv_l1_16x8_1);
  } else if (mbmode == MODE_8x16){
    Mv mv_l0_8x16_0 = rMacroblock.getMbMvdData(LIST_0).getMv(PART_8x16_0);
    Mv mv_l0_8x16_1 = rMacroblock.getMbMvdData(LIST_0).getMv(PART_8x16_1);
    Mv mv_l1_8x16_0 = rMacroblock.getMbMvdData(LIST_1).getMv(PART_8x16_0);
    Mv mv_l1_8x16_1 = rMacroblock.getMbMvdData(LIST_1).getMv(PART_8x16_1);
    writeHalfBlockVector(uiMbId, uiMbX, uiMbY, "8x16", mv_l0_8x16_0, mv_l0_8x16_1, mv_l1_8x16_0, mv_l1_8x16_1);
  } else if (mbmode == MODE_8x8 || mbmode == MODE_8x8ref0) {
    std::string mode_str;
    if (mbmode == MODE_8x8) {
      mode_str = "8x8";
    } else {
      mode_str = "8x8ref0";
    }
    Mv mv_l0_8x8_0 = rMacroblock.getMbMvdData(LIST_0).getMv(PART_8x8_0);
    Mv mv_l0_8x8_1 = rMacroblock.getMbMvdData(LIST_0).getMv(PART_8x8_1);
    Mv mv_l0_8x8_2 = rMacroblock.getMbMvdData(LIST_0).getMv(PART_8x8_2);
    Mv mv_l0_8x8_3 = rMacroblock.getMbMvdData(LIST_0).getMv(PART_8x8_3);
    Mv mv_l1_8x8_0 = rMacroblock.getMbMvdData(LIST_1).getMv(PART_8x8_0);
    Mv mv_l1_8x8_1 = rMacroblock.getMbMvdData(LIST_1).getMv(PART_8x8_1);
    Mv mv_l1_8x8_2 = rMacroblock.getMbMvdData(LIST_1).getMv(PART_8x8_2);
    Mv mv_l1_8x8_3 = rMacroblock.getMbMvdData(LIST_1).getMv(PART_8x8_3);
    writeQuarterBlockVector(uiMbId, uiMbX, uiMbY, mode_str, mv_l0_8x8_0, mv_l0_8x8_1, mv_l0_8x8_2, mv_l0_8x8_3, mv_l1_8x8_0, mv_l1_8x8_1, mv_l1_8x8_2, mv_l1_8x8_3);
  } else if (mbmode == MODE_SKIP) {
    writeBlankMb(uiMbId, uiMbX, uiMbY, "Skip");
  } else { // Intra-coded
      Bool    bIntra16x16 = rMacroblock.isIntra16x16 ();
      Bool    bIntra8x8   = rMacroblock.isIntra4x4   () &&  rMacroblock.isTransformSize8x8();
      Bool    bIntra4x4   = rMacroblock.isIntra4x4   () && !rMacroblock.isTransformSize8x8();
      // If a block reports as multiple types of Intra coded there is a problem
      if ((bIntra16x16 && (bIntra8x8 || bIntra4x4)) || (bIntra8x8 && bIntra4x4)) {
        return Err::m_nERR;
      }
      std::string intraMode;
      if( bIntra16x16 ) {
        intraMode = "Intra16x16";
      } else if( bIntra8x8 ) {
        intraMode = "Intra8x8";
      } else if( bIntra4x4 ) {
        intraMode = "Intra4x4";
      } else { // If a block is neither inter-coded, nor one of these types of intra-coded, not a skip block, there is a problem
        return Err::m_nERR;
      }
      writeBlankMb(uiMbId, uiMbX, uiMbY, intraMode);
  }
  return Err::m_nOK;
}

// Write 16x16 macroblock vectors
ErrVal WriteMotionVectorsToFile::writeFullBlockVector(UInt uiMbId,
                                                      UInt uiMbX, UInt uiMbY,
                                                      const std::string& mbType,
                                                      Mv& mv_l0,
                                                      Mv& mv_l1)
{
  if ( 0 > fprintf(m_fMotionVectorFile, "%-12d%-12c%-12u%3u,%-8u%-12s%4hd,%-31hd%4hd,%-31hd\n", m_iPoc, sliceToChar(m_cSliceType), uiMbId, uiMbX, uiMbY, mbType.c_str(), mv_l0.getHor(), mv_l0.getVer(), mv_l1.getHor(), mv_l1.getVer()) ) {
    return Err::m_nERR;
  }
  return Err::m_nOK;
}

// Write 16x8 or 8x16 macroblock vectors
ErrVal WriteMotionVectorsToFile::writeHalfBlockVector(UInt uiMbId,
                                                      UInt uiMbX, UInt uiMbY,
                                                      const std::string& mbType,
                                                      Mv& mv_l0_0,
                                                      Mv& mv_l0_1,
                                                      Mv& mv_l1_0,
                                                      Mv& mv_l1_1)
{
  if ( 0 > fprintf(m_fMotionVectorFile, "%-12d%-12c%-12u%3u,%-8u%-12s%4hd,%-4hd%4hd,%-22hd%4hd,%-4hd%4hd,%-22hd\n", m_iPoc, sliceToChar(m_cSliceType), uiMbId, uiMbX, uiMbY, mbType.c_str(), mv_l0_0.getHor(), mv_l0_0.getVer(), mv_l0_1.getHor(), mv_l0_1.getVer(), mv_l1_0.getHor(), mv_l1_0.getVer(), mv_l1_1.getHor(), mv_l1_1.getVer()) ) {
    return Err::m_nERR;
  }
  return Err::m_nOK;
}

// Write 8x8 macroblock vectors
ErrVal WriteMotionVectorsToFile::writeQuarterBlockVector(UInt uiMbId,
                                                         UInt uiMbX, UInt uiMbY,
                                                         const std::string& mbType,
                                                         Mv& mv_l0_0,
                                                         Mv& mv_l0_1,
                                                         Mv& mv_l0_2,
                                                         Mv& mv_l0_3,
                                                         Mv& mv_l1_0,
                                                         Mv& mv_l1_1,
                                                         Mv& mv_l1_2,
                                                         Mv& mv_l1_3)
{
  if ( 0 > fprintf(m_fMotionVectorFile, "%-12d%-12c%-12u%3u,%-8u%-12s%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd%4hd,%-4hd\n", m_iPoc, sliceToChar(m_cSliceType), uiMbId, uiMbX, uiMbY, mbType.c_str(), mv_l0_0.getHor(), mv_l0_0.getVer(), mv_l0_1.getHor(), mv_l0_1.getVer(), mv_l0_2.getHor(), mv_l0_2.getVer(), mv_l0_3.getHor(), mv_l0_3.getVer(), mv_l1_0.getHor(), mv_l1_0.getVer(), mv_l1_1.getHor(), mv_l1_1.getVer(), mv_l1_2.getHor(), mv_l1_2.getVer(), mv_l1_3.getHor(), mv_l1_3.getVer()) ) {
    return Err::m_nERR;
  }
  return Err::m_nOK;
}

ErrVal WriteMotionVectorsToFile::writeBlankMb(UInt uiMbId, UInt uiMbX, UInt uiMbY, const std::string& mbType)
{
  if ( 0 > fprintf(m_fMotionVectorFile, "%-12d%-12c%-12u%3u,%-8u%-12s\n",m_iPoc, sliceToChar(m_cSliceType),  uiMbId, uiMbX, uiMbY, mbType.c_str() )) {
    return Err::m_nERR;
  }
  return Err::m_nOK;
}

H264AVC_NAMESPACE_END

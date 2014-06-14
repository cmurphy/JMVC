// Write human-readable motion vectors for every macroblock on every frame to a file.
// This code attempts to follow the model set by the WriteYuvToFile and WriteBitStreamToFile
// classes.

#if !defined(WRITE_MOTION_VECTORS_TO_FILE)
#define WRITE_MOTION_VECTORS_TO_FILE

#include <cstdio>
#include <stdlib.h>
#include "H264AVCCommonIf.h"
#include "H264AVCCommonLib.h"

H264AVC_NAMESPACE_BEGIN

class WriteMotionVectorsToFile
{
  public:
    WriteMotionVectorsToFile();
    ~WriteMotionVectorsToFile();
    static ErrVal create(WriteMotionVectorsToFile *& writeMotionVectorsToFile );
    ErrVal init( const std::string& rcFileName );
    ErrVal uninit();
    ErrVal destroy();
    ErrVal initSlice(Int iPoc, enum SliceType cSliceType);
    ErrVal writeMotionVector(MbData& rMacroblock, UInt uiMbId, UInt uiMbX, UInt uiMbY);

  private:
    ErrVal printColumnHeader();
    Char sliceToChar(SliceType type);
    ErrVal writeFullBlockVector(UInt uiMbId,
                                UInt uiMbX, UInt uiMbY,
                                const std::string& mbType,
                                Mv& mv_l0,
                                Mv& mv_l1);
    ErrVal writeHalfBlockVector(UInt uiMbId,
                                UInt uiMbX, UInt uiMbY,
                                const std::string& mbType,
                                Mv& mv_l0_0,
                                Mv& mv_l0_1,
                                Mv& mv_l1_0,
                                Mv& mv_l1_1);
    ErrVal writeQuarterBlockVector(UInt mvId,
                                   UInt uiMbX, UInt uiMbY,
                                   const std::string& mbType,
                                   Mv& mv_l0_0,
                                   Mv& mv_l0_1,
                                   Mv& mv_l0_2,
                                   Mv& mv_l0_3,
                                   Mv& mv_l1_0,
                                   Mv& mv_l1_1,
                                   Mv& mv_l1_2,
                                   Mv& mv_l1_3);
    ErrVal writeBlankMb(UInt uiMbId, UInt uiMbX, UInt uiMbY, const std::string& mbType);

    FILE * m_fMotionVectorFile; // Not using LargeFile since we want strings instead of binary
                                // Not using ofstream since fstream conflicts with defined
                                // min() and max() macros in H264AVCCommonIf.h
    Int m_iPoc; // Temp storage for current picture order count
    SliceType m_cSliceType; // Temp storage for current slice type
};

H264AVC_NAMESPACE_END

#endif

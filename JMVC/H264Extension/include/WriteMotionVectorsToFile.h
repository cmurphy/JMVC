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
    ErrVal writeMotionVector(UInt mvId,
                             UInt mvX, UInt mvY,
                             const std::string& mbType,
                             Short mv_l0_hor,
                             Short mv_l0_ver,
                             Short mv_l1_hor,
                             Short mv_l1_ver);
    ErrVal writeMotionVector(UInt mvId,
                             UInt mvX, UInt mvY,
                             const std::string& mbType,
                             Short mv_l0_0_hor,
                             Short mv_l0_0_ver,
                             Short mv_l0_1_hor,
                             Short mv_l0_1_ver,
                             Short mv_l1_0_hor,
                             Short mv_l1_0_ver,
                             Short mv_l1_1_hor,
                             Short mv_l1_1_ver);
    ErrVal writeMotionVector(UInt mvId,
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
                             Short mv_l1_3_ver);
    ErrVal writeBlankMb(UInt mvId, UInt mvX, UInt mvY, const std::string& mbType);

  private:
    ErrVal printColumnHeader();
    Char sliceToChar(SliceType type);

    FILE * m_fMotionVectorFile; // Not using LargeFile since we want strings instead of binary
                                // Not using ofstream since fstream conflicts with defined
                                // min() and max() macros in H264AVCCommonIf.h
    Int m_iPoc; // Temp storage for current picture order count
    SliceType m_cSliceType; // Temp storage for current slice type
};

H264AVC_NAMESPACE_END

#endif

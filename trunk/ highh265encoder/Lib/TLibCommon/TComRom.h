/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2013, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TComRom.h
    \brief    global variables & functions (header)
*/

#ifndef X265_TCOMROM_H
#define X265_TCOMROM_H

#include "CommonDef.h"

#include <iostream>

namespace x265 {
// private namespace

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Macros
// ====================================================================================================================

#define MAX_CU_DEPTH            6                           // log2(LCUSize)
#define MAX_CU_SIZE             (1 << (MAX_CU_DEPTH))       // maximum allowable size of CU
#define MIN_PU_SIZE             4
#define MIN_TU_SIZE             4
#define MAX_NUM_SPU_W           (MAX_CU_SIZE / MIN_PU_SIZE) // maximum number of SPU in horizontal line
#define ADI_BUF_STRIDE          (2 * MAX_CU_SIZE + 1 + 15)  // alignment to 16 bytes

// ====================================================================================================================
// Initialize / destroy functions
// ====================================================================================================================

void initROM();
void destroyROM();

// ====================================================================================================================
static const int chromaQPMappingTableSize = 58;

extern const uint8_t g_chromaScale[NUM_CHROMA_FORMAT][chromaQPMappingTableSize];
// Data structure related table & variable
// ====================================================================================================================

// flexible conversion from relative to absolute index
extern uint32_t g_zscanToRaster[MAX_NUM_SPU_W * MAX_NUM_SPU_W];
extern uint32_t g_rasterToZscan[MAX_NUM_SPU_W * MAX_NUM_SPU_W];
extern uint16_t*  g_scanOrder[SCAN_NUMBER_OF_GROUP_TYPES][SCAN_NUMBER_OF_TYPES][MAX_CU_DEPTH];
void initZscanToRaster(int maxDepth, int depth, uint32_t startVal, uint32_t*& curIdx);
void initRasterToZscan(uint32_t maxCUSize, uint32_t maxCUDepth);

// conversion of partition index to picture pel position
extern uint32_t g_rasterToPelX[MAX_NUM_SPU_W * MAX_NUM_SPU_W];
extern uint32_t g_rasterToPelY[MAX_NUM_SPU_W * MAX_NUM_SPU_W];

void initRasterToPelXY(uint32_t maxCUSize, uint32_t maxCUDepth);

// global variable (LCU width/height, max. CU depth)
extern uint32_t g_maxCUSize;
extern uint32_t g_maxCUDepth;
extern uint32_t g_addCUDepth;

#define MAX_TS_WIDTH  4
#define MAX_TS_HEIGHT 4

extern const uint32_t g_puOffset[8];

#define QUANT_IQUANT_SHIFT    20 // Q(QP%6) * IQ(QP%6) = 2^20
#define QUANT_SHIFT           14 // Q(4) = 2^14
#define SCALE_BITS            15 // Inherited from TMuC, presumably for fractional bit estimates in RDOQ
#define MAX_TR_DYNAMIC_RANGE  15 // Maximum transform dynamic range (excluding sign bit)

#define SHIFT_INV_1ST          7 // Shift after first inverse transform stage
#define SHIFT_INV_2ND         12 // Shift after second inverse transform stage

extern const int g_quantScales[6];     // Q(QP%6)
extern const int g_invQuantScales[6];  // IQ(QP%6)
extern const int16_t g_t4[4][4];
extern const int16_t g_t8[8][8];
extern const int16_t g_t16[16][16];
extern const int16_t g_t32[32][32];

// ====================================================================================================================
// Subpel interpolation defines and constants
// ====================================================================================================================

#define NTAPS_LUMA        8                            ///< Number of taps for luma
#define NTAPS_CHROMA      4                            ///< Number of taps for chroma
#define IF_INTERNAL_PREC 14                            ///< Number of bits for internal precision
#define IF_FILTER_PREC    6                            ///< Log2 of sum of filter taps
#define IF_INTERNAL_OFFS (1 << (IF_INTERNAL_PREC - 1)) ///< Offset used internally

extern const int16_t g_lumaFilter[4][NTAPS_LUMA];     ///< Luma filter taps
extern const int16_t g_chromaFilter[8][NTAPS_CHROMA]; ///< Chroma filter taps

// ====================================================================================================================
// Scanning order & context mapping table
// ====================================================================================================================

//extern const uint8_t g_groupIdx[32];
static inline uint32_t getGroupIdx(const uint32_t idx)
{
    uint32_t group = (idx >> 3);
    if (idx >= 24)
        group = 2;
    uint32_t groupIdx = ((idx >> (group + 1)) - 2) + 4 + (group << 1);
    if (idx <= 3)
        groupIdx = idx;

#ifdef _DEBUG
    static const uint8_t g_groupIdx[32]   = { 0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9 };
    assert(groupIdx == g_groupIdx[idx]);
#endif

    return groupIdx;
}

extern const uint8_t g_minInGroup[10];

extern const uint8_t g_goRiceRange[5];      //!< maximum value coded with Rice codes
//extern const uint8_t g_goRicePrefixLen[5];  //!< prefix length for each maximum value

// ====================================================================================================================
// Misc.
// ====================================================================================================================

extern uint8_t g_convertToBit[MAX_CU_SIZE + 1]; // from width to log2(width)-2

#ifndef ENC_DEC_TRACE
# define ENC_DEC_TRACE 0
#endif

#if ENC_DEC_TRACE
extern FILE*  g_hTrace;
extern bool   g_bJustDoIt;
extern const bool g_bEncDecTraceEnable;
extern const bool g_bEncDecTraceDisable;
extern bool   g_HLSTraceEnable;
extern uint64_t g_nSymbolCounter;

#define COUNTER_START    1
#define COUNTER_END      0 //( uint64_t(1) << 63 )

#define DTRACE_CABAC_F(x)     if ((g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END) || g_bJustDoIt) fprintf(g_hTrace, "%f", x);
#define DTRACE_CABAC_V(x)     if ((g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END) || g_bJustDoIt) fprintf(g_hTrace, "%d", x);
#define DTRACE_CABAC_VL(x)    if ((g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END) || g_bJustDoIt) fprintf(g_hTrace, "%ld", x);
#define DTRACE_CABAC_T(x)     if ((g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END) || g_bJustDoIt) fprintf(g_hTrace, "%s", x);
#define DTRACE_CABAC_X(x)     if ((g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END) || g_bJustDoIt) fprintf(g_hTrace, "%x", x);
#define DTRACE_CABAC_R(x, y)  if ((g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END) || g_bJustDoIt) fprintf(g_hTrace, x,    y);
#define DTRACE_CABAC_N        if ((g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END) || g_bJustDoIt) fprintf(g_hTrace, "\n");
#else // if ENC_DEC_TRACE

#define DTRACE_CABAC_F(x)
#define DTRACE_CABAC_V(x)
#define DTRACE_CABAC_VL(x)
#define DTRACE_CABAC_T(x)
#define DTRACE_CABAC_X(x)
#define DTRACE_CABAC_R(x, y)
#define DTRACE_CABAC_N

#endif // if ENC_DEC_TRACE

#define SCALING_LIST_NUM 6         ///< list number for quantization matrix
#define SCALING_LIST_NUM_32x32 2   ///< list number for quantization matrix 32x32
#define SCALING_LIST_REM_NUM 6     ///< remainder of QP/6
#define SCALING_LIST_START_VALUE 8 ///< start value for dpcm mode
#define MAX_MATRIX_COEF_NUM 64     ///< max coefficient number for quantization matrix
#define MAX_MATRIX_SIZE_NUM 8      ///< max size number for quantization matrix
#define SCALING_LIST_DC 16         ///< default DC value
enum ScalingListSize
{
    SCALING_LIST_4x4 = 0,
    SCALING_LIST_8x8,
    SCALING_LIST_16x16,
    SCALING_LIST_32x32,
    SCALING_LIST_SIZE_NUM
};

static const char MatrixType[4][6][20] =
{
    {
        "INTRA4X4_LUMA",
        "INTRA4X4_CHROMAU",
        "INTRA4X4_CHROMAV",
        "INTER4X4_LUMA",
        "INTER4X4_CHROMAU",
        "INTER4X4_CHROMAV"
    },
    {
        "INTRA8X8_LUMA",
        "INTRA8X8_CHROMAU",
        "INTRA8X8_CHROMAV",
        "INTER8X8_LUMA",
        "INTER8X8_CHROMAU",
        "INTER8X8_CHROMAV"
    },
    {
        "INTRA16X16_LUMA",
        "INTRA16X16_CHROMAU",
        "INTRA16X16_CHROMAV",
        "INTER16X16_LUMA",
        "INTER16X16_CHROMAU",
        "INTER16X16_CHROMAV"
    },
    {
        "INTRA32X32_LUMA",
        "INTER32X32_LUMA",
    },
};
static const char MatrixType_DC[4][12][22] =
{
    {},
    {},
    {
        "INTRA16X16_LUMA_DC",
        "INTRA16X16_CHROMAU_DC",
        "INTRA16X16_CHROMAV_DC",
        "INTER16X16_LUMA_DC",
        "INTER16X16_CHROMAU_DC",
        "INTER16X16_CHROMAV_DC"
    },
    {
        "INTRA32X32_LUMA_DC",
        "INTER32X32_LUMA_DC",
    },
};
extern int g_quantIntraDefault8x8[64];
extern int g_quantIntraDefault16x16[256];
extern int g_quantIntraDefault32x32[1024];
extern int g_quantInterDefault8x8[64];
extern int g_quantInterDefault16x16[256];
extern int g_quantInterDefault32x32[1024];
extern int g_quantTSDefault4x4[16];
extern const uint32_t g_scalingListSize[SCALING_LIST_SIZE_NUM];
extern const uint32_t g_scalingListSizeX[SCALING_LIST_SIZE_NUM];
extern const uint32_t g_scalingListNum[SCALING_LIST_SIZE_NUM];
//! \}

// Map Luma samples to chroma samples
extern const int g_winUnitX[MAX_CHROMA_FORMAT_IDC + 1];
extern const int g_winUnitY[MAX_CHROMA_FORMAT_IDC + 1];
extern const double x265_lambda2_tab_I[MAX_MAX_QP + 1];
extern const double x265_lambda2_non_I[MAX_MAX_QP + 1];
// CABAC tables
extern const uint8_t g_lpsTable[64][4];
extern const uint8_t x265_exp2_lut[64];
}

#endif  //ifndef X265_TCOMROM_H

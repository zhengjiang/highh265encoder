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

/** \file     TComTrQuant.h
    \brief    transform and quantization class (header)
*/

#ifndef X265_TCOMTRQUANT_H
#define X265_TCOMTRQUANT_H

#include "CommonDef.h"
#include "TComYuv.h"
#include "TComDataCU.h"
#include "ContextTables.h"

namespace x265 {
// private namespace

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define QP_BITS 15

// ====================================================================================================================
// Type definition
// ====================================================================================================================

typedef struct
{
    int significantCoeffGroupBits[NUM_SIG_CG_FLAG_CTX][2];
    uint32_t significantBits[NUM_SIG_FLAG_CTX][2];
    int lastXBits[10];
    int lastYBits[10];
    int greaterOneBits[NUM_ONE_FLAG_CTX][2];
    int levelAbsBits[NUM_ABS_FLAG_CTX][2];

    int blockCbpBits[NUM_QT_CBF_CTX][2];
    int blockRootCbpBits[NUM_QT_ROOT_CBF_CTX][2];
} estBitsSbacStruct;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class QpParam
{
public:

    QpParam() {}

    int m_qp;
    int m_per;
    int m_rem;

    int m_bits;

public:

    void setQpParam(int qpScaled)
    {
        m_qp   = qpScaled;
        m_per  = qpScaled / 6;
        m_rem  = qpScaled % 6;
        m_bits = QP_BITS + m_per;
    }

    void clear()
    {
        m_qp   = 0;
        m_per  = 0;
        m_rem  = 0;
        m_bits = 0;
    }

    int per()   const { return m_per; }

    int rem()   const { return m_rem; }

    int bits()  const { return m_bits; }

    int qp() { return m_qp; }
};

/// transform and quantization class
class TComTrQuant
{
public:

    TComTrQuant();
    ~TComTrQuant();

    // initialize class
    void init(uint32_t maxTrSize, int useRDOQ, int useRDOQTS, int useTransformSkipFast);

    // transform & inverse transform functions
    uint32_t transformNxN(TComDataCU* cu, int16_t* residual, uint32_t stride, coeff_t* coeff, uint32_t trSize,
                          TextType ttype, uint32_t absPartIdx, int32_t* lastPos, bool useTransformSkip = false, bool curUseRDOQ = true);

    void invtransformNxN(bool transQuantBypass, uint32_t mode, int16_t* residual, uint32_t stride, coeff_t* coeff, uint32_t trSize, int scalingListType, bool useTransformSkip = false, int lastPos = MAX_INT);

    // Misc functions
    void setQPforQuant(int qpy, TextType ttype, int qpBdOffset, int chromaQPOffset, int chFmt);
    void setLambda(double lambdaLuma, double lambdaChroma) { m_lumaLambda = lambdaLuma; m_chromaLambda = lambdaChroma; }

    void selectLambda(TextType ttype) { m_lambda = (ttype == TEXT_LUMA) ? m_lumaLambda : m_chromaLambda; }

    void initScalingList();
    void destroyScalingList();
    void setErrScaleCoeff(uint32_t list, uint32_t size, uint32_t qp);
    double* getErrScaleCoeff(uint32_t list, uint32_t size, uint32_t qp) { return m_errScale[size][list][qp]; }   //!< get Error Scale Coefficent

    int32_t* getQuantCoeff(uint32_t list, uint32_t qp, uint32_t size) { return m_quantCoef[size][list][qp]; }        //!< get Quant Coefficent

    int32_t* getDequantCoeff(uint32_t list, uint32_t qp, uint32_t size) { return m_dequantCoef[size][list][qp]; }    //!< get DeQuant Coefficent

    void setUseScalingList(bool bUseScalingList) { m_scalingListEnabledFlag = bUseScalingList; }

    bool getUseScalingList() { return m_scalingListEnabledFlag; }

    void setFlatScalingList();
    void xsetFlatScalingList(uint32_t list, uint32_t size, uint32_t qp);
    void xSetScalingListEnc(TComScalingList *scalingList, uint32_t list, uint32_t size, uint32_t qp);
    void xSetScalingListDec(TComScalingList *scalingList, uint32_t list, uint32_t size, uint32_t qp);
    void setScalingList(TComScalingList *scalingList);
    void processScalingListEnc(int32_t *coeff, int32_t *quantcoeff, int quantScales, uint32_t height, uint32_t width, uint32_t ratio, int sizuNum, uint32_t dc);
    void processScalingListDec(int32_t *coeff, int32_t *dequantcoeff, int invQuantScales, uint32_t height, uint32_t width, uint32_t ratio, int sizuNum, uint32_t dc);
    static uint32_t calcPatternSigCtx(const uint64_t sigCoeffGroupFlag64, uint32_t cgPosX, uint32_t cgPosY, uint32_t log2TrSizeCG);
    static uint32_t getSigCtxInc(uint32_t patternSigCtx, const uint32_t log2TrSize, const uint32_t trSize, const uint32_t blkPos, const TextType ctype, const uint32_t firstSignificanceMapContext);
    static uint32_t getSigCoeffGroupCtxInc(const uint64_t sigCoeffGroupFlag64, uint32_t cgPosX, uint32_t cgPosY, const uint32_t log2TrSizeCG);
    inline static void getTUEntropyCodingParameters(TComDataCU* cu, TUEntropyCodingParameters &result, uint32_t absPartIdx, uint32_t log2TrSize, TextType ttype)
    {
        //set the group layout
        const uint32_t log2TrSizeCG = log2TrSize - MLS_CG_LOG2_SIZE;
        result.log2TrSizeCG = log2TrSizeCG;

        //set the scan orders
        result.scanType = COEFF_SCAN_TYPE(cu->getCoefScanIdx(absPartIdx, log2TrSize, ttype == TEXT_LUMA, cu->isIntra(absPartIdx)));
        result.scan   = g_scanOrder[SCAN_GROUPED_4x4][result.scanType][log2TrSize];
        result.scanCG = g_scanOrder[SCAN_UNGROUPED][result.scanType][log2TrSizeCG];

        //set the significance map context selection parameters
        TextType ctype = (ttype == TEXT_LUMA) ? TEXT_LUMA : TEXT_CHROMA;
        if (log2TrSize == 2)
        {
            result.firstSignificanceMapContext = 0;
            assert(significanceMapContextSetStart[ctype][CONTEXT_TYPE_4x4] == 0);
        }
        else if (log2TrSize == 3)
        {
            result.firstSignificanceMapContext = 9;
            assert(significanceMapContextSetStart[ctype][CONTEXT_TYPE_8x8] == 9);
            if (result.scanType != SCAN_DIAG && !ctype)
            {
                result.firstSignificanceMapContext += 6;
                assert(nonDiagonalScan8x8ContextOffset[ctype] == 6);
            }
        }
        else
        {
            result.firstSignificanceMapContext = (ctype ? 12 : 21);
            assert(significanceMapContextSetStart[ctype][CONTEXT_TYPE_NxN] == (uint32_t)(ctype ? 12 : 21));
        }
    }
    estBitsSbacStruct* m_estBitsSbac;

protected:

    QpParam  m_qpParam;

    double   m_lambda;
    double   m_lumaLambda;
    double   m_chromaLambda;

    uint32_t     m_maxTrSize;
    bool     m_useRDOQ;
    bool     m_useRDOQTS;
    bool     m_useTransformSkipFast;
    bool     m_scalingListEnabledFlag;

    int32_t*     m_tmpCoeff;
    int32_t*     m_quantCoef[SCALING_LIST_SIZE_NUM][SCALING_LIST_NUM][SCALING_LIST_REM_NUM];     ///< array of quantization matrix coefficient 4x4
    int32_t*     m_dequantCoef[SCALING_LIST_SIZE_NUM][SCALING_LIST_NUM][SCALING_LIST_REM_NUM];   ///< array of dequantization matrix coefficient 4x4

    double  *m_errScale[SCALING_LIST_SIZE_NUM][SCALING_LIST_NUM][SCALING_LIST_REM_NUM];

private:

    void xTransformSkip(int16_t* resiBlock, uint32_t stride, int32_t* coeff, int trSize);
    void signBitHidingHDQ(coeff_t* qcoeff, coeff_t* coeff, int32_t* deltaU, const TUEntropyCodingParameters &codingParameters);
    uint32_t xQuant(TComDataCU* cu, int32_t* src, coeff_t* dst, int trSize, TextType ttype, uint32_t absPartIdx, int32_t *lastPos, bool curUseRDOQ = true);

    // RDOQ functions
    uint32_t xRateDistOptQuant(TComDataCU* cu, int32_t* srcCoeff, coeff_t* dstCoeff, uint32_t trSize, TextType ttype, uint32_t absPartIdx, int32_t *lastPos);

    inline uint32_t xGetCodedLevel(double& codedCost, const double curCostSig, double& codedCostSig, int levelDouble,
                                   uint32_t maxAbsLevel, uint32_t baseLevel, const int *greaterOneBits, const int *levelAbsBits, uint32_t absGoRice,
                                   uint32_t c1c2Idx, int qbits, double scale, bool bLast) const;

    inline double xGetICRateCost(uint32_t absLevel, int32_t  diffLevel, const int *greaterOneBits, const int *levelAbsBits, uint32_t absGoRice, uint32_t c1c2Idx) const;

    inline int    xGetICRate(uint32_t absLevel, int32_t diffLevel, const int *greaterOneBits, const int *levelAbsBits, uint32_t absGoRice, uint32_t c1c2Idx) const;

    inline double xGetRateLast(uint32_t posx, uint32_t posy) const;

    inline double xGetRateSigCoeffGroup(uint16_t sigCoeffGroup, uint16_t ctxNumSig) const { return m_lambda * m_estBitsSbac->significantCoeffGroupBits[ctxNumSig][sigCoeffGroup]; }

    inline double xGetRateSigCoef(uint32_t sig, uint32_t ctxNumSig) const { return m_lambda * m_estBitsSbac->significantBits[ctxNumSig][sig]; }

    inline double xGetICost(double rate) const { return m_lambda * rate; } ///< Get the cost for a specific rate

    inline uint32_t xGetIEPRate() const          { return 32768; }            ///< Get the cost of an equal probable bit

    void xITransformSkip(int32_t* coeff, int16_t* residual, uint32_t stride, int trSize);
};
}
//! \}

#endif // ifndef X265_TCOMTRQUANT_H

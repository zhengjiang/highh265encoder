/*****************************************************************************
 * Copyright (C) 2013 x265 project
 *
 * Authors: Chung Shin Yee <shinyee@multicorewareinc.com>
 *          Min Chen <chenm003@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@multicorewareinc.com.
 *****************************************************************************/

#include "encoder.h"
#include "PPA/ppa.h"
#include "framefilter.h"
#include "wavefront.h"

using namespace x265;

static uint64_t computeSSD(pixel *fenc, pixel *rec, int stride, int width, int height);
static float calculateSSIM(pixel *pix1, intptr_t stride1, pixel *pix2, intptr_t stride2, int width, int height, void *buf, int32_t *cnt);

// **************************************************************************
// * LoopFilter
// **************************************************************************
FrameFilter::FrameFilter()
    : m_param(NULL)
    , m_rdGoOnBinCodersCABAC(true)
    , m_ssimBuf(NULL)
{
}

void FrameFilter::destroy()
{
    if (m_param->bEnableLoopFilter)
    {
        m_loopFilter.destroy();
    }

    if (m_param->bEnableSAO)
    {
        // NOTE: I don't check sao flag since loopfilter and sao have same control status
        m_sao.destroy();
        m_sao.destroyEncBuffer();
    }
    X265_FREE(m_ssimBuf);
}

void FrameFilter::init(Encoder *top, int numRows, TEncSbac* rdGoOnSbacCoder)
{
    m_param = top->param;
    m_numRows = numRows;
    m_hChromaShift = CHROMA_H_SHIFT(m_param->internalCsp);
    m_vChromaShift = CHROMA_V_SHIFT(m_param->internalCsp);

    // NOTE: for sao only, I write this code because I want to exact match with HM's bug bitstream
    m_rdGoOnSbacCoderRow0 = rdGoOnSbacCoder;

    if (top->param->bEnableLoopFilter)
    {
        m_loopFilter.create(g_maxCUDepth);
    }

    if (top->param->bEnableSAO)
    {
        m_sao.setSaoLcuBoundary(top->param->saoLcuBoundary);
        m_sao.setSaoLcuBasedOptimization(top->param->saoLcuBasedOptimization);
        m_sao.setMaxNumOffsetsPerPic(top->m_maxNumOffsetsPerPic);
        m_sao.create(top->param->sourceWidth, top->param->sourceHeight, g_maxCUSize, g_maxCUSize, m_param->internalCsp);
        m_sao.createEncBuffer();
    }

    if (m_param->bEnableSsim)
        m_ssimBuf = (int*)x265_malloc(sizeof(int) * 8 * (m_param->sourceWidth / 4 + 3));
}

void FrameFilter::start(TComPic *pic)
{
    m_pic = pic;

    m_saoRowDelay = m_param->bEnableLoopFilter ? 1 : 0;
    m_loopFilter.setCfg(pic->getSlice()->getPPS()->getLoopFilterAcrossTilesEnabledFlag());
    m_rdGoOnSbacCoder.init(&m_rdGoOnBinCodersCABAC);
    m_entropyCoder.setEntropyCoder(&m_rdGoOnSbacCoder, pic->getSlice());
    m_entropyCoder.setBitstream(&m_bitCounter);
    m_rdGoOnBinCodersCABAC.m_fracBits = 0;

    if (m_param->bEnableSAO)
    {
        m_sao.resetStats();
        m_sao.createPicSaoInfo(pic);

        SAOParam* saoParam = pic->getPicSym()->getSaoParam();
        m_sao.resetSAOParam(saoParam);
        m_sao.rdoSaoUnitRowInit(saoParam);

        // NOTE: Disable SAO automatic turn-off when frame parallelism is
        // enabled for output exact independent of frame thread count
        if (m_param->frameNumThreads > 1)
        {
            saoParam->bSaoFlag[0] = true;
            saoParam->bSaoFlag[1] = true;
        }
    }
}

void FrameFilter::end()
{
}

void FrameFilter::processRow(int row, Encoder* cfg)
{
    PPAScopeEvent(Thread_filterCU);

    if (!m_param->bEnableLoopFilter && !m_param->bEnableSAO)
    {
        processRowPost(row, cfg);
        return;
    }

    // NOTE: We are here only active both of loopfilter and sao, the row 0 always finished, so we can safe to copy row[0]'s data
    if (row == 0 && m_param->bEnableSAO)
    {
        // NOTE: not need, seems HM's bug, I want to keep output exact matched.
        m_rdGoOnBinCodersCABAC.m_fracBits = ((TEncBinCABAC*)((TEncSbac*)m_rdGoOnSbacCoderRow0->m_binIf))->m_fracBits;
        m_sao.startSaoEnc(m_pic, &m_entropyCoder, &m_rdGoOnSbacCoder);
    }

    const uint32_t numCols = m_pic->getPicSym()->getFrameWidthInCU();
    const uint32_t lineStartCUAddr = row * numCols;

    // SAO parameter estimation using non-deblocked pixels for LCU bottom and right boundary areas
    if (m_param->bEnableSAO && m_param->saoLcuBasedOptimization && m_param->saoLcuBoundary)
    {
        m_sao.calcSaoStatsRowCus_BeforeDblk(m_pic, row);
    }

    if (m_param->bEnableLoopFilter)
    {
        for (uint32_t col = 0; col < numCols; col++)
        {
            const uint32_t cuAddr = lineStartCUAddr + col;
            TComDataCU* cu = m_pic->getCU(cuAddr);

            m_loopFilter.loopFilterCU(cu, EDGE_VER);

            if (col > 0)
            {
                TComDataCU* cu_prev = m_pic->getCU(cuAddr - 1);
                m_loopFilter.loopFilterCU(cu_prev, EDGE_HOR);
            }
        }

        {
            TComDataCU* cu_prev = m_pic->getCU(lineStartCUAddr + numCols - 1);
            m_loopFilter.loopFilterCU(cu_prev, EDGE_HOR);
        }
    }

    // SAO
    SAOParam* saoParam = m_pic->getPicSym()->getSaoParam();
    if (m_param->bEnableSAO && m_sao.getSaoLcuBasedOptimization())
    {
        m_sao.rdoSaoUnitRow(saoParam, row);

        // NOTE: Delay a row because SAO decide need top row pixels at next row, is it HM's bug?
        if (row >= m_saoRowDelay)
        {
            processSao(row - m_saoRowDelay);
        }
    }

    // this row of CTUs has been encoded

    // NOTE: in --sao-lcu-opt=0 mode, we do it later
    if (m_param->bEnableSAO && !m_sao.getSaoLcuBasedOptimization())
        return;

    if (row > 0)
    {
        processRowPost(row - 1, cfg);
    }

    if (row == m_numRows - 1)
    {
        if (m_param->bEnableSAO && m_sao.getSaoLcuBasedOptimization())
        {
            m_sao.rdoSaoUnitRowEnd(saoParam, m_pic->getNumCUsInFrame());

            for (int i = m_numRows - m_saoRowDelay; i < m_numRows; i++)
            {
                processSao(i);
            }
        }

        processRowPost(row, cfg);
    }
}

void FrameFilter::processRowPost(int row, Encoder* cfg)
{
    const uint32_t numCols = m_pic->getPicSym()->getFrameWidthInCU();
    const uint32_t lineStartCUAddr = row * numCols;
    TComPicYuv *recon = m_pic->getPicYuvRec();
    const int lastH = ((recon->getHeight() % g_maxCUSize) ? (recon->getHeight() % g_maxCUSize) : g_maxCUSize);
    const int realH = (row != m_numRows - 1) ? g_maxCUSize : lastH;

    // Border extend Left and Right
    primitives.extendRowBorder(recon->getLumaAddr(lineStartCUAddr), recon->getStride(), recon->getWidth(), realH, recon->getLumaMarginX());
    primitives.extendRowBorder(recon->getCbAddr(lineStartCUAddr), recon->getCStride(), recon->getWidth() >> m_hChromaShift, realH >> m_vChromaShift, recon->getChromaMarginX());
    primitives.extendRowBorder(recon->getCrAddr(lineStartCUAddr), recon->getCStride(), recon->getWidth() >> m_hChromaShift, realH >> m_vChromaShift, recon->getChromaMarginX());
    // Border extend Top
    if (row == 0)
    {
        const intptr_t stride = recon->getStride();
        const intptr_t strideC = recon->getCStride();
        pixel *pixY = recon->getLumaAddr(lineStartCUAddr) - recon->getLumaMarginX();
        pixel *pixU = recon->getCbAddr(lineStartCUAddr) - recon->getChromaMarginX();
        pixel *pixV = recon->getCrAddr(lineStartCUAddr) - recon->getChromaMarginX();

        for (int y = 0; y < recon->getLumaMarginY(); y++)
        {
            memcpy(pixY - (y + 1) * stride, pixY, stride * sizeof(pixel));
        }

        for (int y = 0; y < recon->getChromaMarginY(); y++)
        {
            memcpy(pixU - (y + 1) * strideC, pixU, strideC * sizeof(pixel));
            memcpy(pixV - (y + 1) * strideC, pixV, strideC * sizeof(pixel));
        }
    }

    // Border extend Bottom
    if (row == m_numRows - 1)
    {
        const intptr_t stride = recon->getStride();
        const intptr_t strideC = recon->getCStride();
        pixel *pixY = recon->getLumaAddr(lineStartCUAddr) - recon->getLumaMarginX() + (realH - 1) * stride;
        pixel *pixU = recon->getCbAddr(lineStartCUAddr) - recon->getChromaMarginX() + ((realH >> m_vChromaShift) - 1) * strideC;
        pixel *pixV = recon->getCrAddr(lineStartCUAddr) - recon->getChromaMarginX() + ((realH >> m_vChromaShift) - 1) * strideC;
        for (int y = 0; y < recon->getLumaMarginY(); y++)
        {
            memcpy(pixY + (y + 1) * stride, pixY, stride * sizeof(pixel));
        }

        for (int y = 0; y < recon->getChromaMarginY(); y++)
        {
            memcpy(pixU + (y + 1) * strideC, pixU, strideC * sizeof(pixel));
            memcpy(pixV + (y + 1) * strideC, pixV, strideC * sizeof(pixel));
        }
    }

    // Notify other FrameEncoders that this row of reconstructed pixels is available
    m_pic->m_reconRowCount.incr();

    int cuAddr = lineStartCUAddr;
    if (m_param->bEnablePsnr)
    {
        TComPicYuv* orig  = m_pic->getPicYuvOrg();

        int stride = recon->getStride();
        int width  = recon->getWidth() - cfg->m_pad[0];
        int height;

        if (row == m_numRows - 1)
            height = ((recon->getHeight() % g_maxCUSize) ? (recon->getHeight() % g_maxCUSize) : g_maxCUSize);
        else
            height = g_maxCUSize;

        uint64_t ssdY = computeSSD(orig->getLumaAddr(cuAddr), recon->getLumaAddr(cuAddr), stride, width, height);
        height >>= m_vChromaShift;
        width  >>= m_hChromaShift;
        stride = recon->getCStride();

        uint64_t ssdU = computeSSD(orig->getCbAddr(cuAddr), recon->getCbAddr(cuAddr), stride, width, height);
        uint64_t ssdV = computeSSD(orig->getCrAddr(cuAddr), recon->getCrAddr(cuAddr), stride, width, height);

        m_pic->m_SSDY += ssdY;
        m_pic->m_SSDU += ssdU;
        m_pic->m_SSDV += ssdV;
    }
    if (m_param->bEnableSsim && m_ssimBuf)
    {
        pixel *rec = (pixel*)m_pic->getPicYuvRec()->getLumaAddr();
        pixel *org = (pixel*)m_pic->getPicYuvOrg()->getLumaAddr();
        int stride1 = m_pic->getPicYuvOrg()->getStride();
        int stride2 = m_pic->getPicYuvRec()->getStride();
        int bEnd = ((row + 1) == (this->m_numRows - 1));
        int bStart = (row == 0);
        int minPixY = row * g_maxCUSize - 4 * !bStart;
        int maxPixY = (row + 1) * g_maxCUSize - 4 * !bEnd;
        int ssim_cnt;
        x265_emms();

        /* SSIM is done for each row in blocks of 4x4 . The First blocks are offset by 2 pixels to the right
        * to avoid alignment of ssim blocks with DCT blocks. */
        minPixY += bStart ? 2 : -6;
        m_pic->m_ssim += calculateSSIM(rec + 2 + minPixY * stride1, stride1, org + 2 + minPixY * stride2, stride2,
                                       m_param->sourceWidth - 2, maxPixY - minPixY, m_ssimBuf, &ssim_cnt);
        m_pic->m_ssimCnt += ssim_cnt;
    }
    if (m_param->decodedPictureHashSEI == 1)
    {
        uint32_t width = recon->getWidth();
        uint32_t height = recon->getCUHeight(row);
        uint32_t stride = recon->getStride();

        if (row == 0)
        {
            for (int i = 0; i < 3; i++)
            {
                MD5Init(&(m_pic->m_state[i]));
            }
        }

        updateMD5Plane(m_pic->m_state[0], recon->getLumaAddr(cuAddr), width, height, stride);
        width  >>= m_hChromaShift;
        height >>= m_vChromaShift;
        stride = recon->getCStride();

        updateMD5Plane(m_pic->m_state[1], recon->getCbAddr(cuAddr), width, height, stride);

        updateMD5Plane(m_pic->m_state[2], recon->getCrAddr(cuAddr), width, height, stride);
    }
    else if (m_param->decodedPictureHashSEI == 2)
    {
        uint32_t width = recon->getWidth();
        uint32_t height = recon->getCUHeight(row);
        uint32_t stride = recon->getStride();
        if (row == 0)
        {
            m_pic->m_crc[0] = m_pic->m_crc[1] = m_pic->m_crc[2] = 0xffff;
        }
        updateCRC(recon->getLumaAddr(cuAddr), m_pic->m_crc[0], height, width, stride);
        width  >>= m_hChromaShift;
        height >>= m_vChromaShift;
        stride = recon->getCStride();

        updateCRC(recon->getCbAddr(cuAddr), m_pic->m_crc[1], height, width, stride);
        updateCRC(recon->getCrAddr(cuAddr), m_pic->m_crc[2], height, width, stride);
    }
    else if (m_param->decodedPictureHashSEI == 3)
    {
        uint32_t width = recon->getWidth();
        uint32_t height = recon->getCUHeight(row);
        uint32_t stride = recon->getStride();
        uint32_t cuHeight = g_maxCUSize;
        if (row == 0)
        {
            m_pic->m_checksum[0] = m_pic->m_checksum[1] = m_pic->m_checksum[2] = 0;
        }
        updateChecksum(recon->getLumaAddr(), m_pic->m_checksum[0], height, width, stride, row, cuHeight);
        width  >>= m_hChromaShift;
        height >>= m_vChromaShift;
        stride = recon->getCStride();
        cuHeight >>= m_vChromaShift;
        updateChecksum(recon->getCbAddr(), m_pic->m_checksum[1], height, width, stride, row, cuHeight);
        updateChecksum(recon->getCrAddr(), m_pic->m_checksum[2], height, width, stride, row, cuHeight);
    }
}

static uint64_t computeSSD(pixel *fenc, pixel *rec, int stride, int width, int height)
{
    uint64_t ssd = 0;

    if ((width | height) & 3)
    {
        /* Slow Path */
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int diff = (int)(fenc[x] - rec[x]);
                ssd += diff * diff;
            }

            fenc += stride;
            rec += stride;
        }

        return ssd;
    }

    int y = 0;
    /* Consume Y in chunks of 64 */
    for (; y + 64 <= height; y += 64)
    {
        int x = 0;

        if (!(stride & 31))
            for (; x + 64 <= width; x += 64)
            {
                ssd += primitives.sse_pp[LUMA_64x64](fenc + x, stride, rec + x, stride);
            }

        if (!(stride & 15))
            for (; x + 16 <= width; x += 16)
            {
                ssd += primitives.sse_pp[LUMA_16x64](fenc + x, stride, rec + x, stride);
            }

        for (; x + 4 <= width; x += 4)
        {
            ssd += primitives.sse_pp[LUMA_4x16](fenc + x, stride, rec + x, stride);
            ssd += primitives.sse_pp[LUMA_4x16](fenc + x + 16 * stride, stride, rec + x + 16 * stride, stride);
            ssd += primitives.sse_pp[LUMA_4x16](fenc + x + 32 * stride, stride, rec + x + 32 * stride, stride);
            ssd += primitives.sse_pp[LUMA_4x16](fenc + x + 48 * stride, stride, rec + x + 48 * stride, stride);
        }

        fenc += stride * 64;
        rec += stride * 64;
    }

    /* Consume Y in chunks of 16 */
    for (; y + 16 <= height; y += 16)
    {
        int x = 0;

        if (!(stride & 31))
            for (; x + 64 <= width; x += 64)
            {
                ssd += primitives.sse_pp[LUMA_64x16](fenc + x, stride, rec + x, stride);
            }

        if (!(stride & 15))
            for (; x + 16 <= width; x += 16)
            {
                ssd += primitives.sse_pp[LUMA_16x16](fenc + x, stride, rec + x, stride);
            }

        for (; x + 4 <= width; x += 4)
        {
            ssd += primitives.sse_pp[LUMA_4x16](fenc + x, stride, rec + x, stride);
        }

        fenc += stride * 16;
        rec += stride * 16;
    }

    /* Consume Y in chunks of 4 */
    for (; y + 4 <= height; y += 4)
    {
        int x = 0;

        if (!(stride & 15))
            for (; x + 16 <= width; x += 16)
            {
                ssd += primitives.sse_pp[LUMA_16x4](fenc + x, stride, rec + x, stride);
            }

        for (; x + 4 <= width; x += 4)
        {
            ssd += primitives.sse_pp[LUMA_4x4](fenc + x, stride, rec + x, stride);
        }

        fenc += stride * 4;
        rec += stride * 4;
    }

    return ssd;
}

/* Function to calculate SSIM for each row */
static float calculateSSIM(pixel *pix1, intptr_t stride1, pixel *pix2, intptr_t stride2, int width, int height, void *buf, int32_t* cnt)
{
    int z = 0;
    float ssim = 0.0;

    int(*sum0)[4] = (int(*)[4])buf;
    int(*sum1)[4] = sum0 + (width >> 2) + 3;
    width >>= 2;
    height >>= 2;

    for (int y = 1; y < height; y++)
    {
        for (; z <= y; z++)
        {
            void* swap = sum0;
            sum0 = sum1;
            sum1 = (int(*)[4])swap;
            for (int x = 0; x < width; x += 2)
            {
                primitives.ssim_4x4x2_core(&pix1[(4 * x + (z * stride1))], stride1, &pix2[(4 * x + (z * stride2))], stride2, &sum0[x]);
            }
        }

        for (int x = 0; x < width - 1; x += 4)
        {
            ssim += primitives.ssim_end_4(sum0 + x, sum1 + x, X265_MIN(4, width - x - 1));
        }
    }

    *cnt = (height - 1) * (width - 1);
    return ssim;
}

void FrameFilter::processSao(int row)
{
    const uint32_t numCols = m_pic->getPicSym()->getFrameWidthInCU();
    const uint32_t lineStartCUAddr = row * numCols;
    SAOParam* saoParam = m_pic->getPicSym()->getSaoParam();

    // NOTE: these flag is not use in this mode
    assert(saoParam->oneUnitFlag[0] == false);
    assert(saoParam->oneUnitFlag[1] == false);
    assert(saoParam->oneUnitFlag[2] == false);

    if (saoParam->bSaoFlag[0])
    {
        m_sao.processSaoUnitRow(saoParam->saoLcuParam[0], row, 0);
    }
    if (saoParam->bSaoFlag[1])
    {
        m_sao.processSaoUnitRow(saoParam->saoLcuParam[1], row, 1);
        m_sao.processSaoUnitRow(saoParam->saoLcuParam[2], row, 2);
    }

    // TODO: this code is NOT VERIFIED because TransformSkip and PCM modes have some bugs, they are never enabled
    bool  bPCMFilter = (m_pic->getSlice()->getSPS()->getUsePCM() && m_pic->getSlice()->getSPS()->getPCMFilterDisableFlag()) ? true : false;
    if (bPCMFilter || m_pic->getSlice()->getPPS()->getTransquantBypassEnableFlag())
    {
        for (uint32_t col = 0; col < numCols; col++)
        {
            const uint32_t cuAddr = lineStartCUAddr + col;
            TComDataCU* cu = m_pic->getCU(cuAddr);

            xPCMCURestoration(cu, 0, 0);
        }
    }
}

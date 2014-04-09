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

/** \file     TComPicSym.cpp
    \brief    picture symbol class
*/

#include "TComPicSym.h"
#include "TComSampleAdaptiveOffset.h"

using namespace x265;

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComPicSym::TComPicSym()
    : m_widthInCU(0)
    , m_heightInCU(0)
    , m_maxCUSize(0)
    , m_unitSize(0)
    , m_log2UnitSize(0)
    , m_totalDepth(0)
    , m_numPartitions(0)
    , m_numPartInCUSize(0)
    , m_numCUsInFrame(0)
    , m_slice(NULL)
    , m_cuData(NULL)
{}

bool TComPicSym::create(int picWidth, int picHeight, int picCsp, uint32_t maxCUSize, uint32_t maxDepth)
{
    uint32_t i;

    m_saoParam        = NULL;
    m_totalDepth      = maxDepth;
    m_numPartitions   = 1 << (m_totalDepth << 1);

    m_maxCUSize       = maxCUSize;

    m_unitSize        = maxCUSize  >> m_totalDepth;
    m_log2UnitSize    = g_convertToBit[m_unitSize] + 2;

    m_numPartInCUSize = m_maxCUSize >> m_log2UnitSize;

    m_widthInCU       = (picWidth  + m_maxCUSize - 1) / m_maxCUSize;
    m_heightInCU      = (picHeight + m_maxCUSize - 1) / m_maxCUSize;

    m_numCUsInFrame   = m_widthInCU * m_heightInCU;

    m_slice = new TComSlice;
    m_cuData = new TComDataCU*[m_numCUsInFrame];
    if (!m_slice || !m_cuData)
        return false;

    for (i = 0; i < m_numCUsInFrame; i++)
    {
        m_cuData[i] = new TComDataCU;
        if (!m_cuData[i])
            return false;
        if (!m_cuData[i]->create(m_numPartitions, m_maxCUSize, m_unitSize, picCsp))
            return false;
    }

    return true;
}

void TComPicSym::destroy()
{
    delete m_slice;
    m_slice = NULL;

    for (int i = 0; i < m_numCUsInFrame; i++)
    {
        m_cuData[i]->destroy();
        delete m_cuData[i];
    }

    delete [] m_cuData;
    m_cuData = NULL;

    if (m_saoParam)
    {
        TComSampleAdaptiveOffset::freeSaoParam(m_saoParam);
        delete m_saoParam;
        m_saoParam = NULL;
    }
}

void TComPicSym::allocSaoParam(TComSampleAdaptiveOffset *sao)
{
    m_saoParam = new SAOParam;
    sao->allocSaoParam(m_saoParam);
}

//! \}

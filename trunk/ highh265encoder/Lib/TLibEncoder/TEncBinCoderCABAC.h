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

/** \file     TEncBinCoderCABAC.h
    \brief    binary entropy encoder of CABAC
*/

#ifndef X265_TENCBINCODERCABAC_H
#define X265_TENCBINCODERCABAC_H

#include "TLibCommon/ContextTables.h"
#include "TLibCommon/TComBitStream.h"

//! \ingroup TLibEncoder
//! \{

namespace x265 {
// private namespace

class TEncBinCABAC
{
public:

    TEncBinCABAC(bool isCounter = false);
    virtual ~TEncBinCABAC();

    void  init(TComBitIf* bitIf);

    void  start();
    void  finish();
    void  copyState(TEncBinCABAC* binIf);
    void  flush();

    void  resetBac();
    void  encodePCMAlignBits();
    void  xWritePCMCode(uint32_t code, uint32_t length);

    void  resetBits();

    uint32_t getNumWrittenBits()
    {
        // NOTE: the HM go here only in Counter mode
        assert(!m_bIsCounter || (m_bitIf->getNumberOfWrittenBits() == 0));
        assert(m_bIsCounter);
        return uint32_t(m_fracBits >> 15);

        // NOTE: I keep the old code, so someone may active if they want
        //return m_bitIf->getNumberOfWrittenBits() + 8 * m_numBufferedBytes + 23 - m_bitsLeft;
    }

    void  encodeBin(uint32_t binValue, ContextModel& ctxModel);
    void  encodeBinEP(uint32_t binValue);
    void  encodeBinsEP(uint32_t binValues, int numBins);
    void  encodeBinTrm(uint32_t binValue);

protected:

    void writeOut();

public:

    TComBitIf* m_bitIf;
    uint32_t   m_low;
    uint32_t   m_range;
    uint32_t   m_bufferedByte;
    int        m_numBufferedBytes;
    int        m_bitsLeft;
    uint64_t   m_fracBits;
    bool       m_bIsCounter;
};
}
//! \}

#endif // ifndef X265_TENCBINCODERCABAC_H

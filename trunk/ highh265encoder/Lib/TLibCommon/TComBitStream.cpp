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

/** \file     TComBitStream.cpp
    \brief    class for handling bitstream
*/

#include "TComBitStream.h"
#include "common.h"

using namespace x265;

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComOutputBitstream::TComOutputBitstream()
{
    m_fifo = X265_MALLOC(uint8_t, MIN_FIFO_SIZE);
    m_buffsize = MIN_FIFO_SIZE;
    clear();
}

TComOutputBitstream::~TComOutputBitstream()
{
    X265_FREE(m_fifo);
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

char* TComOutputBitstream::getByteStream() const
{
    return (char*)m_fifo;
}

uint32_t TComOutputBitstream::getByteStreamLength() const
{
    return m_fsize;
}

void TComOutputBitstream::clear()
{
    m_held_bits = 0;
    m_num_held_bits = 0;
    m_fsize = 0;
}

void TComOutputBitstream::write(uint32_t bits, uint32_t numBits)
{
    assert(numBits <= 32);
    assert(numBits == 32 || (bits & (~0 << numBits)) == 0);

    /* any modulo 8 remainder of num_total_bits cannot be written this time,
     * and will be held until next time. */
    uint32_t num_total_bits = numBits + m_num_held_bits;
    uint32_t next_num_held_bits = num_total_bits % 8;

    /* form a byte aligned word (write_bits), by concatenating any held bits
     * with the new bits, discarding the bits that will form the next_held_bits.
     * eg: H = held bits, V = n new bits        /---- next_held_bits
     * len(H)=7, len(V)=1: ... ---- HHHH HHHV . 0000 0000, next_num_held_bits=0
     * len(H)=7, len(V)=2: ... ---- HHHH HHHV . V000 0000, next_num_held_bits=1
     * if total_bits < 8, the value of v_ is not used */
    uint8_t next_held_bits = bits << (8 - next_num_held_bits);

    if (!(num_total_bits >> 3))
    {
        /* insufficient bits accumulated to write out, append new_held_bits to
         * current held_bits */
        /* NB, this requires that v only contains 0 in bit positions {31..n} */
        m_held_bits |= next_held_bits;
        m_num_held_bits = next_num_held_bits;
        return;
    }

    /* topword serves to justify held_bits to align with the msb of uiBits */
    uint32_t topword = (numBits - next_num_held_bits) & ~((1 << 3) - 1);
    uint32_t write_bits = (m_held_bits << topword) | (bits >> next_num_held_bits);

    switch (num_total_bits >> 3)
    {
    case 4: push_back(write_bits >> 24);
    case 3: push_back(write_bits >> 16);
    case 2: push_back(write_bits >> 8);
    case 1: push_back(write_bits);
    }

    m_held_bits = next_held_bits;
    m_num_held_bits = next_num_held_bits;
}

void TComOutputBitstream::writeByte(uint32_t val)
{
    // NOTE: we are here only in Cabac
    assert(m_num_held_bits == 0);

    push_back(val);
}

void TComOutputBitstream::writeAlignOne()
{
    uint32_t numBits = getNumBitsUntilByteAligned();

    write((1 << numBits) - 1, numBits);
}

void TComOutputBitstream::writeAlignZero()
{
    if (!m_num_held_bits)
        return;

    push_back(m_held_bits);
    m_held_bits = 0;
    m_num_held_bits = 0;
}

/**
 * add substream to the end of the current bitstream
 */
void TComOutputBitstream::addSubstream(TComOutputBitstream* substream)
{
    uint32_t numBits = substream->getNumberOfWrittenBits();

    const uint8_t* rbsp = substream->getFIFO();

    for (uint32_t count = 0; count < substream->m_fsize; count++)
    {
        write(rbsp[count], 8);
    }

    if (numBits & 0x7)
    {
        write(substream->getHeldBits() >> (8 - (numBits & 0x7)), numBits & 0x7);
    }
}

void TComOutputBitstream::writeByteAlignment()
{
    write(1, 1);
    writeAlignZero();
}

int TComOutputBitstream::countStartCodeEmulations()
{
    int numStartCodes = 0;
    uint8_t *rbsp = getFIFO();
    uint32_t fsize = getByteStreamLength();

    for (uint32_t i = 0; i + 2 < fsize; i++)
    {
        if (!rbsp[i] && !rbsp[i + 1] && rbsp[i + 2] <= 3)
        {
            numStartCodes++;
            i++;
        }
    }

    return numStartCodes;
}

void TComOutputBitstream::push_back(uint8_t val)
{
    if (!m_fifo)
        return;

    if (m_fsize >= m_buffsize)
    {
        /** reallocate buffer with doubled size */
        uint8_t *temp = X265_MALLOC(uint8_t, m_buffsize * 2);
        if (temp)
        {
            ::memcpy(temp, m_fifo, m_fsize);
            X265_FREE(m_fifo);
            m_fifo = temp;
            m_buffsize *= 2;
        }
        else
        {
            x265_log(NULL, X265_LOG_ERROR, "Unable to realloc bitstream buffer");
            return;
        }
    }
    m_fifo[m_fsize++] = val;
}

//! \}

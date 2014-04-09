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

#include "TComPicYuv.h"
#include "md5.h"

namespace x265 {
//! \ingroup TLibCommon
//! \{

/**
 * Update md5 using n samples from plane, each sample is adjusted to
 * OUTBIT_BITDEPTH_DIV8.
 */
template<uint32_t OUTPUT_BITDEPTH_DIV8>
static void md5_block(MD5Context& md5, const pixel* plane, uint32_t n)
{
    /* create a 64 byte buffer for packing pixel's into */
    uint8_t buf[64 / OUTPUT_BITDEPTH_DIV8][OUTPUT_BITDEPTH_DIV8];

    for (uint32_t i = 0; i < n; i++)
    {
        pixel pel = plane[i];
        /* perform bitdepth and endian conversion */
        for (uint32_t d = 0; d < OUTPUT_BITDEPTH_DIV8; d++)
        {
            buf[i][d] = pel >> (d * 8);
        }
    }

    MD5Update(&md5, (uint8_t*)buf, n * OUTPUT_BITDEPTH_DIV8);
}

/**
 * Update md5 with all samples in plane in raster order, each sample
 * is adjusted to OUTBIT_BITDEPTH_DIV8.
 */
template<uint32_t OUTPUT_BITDEPTH_DIV8>
static void md5_plane(MD5Context& md5, const pixel* plane, uint32_t width, uint32_t height, uint32_t stride)
{
    /* N is the number of samples to process per md5 update.
     * All N samples must fit in buf */
    uint32_t N = 32;
    uint32_t width_modN = width % N;
    uint32_t width_less_modN = width - width_modN;

    for (uint32_t y = 0; y < height; y++)
    {
        /* convert pel's into uint32_t chars in little endian byte order.
         * NB, for 8bit data, data is truncated to 8bits. */
        for (uint32_t x = 0; x < width_less_modN; x += N)
        {
            md5_block<OUTPUT_BITDEPTH_DIV8>(md5, &plane[y * stride + x], N);
        }

        /* mop up any of the remaining line */
        md5_block<OUTPUT_BITDEPTH_DIV8>(md5, &plane[y * stride + width_less_modN], width_modN);
    }
}

void updateCRC(const pixel* plane, uint32_t& crcVal, uint32_t height, uint32_t width, uint32_t stride)
{
    uint32_t crcMsb;
    uint32_t bitVal;
    uint32_t bitIdx;

    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            // take CRC of first pictureData byte
            for (bitIdx = 0; bitIdx < 8; bitIdx++)
            {
                crcMsb = (crcVal >> 15) & 1;
                bitVal = (plane[y * stride + x] >> (7 - bitIdx)) & 1;
                crcVal = (((crcVal << 1) + bitVal) & 0xffff) ^ (crcMsb * 0x1021);
            }

#if _MSC_VER
#pragma warning(disable: 4127) // conditional expression is constant
#endif
            // take CRC of second pictureData byte if bit depth is greater than 8-bits
            if (X265_DEPTH > 8)
            {
                for (bitIdx = 0; bitIdx < 8; bitIdx++)
                {
                    crcMsb = (crcVal >> 15) & 1;
                    bitVal = (plane[y * stride + x] >> (15 - bitIdx)) & 1;
                    crcVal = (((crcVal << 1) + bitVal) & 0xffff) ^ (crcMsb * 0x1021);
                }
            }
        }
    }
}

void crcFinish(uint32_t& crcVal, uint8_t digest[16])
{
    uint32_t crcMsb;

    for (int bitIdx = 0; bitIdx < 16; bitIdx++)
    {
        crcMsb = (crcVal >> 15) & 1;
        crcVal = ((crcVal << 1) & 0xffff) ^ (crcMsb * 0x1021);
    }

    digest[0] = (crcVal >> 8)  & 0xff;
    digest[1] =  crcVal        & 0xff;
}

void updateChecksum(const pixel* plane, uint32_t& checksumVal, uint32_t height, uint32_t width, uint32_t stride, int row, uint32_t cuHeight)
{
    uint8_t xor_mask;

    for (uint32_t y = row * cuHeight; y < ((row * cuHeight) + height); y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            xor_mask = (x & 0xff) ^ (y & 0xff) ^ (x >> 8) ^ (y >> 8);
            checksumVal = (checksumVal + ((plane[y * stride + x] & 0xff) ^ xor_mask)) & 0xffffffff;

            if (X265_DEPTH > 8)
            {
                checksumVal = (checksumVal + ((plane[y * stride + x] >> 7 >> 1) ^ xor_mask)) & 0xffffffff;
            }
        }
    }
}

void checksumFinish(uint32_t& checksum, uint8_t digest[16])
{
    digest[0] = (checksum >> 24) & 0xff;
    digest[1] = (checksum >> 16) & 0xff;
    digest[2] = (checksum >> 8)  & 0xff;
    digest[3] =  checksum        & 0xff;
}

void updateMD5Plane(MD5Context& md5, const pixel* plane, uint32_t width, uint32_t height, uint32_t stride)
{
    /* choose an md5_plane packing function based on the system bitdepth */
    typedef void (*MD5PlaneFunc)(MD5Context&, const pixel*, uint32_t, uint32_t, uint32_t);
    MD5PlaneFunc md5_plane_func;
    md5_plane_func = X265_DEPTH <= 8 ? (MD5PlaneFunc)md5_plane<1> : (MD5PlaneFunc)md5_plane<2>;

    md5_plane_func(md5, plane, width, height, stride);
}
}
//! \}

/*****************************************************************************
 * Copyright (C) 2013 x265 project
 *
 * Authors: Steve Borho <steve@borho.org>
 *          Mandar Gurav <mandar@multicorewareinc.com>
 *          Mahesh Pittala <mahesh@multicorewareinc.com>
 *          Min Chen <min.chen@multicorewareinc.com>
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

#include "TLibCommon/TComRom.h"
#include "primitives.h"
#include "x265.h"

#include <cstdlib> // abs()

using namespace x265;

#define SET_FUNC_PRIMITIVE_TABLE_C(FUNC_PREFIX, FUNC_PREFIX_DEF, FUNC_TYPE_CAST, DATA_TYPE1, DATA_TYPE2) \
    p.FUNC_PREFIX[LUMA_4x4]   = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<4,  4, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_8x8]   = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<8,  8, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_8x4]   = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<8,  4, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_4x8]   = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<4,  8, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_16x16] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<16, 16, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_16x8]  = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<16,  8, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_8x16]  = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<8, 16, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_16x12] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<16, 12, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_12x16] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<12, 16, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_16x4]  = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<16,  4, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_4x16]  = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<4, 16, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_32x32] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<32, 32, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_32x16] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<32, 16, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_16x32] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<16, 32, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_32x24] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<32, 24, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_24x32] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<24, 32, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_32x8]  = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<32,  8, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_8x32]  = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<8, 32, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_64x64] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<64, 64, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_64x32] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<64, 32, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_32x64] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<32, 64, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_64x48] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<64, 48, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_48x64] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<48, 64, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_64x16] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<64, 16, DATA_TYPE1, DATA_TYPE2>; \
    p.FUNC_PREFIX[LUMA_16x64] = (FUNC_TYPE_CAST)FUNC_PREFIX_DEF<16, 64, DATA_TYPE1, DATA_TYPE2>;

#define SET_FUNC_PRIMITIVE_TABLE_C2(FUNC_PREFIX) \
    p.FUNC_PREFIX[LUMA_4x4]   = FUNC_PREFIX<4,  4>; \
    p.FUNC_PREFIX[LUMA_8x8]   = FUNC_PREFIX<8,  8>; \
    p.FUNC_PREFIX[LUMA_8x4]   = FUNC_PREFIX<8,  4>; \
    p.FUNC_PREFIX[LUMA_4x8]   = FUNC_PREFIX<4,  8>; \
    p.FUNC_PREFIX[LUMA_16x16] = FUNC_PREFIX<16, 16>; \
    p.FUNC_PREFIX[LUMA_16x8]  = FUNC_PREFIX<16,  8>; \
    p.FUNC_PREFIX[LUMA_8x16]  = FUNC_PREFIX<8, 16>; \
    p.FUNC_PREFIX[LUMA_16x12] = FUNC_PREFIX<16, 12>; \
    p.FUNC_PREFIX[LUMA_12x16] = FUNC_PREFIX<12, 16>; \
    p.FUNC_PREFIX[LUMA_16x4]  = FUNC_PREFIX<16,  4>; \
    p.FUNC_PREFIX[LUMA_4x16]  = FUNC_PREFIX<4, 16>; \
    p.FUNC_PREFIX[LUMA_32x32] = FUNC_PREFIX<32, 32>; \
    p.FUNC_PREFIX[LUMA_32x16] = FUNC_PREFIX<32, 16>; \
    p.FUNC_PREFIX[LUMA_16x32] = FUNC_PREFIX<16, 32>; \
    p.FUNC_PREFIX[LUMA_32x24] = FUNC_PREFIX<32, 24>; \
    p.FUNC_PREFIX[LUMA_24x32] = FUNC_PREFIX<24, 32>; \
    p.FUNC_PREFIX[LUMA_32x8]  = FUNC_PREFIX<32,  8>; \
    p.FUNC_PREFIX[LUMA_8x32]  = FUNC_PREFIX<8, 32>; \
    p.FUNC_PREFIX[LUMA_64x64] = FUNC_PREFIX<64, 64>; \
    p.FUNC_PREFIX[LUMA_64x32] = FUNC_PREFIX<64, 32>; \
    p.FUNC_PREFIX[LUMA_32x64] = FUNC_PREFIX<32, 64>; \
    p.FUNC_PREFIX[LUMA_64x48] = FUNC_PREFIX<64, 48>; \
    p.FUNC_PREFIX[LUMA_48x64] = FUNC_PREFIX<48, 64>; \
    p.FUNC_PREFIX[LUMA_64x16] = FUNC_PREFIX<64, 16>; \
    p.FUNC_PREFIX[LUMA_16x64] = FUNC_PREFIX<16, 64>;

namespace {
// place functions in anonymous namespace (file static)

template<int lx, int ly>
int sad(pixel *pix1, intptr_t stride_pix1, pixel *pix2, intptr_t stride_pix2)
{
    int sum = 0;

    for (int y = 0; y < ly; y++)
    {
        for (int x = 0; x < lx; x++)
        {
            sum += abs(pix1[x] - pix2[x]);
        }

        pix1 += stride_pix1;
        pix2 += stride_pix2;
    }

    return sum;
}

template<int lx, int ly>
void sad_x3(pixel *pix1, pixel *pix2, pixel *pix3, pixel *pix4, intptr_t frefstride, int32_t *res)
{
    res[0] = 0;
    res[1] = 0;
    res[2] = 0;
    for (int y = 0; y < ly; y++)
    {
        for (int x = 0; x < lx; x++)
        {
            res[0] += abs(pix1[x] - pix2[x]);
            res[1] += abs(pix1[x] - pix3[x]);
            res[2] += abs(pix1[x] - pix4[x]);
        }

        pix1 += FENC_STRIDE;
        pix2 += frefstride;
        pix3 += frefstride;
        pix4 += frefstride;
    }
}

template<int lx, int ly>
void sad_x4(pixel *pix1, pixel *pix2, pixel *pix3, pixel *pix4, pixel *pix5, intptr_t frefstride, int32_t *res)
{
    res[0] = 0;
    res[1] = 0;
    res[2] = 0;
    res[3] = 0;
    for (int y = 0; y < ly; y++)
    {
        for (int x = 0; x < lx; x++)
        {
            res[0] += abs(pix1[x] - pix2[x]);
            res[1] += abs(pix1[x] - pix3[x]);
            res[2] += abs(pix1[x] - pix4[x]);
            res[3] += abs(pix1[x] - pix5[x]);
        }

        pix1 += FENC_STRIDE;
        pix2 += frefstride;
        pix3 += frefstride;
        pix4 += frefstride;
        pix5 += frefstride;
    }
}

template<int lx, int ly, class T1, class T2>
int sse(T1 *pix1, intptr_t stride_pix1, T2 *pix2, intptr_t stride_pix2)
{
    int sum = 0;
    int iTemp;

    for (int y = 0; y < ly; y++)
    {
        for (int x = 0; x < lx; x++)
        {
            iTemp = pix1[x] - pix2[x];
            sum += (iTemp * iTemp);
        }

        pix1 += stride_pix1;
        pix2 += stride_pix2;
    }

    return sum;
}

#define BITS_PER_SUM (8 * sizeof(sum_t))

#define HADAMARD4(d0, d1, d2, d3, s0, s1, s2, s3) { \
        sum2_t t0 = s0 + s1; \
        sum2_t t1 = s0 - s1; \
        sum2_t t2 = s2 + s3; \
        sum2_t t3 = s2 - s3; \
        d0 = t0 + t2; \
        d2 = t0 - t2; \
        d1 = t1 + t3; \
        d3 = t1 - t3; \
}

// in: a pseudo-simd number of the form x+(y<<16)
// return: abs(x)+(abs(y)<<16)
inline sum2_t abs2(sum2_t a)
{
    sum2_t s = ((a >> (BITS_PER_SUM - 1)) & (((sum2_t)1 << BITS_PER_SUM) + 1)) * ((sum_t)-1);

    return (a + s) ^ s;
}

int satd_4x4(pixel *pix1, intptr_t stride_pix1, pixel *pix2, intptr_t stride_pix2)
{
    sum2_t tmp[4][2];
    sum2_t a0, a1, a2, a3, b0, b1;
    sum2_t sum = 0;

    for (int i = 0; i < 4; i++, pix1 += stride_pix1, pix2 += stride_pix2)
    {
        a0 = pix1[0] - pix2[0];
        a1 = pix1[1] - pix2[1];
        b0 = (a0 + a1) + ((a0 - a1) << BITS_PER_SUM);
        a2 = pix1[2] - pix2[2];
        a3 = pix1[3] - pix2[3];
        b1 = (a2 + a3) + ((a2 - a3) << BITS_PER_SUM);
        tmp[i][0] = b0 + b1;
        tmp[i][1] = b0 - b1;
    }

    for (int i = 0; i < 2; i++)
    {
        HADAMARD4(a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i]);
        a0 = abs2(a0) + abs2(a1) + abs2(a2) + abs2(a3);
        sum += ((sum_t)a0) + (a0 >> BITS_PER_SUM);
    }

    return (int)(sum >> 1);
}

// x264's SWAR version of satd 8x4, performs two 4x4 SATDs at once
int satd_8x4(pixel *pix1, intptr_t stride_pix1, pixel *pix2, intptr_t stride_pix2)
{
    sum2_t tmp[4][4];
    sum2_t a0, a1, a2, a3;
    sum2_t sum = 0;

    for (int i = 0; i < 4; i++, pix1 += stride_pix1, pix2 += stride_pix2)
    {
        a0 = (pix1[0] - pix2[0]) + ((sum2_t)(pix1[4] - pix2[4]) << BITS_PER_SUM);
        a1 = (pix1[1] - pix2[1]) + ((sum2_t)(pix1[5] - pix2[5]) << BITS_PER_SUM);
        a2 = (pix1[2] - pix2[2]) + ((sum2_t)(pix1[6] - pix2[6]) << BITS_PER_SUM);
        a3 = (pix1[3] - pix2[3]) + ((sum2_t)(pix1[7] - pix2[7]) << BITS_PER_SUM);
        HADAMARD4(tmp[i][0], tmp[i][1], tmp[i][2], tmp[i][3], a0, a1, a2, a3);
    }

    for (int i = 0; i < 4; i++)
    {
        HADAMARD4(a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i]);
        sum += abs2(a0) + abs2(a1) + abs2(a2) + abs2(a3);
    }

    return (((sum_t)sum) + (sum >> BITS_PER_SUM)) >> 1;
}

template<int w, int h>
// calculate satd in blocks of 4x4
int satd4(pixel *pix1, intptr_t stride_pix1, pixel *pix2, intptr_t stride_pix2)
{
    int satd = 0;

    for (int row = 0; row < h; row += 4)
    {
        for (int col = 0; col < w; col += 4)
        {
            satd += satd_4x4(pix1 + row * stride_pix1 + col, stride_pix1,
                             pix2 + row * stride_pix2 + col, stride_pix2);
        }
    }

    return satd;
}

template<int w, int h>
// calculate satd in blocks of 8x4
int satd8(pixel *pix1, intptr_t stride_pix1, pixel *pix2, intptr_t stride_pix2)
{
    int satd = 0;

    for (int row = 0; row < h; row += 4)
    {
        for (int col = 0; col < w; col += 8)
        {
            satd += satd_8x4(pix1 + row * stride_pix1 + col, stride_pix1,
                             pix2 + row * stride_pix2 + col, stride_pix2);
        }
    }

    return satd;
}

inline int _sa8d_8x8(pixel *pix1, intptr_t i_pix1, pixel *pix2, intptr_t i_pix2)
{
    sum2_t tmp[8][4];
    sum2_t a0, a1, a2, a3, a4, a5, a6, a7, b0, b1, b2, b3;
    sum2_t sum = 0;

    for (int i = 0; i < 8; i++, pix1 += i_pix1, pix2 += i_pix2)
    {
        a0 = pix1[0] - pix2[0];
        a1 = pix1[1] - pix2[1];
        b0 = (a0 + a1) + ((a0 - a1) << BITS_PER_SUM);
        a2 = pix1[2] - pix2[2];
        a3 = pix1[3] - pix2[3];
        b1 = (a2 + a3) + ((a2 - a3) << BITS_PER_SUM);
        a4 = pix1[4] - pix2[4];
        a5 = pix1[5] - pix2[5];
        b2 = (a4 + a5) + ((a4 - a5) << BITS_PER_SUM);
        a6 = pix1[6] - pix2[6];
        a7 = pix1[7] - pix2[7];
        b3 = (a6 + a7) + ((a6 - a7) << BITS_PER_SUM);
        HADAMARD4(tmp[i][0], tmp[i][1], tmp[i][2], tmp[i][3], b0, b1, b2, b3);
    }

    for (int i = 0; i < 4; i++)
    {
        HADAMARD4(a0, a1, a2, a3, tmp[0][i], tmp[1][i], tmp[2][i], tmp[3][i]);
        HADAMARD4(a4, a5, a6, a7, tmp[4][i], tmp[5][i], tmp[6][i], tmp[7][i]);
        b0  = abs2(a0 + a4) + abs2(a0 - a4);
        b0 += abs2(a1 + a5) + abs2(a1 - a5);
        b0 += abs2(a2 + a6) + abs2(a2 - a6);
        b0 += abs2(a3 + a7) + abs2(a3 - a7);
        sum += (sum_t)b0 + (b0 >> BITS_PER_SUM);
    }

    return (int)sum;
}

int sa8d_8x8(pixel *pix1, intptr_t i_pix1, pixel *pix2, intptr_t i_pix2)
{
    return (int)((_sa8d_8x8(pix1, i_pix1, pix2, i_pix2) + 2) >> 2);
}

int sa8d_16x16(pixel *pix1, intptr_t i_pix1, pixel *pix2, intptr_t i_pix2)
{
    int sum = _sa8d_8x8(pix1, i_pix1, pix2, i_pix2)
        + _sa8d_8x8(pix1 + 8, i_pix1, pix2 + 8, i_pix2)
        + _sa8d_8x8(pix1 + 8 * i_pix1, i_pix1, pix2 + 8 * i_pix2, i_pix2)
        + _sa8d_8x8(pix1 + 8 + 8 * i_pix1, i_pix1, pix2 + 8 + 8 * i_pix2, i_pix2);

    // This matches x264 sa8d_16x16, but is slightly different from HM's behavior because
    // this version only rounds once at the end
    return (sum + 2) >> 2;
}

template<int w, int h>
// Calculate sa8d in blocks of 8x8
int sa8d8(pixel *pix1, intptr_t i_pix1, pixel *pix2, intptr_t i_pix2)
{
    int cost = 0;

    for (int y = 0; y < h; y += 8)
    {
        for (int x = 0; x < w; x += 8)
        {
            cost += sa8d_8x8(pix1 + i_pix1 * y + x, i_pix1, pix2 + i_pix2 * y + x, i_pix2);
        }
    }

    return cost;
}

template<int w, int h>
// Calculate sa8d in blocks of 16x16
int sa8d16(pixel *pix1, intptr_t i_pix1, pixel *pix2, intptr_t i_pix2)
{
    int cost = 0;

    for (int y = 0; y < h; y += 16)
    {
        for (int x = 0; x < w; x += 16)
        {
            cost += sa8d_16x16(pix1 + i_pix1 * y + x, i_pix1, pix2 + i_pix2 * y + x, i_pix2);
        }
    }

    return cost;
}

void blockcopy_p_p(int bx, int by, pixel *a, intptr_t stridea, pixel *b, intptr_t strideb)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
        {
            a[x] = b[x];
        }

        a += stridea;
        b += strideb;
    }
}

void blockcopy_p_s(int bx, int by, pixel *a, intptr_t stridea, int16_t *b, intptr_t strideb)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
        {
            assert((b[x] >= 0) && (b[x] <= ((1 << X265_DEPTH) - 1)));
            a[x] = (pixel)b[x];
        }

        a += stridea;
        b += strideb;
    }
}

template<int size>
void blockfil_s_c(int16_t *dst, intptr_t dstride, int16_t val)
{
    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            dst[y * dstride + x] = val;
        }
    }
}

void convert16to32_shl(int32_t *dst, int16_t *src, intptr_t stride, int shift, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            dst[i * size + j] = ((int)src[i * stride + j]) << shift;
        }
    }
}

void convert32to16_shr(int16_t *dst, int32_t *src, intptr_t stride, int shift, int size)
{
    int round = 1 << (shift - 1);

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            dst[j] = (int16_t)((src[j] + round) >> shift);
        }

        src += size;
        dst += stride;
    }
}

template<int blockSize>
void getResidual(pixel *fenc, pixel *pred, int16_t *residual, intptr_t stride)
{
    for (int y = 0; y < blockSize; y++)
    {
        for (int x = 0; x < blockSize; x++)
        {
            residual[x] = static_cast<int16_t>(fenc[x]) - static_cast<int16_t>(pred[x]);
        }

        fenc += stride;
        residual += stride;
        pred += stride;
    }
}

template<int blockSize>
void calcRecons(pixel* pred, int16_t* residual, int16_t* recqt, pixel* recipred, int stride, int qtstride, int ipredstride)
{
    for (int y = 0; y < blockSize; y++)
    {
        for (int x = 0; x < blockSize; x++)
        {
            recqt[x] = (int16_t)Clip(static_cast<int16_t>(pred[x]) + residual[x]);
            recipred[x] = (pixel)recqt[x];
        }

        pred += stride;
        residual += stride;
        recqt += qtstride;
        recipred += ipredstride;
    }
}

template<int blockSize>
void transpose(pixel* dst, pixel* src, intptr_t stride)
{
    for (int k = 0; k < blockSize; k++)
    {
        for (int l = 0; l < blockSize; l++)
        {
            dst[k * blockSize + l] = src[l * stride + k];
        }
    }
}

void weight_sp_c(int16_t *src, pixel *dst, intptr_t srcStride, intptr_t dstStride, int width, int height, int w0, int round, int shift, int offset)
{
    int x, y;

    for (y = 0; y <= height - 1; y++)
    {
        for (x = 0; x <= width - 1; )
        {
            // note: width can be odd
            dst[x] = (pixel)Clip3(0, ((1 << X265_DEPTH) - 1), ((w0 * (src[x] + IF_INTERNAL_OFFS) + round) >> shift) + offset);
            x++;
        }

        src += srcStride;
        dst += dstStride;
    }
}

void weight_pp_c(pixel *src, pixel *dst, intptr_t srcStride, intptr_t dstStride, int width, int height, int w0, int round, int shift, int offset)
{
    int x, y;

    assert(!(width & 15));

    for (y = 0; y <= height - 1; y++)
    {
        for (x = 0; x <= width - 1; )
        {
            // simulating pixel to short conversion
            int16_t val = src[x] << (IF_INTERNAL_PREC - X265_DEPTH);
            dst[x] = (pixel)Clip3(0, ((1 << X265_DEPTH) - 1), ((w0 * (val) + round) >> shift) + offset);
            x++;
        }

        src += srcStride;
        dst += dstStride;
    }
}

void pixeladd_ss_c(int bx, int by, int16_t *a, intptr_t dstride, int16_t *b0, int16_t *b1, intptr_t sstride0, intptr_t sstride1)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
        {
            a[x] = (int16_t)Clip(b0[x] + b1[x]);
        }

        b0 += sstride0;
        b1 += sstride1;
        a += dstride;
    }
}

template<int lx, int ly>
void pixelavg_pp(pixel* dst, intptr_t dstride, pixel* src0, intptr_t sstride0, pixel* src1, intptr_t sstride1, int)
{
    for (int y = 0; y < ly; y++)
    {
        for (int x = 0; x < lx; x++)
        {
            dst[x] = (src0[x] + src1[x] + 1) >> 1;
        }

        src0 += sstride0;
        src1 += sstride1;
        dst += dstride;
    }
}

void scale1D_128to64(pixel *dst, pixel *src, intptr_t /*stride*/)
{
    int x;

    for (x = 0; x < 128; x += 2)
    {
        pixel pix0 = src[(x + 0)];
        pixel pix1 = src[(x + 1)];
        int sum = pix0 + pix1;

        dst[x >> 1] = (pixel)((sum + 1) >> 1);
    }
}

void scale2D_64to32(pixel *dst, pixel *src, intptr_t stride)
{
    int x, y;

    for (y = 0; y < 64; y += 2)
    {
        for (x = 0; x < 64; x += 2)
        {
            pixel pix0 = src[(y + 0) * stride + (x + 0)];
            pixel pix1 = src[(y + 0) * stride + (x + 1)];
            pixel pix2 = src[(y + 1) * stride + (x + 0)];
            pixel pix3 = src[(y + 1) * stride + (x + 1)];
            int sum = pix0 + pix1 + pix2 + pix3;

            dst[y / 2 * 32 + x / 2] = (pixel)((sum + 2) >> 2);
        }
    }
}

void frame_init_lowres_core(pixel *src0, pixel *dst0, pixel *dsth, pixel *dstv, pixel *dstc,
                            intptr_t src_stride, intptr_t dst_stride, int width, int height)
{
    for (int y = 0; y < height; y++)
    {
        pixel *src1 = src0 + src_stride;
        pixel *src2 = src1 + src_stride;
        for (int x = 0; x < width; x++)
        {
            // slower than naive bilinear, but matches asm
#define FILTER(a, b, c, d) ((((a + b + 1) >> 1) + ((c + d + 1) >> 1) + 1) >> 1)
            dst0[x] = FILTER(src0[2 * x], src1[2 * x], src0[2 * x + 1], src1[2 * x + 1]);
            dsth[x] = FILTER(src0[2 * x + 1], src1[2 * x + 1], src0[2 * x + 2], src1[2 * x + 2]);
            dstv[x] = FILTER(src1[2 * x], src2[2 * x], src1[2 * x + 1], src2[2 * x + 1]);
            dstc[x] = FILTER(src1[2 * x + 1], src2[2 * x + 1], src1[2 * x + 2], src2[2 * x + 2]);
#undef FILTER
        }
        src0 += src_stride * 2;
        dst0 += dst_stride;
        dsth += dst_stride;
        dstv += dst_stride;
        dstc += dst_stride;
    }
}

/* structural similarity metric */
void ssim_4x4x2_core(const pixel *pix1, intptr_t stride1, const pixel *pix2, intptr_t stride2, int sums[2][4])
{
    for (int z = 0; z < 2; z++)
    {
        uint32_t s1 = 0, s2 = 0, ss = 0, s12 = 0;
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                int a = pix1[x + y * stride1];
                int b = pix2[x + y * stride2];
                s1 += a;
                s2 += b;
                ss += a * a;
                ss += b * b;
                s12 += a * b;
            }
        }

        sums[z][0] = s1;
        sums[z][1] = s2;
        sums[z][2] = ss;
        sums[z][3] = s12;
        pix1 += 4;
        pix2 += 4;
    }
}

float ssim_end_1(int s1, int s2, int ss, int s12)
{
/* Maximum value for 10-bit is: ss*64 = (2^10-1)^2*16*4*64 = 4286582784, which will overflow in some cases.
 * s1*s1, s2*s2, and s1*s2 also obtain this value for edge cases: ((2^10-1)*16*4)^2 = 4286582784.
 * Maximum value for 9-bit is: ss*64 = (2^9-1)^2*16*4*64 = 1069551616, which will not overflow. */

#define PIXEL_MAX ((1 << X265_DEPTH) - 1)
#if HIGH_BIT_DEPTH
    assert(X265_DEPTH == 10);
#define type float
    static const float ssim_c1 = (float)(.01 * .01 * PIXEL_MAX * PIXEL_MAX * 64);
    static const float ssim_c2 = (float)(.03 * .03 * PIXEL_MAX * PIXEL_MAX * 64 * 63);
#else
    assert(X265_DEPTH == 8);
#define type int
    static const int ssim_c1 = (int)(.01 * .01 * PIXEL_MAX * PIXEL_MAX * 64 + .5);
    static const int ssim_c2 = (int)(.03 * .03 * PIXEL_MAX * PIXEL_MAX * 64 * 63 + .5);
#endif
    type fs1 = (type)s1;
    type fs2 = (type)s2;
    type fss = (type)ss;
    type fs12 = (type)s12;
    type vars = (type)(fss * 64 - fs1 * fs1 - fs2 * fs2);
    type covar = (type)(fs12 * 64 - fs1 * fs2);
    return (float)(2 * fs1 * fs2 + ssim_c1) * (float)(2 * covar + ssim_c2)
           / ((float)(fs1 * fs1 + fs2 * fs2 + ssim_c1) * (float)(vars + ssim_c2));
#undef type
#undef PIXEL_MAX
}

float ssim_end_4(int sum0[5][4], int sum1[5][4], int width)
{
    float ssim = 0.0;

    for (int i = 0; i < width; i++)
    {
        ssim += ssim_end_1(sum0[i][0] + sum0[i + 1][0] + sum1[i][0] + sum1[i + 1][0],
                           sum0[i][1] + sum0[i + 1][1] + sum1[i][1] + sum1[i + 1][1],
                           sum0[i][2] + sum0[i + 1][2] + sum1[i][2] + sum1[i + 1][2],
                           sum0[i][3] + sum0[i + 1][3] + sum1[i][3] + sum1[i + 1][3]);
    }

    return ssim;
}

template<int size>
uint64_t pixel_var(pixel *pix, intptr_t i_stride)
{
    uint32_t sum = 0, sqr = 0;

    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            sum += pix[x];
            sqr += pix[x] * pix[x];
        }

        pix += i_stride;
    }

    return sum + ((uint64_t)sqr << 32);
}

void plane_copy_deinterleave_chroma(pixel *dstu, intptr_t dstuStride, pixel *dstv, intptr_t dstvStride,
                                    pixel *src,  intptr_t srcStride, int w, int h)
{
    for (int y = 0; y < h; y++, dstu += dstuStride, dstv += dstvStride, src += srcStride)
    {
        for (int x = 0; x < w; x++)
        {
            dstu[x] = src[2 * x];
            dstv[x] = src[2 * x + 1];
        }
    }
}

template<int bx, int by>
void blockcopy_pp_c(pixel *a, intptr_t stridea, pixel *b, intptr_t strideb)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
        {
            a[x] = b[x];
        }

        a += stridea;
        b += strideb;
    }
}

template<int bx, int by>
void blockcopy_ss_c(int16_t *a, intptr_t stridea, int16_t *b, intptr_t strideb)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
        {
            a[x] = b[x];
        }

        a += stridea;
        b += strideb;
    }
}

template<int bx, int by>
void blockcopy_sp_c(pixel *a, intptr_t stridea, int16_t *b, intptr_t strideb)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
        {
            assert((b[x] >= 0) && (b[x] <= ((1 << X265_DEPTH) - 1)));
            a[x] = (pixel)b[x];
        }

        a += stridea;
        b += strideb;
    }
}

template<int bx, int by>
void blockcopy_ps_c(int16_t *a, intptr_t stridea, pixel *b, intptr_t strideb)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
        {
            a[x] = (int16_t)b[x];
        }

        a += stridea;
        b += strideb;
    }
}

template<int bx, int by>
void pixel_sub_ps_c(int16_t *a, intptr_t dstride, pixel *b0, pixel *b1, intptr_t sstride0, intptr_t sstride1)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
        {
            a[x] = (int16_t)(b0[x] - b1[x]);
        }

        b0 += sstride0;
        b1 += sstride1;
        a += dstride;
    }
}

template<int bx, int by>
void pixel_add_ps_c(pixel *a, intptr_t dstride, pixel *b0, int16_t *b1, intptr_t sstride0, intptr_t sstride1)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
        {
            a[x] = Clip(b0[x] + b1[x]);
        }

        b0 += sstride0;
        b1 += sstride1;
        a += dstride;
    }
}

template<int bx, int by>
void addAvg(int16_t* src0, int16_t* src1, pixel* dst, intptr_t src0Stride, intptr_t src1Stride, intptr_t dstStride)
{
    int shiftNum, offset;

    shiftNum = IF_INTERNAL_PREC + 1 - X265_DEPTH;
    offset = (1 << (shiftNum - 1)) + 2 * IF_INTERNAL_OFFS;

    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x += 2)
        {
            dst[x + 0] = Clip((src0[x + 0] + src1[x + 0] + offset) >> shiftNum);
            dst[x + 1] = Clip((src0[x + 1] + src1[x + 1] + offset) >> shiftNum);
        }

        src0 += src0Stride;
        src1 += src1Stride;
        dst  += dstStride;
    }
}

void planecopy_cp_c(uint8_t *src, intptr_t srcStride, pixel *dst, intptr_t dstStride, int width, int height, int shift)
{
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            dst[c] = ((pixel)src[c]) << shift;
        }

        dst += dstStride;
        src += srcStride;
    }
}

void planecopy_sp_c(uint16_t *src, intptr_t srcStride, pixel *dst, intptr_t dstStride, int width, int height, int shift, uint16_t mask)
{
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            dst[c] = (pixel)((src[c] >> shift) & mask);
        }

        dst += dstStride;
        src += srcStride;
    }
}
}  // end anonymous namespace

namespace x265 {
// x265 private namespace

/* Extend the edges of a picture so that it may safely be used for motion
 * compensation. This function assumes the picture is stored in a buffer with
 * sufficient padding for the X and Y margins */
void extendPicBorder(pixel* pic, int stride, int width, int height, int marginX, int marginY)
{
    /* extend left and right margins */
    primitives.extendRowBorder(pic, stride, width, height, marginX);

    /* copy top row to create above margin */
    pixel *top = pic - marginX;
    for (int y = 0; y < marginY; y++)
    {
        memcpy(top - (y + 1) * stride, top, stride * sizeof(pixel));
    }

    /* copy bottom row to create below margin */
    pixel *bot = pic - marginX + (height - 1) * stride;
    for (int y = 0; y < marginY; y++)
    {
        memcpy(bot + (y + 1) * stride, bot, stride * sizeof(pixel));
    }
}

/* Initialize entries for pixel functions defined in this file */
void Setup_C_PixelPrimitives(EncoderPrimitives &p)
{
    SET_FUNC_PRIMITIVE_TABLE_C2(sad)
    SET_FUNC_PRIMITIVE_TABLE_C2(sad_x3)
    SET_FUNC_PRIMITIVE_TABLE_C2(sad_x4)
    SET_FUNC_PRIMITIVE_TABLE_C2(pixelavg_pp)

    // satd
    p.satd[LUMA_4x4]   = satd_4x4;
    p.satd[LUMA_8x8]   = satd8<8, 8>;
    p.satd[LUMA_8x4]   = satd_8x4;
    p.satd[LUMA_4x8]   = satd4<4, 8>;
    p.satd[LUMA_16x16] = satd8<16, 16>;
    p.satd[LUMA_16x8]  = satd8<16, 8>;
    p.satd[LUMA_8x16]  = satd8<8, 16>;
    p.satd[LUMA_16x12] = satd8<16, 12>;
    p.satd[LUMA_12x16] = satd4<12, 16>;
    p.satd[LUMA_16x4]  = satd8<16, 4>;
    p.satd[LUMA_4x16]  = satd4<4, 16>;
    p.satd[LUMA_32x32] = satd8<32, 32>;
    p.satd[LUMA_32x16] = satd8<32, 16>;
    p.satd[LUMA_16x32] = satd8<16, 32>;
    p.satd[LUMA_32x24] = satd8<32, 24>;
    p.satd[LUMA_24x32] = satd8<24, 32>;
    p.satd[LUMA_32x8]  = satd8<32, 8>;
    p.satd[LUMA_8x32]  = satd8<8, 32>;
    p.satd[LUMA_64x64] = satd8<64, 64>;
    p.satd[LUMA_64x32] = satd8<64, 32>;
    p.satd[LUMA_32x64] = satd8<32, 64>;
    p.satd[LUMA_64x48] = satd8<64, 48>;
    p.satd[LUMA_48x64] = satd8<48, 64>;
    p.satd[LUMA_64x16] = satd8<64, 16>;
    p.satd[LUMA_16x64] = satd8<16, 64>;

#define CHROMA_420(W, H) \
    p.chroma[X265_CSP_I420].addAvg[CHROMA_ ## W ## x ## H]  = addAvg<W, H>; \
    p.chroma[X265_CSP_I420].copy_pp[CHROMA_ ## W ## x ## H] = blockcopy_pp_c<W, H>; \
    p.chroma[X265_CSP_I420].copy_sp[CHROMA_ ## W ## x ## H] = blockcopy_sp_c<W, H>; \
    p.chroma[X265_CSP_I420].copy_ps[CHROMA_ ## W ## x ## H] = blockcopy_ps_c<W, H>; \
    p.chroma[X265_CSP_I420].copy_ss[CHROMA_ ## W ## x ## H] = blockcopy_ss_c<W, H>; \
    p.chroma[X265_CSP_I420].sub_ps[CHROMA_ ## W ## x ## H] = pixel_sub_ps_c<W, H>; \
    p.chroma[X265_CSP_I420].add_ps[CHROMA_ ## W ## x ## H] = pixel_add_ps_c<W, H>;

#define CHROMA_444(W, H) \
    p.chroma[X265_CSP_I444].addAvg[LUMA_ ## W ## x ## H]  = addAvg<W, H>; \
    p.chroma[X265_CSP_I444].copy_pp[LUMA_ ## W ## x ## H] = blockcopy_pp_c<W, H>; \
    p.chroma[X265_CSP_I444].copy_sp[LUMA_ ## W ## x ## H] = blockcopy_sp_c<W, H>; \
    p.chroma[X265_CSP_I444].copy_ps[LUMA_ ## W ## x ## H] = blockcopy_ps_c<W, H>; \
    p.chroma[X265_CSP_I444].copy_ss[LUMA_ ## W ## x ## H] = blockcopy_ss_c<W, H>; \
    p.chroma[X265_CSP_I444].sub_ps[LUMA_ ## W ## x ## H] = pixel_sub_ps_c<W, H>; \
    p.chroma[X265_CSP_I444].add_ps[LUMA_ ## W ## x ## H] = pixel_add_ps_c<W, H>;

#define LUMA(W, H) \
    p.luma_addAvg[LUMA_ ## W ## x ## H]  = addAvg<W, H>; \
    p.luma_copy_pp[LUMA_ ## W ## x ## H] = blockcopy_pp_c<W, H>; \
    p.luma_copy_sp[LUMA_ ## W ## x ## H] = blockcopy_sp_c<W, H>; \
    p.luma_copy_ps[LUMA_ ## W ## x ## H] = blockcopy_ps_c<W, H>; \
    p.luma_copy_ss[LUMA_ ## W ## x ## H] = blockcopy_ss_c<W, H>; \
    p.luma_sub_ps[LUMA_ ## W ## x ## H] = pixel_sub_ps_c<W, H>; \
    p.luma_add_ps[LUMA_ ## W ## x ## H] = pixel_add_ps_c<W, H>;

    LUMA(4, 4);
    LUMA(8, 8);
    CHROMA_420(4, 4);
    LUMA(4, 8);
    CHROMA_420(2, 4);
    LUMA(8, 4);
    CHROMA_420(4, 2);
    LUMA(16, 16);
    CHROMA_420(8,  8);
    LUMA(16,  8);
    CHROMA_420(8,  4);
    LUMA(8, 16);
    CHROMA_420(4,  8);
    LUMA(16, 12);
    CHROMA_420(8,  6);
    LUMA(12, 16);
    CHROMA_420(6,  8);
    LUMA(16,  4);
    CHROMA_420(8,  2);
    LUMA(4, 16);
    CHROMA_420(2,  8);
    LUMA(32, 32);
    CHROMA_420(16, 16);
    LUMA(32, 16);
    CHROMA_420(16, 8);
    LUMA(16, 32);
    CHROMA_420(8,  16);
    LUMA(32, 24);
    CHROMA_420(16, 12);
    LUMA(24, 32);
    CHROMA_420(12, 16);
    LUMA(32,  8);
    CHROMA_420(16, 4);
    LUMA(8, 32);
    CHROMA_420(4,  16);
    LUMA(64, 64);
    CHROMA_420(32, 32);
    LUMA(64, 32);
    CHROMA_420(32, 16);
    LUMA(32, 64);
    CHROMA_420(16, 32);
    LUMA(64, 48);
    CHROMA_420(32, 24);
    LUMA(48, 64);
    CHROMA_420(24, 32);
    LUMA(64, 16);
    CHROMA_420(32, 8);
    LUMA(16, 64);
    CHROMA_420(8,  32);

    CHROMA_444(4,  4);
    CHROMA_444(8,  8);
    CHROMA_444(4,  8);
    CHROMA_444(8,  4);
    CHROMA_444(16, 16);
    CHROMA_444(16, 8);
    CHROMA_444(8,  16);
    CHROMA_444(16, 12);
    CHROMA_444(12, 16);
    CHROMA_444(16, 4);
    CHROMA_444(4,  16);
    CHROMA_444(32, 32);
    CHROMA_444(32, 16);
    CHROMA_444(16, 32);
    CHROMA_444(32, 24);
    CHROMA_444(24, 32);
    CHROMA_444(32, 8);
    CHROMA_444(8,  32);
    CHROMA_444(64, 64);
    CHROMA_444(64, 32);
    CHROMA_444(32, 64);
    CHROMA_444(64, 48);
    CHROMA_444(48, 64);
    CHROMA_444(64, 16);
    CHROMA_444(16, 64);

    SET_FUNC_PRIMITIVE_TABLE_C(sse_pp, sse, pixelcmp_t, pixel, pixel)
    SET_FUNC_PRIMITIVE_TABLE_C(sse_sp, sse, pixelcmp_sp_t, int16_t, pixel)
    SET_FUNC_PRIMITIVE_TABLE_C(sse_ss, sse, pixelcmp_ss_t, int16_t, int16_t)

    p.blockcpy_pp = blockcopy_p_p;
    p.blockcpy_ps = blockcopy_p_s;

    p.blockfill_s[BLOCK_4x4]   = blockfil_s_c<4>;
    p.blockfill_s[BLOCK_8x8]   = blockfil_s_c<8>;
    p.blockfill_s[BLOCK_16x16] = blockfil_s_c<16>;
    p.blockfill_s[BLOCK_32x32] = blockfil_s_c<32>;
    p.blockfill_s[BLOCK_64x64] = blockfil_s_c<64>;

    p.cvt16to32_shl = convert16to32_shl;
    p.cvt32to16_shr = convert32to16_shr;

    p.sa8d[BLOCK_4x4]   = satd_4x4;
    p.sa8d[BLOCK_8x8]   = sa8d_8x8;
    p.sa8d[BLOCK_16x16] = sa8d_16x16;
    p.sa8d[BLOCK_32x32] = sa8d16<32, 32>;
    p.sa8d[BLOCK_64x64] = sa8d16<64, 64>;

    p.sa8d_inter[LUMA_4x4]   = satd_4x4;
    p.sa8d_inter[LUMA_8x8]   = sa8d_8x8;
    p.sa8d_inter[LUMA_8x4]   = satd_8x4;
    p.sa8d_inter[LUMA_4x8]   = satd4<4, 8>;
    p.sa8d_inter[LUMA_16x16] = sa8d_16x16;
    p.sa8d_inter[LUMA_16x8]  = sa8d8<16, 8>;
    p.sa8d_inter[LUMA_8x16]  = sa8d8<8, 16>;
    p.sa8d_inter[LUMA_16x12] = satd8<16, 12>;
    p.sa8d_inter[LUMA_12x16] = satd4<12, 16>;
    p.sa8d_inter[LUMA_4x16]  = satd4<4, 16>;
    p.sa8d_inter[LUMA_16x4]  = satd8<16, 4>;
    p.sa8d_inter[LUMA_32x32] = sa8d16<32, 32>;
    p.sa8d_inter[LUMA_32x16] = sa8d16<32, 16>;
    p.sa8d_inter[LUMA_16x32] = sa8d16<16, 32>;
    p.sa8d_inter[LUMA_32x24] = sa8d8<32, 24>;
    p.sa8d_inter[LUMA_24x32] = sa8d8<24, 32>;
    p.sa8d_inter[LUMA_32x8]  = sa8d8<32, 8>;
    p.sa8d_inter[LUMA_8x32]  = sa8d8<8, 32>;
    p.sa8d_inter[LUMA_64x64] = sa8d16<64, 64>;
    p.sa8d_inter[LUMA_64x32] = sa8d16<64, 32>;
    p.sa8d_inter[LUMA_32x64] = sa8d16<32, 64>;
    p.sa8d_inter[LUMA_64x48] = sa8d16<64, 48>;
    p.sa8d_inter[LUMA_48x64] = sa8d16<48, 64>;
    p.sa8d_inter[LUMA_64x16] = sa8d16<64, 16>;
    p.sa8d_inter[LUMA_16x64] = sa8d16<16, 64>;

    p.calcresidual[BLOCK_4x4] = getResidual<4>;
    p.calcresidual[BLOCK_8x8] = getResidual<8>;
    p.calcresidual[BLOCK_16x16] = getResidual<16>;
    p.calcresidual[BLOCK_32x32] = getResidual<32>;
    p.calcresidual[BLOCK_64x64] = NULL;
    p.calcrecon[BLOCK_4x4] = calcRecons<4>;
    p.calcrecon[BLOCK_8x8] = calcRecons<8>;
    p.calcrecon[BLOCK_16x16] = calcRecons<16>;
    p.calcrecon[BLOCK_32x32] = calcRecons<32>;
    p.calcrecon[BLOCK_64x64] = NULL;

    p.transpose[0] = transpose<4>;
    p.transpose[1] = transpose<8>;
    p.transpose[2] = transpose<16>;
    p.transpose[3] = transpose<32>;
    p.transpose[4] = transpose<64>;

    p.weight_pp = weight_pp_c;
    p.weight_sp = weight_sp_c;

    p.pixeladd_ss = pixeladd_ss_c;

    p.scale1D_128to64 = scale1D_128to64;
    p.scale2D_64to32 = scale2D_64to32;
    p.frame_init_lowres_core = frame_init_lowres_core;
    p.ssim_4x4x2_core = ssim_4x4x2_core;
    p.ssim_end_4 = ssim_end_4;

    p.var[BLOCK_8x8] = pixel_var<8>;
    p.var[BLOCK_16x16] = pixel_var<16>;
    p.var[BLOCK_32x32] = pixel_var<32>;
    p.var[BLOCK_64x64] = pixel_var<64>;
    p.plane_copy_deinterleave_c = plane_copy_deinterleave_chroma;
    p.planecopy_cp = planecopy_cp_c;
    p.planecopy_sp = planecopy_sp_c;
}
}

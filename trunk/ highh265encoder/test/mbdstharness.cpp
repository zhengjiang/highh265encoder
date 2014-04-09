/*****************************************************************************
 * Copyright (C) 2013 x265 project
 *
 * Authors: Steve Borho <steve@borho.org>
 *          Min Chen <min.chen@multicorewareinc.com>
 *          Praveen Kumar Tiwari <praveen@multicorewareinc.com>
 *          Nabajit Deka <nabajit@multicorewareinc.com>
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

#include "mbdstharness.h"
#include "TLibCommon/TComRom.h"
#include "common.h"

#define ITERS  100
#define TEST_CASES 3
using namespace x265;
struct DctConf_t
{
    const char *name;
    int width;
};

const DctConf_t DctConf_infos[] =
{
    { "dst4x4\t",    4 },
    { "dct4x4\t",    4 },
    { "dct8x8\t",    8 },
    { "dct16x16",   16 },
    { "dct32x32",   32 },
};

const DctConf_t IDctConf_infos[] =
{
    { "idst4x4\t",    4 },
    { "idct4x4\t",    4 },
    { "idct8x8\t",    8 },
    { "idct16x16",   16 },
    { "idct32x32",   32 },
};

MBDstHarness::MBDstHarness()
{
    const int idct_max = (1 << (BIT_DEPTH + 4)) - 1;
    CHECKED_MALLOC(mbuf1, int16_t, mb_t_size);
    CHECKED_MALLOC(mbufdct, int16_t, mb_t_size);
    CHECKED_MALLOC(mbufidct, int, mb_t_size);

    CHECKED_MALLOC(mbuf2, int16_t, mem_cmp_size);
    CHECKED_MALLOC(mbuf3, int16_t, mem_cmp_size);
    CHECKED_MALLOC(mbuf4, int16_t, mem_cmp_size);

    CHECKED_MALLOC(mintbuf1, int, mb_t_size);
    CHECKED_MALLOC(mintbuf2, int, mb_t_size);
    CHECKED_MALLOC(mintbuf3, int, mem_cmp_size);
    CHECKED_MALLOC(mintbuf4, int, mem_cmp_size);
    CHECKED_MALLOC(mintbuf5, int, mem_cmp_size);
    CHECKED_MALLOC(mintbuf6, int, mem_cmp_size);
    CHECKED_MALLOC(mintbuf7, int, mem_cmp_size);
    CHECKED_MALLOC(mintbuf8, int, mem_cmp_size);

    CHECKED_MALLOC(short_test_buff, int16_t*, TEST_CASES);
    CHECKED_MALLOC(int_test_buff, int*, TEST_CASES);
    CHECKED_MALLOC(int_idct_test_buff, int*, TEST_CASES);

    for (int i = 0; i < TEST_CASES; i++)
    {
        CHECKED_MALLOC(short_test_buff[i], int16_t, mb_t_size);
        CHECKED_MALLOC(int_test_buff[i], int, mb_t_size);
        CHECKED_MALLOC(int_idct_test_buff[i], int, mb_t_size);
    }

    /* [0] --- Random values
     * [1] --- Minimum
     * [2] --- Maximum */
    for (int i = 0; i < mb_t_size; i++)
    {
        short_test_buff[0][i]    = (rand() & PIXEL_MAX) - (rand() & PIXEL_MAX);
        int_test_buff[0][i]      = rand() % PIXEL_MAX;
        int_idct_test_buff[0][i] = (rand() % (SHORT_MAX - SHORT_MIN)) - SHORT_MAX;
        short_test_buff[1][i]    = -PIXEL_MAX;
        int_test_buff[1][i]      = -PIXEL_MAX;
        int_idct_test_buff[1][i] = SHORT_MIN;
        short_test_buff[2][i]    = PIXEL_MAX;
        int_test_buff[2][i]      = PIXEL_MAX;
        int_idct_test_buff[2][i] = SHORT_MAX;
    }

    for (int i = 0; i < mb_t_size; i++)
    {
        mbuf1[i] = rand() & PIXEL_MAX;
        mbufdct[i] = (rand() & PIXEL_MAX) - (rand() & PIXEL_MAX);
        mbufidct[i] = (rand() & idct_max);
    }

    for (int i = 0; i < mb_t_size; i++)
    {
        mintbuf1[i] = rand() & PIXEL_MAX;
        mintbuf2[i] = rand() & PIXEL_MAX;
    }

#if _DEBUG
    memset(mbuf2, 0, mem_cmp_size);
    memset(mbuf3, 0, mem_cmp_size);
    memset(mbuf4, 0, mem_cmp_size);
    memset(mbufidct, 0, mb_t_size);

    memset(mintbuf3, 0, mem_cmp_size);
    memset(mintbuf4, 0, mem_cmp_size);
    memset(mintbuf5, 0, mem_cmp_size);
    memset(mintbuf6, 0, mem_cmp_size);
    memset(mintbuf7, 0, mem_cmp_size);
    memset(mintbuf8, 0, mem_cmp_size);
#endif // if _DEBUG
    return;

fail:
    exit(1);
}

MBDstHarness::~MBDstHarness()
{
    X265_FREE(mbuf1);
    X265_FREE(mbuf2);
    X265_FREE(mbuf3);
    X265_FREE(mbuf4);
    X265_FREE(mbufdct);
    X265_FREE(mbufidct);

    X265_FREE(mintbuf1);
    X265_FREE(mintbuf2);
    X265_FREE(mintbuf3);
    X265_FREE(mintbuf4);
    X265_FREE(mintbuf5);
    X265_FREE(mintbuf6);
    X265_FREE(mintbuf7);
    X265_FREE(mintbuf8);
    for (int i = 0; i < TEST_CASES; i++)
    {
        X265_FREE(short_test_buff[i]);
        X265_FREE(int_test_buff[i]);
        X265_FREE(int_idct_test_buff[i]);
    }

    X265_FREE(short_test_buff);
    X265_FREE(int_test_buff);
    X265_FREE(int_idct_test_buff);
}

bool MBDstHarness::check_dct_primitive(dct_t ref, dct_t opt, int width)
{
    int j = 0;
    int cmp_size = sizeof(int) * width * width;
    for (int i = 0; i <= 100; i++)
    {
        int index = rand() % TEST_CASES;
        ref(short_test_buff[index] + j, mintbuf3, width);
        opt(short_test_buff[index] + j, mintbuf4, width);

        if (memcmp(mintbuf3, mintbuf4, cmp_size))
        {
#if _DEBUG
            // redo for debug
            ref(short_test_buff[index] + j, mintbuf3, width);
            opt(short_test_buff[index] + j, mintbuf4, width);
#endif

            return false;
        }
        j += 16;
#if _DEBUG
        memset(mbuf2, 0xCD, mem_cmp_size);
        memset(mbuf3, 0xCD, mem_cmp_size);
#endif
    }

    return true;
}

bool MBDstHarness::check_idct_primitive(idct_t ref, idct_t opt, int width)
{
    int j = 0;
    int cmp_size = sizeof(int16_t) * width * width;
    for (int i = 0; i <= 100; i++)
    {
        int index = rand() % TEST_CASES;
        ref(int_idct_test_buff[index] + j, mbuf2, width);
        opt(int_idct_test_buff[index] + j, mbuf3, width);
        if (memcmp(mbuf2, mbuf3, cmp_size))
        {
#if _DEBUG
            // redo for debug
            ref(int_idct_test_buff[index] + j, mbuf2, width);
            opt(int_idct_test_buff[index] + j, mbuf3, width);
#endif

            return false;
        }
        j += 16;
#if _DEBUG
        memset(mbuf2, 0xCD, mem_cmp_size);
        memset(mbuf3, 0xCD, mem_cmp_size);
#endif
    }

    return true;
}

bool MBDstHarness::check_dequant_primitive(dequant_normal_t ref, dequant_normal_t opt)
{
    int j = 0;

    for (int i = 0; i <= 5; i++)
    {
        int log2TrSize = (rand() % 4) + 2;

        int width = (1 << log2TrSize);
        int height = width;
#if HIGH_BIT_DEPTH
        int qp = rand() % 64;
#else
        int qp = rand() % 51;
#endif
        int per = qp / 6;
        int rem = qp % 6;
        static const int invQuantScales[6] = { 40, 45, 51, 57, 64, 72 };
        int scale = invQuantScales[rem] << per;
        int transformShift = MAX_TR_DYNAMIC_RANGE - X265_DEPTH - log2TrSize;
        int shift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - transformShift;

        int cmp_size = sizeof(int) * height * width;
        int index = rand() % TEST_CASES;

        ref(int_test_buff[index] + j, mintbuf3, width * height, scale, shift);
        opt(int_test_buff[index] + j, mintbuf4, width * height, scale, shift);

        if (memcmp(mintbuf3, mintbuf4, cmp_size))
            return false;

        j += 16;
#if _DEBUG
        memset(mintbuf3, 0xCD, mem_cmp_size);
        memset(mintbuf4, 0xCD, mem_cmp_size);
#endif
    }

    return true;
}

bool MBDstHarness::check_dequant_primitive(dequant_scaling_t ref, dequant_scaling_t opt)
{
    int j = 0;

    for (int i = 0; i <= 5; i++)
    {
        int log2TrSize = (rand() % 4) + 2;

        int width = (1 << log2TrSize);
        int height = width;

        int qp = rand() % 52;
        int per = qp / 6;
        int transformShift = MAX_TR_DYNAMIC_RANGE - X265_DEPTH - log2TrSize;
        int shift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - transformShift;

        int cmp_size = sizeof(int) * height * width;
        int index1 = rand() % TEST_CASES;
        int index2 = rand() % TEST_CASES;

        ref(int_test_buff[index1] + j, mintbuf3, int_test_buff[index2] + j, width * height, per, shift);
        opt(int_test_buff[index1] + j, mintbuf4, int_test_buff[index2] + j, width * height, per, shift);

        if (memcmp(mintbuf3, mintbuf4, cmp_size))
            return false;

        j += 16;
#if _DEBUG
        memset(mintbuf3, 0xCD, mem_cmp_size);
        memset(mintbuf4, 0xCD, mem_cmp_size);
#endif
    }

    return true;
}

bool MBDstHarness::check_quant_primitive(quant_t ref, quant_t opt)
{
    int j = 0;

    for (int i = 0; i <= ITERS; i++)
    {
        int width = (rand() % 4 + 1) * 4;

        if (width == 12)
        {
            width = 32;
        }
        int height = width;

        uint32_t optReturnValue = 0;
        uint32_t refReturnValue = 0;

        int bits = rand() % 32;
        int valueToAdd = rand() % (32 * 1024);
        int cmp_size = sizeof(int) * height * width;
        int numCoeff = height * width;
        int optLastPos = -1, refLastPos = -1;

        int index1 = rand() % TEST_CASES;
        int index2 = rand() % TEST_CASES;

        refReturnValue = ref(int_test_buff[index1] + j, int_test_buff[index2] + j, mintbuf5, mintbuf6, bits, valueToAdd, numCoeff, &refLastPos);
        optReturnValue = opt(int_test_buff[index1] + j, int_test_buff[index2] + j, mintbuf3, mintbuf4, bits, valueToAdd, numCoeff, &optLastPos);

        if (memcmp(mintbuf3, mintbuf5, cmp_size))
            return false;

        if (memcmp(mintbuf4, mintbuf6, cmp_size))
            return false;

        if (optReturnValue != refReturnValue)
            return false;

        if (optLastPos != refLastPos)
            return false;

        j += 16;

#if _DEBUG
        memset(mintbuf3, 0xCD, mem_cmp_size);
        memset(mintbuf4, 0xCD, mem_cmp_size);
        memset(mintbuf5, 0xCD, mem_cmp_size);
        memset(mintbuf6, 0xCD, mem_cmp_size);
#endif
    }

    return true;
}

bool MBDstHarness::check_count_nonzero_primitive(count_nonzero_t ref, count_nonzero_t opt)
{
    ALIGN_VAR_32(int32_t, qcoeff[32 * 32]);

    for (int i = 0; i < 4; i++)
    {
        int log2TrSize = i + 2;
        int num = 1 << (log2TrSize * 2);
        int mask = num - 1;

        for (int n = 0; n <= num; n++)
        {
            memset(qcoeff, 0, num * sizeof(int32_t));

            for (int j = 0; j < n; j++)
            {
                int k = rand() & mask;
                while (qcoeff[k])
                {
                    k = (k + 11) & mask;
                }

                qcoeff[k] = rand() - RAND_MAX / 2;
            }

            int refval = ref(qcoeff, num);
            int optval = opt(qcoeff, num);

            if (refval != optval)
                return false;
        }
    }

    return true;
}

bool MBDstHarness::testCorrectness(const EncoderPrimitives& ref, const EncoderPrimitives& opt)
{
    for (int i = 0; i < NUM_DCTS; i++)
    {
        if (opt.dct[i])
        {
            if (!check_dct_primitive(ref.dct[i], opt.dct[i], DctConf_infos[i].width))
            {
                printf("\n%s failed\n", DctConf_infos[i].name);
                return false;
            }
        }
    }

    for (int i = 0; i < NUM_IDCTS; i++)
    {
        if (opt.idct[i])
        {
            if (!check_idct_primitive(ref.idct[i], opt.idct[i], IDctConf_infos[i].width))
            {
                printf("%s failed\n", IDctConf_infos[i].name);
                return false;
            }
        }
    }

    if (opt.dequant_normal)
    {
        if (!check_dequant_primitive(ref.dequant_normal, opt.dequant_normal))
        {
            printf("dequant: Failed!\n");
            return false;
        }
    }

    if (opt.quant)
    {
        if (!check_quant_primitive(ref.quant, opt.quant))
        {
            printf("quant: Failed!\n");
            return false;
        }
    }

    if (opt.count_nonzero)
    {
        if (!check_count_nonzero_primitive(ref.count_nonzero, opt.count_nonzero))
        {
            printf("count_nonzero: Failed!\n");
            return false;
        }
    }

    return true;
}

void MBDstHarness::measureSpeed(const EncoderPrimitives& ref, const EncoderPrimitives& opt)
{
    for (int value = 0; value < NUM_DCTS; value++)
    {
        if (opt.dct[value])
        {
            printf("%s\t", DctConf_infos[value].name);
            REPORT_SPEEDUP(opt.dct[value], ref.dct[value], mbuf1, mintbuf3, DctConf_infos[value].width);
        }
    }

    for (int value = 0; value < NUM_IDCTS; value++)
    {
        if (opt.idct[value])
        {
            printf("%s\t", IDctConf_infos[value].name);
            REPORT_SPEEDUP(opt.idct[value], ref.idct[value], mbufidct, mbuf2, IDctConf_infos[value].width);
        }
    }

    if (opt.dequant_normal)
    {
        printf("dequant_normal\t");
        REPORT_SPEEDUP(opt.dequant_normal, ref.dequant_normal, mintbuf1, mintbuf3, 32 * 32, 70, 1);
    }

    if (opt.dequant_scaling)
    {
        printf("dequant_scaling\t");
        REPORT_SPEEDUP(opt.dequant_scaling, ref.dequant_scaling, mintbuf1, mintbuf3, mintbuf2, 32 * 32, 5, 1);
    }

    if (opt.quant)
    {
        printf("quant\t\t");
        int dummy = -1;
        REPORT_SPEEDUP(opt.quant, ref.quant, mintbuf1, mintbuf2, mintbuf3, mintbuf4, 23, 23785, 32 * 32, &dummy);
    }

    if (opt.count_nonzero)
    {
        for (int i = 4; i <= 32; i <<= 1)
        {
            printf("count_nonzero[%dx%d]", i, i);
            REPORT_SPEEDUP(opt.count_nonzero, ref.count_nonzero, mbufidct, i * i)
        }
    }
}

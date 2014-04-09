/*****************************************************************************
 * Copyright (C) 2013 x265 project
 *
 * Authors: Steve Borho <steve@borho.org>
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

#include "TLibCommon/CommonDef.h"
#include "param.h"
#include "encoder.h"
#include "frameencoder.h"

using namespace x265;

extern "C"
x265_encoder *x265_encoder_open(x265_param *param)
{
    if (!param)
        return NULL;

    x265_log(param, X265_LOG_INFO, "HEVC encoder version %s\n", x265_version_str);
    x265_log(param, X265_LOG_INFO, "build info %s\n", x265_build_info_str);

    x265_setup_primitives(param, param->cpuid);

    if (x265_check_params(param))
        return NULL;

    if (x265_set_globals(param))
        return NULL;

    Encoder *encoder = new Encoder;
    if (encoder)
    {
        // these may change params for auto-detect, etc
        encoder->determineLevelAndProfile(param);
        encoder->configure(param);

        encoder->param = X265_MALLOC(x265_param, 1);
        if (!encoder->param)
        {
            encoder->destroy();
            return NULL;
        }

        memcpy(encoder->param, param, sizeof(x265_param));
        x265_print_params(param);
        encoder->create();
        encoder->init();
    }

    return encoder;
}

extern "C"
int x265_encoder_headers(x265_encoder *enc, x265_nal **pp_nal, uint32_t *pi_nal)
{
    if (!pp_nal || !enc)
        return -1;

    Encoder *encoder = static_cast<Encoder*>(enc);

    int ret = 0;
    NALUnitEBSP *nalunits[MAX_NAL_UNITS] = { 0, 0, 0, 0, 0 };
    if (encoder->getStreamHeaders(nalunits) > 0)
    {
        int nalcount = encoder->extractNalData(nalunits, ret);
        *pp_nal = &encoder->m_nals[0];
        if (pi_nal) *pi_nal = nalcount;
    }
    else if (pi_nal)
    {
        *pi_nal = 0;
        ret = -1;
    }

    for (int i = 0; i < MAX_NAL_UNITS; i++)
    {
        if (nalunits[i])
        {
            free(nalunits[i]->m_nalUnitData);
            X265_FREE(nalunits[i]);
        }
    }

    return ret;
}

extern "C"
int x265_encoder_encode(x265_encoder *enc, x265_nal **pp_nal, uint32_t *pi_nal, x265_picture *pic_in, x265_picture *pic_out)
{
    if (!enc)
        return -1;

    Encoder *encoder = static_cast<Encoder*>(enc);
    NALUnitEBSP *nalunits[MAX_NAL_UNITS] = { 0, 0, 0, 0, 0 };
    int numEncoded = encoder->encode(!pic_in, pic_in, pic_out, nalunits);

    if (pp_nal && numEncoded > 0)
    {
        int memsize;
        int nalcount = encoder->extractNalData(nalunits, memsize);
        *pp_nal = &encoder->m_nals[0];
        if (pi_nal) *pi_nal = nalcount;
    }
    else if (pi_nal)
        *pi_nal = 0;

    for (int i = 0; i < MAX_NAL_UNITS; i++)
    {
        if (nalunits[i])
        {
            free(nalunits[i]->m_nalUnitData);
            X265_FREE(nalunits[i]);
        }
    }

    return numEncoded;
}

extern "C"
void x265_encoder_get_stats(x265_encoder *enc, x265_stats *outputStats, uint32_t statsSizeBytes)
{
    if (enc && outputStats)
    {
        Encoder *encoder = static_cast<Encoder*>(enc);
        encoder->fetchStats(outputStats, statsSizeBytes);
    }
}

extern "C"
void x265_encoder_log(x265_encoder* enc, int argc, char **argv)
{
    if (enc)
    {
        Encoder *encoder = static_cast<Encoder*>(enc);
        encoder->writeLog(argc, argv);
    }
}

extern "C"
void x265_encoder_close(x265_encoder *enc)
{
    if (enc)
    {
        Encoder *encoder = static_cast<Encoder*>(enc);

        encoder->printSummary();
        encoder->destroy();
        delete encoder;
    }
}

extern "C"
void x265_cleanup(void)
{
    destroyROM();
    BitCost::destroy();
}

extern "C"
x265_picture *x265_picture_alloc()
{
    return (x265_picture*)x265_malloc(sizeof(x265_picture));
}

extern "C"
void x265_picture_init(x265_param *param, x265_picture *pic)
{
    memset(pic, 0, sizeof(x265_picture));

    pic->bitDepth = param->internalBitDepth;
    pic->colorSpace = param->internalCsp;
}

extern "C"
void x265_picture_free(x265_picture *p)
{
    return x265_free(p);
}

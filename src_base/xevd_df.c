﻿/* Copyright (c) 2020, Samsung Electronics Co., Ltd.
   All Rights Reserved. */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   - Neither the name of the copyright owner, nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/
#include "xevd_def.h"
#include "xevd_df.h"
#include "xevd_tbl.h"

const u8 * get_tbl_qp_to_st(u32 mcu0, u32 mcu1, s8 *refi0, s8 *refi1, s16 (*mv0)[MV_D], s16 (*mv1)[MV_D])
{
    int idx = 3;

    if(MCU_GET_IF(mcu0) || MCU_GET_IF(mcu1))
    {
        idx = 0;
    }
    else if(MCU_GET_CBFL(mcu0) == 1 || MCU_GET_CBFL(mcu1) == 1)
    {
        idx = 1;
    }
    else
    {
        int mv0_l0[2] = {mv0[REFP_0][MV_X], mv0[REFP_0][MV_Y]};
        int mv0_l1[2] = {mv0[REFP_1][MV_X], mv0[REFP_1][MV_Y]};
        int mv1_l0[2] = {mv1[REFP_0][MV_X], mv1[REFP_0][MV_Y]};
        int mv1_l1[2] = {mv1[REFP_1][MV_X], mv1[REFP_1][MV_Y]};

        if(!REFI_IS_VALID(refi0[REFP_0]))
        {
            mv0_l0[0] = mv0_l0[1] = 0;
        }

        if(!REFI_IS_VALID(refi0[REFP_1]))
        {
            mv0_l1[0] = mv0_l1[1] = 0;
        }

        if(!REFI_IS_VALID(refi1[REFP_0]))
        {
            mv1_l0[0] = mv1_l0[1] = 0;
        }

        if(!REFI_IS_VALID(refi1[REFP_1]))
        {
            mv1_l1[0] = mv1_l1[1] = 0;
        }

        if(((refi0[REFP_0] == refi1[REFP_0]) && (refi0[REFP_1] == refi1[REFP_1])))
        {
            idx = (XEVD_ABS(mv0_l0[MV_X] - mv1_l0[MV_X]) >= 4 ||
                   XEVD_ABS(mv0_l0[MV_Y] - mv1_l0[MV_Y]) >= 4 ||
                   XEVD_ABS(mv0_l1[MV_X] - mv1_l1[MV_X]) >= 4 ||
                   XEVD_ABS(mv0_l1[MV_Y] - mv1_l1[MV_Y]) >= 4 ) ? 2 : 3;
        }
        else if((refi0[REFP_0] == refi1[REFP_1]) && (refi0[REFP_1] == refi1[REFP_0]))
        {
            idx = (XEVD_ABS(mv0_l0[MV_X] - mv1_l1[MV_X]) >= 4 ||
                   XEVD_ABS(mv0_l0[MV_Y] - mv1_l1[MV_Y]) >= 4 ||
                   XEVD_ABS(mv0_l1[MV_X] - mv1_l0[MV_X]) >= 4 ||
                   XEVD_ABS(mv0_l1[MV_Y] - mv1_l0[MV_Y]) >= 4) ? 2 : 3;
        }
        else
        {
            idx = 2;
        }
    }

    return xevd_tbl_df_st[idx];
}

void deblock_scu_hor(pel *buf, int qp, int stride, int is_luma, const u8 *tbl_qp_to_st

    , int bit_depth_minus8, int chroma_format_idc

)
{
    s16 A, B, C, D, d, d1, d2;
    s16 abs, t16, clip, sign, st;
    int i, size;


    st = tbl_qp_to_st[qp] << bit_depth_minus8;
    size = (is_luma ? MIN_CU_SIZE : (MIN_CU_SIZE >> (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc))));


    if(st)
    {
        for(i = 0; i < size; i++)
        {
            A = buf[-2 * stride];
            B = buf[-stride];
            C = buf[0];
            D = buf[stride];

            d = (A - (B << 2) + (C << 2) - D) / 8;

            abs = XEVD_ABS16(d);
            sign = XEVD_SIGN_GET(d);

            t16 = XEVD_MAX(0, ((abs - st) << 1));
            clip = XEVD_MAX(0, (abs - t16));
            d1 = XEVD_SIGN_SET(clip, sign);
            clip >>= 1;
            d2 = XEVD_CLIP(((A - D) / 4), -clip, clip);

            A -= d2;
            B += d1;
            C -= d1;
            D += d2;

            buf[-2 * stride] = XEVD_CLIP3(0, (1 << (bit_depth_minus8+8)) - 1, A);
            buf[-stride] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, B);
            buf[0] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, C);
            buf[stride] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, D);
            buf++;
        }
    }
}

void deblock_scu_hor_chroma(pel *buf, int qp, int stride, int is_luma, const u8 *tbl_qp_to_st

    , int bit_depth_minus8, int chroma_format_idc

)
{
    s16 A, B, C, D, d, d1;
    s16 abs, t16, clip, sign, st;
    int i, size;


    st = tbl_qp_to_st[qp] << bit_depth_minus8; 
   size = (is_luma ? MIN_CU_SIZE : (MIN_CU_SIZE >> (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc))));


    if(st)
    {
        for(i = 0; i < size; i++)
        {
            A = buf[-2 * stride];
            B = buf[-stride];
            C = buf[0];
            D = buf[stride];

            d = (A - (B << 2) + (C << 2) - D) / 8;

            abs = XEVD_ABS16(d);
            sign = XEVD_SIGN_GET(d);

            t16 = XEVD_MAX(0, ((abs - st) << 1));
            clip = XEVD_MAX(0, (abs - t16));
            d1 = XEVD_SIGN_SET(clip, sign);

            B += d1;
            C -= d1;

            buf[-stride] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, B);
            buf[0] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, C);


            buf++;
        }
    }
}

void deblock_scu_ver(pel *buf, int qp, int stride, int is_luma, const u8 *tbl_qp_to_st

    , int bit_depth_minus8, int chroma_format_idc

)
{
    s16 A, B, C, D, d, d1, d2;
    s16 abs, t16, clip, sign, st;
    int i, size;

    st = tbl_qp_to_st[qp] << bit_depth_minus8;

    size = (is_luma ? MIN_CU_SIZE : (MIN_CU_SIZE >> (XEVD_GET_CHROMA_H_SHIFT(chroma_format_idc))));


    if(st)
    {
        for(i = 0; i < size; i++)
        {
            A = buf[-2];
            B = buf[-1];
            C = buf[0];
            D = buf[1];

            d = (A - (B << 2) + (C << 2) - D) / 8;

            abs = XEVD_ABS16(d);
            sign = XEVD_SIGN_GET(d);

            t16 = XEVD_MAX(0, ((abs - st) << 1));
            clip = XEVD_MAX(0, (abs - t16));
            d1 = XEVD_SIGN_SET(clip, sign);
            clip >>= 1;
            d2 = XEVD_CLIP(((A - D) / 4), -clip, clip);

            A -= d2;
            B += d1;
            C -= d1;
            D += d2;

            buf[-2] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, A);
            buf[-1] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, B);
            buf[0] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, C);
            buf[1] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, D);

            buf += stride;
        }
    }
}

void deblock_scu_ver_chroma(pel *buf, int qp, int stride, int is_luma, const u8 *tbl_qp_to_st

    , int bit_depth_minus8, int chroma_format_idc

)
{
    s16 A, B, C, D, d, d1;
    s16 abs, t16, clip, sign, st;
    int i, size;

    st = tbl_qp_to_st[qp] << bit_depth_minus8;
    size = (is_luma ? MIN_CU_SIZE : (MIN_CU_SIZE >> (XEVD_GET_CHROMA_H_SHIFT(chroma_format_idc))));


    if(st)
    {
        for(i = 0; i < size; i++)
        {
            A = buf[-2];
            B = buf[-1];
            C = buf[0];
            D = buf[1];

            d = (A - (B << 2) + (C << 2) - D) / 8;

            abs = XEVD_ABS16(d);
            sign = XEVD_SIGN_GET(d);

            t16 = XEVD_MAX(0, ((abs - st) << 1));
            clip = XEVD_MAX(0, (abs - t16));
            d1 = XEVD_SIGN_SET(clip, sign);

            B += d1;
            C -= d1;

            buf[-1] = XEVD_CLIP3(0, (1 << (bit_depth_minus8+8)) - 1, B);
            buf[0] = XEVD_CLIP3(0, (1 << (bit_depth_minus8 + 8)) - 1, C);


            buf += stride;
        }
    }
}

static void deblock_cu_hor(XEVD_PIC *pic, int x_pel, int y_pel, int cuw, int cuh, u32 *map_scu, s8(*map_refi)[REFP_NUM], s16(*map_mv)[REFP_NUM][MV_D], int w_scu
    , u8* map_tidx, int boundary_filtering

    , int bit_depth_luma, int bit_depth_chroma
                           , int chroma_format_idc

)
{
    pel       * y, *u, *v;
    const u8  * tbl_qp_to_st;
    int         i, t, qp, s_l, s_c;
    int         w = cuw >> MIN_CU_LOG2;
    int         h = cuh >> MIN_CU_LOG2;
    u32       * map_scu_tmp;
    int         j;
    int         t1, t_copy; // Next row scu number

    t = (x_pel >> MIN_CU_LOG2) + (y_pel >> MIN_CU_LOG2) * w_scu;
    t_copy = t;
    t1 = (x_pel >> MIN_CU_LOG2) + ((y_pel - (1 << MIN_CU_LOG2)) >> MIN_CU_LOG2) * w_scu;
    map_scu += t;
    map_refi += t;
    map_mv += t;
    map_scu_tmp = map_scu;
    s_l = pic->s_l;
    s_c = pic->s_c;
    y = pic->y + x_pel + y_pel * s_l;

    t = (x_pel >> (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc))) + (y_pel >> (XEVD_GET_CHROMA_H_SHIFT(chroma_format_idc))) * s_c;
    u = pic->u + t;
    v = pic->v + t;

    unsigned int no_boundary = map_tidx[t_copy] == map_tidx[t1];
    if (boundary_filtering)
    {
        no_boundary = !(map_tidx[t_copy] == map_tidx[t1]);
        if ((t_copy - w_scu)>0)
            no_boundary  = no_boundary && ((MCU_GET_SN(map_scu[0])) == (MCU_GET_SN(map_scu[-w_scu])));

    }
    /* horizontal filtering */
    if(y_pel > 0 && (no_boundary))
    {
        for(i = 0; i < (cuw >> MIN_CU_LOG2); i++)
        {
            tbl_qp_to_st = get_tbl_qp_to_st(map_scu[i], map_scu[i - w_scu], map_refi[i], map_refi[i - w_scu], map_mv[i], map_mv[i - w_scu]);

            qp = MCU_GET_QP(map_scu[i]);
            t = (i << MIN_CU_LOG2);

            
            deblock_scu_hor(y + t, qp, s_l, 1, tbl_qp_to_st
                , bit_depth_luma - 8
                            , chroma_format_idc);
            

          

               if (chroma_format_idc != 0)
             
            {
                t = t >> (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc));


            int qp_u = XEVD_CLIP3(-6 * (bit_depth_chroma - 8), 57, qp + pic->pic_qp_u_offset);
            int qp_v = XEVD_CLIP3(-6 * (bit_depth_chroma - 8), 57, qp + pic->pic_qp_v_offset);
                deblock_scu_hor_chroma(u + t, xevd_qp_chroma_dynamic[0][qp_u], s_c, 0, tbl_qp_to_st, bit_depth_chroma - 8
                                       , chroma_format_idc);
                deblock_scu_hor_chroma(v + t, xevd_qp_chroma_dynamic[1][qp_v], s_c, 0, tbl_qp_to_st, bit_depth_chroma - 8
                                       , chroma_format_idc);

            }
        }
    }

    map_scu = map_scu_tmp;
    for(i = 0; i < h; i++)
    {
        for(j = 0; j < w; j++)
        {
            MCU_SET_COD(map_scu[j]);
        }
        map_scu += w_scu;
    }
}

static void deblock_cu_ver(XEVD_PIC *pic, int x_pel, int y_pel, int cuw, int cuh, u32 *map_scu, s8(*map_refi)[REFP_NUM], s16(*map_mv)[REFP_NUM][MV_D], int w_scu
                         , u32  *map_cu
                         ,u8* map_tidx, int boundary_filtering
    , int bit_depth_luma, int bit_depth_chroma
                           , int chroma_format_idc

)
{
    pel       * y, *u, *v;
    const u8  * tbl_qp_to_st;
    int         i, t, qp, s_l, s_c;
    int         w = cuw >> MIN_CU_LOG2;
    int         h = cuh >> MIN_CU_LOG2;
    int         j;
    u32       * map_scu_tmp;
    s8       (* map_refi_tmp)[REFP_NUM];
    s16      (* map_mv_tmp)[REFP_NUM][MV_D];
    int         t1, t2, t_copy; // Next row scu number

    t = (x_pel >> MIN_CU_LOG2) + (y_pel >> MIN_CU_LOG2) * w_scu;
    t_copy = t;
    t1 = ((x_pel - (1 << MIN_CU_LOG2)) >> MIN_CU_LOG2) + (y_pel >> MIN_CU_LOG2) * w_scu;
    t2 = ((x_pel + (w << MIN_CU_LOG2)) >> MIN_CU_LOG2) + (y_pel >> MIN_CU_LOG2) * w_scu;
    map_scu += t;
    map_refi += t;
    map_mv += t;

    s_l = pic->s_l;
    s_c = pic->s_c;
    y = pic->y + x_pel + y_pel * s_l;

    t = (x_pel >> (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc))) + (y_pel >> (XEVD_GET_CHROMA_H_SHIFT(chroma_format_idc))) * s_c;

    u = pic->u + t;
    v = pic->v + t;

    map_scu_tmp = map_scu;
    map_refi_tmp = map_refi;
    map_mv_tmp = map_mv;

    unsigned int  no_boundary = map_tidx[t_copy] == map_tidx[t1];
    if (boundary_filtering)
    {
        no_boundary = !(map_tidx[t_copy] == map_tidx[t1])&& ((MCU_GET_SN(map_scu[0])) == (MCU_GET_SN(map_scu[-1])));
    }
    /* vertical filtering */
    if(x_pel > 0 && MCU_GET_COD(map_scu[-1]) && (no_boundary))
    {
        for(i = 0; i < (cuh >> MIN_CU_LOG2); i++)
        {
            tbl_qp_to_st = get_tbl_qp_to_st(map_scu[0], map_scu[-1], \
                                            map_refi[0], map_refi[-1], map_mv[0], map_mv[-1]);
            qp = MCU_GET_QP(map_scu[0]);

            
            
            deblock_scu_ver(y, qp, s_l, 1, tbl_qp_to_st
                , bit_depth_luma - 8
                            , chroma_format_idc);
            

           

                if (chroma_format_idc != 0)

                
            {

                int qp_u = XEVD_CLIP3(-6 * (bit_depth_chroma - 8), 57, qp + pic->pic_qp_u_offset);
                int qp_v = XEVD_CLIP3(-6 * (bit_depth_chroma - 8), 57, qp + pic->pic_qp_v_offset);
                deblock_scu_ver_chroma(u, xevd_qp_chroma_dynamic[0][qp_u], s_c, 0, tbl_qp_to_st, bit_depth_chroma - 8
                                       , chroma_format_idc);
                deblock_scu_ver_chroma(v, xevd_qp_chroma_dynamic[1][qp_v], s_c, 0, tbl_qp_to_st, bit_depth_chroma - 8
                                       , chroma_format_idc);

            }

            y += (s_l << MIN_CU_LOG2);
            u += (s_c << (MIN_CU_LOG2 - (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc))));
            v += (s_c << (MIN_CU_LOG2 - (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc))));

            map_scu += w_scu;
            map_refi += w_scu;
            map_mv += w_scu;
        }
    }

    no_boundary = map_tidx[t_copy] == map_tidx[t2];
    if (boundary_filtering)
    {
        no_boundary = !(map_tidx[t_copy] == map_tidx[t2])&& ((MCU_GET_SN(map_scu[0])) == (MCU_GET_SN(map_scu[w])));
    }

    map_scu = map_scu_tmp;
    map_refi = map_refi_tmp;
    map_mv = map_mv_tmp;
    if(x_pel + cuw < pic->w_l && MCU_GET_COD(map_scu[w]) && (no_boundary))
    {
        y = pic->y + x_pel + y_pel * s_l;
        u = pic->u + t;
        v = pic->v + t;

        y += cuw;
        u += (cuw >> (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc)));
        v += (cuw >> (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc)));


        for(i = 0; i < (cuh >> MIN_CU_LOG2); i++)
        {
            tbl_qp_to_st = get_tbl_qp_to_st(map_scu[w], map_scu[w - 1], map_refi[w], map_refi[w - 1], map_mv[w], map_mv[w - 1]);
            qp = MCU_GET_QP(map_scu[w]);

           
            deblock_scu_ver(y, qp, s_l, 1, tbl_qp_to_st
                , bit_depth_luma - 8
                                , chroma_format_idc);
            
            

               if (chroma_format_idc != 0)
              
            {

                int qp_u = XEVD_CLIP3(-6 * (bit_depth_chroma - 8), 57, qp + pic->pic_qp_u_offset);
                int qp_v = XEVD_CLIP3(-6 * (bit_depth_chroma - 8), 57, qp + pic->pic_qp_v_offset);
                deblock_scu_ver_chroma(u, xevd_qp_chroma_dynamic[0][qp_u], s_c, 0, tbl_qp_to_st, bit_depth_chroma - 8
                                       , chroma_format_idc);
                deblock_scu_ver_chroma(v, xevd_qp_chroma_dynamic[1][qp_v], s_c, 0, tbl_qp_to_st, bit_depth_chroma - 8
                                       , chroma_format_idc);

            }

            y += (s_l << MIN_CU_LOG2);
            u += (s_c << (MIN_CU_LOG2 - (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc))));
            v += (s_c << (MIN_CU_LOG2 - (XEVD_GET_CHROMA_W_SHIFT(chroma_format_idc))));

            map_scu += w_scu;
            map_refi += w_scu;
            map_mv += w_scu;
        }
    }

    map_scu = map_scu_tmp;
    for(i = 0; i < h; i++)
    {
        for(j = 0; j < w; j++)
        {
            MCU_SET_COD(map_scu[j]);
        }
        map_scu += w_scu;
    }
}


void xevd_deblock_cu_hor(XEVD_PIC *pic, int x_pel, int y_pel, int cuw, int cuh, u32 *map_scu, s8(*map_refi)[REFP_NUM], s16(*map_mv)[REFP_NUM][MV_D], int w_scu, int log2_max_cuwh, XEVD_REFP(*refp)[REFP_NUM]
                        , u8* map_tidx, int boundary_filtering
    , int bit_depth_luma, int bit_depth_chroma
                        , int chroma_format_idc
)
{
 
    
    deblock_cu_hor(pic, x_pel, y_pel, cuw, cuh, map_scu, map_refi, map_mv, w_scu
        , map_tidx, boundary_filtering
        , bit_depth_luma, bit_depth_chroma
                    , chroma_format_idc);
    
}

void xevd_deblock_cu_ver(XEVD_PIC *pic, int x_pel, int y_pel, int cuw, int cuh, u32 *map_scu, s8(*map_refi)[REFP_NUM], s16(*map_mv)[REFP_NUM][MV_D], int w_scu, int log2_max_cuwh
    , u32  *map_cu, XEVD_REFP(*refp)[REFP_NUM]
    , u8* map_tidx, int boundary_filtering
    , int bit_depth_luma, int bit_depth_chroma
                        , int chroma_format_idc)

{
    
    
    deblock_cu_ver(pic, x_pel, y_pel, cuw, cuh, map_scu, map_refi, map_mv, w_scu
        , map_cu, map_tidx, boundary_filtering
        , bit_depth_luma, bit_depth_chroma
        , chroma_format_idc );
    
}


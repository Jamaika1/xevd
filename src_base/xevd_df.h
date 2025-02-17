/* Copyright (c) 2020, Samsung Electronics Co., Ltd.
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

#ifndef _XEVD_DF_H_
#define _XEVD_DF_H_
void deblock_scu_hor(pel *buf, int qp, int stride, int is_luma, const u8 *tbl_qp_to_st
                   , int bit_depth_minus8, int chroma_format_idc);
void deblock_scu_hor_chroma(pel *buf, int qp, int stride, int is_luma, const u8 *tbl_qp_to_st
                          , int bit_depth_minus8, int chroma_format_idc);
void deblock_scu_ver(pel *buf, int qp, int stride, int is_luma, const u8 *tbl_qp_to_st
                   , int bit_depth_minus8, int chroma_format_idc);
void deblock_scu_ver_chroma(pel *buf, int qp, int stride, int is_luma, const u8 *tbl_qp_to_st
                          , int bit_depth_minus8, int chroma_format_idc);
void xevd_deblock_cu_hor(XEVD_PIC *pic, int x_pel, int y_pel, int cuw, int cuh, u32 *map_scu, s8(*map_refi)[REFP_NUM], s16(*map_mv)[REFP_NUM][MV_D], int w_scu, int log2_max_cuwh, XEVD_REFP(*refp)[REFP_NUM]
                       , u8* map_tidx, int boundary_filtering, int bit_depth_luma, int bit_depth_chroma, int chroma_format_idc);
void xevd_deblock_cu_ver(XEVD_PIC *pic, int x_pel, int y_pel, int cuw, int cuh, u32 *map_scu, s8(*map_refi)[REFP_NUM], s16(*map_mv)[REFP_NUM][MV_D], int w_scu, int log2_max_cuwh
                       , u32  *map_cu, XEVD_REFP(*refp)[REFP_NUM], u8 *map_tidx, int boundary_filtering, int bit_depth_luma, int bit_depth_chroma, int chroma_format_idc);

#endif /* _XEVD_DF_H_ */

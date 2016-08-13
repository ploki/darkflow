/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
#include "hotpixels.h"
#include <Magick++.h>
#include "hdr.h"
#include "console.h"
using Magick::Quantum;

typedef double extended_quantum_t;

HotPixels::HotPixels(double delta, bool aggressive, bool naive, QObject *parent) :
    Algorithm(false, parent),
    m_delta(delta),
    m_aggressive(aggressive),
    m_naive(naive)
{
}

#define color_op_naive(q) \
    sum_rgb[q]+=nrgb[q];

#define color_op(q) \
    if ( nrgb[q] > max_rgb[q] ) max_rgb[q] = nrgb[q]; \
    if ( nrgb[q] < min_rgb[q] ) min_rgb[q] = nrgb[q]; \
    sum_rgb[q]+=nrgb[q];


#define loop_code_naive(j,i) \
    nrgb[0]=input_pixels[j+1][x+i].red; \
    nrgb[1]=input_pixels[j+1][x+i].green; \
    nrgb[2]=input_pixels[j+1][x+i].blue; \
    color_op_naive(0); color_op_naive(1); color_op_naive(2)

#define loop_code(j,i) \
    nrgb[0]=input_pixels[j+1][x+i].red; \
    nrgb[1]=input_pixels[j+1][x+i].green; \
    nrgb[2]=input_pixels[j+1][x+i].blue; \
    color_op(0); color_op(1); color_op(2)


#define color_op2(q) \
    other_channels = (rgb[(q+2)%3]+rgb[(q+1)%3])/2; \
    sum_rgb[q]-=(max_rgb[q]+min_rgb[q]); \
    sum_rgb[q]/=6; \
    if ( sum_rgb[q]*m_delta < rgb[q] && other_channels*m_delta < rgb[q] ) rgb[q]=sum_rgb[q]; \
    if ( sum_rgb[q]/m_delta > rgb[q] && other_channels/m_delta > rgb[q] ) rgb[q]=sum_rgb[q];

#define color_op2_naive(q) \
    other_channels = (rgb[(q+2)%3]+rgb[(q+1)%3])/2; \
    sum_rgb[q]/=8; \
    if ( sum_rgb[q]*m_delta < rgb[q] && other_channels*m_delta < rgb[q] ) rgb[q]=sum_rgb[q]; \
    if ( sum_rgb[q]/m_delta > rgb[q] && other_channels/m_delta > rgb[q] ) rgb[q]=sum_rgb[q];


#define color_op2_aggressive(q) \
    sum_rgb[q]-=(max_rgb[q]+min_rgb[q]); \
    sum_rgb[q]/=6; \
    if ( sum_rgb[q]*m_delta < rgb[q] ) rgb[q]=sum_rgb[q]; \
    if ( sum_rgb[q]/m_delta > rgb[q] ) rgb[q]=sum_rgb[q];

#define color_op2_naive_aggressive(q) \
    sum_rgb[q]/=8; \
    if ( sum_rgb[q]*m_delta < rgb[q] ) rgb[q]=sum_rgb[q]; \
    if ( sum_rgb[q]/m_delta > rgb[q] ) rgb[q]=sum_rgb[q];

void HotPixels::applyOnImage(Magick::Image &image, bool hdr)
{
    Magick::Image input(image);
    ResetImage(image);

    std::shared_ptr<Magick::Pixels>
            output_cache(new Magick::Pixels(image)),
            input_cache(new Magick::Pixels(input));

    int w = image.columns();
    int h = image.rows();
    dfl_block bool error=false;
    dfl_parallel_for(y, 1, h-1, 4, (input, image), {
        Magick::PixelPacket *output_pixels = output_cache->get(0,y,w,1);
        const Magick::PixelPacket *input_pixels[3];
        input_pixels[0] = input_cache->getConst(0,y-1,w,1);
        input_pixels[1] = input_cache->getConst(0,y,w,1);
        input_pixels[2] = input_cache->getConst(0,y+1,w,1);
        if ( error || !output_pixels || !input_pixels[0] || !input_pixels[1] || !input_pixels[2] ) {
            if (!error)
                dflError(DF_NULL_PIXELS);
            error=true;
            continue;
        }
        for ( int x = 1 ; x < w-1 ; ++x ) {
            extended_quantum_t max_rgb[3]={0,0,0};
            extended_quantum_t min_rgb[3]={QuantumRange,QuantumRange,QuantumRange};
            extended_quantum_t sum_rgb[3]={0,0,0};
            extended_quantum_t rgb[3];
            extended_quantum_t nrgb[3];
            extended_quantum_t other_channels=0;

            if (hdr) {
                rgb[0]=DF_ROUND(fromHDR(input_pixels[1][x].red));
                rgb[1]=DF_ROUND(fromHDR(input_pixels[1][x].green));
                rgb[2]=DF_ROUND(fromHDR(input_pixels[1][x].blue));
            }
            else {
                rgb[0]=input_pixels[1][x].red;
                rgb[1]=input_pixels[1][x].green;
                rgb[2]=input_pixels[1][x].blue;
            }


            if ( m_naive ) {
                loop_code_naive(-1,-1); loop_code_naive(-1, 0); loop_code_naive(-1, 1);
                loop_code_naive( 0,-1);                         loop_code_naive( 0, 1);
                loop_code_naive( 1,-1); loop_code_naive( 1, 0); loop_code_naive( 1, 1);

                if ( m_aggressive ) {
                    color_op2_naive_aggressive(0); color_op2_naive_aggressive(1); color_op2_naive_aggressive(2);
                }
                else {
                    color_op2_naive(0); color_op2_naive(1); color_op2_naive(2);
                }
            }
            else {
                loop_code(-1,-1); loop_code(-1, 0); loop_code(-1, 1);
                loop_code( 0,-1);                   loop_code( 0, 1);
                loop_code( 1,-1); loop_code( 1, 0); loop_code( 1, 1);

                if ( m_aggressive ) {
                    color_op2_aggressive(0); color_op2_aggressive(1); color_op2_aggressive(2);
                }
                else {
                    color_op2(0); color_op2(1); color_op2(2);
                }
            }

            if (hdr) {
                output_pixels[x].red=toHDR(rgb[0]>QuantumRange?QuantumRange:rgb[0]);
                output_pixels[x].green=toHDR(rgb[1]>QuantumRange?QuantumRange:rgb[1]);
                output_pixels[x].blue=toHDR(rgb[2]>QuantumRange?QuantumRange:rgb[2]);
            }
            else {
                output_pixels[x].red=rgb[0]>QuantumRange?QuantumRange:rgb[0];
                output_pixels[x].green=rgb[1]>QuantumRange?QuantumRange:rgb[1];
                output_pixels[x].blue=rgb[2]>QuantumRange?QuantumRange:rgb[2];
            }
        }
        output_cache->sync();
    });
}

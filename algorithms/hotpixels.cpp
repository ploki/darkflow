#include "hotpixels.h"
#include <Magick++.h>

using Magick::Quantum;

typedef int extended_quantum_t;

HotPixels::HotPixels(double delta, bool aggressive, bool naive, QObject *parent) :
    Algorithm(parent),
    m_delta(delta),
    m_aggressive(aggressive),
    m_naive(naive)
{
}

void HotPixels::applyOnImage(Magick::Image &image)
{
    Magick::Image input(image);
    image.modifyImage();

    Magick::Pixels
            output_cache(image),
            input_cache(input);

    int w = image.columns();
    int h = image.rows();

#pragma omp parallel for
    for ( int y = 1 ; y < h-1 ; ++y ) {
        Magick::PixelPacket *output_pixels = output_cache.get(0,y,w,1);
        const Magick::PixelPacket *input_pixels[3];
        input_pixels[0] = input_cache.get(0,y-1,w,1);
        input_pixels[1] = input_cache.get(0,y,w,1);
        input_pixels[2] = input_cache.get(0,y+1,w,1);

        for ( int x = 1 ; x < w-1 ; ++x ) {
            extended_quantum_t max_rgb[3]={0,0,0};
            extended_quantum_t min_rgb[3]={QuantumRange,QuantumRange,QuantumRange};
            extended_quantum_t sum_rgb[3]={0,0,0};
            extended_quantum_t rgb[3];
            extended_quantum_t nrgb[3];
            extended_quantum_t other_channels=0;

            rgb[0]=input_pixels[1][x].red;
            rgb[1]=input_pixels[1][x].green;
            rgb[2]=input_pixels[1][x].blue;

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



#undef loop_code
#undef loop_code_naive
#undef color_op
#undef color_op_naive
#undef color_op2
#undef color_op2_naive
#undef color_op2_aggressive
#undef color_op2_naive_aggressive
            //end of loops unroll


            output_pixels[x].red=rgb[0]>QuantumRange?QuantumRange:rgb[0];
            output_pixels[x].green=rgb[1]>QuantumRange?QuantumRange:rgb[1];
            output_pixels[x].blue=rgb[2]>QuantumRange?QuantumRange:rgb[2];

        }
    }
#pragma omp barrier
    output_cache.sync();
}

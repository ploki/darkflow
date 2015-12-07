/*
 * 1394-Based Digital Camera Control Library
 *
 * Color conversion functions, including Bayer pattern decoding
 *
 * Written by Damien Douxchamps and Frederic Devernay
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __DC1394_CONVERSIONS_H__
#define __DC1394_CONVERSIONS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! \file dc1394/conversions.h
    \brief functions to convert video formats
    \author Damien Douxchamps: coding
    \author Frederic Devernay: coding
    \author Peter Antoniac: documentation maintainer

    More details soon
*/

#define restrict __restrict

/**
 * A list of de-mosaicing techniques for Bayer-patterns.
 *
 * The speed of the techniques can vary greatly, as well as their quality.
 */
typedef enum {
    DC1394_BAYER_METHOD_NEAREST=0,
    DC1394_BAYER_METHOD_SIMPLE,
    DC1394_BAYER_METHOD_BILINEAR,
    DC1394_BAYER_METHOD_HQLINEAR,
    DC1394_BAYER_METHOD_DOWNSAMPLE,
    DC1394_BAYER_METHOD_EDGESENSE,
    DC1394_BAYER_METHOD_VNG,
    DC1394_BAYER_METHOD_AHD
} dc1394bayer_method_t;
#define DC1394_BAYER_METHOD_MIN      DC1394_BAYER_METHOD_NEAREST
#define DC1394_BAYER_METHOD_MAX      DC1394_BAYER_METHOD_AHD
#define DC1394_BAYER_METHOD_NUM     (DC1394_BAYER_METHOD_MAX-DC1394_BAYER_METHOD_MIN+1)

/**
 * RAW sensor filters. These elementary tiles tesselate the image plane in RAW modes. RGGB should be interpreted in 2D
as
 *
 *    RG
 *    GB
 *
 * and similarly for other filters.
 */
typedef enum {
    DC1394_COLOR_FILTER_RGGB = 512,
    DC1394_COLOR_FILTER_GBRG,
    DC1394_COLOR_FILTER_GRBG,
    DC1394_COLOR_FILTER_BGGR
} dc1394color_filter_t;
#define DC1394_COLOR_FILTER_MIN        DC1394_COLOR_FILTER_RGGB
#define DC1394_COLOR_FILTER_MAX        DC1394_COLOR_FILTER_BGGR
#define DC1394_COLOR_FILTER_NUM       (DC1394_COLOR_FILTER_MAX - DC1394_COLOR_FILTER_MIN + 1)

/**
 * Error codes returned by most libdc1394 functions.
 *
 * General rule: 0 is success, negative denotes a problem.
 */
typedef enum {
    DC1394_SUCCESS                     =  0,
    DC1394_FAILURE                     = -1,
    DC1394_NOT_A_CAMERA                = -2,
    DC1394_FUNCTION_NOT_SUPPORTED      = -3,
    DC1394_CAMERA_NOT_INITIALIZED      = -4,
    DC1394_MEMORY_ALLOCATION_FAILURE   = -5,
    DC1394_TAGGED_REGISTER_NOT_FOUND   = -6,
    DC1394_NO_ISO_CHANNEL              = -7,
    DC1394_NO_BANDWIDTH                = -8,
    DC1394_IOCTL_FAILURE               = -9,
    DC1394_CAPTURE_IS_NOT_SET          = -10,
    DC1394_CAPTURE_IS_RUNNING          = -11,
    DC1394_RAW1394_FAILURE             = -12,
    DC1394_FORMAT7_ERROR_FLAG_1        = -13,
    DC1394_FORMAT7_ERROR_FLAG_2        = -14,
    DC1394_INVALID_ARGUMENT_VALUE      = -15,
    DC1394_REQ_VALUE_OUTSIDE_RANGE     = -16,
    DC1394_INVALID_FEATURE             = -17,
    DC1394_INVALID_VIDEO_FORMAT        = -18,
    DC1394_INVALID_VIDEO_MODE          = -19,
    DC1394_INVALID_FRAMERATE           = -20,
    DC1394_INVALID_TRIGGER_MODE        = -21,
    DC1394_INVALID_TRIGGER_SOURCE      = -22,
    DC1394_INVALID_ISO_SPEED           = -23,
    DC1394_INVALID_IIDC_VERSION        = -24,
    DC1394_INVALID_COLOR_CODING        = -25,
    DC1394_INVALID_COLOR_FILTER        = -26,
    DC1394_INVALID_CAPTURE_POLICY      = -27,
    DC1394_INVALID_ERROR_CODE          = -28,
    DC1394_INVALID_BAYER_METHOD        = -29,
    DC1394_INVALID_VIDEO1394_DEVICE    = -30,
    DC1394_INVALID_OPERATION_MODE      = -31,
    DC1394_INVALID_TRIGGER_POLARITY    = -32,
    DC1394_INVALID_FEATURE_MODE        = -33,
    DC1394_INVALID_LOG_TYPE            = -34,
    DC1394_INVALID_BYTE_ORDER          = -35,
    DC1394_INVALID_STEREO_METHOD       = -36,
    DC1394_BASLER_NO_MORE_SFF_CHUNKS   = -37,
    DC1394_BASLER_CORRUPTED_SFF_CHUNK  = -38,
    DC1394_BASLER_UNKNOWN_SFF_CHUNK    = -39
} dc1394error_t;
#define DC1394_ERROR_MIN  DC1394_BASLER_UNKNOWN_SFF_CHUNK
#define DC1394_ERROR_MAX  DC1394_SUCCESS
#define DC1394_ERROR_NUM (DC1394_ERROR_MAX-DC1394_ERROR_MIN+1)

/**
 * Yet another boolean data type
 */
typedef enum {
    DC1394_FALSE= 0,
    DC1394_TRUE
} dc1394bool_t;

/**********************************************************************
 *  CONVERSION FUNCTIONS FOR STEREO IMAGES
 **********************************************************************/

/**
 * changes a 16bit stereo image (8bit/channel) into two 8bit images on top of each other
 */
dc1394error_t
dc1394_deinterlace_stereo(uint8_t *src, uint8_t *dest, uint32_t width, uint32_t height);

/************************************************************************************************
 *                                                                                              *
 *      Color conversion functions for cameras that can output raw Bayer pattern images (color  *
 *  codings DC1394_COLOR_CODING_RAW8 and DC1394_COLOR_CODING_RAW16).                            *
 *                                                                                              *
 *  Credits and sources:                                                                        *
 *  - Nearest Neighbor : OpenCV library                                                         *
 *  - Bilinear         : OpenCV library                                                         *
 *  - HQLinear         : High-Quality Linear Interpolation For Demosaicing Of Bayer-Patterned   *
 *                       Color Images, by Henrique S. Malvar, Li-wei He, and Ross Cutler,       *
 *                          in Proceedings of the ICASSP'04 Conference.                            *
 *  - Edge Sense II    : Laroche, Claude A. "Apparatus and method for adaptively interpolating  *
 *                       a full color image utilizing chrominance gradients"                    *
 *                          U.S. Patent 5,373,322. Based on the code found on the website          *
 *                       http://www-ise.stanford.edu/~tingchen/ Converted to C and adapted to   *
 *                       all four elementary patterns.                                          *
 *  - Downsample       : "Known to the Ancients"                                                *
 *  - Simple           : Implemented from the information found in the manual of Allied Vision  *
 *                       Technologies (AVT) cameras.                                            *
 *  - VNG              : Variable Number of Gradients, a method described in                    *
 *                       http://www-ise.stanford.edu/~tingchen/algodep/vargra.html              *
 *                       Sources import from DCRAW by Frederic Devernay. DCRAW is a RAW         *
 *                       converter program by Dave Coffin. URL:                                 *
 *                       http://www.cybercom.net/~dcoffin/dcraw/                                *
 *  - AHD              : Adaptive Homogeneity-Directed Demosaicing Algorithm, by K. Hirakawa    *
 *                       and T.W. Parks, IEEE Transactions on Image Processing, Vol. 14, Nr. 3, *
 *                       March 2005, pp. 360 - 369.                                             *
 *                                                                                              *
 ************************************************************************************************/

/**
 * Perform de-mosaicing on an 16-bit image buffer
 */
dc1394error_t
dc1394_bayer_decoding_16bit(const uint16_t *bayer, uint16_t *rgb,
                            uint32_t width, uint32_t height, dc1394color_filter_t tile,
                            dc1394bayer_method_t method, uint32_t bits);


#ifdef __cplusplus
}
#endif

#endif /* _DC1394_CONVERSIONS_H */

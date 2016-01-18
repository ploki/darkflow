#include "workerdebayer.h"
#include "bayer.h"
#include "console.h"

WorkerDebayer::WorkerDebayer(OpDebayer::Debayer quality, QThread *thread, Operator *op) :
    OperatorWorker(thread, op),
    m_quality(quality)
{
}

/*
 * inspiration dcraw
 */

static u_int32_t
getFilterPattern(const Photo &photo)
{
    QString str = photo.getTag(TAG_FILTER_PATTERN);
    u_int32_t filters = 0;
    if ( str == "BGGRBGGRBGGRBGGR" || str == "BG/GR" )
        filters=0x16161616;
    else if ( str == "GRBGGRBGGRBGGRBG" || str == "GR/BG" )
        filters=0x61616161;
    else if ( str == "GBRGGBRGGBRGGBRG" || str == "GB/RG" )
        filters=0x49494949;
    else if ( str == "RGGBRGGBRGGBRGGB" || str == "RG/GB")
        filters=0x94949494;
    return filters;
}

#define FC(filters, row, col) \
    (filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)

#define BAYER(filters, row,col) \
    image[((row) >> shrink)*iwidth + ((col) >> shrink)][FC(filters, row,col)]

void bayerToPixel(const Magick::PixelPacket& src,
                  quantum_t *dst,
                  u_int32_t filters,
                  int row, int col,
                  int *count)
{
    int c = FC(filters, row, col);
    switch(c) {
    case 0:
        dst[0] += src.red;
        ++count[0];
        break;
    case 3:
    case 1:
        dst[1] += src.green;
        ++count[1];
        break;
    case 2:
        dst[2] += src.blue;
        ++count[2];
        break;
    default:
        dflError("bayerToPixel: color index out of range");
    }
}
static Photo debayerMask(const Photo &photo, u_int32_t filters)
{
    Photo newPhoto(photo);
    Magick::Image srcImage(newPhoto.image());
    Magick::Image& dst = newPhoto.image();
    ResetImage(dst);

    int w = srcImage.columns();
    int h = srcImage.rows();
    Magick::Pixels src_cache(srcImage);
    Magick::Pixels dst_cache(dst);
    bool error=false;
#pragma omp parallel for dfl_threads(4, srcImage, dst)
    for (int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixel = dst_cache.get(0, y, w, 1);
        const Magick::PixelPacket *src = src_cache.getConst(0, y, w, 1);
        if ( error || !pixel || !src ) {
            if (!error)
                dflError(DF_NULL_PIXELS);
            error=true;
            continue;
        }
        for (int x = 0 ; x < w ; ++x ) {
            int c = FC(filters, y, x);
            switch (c) {
            case 0:
                pixel[x].red = src[x].red; pixel[x].green = 0 ; pixel[x].blue = 0; break;
            case 1:
            case 3:
                pixel[x].red = 0 ; pixel[x].green = src[x].green ; pixel[x].blue = 0; break;
            case 2:
                pixel[x].red = 0; pixel[x].green = 0; pixel[x].blue = src[x].blue; break;
            }
        }
        dst_cache.sync();
    }
    return newPhoto;
}

static Photo debayerHalfSize(const Photo &photo, u_int32_t filters)
{
    Photo newPhoto(photo);
    Magick::Image& image = const_cast<Magick::Image&>(photo.image());
    Magick::Image& dst   = newPhoto.image();
    dst = Magick::Image(Magick::Geometry(image.columns()/2,image.rows()/2),Magick::Color(0,0,0));

    int w = dst.columns();
    int h = dst.rows();

    ResetImage(dst);
    Magick::Pixels image_cache(image);
    Magick::Pixels dst_cache(dst);
    const Magick::PixelPacket *image_pixels = image_cache.getConst(0, 0, w*2, h*2);
    Magick::PixelPacket *pixels = dst_cache.get(0, 0, w, h);
    bool failure = false;
#pragma omp parallel for dfl_threads(4, image, dst)
    for ( int y = 0 ; y < h ; ++y ) {
        for ( int x = 0 ; x < w ; ++x ) {
            if ( failure ) continue;
            int count[3] = {};
            quantum_t rgb[3] = {};
            bayerToPixel(image_pixels[(y*2+0)*w*2+(x*2+0)], rgb, filters, y*2+0, x*2+0, count);
            bayerToPixel(image_pixels[(y*2+0)*w*2+(x*2+1)], rgb, filters, y*2+0, x*2+1, count);
            bayerToPixel(image_pixels[(y*2+1)*w*2+(x*2+0)], rgb, filters, y*2+1, x*2+0, count);
            bayerToPixel(image_pixels[(y*2+1)*w*2+(x*2+1)], rgb, filters, y*2+1, x*2+1, count);
            for ( int i = 0 ; i < 3 ; ++i ) {
                if ( count[i] == 0 ) {
                    failure = true;
                }
                else {
                    rgb[i] /= count[i];
                }
            }
            pixels[y*w+x].red=rgb[0];
            pixels[y*w+x].green=rgb[1];
            pixels[y*w+x].blue=rgb[2];
        }
    }
    dst_cache.sync();
    if ( failure ) {
        dflError("Debayer(Worker): missing color component");
        Photo dummy;
        dummy.setUndefined();
        return dummy;
    }
    return newPhoto;
}

// inspiration dc1394

static dc1394color_filter_t
get_color_filter(const Photo& photo)
{
    QString str = photo.getTag(TAG_FILTER_PATTERN);
    if ( str == "RGGBRGGBRGGBRGGB" || str == "RG/GB" )
        return DC1394_COLOR_FILTER_RGGB;
    else if ( str == "GBRGGBRGGBRGGBRG" || str == "GB/RG" )
        return DC1394_COLOR_FILTER_GBRG;
    else if ( str == "GRBGGRBGGRBGGRBG" || str == "GR/BG" )
        return DC1394_COLOR_FILTER_GRBG;
    else if ( str == "BGGRBGGRBGGRBGGR" || str == "BG/GR")
        return DC1394_COLOR_FILTER_BGGR;
    else {
        return DARKFLOW_COLOR_FILTER_UNKNOWN;
    }
}

static uint16_t *
newBayer(int w, int h)
{
    return new uint16_t[w*h];
}

static uint16_t *
newRGB(int w, int h)
{
    return new uint16_t[w*h*3];
}

static void deleteBuffer(uint16_t *buffer)
{
    delete[] buffer;
}

static uint16_t *
imageToBayer(Magick::Image& image, u_int32_t filters)
{
    int w = image.columns();
    int h = image.rows();
    uint16_t *buffer = newBayer(w, h);
    Magick::Pixels cache(image);
    const Magick::PixelPacket *pixel = cache.getConst(0, 0, w, h);
    for ( int y = 0 ; y < h ; ++y ) {
        for (int x = 0 ; x < w ; ++x ) {
            switch (FC(filters, y, x)) {
            case 0:
                buffer[y*w+x]=pixel[y*w+x].red; break;
            case 1:
            case 3:
                buffer[y*w+x]=pixel[y*w+x].green; break;
            case 2:
                buffer[y*w+x]=pixel[y*w+x].blue; break;
            }
        }
    }
    return buffer;
}

static Magick::Image
bufferToImage(uint16_t *buffer, int w, int h) {
    Magick::Image image(Magick::Geometry(w,h), Magick::Color(0,0,0));
    ResetImage(image);
    int s = w * h;
    Magick::Pixels cache(image);
    Magick::PixelPacket *pixel = cache.get(0, 0, w, h);
    for(int i = 0 ; i < s ; ++i) {
        pixel[i].red = buffer[i*3+0];
        pixel[i].green = buffer[i*3+1];
        pixel[i].blue = buffer[i*3+2];
    }
    cache.sync();
    return image;
}


Photo WorkerDebayer::process(const Photo &photo, int /*p*/, int /*c*/)
{
    u_int32_t filters = getFilterPattern(photo);
    dc1394color_filter_t dc_filters = get_color_filter(photo);
    dc1394bayer_method_t method = DC1394_BAYER_METHOD_SIMPLE;
    bool use_dc = true;

    if ( dc_filters == DARKFLOW_COLOR_FILTER_UNKNOWN) {
        setError(photo, "Unknown color filter pattern");
        return photo;
    }

    Photo newPhoto;
    switch (m_quality) {
    case OpDebayer::NoDebayer:
    default:
        use_dc = false;
        newPhoto = photo;
        break;
    case OpDebayer::Mask:
        use_dc = false;
        newPhoto = debayerMask(photo, filters);
        break;
    case OpDebayer::HalfSize:
        use_dc = false;
        newPhoto = debayerHalfSize(photo, filters);
        if ( newPhoto.isUndefined() ) {
            setError(photo, "Half Size debayer failed");

        }
        break;
    case OpDebayer::Simple:
        method = DC1394_BAYER_METHOD_SIMPLE;
        break;
    case OpDebayer::Bilinear:
        method = DC1394_BAYER_METHOD_BILINEAR;
        break;
    case OpDebayer::HQLinear:
        method = DC1394_BAYER_METHOD_HQLINEAR;
        break;
        /*
    case OpDebayer::DownSample:
        method = DC1394_BAYER_METHOD_DOWNSAMPLE;
        break;
        */
    case OpDebayer::VNG:
        method = DC1394_BAYER_METHOD_VNG;
        break;
    case OpDebayer::AHD:
        method = DC1394_BAYER_METHOD_AHD;
        break;
    }
    if (use_dc) {
        newPhoto = photo;
        Magick::Image &image = newPhoto.image();
        int w = image.columns();
        int h = image.rows();
        uint16_t *bayer = imageToBayer(image, filters);
        uint16_t *rgb = newRGB(w,h);
        dc1394error_t err;
        err = dc1394_bayer_decoding_16bit(bayer, rgb, w, h, dc_filters, method, 16);
        if ( err == DC1394_SUCCESS ) {
            /*
            if ( m_quality == OpDebayer::DownSample ) {
                w/=2;
                h/=2;
            }
            */
            image = bufferToImage(rgb, w, h);
        }
        deleteBuffer(rgb);
        deleteBuffer(bayer);
    }
    newPhoto.removeTag(TAG_FILTER_PATTERN);
    return newPhoto;
}

bool WorkerDebayer::play_onInput(int idx)
{
    return play_onInputParallel(idx);
}

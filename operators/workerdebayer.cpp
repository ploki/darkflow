#include "workerdebayer.h"
#include "bayer.h"




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
    QString str = photo.getTag("Filter pattern");
    u_int32_t filters = 0;
    if ( str == "BGGRBGGRBGGRBGGR" )
        filters=0x16161616;
    else if ( str == "GRBGGRBGGRBGGRBG" )
        filters=0x61616161;
    else if ( str == "GBRGGBRGGBRGGBRG" )
        filters=0x49494949;
    else if ( str == "RGGBRGGBRGGBRGGB")
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
        qWarning("color index out of range");
    }
}

static Photo debayerHalfSize(const Photo &photo, u_int32_t filters)
{
    Photo newPhoto(photo);
    Magick::Image& image = const_cast<Magick::Image&>(photo.image());
    Magick::Image& dst   = newPhoto.image();
    dst = Magick::Image(Magick::Geometry(image.columns()/2,image.rows()/2),Magick::Color(0,0,0));

    int w = dst.columns();
    int h = dst.rows();

    dst.modifyImage();
    Magick::Pixels image_cache(image);
    Magick::Pixels dst_cache(dst);
    const Magick::PixelPacket *image_pixels = image_cache.getConst(0, 0, w*2, h*2);
    Magick::PixelPacket *pixels = dst_cache.get(0, 0, w, h);
#pragma omp parallel for
    for ( int y = 0 ; y < h ; ++y ) {
        for ( int x = 0 ; x < w ; ++x ) {
            int count[3] = {};
            quantum_t rgb[3] = {};
            bayerToPixel(image_pixels[(y*2+0)*w*2+(x*2+0)], rgb, filters, y*2+0, x*2+0, count);
            bayerToPixel(image_pixels[(y*2+0)*w*2+(x*2+1)], rgb, filters, y*2+0, x*2+1, count);
            bayerToPixel(image_pixels[(y*2+1)*w*2+(x*2+0)], rgb, filters, y*2+1, x*2+0, count);
            bayerToPixel(image_pixels[(y*2+1)*w*2+(x*2+1)], rgb, filters, y*2+1, x*2+1, count);
            Q_ASSERT(count[0]==1);
            Q_ASSERT(count[1]==2);
            Q_ASSERT(count[2]==1);
            for ( int i = 0 ; i < 3 ; ++i ) {
                rgb[i] /= count[i];
            }
            pixels[y*w+x].red=rgb[0];
            pixels[y*w+x].green=rgb[1];
            pixels[y*w+x].blue=rgb[2];
        }
    }
    dst_cache.sync();
    return newPhoto;
}

void
borderInterpolate(Photo& photo, u_int32_t filters, unsigned border)
{
    unsigned row, col, y, x, f, c, sum[8];
    unsigned height = photo.image().rows();
    unsigned width = photo.image().columns();
    Magick::Image& image = photo.image();
    image.modifyImage();
    Magick::Pixels cache(image);
    Magick::PixelPacket *pixel = cache.get(0, 0, width, height);
    for (row=0; row < height; row++)
        for (col=0; col < width; col++) {
            if (col==border && row >= border && row < height-border)
                col = width-border;
            memset (sum, 0, sizeof sum);
            for (y=row-1; y != row+2; y++)
                for (x=col-1; x != col+2; x++)
                    if (y < height && x < width) {
                        f = FC(filters, y,x);
                        switch (f) {
                        case 0:
                            sum[f] += pixel[y*width+x].red; break;
                        case 1:
                        case 3:
                            sum[f] += pixel[y*width+x].green; break;
                        case 2:
                            sum[f] += pixel[y*width+x].blue; break;
                        }
                        sum[f+4]++;
                    }
            f = FC(filters, row,col);
            for ( c = 0 ; c < 3 ; ++c ) {
                if (c != f && sum[c+4]) {
                    switch (c) {
                    case 0:
                        pixel[row*width+col].red = sum[c] / sum[c+4]; break;
                    case 1:
                    case 3:
                        pixel[row*width+col].green = sum[c] / sum[c+4]; break;
                    case 2:
                        pixel[row*width+col].blue = sum[c] / sum[c+4]; break;
                    default:
                        Q_ASSERT(!"No!");
                    }
                }
            }
        }
    cache.sync();
}

static void
linInterpolate(Photo& photo, u_int32_t filters)
{
    int code[16][16][32], size=16, *ip, sum[4];
    int f, c, i, x, y, row, col, shift, color;
    Magick::Image& image = photo.image();
    int width = image.columns();
    int height = image.rows();
    image.modifyImage();
    Magick::Pixels cache(image);
    Magick::PixelPacket *pixel = cache.get(0, 0, width, height);

    for (row=0; row < size; row++)
        for (col=0; col < size; col++) {
            ip = code[row][col]+1;
            f = FC(filters, row,col);
            memset (sum, 0, sizeof sum);
            for (y=-1; y <= 1; y++)
                for (x=-1; x <= 1; x++) {
                    shift = (y==0) + (x==0);
                    color = FC(filters,row+y,col+x);
                    if (color == f) continue;
                    *ip++ = (width*y + x)*4 + color;
                    *ip++ = shift;
                    *ip++ = color;
                    sum[color] += 1 << shift;
                }
            code[row][col][0] = (ip - code[row][col]) / 3;
            for ( c = 0 ; c < 3 ; ++c )
                if (c != f) {
                    *ip++ = c;
                    *ip++ = 256 / sum[c];
                }
        }
    for (row=1; row < height-1; row++)
        for (col=1; col < width-1; col++) {
            ip = code[row % size][col % size];
            memset (sum, 0, sizeof sum);
            for (i=*ip++; i--; ip+=3) {
                switch (((ip[0]%4)+4)%4) {
                case 0:
                    sum[ip[2]] += pixel[row*width+col+ip[0]/4].red << ip[1]; break;
                case 1:
                case 3:
                    sum[ip[2]] += pixel[row*width+col+ip[0]/4].green << ip[1]; break;
                case 2:
                    sum[ip[2]] += pixel[row*width+col+ip[0]/4].blue << ip[1]; break;
                default:
                    Q_ASSERT(0);
                }
            }
            for (i=3; --i; ip+=2) {
                switch  (((ip[0]%4)+4)%4) {
                case 0:
                    pixel[row*width+col+ip[0]/4].red = sum[ip[0]] * ip[1] >> 8; break;
                case 1:
                case 3:
                    pixel[row*width+col+ip[0]/4].green = sum[ip[0]] * ip[1] >> 8; break;
                case 2:
                    pixel[row*width+col+ip[0]/4].blue = sum[ip[0]] * ip[1] >> 8; break;
                default:
                    Q_ASSERT(0);
                }
            }
        }
    cache.sync();
}

// inspiration dc1394

static dc1394color_filter_t
get_color_filter(const Photo& photo)
{
    QString str = photo.getTag("Filter pattern");
    str = str.left(4);
    if ( str == "RGGB ")
        return DC1394_COLOR_FILTER_RGGB;
    else if ( str == "GBRG" )
        return DC1394_COLOR_FILTER_GBRG;
    else if ( str == "GRBG" )
        return DC1394_COLOR_FILTER_GRBG;
    else if ( str == "BGGR" )
        return DC1394_COLOR_FILTER_BGGR;
    else {
        qWarning("Unknown color filter pattern");
        return DC1394_COLOR_FILTER_RGGB;
    }
}

static uint16_t *
imageToBuffer(Photo &photo)
{
    Magick::Image& image = photo.image();
    int w = image.columns();
    int h = image.rows();

}



Photo WorkerDebayer::process(const Photo &photo, int /*p*/, int /*c*/)
{
    u_int32_t filters = getFilterPattern(photo);
    Photo newPhoto;
    switch (m_quality) {
    case OpDebayer::NoDebayer:
        break;
    case OpDebayer::HalfSize:
        newPhoto = debayerHalfSize(photo, filters);
        break;
    case OpDebayer::Low:
        newPhoto = photo;
        //preInterpolate(newPhoto, filters);
        borderInterpolate(newPhoto, filters, 0);
        linInterpolate(newPhoto, filters);
        break;
    case OpDebayer::VNG:
    case OpDebayer::PPG:
    case OpDebayer::AHD:
            default:
        newPhoto = photo;
    }
    newPhoto.removeTag("Filter pattern");
    return newPhoto;
}

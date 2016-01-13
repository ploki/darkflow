#include <Magick++.h>
#include "cielab.h"
#include "photo.h"

/*
//convertion rgv <-> lch depuis ufraw et dcraw
static const double xyz_rgb[3][3] = {                  //XYZ from RGB
  { 0.412453, 0.357580, 0.180423 },
  { 0.212671, 0.715160, 0.072169 },
  { 0.019334, 0.119193, 0.950227 } };
static const double rgb_xyz[3][3] = {                   //RGB from XYZ
    { 3.24048, -1.53715, -0.498536 },
    { -0.969255, 1.87599, 0.0415559 },
    { 0.0556466, -0.204041, 1.05731 } };
*/

//sRGB D65 http://www.brucelindbloom.com/
const double xyz_rgb[3][3] = {                  //XYZ from RGB
  { 0.4124564, 0.3575761, 0.1804375 },
  { 0.2126729, 0.7151522, 0.0721750 },
  { 0.0193339, 0.1191920, 0.9503041 } };
const double rgb_xyz[3][3] = {                   //RGB from XYZ
    {  3.2404542, -1.5371385, -0.4985314 },
    { -0.9692660,  1.8760108,  0.0415560 },
    {  0.0556434, -0.2040259,  1.0572252 } };

/*
//sRGB D50 http://www.brucelindbloom.com/
const double xyz_rgb[3][3] = {
  { 0.4360747, 0.3850649, 0.1430804 },
  { 0.2225045, 0.7168786, 0.0606169 },
  { 0.0139322, 0.0971045, 0.7141733 } };
const double rgb_xyz[3][3] = {
  {  3.1338561, -1.6168667, -0.4906146 },
  { -0.9787684,  1.9161415,  0.0334540 },
  {  0.0719453, -0.2289914,  1.4052427 } };
*/


// Illuminant D50
//static const double illuminant[3] = { 0.96422, 1.00000,	0.82521 };

// Illuminent D65
const double illuminant[3] = { 0.95047, 1.00000, 1.08883 };


void Labize(Magick::Image &image, Magick::Image &labImage) {

    int h = image.rows(),
            w = image.columns();
    image.modifyImage();
    labImage.modifyImage();
    Magick::Pixels pixel_cache(image);
    Magick::Pixels lab_cache(labImage);

#pragma omp parallel for
    for (int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        Magick::PixelPacket *labpixels = lab_cache.get(0,y,w,1);
        if ( !pixels || !labpixels ) continue;
        for (int x = 0 ; x < w ; ++x ) {
            quantum_t rgb[3];
            rgb[0]=pixels[x].red;
            rgb[1]=pixels[x].green;
            rgb[2]=pixels[x].blue;
            quantum_t lab[3];
            RGB_to_CIELab16(rgb,lab);
            //int l=  int( lab[0] * ( double(QuantumRange)/100.L ) );
            //l= l<0?0:(l>signed(QuantumRange)?QuantumRange:l);
            labpixels[x].red=
                    labpixels[x].green=
                    labpixels[x].blue=lab[0];
        }
        lab_cache.sync();
        pixel_cache.sync();
    }

}

void unLabize(Magick::Image &image, Magick::Image &labImage) {
    int h = image.rows(),
            w = image.columns();
    image.modifyImage();
    labImage.modifyImage();
    Magick::Pixels pixel_cache(image);
    Magick::Pixels lab_cache(labImage);

#pragma omp parallel for
    for (int y = 0 ; y < h ; ++y ) {
        Magick::PixelPacket *pixels = pixel_cache.get(0,y,w,1);
        Magick::PixelPacket *labpixels = lab_cache.get(0,y,w,1);
        if ( !pixels || !labpixels ) continue;
        for (int x = 0 ; x < w ; ++x ) {
            quantum_t rgb[3];
            //triplé rgb de l'image d'origine

            rgb[0]=pixels[x].red;
            rgb[1]=pixels[x].green;
            rgb[2]=pixels[x].blue;
            //conversion du triplé de rgb ver Lab
            quantum_t lab[3];
            RGB_to_CIELab16(rgb,lab);
            quantum_t l=labpixels[x].red;
            //lab[0]= l / ( double(QuantumRange)/100.L ) ;

            //copie du L de l'image lab vers le L de l'image RGB
            lab[0]=l;
            CIELab16_to_RGB(lab,rgb);
            //rgb[0]=rgb[0]>signed(QuantumRange)?QuantumRange:rgb[0];
            //rgb[1]=rgb[1]>signed(QuantumRange)?QuantumRange:rgb[1];
            //rgb[2]=rgb[2]>signed(QuantumRange)?QuantumRange:rgb[2];
            pixels[x].red=rgb[0];
            pixels[x].green=rgb[1];
            pixels[x].blue=rgb[2];

        }
        lab_cache.sync();
        pixel_cache.sync();
    }
}

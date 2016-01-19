#ifndef CIELAB_H
#define CIELAB_H
/*
 * Copyright (c) 2006-2011, Guillaume Gimenez <guillaume@blackmilk.fr>
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
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez SA BE LIABLE FOR ANY DIRECT,
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
#include "cielab.h"
#include "photo.h"
#include <cmath>

extern const double xyz_rgb[3][3];
extern const double rgb_xyz[3][3];
extern const double illuminant[3];

#define DF_MAX_AB_MODULE (133.807616L)
#define DF_MAX_AB (110)

#define RGB_to_CIELab(rgb,lab) \
do { \
	const double epsilon = 216./24389., \
		     kappa = 24389./27.; \
        double xyz[3] = { 0., 0., 0. }; \
	xyz[0] += xyz_rgb[0][0] * (double(rgb[0])/65535.);\
	xyz[1] += xyz_rgb[1][0] * (double(rgb[0])/65535.);\
	xyz[2] += xyz_rgb[2][0] * (double(rgb[0])/65535.);\
	xyz[0] += xyz_rgb[0][1] * (double(rgb[1])/65535.);\
	xyz[1] += xyz_rgb[1][1] * (double(rgb[1])/65535.);\
	xyz[2] += xyz_rgb[2][1] * (double(rgb[1])/65535.);\
	xyz[0] += xyz_rgb[0][2] * (double(rgb[2])/65535.);\
	xyz[1] += xyz_rgb[1][2] * (double(rgb[2])/65535.);\
	xyz[2] += xyz_rgb[2][2] * (double(rgb[2])/65535.);\
	double xr = xyz[0]/illuminant[0]; \
	double yr = xyz[1]/illuminant[1]; \
	double zr = xyz[2]/illuminant[2]; \
	double fx = xr > epsilon ? pow(xr,1./3.) : ( kappa * xr + 16. ) / 116.;\
	double fy = yr > epsilon ? pow(yr,1./3.) : ( kappa * yr + 16. ) / 116.;\
	double fz = zr > epsilon ? pow(zr,1./3.) : ( kappa * zr + 16. ) / 116.;\
	lab[0] = 116. * fy - 16.;\
	lab[1] = 500. * (fx - fy);\
	lab[2] = 200. * (fy - fz);\
} while(0)



#define CIELab_to_RGB(lab,rgb) \
do { \
    double xyz[3], fx, fy, fz, xr, yr, zr, tmpf; \
    const double epsilon =  216.L/24389.L, \
		 kappa = 24389.L/27.L; \
    yr = (lab[0]<=kappa*epsilon) ? \
        (lab[0]/kappa) : (pow((lab[0]+16.0)/116.0, 3.0)); \
	fy = ( lab[0]+16.)/116.; \
    fz = fy - lab[2]/200.0; \
    fx = lab[1]/500.0 + fy; \
    zr = (pow(fz, 3.0)<=epsilon) ? ((116.0*fz-16.0)/kappa) : (pow(fz, 3.0)); \
    xr = (pow(fx, 3.0)<=epsilon) ? ((116.0*fx-16.0)/kappa) : (pow(fx, 3.0)); \
    xyz[0] = xr*illuminant[0]; \
    xyz[1] = yr*illuminant[1]; \
    xyz[2] = zr*illuminant[2]; \
        tmpf = 0; \
        tmpf += rgb_xyz[0][0] * xyz[0]; \
        tmpf += rgb_xyz[0][1] * xyz[1]; \
        tmpf += rgb_xyz[0][2] * xyz[2]; \
        rgb[0] = ( tmpf<0?0:tmpf ) * 65535.0 + 0.5; \
    if ( rgb[0] > QuantumRange ) rgb[0] = QuantumRange; \
        tmpf = 0; \
        tmpf += rgb_xyz[1][0] * xyz[0]; \
        tmpf += rgb_xyz[1][1] * xyz[1]; \
        tmpf += rgb_xyz[1][2] * xyz[2]; \
        rgb[1] = ( tmpf<0?0:tmpf ) * 65535.0 + 0.5; \
    if ( rgb[1] > QuantumRange ) rgb[1] = QuantumRange; \
        tmpf = 0; \
        tmpf += rgb_xyz[2][0] * xyz[0]; \
        tmpf += rgb_xyz[2][1] * xyz[1]; \
        tmpf += rgb_xyz[2][2] * xyz[2]; \
        rgb[2] = ( tmpf<0?0:tmpf ) * 65535.0 + 0.5; \
    if ( rgb[2] > QuantumRange ) rgb[2] = QuantumRange; \
} while(0)


//QuantumRange is defined as Quantum which is not in scope
using Magick::Quantum;

static inline void lab_to_int(const double lab[3], quantum_t lab16[3]) {
	lab16[0]=lab[0]*65535.L/100.L;
	lab16[1]=(lab[1]+120.L)*65535.L/240.L;
	lab16[2]=(lab[2]+120.L)*65535.L/240.L;
}

static inline void int_to_lab(const quantum_t lab16[3], double lab[3]) {
	lab[0] = double(lab16[0])*100.L/65535.L;
	lab[1] = double(lab16[1])*240.L/65535.L-120;
	lab[2] = double(lab16[2])*240.L/65535.L-120;
}



static inline void RGB_to_CIELab16(const quantum_t rgb[3], quantum_t lab16[3] ) {
	double lab[3];
	RGB_to_CIELab(rgb,lab);
	lab_to_int(lab,lab16);
}

static inline void CIELab16_to_RGB(const quantum_t lab16[3], quantum_t rgb[3] ) {
	double lab[3];
	int_to_lab(lab16,lab);
	CIELab_to_RGB(lab,rgb);
}

#define __lab_kappa (24389.L/27.L)
#define __lab_epsilon (216.L/24389.L)

#define lab_linearize(l) ( (l<=__lab_kappa*__lab_epsilon) ? (l/__lab_kappa) : (pow((l+16.0)/116.0, 3.0)) )

#define lab_gammaize(v) ( 116. * ( v > __lab_epsilon ? pow(v,1./3.) : ( __lab_kappa * v + 16. ) / 116. ) - 16. )


/**
 * @param rgb int[3] linear [0..65535]
 * @param[out] lab double[3] Lab [0..1,-120..120,-120..120]
 */
#define RGB_to_LinearLab(rgb,lab) \
do { \
	const double epsilon = 216./24389., \
		     kappa = 24389./27.; \
	double xyz[3] = { 0., 0., 0. }; \
	xyz[0] += xyz_rgb[0][0] * (double(rgb[0])/65535.);\
	xyz[1] += xyz_rgb[1][0] * (double(rgb[0])/65535.);\
	xyz[2] += xyz_rgb[2][0] * (double(rgb[0])/65535.);\
	xyz[0] += xyz_rgb[0][1] * (double(rgb[1])/65535.);\
	xyz[1] += xyz_rgb[1][1] * (double(rgb[1])/65535.);\
	xyz[2] += xyz_rgb[2][1] * (double(rgb[1])/65535.);\
	xyz[0] += xyz_rgb[0][2] * (double(rgb[2])/65535.);\
	xyz[1] += xyz_rgb[1][2] * (double(rgb[2])/65535.);\
	xyz[2] += xyz_rgb[2][2] * (double(rgb[2])/65535.);\
	double xr = xyz[0]/illuminant[0]; \
	double yr = xyz[1]/illuminant[1]; \
	double zr = xyz[2]/illuminant[2]; \
	double fx = xr > epsilon ? pow(xr,1./3.) : ( kappa * xr + 16. ) / 116.; \
	double fy = yr > epsilon ? pow(yr,1./3.) : ( kappa * yr + 16. ) / 116.; \
	double fz = zr > epsilon ? pow(zr,1./3.) : ( kappa * zr + 16. ) / 116.; \
	lab[0] = yr; \
	lab[1] = 500. * (fx - fy); \
	lab[2] = 200. * (fy - fz); \
} while(0)



/**
 * @param lab double[3] Lab [0..1,-120..120,-120..120]
 * @param[out] rgb int[3] linear [0..65535]
 */
#define LinearLab_to_RGB(lab,rgb) \
do { \
    double xyz[3], fx, fy, fz, xr, yr, zr, tmpf; \
    const double epsilon =  216.L/24389.L, \
		 kappa = 24389.L/27.L; \
        yr = lab[0]; \
        fy = yr > epsilon ? pow(yr,1./3.) : ( kappa * yr + 16. ) / 116.; \
    fz = fy - lab[2]/200.0; \
    fx = lab[1]/500.0 + fy; \
    zr = (pow(fz, 3.0)<=epsilon) ? ((116.0*fz-16.0)/kappa) : (pow(fz, 3.0)); \
    xr = (pow(fx, 3.0)<=epsilon) ? ((116.0*fx-16.0)/kappa) : (pow(fx, 3.0)); \
    xyz[0] = xr*illuminant[0]; \
    xyz[1] = yr*illuminant[1]; \
    xyz[2] = zr*illuminant[2]; \
        tmpf = 0; \
        tmpf += rgb_xyz[0][0] * xyz[0]; \
        tmpf += rgb_xyz[0][1] * xyz[1]; \
        tmpf += rgb_xyz[0][2] * xyz[2]; \
        rgb[0] = ( tmpf<0?0:tmpf ) * 65535.0 + 0.5; \
	if ( rgb[0] > QuantumRange ) rgb[0] = QuantumRange; \
        tmpf = 0; \
        tmpf += rgb_xyz[1][0] * xyz[0]; \
        tmpf += rgb_xyz[1][1] * xyz[1]; \
        tmpf += rgb_xyz[1][2] * xyz[2]; \
        rgb[1] = ( tmpf<0?0:tmpf ) * 65535.0 + 0.5; \
	if ( rgb[1] > QuantumRange ) rgb[1] = QuantumRange; \
        tmpf = 0; \
        tmpf += rgb_xyz[2][0] * xyz[0]; \
        tmpf += rgb_xyz[2][1] * xyz[1]; \
        tmpf += rgb_xyz[2][2] * xyz[2]; \
        rgb[2] = ( tmpf<0?0:tmpf ) * 65535.0 + 0.5; \
	if ( rgb[2] > QuantumRange ) rgb[2] = QuantumRange; \
} while(0)

#endif // CIELAB_H

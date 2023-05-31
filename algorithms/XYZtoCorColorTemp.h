/*
 *      Name:   XYZtoCorColorTemp.c
 *
 *      Author: Bruce Justin Lindbloom
 *
 *      Copyright (c) 2003 Bruce Justin Lindbloom. All rights reserved.
 *
 *      Input:  xyz = pointer to the input array of X, Y and Z color components (in that order).
 *              temp = pointer to where the computed correlated color temperature should be placed.
 *
 *      Output: *temp = correlated color temperature, if successful.
 *                    = unchanged if unsuccessful.
 *
 *      Return: 0 if successful, else -1.
 *
 *      Description:
 *              This is an implementation of Robertson's method of computing the correlated color
 *              temperature of an XYZ color. It can compute correlated color temperatures in the
 *              range [1666.7K, infinity].
 *
 *      Reference:
 *              "Color Science: Concepts and Methods, Quantitative Data and Formulae", Second Edition,
 *              Gunter Wyszecki and W. S. Stiles, John Wiley & Sons, 1982, pp. 227, 228.
 */

#ifndef __XYZTOCORCOLORTEMP__
#define __XYZTOCORCOLORTEMP__
extern "C" int XYZtoCorColorTemp(double *xyz, double *temp);


#endif

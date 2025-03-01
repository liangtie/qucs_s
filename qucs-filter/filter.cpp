/***************************************************************************
  copyright: (C) 2010 by Michael Margraf
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "lc_filter.h"

#include <QString>
#include <QMessageBox>

Filter::Filter()
{
}

// -----------------------------------------------------------------------
// Returns the value of the E6 serie that is the next higher number to "value".
double Filter::getE6value(double value)
{
  double e = floor(log10(value));
  value /= pow(10.0, e);
  if(value == 1.0)
    value = 1.0;
  else if(value <= 1.2)
    value = 1.2;
  else if(value <= 1.5)
    value = 1.5;
  else if(value <= 2.2)
    value = 2.2;
  else if(value <= 3.3)
    value = 3.3;
  else if(value <= 4.7)
    value = 4.7;
  else if(value <= 6.8)
    value = 6.8;
  else
    value = 10.0;

  return value * pow(10.0, e);
}

// -----------------------------------------------------------------------
double Filter::getNormValue(int No, tFilter *theFilter)
{
  if((No < 0) || (No >= theFilter->Order))
    return 1.0;

  switch(theFilter->Type) {
    case TYPE_BESSEL:
      return BesselValue(No, theFilter->Order);
    case TYPE_BUTTERWORTH:
      return ButterworthValue(No, theFilter->Order);
    case TYPE_CHEBYSHEV:
      if((theFilter->Order & 1) == 0) {
        QMessageBox::critical(0, "Error", "Even order Chebyshev can't be realized with passive filters.");
        return 2e30;
      }
      return ChebyshevValue(No, theFilter->Order, theFilter->Ripple);
  }

  QMessageBox::critical(0, "Error", "Filter type not supported.");
  return 2e30;
}

// -----------------------------------------------------------------------
double Filter::getQuadraticNormValues(int No, tFilter *theFilter, double &b)
{
  if((No < 0) || (No >= theFilter->Order))
    return 1.0;

  switch(theFilter->Type) {
    case TYPE_BESSEL:
      return quadraticBesselValues(No, theFilter->Order, b);
    case TYPE_BUTTERWORTH:
      return quadraticButterworthValues(No, theFilter->Order, b);
    case TYPE_CHEBYSHEV:
      return quadraticChebyshevValues(No, theFilter->Order, theFilter->Ripple, b);
  }

  QMessageBox::critical(0, "Error", "Filter type not supported.");
  return 2e30;
}

// -----------------------------------------------------------------------
QString Filter::num2str(double Num)
{
  char c = 0;
  double cal = fabs(Num);
  if(cal > 1e-20) {
    cal = log10(cal) / 3.0;
    if(cal < -0.2)  cal -= 0.98;
    int Expo = int(cal);

    if(Expo >= -5) if(Expo <= 4)
      switch(Expo) {
        case -5: c = 'f'; break;
        case -4: c = 'p'; break;
        case -3: c = 'n'; break;
        case -2: c = 'u'; break;
        case -1: c = 'm'; break;
        case  1: c = 'k'; break;
        case  2: c = 'M'; break;
        case  3: c = 'G'; break;
        case  4: c = 'T'; break;
      }

    if(c)  Num /= pow(10.0, double(3*Expo));
  }

  QString Str = QString::number(Num, 'g', 4);
  if(c)  Str += c;
  
  return Str;
}


// -----------------------------------------------------------------------
double BesselCoef[18][23] = {

/*  2 */ {0.57550275, 2.1478055,    0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/*  3 */ {0.33742149, 0.97051182,   2.2034114,  0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/*  4 */ {0.2334158,  0.67252481,   1.0815161,  2.2403786,  0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/*  5 */ {0.17431938, 0.50724063,   0.80401117, 1.1110332,  2.2582171,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/*  6 */ {0.13649238, 0.40018984,   0.6391554,  0.85378587, 1.112643,
          2.2645236,  0.0,          0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/*  7 */ {0.11056245, 0.32588813,   0.52489273, 0.70200915, 0.86902684,
          1.1051644,  2.2659006,    0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/*  8 */ {0.091905558, 0.27191069,  0.44092213, 0.59357268, 0.73025665,
          0.86950037,  1.0955593,   2.2656071,  0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/*  9 */ {0.077965506, 0.23129119,  0.37698651, 0.5107787,  0.63059516,
          0.74073299,  0.86387345,  1.0862838,  2.2648789,  0.0,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/* 10 */ {0.067229245, 0.19984023,  0.32699699, 0.44543381, 0.55281473,
          0.64933545,  0.74201735,  0.85607238, 1.0780948,  2.2641262,
          0.0,         0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/* 11 */ {0.058753264, 0.1749102,   0.28706331, 0.39272513, 0.48983467,
          0.57742576,  0.6573719,   0.73864992, 0.84789472, 1.0711184,
          2.2634675,   0.0,         0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/* 12 */ {0.051923,    0.15475798,  0.25458412, 0.34949045, 0.43777572,
          0.51824604,  0.59097358,  0.65913453, 0.73309605, 0.84013853,
          1.0652583,   2.2629233,   0.0,        0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/* 13 */ {0.046323098, 0.13819538,  0.22775963, 0.31352943, 0.39412779,
          0.46842754,  0.53589822,  0.59744624, 0.65727966, 0.72670805,
          0.83311934,  1.0603538,   2.2624829,  0.0,        0.0,
          0.0,         0.0,         0.0,        0.0,},

/* 14 */ {0.041663913, 0.12438811,  0.20530976, 0.28325704, 0.35711858,
          0.42591257,  0.48895292,  0.54624778, 0.59939971, 0.6534283,
          0.72022065,  0.82691896,  1.0562406,  2.262128,   0.0,
          0.0,         0.0,         0.0,        0.0,},

/* 15 */ {0.037738195, 0.11273464,  0.186303,   0.25750279, 0.32543754,
          0.38927393,  0.44833655,  0.5022966,  0.55162099, 0.59850238,
          0.64857809,  0.71401822,  0.8215094,  1.0527729,  2.2618408,
          0.0,         0.0,         0.0,        0.0},

/* 16 */ {0.03439122,  0.102799,    0.1700389,  0.23539257, 0.29808411,
          0.35745153,  0.41285998,  0.4638916,  0.51051757, 0.55360981,
          0.59585789,  0.64334149,  0.70828423, 0.81681581, 1.0498286,
          2.2616066,   0.0,         0.0,        0.0},

/* 17 */ {0.031523496, 0.094214373, 0.1560301,  0.21622781, 0.27429733,
          0.32962199,  0.38165374,  0.42998671, 0.47437364, 0.51507678,
          0.55331904,  0.59220181,  0.63808843, 0.70308744, 0.81274832,
          1.0473087,   2.2614134,   0.0,        0.0},

/* 18 */ {0.028450351, 0.088289337, 0.14199897, 0.20112875, 0.25237658,
          0.30571616,  0.35380272,  0.39992872, 0.44218287, 0.48110224,
          0.5170226,   0.55152405,  0.58802463, 0.63303427, 0.69843256,
          0.80921764,  1.0451338,   2.2612522,  0.0},

/* 19 */ {0.027944243, 0.077389235, 0.13716781, 0.18087554, 0.23831545,
          0.28142035,  0.33055916,  0.37249664, 0.41348365, 0.45066049,
          0.48503552,  0.51711998,  0.54877223, 0.58365096, 0.62829555,
          0.69429034,  0.8061422,   1.043241,   2.2611162}
};

// -----------------------------------------------------------------------
// Calculate normalized component value for Bessel filter
double Filter::BesselValue(int No, int Order)
{
  return BesselCoef [Order - 2] [No];
}


// -----------------------------------------------------------------------
// Calculate normalized component value for Butterworth filter
double Filter::ButterworthValue(int No, int Order)
{
  return 2.0 * sin(double(2*No + 1) / double(2*Order) * pi);
}


// -----------------------------------------------------------------------
// This function calculates normalized component value for Chebyshev filter
// "Ripple" is in dB !
double Filter::ChebyshevValue(int No, int Order, double Ripple)
{
  int i;
  double ak, gk, a, b;

  Ripple = sqrt(pow(10.0, Ripple / 10.0) - 1.0);
  Ripple = sinh(asinh(1.0 / Ripple) / double(Order));

  a = sin(0.5 / double(Order) * pi);
  gk = a / Ripple;
  for(i=1; i<=No; i++) {  // coefficients are defined recursively
    ak  = a;
    a   = sin(double(2 * i + 1) / double(2 * Order) * pi);
    b   = sin(double(i) * pi / double(Order));
    gk *= Ripple * Ripple + b * b;
    gk  = ak * a / gk;
  }

  return 2.0 * gk;
}

// -----------------------------------------------------------------------
double BesselCoefA[18][10] = {
 {1.36165413e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {9.99629202e-1, 7.56043166e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {7.74253975e-1, 1.33966370e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {6.21595206e-1, 1.14017670e+0, 6.65638800e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {5.13053656e-1, 9.68607026e-1, 1.22173438e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {4.33227776e-1, 8.30363090e-1, 1.09443685e+0, 5.93694426e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {3.72765065e-1, 7.20236277e-1, 9.75366333e-1, 1.11124956e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {3.25741595e-1, 6.31959746e-1, 8.71016716e-1, 1.02435625e+0, 5.38618836e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {2.88317886e-1, 5.60356263e-1, 7.81532026e-1, 9.39275303e-1, 1.02149912e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {2.57940189e-1, 5.01515079e-1, 7.05205806e-1, 8.60697679e-1, 9.58389453e-1, 4.95859210e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {2.32861727e-1, 4.52546178e-1, 6.40003229e-1, 7.89952794e-1, 8.94878610e-1, 9.48908283e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {2.11855068e-1, 4.11312404e-1, 5.84048406e-1, 7.26963090e-1, 8.34217677e-1, 9.00765966e-1, 4.61662889e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.94036277e-1, 3.76219036e-1, 5.35748797e-1, 6.71104591e-1, 7.77775499e-1, 8.51512428e-1, 8.89196782e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.78754672e-1, 3.46061743e-1, 4.93795485e-1, 6.21587105e-1, 7.25971504e-1, 8.03409152e-1, 8.51071594e-1, 4.33581765e-1, 0.00000000e+0, 0.00000000e+0},
 {1.65521843e-1, 3.19919293e-1, 4.57125889e-1, 5.77618536e-1, 6.78756100e-1, 7.57607810e-1, 8.11671545e-1, 8.39164449e-1, 0.00000000e+0, 0.00000000e+0},
 {1.53964763e-1, 2.97078021e-1, 4.24880024e-1, 5.38470831e-1, 6.35860400e-1, 7.14630916e-1, 7.72600158e-1, 8.08085104e-1, 4.10016332e-1, 0.00000000e+0},
 {1.43794085e-1, 2.76978325e-1, 3.96360733e-1, 5.03501168e-1, 5.96926221e-1, 6.74646597e-1, 7.34775005e-1, 7.75766941e-1, 7.96540602e-1, 0.00000000e+0},
 {1.34782229e-1, 2.59176387e-1, 3.71000554e-1, 4.72153283e-1, 5.61572995e-1, 6.37627122e-1, 6.98691780e-1, 7.43373179e-1, 7.70618449e-1, 3.89886704e-1}};

double BesselCoefB[18][10] = {
 {6.18033989e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {4.77191359e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {3.88990734e-1, 4.88904151e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {3.24532958e-1, 4.12845035e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {2.75640713e-1, 3.50472682e-1, 3.88718371e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {2.38071830e-1, 3.01095494e-1, 3.39457362e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {2.08745496e-1, 2.62125007e-1, 2.97923772e-1, 3.16161446e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.85417727e-1, 2.31048972e-1, 2.63561659e-1, 2.83414384e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.66512035e-1, 2.05908519e-1, 2.35144786e-1, 2.54933562e-1, 2.64963862e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.50928257e-1, 1.85267830e-1, 2.11494870e-1, 2.30458269e-1, 2.41997904e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.37889361e-1, 1.68085979e-1, 1.91640272e-1, 2.09464363e-1, 2.21510535e-1, 2.27594748e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.26836232e-1, 1.53603125e-1, 1.74816790e-1, 1.91408216e-1, 2.03410089e-1, 2.10690872e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.17358569e-1, 1.41257383e-1, 1.60431675e-1, 1.75804748e-1, 1.87465329e-1, 1.95327042e-1, 1.99288298e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.09149583e-1, 1.30627119e-1, 1.48025483e-1, 1.62244776e-1, 1.73411905e-1, 1.81477112e-1, 1.86358434e-1, 0.00000000e+0, 0.00000000e+0, 0.00000000e+0},
 {1.01975884e-1, 1.21391278e-1, 1.37240264e-1, 1.50391095e-1, 1.60995980e-1, 1.69033936e-1, 1.74442048e-1, 1.77162502e-1, 0.00000000e+0, 0.00000000e+0},
 {9.56571087e-2, 1.13301940e-1, 1.27794991e-1, 1.39968287e-1, 1.49990121e-1, 1.57861706e-1, 1.63537080e-1, 1.66966155e-1, 0.00000000e+0, 0.00000000e+0},
 {9.00518801e-2, 1.06165141e-1, 1.19467098e-1, 1.30751802e-1, 1.40197067e-1, 1.47821002e-1, 1.53592086e-1, 1.57468684e-1, 1.59416436e-1, 0.00000000e+0},
 {8.50479394e-2, 9.98272764e-2, 1.12078713e-1, 1.22558206e-1, 1.31448429e-1, 1.38780190e-1, 1.44534000e-1, 1.48676318e-1, 1.51176082e-1, 0.00000000e+0}};

// -----------------------------------------------------------------------
// Calculate normalized quadratic polynom coefficients of Bessel filters.
double Filter::quadraticBesselValues(int No, int Order, double &b)
{
  if(No == 0) {  // linear coefficient for odd order filter?
    b = 0;
    return BesselCoefA [Order - 2] [Order/2];
  }

  b = BesselCoefB [Order - 2] [No-1];
  return BesselCoefA [Order - 2] [No-1];
}

// -----------------------------------------------------------------------
// Calculate normalized quadratic polynom coefficients of Butterworth filters.
double Filter::quadraticButterworthValues(int No, int Order, double &b)
{
  if(No == 0) {  // linear coefficient for odd order filter?
    b = 0;
    return 1.0;
  }

  double a;
  if(Order & 1)
    a = double(No);
  else
    a = double(2*No-1) / 2.0;
  a = 2.0 * cos(a * pi / double(Order));
  b = 1.0;
  return a;
}

// -----------------------------------------------------------------------
// Calculate normalized quadratic polynom coefficients of Chebyshev filters.
double Filter::quadraticChebyshevValues(int No, int Order, double Ripple, double &b)
{
  double a;
  if(No == 0) {  // linear coefficient for odd order filter?
    b = 0;
    a = asinh(1.0 / sqrt(pow(10.0, Ripple/10.0) - 1.0)) / double(Order);
    a = 1.0 / sinh(a);
	return a;
  }

  double c = asinh(1.0 / sqrt(pow(10.0, Ripple/10.0) - 1.0)) / double(Order);
  if(Order & 1)
    a = double(No);
  else
    a = double(2*No-1) / 2.0;
  a  = cos(a * pi / double(Order));
  b  = 1.0 / (cosh(c) * cosh(c) - a*a);
  a *= 2.0 * b * sinh(c);
  return a;
}

// SCHEMATIC DRAWING FUNCTIONS

QString Filter::getLineString(bool isMicrostrip, double width_or_impedance, double l, int x, int y, int rotate)
{
  QString code;
  double text_x, text_y; // Coordinates for the text. They change with the rotation
  if (isMicrostrip)
  {
     if (rotate)
     {
         text_x = 20;
         text_y = -25;
     }
     else
     {
         text_x = -30;
         text_y = -70;
     }
     code = QStringLiteral("<MLIN MS1 1 %1 %2 %3 %4 0 %5 \"Sub1\" 0 \"%6 mm\" 1 \"%7 mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"26.85\" 0>\n")
              .arg(x).arg(y).arg(text_x).arg(text_y).arg(rotate)
              .arg(QString::number(width_or_impedance*1e3, 'f', 2)).arg(QString::number(l*1e3, 'f', 2));
  }
  else
  {
      if (rotate)
      {
          text_x = 10;
          text_y = -25;
      }
      else
      {
          text_x = -30;
          text_y = -60;
      }
     code =  QStringLiteral("<TLIN Line1 1 %1 %2 %3 %4 0 %5 \"%6 Ohm\" 1 \"%7 mm\" 1 \"0 dB\" 0 \"26.85\" 0>\n")
            .arg(x).arg(y).arg(text_x).arg(text_y).arg(rotate).arg(QString::number(width_or_impedance, 'f', 1)).arg(QString::number(l*1e3, 'f', 2));
  }
  return code;
}

QString Filter::getMS_Via(double height, int x, int y, int rotate)
{
    QString code;
    switch (rotate)
    {
        case 0: // No rotation
            code = QStringLiteral("<MVIA MS31 1 %1 %2 20 -10 0 0 \"Sub1\" 0 \"%3 mm\" 1 \"26.85\" 0>\n").arg(x).arg(y).arg(height);
            break;

        case 1: // CTRL+R
            code = QStringLiteral("<MVIA MS31 1 %1 %2 40 -20 0 1 \"Sub1\" 0 \"%3 mm\" 1 \"26.85\" 0>\n").arg(x).arg(y).arg(height);
            break;

        case 2: // 2 x (CTRL+R)
            code = QStringLiteral("<MVIA MS31 1 %1 %2 -85 -40 0 2 \"Sub1\" 0 \"%3 mm\" 1 \"26.85\" 0>\n").arg(x).arg(y).arg(height);
            break;

        case 3: // 3 x (CTRL+R)
            code = QStringLiteral("<MVIA MS31 1 %1 %2 -90 -20 0 3 \"Sub1\" 0 \"%3 mm\" 1 \"26.85\" 0>\n").arg(x).arg(y).arg(height);
            break;
    }
    return code;
}

QString Filter::getMS_Open(double width, int x, int y, int rotate)
{
    QString code;
    switch (rotate)
    {
        case 0: // No rotation
            code = QStringLiteral("<MOPEN MS93 1 %1 %2 -20 -50 0 0 \"Sub1\" 0 \"%3 mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"Kirschning\" 0>\n").arg(x).arg(y).arg(QString::number(width*1e3, 'f', 2));
            break;

        case 1: // CTRL+R
            code = QStringLiteral("<MOPEN MS93 1 %1 %2 15 -12 0 1 \"Sub1\" 0 \"%3 mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"Kirschning\" 0>\n").arg(x).arg(y).arg(QString::number(width*1e3, 'f', 2));
            break;

        case 2: // 2 x (CTRL+R)
            code = QStringLiteral("<MOPEN MS93 1 %1 %2 -20 -50 0 2 \"Sub1\" 0 \"%3 mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"Kirschning\" 0>\n").arg(x).arg(y).arg(QString::number(width*1e3, 'f', 2));
            break;

        case 3: // 3 x (CTRL+R)
            code = QStringLiteral("<MOPEN MS93 1 %1 %2 15 -20 0 3 \"Sub1\" 0 \"%3 mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"Kirschning\" 0>\n").arg(x).arg(y).arg(QString::number(width*1e3, 'f', 2));
            break;
    }
    return code;
}

QString Filter::getWireString(int x1, int y1, int x2, int y2)
{
  return QStringLiteral("<%1 %2 %3 %4 \"\" 0 0 0>\n").arg(x1).arg(y1).arg(x2).arg(y2);
}
QString Filter::getTeeString(int x, int y, double width1, double width2, double width3)
{
  return QStringLiteral("<MTEE MS1 1 %1 %2 -26 20 1 0 \"Sub1\" 0 \"%3mm\" 1 \"%4mm\" 1 \"%5mm\" 1 \"Hammerstad\" 0 \"Kirschning\" 0 \"26.85\" 0>\n")
        .arg(x).arg(y).arg(QString::number(width1*1e3, 'f', 2)).arg(QString::number(width2*1e3, 'f', 2)).arg(QString::number(width3*1e3, 'f', 2));
}

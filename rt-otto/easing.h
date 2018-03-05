#ifndef EASING_H
#define EASING_H

#include <math.h>

extern const double PI;

double easeLinear(double t);
double easeSineIn(double t);
double easeSineOut(double t);
double easeSineInOut(double t);
double easeQuadIn(double t);
double easeQuadOut(double t);
double easeQuadInOut(double t);
double easeCubicIn(double t);
double easeCubicOut(double t);
double easeCubicInOut(double t);
double easeQuarticIn(double t);
double easeQuarticOut(double t);
double easeQuarticInOut(double t);
double easeExponentialIn(double t);
double easeExponentialOut(double t);
double easeExponentialInOut(double t);
double easeCircularIn(double t);
double easeCircularOut(double t);
double easeCircularInOut(double t);
double easeBackIn(double t);
double easeBackOut(double t);
double easeBackInOut(double t);
double easeElasticIn(double t);
double easeElasticOut(double t);
double easeElasticInOut(double t);
double easeBounceOut(double t);
double easeBounceIn(double t);
double easeBounceInOut(double t);

double oscillate(double t);

extern double (*EASING[28])(double);

#endif

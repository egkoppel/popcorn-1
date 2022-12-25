#ifndef _HUGOS_MATH_H
#define _HUGOS_MATH_H

float acosf(float);
double acos(double);
long double acosl(long double);
    
float asinf(float);
double asin(double);
long double asinl(long double);
    
float atanf(float);
double atan(double);
long double atanl(long double);
    
float atan2f(float, float);
double atan2(double, double);
long double atan2l(long double, long double);
    
float cosf(float);
double cos(double);
long double cosl(long double);
    
float sinf(float);
double sin(double);
long double sinl(long double);
    
float tanf(float);
double tan(double);
long double tanl(long double);
    
float acoshf(float);
double acosh(double);
long double acoshl(long double);
    
float asinhf(float);
double asinh(double);
long double asinhl(long double);
    
float atanhf(float);
double atanh(double);
long double atanhl(long double);
    
float coshf(float);
double cosh(double);
long double coshl(long double);
    
float sinhf(float);
double sinh(double);
long double sinhl(long double);
    
float tanhf(float);
double tanh(double);
long double tanhl(long double);
    
float expf(float);
double exp(double);
long double expl(long double);

float exp2f(float);
double exp2(double); 
long double exp2l(long double); 

float expm1f(float);
double expm1(double); 
long double expm1l(long double); 

float logf(float);
double log(double);
long double logl(long double);

float log10f(float);
double log10(double);
long double log10l(long double);

float log2f(float);
double log2(double);
long double log2l(long double);

float log1pf(float);
double log1p(double);
long double log1pl(long double);

float logbf(float);
double logb(double);
long double logbl(long double);

float modff(float, float *);
double modf(double, double *);
long double modfl(long double, long double *);

float ldexpf(float, int);
double ldexp(double, int);
long double ldexpl(long double, int);

float frexpf(float, int *);
double frexp(double, int *);
long double frexpl(long double, int *);

int ilogbf(float);
int ilogb(double);
int ilogbl(long double);

float scalbnf(float, int);
double scalbn(double, int);
long double scalbnl(long double, int);

float scalblnf(float, long int);
double scalbln(double, long int);
long double scalblnl(long double, long int);

float fabsf(float);
double fabs(double);
long double fabsl(long double);

float cbrtf(float);
double cbrt(double);
long double cbrtl(long double);

float hypotf(float, float);
double hypot(double, double);
long double hypotl(long double, long double);

float powf(float, float);
double pow(double, double);
long double powl(long double, long double);

float sqrtf(float);
double sqrt(double);
long double sqrtl(long double);

float erff(float);
double erf(double);
long double erfl(long double);

float erfcf(float);
double erfc(double);
long double erfcl(long double);

float lgammaf(float);
double lgamma(double);
long double lgammal(long double);

float tgammaf(float);
double tgamma(double);
long double tgammal(long double);

float ceilf(float);
double ceil(double);
long double ceill(long double);

float floorf(float);
double floor(double);
long double floorl(long double);

float nearbyintf(float);
double nearbyint(double);
long double nearbyintl(long double);

float rintf(float);
double rint(double);
long double rintl(long double);

long int lrintf(float);
long int lrint(double);
long int lrintl(long double);

float roundf(float);
double round(double);
long double roundl(long double);

long int lroundf(float);
long int lround(double);
long int lroundl(long double);

float truncf(float);
double trunc(double);
long double truncl(long double);

float fmodf(float, float);
double fmod(double, double);
long double fmodl(long double, long double);

float remainderf(float, float);
double remainder(double, double);
long double remainderl(long double, long double);

float remquof(float, float, int *);
double remquo(double, double, int *);
long double remquol(long double, long double, int *);

float copysignf(float, float);
double copysign(double, double);
long double copysignl(long double, long double);

float nanf(const char *);
double nan(const char *);
long double nanl(const char *);

float nextafterf(float, float);
double nextafter(double, double);
long double nextafterl(long double, long double);

double nexttoward(double, long double);
float nexttowardf(float, long double);
long double nexttowardl(long double, long double);

float fdimf(float, float);
double fdim(double, double);
long double fdiml(long double, long double);

float fmaxf(float, float);
double fmax(double, double);
long double fmaxl(long double, long double);

float fminf(float, float);
double fmin(double, double);
long double fminl(long double, long double);

float fmaf(float, float, float);
double fma(double, double, double);
long double fmal(long double, long double, long double);

#endif
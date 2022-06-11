#include <limits.h>
#include <math.h>
#include <string.h>
#include "proton-aux.h"

#if !NO_SSE
#   include <immintrin.h>
#endif

#define UCHAR_MAXF (float)UCHAR_MAX

#define STATIC_CAST(type, expr) (type)(expr)

#if defined _MSC_VER
#   define _q(qualifiers)
#else
#   define _q(qualifiers) qualifiers
#endif


static float cmap_maxf(float x, float y)
{
    return (x < y) ? y : x;
}

static float cmap_minf(float x, float y)
{
    return (x < y) ? x : y;
}

static float cmap_clamp(float x, const float lbound, const float ubound)
{
    return cmap_minf(cmap_maxf(x, lbound), ubound);
}

static unsigned char cmap_red(float x)
{   
    x = cmap_clamp(4.0f * (x - 0.25f), 0.0f, 1.0f);
    return STATIC_CAST(unsigned char, UCHAR_MAXF * x);
}

static unsigned char cmap_green(float x)
{
    if (x < 0.5f) {
        x = 2.0f * x;
    } else if (x < 0.75f) {
        x = 1.5f - x;
    } else {
        x = 3.0f * (1.0f - x);
    }
    return STATIC_CAST(unsigned char, UCHAR_MAXF * x);
}

static unsigned char cmap_blue(float x)
{
    x = cmap_clamp(4.0f * (0.25f - x), 0.0f, 1.0f);
    return STATIC_CAST(unsigned char, UCHAR_MAXF * x);
}

void proton_colormap(float x, unsigned char pixel[_q(static 3)])
{
    pixel[0] = cmap_red(x);
    pixel[1] = cmap_green(x);
    pixel[2] = cmap_blue(x);
}

#if NO_SSE
#   pragma warning("Compiling WITHOUT SSE support!")

static void proton_fmadds(const double a[_q(static 2)],
                          const double *b,
                          double c[_q(static 2)])
{
    c[0] = fma(a[0], *b, c[0]);
    c[1] = fma(a[1], *b, c[1]);
}

static void proton_vecmul(const double mat[_q(static 4)],
                          double vec[_q(static 2)])
{
    double tmp[2] = { 0 };
    proton_fmadds(mat + 0x0, vec + 0x0, tmp);
    proton_fmadds(mat + 0x2, vec + 0x1, tmp);
    memcpy(vec, tmp, sizeof tmp);
}

static void proton_vecadd(const double vec1[_q(static 2)],
                          double vec2[_q(static 2)])
{
    vec2[0] += vec1[0];
    vec2[1] += vec1[1];
}

#else

static void proton_fmadds(const double a[_q(static 2)],
                          const double b,
                          __m128d *c)
{
    *c = _mm_fmadd_pd(_mm_load_pd(a), _mm_set1_pd(b), *c);
}

static void proton_vecmul(const double mat[_q(static 4)],
                          double vec[_q(static 2)])
{
    __m128d tmp = _mm_setzero_pd();
    proton_fmadds(mat + 0x0, vec[0], &tmp);
    proton_fmadds(mat + 0x2, vec[1], &tmp);
    _mm_store_pd(vec, tmp);
}

static void proton_vecadd(const double vec1[_q(static 2)],
                          double vec2[_q(static 2)])
{
    __m128d v1 = _mm_load_pd(vec1);
    __m128d v2 = _mm_load_pd(vec2);
    _mm_store_pd(vec2, _mm_add_pd(v1, v2));
}

#endif

void proton_affine_matmul(const double mat1[_q(static 6)],
                          double mat2[_q(static 6)])
{
    proton_vecmul(mat1, mat2 + 0x0); 
    proton_vecmul(mat1, mat2 + 0x2);
    proton_vecmul(mat1, mat2 + 0x4);
    proton_vecadd(mat1 + 0x4, mat2 + 0x4);
}

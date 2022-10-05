#include <math.h>

#include <stdio.h>  /* DELETEME PL0X */

#include <stdlib.h>
#include <string.h>
#include "proton-dose.h"
#include "dcmload.h"

#if defined _MSC_VER
#   define _q(qualifiers)
#else
#   define _q(qualifiers) qualifiers
#endif

#define STATIC_CAST(type, expr) (type)(expr)

#define IDIVCEIL(num, denom) (((num) + (denom) - 1) / (denom))

/* The threshold dose used to determine whether a plane is empty or not
If all dose values are less than this, it is empty */
#define NULL_THRESH 0.001

/** TODO: Rewrite the cell interpolator: create functions for some of its
 *  branches (SEE BELOW) */


struct _proton_dose {
    /* Everything in this section must be extracted from the DICOM */    
    double top_left[3];
    double px_spacing[3];
    long px_dimensions[3];

    /* Everything in this section (except data) is derived from the above */
    long nplanes;
    float *planes, *linedose;
    float dmax, data[];
};


/* ---------------------------------------------------------------------- */
/*                                 Planes                                 */
/* ---------------------------------------------------------------------- */

static float proton_planes_linesum(const float *fptr, const float *const end)
{
    float acc = 0.0;
    do {
        acc += *fptr;
    } while (++fptr < end);
    return acc;
}

static void proton_planes_integrate(ProtonDose *dose)
{
    const float *f1 = dose->data, *f2 = dose->data + dose->px_dimensions[0];
    long j, k;
    for (k = 0; k < dose->px_dimensions[2]; k++) {
        for (j = 0; j < dose->px_dimensions[1]; j++) {
            dose->planes[j] += proton_planes_linesum(f1, f2);
            f1 = f2;
            f2 += dose->px_dimensions[0];
        }
    }
}

static void proton_planes_constrict(ProtonDose *dose)
{
    void *testptr;
    long i, end;
    for (i = end = 0; i < dose->px_dimensions[1]; i++) {
        end = (dose->planes[i] > NULL_THRESH) ? i : end;
    }
    testptr = realloc(dose->planes, sizeof *dose->planes * ++end);
    if (testptr) {
        dose->planes = testptr;
        dose->nplanes = end;
    } else {
        /* Honestly, is this even possible on the target machines? */
        free(dose->planes);
        dose->planes = NULL;
    }
}

static void proton_planes_create(ProtonDose *dose)
{
    dose->planes = calloc(dose->px_dimensions[1], sizeof *dose->planes);
    if (dose->planes) {
        proton_planes_integrate(dose);
        proton_planes_constrict(dose);
    }
}


/* ---------------------------------------------------------------------- */
/*                               Line dose                                */
/* ---------------------------------------------------------------------- */


/** This one is nearly free--only one half the pixel spacing */
static double proton_dose_min_depth(const ProtonDose *dose)
{
    return dose->px_spacing[1] / 2.0;
}

static double proton_dose_max_depth(const ProtonDose *dose)
{
    return (double)dose->nplanes * dose->px_spacing[1];
}

double proton_line_get_dose(const ProtonDose *dose, double depth)
{
    long i;
    depth = (depth - proton_dose_min_depth(dose)) / dose->px_spacing[1];
    {
        const double x = floor(depth);
        depth -= x;
        i = STATIC_CAST(long, x);
    }
    return dose->linedose[i] + (dose->linedose[i + 1] - dose->linedose[i]) * depth;
}

static void proton_dose_find_square(const ProtonDose *dose, long a[_q(static 2)],
                                    double x, double y, float r[_q(static 2)])
{
    const double tmp[2] = {
        floor(x = (x - dose->top_left[0]) / dose->px_spacing[0]),
        floor(y = (y - dose->top_left[2]) / dose->px_spacing[2])
    };
    r[0] = STATIC_CAST(float, x - tmp[0]);
    r[1] = STATIC_CAST(float, y - tmp[1]);
    a[0] = STATIC_CAST(long, tmp[0]);
    a[1] = STATIC_CAST(long, tmp[1]);
}

static bool proton_dose_square_out_of_bounds(const ProtonDose *dose,
                                             const long a[_q(static 2)])
{
    const bool xbnd = (a[0] < 0) || (a[0] >= (dose->px_dimensions[0] - 1));
    const bool ybnd = (a[1] < 0) || (a[1] >= (dose->px_dimensions[2] - 1));
    return xbnd || ybnd;
}

static float proton_dose_interp_eval(const float interp[_q(static 4)],
                                     const float x[_q(static 2)])
{
    return fmaf(x[1],
        fmaf(x[0], interp[3], interp[2]),
        fmaf(x[0], interp[1], interp[0]));
}

static float proton_dose_interpolate_square(const float *dptr, const long axskip,
                                            const float r[_q(static 2)])
{
    float interp[4] = { dptr[0], dptr[1], dptr[axskip], dptr[axskip + 1] };
    interp[1] -= interp[0];
    interp[3] -= interp[1] + interp[2];
    interp[2] -= interp[0];
    return proton_dose_interp_eval(interp, r);
}

void proton_dose_get_line(ProtonDose *dose, double x, double y)
{
    long a[2];
    float r[2];
    proton_dose_find_square(dose, a, x, y, r);
    if (proton_dose_square_out_of_bounds(dose, a)) {
        memset(dose->linedose, 0, sizeof *dose->linedose * dose->nplanes);
    } else {
        const unsigned long axskip = dose->px_dimensions[0] * dose->px_dimensions[1];
        float *lptr, *const lend = dose->linedose + dose->nplanes;
        const float *dptr = dose->data + a[1] * axskip + a[0];
        for (lptr = dose->linedose; lptr < lend; lptr++) {
            *lptr = proton_dose_interpolate_square(dptr, axskip, r);
            dptr += dose->px_dimensions[0];
        }
    }
}

const float *proton_line_raw(const ProtonDose *dose)
{
    return dose->linedose;
}

long proton_line_length(const ProtonDose *dose)
{
    return dose->nplanes;
}


/* ---------------------------------------------------------------------- */
/*                              Dose structure                            */
/* ---------------------------------------------------------------------- */


static ProtonDose *proton_dose_flexible_alloc(const long N)
{
    ProtonDose *dose = malloc(sizeof *dose + sizeof *dose->data * N);
    return dose;
}

/** Allocates the structure and initializes all components derived directly
 *  from the DICOM */
static ProtonDose *proton_dose_init(RTDose *dcm)
{
    ProtonDose *dose;
    long dim[3];
    if (rtdose_get_dimensions(dcm, dim)) {
        return NULL;
    }
    dose = proton_dose_flexible_alloc(dim[0] * dim[1] * dim[2]);
    if (!dose) {
        return NULL;
    }
    memcpy(dose->px_dimensions, dim, sizeof dim);
    if (rtdose_get_img_pos_pt(dcm, dose->top_left)) {
        free(dose);
        return NULL;
    }
    if (rtdose_get_px_spacing(dcm, dose->px_spacing)) {
        free(dose);
        return NULL;
    }
    if (rtdose_get_dose_data(dcm, dose->px_dimensions, dose->data, &dose->dmax)) {
        free(dose);
        return NULL;
    }
    return dose;
}

ProtonDose *proton_dose_create(const char *filename)
{
    ProtonDose *dose;
    RTDose *dcm = rtdose_create(filename);
    if (!dcm) {
        return NULL;
    }
    dose = proton_dose_init(dcm);
    rtdose_destroy(dcm);
    if (!dose) {
        return NULL;
    }
    dose->linedose = NULL;
    proton_planes_create(dose);
    if (!dose->planes) {
        proton_dose_destroy(dose);
        return NULL;
    }
    /** Allocate one extra point, set it to zero, and don't touch it. This
     *  avoids the need for special fencepost code in the interpolator */
    dose->linedose = malloc(sizeof *dose->linedose * (dose->nplanes + 1));
    if (!dose->linedose) {
        proton_dose_destroy(dose);
        return NULL;
    }
    dose->linedose[dose->nplanes] = 0.0f;
    return dose;
}

void proton_dose_destroy(ProtonDose *dose)
{
    if (dose) {
        free(dose->planes);
        free(dose->linedose);
        free(dose);
    }
}

double proton_dose_origin(const ProtonDose *dose, int dim)
{
    return dose->top_left[dim];
}

double proton_dose_spacing(const ProtonDose *dose, int dim)
{
    return dose->px_spacing[dim];
}

long proton_dose_dimension(const ProtonDose *dose, int dim)
{
    return dose->px_dimensions[dim];
}

double proton_dose_width(const ProtonDose *dose, int dim)
{
    return STATIC_CAST(double, dose->px_dimensions[dim]) * dose->px_spacing[dim];
}

float proton_dose_max(const ProtonDose *dose)
{
    return dose->dmax;
}

void proton_dose_depth_range(const ProtonDose *dose, float range[_q(static 2)])
{
    range[0] = STATIC_CAST(float, proton_dose_min_depth(dose));
    range[1] = STATIC_CAST(float, proton_dose_max_depth(dose));
}

float proton_dose_coronal_aspect(const ProtonDose *dose)
{
    return STATIC_CAST(float, dose->px_dimensions[0]) / STATIC_CAST(float, dose->px_dimensions[2]);
}


/* ---------------------------------------------------------------------- */
/*                                 Image                                  */
/* ---------------------------------------------------------------------- */


struct _proton_image {
    long dim[2];
    long bufwidth;
    unsigned char buf[];
};

static ProtonImage *proton_image_flexible_alloc(long N)
{
    struct _proton_image *img;
    /* No point in calloc(), since the resize will always be handled by 
    realloc() */
    img = malloc(sizeof *img + sizeof *img->buf * N);
    if (img) {
        img->bufwidth = N;
    }
    return img;
}

bool proton_image_realloc(ProtonImage **img, long width, long height)
{
    const long N = 3UL * width * height;
    const long N_old = (*img) ? (*img)->bufwidth : 0;
    if (N > N_old) {
        free(*img);
        *img = proton_image_flexible_alloc((3 * N) / 2);
        if (!*img) {
            return true;
        }
    }
    (*img)->dim[0] = width;
    (*img)->dim[1] = height;
    return false;
}

void proton_image_destroy(ProtonImage *img)
{
    free(img);
}

long proton_image_dimension(const ProtonImage *img, int dim)
{
    return img->dim[dim];
}

unsigned char *proton_image_raw(ProtonImage *img)
{
    return img->buf;
}

bool proton_image_empty(const ProtonImage *img)
{
    return (img->dim[0] == 0) || (img->dim[1] == 0);
}

static void proton_dose_load_interpolant(float interp[_q(static 4)],
                                         const float *dline,
                                         const long yskip,
                                         const long zskip,
                                         const float z,
                                         const float dmax)
{
    interp[0] = dline[0];
    interp[1] = dline[1];
    interp[2] = dline[zskip];
    interp[3] = dline[zskip + 1];

    dline += yskip;
    interp[0] = fmaf(dline[0] - interp[0], z, interp[0]);
    interp[1] = fmaf(dline[1] - interp[1], z, interp[1]);
    interp[2] = fmaf(dline[zskip] - interp[2], z, interp[2]);
    interp[3] = fmaf(dline[zskip + 1] - interp[3], z, interp[3]);

    interp[1] -= interp[0];
    interp[3] -= interp[2] + interp[1];
    interp[2] -= interp[0];

    interp[0] /= dmax;
    interp[1] /= dmax;
    interp[2] /= dmax;
    interp[3] /= dmax;
}

static void proton_dose_sublattice_clamp(long b[_q(restrict static 2)],
                                         const long blim[_q(static 2)],
                                         const long a[_q(static 2)],
                                         const long alim[_q(static 2)])
{
    b[0] = IDIVCEIL(blim[0] * a[0], alim[0]);
    b[1] = IDIVCEIL(blim[1] * a[1], alim[1]);
}

static void proton_dose_fma(float a[_q(restrict static 2)],
                            const float b[_q(static 2)],
                            const float c[_q(static 2)])
{
    a[0] = fmaf(a[0], b[0], c[0]);
    a[1] = fmaf(a[1], b[1], c[1]);
}

static void proton_dose_fcast(float dst[_q(restrict static 2)],
                              const long src[_q(static 2)])
{
    dst[0] = STATIC_CAST(float, src[0]);
    dst[1] = STATIC_CAST(float, src[1]);
}

/** REWRITEME: Add more function calls, this context's alphabet is a little 
 *  too big:
 *      - b0: Could be fused into a row-writing function
 *      - px: Could be calculated and bound to a function's argument 
 */
static void proton_dose_interpolate_cell(const float interp[_q(static 4)],
                                         const long a[_q(static 2)],
                                         const long alim[_q(static 2)],
                                         const long blim[_q(static 2)],
                                         unsigned char *buf,
                                         void (*cmap)(float, unsigned char *))
{
    const float tfm[2] = {
        STATIC_CAST(float, alim[0]) / STATIC_CAST(float, blim[0]),
        STATIC_CAST(float, alim[1]) / STATIC_CAST(float, blim[1]) };
    const float tlt[2] = {
        -STATIC_CAST(float, a[0]), -STATIC_CAST(float, a[1]) };
    const long bskip = blim[0] + 1;
    long b[2], b0;
    float x[2];
    unsigned char *px;
    proton_dose_sublattice_clamp(b, blim, a, alim);
    b0 = b[0];
    px = buf + 3 * (b[1] * bskip + b[0]);
    while (1) {
        proton_dose_fcast(x, b);
        proton_dose_fma(x, tfm, tlt);
        if (x[1] > 1.0f) {
            break;
        } else if (x[0] > 1.0f) {
            b[0] = b0;
            b[1]++;
            px = buf + 3 * (b[1] * bskip + b[0]);
        } else {
            cmap(proton_dose_interp_eval(interp, x), px);
            b[0]++;
            px += 3;
        }
    }
}

/** Given a slice depth in @p z, find the the scan with the greatest z 
 *  coordinate not greater than @p z */
static void proton_dose_find_scan(const ProtonDose *dose, float *z,
                                  const float *restrict *dline)
{
    float flz;
    long idx;
    *z -= STATIC_CAST(float, proton_dose_min_depth(dose));
    *z /= STATIC_CAST(float, dose->px_spacing[1]);
    flz = floorf(*z);
    idx = STATIC_CAST(long, flz);
    *dline = dose->data + idx * dose->px_dimensions[0];
    *z -= flz;
}

/** REWRITEME: Not urgent, mul and imul are at most 3 µops on the target 
 *  machines. Most of the affine transformations are completed with  
 *  fma and lea instructions anyway
 */
void proton_dose_get_plane(const ProtonDose *dose,
                           ProtonImage *img, float depth,
                           void (*colormap)(float, unsigned char *))
{
    const long axskip = dose->px_dimensions[0] * dose->px_dimensions[1];
    const long alim[2] = { dose->px_dimensions[0] - 1, dose->px_dimensions[2] - 1 };
    const long blim[2] = { proton_image_dimension(img, 0) - 1, proton_image_dimension(img, 1) - 1 };
    unsigned char *buf = proton_image_raw(img);
    const float *dptr;
    float interp[4];
    long a[2];
    if (!proton_image_empty(img)) {
        proton_dose_find_scan(dose, &depth, &dptr);
        for (a[1] = 0; a[1] < alim[1]; a[1]++) {
            const float *const dline = dptr;
            for (a[0] = 0; a[0] < alim[0]; a[0]++, dptr++) {
                proton_dose_load_interpolant(interp, dptr, dose->px_dimensions[0],
                                             axskip, depth, dose->dmax);
                proton_dose_interpolate_cell(interp, a, alim, blim, buf, colormap);
            }
            dptr = dline + axskip;
        }
    }
}

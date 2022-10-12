#pragma once
/** This library could potentially be improved by inlining some of the 
 *  trivial calls. Since the objects use C99 flexible arrays, this would 
 *  require some type punning in the header, with any accesses to the flex 
 *  array done from the source file
 *  TODO: Determine how many scopes make only trivial calls to this lib
 *        I am not doing a drastic rewrite to make a handful of functions 
 *        leaf calls
 */
#ifndef PROTON_DOSE_H
#define PROTON_DOSE_H

#if __cplusplus
extern "C" {
#else
#   include <stdbool.h>
#endif


/** Directions are normal to their respective planes (i.e. sagittal 
 *  in the x direction) */
enum {
    DOSE_LR = 0,    /* Sagittal direction */
    DOSE_AP = 1,    /* Coronal direction */
    DOSE_SI = 2     /* Axial direction */
};

/** Typedef'd because I want to use a C99 flexible array member */
typedef struct _proton_dose ProtonDose;

ProtonDose *proton_dose_create(const char *filename);
void proton_dose_destroy(ProtonDose *dose);

double proton_dose_origin(const ProtonDose *dose, int dim);
double proton_dose_spacing(const ProtonDose *dose, int dim);
long proton_dose_dimension(const ProtonDose *dose, int dim);

/** Distance between the bounding PIXELS */
double proton_dose_width(const ProtonDose *dose, int dim);

float proton_dose_max(const ProtonDose *dose);

/** Writes the valid depth range [d0, d1) to the first two floats at @p range */
void proton_dose_depth_range(const ProtonDose *dose, float range[]);

float proton_dose_coronal_aspect(const ProtonDose *dose);

/** Interpolates the value of the line dose at @p depth from the currently
 *  held line dose array */
double proton_line_get_dose(const ProtonDose *dose, double depth);

/** Interpolates the line dose at (x, y) onto the linedose array in @c dose */
void proton_dose_get_line(ProtonDose *dose, double x, double y);

const float *proton_line_raw(const ProtonDose *dose, long *n);
const float *proton_planes_raw(const ProtonDose *dose, long *n);

/** WARNING: This value is NOT stored in the dose struct, so this function
 *  COMPUTES the result! */
float proton_planes_max(const ProtonDose *dose);

double proton_planes_get_dose(const ProtonDose *dose, double depth);

long proton_line_length(const ProtonDose *dose);

/** TODO: Add colormap to the image structure? This would make it easier 
 *  to swap them at runtime */

typedef struct _proton_image ProtonImage;

bool proton_image_realloc(ProtonImage **img, long width, long height);
void proton_image_destroy(ProtonImage *img);

long proton_image_dimension(const ProtonImage *img, int dim);
unsigned char *proton_image_raw(ProtonImage *img);

bool proton_image_empty(const ProtonImage *img);

/** Interpolates the dose grid onto the 2D buffer at @p img */
void proton_dose_get_plane(const ProtonDose *dose,
                           ProtonImage *img, float depth,
                           void (*colormap)(float, unsigned char *));


#if __cplusplus
}
#endif

#endif /* PROTON_DOSE_H */

#pragma once

#ifndef PROTON_DOSE_H
#define PROTON_DOSE_H

#if __cplusplus
extern "C" {
#endif

#include "proton-image.h"


/** Typedef'd because I want to use a C99 flexible array member
 *  I don't want to use the singleton array hack
 */
typedef struct _proton_dose/* {
    double top_left[3];
    double px_spacing[3];
    long px_dimensions[3];
    float dmax;
    float data[];
} */ProtonDose;

ProtonDose *proton_dose_create(const char *filename);
void proton_dose_destroy(ProtonDose *dose);

double proton_dose_origin(const ProtonDose *dose, int dim);
double proton_dose_spacing(const ProtonDose *dose, int dim);
long proton_dose_dimension(const ProtonDose *dose, int dim);

double proton_dose_width(const ProtonDose *dose, int dim);

float proton_dose_max(const ProtonDose *dose);
double proton_dose_max_depth(const ProtonDose *dose);
float proton_dose_coronal_aspect(const ProtonDose *dose);

/** Interpolates the dose grid onto the 2D buffer at @p img
 * 
 *  I could (should) perhaps write a version of this that interpolates 
 *  a plane of arbitrary physical dimension and orientation, but that 
 *  may not be fast enough for real-time display (unless I use SSE?)
 *                                       I should do ^^ this ^^ anyway
 */
void proton_dose_get_plane(const ProtonDose *dose,
                           ProtonImage *img, float depth,
                           void (*colormap)(float, unsigned char *));


typedef struct _proton_line/* {
    double depth;
    long npts;
    float dose[];
} */ProtonLine;

ProtonLine *proton_line_create(const ProtonDose *dose, double depth);
void proton_line_destroy(ProtonLine *line);

long proton_line_length(const ProtonLine *line);
const float *proton_line_raw(const ProtonLine *line);

/** Interpolates the line dose from the top to maxdepth */
void proton_dose_get_line(const ProtonDose *dose, ProtonLine *line,
                          double x, double y);


#if __cplusplus
}
#endif

#endif /* PROTON_DOSE_H */

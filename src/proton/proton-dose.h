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

/** Typedef'd because I want to use a C99 flexible array member
 *  I don't want to use the singleton array hack */
typedef struct _proton_dose/* {
    double top_left[3];
    double px_spacing[3];
    long px_dimensions[3];
    float dmax, depmax;
    float data[];
} */ProtonDose;

ProtonDose *proton_dose_create(const char *filename);
void proton_dose_destroy(ProtonDose *dose);

double proton_dose_origin(const ProtonDose *dose, int dim);
double proton_dose_spacing(const ProtonDose *dose, int dim);
long proton_dose_dimension(const ProtonDose *dose, int dim);

/** Distance between the bounding PIXELS */
double proton_dose_width(const ProtonDose *dose, int dim);

float proton_dose_max(const ProtonDose *dose);

/** DELETED: Replaced with a single call to get the entire range at once
 *  Rewritten: Returns the slice depth of the last indexed plane with 
 *  nonzero integrated dose. It is extremely likely that this depth will 
 *  not be integral */
/* double proton_dose_max_depth(const ProtonDose *dose); */

/** New: Returns 0.5 * dose->px_spacing[1] */
/* double proton_dose_min_depth(const ProtonDose *dose); */

/** Writes the valid depth range [d0, d1) to the first two floats at range */
void proton_dose_depth_range(const ProtonDose *dose, float range[]);

float proton_dose_coronal_aspect(const ProtonDose *dose);


/** TODO: Add colormap to the image structure? This would make it easier 
 *  to swap them at runtime */

typedef struct _proton_image/* {
    long dim[2];
    long bufwidth;
    unsigned char buf[];
} */ProtonImage;

bool proton_image_realloc(ProtonImage **img, long width, long height);
void proton_image_destroy(ProtonImage *img);

long proton_image_dimension(const ProtonImage *img, int dim);
unsigned char *proton_image_raw(ProtonImage *img);

bool proton_image_empty(const ProtonImage *img);

/** Interpolates the dose grid onto the 2D buffer at @p img
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

double proton_line_get_dose(const ProtonLine *line, double depth);

/** Interpolates the line dose from the top to maxdepth */
void proton_dose_get_line(ProtonLine *line, double x, double y);


#if __cplusplus
}
#endif

#endif /* PROTON_DOSE_H */

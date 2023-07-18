#pragma once

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


typedef struct _proton_dose {
    /* Everything in this section must be extracted from the DICOM */
    double top_left[3];
    double px_spacing[3];
    long px_dimensions[3];

    /* Everything in this section (except data) is derived from the above */
    long nplanes;
    float *planes, *stppwr, *linedose;
    float dmax;

    /* Gradient field in y */
    float *grad;
#if !defined(__cplusplus) || !__cplusplus
    float data[];
#endif /* C ONLY */
} ProtonDose;


ProtonDose *proton_dose_create(const char *filename, size_t ebufsz, char err[]);
void proton_dose_destroy(ProtonDose *dose);

inline double proton_dose_origin(const ProtonDose *dose, int dim) { return dose->top_left[dim]; }
inline double proton_dose_spacing(const ProtonDose *dose, int dim) { return dose->px_spacing[dim]; }
inline long proton_dose_dimension(const ProtonDose *dose, int dim) { return dose->px_dimensions[dim]; }

/** Distance between the bounding PIXELS */
double proton_dose_width(const ProtonDose *dose, int dim);

inline float proton_dose_max(const ProtonDose *dose) { return dose->dmax; }

/** Writes the valid depth range [d0, d1) to the first two floats at @p range */
void proton_dose_depth_range(const ProtonDose *dose, float range[]);

float proton_dose_coronal_aspect(const ProtonDose *dose);

/** Interpolates the value of the line dose at @p depth from the currently
 *  held line dose array
 */
double proton_line_get_dose(const ProtonDose *dose, double depth);

/** Interpolates the line dose at (x, y) onto the linedose array in @c dose */
void proton_dose_get_line(ProtonDose *dose, double x, double y);

inline const float *proton_line_raw(const ProtonDose *dose) { return dose->linedose; }
inline const float *proton_planes_raw(const ProtonDose *dose) { return dose->planes; }
inline const float *proton_stppwr_raw(const ProtonDose *dose) { return dose->stppwr; }

/** @warning This value is NOT stored in the dose struct, so this function
 *      COMPUTES the result!
 */
float proton_planes_max(const ProtonDose *dose);
float proton_stppwr_max(const ProtonDose *dose);

double proton_planes_get_dose(const ProtonDose *dose, double depth);
double proton_stppwr_get_dose(const ProtonDose *dose, double depth);

inline long proton_line_length(const ProtonDose *dose) { return dose->nplanes; }


typedef struct _proton_image {
    long dim[2];
    long bufwidth;
    
#if !defined(__cplusplus) || !__cplusplus
    unsigned char buf[];
#endif /* C ONLY */
} ProtonImage;


bool proton_image_realloc(ProtonImage **img, long width, long height);
void proton_image_destroy(ProtonImage *img);

inline long proton_image_dimension(const ProtonImage *img, int dim) { return img->dim[dim]; }
unsigned char *proton_image_raw(ProtonImage *img);

inline bool proton_image_empty(const ProtonImage *img) { return img->dim[0] == 0 || img->dim[1] == 0; }


/** I guess we just fetch this from the visualizer control
 */
typedef struct _proton_plane_params {
    enum {
        PROTON_IMG_DOSE,
        PROTON_IMG_GRAD
    } type;
    void (*colormap)(float, unsigned char *);
    float pct_diff;
    float depth_err;
} ProtonPlaneParams;

/** Interpolates the dose grid onto the 2D buffer at @p img */
void proton_dose_get_plane(const ProtonDose        *dose,
                           const ProtonPlaneParams *params,
                           ProtonImage             *img,
                           float                    depth);


#if __cplusplus
}
#endif

#endif /* PROTON_DOSE_H */

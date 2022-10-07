#pragma once
/** Extremely thin wrapper for DCMTK **/
#ifndef DCM_LOAD_H
#define DCM_LOAD_H

#if __cplusplus
extern "C" {
#else
#   include <stdbool.h>
#endif


typedef struct _dicom_rtdose RTDose;


RTDose *rtdose_create(const char *filename);
void rtdose_destroy(RTDose *dcm);

/* None of these do any NULL checking */
bool rtdose_get_img_pos_pt(const RTDose *dcm, double imgpos[]);
bool rtdose_get_px_spacing(const RTDose *dcm, double spacing[]);
bool rtdose_get_dimensions(const RTDose *dcm, long dim[]);

/* Buffer must be preallocated */
bool rtdose_get_dose_data(RTDose *dcm, const long dim[], float *dptr, float *dmax);


#if __cplusplus
}
#endif

#endif /* DCM_LOAD_H */

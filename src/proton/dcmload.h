#pragma once
/** Extremely thin wrapper for DCMTK **/
#ifndef DCM_LOAD_H
#define DCM_LOAD_H

#if __cplusplus
extern "C" {
#endif


typedef struct _dicom_rtdose RTDose;


RTDose *rtdose_create(const char *filename);
void rtdose_destroy(RTDose *dcm);

/* None of these do any NULL checking */
int rtdose_get_img_pos_pt(const RTDose *dcm, double imgpos[]);
int rtdose_get_px_spacing(const RTDose *dcm, double spacing[]);
int rtdose_get_dimensions(const RTDose *dcm, long dim[]);

/* Buffer must be preallocated */
int rtdose_get_dose_data(RTDose *dcm, const long dim[], float *dptr, float *dmax);


#if __cplusplus
}
#endif

#endif /* DCM_LOAD_H */

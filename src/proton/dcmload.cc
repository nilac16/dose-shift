#include "dcmload.h"
#include <dcmtk/dcmrt/drmdose.h>


struct _dicom_rtdose {
    DRTDose dcm;
};


RTDose *rtdose_create(const char *filename)
{
    RTDose *dose;
    dose = reinterpret_cast<RTDose *>(new (std::nothrow) DRTDose);
    if (dose) {
        OFCondition stat = dose->dcm.loadFile(filename);
        if (stat.bad()) {
            rtdose_destroy(dose);
            dose = NULL;
        }
    }
    return dose;
}

void rtdose_destroy(RTDose *dcm)
{
    delete reinterpret_cast<DRTDose *>(dcm);
}

bool rtdose_get_img_pos_pt(const RTDose *dcm, double imgpos[])
{
    Float64 x;
    for (unsigned long i = 0; i < 3; i++, imgpos++) {
        OFCondition stat = dcm->dcm.getImagePositionPatient(x, i);
        if (stat.bad()) {
            return true;
        }
        *imgpos = static_cast<double>(x);
    }
    return false;
}

bool rtdose_get_px_spacing(const RTDose *dcm, double spacing[])
{
    Float64 x;
    OFCondition stat = dcm->dcm.getPixelSpacing(x, 0);
    if (stat.bad()) {
        return true;
    }
    spacing[0] = static_cast<double>(x);
    stat = dcm->dcm.getPixelSpacing(x, 1);
    if (stat.bad()) {
        return true;
    }
    spacing[1] = static_cast<double>(x);
    /* This may be wrong. I think the correct information is in the grid
    frame offset vector (3004, 000c). For now, all of our dicoms have
    uniform grid frame offsets */
    stat = dcm->dcm.getSliceThickness(x);
    if (stat.bad()) {
        return true;
    }
    spacing[2] = static_cast<double>(x);
    return false;
}

bool rtdose_get_dimensions(const RTDose *dcm, long dim[])
{
    OFCondition stat;
    Uint16 x;
    Sint32 y;
    stat = dcm->dcm.getColumns(x);
    if (stat.bad()) {
        return true;
    }
    dim[0] = static_cast<long>(x);
    stat = dcm->dcm.getRows(x);
    if (stat.bad()) {
        return true;
    }
    dim[1] = static_cast<long>(x);
    stat = dcm->dcm.getNumberOfFrames(y);
    if (stat.bad()) {
        return true;
    }
    dim[2] = static_cast<long>(y);
    return false;
}

bool rtdose_get_dose_data(RTDose *dcm, const long dim[], float *dptr, float *dmax)
{
    const long ax_N = dim[0] * dim[1];
    OFVector<Float64> plane;
    long k;
    *dmax = -HUGE_VAL;
    for (k = 0; k < dim[2]; k++) {
        OFVector<Float64>::iterator it;
        long N = ax_N;
        OFCondition stat = dcm->dcm.getDoseImage(plane, k);
        if (stat.bad()) {
            return true;
        }
        it = plane.begin();
        do {
            *dptr = static_cast<float>(*it);
            if (*dptr > *dmax) {
                *dmax = *dptr;
            }
            dptr++;
            ++it;
            N--;
        } while (N);
    }
    return false;
}

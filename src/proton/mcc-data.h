#pragma once

#ifndef MCC_DATA_H
#define MCC_DATA_H

#if __cplusplus
extern "C" {
#endif


typedef struct _mcc_data/*  {
    struct mcc_triangulation *triangulation;
    unsigned long nscans;
    struct {
        double scanpos;
        unsigned long scanlen;
        struct {
            double pos, dose;
        } data[];
    } *scans[];
}  */MCCData;

MCCData *mcc_data_create(const char *filename);
void mcc_data_destroy(MCCData *mcc);

double mcc_data_get_dose(const MCCData *mcc, double x, double y);


#if __cplusplus
}
#endif

#endif /* MCC_DATA_H */

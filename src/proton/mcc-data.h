#pragma once

#ifndef MCC_DATA_H
#define MCC_DATA_H

#if __cplusplus
extern "C" {
#endif


enum {
    MCC_ERROR_NONE = 0,
    MCC_ERROR_NOMEM,
    MCC_ERROR_FOPEN_FAILED,
    MCC_ERROR_MISMATCHED_DELIM,
    MCC_ERROR_MISSING_OFFAXIS,
    MCC_ERROR_MISSING_CROSSCAL,
    MCC_ERROR_MALFORMED_ATTRIBUTE,
    MCC_ERROR_UNCLASSIFIABLE_STATEMENT
};

const char *mcc_get_error(int err);

typedef struct _mcc_data MCCData;

MCCData *mcc_data_create(const char *filename, int *stat);
void mcc_data_destroy(MCCData *mcc);

double mcc_data_get_dose(const MCCData *mcc, double x, double y);


#if __cplusplus
}
#endif

#endif /* MCC_DATA_H */

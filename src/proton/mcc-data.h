#pragma once

#ifndef MCC_DATA_H
#define MCC_DATA_H

#if __cplusplus
extern "C" {
#endif


enum {
    MCC_ERROR_NONE = 0,

/** System errors */
    MCC_ERROR_NOMEM,                    /* Out of memory */
    MCC_ERROR_FOPEN_FAILED,             /* No such file or directory */

/** File errors */
    MCC_ERROR_MISMATCHED_DELIM,         /* Mismatched delimiters */
    MCC_ERROR_MISSING_OFFAXIS,          /* Missing scan coordinate */
    MCC_ERROR_MISSING_CROSSCAL,         /* Missing cross calibration */
    MCC_ERROR_MALFORMED_ATTRIBUTE,      /* Parser failure */
    MCC_ERROR_UNCLASSIFIABLE_STATEMENT  /* Lexer failure */
};


/** FYI: This returns NULL if err == MCC_ERROR_NONE -- Do NOT puts() this! */
const char *mcc_get_error(int err);


typedef struct _mcc_data MCCData;

MCCData *mcc_data_create(const char *filename, int *stat);
void mcc_data_destroy(MCCData *mcc);

double mcc_data_get_point_dose(const MCCData *mcc, double x, double y);
double mcc_data_get_sum(const MCCData *mcc);
long mcc_data_get_supp(const MCCData *mcc);


#if __cplusplus
}
#endif

#endif /* MCC_DATA_H */

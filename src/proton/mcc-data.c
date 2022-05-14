#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcc-data.h"
#include "mcc/mcc.h"

#if _MSC_VER
#   define _q(qualifiers)
#else
#   define _q(qualifiers) qualifiers
#endif



struct _mcc_data {
    struct mcc_triangulation {
        struct mcc_triangle *end;
        struct mcc_triangle {
            unsigned int i, j;
            struct {
                struct mcc_triangle *next;
                unsigned int rot;
            } edges[3];
        } tris[];
    } *triangulation;

    unsigned long nscans;
    struct mcc_scan_tag {
        double scanpos;
        unsigned long scanlen;
        struct mcc_scandata {
            double pos, dose;
        } data[];
    } *scans[];
};

static int mcc_data_triangulate(MCCData *data, unsigned long N)
{
    
}

static MCCData *mcc_data_load(mccfile *file)
{
    MCCData *data;
    const unsigned long nscans = get_n_mcc_scans(file);
    unsigned long i;
    /* Faulting in zero pages to guarantee that all scan pointers are NULL,
    just in case this fails and must be destroyed */
    data = calloc(1, sizeof *data + sizeof *data->scans * nscans);
    if (data) {
        data->nscans = nscans;
        for (i = 0; i < nscans; i++) {
            const mccscan *scan = get_scan(file, i);
            double *x, *y, crosscal;
            long *idx, j;
            const ndata = get_scan_data(scan, &x, &y, &idx);
            data->scans[i] = malloc(sizeof *data->scans[i] + sizeof *data->scans[i]->data * ndata);
            if (!data->scans[i]) {
                goto mcc_data_load_loop_error;
            }
            if (get_scan_double(scan, "CROSS_CALIBRATION", &crosscal)) {
                goto mcc_data_load_loop_error;
            }
            if (get_scan_double(scan, "SCAN_OFFAXIS_INPLANE", &data->scans[i]->scanpos)) {
                goto mcc_data_load_loop_error;
            }
            for (j = 0; j < ndata; j++) {
                data->scans[i]->data[j].pos = x[j];
                data->scans[i]->data[j].dose = y[j] * crosscal;
            }
            continue;
mcc_data_load_loop_error:
            mcc_data_destroy(data);
            data = NULL;
            break;
        }
    }
    return data;
}

static unsigned long mcc_data_nodecount(const MCCData *data)
{
    unsigned long N = 0;
    const struct mcc_scan_tag *scan, *const last = data->scans + data->nscans;
    for (scan = data->scans; scan < last; scan++) {
        N += scan->scanlen;
    }
    return N;
}

MCCData *mcc_data_create(const char *filename)
{
    mccfile *mcc = open_mcc_file(filename);
    MCCData *data = NULL;
    if (mcc) {
        data = mcc_data_load(mcc);
        close_mcc_file(mcc);
        if (data) {
            unsigned long N = mcc_data_nodecount(data);
            mcc_data_triangulate(data, N);
            if (!data->triangulation) {
                mcc_data_destroy(data);
                data = NULL;
            }
        }
    }
    return data;
}

void mcc_data_destroy(MCCData *mcc)
{
    unsigned long i;
    for (i = 0; i < mcc->nscans; i++) {
        free(mcc->scans[i]);
    }
    free(mcc);
}

static void mcc_data_find_triangle(const MCCData *mcc, double tri[][],
                                   double x, double y)
{
    
}

double mcc_data_get_dose(const MCCData *mcc, double x, double y)
{
    double tri[3][3], area[4];
    mcc_data_find_triangle(mcc, tri, x, y);
}

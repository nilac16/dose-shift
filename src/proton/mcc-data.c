#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcc-data.h"

#if _MSC_VER
#   define _q(qualifiers)
typedef int (*__compar_fn_t)(const void *, const void *);
#else
#   define _q(qualifiers) qualifiers
#endif

#define INIT_SCANCAP 64
#define INIT_DATACAP 128
#define LINEBUFSZ 512

#define EVQ 1.602176634e-19

/** Backus-Naur rules
 *  <file>          ::= "BEGIN_SCAN_DATA" <file-contents> "END_SCAN_DATA"
 *  <file-contents> ::= <kv-list> <scan-list> [<kv-list>?]
 *
 *  <kv-list> ::= [<kv-pair>] <kv-list>
 *  <kv-pair> ::= <key> "=" <value>
 *
 *  <scan-list>  ::= [<scan>] <scan-list>
 *
 *  <scan>       ::= <scan-begin> <scan-contents> <scan-end>
 *  <scan-begin> ::= "BEGIN_SCAN" <scan-index>
 *  <scan-end>   ::= "END_SCAN" <scan-index>
 *
 *  <scan-contents> ::= <kv-list> <data> [<kv-list>?]
 *
 *  <data>          ::= "BEGIN_DATA" <data-contents> "END_DATA"
 *  <data-contents> ::= <data-pos> <data-dose> <data-idx>
 *  <data-idx>     ::= "#" <datum-idx>
 */


const char *mcc_get_error(int err)
{
    static const char *errstrings[] = {
        NULL,
        "Allocation failure: Out of memory",
        "File failed to open: No such file or directory",
        "Mismatched delimiter sequence",
        "Scan is missing OFFAXIS_INPLANE attribute",
        "Missing cross calibration",
        "Attribute value is malformed/cannot be parsed",
        "Unclassifiable statement",
    };
    static const int nstrings = sizeof errstrings / sizeof *errstrings;

    return (err > 0 && err < nstrings) ? errstrings[err] : NULL;
}


static struct mcc_scan *mcc_scan_alloc(double y, unsigned initsz, jmp_buf env)
{
    struct mcc_scan *scan;

    scan = malloc(sizeof *scan + sizeof *scan->data * initsz);
    if (scan) {
        scan->sz = 0;
        scan->_cap = initsz;
        scan->y = y;
    } else {
        longjmp(env, MCC_ERROR_NOMEM);
    }
    return scan;
}


static void mcc_scan_push_back(struct mcc_scan **scan, double x, double dose, jmp_buf env)
{
    unsigned newcap;
    void *newptr;

    if ((*scan)->sz == (*scan)->_cap) {
        newcap = (*scan)->_cap * 2;
        newptr = realloc(*scan, sizeof **scan + sizeof *(*scan)->data * newcap);
        if (newptr) {
            *scan = newptr;
            (*scan)->_cap = newcap;
        } else {
            longjmp(env, MCC_ERROR_NOMEM);
        }
    }
    (*scan)->data[(*scan)->sz].x = x;
    (*scan)->data[(*scan)->sz].dose = dose;
    (*scan)->sz++;
}


static void mcc_data_push_scan(MCCData **data, double y, jmp_buf env)
{
    unsigned newcap;
    void *newptr;

    if ((*data)->sz == (*data)->_cap) {
        newcap = (*data)->_cap * 2;
        newptr = realloc(*data, sizeof **data + sizeof *(*data)->scans);
        if (newptr) {
            *data = newptr;
            (*data)->_cap = newcap;
        } else {
            longjmp(env, MCC_ERROR_NOMEM);
        }
    }
    (*data)->scans[(*data)->sz] = mcc_scan_alloc(y, INIT_DATACAP, env);
    (*data)->sz++;
}


struct mcc_stmt {
    enum {
        MCC_STMT_DELIM,
        MCC_STMT_KEYVAL,
        MCC_STMT_DATA,
        MCC_STMT_INVALID
    } type;
    union {
        enum {
            MCC_DLIM_SCANOPEN  = 0,
            MCC_DLIM_DATAOPEN  = 1,
            MCC_DLIM_SCANCLOSE = 2,
            MCC_DLIM_DATACLOSE = 3,
            MCC_DLIM_FILEOPEN  = 4,
            MCC_DLIM_FILECLOSE = 5
        } delim;
        struct {
            char *key, *val;
        } keyval;
        struct {
            double pos, dose;
        } data;
    } u;
};

enum mcc_scope {
    SCOPE_OUT_OF_FILE = 0,
    SCOPE_FILE        = 1,
    SCOPE_SCAN        = 2,
    SCOPE_DATA        = 3
};

struct mcc_parse_context {
    enum mcc_scope scope;
    struct mcc_stmt stmt;
    double offaxis, crosscal;
    bool offaxis_fnd:   1;
    bool crosscal_fnd:  1;
};

static const char *delims[] = {
    "BEGIN_SCAN",
    "BEGIN_DATA",
    "END_SCAN",
    "END_DATA",
    "BEGIN_SCAN_DATA",
    "END_SCAN_DATA"
};


/** Reimplementation of strtok that delimits on all whitespace, as
 *  determined by isspace(3)
 *  If @p s is at end-of-string, BOTH @p s and @p endptr are set to NULL!
 *  \param s      Pointer to pointer to start of string. Set to NULL at
 *      end of string ONLY
 *  \param endptr Pointer to char * that will be set to the end of the
 *      token. Set to NULL at end of string ONLY
 */
static void strtok_ws(const char **s, const char **endptr)
{
    const char *tok = *s;

    while (isspace(*tok)) {
        tok++;
    }
    if (*tok) {
        *s = tok;
        while (*tok && !isspace(*tok)) {
            tok++;
        }
        *endptr = tok;
    } else {
        *s = NULL;
        *endptr = NULL;
    }
}


/** Compares the range [s1, s2) to the C string at sptr, returning one if
 *  and only if they are equivalent within the range. The string pointed to
 *  by sptr must contain a nul terminator by the end of the range */
static bool strcmprng(const char *s1, const char *const s2, const char *sptr)
{
    while ((s1 < s2) && *sptr) {
        if (*s1 == *sptr) {
            s1++;
            sptr++;
        } else {
            return false;
        }
    }
    return s1 == s2 && *sptr == '\0';
}


/** Returns a pointer to the first non-whitespace character from the start
 *  of the string */
static char *strltrim(char *s)
{
    while (isspace(*s)) {
        s++;
    }
    return s;
}


/** Replaces the line break character with a nul terminator */
static void strlnterm(char *s)
{
    while (*s && *s != '\n') {
        s++;
    }
    *s = '\0';
}


static bool mcc_data_classify_assignment(char *s, struct mcc_stmt *stmt)
{
    char *eq;

    eq = strchr(s, '=');
    if (eq) {
        stmt->u.keyval.key = s;
        *eq = '\0';
        stmt->u.keyval.val = eq + 1;
        strlnterm(eq + 1);
        return true;
    } else {
        return false;
    }
}


static bool mcc_data_classify_data(const char *s, struct mcc_stmt *stmt)
{
    const char *s1 = s, *s2;
    char *endptr;

    strtok_ws(&s1, &s2);
    if (!s1) {
        return false;
    }
    stmt->u.data.pos = strtod(s1, &endptr);
    if (s2 != endptr) {
        return false;
    }
    s1 = s2;
    strtok_ws(&s1, &s2);
    if (!s1) {
        return false;
    }
    stmt->u.data.dose = strtod(s1, &endptr);
    return s2 == endptr;
}


static bool mcc_data_classify_delim(const char *s, struct mcc_stmt *stmt)
{
    const char *s1 = s, *s2;
    unsigned i;

    strtok_ws(&s1, &s2);
    if (s1) {
        for (i = 0; i < sizeof delims / sizeof *delims; i++) {
            if (strcmprng(s1, s2, delims[i])) {
                stmt->u.delim = i;
                return true;
            }
        }
    }
    return false;
}


static void mcc_data_classify_statement(char *s, struct mcc_stmt *stmt)
{
    if (mcc_data_classify_assignment(s, stmt)) {
        stmt->type = MCC_STMT_KEYVAL;
    } else if (mcc_data_classify_data(s, stmt)) {
        stmt->type = MCC_STMT_DATA;
    } else if (mcc_data_classify_delim(s, stmt)) {
        stmt->type = MCC_STMT_DELIM;
    } else {
        stmt->type = MCC_STMT_INVALID;
    }
}


/** Checks the current combination of scope and delimiter to determine if
 *  it is valid. Also verifies that the cross calibration and scan
 *  offaxis position are present if entering a data scope. If leaving a data
 *  scope, clears the cal and pos flags.
 *
 *  longjmp if: bad delimiter; missing calibration/offaxis position */
static void mcc_data_scope_check(struct mcc_parse_context *ctx, MCCData **data,
                                 jmp_buf env)
/** Valid combinations:
 *    - SCOPE_OUT_OF_FILE:             (0000 0000)   (0x00)
 *          FILEOPEN    ->  scope++     0000 0100     0x04  [++]
 *
 *    - SCOPE_FILE                     (0001 0000)   (0x10)
 *          SCANOPEN    ->  scope++     0001 0000     0x10  [++]
 *          FILECLOSE   ->  scope--     0001 0101     0x15  [--]
 *
 *    - SCOPE_SCAN                     (0010 0000)   (0x20)
 *          DATAOPEN    ->  scope++     0010 0001     0x21  [++] [pverify]
 *          SCANCLOSE   ->  scope--     0010 0010     0x22  [--]
 *
 *    - SCOPE_DATA                     (0011 0000)   (0x30)
 *          DATACLOSE   ->  scope--     0011 0011    (0x33) [--] [pclear]
 */
{
    const uint8_t combin = ((uint8_t)(ctx->scope) << 4) | (uint8_t)ctx->stmt.u.delim;
    int err;

    switch (combin) {
    case 0x21:
        if (ctx->offaxis_fnd && ctx->crosscal_fnd) {
            /* Push a new scan onto the scanvector */
            mcc_data_push_scan(data, ctx->offaxis, env);
        } else {
            err = (ctx->offaxis_fnd)
                ? MCC_ERROR_MISSING_CROSSCAL
                : MCC_ERROR_MISSING_OFFAXIS;
            longjmp(env, err);
        }
    case 0x04:
    case 0x10:
        ctx->scope++;
        break;

    case 0x33:
        ctx->offaxis_fnd = 0;
        ctx->crosscal_fnd = 0;
    case 0x15:
    case 0x22:
        ctx->scope--;
        break;

    default:
        longjmp(env, MCC_ERROR_MISMATCHED_DELIM);
    }
}


static double mcc_data_doubleconv(const char *nptr, jmp_buf env)
{
    char *endptr;
    double x;

    x = strtod(nptr, &endptr);
    if (nptr != endptr) {
        return x;
    } else {
        longjmp(env, MCC_ERROR_MALFORMED_ATTRIBUTE);
    }
}


static void mcc_data_keyval_check(struct mcc_parse_context *ctx, jmp_buf env)
{
    static const char *const offax_key = "SCAN_OFFAXIS_INPLANE";
    static const char *const crosscal_key = "CROSS_CALIBRATION";
    const char *const key = ctx->stmt.u.keyval.key;

    if (ctx->scope == SCOPE_SCAN) {
        if (!strcmp(key, offax_key)) {
            ctx->offaxis = mcc_data_doubleconv(ctx->stmt.u.keyval.val, env);
            ctx->offaxis_fnd = 1;
        } else if (!strcmp(key, crosscal_key)) {
            ctx->crosscal = mcc_data_doubleconv(ctx->stmt.u.keyval.val, env);
            ctx->crosscal_fnd = 1;
        }
    }
}


static void mcc_data_data_check(const struct mcc_parse_context *ctx,
                                struct mcc_scan **scan, jmp_buf env)
{
    const double dose = ctx->stmt.u.data.dose;

    mcc_scan_push_back(scan, ctx->stmt.u.data.pos, dose, env);
}


static void mcc_data_load_nodes(FILE *mcc, MCCData **data, jmp_buf env)
{
    struct mcc_parse_context ctx = {
        .scope = SCOPE_OUT_OF_FILE,
        .offaxis_fnd = 0,
        .crosscal_fnd = 0
    };
    char linebuf[LINEBUFSZ];

    while (fgets(linebuf, sizeof linebuf, mcc)) {
        mcc_data_classify_statement(strltrim(linebuf), &ctx.stmt);
        switch (ctx.stmt.type) {
        case MCC_STMT_DELIM:
            mcc_data_scope_check(&ctx, data, env);
            break;
        case MCC_STMT_KEYVAL:
            mcc_data_keyval_check(&ctx, env);
            break;
        case MCC_STMT_DATA:
            mcc_data_data_check(&ctx, (*data)->scans + (*data)->sz - 1, env);
            break;
        /* case MCC_STMT_INVALID: */
        default:
            longjmp(env, MCC_ERROR_UNCLASSIFIABLE_STATEMENT);
        }
    }
}


static struct mcc_scan *mcc_scan_trim(struct mcc_scan *scan, jmp_buf env)
{
    void *newptr;

    newptr = realloc(scan, sizeof *scan + sizeof *scan->data * scan->sz);
    if (newptr) {
        scan = newptr;
        scan->_cap = scan->sz;
    } else {
        longjmp(env, MCC_ERROR_NOMEM);
    }
    return scan;
}


static MCCData *mcc_data_trim(MCCData *data, jmp_buf env)
{
    void *newptr;
    unsigned i;

    for (i = 0; i < data->sz; i++) {
        data->scans[i] = mcc_scan_trim(data->scans[i], env);
    }
    newptr = realloc(data, sizeof *data + sizeof *data->scans * data->sz);
    if (newptr) {
        data = newptr;
        data->_cap = data->sz;
    } else {
        longjmp(env, MCC_ERROR_NOMEM);
    }
    return data;
}


static int mcc_data_scancmp(const struct mcc_scan **s1, const struct mcc_scan **s2)
{
    const double y1 = (*s1)->y, y2 = (*s2)->y;

    return (y1 > y2) - (y1 < y2);
}


static int mcc_data_datacmp(const double *x1, const double *x2)
{
    return (*x1 > *x2) - (*x1 < *x2);
}


static void mcc_data_sort(MCCData *data)
{
    unsigned i;

    qsort(data->scans, data->sz, sizeof *data->scans,
          (__compar_fn_t)mcc_data_scancmp);
    for (i = 0; i < data->sz; i++) {
        qsort(data->scans[i]->data, data->scans[i]->sz,
              sizeof *data->scans[i]->data, (__compar_fn_t)mcc_data_datacmp);
    }
}


static double maxf(double x, double y)
{
    return (x > y) ? x : y;
}


static double mcc_data_threshold(MCCData *data)
{
    double max = 0.0;
    unsigned i, j;

    for (j = 0; j < data->sz; j++) {
        for (i = 0; i < data->scans[j]->sz; i++) {
            max = maxf(max, data->scans[j]->data[i].dose);
        }
    }
    return 0.1 * max;
}


static void mcc_data_integrate(MCCData *data)
{
    double threshold;
    unsigned i, j;

    threshold = mcc_data_threshold(data);
    data->sum = 0.0;
    data->nsupp = 0;
    for (j = 0; j < data->sz; j++) {
        for (i = 0; i < data->scans[j]->sz; i++) {
            double dose = data->scans[j]->data[i].dose;
            data->sum += dose;
            data->nsupp += dose > threshold;
        }
    }
}


static MCCData *mcc_data_alloc(FILE *mcc, int *stat)
{
    MCCData *volatile data;
    jmp_buf env;

    data = calloc(1UL, sizeof *data + sizeof *data->scans * INIT_SCANCAP);
    if (!data) {
        *stat = MCC_ERROR_NOMEM;
        return NULL;
    }
    data->_cap = INIT_SCANCAP;
    if ((*stat = setjmp(env))) {
        free(data);
        return NULL;
    }
    mcc_data_load_nodes(mcc, (MCCData **)&data, env);
    data = mcc_data_trim(data, env);
    mcc_data_sort(data);
    mcc_data_integrate(data);
    return data;
}


MCCData *mcc_data_create(const char *filename, int *stat)
{
    MCCData *data = NULL;
    FILE *mfile;

    mfile = fopen(filename, "r");
    if (mfile) {
        data = mcc_data_alloc(mfile, stat);
    } else {
        *stat = MCC_ERROR_FOPEN_FAILED;
    }
    fclose(mfile);
    return data;
}


void mcc_data_destroy(MCCData *data)
{
    unsigned i;

    if (data) {
        for (i = 0; i < data->sz; i++) {
            free(data->scans[i]);
        }
        free(data);
    }
}


/** Returns the index of the scan with the LARGEST ordinate NOT GREATER
 *  than y */
static int mcc_data_scan_bsearch(const MCCData *data, const double y)
{
    int l = 0, r = data->sz, med;
    double y0;

    while (l < r) {
        med = (r + l) / 2;
        y0 = data->scans[med]->y;
        if (y < y0) {
            r = med;
        } else if (y0 < y) {
            l = med + 1;
        } else {
            return med;
        }
    }
    return l - 1;
}


static int mcc_data_data_bsearch(const struct mcc_scan *scan, const double x)
{
    int l = 0, r = scan->sz, med;
    double x0;

    while (l < r) {
        med = (r + l) / 2;
        x0 = scan->data[med].x;
        if (x < x0) {
            r = med;
        } else if (x0 < x) {
            l = med + 1;
        } else {
            return med;
        }
    }
    return l - 1;
}


#define SIG_NONCOMPACT -1.0

static double mcc_data_interp_scan(const struct mcc_scan *scan, double x)
{
    double d0, m, X;
    unsigned r;
    int l;

    l = mcc_data_data_bsearch(scan, x);
    r = l + 1;
    if (l >= 0 && r < scan->sz) {
        d0 = scan->data[l].dose;
        m = scan->data[r].dose - d0;
        X = (x - scan->data[l].x) / (scan->data[r].x - scan->data[l].x);
        return fma(m, X, d0);
    } else {
        return SIG_NONCOMPACT;
    }
}


static double mcc_data_interp_scans(const struct mcc_scan *scan1,
                                    const struct mcc_scan *scan2,
                                    double x, double y)
{
    const double interp[2] = {
        mcc_data_interp_scan(scan1, x),
        mcc_data_interp_scan(scan2, x)
    };
    double m, Y;

    if (interp[0] != SIG_NONCOMPACT && interp[1] != SIG_NONCOMPACT) {
        m = interp[1] - interp[0];
        Y = (y - scan1->y) / (scan2->y - scan1->y);
        return fma(m, Y, interp[0]);
    } else {
        return 0.0;
    }
}


double mcc_data_get_point_dose(const MCCData *data, double x, double y)
{
    unsigned r;
    int l;

    l = mcc_data_scan_bsearch(data, y);
    r = l + 1;
    if (l >= 0 && r < data->sz) {
        return mcc_data_interp_scans(data->scans[l], data->scans[r], x, y);
    } else {
        return 0.0;
    }
}

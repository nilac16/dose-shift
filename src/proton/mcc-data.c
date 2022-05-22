#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcc-data.h"
#include "delaunay/delaunay.h"

#if _MSC_VER
#   define _q(qualifiers)
#else
#   define _q(qualifiers) qualifiers
#endif

#define INITCAP 1024
#define LINEBUFSZ 512


struct _mcc_data {
    struct delaunay_triangle_pool *triangulation;
    struct delauany_r_tree *rtree;
    unsigned long size;
    double nodes[];
};

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
    int offaxis_fnd, crosscal_fnd;
    double offaxis, crosscal;
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
 *  determined by isspace(3) */
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
 *  by sptr must contain a nul terminator at the end of the range */
static int strcmprng(const char *s1, const char *const s2, const char *sptr)
{
    while ((s1 < s2) && *sptr) {
        if (*s1 == *sptr) {
            s1++;
            sptr++;
        } else {
            return 0;
        }
    }
    return (s1 == s2) && (*sptr == '\0');
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
    while (*s && !(*s == '\n')) {
        s++;
    }
    *s = '\0';
}

static int mcc_data_classify_assignment(char *s, struct mcc_stmt *stmt)
{
    char *eq = strchr(s, '=');
    if (eq) {
        stmt->u.keyval.key = s;
        *eq = '\0';
        stmt->u.keyval.val = eq + 1;
        strlnterm(eq + 1);
        return 1;
    } else {
        return 0;
    }
}

static int mcc_data_classify_data(const char *s, struct mcc_stmt *stmt)
{
    const char *s1 = s, *s2;
    char *endptr;
    strtok_ws(&s1, &s2);
    stmt->u.data.pos = strtod(s1, &endptr);
    if (s2 != endptr) {
        return 0;
    }
    s1 = s2;
    strtok_ws(&s1, &s2);
    stmt->u.data.dose = strtod(s1, &endptr);
    return s2 == endptr;
}

static int mcc_data_classify_delim(const char *s, struct mcc_stmt *stmt)
{
    const char *s1 = s, *s2;
    unsigned i;
    strtok_ws(&s1, &s2);
    for (i = 0; i < sizeof delims / sizeof *delims; i++) {
        if (strcmprng(s1, s2, delims[i])) {
            stmt->u.delim = i;
            return 1;
        }
    }
    return 0;
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
static void mcc_data_scope_check(struct mcc_parse_context *ctx, jmp_buf env)
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
    switch (combin) {
    case 0x21:
        if (!ctx->offaxis_fnd || !ctx->crosscal_fnd) {
            longjmp(env, MCC_ERROR_MISSING_ATTRIBUTE);
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
    double x = strtod(nptr, &endptr);
    if (nptr != endptr) {
        return x;
    } else {
        longjmp(env, MCC_ERROR_MALFORMED_ATTRIBUTE);
    }
}

static void mcc_data_keyval_check(struct mcc_parse_context *ctx, jmp_buf env)
/** Bluntly, we only care about two of these, and only in scan scope */
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
                                MCCData **data, unsigned long *capacity,
                                jmp_buf env)
{
    unsigned long i = (*data)->size;
    const double dose = ctx->stmt.u.data.dose * ctx->crosscal;
    if (i == *capacity) {
        unsigned long newcap = *capacity * 2;
        void *newptr = realloc(*data, sizeof **data + sizeof *(*data)->nodes * 3UL * newcap);
        if (newptr) {
            *data = newptr;
            *capacity = newcap;
        } else {
            longjmp(env, MCC_ERROR_NOMEM);
        }
    }
    i *= 3;
    (*data)->nodes[i] = ctx->stmt.u.data.pos;
    (*data)->nodes[i + 1] = ctx->offaxis;
    (*data)->nodes[i + 2] = dose;
    (*data)->size++;
}

/** DELETEME: */
/* static void PRINT_STATEMENT(const struct mcc_stmt *stmt)
{
    switch (stmt->type) {
    case MCC_STMT_DELIM:
        printf(" Delimiter: %s\n", delims[stmt->u.delim]);
        break;
    case MCC_STMT_KEYVAL:
        printf(" Key-value: %s -> %s\n", stmt->u.keyval.key, stmt->u.keyval.val);
        break;
    case MCC_STMT_DATA:
        printf(" Data:      % .2f\t%.3e\n", stmt->u.data.pos, stmt->u.data.dose);
        break;
    default:
        puts(" Invalid statement");
    }
} */

static void mcc_data_load_nodes(FILE *mcc, MCCData **data,
                                unsigned long *capacity, jmp_buf env)
{
    struct mcc_parse_context ctx = {
        .scope = SCOPE_OUT_OF_FILE,
        .offaxis_fnd = 0,
        .crosscal_fnd = 0
    };
    char linebuf[LINEBUFSZ];
    while (fgets(linebuf, sizeof linebuf, mcc)) {
        mcc_data_classify_statement(strltrim(linebuf), &ctx.stmt);
        //PRINT_STATEMENT(&ctx.stmt);
        switch (ctx.stmt.type) {
        case MCC_STMT_DELIM:
            mcc_data_scope_check(&ctx, env);
            break;
        case MCC_STMT_KEYVAL:
            mcc_data_keyval_check(&ctx, env);
            break;
        case MCC_STMT_DATA:
            mcc_data_data_check(&ctx, data, capacity, env);
            break;
        /* case MCC_STMT_INVALID: */
        default:
            longjmp(env, MCC_ERROR_UNCLASSIFIABLE_STATEMENT);
        }
    }
}

static MCCData *mcc_data_alloc(FILE *mcc, int *stat)
{
    unsigned long capacity = INITCAP;
    MCCData *data = malloc(sizeof *data + sizeof *data->nodes * 3UL * capacity);
    jmp_buf env;
    if (data) {
        data->triangulation = NULL;
        data->size = 0;
    } else {
        return NULL;
    }
    if ((*stat = setjmp(env))) {
        free(data);
        return NULL;
    }
    mcc_data_load_nodes(mcc, (MCCData **)&data, &capacity, env);
    /** Oh shit, realloc() down is not actually guaranteed, check this */
    return realloc(data, sizeof *data + sizeof *data->nodes * 3UL * data->size);
}

/* static void PRINT_DATA(const MCCData *data)
{
    unsigned i;
    for (i = 0; i < 3UL * data->size; i += 3) {
        printf(" Node: (% .2f, % .2f; % .3f Gy)\n",
            data->nodes[i], data->nodes[i + 1], data->nodes[i + 2]);
    }
} */

MCCData *mcc_data_create(const char *filename, int *stat)
{
    MCCData *data = NULL;
    FILE *mfile = fopen(filename, "r");
    if (mfile) {
        data = mcc_data_alloc(mfile, stat);
        if (data) {
            data->triangulation = triangulate(data->size, data->nodes);
            if (data->triangulation) {
                *stat = MCC_ERROR_NONE;
            } else {
                *stat = MCC_ERROR_TRIANGULATION_FAILED;
                free(data);
                return NULL;
            }
        }
    } else {
        *stat = MCC_ERROR_FOPEN_FAILED;
    }
    fclose(mfile);
    return data;
}

void mcc_data_destroy(MCCData *data)
{
    if (data) {
        free_triangle_pool(data->triangulation);
        free(data);
    }
}

static int mcc_data_hit_test(const struct delaunay_triangle *t,
                             const double x, const double y, double *out)
{
    const double ra[2] = { t->vertices[0][0] - x, t->vertices[0][1] - y };
    const double rb[2] = { t->vertices[1][0] - x, t->vertices[1][1] - y };
    const double rc[2] = { t->vertices[2][0] - x, t->vertices[2][1] - y };
    const double A = 0.5 * (rb[0] * rc[1] - rb[1] * rc[0]);
    const double B = 0.5 * (rc[0] * ra[1] - rc[1] * ra[0]);
    const double C = 0.5 * (ra[0] * rb[1] - ra[1] * rb[0]);
    if ((A < 0.0) || (B < 0.0) || (C < 0.0)) {
        return 0;
    } else {
        const double ab[2] = { t->vertices[1][0] - t->vertices[0][0], t->vertices[1][1] - t->vertices[0][1] };
        const double ac[2] = { t->vertices[2][0] - t->vertices[0][0], t->vertices[2][1] - t->vertices[0][1] };
        const double area = 0.5 * (ab[0] * ac[1] - ab[1] * ac[0]);
        /* *out = A * t->vertices[0][2] + B * t->vertices[1][2] + C * t->vertices[2][2]; */
        *out = 0.0;
        *out = fma(A, t->vertices[0][2], *out);
        *out = fma(B, t->vertices[1][2], *out);
        *out = fma(C, t->vertices[2][2], *out);
        *out /= area;
        return 1;
    }
}

double mcc_data_get_dose(const MCCData *data, double x, double y)
{
    const struct delaunay_triangle *t;
    double out = 0.0;
    for (t = data->triangulation->start; t < data->triangulation->end; t++) {
        if (!delaunay_is_manifold_tri(t)) {
            if (mcc_data_hit_test(t, x, y, &out)) {
                break;
            }
        }
    }
    return out;
}

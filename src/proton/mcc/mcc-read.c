#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "mcc.h"
#include "mcc-stmt.h"
#include "mcc-tree.h"


struct _mccfile {
    struct mcc_map *map;
    FILE *fp;
    size_t n_scans;
    fpos_t *scan_pos;
    struct _mccscan *scan;
};


struct _mccscan {
    struct mcc_map *map;
    long idx;
    struct mcc_scanvector {
        unsigned long n_data;
        double *x;
        double *y;
        long *idx;
    } data;
};

static struct _mccscan *empty_scan()
{
    struct _mccscan *scan = malloc(sizeof *scan);
    if (!scan) {
        return NULL;
    }
    scan->map = new_mcc_map();
    if (!scan->map) {
        free(scan);
        return NULL;
    }
    scan->data.n_data = 1;
    scan->data.x = malloc(sizeof *scan->data.x);
    if (!scan->data.x) {
        free_mcc_map(scan->map);
        free(scan);
        return NULL;
    }
    scan->data.y = malloc(sizeof *scan->data.y);
    if (!scan->data.y) {
        free(scan->data.x);
        free_mcc_map(scan->map);
        free(scan);
        return NULL;
    }
    scan->data.idx = malloc(sizeof *scan->data.idx);
    if (!scan->data.idx) {
        free(scan->data.y);
        free(scan->data.x);
        free_mcc_map(scan->map);
        free(scan);
        return NULL;
    }
    return scan;
}

static void free_scan(struct _mccscan *scan)
{
    free(scan->data.idx);
    free(scan->data.y);
    free(scan->data.x);
    free_mcc_map(scan->map);
    free(scan);
}


/*--- Linebuffer API ---*/

struct linebuffer {
    unsigned int size, capacity;
    char *buf;
};

static struct linebuffer *new_linebuffer(size_t capacity)
{
    struct linebuffer *lb = malloc(sizeof *lb);
    if (!lb) {
        return NULL;
    }
    lb->buf = malloc(sizeof *lb->buf * capacity);
    if (!lb->buf) {
        free(lb);
        return NULL;
    }
    lb->size = 0;
    lb->capacity = capacity;
    return lb;
}

static void free_linebuffer(struct linebuffer *lb)
{
    free(lb->buf);
    free(lb);
}

static int double_linebuffer(struct linebuffer *lb)
{
    unsigned int new_cap = lb->capacity * 2;
    char *new_buf = realloc(lb->buf, sizeof *new_buf * new_cap);
    if (!new_buf) {
        return 1;
    }
    lb->buf = new_buf;
    lb->capacity = new_cap;
    return 0;
}

static void lb_reset(struct linebuffer *lb)
{
    lb->size = 0;
}

static int lb_push_back(struct linebuffer *lb, char c)
{
    if (lb->size == lb->capacity) {
        if (double_linebuffer(lb)) {
            return 1;
        }
    }
    lb->buf[lb->size++] = c;
    return 0;
}

/*--- End linebuffer API ---*/

/*--- fpos_t vector API ---*/

struct fpos_vector {
    unsigned int size, capacity;
    fpos_t *buf;
};

static struct fpos_vector *new_fpos_vector(size_t capacity)
{
    struct fpos_vector *fv = malloc(sizeof *fv);
    if (!fv) {
        return NULL;
    }
    fv->buf = malloc(sizeof *fv->buf * capacity);
    if (!fv->buf) {
        free(fv);
        return NULL;
    }
    fv->size = 0;
    fv->capacity = capacity;
    return fv;
}

static void free_fpos_vector(struct fpos_vector *fv)
{
    free(fv->buf);
    free(fv);
}

static int double_fpos_vector(struct fpos_vector *fv)
{
    unsigned int new_cap = fv->capacity * 2;
    fpos_t *new_buf = realloc(fv->buf, sizeof *new_buf * new_cap);
    if (!new_buf) {
        return 1;
    }
    fv->buf = new_buf;
    fv->capacity = new_cap;
    return 0;
}

static int fv_push_back(struct fpos_vector *fv, fpos_t pos)
{
    if (fv->size == fv->capacity) {
        if (double_fpos_vector(fv)) {
            return MCC_PARSE_ALLOC_FAILED;
        }
    }
    fv->buf[fv->size++] = pos;
    return 0;
}

static void shrink_fpos_vector(struct fpos_vector *fv)
{
    fpos_t *new_buf = realloc(fv->buf, sizeof *new_buf * fv->size);
    /* realloc is guaranteed to work when the new size is leq the old */
    fv->buf = new_buf;
    fv->capacity = fv->size;
}

/* The vector is INVALID after this call */
static fpos_t *strip_fpos_vector(struct fpos_vector *v, size_t *n)
{
    fpos_t *buf = v->buf;
    *n = v->size;
    v->buf = NULL;
    return buf;
}

/*--- End fpos_t vector API ---*/


static int isEOF(char c)
{
    return c == EOF;
}

/** Does NOT place the newline char in the buffer! */
static int fgetline_b(FILE *f, struct linebuffer *lb)
{
    char c;
    lb_reset(lb);
    while (1) {
        c = fgetc(f);
        if (isEOF(c) || (c == '\n')) {
            break;
        } else if (c == '\r') {
            c = fgetc(f);
            if (c == '\n') {
                break;
            } else {
                lb_push_back(lb, '\r');
            }
        }
        if (lb_push_back(lb, c)) {
            return 1;
        }
    }
    return lb_push_back(lb, '\0');
}

enum {
    SCOPE_OUT_OF_FILE = 0,
    SCOPE_FILE        = 1,
    SCOPE_SCAN        = 2,
    SCOPE_DATA        = 3
};

struct mcc_parse_ctx {
    struct _mccfile *mcc;
    struct linebuffer *lb;
    struct fpos_vector *fv;
    struct mcc_statement stmt;
    fpos_t pos;
    int depth;
    int last_scan;
};

static int initialize_mcc_parse_context(struct mcc_parse_ctx *ctx, struct _mccfile *mcc)
{
    ctx->lb = new_linebuffer(64);
    if (!ctx->lb) {
        return 1;
    }
    ctx->fv = new_fpos_vector(64);
    if (!ctx->fv) {
        free_linebuffer(ctx->lb);
        return 1;
    }
    ctx->mcc = mcc;
    ctx->depth = SCOPE_OUT_OF_FILE;
    ctx->last_scan = 0;
    return 0;
}

static void cleanup_mcc_parse_context(struct mcc_parse_ctx *ctx)
{
    free_fpos_vector(ctx->fv);
    free_linebuffer(ctx->lb);
}

static void swap_fpos_vectors(struct _mccfile *mcc, struct mcc_parse_ctx *ctx)
{
    shrink_fpos_vector(ctx->fv);
    mcc->scan_pos = strip_fpos_vector(ctx->fv, &mcc->n_scans);
}

static int validate_assignment(struct mcc_parse_ctx *ctx)
{
    switch (ctx->depth) {
    case SCOPE_OUT_OF_FILE:
        return MCC_PARSE_ASSIGN_OUT_OF_FILE;
    case SCOPE_DATA:
        return MCC_PARSE_ASSIGN_IN_DATA;
    case SCOPE_FILE:
        /* Do something with the assignment here */
        if (insert_mcc_kvpair(ctx->mcc->map, ctx->stmt.udata.assignment.begin, ctx->stmt.udata.assignment.eq_sgn)) {
            return MCC_PARSE_ALLOC_FAILED;
        }
    default:
        return MCC_PARSE_NO_ERROR;
    }
}

static int validate_open_delim(struct mcc_parse_ctx *ctx)
{
    switch (ctx->stmt.udata.delim.type) {
    case MCC_DELIM_FILE:
        if (ctx->depth != SCOPE_OUT_OF_FILE) {
            return MCC_PARSE_UNEXPECTED_FILE_SCOPE;
        } else {
            break;
        }
    case MCC_DELIM_SCAN:
        if (ctx->depth != SCOPE_FILE) {
            return MCC_PARSE_UNEXPECTED_SCAN_SCOPE;
        } else {
            if (ctx->last_scan != (ctx->stmt.udata.delim.idx - 1)) {
                return MCC_PARSE_SCAN_SKIPPED;
            } else {
                if (fv_push_back(ctx->fv, ctx->pos)) {
                    return MCC_PARSE_ALLOC_FAILED;
                } else {
                    ctx->last_scan = ctx->stmt.udata.delim.idx;
                    break;
                }
            }
        }
    case MCC_DELIM_DATA:
        if (ctx->depth != SCOPE_SCAN) {
            return MCC_PARSE_UNEXPECTED_DATA_SCOPE;
        } else {
            break;
        }
    }
    ctx->depth++;
    return MCC_PARSE_NO_ERROR;
}

static int validate_close_delim(struct mcc_parse_ctx *ctx)
{
    switch (ctx->stmt.udata.delim.type) {
    case MCC_DELIM_FILE:
        if (ctx->depth != SCOPE_FILE) {
            return MCC_PARSE_UNEXPECTED_FILE_EXIT; 
        } else {
            break;
        }
    case MCC_DELIM_SCAN:
        if (ctx->depth != SCOPE_SCAN) {
            return MCC_PARSE_UNEXPECTED_SCAN_EXIT;
        } else {
            if (ctx->stmt.udata.delim.idx != ctx->last_scan) {
                return MCC_PARSE_MISMATCHED_SCAN_EXIT;
            } else {
                break;
            }
        }
    case MCC_DELIM_DATA:
        if (ctx->depth != SCOPE_DATA) {
            return MCC_PARSE_UNEXPECTED_DATA_EXIT;
        } else {
            break;
        }
    }
    ctx->depth--;
    return MCC_PARSE_NO_ERROR;
}

static int validate_data(struct mcc_parse_ctx *ctx)
{
    if (ctx->depth != SCOPE_DATA) {
        return MCC_PARSE_DATA_OUT_OF_SCOPE;
    } else {
        return MCC_PARSE_NO_ERROR;
    }
}

static int validate_statement(struct mcc_parse_ctx *ctx)
{
    switch (ctx->stmt.type) {
    case MCC_STMT_EMPTY:
        /* Are empty statements always valid? */
        break;
    case MCC_STMT_UNCLASSIFIABLE:
        return MCC_PARSE_UNCLASSIFIABLE_STMT;
    case MCC_STMT_ASSIGN:
        return validate_assignment(ctx);
    case MCC_STMT_DELIM_BEGIN:
        return validate_open_delim(ctx);
    case MCC_STMT_DELIM_END:
        return validate_close_delim(ctx);
    case MCC_STMT_DATA:
        return validate_data(ctx);
    }
    return MCC_PARSE_NO_ERROR;
}

static int validate_exit_context(struct mcc_parse_ctx *ctx)
{
    if (ctx->depth != SCOPE_OUT_OF_FILE) {
        return MCC_PARSE_SCOPE_ON_EXIT;
    } else {
        return MCC_PARSE_NO_ERROR;
    }
}

static int enumerate_mcc_scans(struct _mccfile *mcc)
{
    struct mcc_parse_ctx ctx;
    int error;
    if (initialize_mcc_parse_context(&ctx, mcc)) {
        return MCC_PARSE_ALLOC_FAILED;
    }
    do {
        fgetpos(mcc->fp, &ctx.pos);
        if (fgetline_b(mcc->fp, ctx.lb)) {
            error = MCC_PARSE_ALLOC_FAILED;
            goto enumerate_mcc_scans_failed;
        }
        classify_statement(ctx.lb->buf, &ctx.stmt);
        error = validate_statement(&ctx);
        if (error) {
            goto enumerate_mcc_scans_failed;
        }
        /* PRINT_STMT(&stmt); */
    } while (!feof(mcc->fp));
    error = validate_exit_context(&ctx);
    if (error) {
        goto enumerate_mcc_scans_failed;
    }
    swap_fpos_vectors(mcc, &ctx);
    cleanup_mcc_parse_context(&ctx);
    return 0;
enumerate_mcc_scans_failed:
    cleanup_mcc_parse_context(&ctx);
    return MCC_PARSE_NO_ERROR;
}

struct _mccfile *open_mcc_file(const char *filename)
{
    struct _mccfile *mcc;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return NULL;
    }
    mcc = malloc(sizeof *mcc);
    if (!mcc) {
        fclose(fp);
        return NULL;    
    }
    mcc->map = new_mcc_map();
    if (!mcc->map) {
        free(mcc);
        fclose(fp);
        return NULL;
    }
    mcc->scan = empty_scan();
    if (!mcc->scan) {
        free_mcc_map(mcc->map);
        free(mcc);
        fclose(fp);
        return NULL;
    }
    mcc->fp = fp;
    if (enumerate_mcc_scans(mcc)) {
        close_mcc_file(mcc);
        mcc = NULL;
    }
    return mcc;
}

void close_mcc_file(struct _mccfile *mcc)
{
    free_scan(mcc->scan);
    free_mcc_map(mcc->map);
    fclose(mcc->fp);
    free(mcc->scan_pos);
    free(mcc);
}

unsigned long get_n_mcc_scans(const struct _mccfile *mcc)
{
    return mcc->n_scans;
}

static int double_mcc_scanvector(struct mcc_scanvector *v)
{
    unsigned long new_cap = v->n_data * 2;
    double *x_new, *y_new;
    long *idx_new;
    x_new = realloc(v->x, sizeof *x_new * new_cap);
    if (!x_new) {
        return 1;
    } else {
        v->x = x_new;
    }
    y_new = realloc(v->y, sizeof *y_new * new_cap);
    if (!y_new) {
        return 1;
    } else {
        v->y = y_new;
    }
    idx_new = realloc(v->idx, sizeof *idx_new * new_cap);
    if (!idx_new) {
        return 1;
    } else {
        v->idx = idx_new;
    }
    v->n_data = new_cap;
    return 0;
}

static void shrink_mcc_scanvector(struct mcc_scanvector *v, unsigned long i)
{
    v->n_data = i;
    v->x = realloc(v->x, sizeof *v->x * i);
    v->y = realloc(v->y, sizeof *v->y * i);
    v->idx = realloc(v->idx, sizeof *v->idx * i);
}

static void clear_scan(struct _mccscan *scan)
{
    clear_mcc_map(scan->map);
    shrink_mcc_scanvector(&scan->data, 1);
}

const struct _mccscan *get_scan(struct _mccfile *mcc, unsigned long n)
{
    struct mcc_statement stmt;
    struct linebuffer *lb = new_linebuffer(32);
    unsigned long i = 0;
    clear_mcc_map(mcc->scan->map);
    if (!lb) {
        return NULL;
    }
    if (fsetpos(mcc->fp, mcc->scan_pos + n)) {
        free_linebuffer(lb);
        return NULL;
    }
    while (1) {
        if (fgetline_b(mcc->fp, lb)) {
            free_linebuffer(lb);
            return NULL;
        }
        classify_statement(lb->buf, &stmt);
        /* PRINT_STMT(&stmt); */
        switch (stmt.type) {
        case MCC_STMT_ASSIGN:
            if (insert_mcc_kvpair(mcc->scan->map, stmt.udata.assignment.begin, stmt.udata.assignment.eq_sgn)) {
                free_linebuffer(lb);
                return NULL;
            }
            break;
        case MCC_STMT_DATA:
            if (i == mcc->scan->data.n_data) {
                if (double_mcc_scanvector(&mcc->scan->data)) {
                    clear_scan(mcc->scan);
                    free_linebuffer(lb);
                    return NULL;
                }
            }
            mcc->scan->data.x[i] = stmt.udata.data.x;
            mcc->scan->data.y[i] = stmt.udata.data.y;
            mcc->scan->data.idx[i] = stmt.udata.data.idx;
            i++;
            break;
        default:
            break;
        }
        if (stmt.type == MCC_STMT_DELIM_END) {
            if (stmt.udata.delim.type == MCC_DELIM_SCAN) {
                break;
            }
        }
    }
    shrink_mcc_scanvector(&mcc->scan->data, i);
    free_linebuffer(lb);
    return mcc->scan;
}

int get_scan_double(const struct _mccscan *scan, const char *key, double *val)
{
    const char *sval;
    char *endptr;
    sval = mcc_map_lookup(scan->map, key);
    if (!sval) {
        return 1;
    }
    *val = strtod(sval, &endptr);
    if (sval == endptr) {
        return 2;
    } else {
        return 0;
    }
}

int get_scan_integer(const struct _mccscan *scan, const char *key, long *val)
{
    const char *sval;
    char *endptr;
    sval = mcc_map_lookup(scan->map, key);
    if (!sval) {
        return 1;
    }
    *val = strtol(sval, &endptr, 10);
    if (sval == endptr) {
        return 2;
    } else {
        return 0;
    }
}

int get_scan_string(const struct _mccscan *scan, const char *key, const char **val)
{
    *val = mcc_map_lookup(scan->map, key);
    return *val == NULL;
}

unsigned long get_scan_data(const struct _mccscan *scan,
                            const double **x,
                            const double **y,
                            const long **idx)
{
    *x = scan->data.x;
    *y = scan->data.y;
    *idx = scan->data.idx;
    return scan->data.n_data;
}

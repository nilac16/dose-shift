#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "mcc-stmt.h"


enum {
    TOKEN_UNRECOGNIZED    = -1,
    TOKEN_BEGIN_SCAN_DATA = 0,
    TOKEN_BEGIN_SCAN      = 1,
    TOKEN_BEGIN_DATA      = 2,
    TOKEN_END_SCAN        = 3,
    TOKEN_END_DATA        = 4,
    TOKEN_END_SCAN_DATA   = 5
};

static const char *delims[] = {
    "BEGIN_SCAN_DATA",
    "BEGIN_SCAN",
    "BEGIN_DATA",
    "END_SCAN",
    "END_DATA",
    "END_SCAN_DATA"
};


static int isassign(char c)
{
    return c == '=';
}

static void ltrim_whitespace(const char **str)
{
    while (isspace(**str)) {
        (*str)++;
    }
}

static int strempty(const char *str)
{
    ltrim_whitespace(&str);
    return *str == '\0';
}

static int strmatch(const char *str, const char *const strend, const char *cmpstr)
{
    while (str < strend) {
        if (*str != *cmpstr) {
            return 0;
        } else {
            str++;
            cmpstr++;
        }
    }
    return *cmpstr == '\0';
}

static int token_type(const char *nptr, const char *endptr)
{
    unsigned int i;
    for (i = 0; i < sizeof delims / sizeof *delims; i++) {
        if (strmatch(nptr, endptr, delims[i])) {
            return i;
        }
    }
    return TOKEN_UNRECOGNIZED;
}

static void set_empty_stmt(struct mcc_statement *stmt)
{
    stmt->type = MCC_STMT_EMPTY;
}

static void set_unclassifiable_stmt(const char *line, struct mcc_statement *stmt)
{
    stmt->type = MCC_STMT_UNCLASSIFIABLE;
    stmt->udata.assignment.begin = line;
}

static void set_assignment_stmt(const char *line, const char *eq_sgn,
                                struct mcc_statement *stmt)
{
    stmt->type = MCC_STMT_ASSIGN;
    stmt->udata.assignment.begin = line;
    stmt->udata.assignment.eq_sgn = eq_sgn;
}

static int set_indexed_delim_stmt(int ttype, const char *strrem,
                                   struct mcc_statement *stmt)
{
    char *endptr;
    long x = strtol(strrem, &endptr, 10);
    if (strrem == endptr) {
        /* Bad index */
        return 1;
    } else if (!strempty(endptr)) {
        /* Junk after index */
        return 1;
    } else {
        stmt->udata.delim.type = MCC_DELIM_SCAN;
        stmt->udata.delim.idx = x;
        if (ttype == TOKEN_BEGIN_SCAN) {
            stmt->type = MCC_STMT_DELIM_BEGIN;
        } else {
            stmt->type = MCC_STMT_DELIM_END;
        }
        return 0;
    }
}

static int set_nonindexed_delim_stmt(int ttype, const char *strrem,
                                     struct mcc_statement *stmt)
{
    if (!strempty(strrem)) {
        /* Junk after a nonindexed delimiter */
        return 1;
    } else {
        switch (ttype) {
        case TOKEN_BEGIN_SCAN_DATA:
            stmt->type = MCC_STMT_DELIM_BEGIN;
            stmt->udata.delim.type = MCC_DELIM_FILE;
            break;
        case TOKEN_BEGIN_DATA:
            stmt->type = MCC_STMT_DELIM_BEGIN;
            stmt->udata.delim.type = MCC_DELIM_DATA;
            break;
        case TOKEN_END_DATA:
            stmt->type = MCC_STMT_DELIM_END;
            stmt->udata.delim.type = MCC_DELIM_DATA;
            break;
        case TOKEN_END_SCAN_DATA:
            stmt->type = MCC_STMT_DELIM_END;
            stmt->udata.delim.type = MCC_DELIM_FILE;
            break;
        }
        return 0;
    }
}

static int set_data_stmt(const char *line, const char *rem,
                         struct mcc_statement *stmt)
/** First two tokens must be valid floating-point values, and the last one 
 *  must be an integer preceded by a single pound sign */
{
    double x, y;
    long idx;
    x = strtod(line, (char **)&rem);
    if (line == rem) {
        /* Bad independent variable value */
        return 1;
    }
    line = rem;
    ltrim_whitespace(&line);
    y = strtod(line, (char **)&rem);
    if (line == rem) {
        /* Bad dependent variable value */
        return 1;
    }
    line = rem;
    ltrim_whitespace(&line);
    if (*line != '#') {
        /* Missing index */
        return 1;
    } else {
        line++;
    }
    idx = strtol(line, (char **)&rem, 10);
    if (line == rem) {
        /* Bad index */
        return 1;
    }
    if (!strempty(rem)) {
        /* Junk after index */
        return 1;
    }
    stmt->type = MCC_STMT_DATA;
    stmt->udata.data.x = x;
    stmt->udata.data.y = y;
    stmt->udata.data.idx = idx;
    return 0;
}

static int set_nonassignment_stmt(const char *line, const char *rem,
                                  struct mcc_statement *stmt)
{
    int ttype = token_type(line, rem);
    switch (ttype) {
    case TOKEN_UNRECOGNIZED:
        return set_data_stmt(line, rem, stmt);
    case TOKEN_BEGIN_SCAN:
    case TOKEN_END_SCAN:
        return set_indexed_delim_stmt(ttype, rem, stmt);
    default:
        return set_nonindexed_delim_stmt(ttype, rem, stmt);
    }
}

void classify_statement(const char *line, struct mcc_statement *stmt)
{
    const char *c;
    ltrim_whitespace(&line);
    if (!*line) {
        set_empty_stmt(stmt);
        return;
    }
    c = line + 1;
    while (1) {
        if (isassign(*c)) {
            set_assignment_stmt(line, c, stmt);
            break;
        } else if (!*c || isspace(*c)) {
            if (set_nonassignment_stmt(line, c, stmt)) {
                set_unclassifiable_stmt(line, stmt);
            }
            break;
        } else {
            c++;
        }
    }
}

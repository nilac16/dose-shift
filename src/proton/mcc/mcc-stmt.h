#pragma once
/** MCC files appear to be sequences of line-broken statements. Statements 
 *  take one of the following forms
 *      - Delimiter:    <id>[ <index>]
 *      - Assignment:   <id>=<string>
 *      - Data:         <pos>   <value>     #<index>
 *                     ({float} {float}     #{int}  )
 *
 *  Any amount of whitespace delimits any token not on the RHS of an
 *  assignment; strings may contain spaces. A space to the left of the
 *  assignment character will result in an unclassifiable statement, and a 
 *  space to the right will be rolled into the string. This will not be an 
 *  issue if the string is numeric (and thus must later be converted), but 
 *  should be considered undefined behavior.
 *  
 *  Specific delimiters carry indices, which are essential to their meaning. 
 *  It may be a better idea to split the grammar for delimiters:
 *      - Delimiter:    <id>
 *      - n-delimiter:  <id>    <index>
 */
#ifndef MCC_STATEMENT_H
#define MCC_STATEMENT_H


struct mcc_statement {
    enum /* statement_type */ {
        MCC_STMT_EMPTY          = -2,
        MCC_STMT_UNCLASSIFIABLE = -1, /* stolen from gfortran ;) */
        MCC_STMT_ASSIGN         = 0,
        MCC_STMT_DELIM_BEGIN    = 1,
        MCC_STMT_DELIM_END      = 2,
        MCC_STMT_DATA           = 3
    } type;
    union {
        struct {
            const char *begin;
            const char *eq_sgn;
        } assignment;
        struct {
            enum /* delim_type */ {
                MCC_DELIM_FILE = 1,
                MCC_DELIM_SCAN = 2,
                MCC_DELIM_DATA = 3
            } type;
            long idx;
        } delim;
        struct {
            double x;
            double y;
            long idx;
        } data;
    } udata;
};

void classify_statement(const char *line, struct mcc_statement *stmt);


enum mcc_parse_errors {
    MCC_PARSE_ALLOC_FAILED = -1,
    MCC_PARSE_NO_ERROR     = 0,
    MCC_PARSE_UNCLASSIFIABLE_STMT,
    MCC_PARSE_ASSIGN_OUT_OF_FILE,
    MCC_PARSE_ASSIGN_IN_DATA,
    MCC_PARSE_UNEXPECTED_FILE_SCOPE,
    MCC_PARSE_UNEXPECTED_SCAN_SCOPE,
    MCC_PARSE_SCAN_SKIPPED,
    MCC_PARSE_UNEXPECTED_DATA_SCOPE,
    MCC_PARSE_UNEXPECTED_FILE_EXIT,
    MCC_PARSE_UNEXPECTED_SCAN_EXIT,
    MCC_PARSE_MISMATCHED_SCAN_EXIT,
    MCC_PARSE_UNEXPECTED_DATA_EXIT,
    MCC_PARSE_DATA_OUT_OF_SCOPE,
    MCC_PARSE_SCOPE_ON_EXIT
};


void PRINT_STMT(const struct mcc_statement *stmt);

#endif /* MCC_STATEMENT_H */

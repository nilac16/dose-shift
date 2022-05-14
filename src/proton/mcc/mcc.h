#pragma once

#ifndef MCC_FILE_H
#define MCC_FILE_H
/** This library was written in December of 2021. There are many small calls 
 *  to malloc() made in this code. Not sure if this will have much of an 
 *  effect on modern heap memory. I guess just try to have no more than one 
 *  MCC file open at a time
 */
#if __cplusplus
extern "C" {
#endif


/** Struct containing data necessary for reading MCC files */
typedef struct _mccfile mccfile;

/** Struct with scan data, including tags */
typedef struct _mccscan mccscan;


/** Opens the specified MCC file at @p filename for reading. If any failure 
 *  occurs, sets errno and returns NULL. The file on disk is kept open for 
 *  the lifetime of this structure. 
 *  
 *  \param filename
 *      Path to MCC file to be opened
 *  \returns A pointer to structure containing MCC data
 */
mccfile *open_mcc_file(const char *filename);


/** Closes the file opened within @p mcc and frees the memory used by the 
 *  MCC structure. The pointer to @p mcc is invalidated after this call, and 
 *  the file stream is closed.
 * 
 *  \param mcc
 *      MCC file structure containing FILE *stream and scan positions
 */
void close_mcc_file(mccfile *mcc);


/** Gets the number of scans in the file.
 */
unsigned long get_n_mcc_scans(const mccfile *mcc);


/** Get the n-th scan from the file */
const mccscan *get_scan(mccfile *mcc, unsigned long n);


/** These functions get the value associated to @p key as the specified type, 
 *  if possible. If the key does not exist, these functions return 1. If the 
 *  conversion fails, they return 2. On success, they return 0 and set the 
 *  value at the passed @p val pointer to the converted value. On failure, 
 *  the value at this pointer is unspecified
 */
int get_scan_double(const mccscan *scan, const char *key, double *val);
int get_scan_integer(const mccscan *scan, const char *key, long *val);
int get_scan_string(const mccscan *scan, const char *key, const char **val);


/** TODO: This function makes the assumption that every scan contains a 
 *  single data segment, but this is not enforced by the parser. This could
 *  result in potentially catastrophic UB for certain malformed files
 */
unsigned long get_scan_data(const mccscan *scan,
                            const double **x,
                            const double **y,
                            const long **idx);


#if __cplusplus
}
#endif

#endif /* MCC_FILE_H */

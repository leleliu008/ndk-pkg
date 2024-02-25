#ifndef _EXE_H
#define _EXE_H

#include <stdlib.h>
#include <stdbool.h>


/** search the given command name in the PATH.
 *
 *  On success, list size is returned.
 *  If not found, 0 is returned.
 *  On error, -1 is returned and errno is set to indicate the error.
 *  On PATH is not set, -2 is returned.
 *  On PATH is set a empty string, -3 is returned.
 */
int exe_search(const char * commandName, char *** listP, const bool findAll);

/** lookup the given command name in the PATH.
 *
 *  On success, path length is returned.
 *  If not found, 0 is returned.
 *  On error, -1 is returned and errno is set to indicate the error.
 *  On PATH is not set, -2 is returned.
 *  On PATH is set a empty string, -3 is returned.
 */
int exe_lookup(const char * commandName, char ** pathP);

/** lookup the given command name in the PATH.
 *
 *  On success, path length is returned.
 *  If not found, 0 is returned.
 *  On error, -1 is returned and errno is set to indicate the error.
 *  On PATH is not set, -2 is returned.
 *  On PATH is set a empty string, -3 is returned.
 */
int exe_where(const char * commandName, char buf[], size_t bufSize);

#endif

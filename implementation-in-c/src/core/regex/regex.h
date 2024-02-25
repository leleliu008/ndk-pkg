#ifndef _REGEX_H_
#define _REGEX_H_

#include<stdbool.h>

/** check if the given string content matches the given regular expression pattern.
 *
 *  On success,  0 is returned.
 *  On error,   -1 is returned and errno is set to indicate the error.
 *  On no match,-1 is returned and errno is set to 0.
 */
int   regex_matched(const char * content, const char * pattern);

/** extract the matched string.
 *
 *  On success, matched string pointer is returned.
 *  On error,   NULL is returned and errno is set to indicate the error.
 *  On no match,NULL is returned and errno is set to 0.
 */
char* regex_extract(const char * content, const char * pattern);

#endif

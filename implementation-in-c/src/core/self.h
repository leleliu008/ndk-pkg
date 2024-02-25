#ifndef _SELF_H
#define _SELF_H

/** get the current executing executable's absolute file path.
 *
 *  On success, char* pointer is returned.
 *  On error, NULL is returned and errno is set to indicate the error.
 */
char* self_realpath();

#endif

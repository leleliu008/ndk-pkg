#ifndef _URL_TRANSFORM_H_
#define _URL_TRANSFORM_H_

#include <stdlib.h>
#include <stdbool.h>

/** transform a old url to new url.
 *
 *  On success, 0 is returned.
 *  On error,  -1 is returned and errno is set to indicate the error.
 */
int url_transform(const char * urlTransformCommandPath, const char * inUrl, char outputBuffer[], size_t outputBufferSizeInBytes, size_t * writtenSizeInBytes, const bool verbose);

#endif

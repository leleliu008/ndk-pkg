#ifndef _URL_H_
#define _URL_H_
    
#include <stdlib.h>
#include <stdbool.h>

int url_encode(char outputBuf[], size_t * writtenSizeInBytes, unsigned char inputBuf[], size_t inputBufSizeInBytes, const bool isToUpper);

int url_decode(unsigned char outputBuf[], size_t * writtenSizeInBytes, char inputBuf[], size_t inputBufSizeInBytes);

#endif

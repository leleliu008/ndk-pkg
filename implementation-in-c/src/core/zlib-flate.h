#ifndef _ZLIB_FLATE_H
#define _ZLIB_FLATE_H

int zlib_deflate_string_to_file(const char * inputBuf, size_t inputBufSizeInBytes, FILE * outputFile, int level);

int zlib_deflate_file_to_file(FILE * inputFile, FILE * outputFile, int level);
int zlib_inflate_file_to_file(FILE * inputFile, FILE * outputFile);

#endif

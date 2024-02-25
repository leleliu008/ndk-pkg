#include <stdio.h>
#include <string.h>

#include <zlib.h>

#include "zlib-flate.h"

#define CHUNK 16384

// this source file was modified from https://www.zlib.net/zpipe.c

// compress
int zlib_deflate_string_to_file(const char * inputBuf, size_t inputBufSizeInBytes, FILE * outputFile, int level) {
    if (inputBufSizeInBytes == 0U) {
        inputBufSizeInBytes = strlen(inputBuf);
    }

    if (level == 0) {
        level = 1;
    }

    z_stream zStream;
    zStream.zalloc = Z_NULL;
    zStream.zfree  = Z_NULL;
    zStream.opaque = Z_NULL;

    int ret = deflateInit(&zStream, level);

    if (ret != Z_OK) {
        return ret;
    }

    unsigned char outputBuf[CHUNK];

    zStream.avail_in = inputBufSizeInBytes;
    zStream.next_in  = (unsigned char *)inputBuf;

    do {
        zStream.avail_out = CHUNK;
        zStream.next_out  = outputBuf;

        ret = deflate(&zStream, Z_FINISH);

        if (ret == Z_STREAM_ERROR) {
            (void)deflateEnd(&zStream);
            return ret;
        }

        unsigned int have = CHUNK - zStream.avail_out;

        if ((fwrite(outputBuf, 1, have, outputFile) != have) || ferror(outputFile)) {
            (void)deflateEnd(&zStream);
            return Z_ERRNO;
        }
    } while (zStream.avail_out == 0);

    (void)deflateEnd(&zStream);
    return Z_OK;
}

int zlib_deflate_file_to_file(FILE * inputFile, FILE * outputFile, int level) {
    z_stream zStream;
    zStream.zalloc = Z_NULL;
    zStream.zfree  = Z_NULL;
    zStream.opaque = Z_NULL;

    int ret = deflateInit(&zStream, level);

    if (ret != Z_OK) {
        return ret;
    }

    int flush;
    unsigned have;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    do {
        zStream.avail_in = fread(in, 1, CHUNK, inputFile);

        if (ferror(inputFile)) {
            (void)deflateEnd(&zStream);
            return Z_ERRNO;
        }

        flush = feof(inputFile) ? Z_FINISH : Z_NO_FLUSH;

        zStream.next_in = in;

        do {
            zStream.avail_out = CHUNK;
            zStream.next_out  = out;

            ret = deflate(&zStream, flush);

            if (ret == Z_STREAM_ERROR) {
                (void)deflateEnd(&zStream);
                return ret;
            }

            have = CHUNK - zStream.avail_out;

            if ((fwrite(out, 1, have, outputFile) != have) || ferror(outputFile)) {
                (void)deflateEnd(&zStream);
                return Z_ERRNO;
            }
        } while (zStream.avail_out == 0);
    } while (flush != Z_FINISH);

    (void)deflateEnd(&zStream);
    return Z_OK;
}

int zlib_inflate_file_to_file(FILE * inputFile, FILE * outputFile) {
    z_stream zStream;
    zStream.zalloc = Z_NULL;
    zStream.zfree = Z_NULL;
    zStream.opaque = Z_NULL;
    zStream.avail_in = 0;
    zStream.next_in = Z_NULL;

    int ret = inflateInit(&zStream);

    if (ret != Z_OK) {
        return ret;
    }

    unsigned char  inputBuf[CHUNK];
    unsigned char outputBuf[CHUNK];

    unsigned have;

    do {
        zStream.avail_in = fread(inputBuf, 1, CHUNK, inputFile);

        if (ferror(inputFile)) {
            (void)inflateEnd(&zStream);
            return Z_ERRNO;
        }

        if (zStream.avail_in == 0) {
            break;
        }

        zStream.next_in = inputBuf;

        do {
            zStream.avail_out = CHUNK;
            zStream.next_out  = outputBuf;

            ret = inflate(&zStream, Z_NO_FLUSH);

            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&zStream);
                return ret;
            }

            have = CHUNK - zStream.avail_out;

            if ((fwrite(outputBuf, 1, have, outputFile) != have) || ferror(outputFile)) {
                (void)inflateEnd(&zStream);
                return Z_ERRNO;
            }
        } while (zStream.avail_out == 0);
    } while (ret != Z_STREAM_END);

    (void)inflateEnd(&zStream);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

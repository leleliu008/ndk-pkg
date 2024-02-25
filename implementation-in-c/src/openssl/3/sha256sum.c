#include <stdio.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include "../../sha256sum.h"
#include "../../ndk-pkg.h"

static inline void tohex(char buf[65], const unsigned char * sha256Bytes) {
    const char * const table = "0123456789abcdef";

    for (size_t i = 0U; i < SHA256_DIGEST_LENGTH; i++) {
        size_t j = i << 1;
        buf[j]      = table[sha256Bytes[i] >> 4];
        buf[j + 1U] = table[sha256Bytes[i] & 0x0F];
    }
}

int sha256sum_of_bytes(char outputBuffer[65], unsigned char * inputBuffer, size_t inputBufferSizeInBytes) {
    if (outputBuffer == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (inputBuffer == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (inputBufferSizeInBytes == 0U) {
        return NDKPKG_ERROR_ARG_IS_INVALID;
    }

    unsigned char sha256Bytes[SHA256_DIGEST_LENGTH] = {0};
 
    EVP_MD_CTX * ctx = EVP_MD_CTX_new();

    if (ctx == NULL) {
        return NDKPKG_ERROR;
    }

    unsigned int len;

    int ret;

    if ((ret = EVP_DigestInit(ctx, EVP_sha256())) != 1) {
        goto finally;
    }

    if ((ret = EVP_DigestUpdate(ctx, inputBuffer, inputBufferSizeInBytes)) != 1) {
        goto finally;
    }

    if ((ret = EVP_DigestFinal(ctx, sha256Bytes, &len)) != 1) {
        goto finally;
    }

    tohex(outputBuffer, sha256Bytes);

finally:
    EVP_MD_CTX_free(ctx);

    return !ret;
}

int sha256sum_of_string(char outputBuffer[65], const char * str) {
    if (str == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (str[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    unsigned char sha256Bytes[SHA256_DIGEST_LENGTH] = {0};
 
    EVP_MD_CTX * ctx = EVP_MD_CTX_new();

    if (ctx == NULL) {
        return NDKPKG_ERROR;
    }

    unsigned int len;

    int ret;

    if ((ret = EVP_DigestInit(ctx, EVP_sha256())) != 1) {
        goto finally;
    }

    if ((ret = EVP_DigestUpdate(ctx, str, strlen(str))) != 1) {
        goto finally;
    }

    if ((ret = EVP_DigestFinal(ctx, sha256Bytes, &len)) != 1) {
        goto finally;
    }

    tohex(outputBuffer, sha256Bytes);

finally:
    EVP_MD_CTX_free(ctx);

    return !ret;
}

int sha256sum_of_stream(char outputBuffer[65], FILE * file) {
    if (outputBuffer == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (file == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    unsigned char sha256Bytes[SHA256_DIGEST_LENGTH] = {0};
  
    EVP_MD_CTX * ctx = EVP_MD_CTX_new();

    if (ctx == NULL) {
        return NDKPKG_ERROR;
    }

    unsigned int len;

    int ret;

    if ((ret = EVP_DigestInit(ctx, EVP_sha256())) != 1) {
        goto finally;
    }

    unsigned char readBuf[1024];

    for (;;) {
        size_t readSize = fread(readBuf, 1, 1024, file);

        if (ferror(file)) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (readSize > 0U) {
            if ((ret = EVP_DigestUpdate(ctx, readBuf, readSize)) != 1) {
                goto finally;
            }
        }

        if (feof(file)) {
            break;
        }
    }

    if ((ret = EVP_DigestFinal(ctx, sha256Bytes, &len)) != 1) {
        goto finally;
    }

    tohex(outputBuffer, sha256Bytes);

finally:
    EVP_MD_CTX_free(ctx);

    return !ret;
}

int sha256sum_of_file(char outputBuffer[65], const char * filepath) {
    if (outputBuffer == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (filepath == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (filepath[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    FILE * file = fopen(filepath, "rb");

    if (file == NULL) {
        perror(filepath);
        return NDKPKG_ERROR;
    }

    int ret = sha256sum_of_stream(outputBuffer, file);

    fclose(file);

    return ret;
}

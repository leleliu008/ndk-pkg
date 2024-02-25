#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "ndk-pkg.h"

int ndkpkg_home_dir(char buf[], size_t bufSize, size_t * outSize) {
    if (buf == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (bufSize == 0U) {
        return NDKPKG_ERROR_ARG_IS_INVALID;
    }

    const char * const ndkpkgHomeDIR = getenv("NDKPKG_HOME");

    if (ndkpkgHomeDIR == NULL) {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL) {
            return NDKPKG_ERROR_ENV_HOME_NOT_SET;
        }

        if (userHomeDIR[0] == '\0') {
            return NDKPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t defaultUppmHomeDIRCapacity = strlen(userHomeDIR) + 10U;
        char   defaultUppmHomeDIR[defaultUppmHomeDIRCapacity];

        int ret = snprintf(defaultUppmHomeDIR, defaultUppmHomeDIRCapacity, "%s/.ndk-pkg", userHomeDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        size_t defaultUppmHomeDIRLength = ret;

        struct stat st;

        if (stat(defaultUppmHomeDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s was expected to be a directory, but it was not.\n", defaultUppmHomeDIR);
                return NDKPKG_ERROR;
            }
        } else {
            if (mkdir(defaultUppmHomeDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(defaultUppmHomeDIR);
                    return NDKPKG_ERROR;
                }
            }
        }

        size_t m = bufSize - 1U;

        size_t n = (m > defaultUppmHomeDIRLength) ? defaultUppmHomeDIRLength : m;

        strncpy(buf, defaultUppmHomeDIR, n);

        buf[n] = '\0';

        if (outSize != NULL) {
            (*outSize) = n;
        }
    } else {
        if (ndkpkgHomeDIR[0] == '\0') {
            fprintf(stderr, "'NDKPKG_HOME' environment variable's value was expected to be a non-empty string, but it was not.\n");
            return NDKPKG_ERROR;
        }

        struct stat st;

        if (stat(ndkpkgHomeDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s was expected to be a directory, but it was not.\n", ndkpkgHomeDIR);
                return NDKPKG_ERROR;
            }
        } else {
            if (mkdir(ndkpkgHomeDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ndkpkgHomeDIR);
                    return NDKPKG_ERROR;
                }
            }
        }

        size_t ndkpkgHomeDIRLength = strlen(ndkpkgHomeDIR);

        size_t m = bufSize - 1U;

        size_t n = (m > ndkpkgHomeDIRLength) ? ndkpkgHomeDIRLength : m;

        strncpy(buf, ndkpkgHomeDIR, n);

        buf[n] = '\0';

        if (outSize != NULL) {
            (*outSize) = n;
        }
    }

    return NDKPKG_OK;
}

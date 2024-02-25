#include <string.h>

#include "ndk-pkg.h"

int ndkpkg_rename_or_copy_file(const char * fromFilePath, const char * toFilePath) {
    if (fromFilePath == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (toFilePath == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (fromFilePath[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (toFilePath[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    size_t i = 0U;
    size_t j = 0U;

    for (;;) {
        char c = toFilePath[i];

        if (c == '\0') {
            break;
        }

        if (c == '/') {
            j = i;
        }

        i++;
    }

    if (j > 0U) {
        char outputDIR[j + 2U];

        strncpy(outputDIR, toFilePath, j);

        outputDIR[j] = '\0';

        int ret = ndkpkg_mkdir_p(outputDIR, false);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    if (rename(fromFilePath, toFilePath) == 0) {
        return NDKPKG_OK;
    } else {
        if (errno == EXDEV) {
            return ndkpkg_copy_file(fromFilePath, toFilePath);
        } else {
            perror(toFilePath);
            return NDKPKG_ERROR;
        }
    }
}

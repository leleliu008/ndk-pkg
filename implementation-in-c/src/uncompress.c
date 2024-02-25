#include <time.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/tar.h"

#include "sha256sum.h"

#include "ndk-pkg.h"

int ndkpkg_uncompress(const char * filePath, const char * unpackDIR, const size_t stripComponentsNumber, const bool verbose) {
    if (filePath == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (filePath[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (unpackDIR == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (unpackDIR[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    int ret = tar_extract(unpackDIR, filePath, ARCHIVE_EXTRACT_TIME, verbose, stripComponentsNumber);

    if (ret == 0) {
        return NDKPKG_OK;
    } else {
        return abs(ret) + NDKPKG_ERROR_ARCHIVE_BASE;
    }
}

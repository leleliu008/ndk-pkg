#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "core/regex/regex.h"

#include "ndk-pkg.h"

static size_t j = 0U;

static int package_name_filter(const char * packageName, const bool verbose, size_t i, const void * regPattern) {
    if (regex_matched(packageName, (char*)regPattern) == 0) {
        if (verbose) {
            if (j != 0U) {
                printf("\n");
            }

            j++;

            return ndkpkg_available_info(packageName, NULL);
        } else {
            puts(packageName);
            return NDKPKG_OK;
        }
    } else {
        if (errno == 0) {
            return NDKPKG_OK;
        } else {
            perror(NULL);
            return NDKPKG_ERROR;
        }
    }
}

int ndkpkg_search(const char * regPattern, const bool verbose) {
    if (regPattern == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (regPattern[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    return ndkpkg_list_the_available_packages(verbose, package_name_filter, regPattern);
}

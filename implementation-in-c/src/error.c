#include <stdio.h>
#include "ndk-pkg.h"

void ndkpkg_show_error_message(int errorCode, const char * str) {
    switch(errorCode) {
        case NDKPKG_ERROR:  fprintf(stderr, "occurs error."); break;

        case NDKPKG_ERROR_ARG_IS_NULL:  fprintf(stderr, "argument is NULL."); break;
        case NDKPKG_ERROR_ARG_IS_EMPTY: fprintf(stderr, "argument is empty string."); break;
        case NDKPKG_ERROR_ARG_IS_INVALID: fprintf(stderr, "argument is invalid."); break;

        case NDKPKG_ERROR_PACKAGE_NOT_AVAILABLE: fprintf(stderr, "package [%s] is not available.", str); break;
        case NDKPKG_ERROR_PACKAGE_NOT_INSTALLED: fprintf(stderr, "package [%s] is not installed.", str); break;
        case NDKPKG_ERROR_PACKAGE_NOT_OUTDATED: fprintf(stderr, "package [%s] is not outdated.", str); break;
    }
}

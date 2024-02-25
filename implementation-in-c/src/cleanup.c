#include "core/log.h"

#include "ndk-pkg.h"

int ndkpkg_cleanup(const bool verbose) {
    if (verbose) {
        LOG_SUCCESS1("Done.");
    }

    return NDKPKG_OK;
}

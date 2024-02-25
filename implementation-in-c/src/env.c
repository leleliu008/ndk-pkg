#include <stdio.h>

#include <limits.h>

#include "core/self.h"

#include "ndk-pkg.h"

int ndkpkg_env(const bool verbose) {
    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    printf("ndk-pkg.version : %s\n", NDKPKG_VERSION);
    printf("ndk-pkg.homedir : %s\n", ndkpkgHomeDIR);

    char * selfRealPath = self_realpath();

    if (selfRealPath == NULL) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    printf("ndk-pkg.exepath : %s\n", selfRealPath);

    free(selfRealPath);

    printf("ndk-pkg.website : %s\n", "https://github.com/leleliu008/ndk-pkg");
   
    return NDKPKG_OK;
}

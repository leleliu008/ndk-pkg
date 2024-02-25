#include <string.h>

#include "ndk-pkg.h"

int ndkpkg_reinstall(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const NDKPKGInstallOptions * installOptions) {
    NDKPKGInstallOptions installOptionsCopy;

    memcpy(&installOptionsCopy, installOptions, sizeof(NDKPKGInstallOptions));

    installOptionsCopy.force = true;

    return ndkpkg_install(packageName, targetPlatform, &installOptionsCopy);
}

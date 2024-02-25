#include <stdio.h>
#include <string.h>

#include "ndk-pkg.h"

int ndkpkg_inspect_package(const char * package, const char * userSpecifiedTargetPlatformSpec, const char ** packageName, NDKPKGTargetPlatform * targetPlatform) {
    if (package == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (package[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////

    int slashIndex = -1;

    for (int i = 0; ;i++) {
        if (package[i] == '\0') {
            break;
        }

        if (package[i] == '/') {
            slashIndex = i;
            break;
        }
    }

    //////////////////////////////////////////////

    if (slashIndex == -1) {
        int ret = ndkpkg_check_if_the_given_argument_matches_package_name_pattern(package);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        if (userSpecifiedTargetPlatformSpec == NULL) {
            const char * NDKPKG_DEFAULT_TARGET = getenv("NDKPKG_DEFAULT_TARGET");

            if (NDKPKG_DEFAULT_TARGET == NULL || NDKPKG_DEFAULT_TARGET[0] == '\0') {
                targetPlatform->api = 21U;
                targetPlatform->abi = AndroidABI_arm64_v8a;

                (*packageName) = package;

                return NDKPKG_OK;
            } else {
                ret = ndkpkg_inspect_target_platform_spec(NDKPKG_DEFAULT_TARGET, targetPlatform);

                if (ret == NDKPKG_OK) {
                    (*packageName) = package;
                    return NDKPKG_OK;
                } else {
                    return NDKPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
                }
            }
        } else {
            ret = ndkpkg_inspect_target_platform_spec(userSpecifiedTargetPlatformSpec, targetPlatform);

            if (ret == NDKPKG_OK) {
                (*packageName) = package;
                return NDKPKG_OK;
            } else {
                return NDKPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
            }
        }
    }

    if (slashIndex == 0) {
        return NDKPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
    }

    const char * p = package + slashIndex + 1;

    int ret = ndkpkg_check_if_the_given_argument_matches_package_name_pattern(p);

    if (ret != NDKPKG_OK) {
        return NDKPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
    }

    char targetPlatformSpec[slashIndex + 1];

    strncpy(targetPlatformSpec, package, slashIndex);

    targetPlatformSpec[slashIndex] = '\0';

    ret = ndkpkg_inspect_target_platform_spec(targetPlatformSpec, targetPlatform);

    if (ret == NDKPKG_OK) {
        (*packageName) = p;
        return NDKPKG_OK;
    } else {
        return NDKPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
    }
}

#include <stdio.h>
#include <string.h>

#include "ndk-pkg.h"

int ndkpkg_inspect_target_platform_spec(const char * targetPlatformSpec, NDKPKGTargetPlatform * targetPlatform) {
    if (targetPlatformSpec == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (targetPlatformSpec[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    size_t n = strlen(targetPlatformSpec);

    if (n < 14 || n > 22) {
        fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
        return NDKPKG_ERROR;
    }

    if (strncmp(targetPlatformSpec, "android-", 8) != 0) {
        fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
        return NDKPKG_ERROR;
    }

    if (targetPlatformSpec[8] < '0' || targetPlatformSpec[8] > '9' || targetPlatformSpec[9] < '0' || targetPlatformSpec[9] > '9') {
        fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
        return NDKPKG_ERROR;
    }

    if (targetPlatformSpec[10] != '-') {
        fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////

    char api[3];
    api[0] = targetPlatformSpec[8];
    api[1] = targetPlatformSpec[9];
    api[2] = '\0';

    targetPlatform->api = atoi(api);

    const char * targetPlatformArch = targetPlatformSpec + 11;

    if (strcmp(targetPlatformArch, "arm64-v8a") == 0) {
        targetPlatform->abi = AndroidABI_arm64_v8a;
    } else if (strcmp(targetPlatformArch, "armeabi-v7a") == 0) {
        targetPlatform->abi = AndroidABI_armeabi_v7a;
    } else if (strcmp(targetPlatformArch, "x86_64") == 0) {
        targetPlatform->abi = AndroidABI_x86_64;
    } else if (strcmp(targetPlatformArch, "x86") == 0) {
        targetPlatform->abi = AndroidABI_x86;
    } else {
        fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
        return NDKPKG_ERROR;
    }

    return NDKPKG_OK;
}

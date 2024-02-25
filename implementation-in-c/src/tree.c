#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>

#include "ndk-pkg.h"

int ndkpkg_tree(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, size_t argc, char* argv[]) {
    int ret = ndkpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    if (targetPlatform == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    const char * targetABI;

    switch (targetPlatform->abi) {
        case AndroidABI_arm64_v8a:
            targetABI = "arm64-v8a";
            break;
        case AndroidABI_armeabi_v7a:
            targetABI = "armeabi-v7a";
            break;
        case AndroidABI_x86_64:
            targetABI = "x86_64";
            break;
        case AndroidABI_x86:
            targetABI = "x86";
            break;
    }

    size_t packageInstalledDIRCapacity = ndkpkgHomeDIRLength + strlen(packageName) + 33U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/installed/android-%2u-%s/%s", ndkpkgHomeDIR, targetPlatform->api, targetABI, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(packageInstalledDIR, &st) != 0) {
        return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t receiptFilePathLength = packageInstalledDIRCapacity + 21U;
    char   receiptFilePath[receiptFilePathLength];

    ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/.ndk-pkg/RECEIPT.yml", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (stat(receiptFilePath, &st) != 0 || (!S_ISREG(st.st_mode))) {
        return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
    }

    //////////////////////////////////////////////////////////////////////////////

    char treeCommandPath[PATH_MAX];

    const char * const uppmHomeDIR = getenv("UPPM_HOME");

    if (uppmHomeDIR == NULL || uppmHomeDIR[0] == '\0') {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL) {
            return NDKPKG_ERROR_ENV_HOME_NOT_SET;
        }

        if (userHomeDIR[0] == '\0') {
            return NDKPKG_ERROR_ENV_HOME_NOT_SET;
        }

        ret = snprintf(treeCommandPath, PATH_MAX, "%s/.uppm/installed/tree/bin/tree", userHomeDIR);
    } else {
        ret = snprintf(treeCommandPath, PATH_MAX, "%s/installed/tree/bin/tree", uppmHomeDIR);
    }

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t n = argc + 5U;
    char*  p[n];

    p[0] = treeCommandPath;
    p[1] = (char*)"--dirsfirst";
    p[2] = (char*)"-a";

    for (size_t i = 0U; i < argc; i++) {
        p[3U + i] = argv[i];
    }

    p[n - 2U] = packageInstalledDIR;
    p[n - 1U]   = NULL;

    execv(treeCommandPath, p);

    perror(treeCommandPath);

    return NDKPKG_ERROR;
}

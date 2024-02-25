#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/regex/regex.h"

#include "ndk-pkg.h"

int ndkpkg_check_if_the_given_argument_matches_package_name_pattern(const char * arg) {
    if (arg == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (arg[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    if (regex_matched(arg, NDKPKG_PACKAGE_NAME_PATTERN) == 0) {
        return NDKPKG_OK;
    } else {
        if (errno == 0) {
            return NDKPKG_ERROR_ARG_IS_INVALID;
        } else {
            perror(NULL);
            return NDKPKG_ERROR;
        }
    }
}

int ndkpkg_check_if_the_given_package_is_available(const char * packageName) {
    int ret = ndkpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    NDKPKGFormulaRepoList * formulaRepoList = NULL;

    ret = ndkpkg_formula_repo_list(&formulaRepoList);

    if (ret != NDKPKG_OK) {
        ndkpkg_formula_repo_list_free(formulaRepoList);
        return ret;
    }

    struct stat st;

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        char * formulaRepoPath = formulaRepoList->repos[i]->path;

        size_t formulaFilePathLength = strlen(formulaRepoPath) + strlen(packageName) + 15U;
        char   formulaFilePath[formulaFilePathLength];

        ret = snprintf(formulaFilePath, formulaFilePathLength, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (ret < 0) {
            perror(NULL);
            ndkpkg_formula_repo_list_free(formulaRepoList);
            return NDKPKG_ERROR;
        }

        if (lstat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            ndkpkg_formula_repo_list_free(formulaRepoList);
            return NDKPKG_OK;
        }
    }

    ndkpkg_formula_repo_list_free(formulaRepoList);
    return NDKPKG_ERROR_PACKAGE_NOT_AVAILABLE;
}

int ndkpkg_check_if_the_given_package_is_installed(const char * packageName, const NDKPKGTargetPlatform * targetPlatform) {
    int ret = ndkpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != NDKPKG_OK) {
        return ret;
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

    struct stat st;

    size_t packageInstalledDIRCapacity = ndkpkgHomeDIRLength + strlen(packageName) + 33U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/installed/android-%2u-%s/%s", ndkpkgHomeDIR, targetPlatform->api, targetABI, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (lstat(packageInstalledDIR, &st) == 0) {
        if (!S_ISLNK(st.st_mode)) {
            return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }
    } else {
        return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t receiptFilePathCapacity = packageInstalledDIRCapacity + 21U;
    char   receiptFilePath[receiptFilePathCapacity];

    ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/.ndk-pkg/RECEIPT.yml", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (lstat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            return NDKPKG_OK;
        } else {
            return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
        }
    } else {
        return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
    }
}

int ndkpkg_check_if_the_given_package_is_outdated(const char * packageName, const NDKPKGTargetPlatform * targetPlatform) {
    NDKPKGFormula * formula = NULL;
    NDKPKGReceipt * receipt = NULL;

    int ret = ndkpkg_formula_lookup(packageName, &formula);

    if (ret != NDKPKG_OK) {
        goto finalize;
    }

    ret = ndkpkg_receipt_parse(packageName, targetPlatform, &receipt);

    if (ret != NDKPKG_OK) {
        goto finalize;
    }

    if (strcmp(receipt->version, formula->version) == 0) {
        ret = NDKPKG_ERROR_PACKAGE_NOT_OUTDATED;
    }

finalize:
    ndkpkg_formula_free(formula);
    ndkpkg_receipt_free(receipt);
    return ret;
}

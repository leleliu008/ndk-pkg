#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "core/log.h"

#include "ndk-pkg.h"

static int _list_dir(const NDKPKGTargetPlatform * targetPlatform, const char * packageInstalledRootDIR, const size_t packageInstalledRootDIRCapacity, const char * platformDIRName, const size_t platformDIRNameCapacity, const bool verbose) {
    size_t packageInstalledRootSubDIRCapacity = packageInstalledRootDIRCapacity + platformDIRNameCapacity;
    char   packageInstalledRootSubDIR[packageInstalledRootSubDIRCapacity];

    int ret = snprintf(packageInstalledRootSubDIR, packageInstalledRootSubDIRCapacity, "%s/%s", packageInstalledRootDIR, platformDIRName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (!((stat(packageInstalledRootSubDIR, &st) == 0) && S_ISDIR(st.st_mode))) {
        return NDKPKG_OK;
    }

    DIR * dir = opendir(packageInstalledRootSubDIR);

    if (dir == NULL) {
        perror(packageInstalledRootSubDIR);
        return NDKPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return NDKPKG_OK;
            } else {
                perror(packageInstalledRootSubDIR);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t packageInstalledDIRCapacity = packageInstalledRootSubDIRCapacity + strlen(dir_entry->d_name) + 2U;
        char   packageInstalledDIR[packageInstalledDIRCapacity];

        ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", packageInstalledRootSubDIR, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return NDKPKG_ERROR;
        }

        if (lstat(packageInstalledDIR, &st) == 0) {
            if (!S_ISLNK(st.st_mode)) {
                continue;
            }
        } else {
            continue;
        }

        size_t receiptFilePathCapacity = packageInstalledDIRCapacity + 21U;
        char   receiptFilePath[receiptFilePathCapacity];

        ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/.ndk-pkg/RECEIPT.yml", packageInstalledDIR);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return NDKPKG_ERROR;
        }

        if (lstat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            //printf("%s\n", dir_entry->d_name);

            NDKPKGReceipt * receipt = NULL;

            ret = ndkpkg_receipt_parse(dir_entry->d_name, targetPlatform, &receipt);

            if (ret != NDKPKG_OK) {
                closedir(dir);
                ndkpkg_receipt_free(receipt);
                receipt = NULL;
                return ret;
            }

            NDKPKGFormula * formula = NULL;

            ret = ndkpkg_formula_lookup(dir_entry->d_name, &formula);

            if (ret == NDKPKG_OK) {
                if (strcmp(receipt->version, formula->version) != 0) {
                    printf("%s %s => %s\n", dir_entry->d_name, receipt->version, formula->version);
                }
            }

            ndkpkg_formula_free(formula);
            ndkpkg_receipt_free(receipt);

            formula = NULL;
            receipt = NULL;
        }
    }
}

static int ndkpkg_list_all__outdated_packages(const char * packageInstalledRootDIR, const size_t packageInstalledRootDIRCapacity, const bool verbose) {
    DIR * dir = opendir(packageInstalledRootDIR);

    if (dir == NULL) {
        perror(packageInstalledRootDIR);
        return NDKPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return NDKPKG_OK;
            } else {
                perror(packageInstalledRootDIR);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        NDKPKGTargetPlatform targetPlatform;

        int ret = ndkpkg_inspect_target_platform_spec(dir_entry->d_name, &targetPlatform);

        if (ret != NDKPKG_OK) {
            continue;
        }

        ret = _list_dir(&targetPlatform, packageInstalledRootDIR, packageInstalledRootDIRCapacity, dir_entry->d_name, strlen(dir_entry->d_name) + 1U, verbose);

        if (ret != NDKPKG_OK) {
            closedir(dir);
            return ret;
        }
    }
}

int ndkpkg_list_the__outdated_packages(const NDKPKGTargetPlatform * targetPlatform, const bool verbose) {
    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    size_t packageInstalledRootDIRCapacity = ndkpkgHomeDIRLength + 11U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (!((stat(packageInstalledRootDIR, &st) == 0) && S_ISDIR(st.st_mode))) {
        return NDKPKG_OK;
    }

    if (targetPlatform == NULL) {
        return ndkpkg_list_all__outdated_packages(packageInstalledRootDIR, packageInstalledRootDIRCapacity, verbose);
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

    char platformDIRName[23];

    ret = snprintf(platformDIRName, 23, "android-%2u-%s", targetPlatform->api, targetABI);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    return _list_dir(targetPlatform, packageInstalledRootDIR, packageInstalledRootDIRCapacity, platformDIRName, 23, verbose);
}

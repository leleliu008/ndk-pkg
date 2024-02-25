#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "ndk-pkg.h"

int ndkpkg_uninstall(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const bool verbose) {
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


    size_t packageInstalledRootDIRCapacity = ndkpkgHomeDIRLength + 32U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed/android-%2u-%s", ndkpkgHomeDIR, targetPlatform->api, targetABI);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t packageInstalledLinkDIRCapacity = packageInstalledRootDIRCapacity + strlen(packageName) + 2U;
    char   packageInstalledLinkDIR[packageInstalledLinkDIRCapacity];

    ret = snprintf(packageInstalledLinkDIR, packageInstalledLinkDIRCapacity, "%s/%s", packageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (lstat(packageInstalledLinkDIR, &st) == 0) {
        if (S_ISLNK(st.st_mode)) {
            size_t receiptFilePathCapacity = packageInstalledLinkDIRCapacity + 21U;
            char   receiptFilePath[receiptFilePathCapacity];

            ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/.ndk-pkg/RECEIPT.yml", packageInstalledLinkDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (lstat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
                char buf[256] = {0};

                ssize_t readSize = readlink(packageInstalledLinkDIR, buf, 255);

                if (readSize == -1) {
                    perror(packageInstalledLinkDIR);
                    return NDKPKG_ERROR;
                } else if (readSize != 64) {
                    // package is broken by other tools?
                    return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
                }

                size_t packageInstalledRealDIRCapacity = packageInstalledRootDIRCapacity + 66U;
                char   packageInstalledRealDIR[packageInstalledRealDIRCapacity];

                ret = snprintf(packageInstalledRealDIR, packageInstalledRealDIRCapacity, "%s/%s", packageInstalledRootDIR, buf);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if (lstat(packageInstalledRealDIR, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        if (unlink(packageInstalledLinkDIR) == 0) {
                            if (verbose) {
                                printf("rm %s\n", packageInstalledLinkDIR);
                            }
                        } else {
                            perror(packageInstalledLinkDIR);
                            return NDKPKG_ERROR;
                        }

                        return ndkpkg_rm_r(packageInstalledRealDIR, verbose);
                    } else {
                        // package is broken by other tools?
                        return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
                    }
                } else {
                    // package is broken by other tools?
                    return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
                }
            } else {
                // package is broken. is not installed completely?
                return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
            }
        } else {
            return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }
    } else {
        return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }
}

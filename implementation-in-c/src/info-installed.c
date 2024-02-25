#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include <jansson.h>

#include "core/log.h"

#include "ndk-pkg.h"

int ndkpkg_installed_info(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const char * key) {
    if (packageName == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (packageName[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    if (targetPlatform == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (key == NULL || key[0] == '\0') {
        key = "--yaml";
    }

    if (strcmp(key, "--prefix") == 0) {
        char   ndkpkgHomeDIR[PATH_MAX];
        size_t ndkpkgHomeDIRLength;

        int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        /////////////////////////////////////////////////////////////////////

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

        /////////////////////////////////////////////////////////////////////

        struct stat st;

        if (stat(packageInstalledDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
            }
        } else {
            return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }

        size_t receiptFilePathLength = packageInstalledDIRCapacity + 21U;
        char   receiptFilePath[receiptFilePathLength];

        ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/.ndk-pkg/RECEIPT.yml", packageInstalledDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (stat(receiptFilePath, &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                printf("%s\n", packageInstalledDIR);
                return NDKPKG_OK;
            } else {
                return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
            }
        } else {
            return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
        }
    } else if (strcmp(key, "--files") == 0) {
        char   ndkpkgHomeDIR[PATH_MAX];
        size_t ndkpkgHomeDIRLength;

        int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        /////////////////////////////////////////////////////////////////////

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

        /////////////////////////////////////////////////////////////////////

        struct stat st;

        if (stat(packageInstalledDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
            }
        } else {
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

        size_t manifestFilePathLength = packageInstalledDIRCapacity + 22U;
        char   manifestFilePath[manifestFilePathLength];

        ret = snprintf(manifestFilePath, manifestFilePathLength, "%s/.ndk-pkg/MANIFEST.txt", packageInstalledDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (stat(manifestFilePath, &st) != 0 || (!S_ISREG(st.st_mode))) {
            return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
        }

        int manifestFD = open(manifestFilePath, O_RDONLY);

        if (manifestFD == -1) {
            perror(manifestFilePath);
            return NDKPKG_ERROR;
        }

        char buf[1024];

        for (;;) {
            ssize_t readSize = read(manifestFD, buf, 1024);

            if (readSize == -1) {
                perror(manifestFilePath);
                close(manifestFD);
                return NDKPKG_ERROR;
            }

            if (readSize == 0) {
                close(manifestFD);
                return NDKPKG_OK;
            }

            ssize_t writeSize = write(STDOUT_FILENO, buf, readSize);

            if (writeSize == -1) {
                perror(manifestFilePath);
                close(manifestFD);
                return NDKPKG_ERROR;
            }

            if (writeSize != readSize) {
                fprintf(stderr, "not fully written to stdout.\n");
                close(manifestFD);
                return NDKPKG_ERROR;
            }
        }
    } else if (strcmp(key, "--path") == 0) {
        char   ndkpkgHomeDIR[PATH_MAX] = {0};
        size_t ndkpkgHomeDIRLength;

        int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        /////////////////////////////////////////////////////////////////////

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

        /////////////////////////////////////////////////////////////////////

        struct stat st;

        if (stat(packageInstalledDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
            }
        } else {
            return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }

        size_t receiptFilePathLength = packageInstalledDIRCapacity + 21U;
        char   receiptFilePath[receiptFilePathLength];

        ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/.ndk-pkg/RECEIPT.yml", packageInstalledDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (stat(receiptFilePath, &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                printf("%s\n", receiptFilePath);
                return NDKPKG_OK;
            } else {
                return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
            }
        } else {
            return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
        }
    } else if (strcmp(key, "--yaml") == 0) {
        char   ndkpkgHomeDIR[PATH_MAX];
        size_t ndkpkgHomeDIRLength;

        int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

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

        if (stat(packageInstalledDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                return NDKPKG_ERROR_PACKAGE_IS_BROKEN;
            }
        } else {
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

        int receiptFD = open(receiptFilePath, O_RDONLY);

        if (receiptFD == -1) {
            perror(receiptFilePath);
            return NDKPKG_ERROR;
        }

        char buf[1024];

        for (;;) {
            ssize_t readSize = read(receiptFD, buf, 1024);

            if (readSize == -1) {
                perror(receiptFilePath);
                close(receiptFD);
                return NDKPKG_ERROR;
            }

            if (readSize == 0) {
                close(receiptFD);
                return NDKPKG_OK;
            }

            ssize_t writeSize = write(STDOUT_FILENO, buf, readSize);

            if (writeSize == -1) {
                perror(NULL);
                close(receiptFD);
                return NDKPKG_ERROR;
            }

            if (writeSize != readSize) {
                perror(NULL);
                close(receiptFD);
                return NDKPKG_ERROR;
            }
        }
    }

    NDKPKGReceipt * receipt = NULL;

    int ret = ndkpkg_receipt_parse(packageName, targetPlatform, &receipt);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    if (strcmp(key, "--json") == 0) {
        json_t * root = json_object();

        json_object_set_new(root, "pkgname", json_string(packageName));

        json_object_set_new(root, "summary", json_string(receipt->summary));
        json_object_set_new(root, "version", json_string(receipt->version));
        json_object_set_new(root, "license", json_string(receipt->license));

        json_object_set_new(root, "web-url", json_string(receipt->web_url));

        json_object_set_new(root, "git-url", json_string(receipt->git_url));
        json_object_set_new(root, "git-sha", json_string(receipt->git_sha));
        json_object_set_new(root, "git-ref", json_string(receipt->git_ref));
        json_object_set_new(root, "git-nth", json_integer(receipt->git_nth));

        json_object_set_new(root, "src-url", json_string(receipt->src_url));
        json_object_set_new(root, "src-uri", json_string(receipt->src_uri));
        json_object_set_new(root, "src-sha", json_string(receipt->src_sha));

        json_object_set_new(root, "fix-url", json_string(receipt->fix_url));
        json_object_set_new(root, "fix-uri", json_string(receipt->fix_uri));
        json_object_set_new(root, "fix-sha", json_string(receipt->fix_sha));

        json_object_set_new(root, "res-url", json_string(receipt->res_url));
        json_object_set_new(root, "res-uri", json_string(receipt->res_uri));
        json_object_set_new(root, "res-sha", json_string(receipt->res_sha));

        json_object_set_new(root, "dep-pkg", json_string(receipt->dep_pkg));
        json_object_set_new(root, "dep-upp", json_string(receipt->dep_upp));
        json_object_set_new(root, "dep-pym", json_string(receipt->dep_pym));
        json_object_set_new(root, "dep-plm", json_string(receipt->dep_plm));

        json_object_set_new(root, "ppflags", json_string(receipt->ppflags));
        json_object_set_new(root, "ccflags", json_string(receipt->ccflags));
        json_object_set_new(root, "xxflags", json_string(receipt->xxflags));
        json_object_set_new(root, "ldfalgs", json_string(receipt->ldflags));

        json_object_set_new(root, "bsystem", json_string(receipt->bscript));
        json_object_set_new(root, "bscript", json_string(receipt->bscript));
        json_object_set_new(root, "binbstd", json_boolean(receipt->binbstd));
        json_object_set_new(root, "parallel", json_boolean(receipt->parallel));
        json_object_set_new(root, "symlink", json_boolean(receipt->symlink));

        json_object_set_new(root, "do12345", json_string(receipt->do12345));
        json_object_set_new(root, "dopatch", json_string(receipt->dopatch));
        json_object_set_new(root, "install", json_string(receipt->install));

        json_object_set_new(root, "builtby", json_string(receipt->builtBy));
        json_object_set_new(root, "builtat", json_string(receipt->builtAt));

        char * jsonStr = json_dumps(root, 0);

        if (jsonStr == NULL) {
            ret = NDKPKG_ERROR;
        } else {
            printf("%s\n", jsonStr);
            free(jsonStr);
        }

        json_decref(root);
    } else if (strcmp(key, "summary") == 0) {
        printf("%s\n", receipt->summary);
    } else if (strcmp(key, "version") == 0) {
        printf("%s\n", receipt->version);
    } else if (strcmp(key, "license") == 0) {
        if (receipt->license != NULL) {
            printf("%s\n", receipt->license);
        }
    } else if (strcmp(key, "web-url") == 0) {
        if (receipt->web_url != NULL) {
            printf("%s\n", receipt->web_url);
        }
    } else if (strcmp(key, "git-url") == 0) {
        if (receipt->git_url != NULL) {
            printf("%s\n", receipt->git_url);
        }
    } else if (strcmp(key, "git-sha") == 0) {
        if (receipt->git_sha != NULL) {
            printf("%s\n", receipt->git_sha);
        }
    } else if (strcmp(key, "git-ref") == 0) {
        if (receipt->git_ref != NULL) {
            printf("%s\n", receipt->git_ref);
        }
    } else if (strcmp(key, "git-nth") == 0) {
        printf("%zu\n", receipt->git_nth);
    } else if (strcmp(key, "src-url") == 0) {
        if (receipt->src_url != NULL) {
            printf("%s\n", receipt->src_url);
        }
    } else if (strcmp(key, "src-uri") == 0) {
        if (receipt->src_uri != NULL) {
            printf("%s\n", receipt->src_uri);
        }
    } else if (strcmp(key, "src-sha") == 0) {
        if (receipt->src_sha != NULL) {
            printf("%s\n", receipt->src_sha);
        }
    } else if (strcmp(key, "fix-url") == 0) {
        if (receipt->fix_url != NULL) {
            printf("%s\n", receipt->fix_url);
        }
    } else if (strcmp(key, "fix-uri") == 0) {
        if (receipt->fix_uri != NULL) {
            printf("%s\n", receipt->fix_uri);
        }
    } else if (strcmp(key, "fix-sha") == 0) {
        if (receipt->fix_sha != NULL) {
            printf("%s\n", receipt->fix_sha);
        }
    } else if (strcmp(key, "res-url") == 0) {
        if (receipt->res_url != NULL) {
            printf("%s\n", receipt->res_url);
        }
    } else if (strcmp(key, "res-uri") == 0) {
        if (receipt->res_uri != NULL) {
            printf("%s\n", receipt->res_uri);
        }
    } else if (strcmp(key, "res-sha") == 0) {
        if (receipt->res_sha != NULL) {
            printf("%s\n", receipt->res_sha);
        }
    } else if (strcmp(key, "dep-pkg") == 0) {
        if (receipt->dep_pkg != NULL) {
            printf("%s\n", receipt->dep_pkg);
        }
    } else if (strcmp(key, "dep-upp") == 0) {
        if (receipt->dep_upp != NULL) {
            printf("%s\n", receipt->dep_upp);
        }
    } else if (strcmp(key, "dep-pym") == 0) {
        if (receipt->dep_pym != NULL) {
            printf("%s\n", receipt->dep_pym);
        }
    } else if (strcmp(key, "dep-plm") == 0) {
        if (receipt->dep_plm != NULL) {
            printf("%s\n", receipt->dep_plm);
        }
    } else if (strcmp(key, "bsystem") == 0) {
        if (receipt->bsystem != NULL) {
            printf("%s\n", receipt->bsystem);
        }
    } else if (strcmp(key, "bscript") == 0) {
        if (receipt->bscript != NULL) {
            printf("%s\n", receipt->bscript);
        }
    } else if (strcmp(key, "binbstd") == 0) {
        printf("%d\n", receipt->binbstd);
    } else if (strcmp(key, "symlink") == 0) {
        printf("%d\n", receipt->symlink);
    } else if (strcmp(key, "parallel") == 0) {
        printf("%d\n", receipt->parallel);
    } else if (strcmp(key, "ppflags") == 0) {
        if (receipt->ppflags != NULL) {
            printf("%s\n", receipt->ppflags);
        }
    } else if (strcmp(key, "ccflags") == 0) {
        if (receipt->ccflags != NULL) {
            printf("%s\n", receipt->ccflags);
        }
    } else if (strcmp(key, "xxflags") == 0) {
        if (receipt->xxflags != NULL) {
            printf("%s\n", receipt->xxflags);
        }
    } else if (strcmp(key, "ldflags") == 0) {
        if (receipt->ldflags != NULL) {
            printf("%s\n", receipt->ldflags);
        }
    } else if (strcmp(key, "do12345") == 0) {
        if (receipt->do12345 != NULL) {
            printf("%s\n", receipt->do12345);
        }
    } else if (strcmp(key, "dopatch") == 0) {
        if (receipt->dopatch != NULL) {
            printf("%s\n", receipt->dopatch);
        }
    } else if (strcmp(key, "install") == 0) {
        if (receipt->install != NULL) {
            printf("%s\n", receipt->install);
        }
    } else if (strcmp(key, "builtfor") == 0) {
        printf("%s\n", receipt->builtFor);
    } else if (strcmp(key, "builtby") == 0) {
        printf("%s\n", receipt->builtBy);
    } else if (strcmp(key, "builtat") == 0) {
        printf("%s\n", receipt->builtAt);
    } else if (strcmp(key, "builtat-rfc-3339") == 0) {
        time_t tt = (time_t)atol(receipt->builtAt);
        struct tm *tms = localtime(&tt);

        char buff[26] = {0};
        strftime(buff, 26, "%Y-%m-%d %H:%M:%S%z", tms);

        buff[24] = buff[23];
        buff[23] = buff[22];
        buff[22] = ':';

        printf("%s\n", buff);
    } else if (strcmp(key, "builtat-rfc-3339-utc") == 0) {
        time_t tt = (time_t)atol(receipt->builtAt);
        struct tm *tms = gmtime(&tt);

        char buff[26] = {0};
        strftime(buff, 26, "%Y-%m-%d %H:%M:%S%z", tms);

        buff[24] = buff[23];
        buff[23] = buff[22];
        buff[22] = ':';

        printf("%s\n", buff);
    } else if (strcmp(key, "builtat-iso-8601") == 0) {
        time_t tt = (time_t)atol(receipt->builtAt);
        struct tm *tms = localtime(&tt);

        char buff[26] = {0};
        strftime(buff, 26, "%Y-%m-%dT%H:%M:%S%z", tms);

        buff[24] = buff[23];
        buff[23] = buff[22];
        buff[22] = ':';

        printf("%s\n", buff);
    } else if (strcmp(key, "builtat-iso-8601-utc") == 0) {
        time_t tt = (time_t)atol(receipt->builtAt);
        struct tm *tms = gmtime(&tt);

        char buff[21] = {0};
        strftime(buff, 21, "%Y-%m-%dT%H:%M:%SZ", tms);

        printf("%s\n", buff);
    } else {
        ret = NDKPKG_ERROR_ARG_IS_UNKNOWN;
    }

    ndkpkg_receipt_free(receipt);

    return ret;
}

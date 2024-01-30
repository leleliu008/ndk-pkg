#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "ndk-manager.h"
#include "log.h"
#include "fs.h"

static char* getNDKVersionFromFile(char* filepath) {
    FILE* file = fopen(filepath, "r");

    if (file == NULL) {
        return NULL;
    }

    char* ndkVersion = NULL;

    regex_t regex;

    if (regcomp(&regex, "Pkg\\.Revision = ", 0) == 0) {
        char line[50] = {0};
        while(fgets(line, 50, file)) {
            regmatch_t regmatch[2];
            if (regexec(&regex, line, 2, regmatch, 0) == 0) {
                printf("regmatch[0].rm_so=%d\n", regmatch[0].rm_so);
                printf("regmatch[0].rm_eo=%d\n", regmatch[0].rm_eo);
                if (regmatch[0].rm_so >= 0 && regmatch[0].rm_eo > regmatch[0].rm_so) {
                    int lineLength = strlen(line);
                    ndkVersion = (char*)calloc(lineLength, sizeof(char));
                    strncpy(ndkVersion, line + regmatch[0].rm_eo, lineLength - 1 - regmatch[0].rm_eo);
                    break;
                }
            }
        }
    }

    regfree(&regex);

    fclose(file);

    return ndkVersion;
}

NDKToolchainInfo* make_ndk_toolchain_info(const char* hostOSKind, const char* ndkRootDir) {
    printf("ndkRootDir = %s\n", ndkRootDir);

    if (hostOSKind == NULL || strcmp(hostOSKind, "") == 0) {
        return NULL;
    }

    if (ndkRootDir == NULL || strcmp(ndkRootDir, "") == 0) {
        return NULL;
    }

    if (!exists_and_is_a_directory(ndkRootDir)) {
        return NULL;
    }

    size_t ndkRootDirLength = strlen(ndkRootDir);

    char* filepath_source_properties = (char*)calloc(ndkRootDirLength + 20, sizeof(char));
    sprintf(filepath_source_properties, "%s/source.properties", ndkRootDir);

    char* filepath_cmake_toolchain = (char*)calloc(ndkRootDirLength + 40, sizeof(char));
    sprintf(filepath_cmake_toolchain, "%s/build/cmake/android.toolchain.cmake", ndkRootDir);

    char* ndkToolchainHostTag = (char*)calloc(15, sizeof(char));

    if (strcmp(hostOSKind, "Linux") == 0) {
        strcpy(ndkToolchainHostTag, "linux-x86_64");
    } else if(strcmp(hostOSKind, "Darwin") == 0) {
        strcpy(ndkToolchainHostTag, "darwin-x86_64");
    } else {
        char error_message[30] = {0};
        sprintf(error_message, "unsupported system: %s", hostOSKind);
        perror(error_message);
    }

    char* ndkToolchainRootDir = (char*)calloc(ndkRootDirLength + 60, sizeof(char));
    sprintf(ndkToolchainRootDir, "%s/toolchains/llvm/prebuilt/%s", ndkRootDir, ndkToolchainHostTag);

    char* ndkToolchainBinDir = (char*)calloc(ndkRootDirLength + 60, sizeof(char));
    sprintf(ndkToolchainBinDir, "%s/toolchains/llvm/prebuilt/%s/bin", ndkRootDir, ndkToolchainHostTag);

    char* ndkToolchainSysrootDir = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    sprintf(ndkToolchainSysrootDir, "%s/toolchains/llvm/prebuilt/%s/sysroot", ndkRootDir, ndkToolchainHostTag);

    char* cc      = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* cxx     = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* cpp     = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* as      = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* ld      = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* ar      = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* nm      = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* size    = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* strip   = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* ranlib  = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* strings = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* objdump = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* objcopy = (char*)calloc(ndkRootDirLength + 70, sizeof(char));
    char* readelf = (char*)calloc(ndkRootDirLength + 70, sizeof(char));

    sprintf(cc,      "%s/clang",        ndkToolchainBinDir);
    sprintf(cxx,     "%s/clang++",      ndkToolchainBinDir);
    sprintf(cpp,     "%s/clang -E",     ndkToolchainBinDir);
    sprintf(as,      "%s/llvm-as",      ndkToolchainBinDir);
    sprintf(ld,      "%s/ld.lld",       ndkToolchainBinDir);
    sprintf(ar,      "%s/llvm-ar",      ndkToolchainBinDir);
    sprintf(nm,      "%s/llvm-nm",      ndkToolchainBinDir);
    sprintf(size,    "%s/llvm-size",    ndkToolchainBinDir);
    sprintf(strip,   "%s/llvm-strip",   ndkToolchainBinDir);
    sprintf(ranlib,  "%s/llvm-ranlib",  ndkToolchainBinDir);
    sprintf(strings, "%s/llvm-strings", ndkToolchainBinDir);
    sprintf(objdump, "%s/llvm-objdump", ndkToolchainBinDir);
    sprintf(objcopy, "%s/llvm-objcopy", ndkToolchainBinDir);
    sprintf(readelf, "%s/llvm-readelf", ndkToolchainBinDir);

    printf("ndkToolchainRootDir = %s\n", ndkToolchainRootDir);
    printf("ndkToolchainBinDir  = %s\n", ndkToolchainBinDir);
    printf("ndkToolchainSysrootDir = %s\n", ndkToolchainSysrootDir);

    if (exists_and_is_a_regular_file(filepath_source_properties) &&
        exists_and_is_a_regular_file(filepath_cmake_toolchain) &&
        exists_and_is_a_directory(ndkToolchainRootDir) &&
        exists_and_is_a_directory(ndkToolchainBinDir) &&
        exists_and_is_a_directory(ndkToolchainSysrootDir)) {
        char* ndkVersion = getNDKVersionFromFile(filepath_source_properties);

        if (ndkVersion == NULL) {
            goto cleanup;
        }

        int  ndkVersionLength = strlen(ndkVersion) + 1;
        char ndkVersionCopy[ndkVersionLength];
        memset(ndkVersionCopy, 0, ndkVersionLength);
        strcpy(ndkVersionCopy, ndkVersion);
        char* ndkVersionMajorStr = strtok(ndkVersionCopy, ".");
        if (ndkVersionMajorStr == NULL) {
            goto cleanup;
        }

        NDKToolchainInfo* ndkToolchainInfo = (NDKToolchainInfo*)calloc(1, sizeof(NDKToolchainInfo));
        ndkToolchainInfo->root = ndkRootDir;
        ndkToolchainInfo->bin_dir = ndkToolchainBinDir;
        ndkToolchainInfo->sysroot = ndkToolchainSysrootDir;
        ndkToolchainInfo->cmake_toolchain_file = filepath_cmake_toolchain ;
        ndkToolchainInfo->source_properties    = filepath_source_properties ;
        ndkToolchainInfo->host_tag = ndkToolchainHostTag ;
        ndkToolchainInfo->version_string = ndkVersion;
        ndkToolchainInfo->version_major = atoi(ndkVersionMajorStr) ;
        ndkToolchainInfo->cc = cc;
        ndkToolchainInfo->cxx = cxx;
        ndkToolchainInfo->cpp = cpp;
        ndkToolchainInfo->as = as;
        ndkToolchainInfo->ld = ld;
        ndkToolchainInfo->ar = ar;
        ndkToolchainInfo->nm = nm;
        ndkToolchainInfo->size = size;
        ndkToolchainInfo->strip = strip;
        ndkToolchainInfo->ranlib = ranlib;
        ndkToolchainInfo->strings = strings;
        ndkToolchainInfo->objcopy = objcopy;
        ndkToolchainInfo->objdump = objdump;
        ndkToolchainInfo->readelf = readelf;

        return ndkToolchainInfo;
    }

cleanup:
    if (filepath_source_properties != NULL) {
        free(filepath_source_properties);
        filepath_source_properties = NULL;
    }

    if (filepath_cmake_toolchain) {
        free(filepath_cmake_toolchain);
        filepath_cmake_toolchain = NULL;
    }

    if (ndkToolchainHostTag != NULL) {
        free(ndkToolchainHostTag);
        ndkToolchainHostTag = NULL;
    }

    if (ndkToolchainRootDir != NULL) {
        free(ndkToolchainRootDir);
        ndkToolchainRootDir = NULL;
    }

    if (ndkToolchainBinDir != NULL) {
        free(ndkToolchainBinDir);
        ndkToolchainBinDir = NULL;
    }

    if (ndkToolchainSysrootDir != NULL) {
        free(ndkToolchainSysrootDir);
        ndkToolchainSysrootDir = NULL;
    }

    return NULL;
}

NDKToolchainInfo* find_ndk_toolchain_info(const char* hostOSKind) {
    NDKToolchainInfo* ndkToolchainInfo = NULL;

    ndkToolchainInfo = make_ndk_toolchain_info(hostOSKind, getenv("ANDROID_NDK_HOME"));

    if (NULL != ndkToolchainInfo) {
        return  ndkToolchainInfo;
    }

    ndkToolchainInfo = make_ndk_toolchain_info(hostOSKind, getenv("ANDROID_NDK_ROOT"));

    if (NULL != ndkToolchainInfo) {
        return ndkToolchainInfo;
    }

    char* androidSdkRootDir = getenv("ANDROID_HOME");

    if (androidSdkRootDir != NULL && strcmp(androidSdkRootDir, "") != 0) {
        char* androidNdkRootDir = (char*)calloc(strlen(androidSdkRootDir) + 12, sizeof(char));
        if (androidNdkRootDir != NULL) {
            sprintf(androidNdkRootDir, "%s/ndk-bundle", androidSdkRootDir);
            ndkToolchainInfo = make_ndk_toolchain_info(hostOSKind, androidNdkRootDir);
            if (ndkToolchainInfo == NULL) {
                free(androidNdkRootDir);
                androidNdkRootDir = NULL;
            } else {
                return ndkToolchainInfo;
            }
        }
    }

    char* userHomeDir = getenv("HOME");

    if (userHomeDir != NULL && strcmp(userHomeDir, "") != 0) {
        char* androidNdkRootDir = (char*)calloc(strlen(userHomeDir) + 30, sizeof(char));
        if (androidNdkRootDir != NULL) {
            sprintf(androidNdkRootDir, "%s/.ndk-pkg/android-ndk-r23b", userHomeDir);
            ndkToolchainInfo = make_ndk_toolchain_info(hostOSKind, androidNdkRootDir);
            if (ndkToolchainInfo == NULL) {
                free(androidNdkRootDir);
                androidNdkRootDir = NULL;
            } else {
                return ndkToolchainInfo;
            }
        }
    }

    return NULL;
}

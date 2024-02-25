#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "ndk-pkg.h"

static int readNDKVersionFromFile(const char * filepath, char outBuf[50], size_t outBufSize) {
    FILE * file = fopen(filepath, "r");

    if (file == NULL) {
        perror(filepath);
        return NDKPKG_ERROR;
    }

    char tmpBuf[51] = {0};

    for (;;) {
        if (fgets(tmpBuf, 50, file) == NULL) {
            if (ferror(file)) {
                perror(filepath);
                fclose(file);
                return NDKPKG_ERROR;
            } else {
                fclose(file);
                return NDKPKG_ERROR;
            }
        }

        if (strncmp(tmpBuf, "Pkg.Revision = ", 15) == 0) {
            char * p = tmpBuf + 15;

            size_t x = strlen(p);

            if (p[x - 1] == '\n') {
                p[x - 1] =  '\0';
                x--;
            }

            size_t y = outBufSize - 1;

            size_t n = (x > y) ? y : x;

            strncpy(outBuf, p, n);

            outBuf[n] = '\0';

            fclose(file);
            return NDKPKG_OK;
        }
    }
}

void ndkpkg_android_ndk_toolchain_free(NDKPKGAndroidNDKToolChain * toolchain) {
    if (toolchain == NULL) {
        return;
    }

    if (toolchain->cc != NULL) {
        free(toolchain->cc);
        toolchain->cc = NULL;
    }

    if (toolchain->cxx != NULL) {
        free(toolchain->cxx);
        toolchain->cxx = NULL;
    }

    if (toolchain->as != NULL) {
        free(toolchain->as);
        toolchain->as = NULL;
    }

    if (toolchain->ar != NULL) {
        free(toolchain->ar);
        toolchain->ar = NULL;
    }

    if (toolchain->ranlib != NULL) {
        free(toolchain->ranlib);
        toolchain->ranlib = NULL;
    }

    if (toolchain->ld != NULL) {
        free(toolchain->ld);
        toolchain->ld = NULL;
    }

    if (toolchain->nm != NULL) {
        free(toolchain->nm);
        toolchain->nm = NULL;
    }

    if (toolchain->size != NULL) {
        free(toolchain->size);
        toolchain->size = NULL;
    }

    if (toolchain->strip != NULL) {
        free(toolchain->strip);
        toolchain->strip = NULL;
    }

    if (toolchain->strings != NULL) {
        free(toolchain->strings);
        toolchain->strings = NULL;
    }

    if (toolchain->objdump != NULL) {
        free(toolchain->objdump);
        toolchain->objdump = NULL;
    }

    if (toolchain->objcopy != NULL) {
        free(toolchain->objcopy);
        toolchain->objcopy = NULL;
    }

    if (toolchain->readelf != NULL) {
        free(toolchain->readelf);
        toolchain->readelf = NULL;
    }

    if (toolchain->sysroot != NULL) {
        free(toolchain->sysroot);
        toolchain->sysroot = NULL;
    }

    if (toolchain->rootdir != NULL) {
        free(toolchain->rootdir);
        toolchain->rootdir = NULL;
    }

    if (toolchain->version != NULL) {
        free(toolchain->version);
        toolchain->version = NULL;
    }

    if (toolchain->ccflags != NULL) {
        free(toolchain->ccflags);
        toolchain->ccflags = NULL;
    }

    if (toolchain->cxxflags != NULL) {
        free(toolchain->cxxflags);
        toolchain->cxxflags = NULL;
    }

    if (toolchain->cppflags != NULL) {
        free(toolchain->cppflags);
        toolchain->cppflags = NULL;
    }

    if (toolchain->ldflags != NULL) {
        free(toolchain->ldflags);
        toolchain->ldflags = NULL;
    }
}

void ndkpkg_android_ndk_toolchain_dump(NDKPKGAndroidNDKToolChain * toolchain) {
    if (toolchain == NULL) {
        return;
    }

    printf("ANDROID_NDK_VERSION: %s\n", toolchain->version);
    printf("ANDROID_NDK_HOME:    %s\n", toolchain->rootdir);
    printf("ANDROID_NDK_CC:      %s\n", toolchain->cc);
    printf("ANDROID_NDK_CXX:     %s\n", toolchain->cxx);
    printf("ANDROID_NDK_AS:      %s\n", toolchain->as);
    printf("ANDROID_NDK_AR:      %s\n", toolchain->ar);
    printf("ANDROID_NDK_LD:      %s\n", toolchain->ld);
    printf("ANDROID_NDK_NM:      %s\n", toolchain->nm);
    printf("ANDROID_NDK_SIZE:    %s\n", toolchain->size);
    printf("ANDROID_NDK_STRIP:   %s\n", toolchain->strip);
    printf("ANDROID_NDK_RANLIB:  %s\n", toolchain->ranlib);
    printf("ANDROID_NDK_STRINGS: %s\n", toolchain->strings);
    printf("ANDROID_NDK_OBJDUMP: %s\n", toolchain->objdump);
    printf("ANDROID_NDK_OBJCOPY: %s\n", toolchain->objcopy);
    printf("ANDROID_NDK_READELF: %s\n", toolchain->readelf);
    printf("ANDROID_NDK_SYSROOT: %s\n", toolchain->sysroot);
}

static int ndkpkg_android_ndk_toolchain_attach_tools(NDKPKGAndroidNDKToolChain * toolchain, const char * ndkRootDIR, const size_t ndkRootDIRLength, const char * ndkToolchainHostTag) {
    size_t binDIRCapacity = ndkRootDIRLength + 50U;
    char   binDIR[binDIRCapacity];

    int ret = snprintf(binDIR, binDIRCapacity, "%s/toolchains/llvm/prebuilt/%s/bin", ndkRootDIR, ndkToolchainHostTag);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(binDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            return NDKPKG_ERROR;
        }
    } else {
        return NDKPKG_ERROR;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    const char * a[13] = { "clang", "clang++", "llvm-as", "llvm-ar", "ld.lld", "llvm-nm", "llvm-size", "llvm-strip", "llvm-ranlib", "llvm-strings", "llvm-objdump", "llvm-objcopy", "llvm-readelf" };

    size_t bufCapacity = binDIRCapacity + 13U;

    for (int i = 0; i < 13; i++) {
        char * p = (char*)calloc(bufCapacity, sizeof(char));

        if (p == NULL) {
            return NDKPKG_ERROR_MEMORY_ALLOCATE;
        }

        ret = snprintf(p, bufCapacity, "%s/%s", binDIR, a[i]);

        if (ret < 0) {
            perror(NULL);
            free(p);
            return NDKPKG_ERROR;
        }

        if (stat(p, &st) != 0) {
            free(p);
            return NDKPKG_ERROR;
        }

        switch (i) {
            case 0: toolchain->cc = p; break;
            case 1: toolchain->cxx = p; break;
            case 2: toolchain->as = p; break;
            case 3: toolchain->ar = p; break;
            case 4: toolchain->ld = p; break;
            case 5: toolchain->nm = p; break;
            case 6: toolchain->size = p; break;
            case 7: toolchain->strip = p; break;
            case 8: toolchain->ranlib = p; break;
            case 9: toolchain->strings = p; break;
            case 10: toolchain->objdump = p; break;
            case 11: toolchain->objcopy = p; break;
            case 12: toolchain->readelf = p; break;
        }
    }

    return NDKPKG_OK;
}

int ndkpkg_android_ndk_toolchain_make(NDKPKGAndroidNDKToolChain * toolchain, const char * ndkRootDIR) {
    if (ndkRootDIR == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (ndkRootDIR[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    if (toolchain == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    if (stat(ndkRootDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            return NDKPKG_ERROR;
        }
    } else {
        return NDKPKG_ERROR;
    }

    size_t ndkRootDIRLength = strlen(ndkRootDIR);

    ///////////////////////////////////////////////////////////////////////////////////

    size_t sourcePropertiesFilePathCapacity = ndkRootDIRLength + 20U;
    char   sourcePropertiesFilePath[sourcePropertiesFilePathCapacity];

    int ret = snprintf(sourcePropertiesFilePath, sourcePropertiesFilePathCapacity, "%s/source.properties", ndkRootDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    char ndkVersion[50];

    ret = readNDKVersionFromFile(sourcePropertiesFilePath, ndkVersion, 50U);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    unsigned int ndkVersionMajor = 0U;

    for (int i = 0; ;i++) {
        if (ndkVersion[i] == '.') {
            ndkVersion[i] = '\0';
            ndkVersionMajor = atoi(ndkVersion);
            ndkVersion[i] = '.';
            break;
        }

        if (ndkVersion[i] < '0') {
            return NDKPKG_ERROR;
        }

        if (ndkVersion[i] > '9') {
            return NDKPKG_ERROR;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////

    size_t cmakeToolchainFilePathCapacity = ndkRootDIRLength + 37U;
    char   cmakeToolchainFilePath[cmakeToolchainFilePathCapacity];

    ret = snprintf(cmakeToolchainFilePath, cmakeToolchainFilePathCapacity, "%s/build/cmake/android.toolchain.cmake", ndkRootDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (stat(cmakeToolchainFilePath, &st) == 0) {
        if (!S_ISREG(st.st_mode)) {
            return NDKPKG_ERROR;
        }
    } else {
        return NDKPKG_ERROR;
    }

    ///////////////////////////////////////////////////////////////////////////////////

#if defined (__APPLE__)
    const char * ndkToolchainHostTag = "darwin-x86_64";
#else
    const char * ndkToolchainHostTag = "linux-x86_64";
#endif

    ///////////////////////////////////////////////////////////////////////////////////

    size_t sysrootCapacity = ndkRootDIRLength + 50U;
    char   sysroot[sysrootCapacity];

    ret = snprintf(sysroot, sysrootCapacity, "%s/toolchains/llvm/prebuilt/%s/sysroot", ndkRootDIR, ndkToolchainHostTag);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (stat(sysroot, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            return NDKPKG_ERROR;
        }
    } else {
        return NDKPKG_ERROR;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    ret = ndkpkg_android_ndk_toolchain_attach_tools(toolchain, ndkRootDIR, ndkRootDIRLength, ndkToolchainHostTag);

    if (ret != NDKPKG_OK) {
        ndkpkg_android_ndk_toolchain_free(toolchain);
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    char * p = strdup(ndkRootDIR);

    if (p == NULL) {
        ndkpkg_android_ndk_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->rootdir = p;

    ///////////////////////////////////////////////////////////////////////////////////

    p = strdup(sysroot);

    if (p == NULL) {
        ndkpkg_android_ndk_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->sysroot = p;

    ///////////////////////////////////////////////////////////////////////////////////

    p = strdup(ndkVersion);

    if (p == NULL) {
        ndkpkg_android_ndk_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->version = p;

    ///////////////////////////////////////////////////////////////////////////////////

    toolchain->versionMajor = ndkVersionMajor;

    return NDKPKG_OK;
}

int ndkpkg_android_ndk_toolchain_find(NDKPKGAndroidNDKToolChain * toolchain) {
    if (toolchain == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    const char * p = getenv("ANDROID_NDK_HOME");

    if (p != NULL && p[0] != '\0') {
        int ret = ndkpkg_android_ndk_toolchain_make(toolchain, p);

        if (ret == NDKPKG_OK) {
            return NDKPKG_OK;
        }

        p = getenv("ANDROID_NDK_ROOT");

        if (p != NULL && p[0] != '\0') {
            ret = ndkpkg_android_ndk_toolchain_make(toolchain, p);

            if (ret == NDKPKG_OK) {
                return NDKPKG_OK;
            }

        }
    }

    return NDKPKG_ERROR;
}

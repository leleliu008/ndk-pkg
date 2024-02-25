#ifndef NDK_MANAGER_H
#define NDK_MAMAGER_H
 
#define NDK_MAMAGER_OK                     0
#define NDK_MAMAGER_ERROR                  1

#define NDK_MAMAGER_ERROR_ARG_IS_NULL      2
#define NDK_MAMAGER_ERROR_ARG_IS_EMPTY     3
#define NDK_MAMAGER_ERROR_ARG_IS_INVALID   4
#define NDK_MAMAGER_ERROR_ARG_IS_UNKNOWN   5

#define NDK_MAMAGER_ERROR_MEMORY_ALLOCATE  6

#define NDK_MAMAGER_ERROR_SHA256_MISMATCH  7

#define NDK_MAMAGER_ERROR_ENV_HOME_NOT_SET 8
#define NDK_MAMAGER_ERROR_ENV_PATH_NOT_SET 9

typedef struct {
    char * rootdir;
    char * sysroot;

    char * version;
    unsigned int versionMajor;

    char * cc;
    char * cxx;
    char * cpp;
    char * as;
    char * ar;
    char * ranlib;
    char * ld;
    char * nm;
    char * strip;
    char * size;
    char * strings;
    char * objdump;
    char * objcopy;
    char * readelf;

    char *  ccflags;
    char * cxxflags;
    char * cppflags;
    char *  ldflags;
} NDKToolChain;

int  ndk_toolchain_find(NDKToolChain * toolchain);
void ndk_toolchain_free(NDKToolChain * toolchain);
void ndk_toolchain_dump(NDKToolChain * toolchain);

#endif

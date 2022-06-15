#ifndef NDK_MANAGER_H
#define NDK_MANAGER_H

#include <stdbool.h>

    typedef struct {
        char host_tag[10];
        char version_string[10];
        int  version_major;
        char source_properties[255];
        char cmake_toolchain_file[255];
        char root[255];
        char bin_dir[255];
        char sysroot[255];
        char cc[255];
        char cxx[255];
        char cpp[255];
        char as[255];
        char ld[255];
        char ar[255];
        char nm[255];
        char size[255];
        char strip[255];
        char ranlib[255];
        char strings[255];
        char objdump[255];
        char objcopy[255];
        char readelf[255];
    } NDKToolchainInfo ;

    NDKToolchainInfo* getNDKToolchainInfo(const char* host_os_kind);
#endif

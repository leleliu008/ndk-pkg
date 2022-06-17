#ifndef NDK_MANAGER_H
#define NDK_MANAGER_H

    typedef struct {
        const char* host_tag;
        const char* version_string;
              int   version_major;
        const char* source_properties;
        const char* cmake_toolchain_file;
        const char* root;
        const char* bin_dir;
        const char* sysroot;
        const char* cc;
        const char* cxx;
        const char* cpp;
        const char* as;
        const char* ld;
        const char* ar;
        const char* nm;
        const char* size;
        const char* strip;
        const char* ranlib;
        const char* strings;
        const char* objdump;
        const char* objcopy;
        const char* readelf;
        char* vvvv;
    } NDKToolchainInfo ;

    NDKToolchainInfo* make_ndk_toolchain_info(const char* hostOSKind, const char* ndkRootDir);
    NDKToolchainInfo* find_ndk_toolchain_info(const char* hostOSKind);
    NDKToolchainInfo* install_ndk_toolchain(const char* hostOSKind);
#endif

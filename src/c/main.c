#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

#include "formula-repo.h"
#include "ndk-manager.h"
#include "ndk-pkg.h"
#include "log.h"

#ifndef NDK_PKG_VERSION
#define NDK_PKG_VERSION "0.0.0"
#endif

int ndk_pkg_main(int argc, char* argv[]) {
    if (argc == 1) {
        ndk_pkg_help();
        return 0;
    }

    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
        ndk_pkg_help();
        return 0;
    }

    if ((strcmp(argv[1], "-V") == 0) || (strcmp(argv[1], "--version") == 0)) {
        printf("%s\n", NDK_PKG_VERSION);
        return 0;
    }

    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
    } 

    fprintf(stderr, "Sysname:  %s\n", uts.sysname);
    fprintf(stderr, "Nodename: %s\n", uts.nodename);
    fprintf(stderr, "Release:  %s\n", uts.release);
    fprintf(stderr, "Version:  %s\n", uts.version);
    fprintf(stderr, "Machine:  %s\n", uts.machine);

    char* host_os_kind = uts.sysname;
    char* host_os_name = uts.nodename;
    char* host_os_arch = uts.machine;

    fprintf(stderr, "host_os_kind:  %s\n", host_os_kind);
    fprintf(stderr, "host_os_name:  %s\n", host_os_name);
    fprintf(stderr, "host_os_arch:  %s\n", host_os_arch);

    if ((strcmp(argv[1], "env") == 0)) {
        NDKToolchainInfo *ndkToolchainInfo = find_ndk_toolchain_info(host_os_kind);
        if (ndkToolchainInfo == NULL) {
            LOG_RED("no Android NDK found");
        } else {
            printf("ANDROID_NDK_ROOT    = %s\n", ndkToolchainInfo->root);
            printf("ANDROID_NDK_BIN_DIR = %s\n", ndkToolchainInfo->bin_dir);
            printf("ANDROID_NDK_SYSROOT = %s\n", ndkToolchainInfo->sysroot);
            printf("ANDROID_NDK_VERS    = %s\n", ndkToolchainInfo->version_string);
            printf("ANDROID_NDK_VERS_MAJOR = %d\n", ndkToolchainInfo->version_major);
            printf("ANDROID_NDK_HOST_TAG   = %s\n", ndkToolchainInfo->host_tag);
            printf("ANDROID_NDK_CMAKE_TOOLCHAIN_FILE = %s\n", ndkToolchainInfo->cmake_toolchain_file);
            printf("CC      = %s\n", ndkToolchainInfo->cc);
            printf("CXX     = %s\n", ndkToolchainInfo->cxx);
            printf("CPP     = %s\n", ndkToolchainInfo->cpp);
            printf("AS      = %s\n", ndkToolchainInfo->as);
            printf("AR      = %s\n", ndkToolchainInfo->ar);
            printf("LD      = %s\n", ndkToolchainInfo->ld);
            printf("NM      = %s\n", ndkToolchainInfo->nm);
            printf("SIZE    = %s\n", ndkToolchainInfo->size);
            printf("STRIP   = %s\n", ndkToolchainInfo->strip);
            printf("RANLIB  = %s\n", ndkToolchainInfo->ranlib);
            printf("STRINGS = %s\n", ndkToolchainInfo->strings);
            printf("OBJCOPY = %s\n", ndkToolchainInfo->objcopy);
            printf("OBJSUMP = %s\n", ndkToolchainInfo->objdump);
            printf("READELF = %s\n", ndkToolchainInfo->readelf);
        }
        return 0;
    }

    if (strcmp(argv[1], "search") == 0) {
        return ndk_pkg_search(argv[2]);
    }

    if (strcmp(argv[1], "install") == 0) {
        return ndk_pkg_install(argv[2]);
    }

    if (strcmp(argv[1], "formula-repo") == 0) {
        if (argv[2] == NULL || strcmp(argv[2], "list") == 0) {
            NdkPkgFormulaRepoList* ndkPkgFormulaRepoList = ndk_pkg_list_formula_repositories();
            if (ndkPkgFormulaRepoList != NULL) {
                for(size_t i = 0; i < ndkPkgFormulaRepoList->size; i++) {
                    NdkPkgFormulaRepo* ndkPkgFormulaRepo = ndkPkgFormulaRepoList->repos[i];
                    printf("%s=%s\n", ndkPkgFormulaRepo->name, ndkPkgFormulaRepo->url);
                }
            }
        }
        return 0;
    }

    fprintf(stderr, "%sunrecognized argument: %s%s\n", COLOR_RED, argv[1], COLOR_OFF);
    return 1;
}

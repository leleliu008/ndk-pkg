#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include "ndk-manager.h"
#include "log.h"

void help() {
    LOG_GREEN("A package manager for Android NDK to build C/C++/Rust/Go project.");
    LOG_GREEN("");
    LOG_GREEN("ndk-pkg <ACTION> [ARGUMENT...]");
    LOG_GREEN("");
    LOG_GREEN("ndk-pkg --help");
    LOG_GREEN("ndk-pkg -h");
    LOG_WHITE("    show help of this command.");
}

#ifndef NDK_PKG_VERSION
#define NDK_PKG_VERSION "0.0.0"
#endif

int main(int argc, char* argv[]) {
    if (argc == 1) {
        help();
        return 0;
    }

    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
        help();
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

    printf("Sysname:  %s\n", uts.sysname);
    printf("Nodename: %s\n", uts.nodename);
    printf("Release:  %s\n", uts.release);
    printf("Version:  %s\n", uts.version);
    printf("Machine:  %s\n", uts.machine);

    char* host_os_kind = uts.sysname;
    char* host_os_name = uts.nodename;
    char* host_os_arch = uts.machine;

    printf("host_os_kind:  %s\n", host_os_kind);
    printf("host_os_name:  %s\n", host_os_name);
    printf("host_os_arch:  %s\n", host_os_arch);

    if ((strcmp(argv[1], "env") == 0)) {
        NDKToolchainInfo *ndkToolchainInfo = getNDKToolchainInfo(host_os_kind);
        if (ndkToolchainInfo != NULL) {
            free(ndkToolchainInfo);
            ndkToolchainInfo = NULL;
        }
        return 0;
    }

    if ((strcmp(argv[1], "install") == 0)) {
        printf("%s\n", NDK_PKG_VERSION);
        return 0;
    }

    fprintf(stderr, "%sunrecognized argument: %s%s\n", COLOR_RED, argv[1], COLOR_OFF);
    return 1;
}

#include "log.h"

int ndk_pkg_help() {
    LOG_GREEN(
        "A package manager for Android NDK to build C/C++/Rust/Go project.\n\n"
        "ndk-pkg <ACTION> [ARGUMENT...]\n\n"
        "ndk-pkg --help\n"
        "ndk-pkg -h"
    );
    LOG_WHITE(
        "    show help of this command.\n"
    );
    LOG_GREEN(
        "ndk-pkg --version\n"
        "ndk-pkg -V"
    );
    LOG_WHITE(
        "    show version of this command.\n"
    );

    return 0;
}

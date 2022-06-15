#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "ndk-manager.h"
#include "log.h"

bool exists_and_is_a_directory(const char* dirpath) {
    struct stat sb;
    return (stat(dirpath, &sb) == 0) && S_ISDIR(sb.st_mode);
}

bool exists_and_is_a_regular_file(const char* filepath) {
    struct stat sb;
    return (stat(filepath, &sb) == 0) && S_ISREG(sb.st_mode);
}

bool is_a_valid_android_ndk_root_dir(const char* dir, const char* host_os_kind) {
    if (!exists_and_is_a_directory(dir)) {
        return false;
    }

    char tmpfilepath1[255] = {0};
    sprintf(tmpfilepath1, "%s/source.properties", dir);

    if (!exists_and_is_a_regular_file(tmpfilepath1)) {
        return false;
    }

    char tmpfilepath2[255] = {0};
    sprintf(tmpfilepath2, "%s/build/cmake/android.toolchain.cmake", dir);

    if (!exists_and_is_a_regular_file(tmpfilepath2)) {
        return false;
    }

    const char* android_ndk_toolchain_host_tag;

    if (strcmp(host_os_kind, "Linux") == 0) {
        android_ndk_toolchain_host_tag="linux-x86_64";
    } else if(strcmp(host_os_kind, "Darwin") == 0) {
        android_ndk_toolchain_host_tag="darwin-x86_64";
    } else {
        perror("unsupport system:");
    }

    char tmpdirpath1[255] = {0};
    sprintf(tmpdirpath1, "%s/toolchains/llvm/prebuilt/%s", dir, android_ndk_toolchain_host_tag);
    if (!exists_and_is_a_directory(tmpdirpath1)) {
        return false;
    }

    char tmpdirpath2[255] = {0};
    sprintf(tmpdirpath2, "%s/toolchains/llvm/prebuilt/%s/bin", dir, android_ndk_toolchain_host_tag);
    if (!exists_and_is_a_directory(tmpdirpath2)) {
        return false;
    }

    char tmpdirpath3[255] = {0};
    sprintf(tmpdirpath3, "%s/toolchains/llvm/prebuilt/%s/sysroot", dir, android_ndk_toolchain_host_tag);
    if (!exists_and_is_a_directory(tmpdirpath3)) {
        return false;
    }

    return true;
}

NDKToolchainInfo* getNDKToolchainInfo(const char* host_os_kind) {
    char* user_home_dir    = getenv("HOME");
    char* android_sdk_root = getenv("ANDROID_HOME");
    char* android_ndk_root = getenv("ANDROID_NDK_HOME");

    if (android_ndk_root != NULL && strcmp(android_ndk_root, "") != 0 && is_a_valid_android_ndk_root_dir(android_ndk_root, host_os_kind)) {
        android_ndk_root = NULL;
    }

    if (android_ndk_root == NULL || strcmp(android_ndk_root, "") == 0) {
        android_ndk_root = getenv("ANDROID_NDK_ROOT");
    }

    if (android_ndk_root != NULL && strcmp(android_ndk_root, "") != 0 && is_a_valid_android_ndk_root_dir(android_ndk_root, host_os_kind)) {
        android_ndk_root = NULL;
    }

    if (android_ndk_root == NULL || strcmp(android_ndk_root, "") == 0) {
        if (android_sdk_root != NULL && strcmp(android_sdk_root, "") != 0) {
            android_ndk_root = (char*)calloc(strlen(android_sdk_root) + 12, sizeof(char));
            if (android_ndk_root != NULL) {
                sprintf(android_ndk_root, "%s/ndk-bundle", android_sdk_root);
                if (!is_a_valid_android_ndk_root_dir(android_ndk_root, host_os_kind)) {
                    free(android_ndk_root);
                    android_ndk_root = NULL;
                }
            }
        }
    }

    if (android_ndk_root == NULL || strcmp(android_ndk_root, "") == 0) {
        if (user_home_dir != NULL && strcmp(user_home_dir, "") != 0) {
            android_ndk_root = (char*)calloc(strlen(user_home_dir) + 30, sizeof(char));
            if (android_ndk_root != NULL) {
                sprintf(android_ndk_root, "%s/.ndk-pkg/android-ndk-r23b", user_home_dir);
                if (!is_a_valid_android_ndk_root_dir(android_ndk_root, host_os_kind)) {
                    free(android_ndk_root);
                    android_ndk_root = NULL;
                }
            }
        }
    }

    if (android_ndk_root == NULL || strcmp(android_ndk_root, "") == 0) {
        LOG_INFO("Couldn't find ndk, I will install it for you.");
    } else {
        fprintf(stderr, "android_ndk_root = %s", android_ndk_root);
    }

    return NULL;
}

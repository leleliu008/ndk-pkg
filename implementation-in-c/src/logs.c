#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "ndk-pkg.h"

int ndkpkg_logs(const char * packageName, const NDKPKGTargetPlatform * targetPlatform) {
    int ret = ndkpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    if (targetPlatform == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

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

    size_t metaInfoDIRCapacity = ndkpkgHomeDIRLength + strlen(packageName) + 42U;
    char   metaInfoDIR[metaInfoDIRCapacity];

    ret = snprintf(metaInfoDIR, metaInfoDIRCapacity, "%s/installed/android-%2u-%s/%s/.ndk-pkg", ndkpkgHomeDIR, targetPlatform->api, targetABI, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t receiptFilePathCapacity = metaInfoDIRCapacity + 13U;
    char   receiptFilePath[receiptFilePathCapacity];

    ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/RECEIPT.yml", metaInfoDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(receiptFilePath, &st) != 0 || !S_ISREG(st.st_mode)) {
        return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    DIR * dir = opendir(metaInfoDIR);

    if (dir == NULL) {
        perror(metaInfoDIR);
        return NDKPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return NDKPKG_OK;
            } else {
                perror(metaInfoDIR);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filepathCapacity = metaInfoDIRCapacity + strlen(dir_entry->d_name) + 2U;
        char   filepath[filepathCapacity];

        ret = snprintf(filepath, filepathCapacity, "%s/%s", metaInfoDIR, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return NDKPKG_ERROR;
        }

        if (stat(filepath, &st) == 0 && S_ISDIR(st.st_mode)) {
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror(NULL);
            closedir(dir);
            return NDKPKG_ERROR;
        }

        if (pid == 0) {
            execlp("bat", "bat", filepath, NULL);
            perror("bat");
            exit(255);
        } else {
            int childProcessExitStatusCode;

            if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
                perror(NULL);
                closedir(dir);
                return NDKPKG_ERROR;
            }

            if (childProcessExitStatusCode != 0) {
                if (WIFEXITED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command 'bat %s' exit with status code: %d\n", filepath, WEXITSTATUS(childProcessExitStatusCode));
                } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command 'bat %s' killed by signal: %d\n", filepath, WTERMSIG(childProcessExitStatusCode));
                } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command 'bat %s' stopped by signal: %d\n", filepath, WSTOPSIG(childProcessExitStatusCode));
                }

                closedir(dir);
                return NDKPKG_ERROR;
            }
        }
    }
}

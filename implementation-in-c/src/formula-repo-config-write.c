#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "ndk-pkg.h"

int ndkpkg_formula_repo_config_write(const char * formulaRepoDIRPath, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled, const char * createdAt, const char * updatedAt) {
    if (formulaRepoDIRPath == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (branchName == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (createdAt == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (updatedAt == NULL) {
        updatedAt = "";
    }

    size_t strCapacity = strlen(formulaRepoUrl) + strlen(branchName) + strlen(createdAt) + strlen(updatedAt) + 78U;
    char   str[strCapacity];

    int ret = snprintf(str, strCapacity, "url: %s\nbranch: %s\npinned: %1d\nenabled: %1d\ncreated: %s\nupdated: %s\n", formulaRepoUrl, branchName, pinned, enabled, createdAt, updatedAt);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    int strLength = ret;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoConfigFilePathCapacity = strlen(formulaRepoDIRPath) + 27U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.ndk-pkg-formula-repo.yml", formulaRepoDIRPath);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    int fd = open(formulaRepoConfigFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(formulaRepoConfigFilePath);
        return NDKPKG_ERROR;
    }

    ssize_t writeSize = write(fd, str, strLength);

    if (writeSize == -1) {
        perror(formulaRepoConfigFilePath);
        close(fd);
        return NDKPKG_ERROR;
    }

    close(fd);

    if (writeSize == strLength) {
        return NDKPKG_OK;
    } else {
        fprintf(stderr, "not fully written to %s\n", formulaRepoConfigFilePath);
        return NDKPKG_ERROR;
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "ndk-pkg.h"

int ndkpkg_formula_repo_remove(const char * formulaRepoName) {
    if (formulaRepoName == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepoName[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    if (strcmp(formulaRepoName, "official-core") == 0) {
        fprintf(stderr, "official-core formula repo is not allowed to delete.\n");
        return NDKPKG_ERROR;
    }

    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    size_t formulaRepoPathCapacity = ndkpkgHomeDIRLength + strlen(formulaRepoName) + 10U;
    char   formulaRepoPath[formulaRepoPathCapacity];

    ret = snprintf(formulaRepoPath, formulaRepoPathCapacity, "%s/repos.d/%s", ndkpkgHomeDIR, formulaRepoName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(formulaRepoPath, &st) != 0) {
        fprintf(stderr, "formula repo not found: %s\n", formulaRepoName);
        return NDKPKG_ERROR;
    }

    size_t formulaRepoConfigFilePathCapacity = formulaRepoPathCapacity + 26U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.ndk-pkg-formula-repo.yml", formulaRepoPath);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (stat(formulaRepoConfigFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        return ndkpkg_rm_r(formulaRepoPath, false);
    } else {
        fprintf(stderr, "formula repo is broken: %s\n", formulaRepoName);
        return NDKPKG_ERROR;
    }
}

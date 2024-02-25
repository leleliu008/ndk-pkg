#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "ndk-pkg.h"

int ndkpkg_formula_repo_lookup(const char * formulaRepoName, NDKPKGFormulaRepo * * formulaRepoPP) {
    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    size_t formulaRepoNameLength = strlen(formulaRepoName);

    size_t formulaRepoDIRPathCapacity = ndkpkgHomeDIRLength + formulaRepoNameLength + 10U;
    char   formulaRepoDIRPath[formulaRepoDIRPathCapacity];

    ret = snprintf(formulaRepoDIRPath, formulaRepoDIRPathCapacity, "%s/repos.d/%s", ndkpkgHomeDIR, formulaRepoName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(formulaRepoDIRPath, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", formulaRepoDIRPath);
            return NDKPKG_ERROR;
        }
    } else {
        return NDKPKG_ERROR_FORMULA_REPO_NOT_FOUND;
    }

    size_t formulaRepoConfigFilePathCapacity = formulaRepoDIRPathCapacity + 26U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.ndk-pkg-formula-repo.yml", formulaRepoDIRPath);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (!((stat(formulaRepoConfigFilePath, &st) == 0) && S_ISREG(st.st_mode))) {
        return NDKPKG_ERROR_FORMULA_REPO_NOT_FOUND;
    }

    NDKPKGFormulaRepo * formulaRepo = NULL;

    ret = ndkpkg_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    formulaRepo->name = strdup(formulaRepoName);

    if (formulaRepo->name == NULL) {
        ndkpkg_formula_repo_free(formulaRepo);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    formulaRepo->path = strdup(formulaRepoDIRPath);

    if (formulaRepo->path == NULL) {
        ndkpkg_formula_repo_free(formulaRepo);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    (*formulaRepoPP) = formulaRepo;
    return NDKPKG_OK;
}

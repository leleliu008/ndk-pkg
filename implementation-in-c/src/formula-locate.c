#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include "ndk-pkg.h"

int ndkpkg_formula_locate(const char * packageName, char ** out) {
    int ret = ndkpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    NDKPKGFormulaRepoList * formulaRepoList = NULL;

    ret = ndkpkg_formula_repo_list(&formulaRepoList);

    if (ret != NDKPKG_OK) {
        ndkpkg_formula_repo_list_free(formulaRepoList);
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t packageNameLength = strlen(packageName);

    for (size_t i = 0U; i < formulaRepoList->size; i++) {
        char * formulaRepoPath = formulaRepoList->repos[i]->path;

        size_t formulaFilePathCapacity = strlen(formulaRepoPath) + packageNameLength + 15U;
        char   formulaFilePath[formulaFilePathCapacity];

        ret = snprintf(formulaFilePath, formulaFilePathCapacity, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (stat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            ndkpkg_formula_repo_list_free(formulaRepoList);

            (*out) = strdup(formulaFilePath);

            if (*out == NULL) {
                return NDKPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return NDKPKG_OK;
            }
        }
    }

    ndkpkg_formula_repo_list_free(formulaRepoList);
    return NDKPKG_ERROR_PACKAGE_NOT_AVAILABLE;
}

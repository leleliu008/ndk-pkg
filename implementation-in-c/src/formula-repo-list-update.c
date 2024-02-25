#include <stdio.h>
#include <string.h>

#include "ndk-pkg.h"

int ndkpkg_formula_repo_list_update() {
    NDKPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = ndkpkg_formula_repo_list(&formulaRepoList);

    if (ret == NDKPKG_OK) {
        bool officialCoreIsThere = false;

        for (size_t i = 0U; i < formulaRepoList->size; i++) {
            NDKPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];

            if (strcmp(formulaRepo->name, "official-core") == 0) {
                officialCoreIsThere = true;
            }

            ret = ndkpkg_formula_repo_sync(formulaRepo);

            if (ret != NDKPKG_OK) {
                break;
            }
        }

        ndkpkg_formula_repo_list_free(formulaRepoList);

        if (!officialCoreIsThere) {
            const char * const formulaRepoUrl = "https://github.com/leleliu008/ndkpkg-formula-repository-official-core";
            ret = ndkpkg_formula_repo_add("official-core", formulaRepoUrl, "master", false, true);
        }
    }

    return ret;
}

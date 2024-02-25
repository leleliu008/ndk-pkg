#include <stdio.h>

#include <unistd.h>

#include "ndk-pkg.h"

int ndkpkg_formula_repo_list_printf() {
    NDKPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = ndkpkg_formula_repo_list(&formulaRepoList);

    if (ret == NDKPKG_OK) {
        for (size_t i = 0; i < formulaRepoList->size; i++) {
            if (i > 0) {
                if (isatty(STDOUT_FILENO)) {
                    printf("\n");
                } else {
                    printf("---\n");
                }
            }

            ndkpkg_formula_repo_info(formulaRepoList->repos[i]);
        }

        ndkpkg_formula_repo_list_free(formulaRepoList);
    }

    return ret;
}

#include "ndk-pkg.h"

int ndkpkg_formula_repo_config(const char * formulaRepoName, const char * url, const char * branch, int pinned, int enabled) {
    NDKPKGFormulaRepo * formulaRepo = NULL;

    int ret = ndkpkg_formula_repo_lookup(formulaRepoName, &formulaRepo);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    if (url == NULL) {
        url = formulaRepo->url;
    }

    if (branch == NULL) {
        branch = formulaRepo->branch;
    }

    if (pinned == -1) {
        pinned = formulaRepo->pinned;
    }

    if (enabled == -1) {
        enabled = formulaRepo->enabled;
    }

    ret = ndkpkg_formula_repo_config_write(formulaRepo->path, url, branch, pinned, enabled, formulaRepo->createdAt, formulaRepo->updatedAt);

    ndkpkg_formula_repo_free(formulaRepo);

    return ret;
}

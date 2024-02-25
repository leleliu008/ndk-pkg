#include <time.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "core/log.h"

#include "ndk-pkg.h"

int ndkpkg_formula_repo_sync_(const char * formulaRepoName) {
    NDKPKGFormulaRepo * formulaRepo = NULL;

    int ret = ndkpkg_formula_repo_lookup(formulaRepoName, &formulaRepo);

    if (ret == NDKPKG_OK) {
        ret = ndkpkg_formula_repo_sync(formulaRepo);
    }

    ndkpkg_formula_repo_free(formulaRepo);

    return ret;
}

int ndkpkg_formula_repo_sync(NDKPKGFormulaRepo * formulaRepo) {
    if (formulaRepo == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepo->pinned) {
        fprintf(stderr, "'%s' formula repo was pinned, skipped.\n", formulaRepo->name);
        return NDKPKG_OK;
    }

    if (isatty(STDOUT_FILENO)) {
        printf("%s%s%s\n", COLOR_PURPLE, "==> Updating formula repo", COLOR_OFF);
    } else {
        printf("=== Updating formula repo\n");
    }

    ndkpkg_formula_repo_info(formulaRepo);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const char * branchName = formulaRepo->branch;
    size_t       branchNameLength = strlen(branchName);

    size_t remoteRefPathCapacity = branchNameLength + 12U;
    char   remoteRefPath[remoteRefPathCapacity];

    int ret = snprintf(remoteRefPath, remoteRefPathCapacity, "refs/heads/%s", branchName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t remoteTrackingRefPathCapacity = branchNameLength + 21U;
    char   remoteTrackingRefPath[remoteTrackingRefPathCapacity];

    ret = snprintf(remoteTrackingRefPath, remoteTrackingRefPathCapacity, "refs/remotes/origin/%s", branchName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = ndkpkg_git_sync(formulaRepo->path, formulaRepo->url, remoteRefPath, remoteTrackingRefPath, branchName, 0);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];

    ret = snprintf(ts, 11, "%ld", time(NULL));

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    return ndkpkg_formula_repo_config_write(formulaRepo->path, formulaRepo->url, formulaRepo->branch, formulaRepo->pinned, formulaRepo->enabled, formulaRepo->createdAt, ts);
}

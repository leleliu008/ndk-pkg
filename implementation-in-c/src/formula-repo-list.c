#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "ndk-pkg.h"

int ndkpkg_formula_repo_list(NDKPKGFormulaRepoList * * out) {
    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    size_t ndkpkgFormulaRepoDIRCapacity = ndkpkgHomeDIRLength + 9U;
    char   ndkpkgFormulaRepoDIR[ndkpkgFormulaRepoDIRCapacity];

    ret = snprintf(ndkpkgFormulaRepoDIR, ndkpkgFormulaRepoDIRCapacity, "%s/repos.d", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if ((stat(ndkpkgFormulaRepoDIR, &st) != 0) || (!S_ISDIR(st.st_mode))) {
        NDKPKGFormulaRepoList* formulaRepoList = (NDKPKGFormulaRepoList*)calloc(1, sizeof(NDKPKGFormulaRepoList));

        if (formulaRepoList == NULL) {
            return NDKPKG_ERROR_MEMORY_ALLOCATE;
        } else {
            (*out) = formulaRepoList;
            return NDKPKG_OK;
        }
    }

    DIR * dir = opendir(ndkpkgFormulaRepoDIR);

    if (dir == NULL) {
        perror(ndkpkgFormulaRepoDIR);
        return NDKPKG_ERROR;
    }

    size_t capcity = 5;

    NDKPKGFormulaRepoList * formulaRepoList = NULL;

    ret = NDKPKG_OK;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                break;
            } else {
                perror(ndkpkgFormulaRepoDIR);
                closedir(dir);
                ndkpkg_formula_repo_list_free(formulaRepoList);
                return NDKPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t formulaRepoPathCapacity = ndkpkgFormulaRepoDIRCapacity + strlen(dir_entry->d_name) + 2U;
        char   formulaRepoPath[formulaRepoPathCapacity];

        ret = snprintf(formulaRepoPath, formulaRepoPathCapacity, "%s/%s", ndkpkgFormulaRepoDIR, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            ndkpkg_formula_repo_list_free(formulaRepoList);
            return NDKPKG_ERROR;
        }

        size_t formulaRepoConfigFilePathCapacity = formulaRepoPathCapacity + 26U;
        char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

        ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.ndk-pkg-formula-repo.yml", formulaRepoPath);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            ndkpkg_formula_repo_list_free(formulaRepoList);
            return NDKPKG_ERROR;
        }

        if (stat(formulaRepoConfigFilePath, &st) != 0) {
            continue;
        }

        NDKPKGFormulaRepo * formulaRepo = NULL;

        ret = ndkpkg_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

        if (ret != NDKPKG_OK) {
            ndkpkg_formula_repo_free(formulaRepo);
            ndkpkg_formula_repo_list_free(formulaRepoList);
            goto finalize;
        }

        if (formulaRepoList == NULL) {
            formulaRepoList = (NDKPKGFormulaRepoList*)calloc(1, sizeof(NDKPKGFormulaRepoList));

            if (formulaRepoList == NULL) {
                ndkpkg_formula_repo_free(formulaRepo);
                ndkpkg_formula_repo_list_free(formulaRepoList);
                ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            formulaRepoList->repos = (NDKPKGFormulaRepo**)calloc(capcity, sizeof(NDKPKGFormulaRepo*));

            if (formulaRepoList->repos == NULL) {
                ndkpkg_formula_repo_free(formulaRepo);
                ndkpkg_formula_repo_list_free(formulaRepoList);
                ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }
        }

        if (capcity == formulaRepoList->size) {
            capcity += 5;
            NDKPKGFormulaRepo ** formulaRepoArray = (NDKPKGFormulaRepo**)realloc(formulaRepoList->repos, capcity * sizeof(NDKPKGFormulaRepo*));

            if (formulaRepoArray == NULL) {
                ndkpkg_formula_repo_free(formulaRepo);
                ndkpkg_formula_repo_list_free(formulaRepoList);
                ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            } else {
                formulaRepoList->repos = formulaRepoArray;
            }
        }

        formulaRepo->name = strdup(dir_entry->d_name);

        if (formulaRepo->name == NULL) {
            ndkpkg_formula_repo_free(formulaRepo);
            ndkpkg_formula_repo_list_free(formulaRepoList);
            ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
            goto finalize;
        }

        formulaRepo->path = strdup(formulaRepoPath);

        if (formulaRepo->path == NULL) {
            ndkpkg_formula_repo_free(formulaRepo);
            ndkpkg_formula_repo_list_free(formulaRepoList);
            ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
            goto finalize;
        }

        formulaRepoList->repos[formulaRepoList->size] = formulaRepo;
        formulaRepoList->size += 1;
    }

    if (formulaRepoList == NULL) {
        formulaRepoList = (NDKPKGFormulaRepoList*)calloc(1, sizeof(NDKPKGFormulaRepoList));

        if (formulaRepoList == NULL) {
            ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
            goto finalize;
        }
    }

finalize:
    if (ret == NDKPKG_OK) {
        (*out) = formulaRepoList;
    } else {
        ndkpkg_formula_repo_list_free(formulaRepoList);
    }

    closedir(dir);

    return ret;
}

void ndkpkg_formula_repo_list_free(NDKPKGFormulaRepoList * formulaRepoList) {
    if (formulaRepoList == NULL) {
        return;
    }

    if (formulaRepoList->repos == NULL) {
        free(formulaRepoList);
        return;
    }

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        NDKPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];
        ndkpkg_formula_repo_free(formulaRepo);
    }

    free(formulaRepoList->repos);
    formulaRepoList->repos = NULL;

    free(formulaRepoList);
}

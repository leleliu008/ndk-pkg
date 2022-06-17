#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "fs.h"
#include "formula-repo.h"

NdkPkgFormulaRepoList* ndk_pkg_list_formula_repositories() {
    char* userHomeDir = getenv("HOME");
    if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
        perror("HOME environment must be set.");
    }

    int userHomeDirLength = strlen(userHomeDir);

    int  ndkPkgHomeDirLength = userHomeDirLength + 10;
    char ndkPkgHomeDir[ndkPkgHomeDirLength];
    memset(ndkPkgHomeDir, 0, ndkPkgHomeDirLength);
    sprintf(ndkPkgHomeDir, "%s/.ndk-pkg", userHomeDir);

    if (!exists_and_is_a_directory(ndkPkgHomeDir)) {
        mkdir(ndkPkgHomeDir, 0751);
    }

    int  formulaRepoConfigFilePathLength = userHomeDirLength + 16;
    char formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
    memset(formulaRepoConfigFilePath, 0, formulaRepoConfigFilePathLength);
    sprintf(formulaRepoConfigFilePath, "%s/.ndk-pkg/repos", userHomeDir);

    if (!exists_and_is_a_regular_file(formulaRepoConfigFilePath)) {
        FILE* formulaRepoConfigFile = fopen(formulaRepoConfigFilePath, "w");
        if (formulaRepoConfigFile == NULL) {
            perror("Can not create file");
            return NULL;
        }
        fprintf(formulaRepoConfigFile, "offical=https://github.com/leleliu008/ndk-pkg-formula-repository.git");
        fclose(formulaRepoConfigFile);
    }

    FILE* formulaRepoConfigFile = fopen(formulaRepoConfigFilePath, "r");

    if (formulaRepoConfigFile == NULL) {
        perror("Can not create file");
        return NULL;
    }

    NdkPkgFormulaRepoList* ndkPkgFormulaRepoList = (NdkPkgFormulaRepoList*)calloc(1, sizeof(NdkPkgFormulaRepoList));

    char line[300];
    while(fgets(line, 300, formulaRepoConfigFile)) {
        char* repoName = NULL;
        char* repoUrl  = NULL;

        repoName = strtok(line, "=");
        repoUrl  = strtok(NULL, "=");

        if (repoName == NULL) {
            perror("repoName must be not empty.");
            return NULL;
        }

        if (repoUrl == NULL) {
            perror("repoUrl must be not empty.");
            return NULL;
        }

        repoUrl[strlen(repoUrl) - 1] = '\0';

        NdkPkgFormulaRepo* ndkPkgFormulaRepo = (NdkPkgFormulaRepo*)calloc(1, sizeof(NdkPkgFormulaRepo));
        ndkPkgFormulaRepo->name = repoName;
        ndkPkgFormulaRepo->url  = repoUrl;

        if (ndkPkgFormulaRepoList->size == ndkPkgFormulaRepoList->capacity) {
            int capacity = ndkPkgFormulaRepoList->capacity + 5;
            ndkPkgFormulaRepoList->repos = (NdkPkgFormulaRepo**)realloc(ndkPkgFormulaRepoList->repos, (capacity) * sizeof(NdkPkgFormulaRepo*));
            ndkPkgFormulaRepoList->capacity = capacity;
        }

        ndkPkgFormulaRepoList->repos[ndkPkgFormulaRepoList->size] = ndkPkgFormulaRepo;
        ndkPkgFormulaRepoList->size += 1;
    }

    fclose(formulaRepoConfigFile);

    return ndkPkgFormulaRepoList;
}

int ndk_pkg_update_formula_repositories() {
    return 0;
}

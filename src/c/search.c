#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>

#include "formula-repo.h"
#include "log.h"
#include "fs.h"

int ndk_pkg_search(const char* keyword) {
    NdkPkgFormulaRepoList* ndkPkgFormulaRepoList = ndk_pkg_list_formula_repositories();

    if (ndkPkgFormulaRepoList == NULL) {
        return 1;
    }

    char* userHomeDir = getenv("HOME");
    int   userHomeDirLength = strlen(userHomeDir);

    for (size_t i = 0; i < ndkPkgFormulaRepoList->size; i++) {
        NdkPkgFormulaRepo* ndkPkgFormulaRepo = ndkPkgFormulaRepoList->repos[i];

        char* repoName = ndkPkgFormulaRepo->name;

        int  formulaDirLength = userHomeDirLength + strlen(repoName) + 30;
        char formulaDir[formulaDirLength];
        memset(formulaDir, 0, formulaDirLength);
        sprintf(formulaDir, "%s/.ndk-pkg/repos.d/%s/formula", userHomeDir, repoName);

        DIR *dir;
        struct dirent *dir_entry;

        dir = opendir (formulaDir);
        if (dir == NULL) {
            perror("Couldn't open the directory");
        } else {
            while ((dir_entry = readdir(dir))) {
                //puts(dir_entry->d_name);

                int patternLength = strlen(keyword) + 6;
                char pattern[patternLength];
                memset(pattern, 0, patternLength);
                sprintf(pattern, "*%s*.sh", keyword);

                int r = fnmatch(pattern, dir_entry->d_name, 0);

                if (r == 0) {
                    int fileNameLength = strlen(dir_entry->d_name);
                    char packageName[fileNameLength];
                    memset(packageName, 0, fileNameLength);
                    strncpy(packageName, dir_entry->d_name, fileNameLength - 3);
                    printf("%s\n", packageName);
                } else if(r == FNM_NOMATCH) {
                    ;
                } else {
                    fprintf(stderr, "fnmatch() error\n");
                }
            }
            closedir(dir);
        }
    }

    return 0;
}

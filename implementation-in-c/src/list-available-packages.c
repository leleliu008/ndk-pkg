#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include "ndk-pkg.h"

static int _list_dir(const char * formulaDIR, const bool verbose, NDKPKGPackageNameFilter packageNameFilter, const void * payload, size_t * counter) {
    DIR * dir = opendir(formulaDIR);

    if (dir == NULL) {
        perror(formulaDIR);
        return NDKPKG_ERROR;
    }

    char * fileName;
    char * fileNameSuffix;
    size_t fileNameLength;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return NDKPKG_OK;
            } else {
                perror(formulaDIR);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        fileName = dir_entry->d_name;

        fileNameLength = strlen(fileName);

        if (fileNameLength > 4) {
            fileNameSuffix = fileName + fileNameLength - 4;

            if (strcmp(fileNameSuffix, ".yml") == 0) {
                fileName[fileNameLength - 4] = '\0';

                int ret = ndkpkg_check_if_the_given_argument_matches_package_name_pattern(fileName);

                if (ret == NDKPKG_OK) {
                    ret = packageNameFilter(fileName, verbose, *counter, payload);

                    (*counter)++;

                    if (ret != NDKPKG_OK) {
                        closedir(dir);
                        return ret;
                    }
                }
            }
        }
    }
}

int ndkpkg_list_the_available_packages(const bool verbose, NDKPKGPackageNameFilter packageNameFilter, const void * payload) {
    NDKPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = ndkpkg_formula_repo_list(&formulaRepoList);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    size_t j = 0U;

    struct stat st;

    for (size_t i = 0U; i < formulaRepoList->size; i++) {
        char * formulaRepoPath  = formulaRepoList->repos[i]->path;

        size_t formulaDIRCapacity = strlen(formulaRepoPath) + 10U;
        char   formulaDIR[formulaDIRCapacity];

        ret = snprintf(formulaDIR, formulaDIRCapacity, "%s/formula", formulaRepoPath);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (stat(formulaDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
            ret = _list_dir(formulaDIR, verbose, packageNameFilter, payload, &j);

            if (ret < 0) {
                ndkpkg_formula_repo_list_free(formulaRepoList);
                return ret;
            }
        }
    }

    ndkpkg_formula_repo_list_free(formulaRepoList);

    return NDKPKG_OK;
}

static size_t j = 0U;

static int package_name_filter(const char * packageName, const bool verbose, size_t i, const void * payload) {
    if (verbose) {
        if (j != 0U) {
            printf("\n");
        }

        j++;

        return ndkpkg_available_info(packageName, NULL);
    } else {
        printf("%s\n", packageName);
        return NDKPKG_OK;
    }
}

int ndkpkg_show_the_available_packages(const bool verbose) {
    return ndkpkg_list_the_available_packages(verbose, package_name_filter, NULL);
}

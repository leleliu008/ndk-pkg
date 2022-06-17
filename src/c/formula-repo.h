#ifndef FORMULA_REPO_H
#define FORMULA_REPO_H

#include <stdlib.h>

typedef struct {
    char* name;
    char* url;
} NdkPkgFormulaRepo ;

typedef struct {
    NdkPkgFormulaRepo** repos;
    size_t capacity;
    size_t size;
} NdkPkgFormulaRepoList ;

NdkPkgFormulaRepoList* ndk_pkg_list_formula_repositories();
int                    ndk_pkg_update_formula_repositories();

#endif

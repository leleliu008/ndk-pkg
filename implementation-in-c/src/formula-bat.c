#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "ndk-pkg.h"

int ndkpkg_formula_bat(const char * packageName) {
    char * formulaFilePath = NULL;

    int ret = ndkpkg_formula_locate(packageName, &formulaFilePath);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    execlp("bat", "bat", "--paging=never", formulaFilePath, NULL);

    perror("bat");

    free(formulaFilePath);

    return NDKPKG_ERROR;
}

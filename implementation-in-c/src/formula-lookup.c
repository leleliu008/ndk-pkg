#include "ndk-pkg.h"

int ndkpkg_formula_lookup(const char * packageName, NDKPKGFormula * * out) {
    char * formulaFilePath = NULL;

    int ret = ndkpkg_formula_locate(packageName, &formulaFilePath);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ret = ndkpkg_formula_parse(formulaFilePath, out);

    free(formulaFilePath);

    return ret;
}

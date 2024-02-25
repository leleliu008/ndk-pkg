#include "ndk-pkg.h"

int ndkpkg_formula_view(const char * packageName, const bool raw) {
    if (raw) {
        return ndkpkg_formula_cat(packageName);
    } else {
        return ndkpkg_formula_bat(packageName);
    }
}

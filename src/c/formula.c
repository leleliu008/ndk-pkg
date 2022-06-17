#include "ndk-pkg.h"
#include "log.h"

int ndk_pkg_formula_load(const char* pkgName, NDKPkg *pkg) {
    LOG_GREEN(pkgName);
    LOG_GREEN(pkg->summary);
    return 0;
}

int ndk_pkg_formula_view(const char* pkgName) {
    LOG_GREEN(pkgName);
    return 0;
}

int ndk_pkg_formula_edit(const char* pkgName) {
    LOG_GREEN(pkgName);
    return 0;
}

int ndk_pkg_formula_create(const char* pkgName) {
    LOG_GREEN(pkgName);
    return 0;
}

int ndk_pkg_formula_delete(const char* pkgName) {
    LOG_GREEN(pkgName);
    return 0;
}

int ndk_pkg_formula_rename(const char* pkgName, char* newPkgName) {
    LOG_GREEN(pkgName);
    LOG_GREEN(newPkgName);
    return 0;
}

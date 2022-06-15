#ifndef NDK_PKG_H
#define NDK_PKG_H

typedef struct {
    char url[255];
    char sha256[40];
} WebRes ;

typedef struct {
    char name[20];
    char version[255];
    char summary[255];
    char webpage[255];
    char formula[255];
    WebRes src;
    WebRes fix;
    WebRes res;
} NDKPkg ;

int ndk_pkg_main(int argc, char* argv[]);

int ndk_pkg_help();

int ndk_pkg_update_formula_repositories();

int ndk_pkg_update_self();

int ndk_pkg_search(const char* keyword);

int ndk_pkg_formula_load(const char* pkgName, NDKPkg *pkg);
int ndk_pkg_formula_view(const char* pkgName);
int ndk_pkg_formula_edit(const char* pkgName);
int ndk_pkg_formula_create(const char* pkgName);
int ndk_pkg_formula_delete(const char* pkgName);
int ndk_pkg_formula_rename(const char* pkgName, char* newPkgName);

int ndk_pkg_info(const char* pkgName);

int ndk_pkg_tree(const char* pkgName, int minSDKAPILevel);

int ndk_pkg_logs(const char* pkgName, int minSDKAPILevel);

int ndk_pkg_pack(const char* pkgName, int minSDKAPILevel, const char* type);

int ndk_pkg_deploy(const char* pkgName, int minSDKAPILevel, const char* type);

int ndk_pkg_depends(const char* pkgName);

int ndk_pkg_fetch(const char* pkgName);

int ndk_pkg_install(const char* pkgName);

int ndk_pkg_reinstall(const char* pkgName);

int ndk_pkg_uninstall(const char* pkgName);

int ndk_pkg_upgrade(const char* pkgName);

int ndk_pkg_cleanup();

#endif

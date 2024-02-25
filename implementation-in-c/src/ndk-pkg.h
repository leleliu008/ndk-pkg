#ifndef NDKPKG_H
#define NDKPKG_H

#include <config.h>

#include <stdlib.h>
#include <stdbool.h>

#include "core/sysinfo.h"
#include "core/tar.h"


#define NDKPKG_PACKAGE_NAME_PATTERN "^[A-Za-z0-9+-._@]{1,50}$"

 
#define NDKPKG_OK                     0
#define NDKPKG_ERROR                  1

#define NDKPKG_ERROR_ARG_IS_NULL      2
#define NDKPKG_ERROR_ARG_IS_EMPTY     3
#define NDKPKG_ERROR_ARG_IS_INVALID   4
#define NDKPKG_ERROR_ARG_IS_UNKNOWN   5

#define NDKPKG_ERROR_MEMORY_ALLOCATE  6

#define NDKPKG_ERROR_SHA256_MISMATCH  7

#define NDKPKG_ERROR_ENV_HOME_NOT_SET 8
#define NDKPKG_ERROR_ENV_PATH_NOT_SET 9

#define NDKPKG_ERROR_NOT_FOUND    10
#define NDKPKG_ERROR_NOT_MATCH    11

#define NDKPKG_ERROR_PACKAGE_SPEC_IS_INVALID 15

#define NDKPKG_ERROR_PACKAGE_NOT_AVAILABLE 20
#define NDKPKG_ERROR_PACKAGE_NOT_INSTALLED 21
#define NDKPKG_ERROR_PACKAGE_NOT_OUTDATED  22
#define NDKPKG_ERROR_PACKAGE_IS_BROKEN     23

#define NDKPKG_ERROR_FORMULA_REPO_NOT_FOUND 30
#define NDKPKG_ERROR_FORMULA_REPO_HAS_EXIST 31
#define NDKPKG_ERROR_FORMULA_REPO_IS_BROKEN 32
#define NDKPKG_ERROR_FORMULA_REPO_CONFIG_SYNTAX 34
#define NDKPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME 35

#define NDKPKG_ERROR_FORMULA_SYNTAX     40
#define NDKPKG_ERROR_FORMULA_SCHEME     41

#define NDKPKG_ERROR_RECEIPT_SYNTAX     45
#define NDKPKG_ERROR_RECEIPT_SCHEME     46

#define NDKPKG_ERROR_URL_TRANSFORM_ENV_NOT_SET           50
#define NDKPKG_ERROR_URL_TRANSFORM_ENV_VALUE_IS_EMPTY    51
#define NDKPKG_ERROR_URL_TRANSFORM_ENV_POINT_TO_PATH_NOT_EXIST 52
#define NDKPKG_ERROR_URL_TRANSFORM_RUN_NO_RESULT         53

// libgit's error [-35, -1]
#define NDKPKG_ERROR_LIBGIT2_BASE    70

// libarchive's error [-30, 1]
#define NDKPKG_ERROR_ARCHIVE_BASE    110

// libcurl's error [1, 99]
#define NDKPKG_ERROR_NETWORK_BASE    150


typedef struct {
    char * summary;
    char * version;
    char * license;

    char * web_url;

    char * git_url;
    char * git_sha;
    char * git_ref;
    size_t git_nth;

    char * src_url;
    char * src_uri;
    char * src_sha;
    bool   src_is_dir;

    char * fix_url;
    char * fix_uri;
    char * fix_sha;

    char * res_url;
    char * res_uri;
    char * res_sha;

    char * dep_pkg;
    char * dep_upp;
    char * dep_pym;
    char * dep_plm;

    char * bsystem;
    char * bscript;

    bool   binbstd;
    bool   parallel;

    bool   symlink;

    bool   sfslink;

    char * ppflags;
    char * ccflags;
    char * xxflags;
    char * ldflags;

    char * do12345;
    char * dopatch;
    char * install;

    char * path;

    bool web_url_is_calculated;
    bool version_is_calculated;
    bool bsystem_is_calculated;

    bool useBuildSystemAutogen;
    bool useBuildSystemAutotools;
    bool useBuildSystemConfigure;
    bool useBuildSystemCmake;
    bool useBuildSystemXmake;
    bool useBuildSystemGmake;
    bool useBuildSystemMeson;
    bool useBuildSystemNinja;
    bool useBuildSystemCargo;
    bool useBuildSystemGolang;

} NDKPKGFormula;

int  ndkpkg_formula_parse(const char * formulaFilePath, NDKPKGFormula * * out);
int  ndkpkg_formula_lookup(const char * packageName, NDKPKGFormula * * formula);
int  ndkpkg_formula_locate(const char * packageName, char * * out);
int  ndkpkg_formula_edit(const char * packageName, const char * editor);
int  ndkpkg_formula_view(const char * packageName, const bool raw);
int  ndkpkg_formula_cat (const char * packageName);
int  ndkpkg_formula_bat (const char * packageName);


void ndkpkg_formula_free(NDKPKGFormula * formula);
void ndkpkg_formula_dump(NDKPKGFormula * formula);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * name;
    char * url;
    char * branch;
    char * path;
    char * createdAt;
    char * updatedAt;
    bool   pinned;
    bool   enabled;
} NDKPKGFormulaRepo ;

typedef struct {
    NDKPKGFormulaRepo * * repos;
    size_t size;
} NDKPKGFormulaRepoList ;

int  ndkpkg_formula_repo_create(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  ndkpkg_formula_repo_add   (const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  ndkpkg_formula_repo_remove(const char * formulaRepoName);
int  ndkpkg_formula_repo_sync_ (const char * formulaRepoName);
int  ndkpkg_formula_repo_info_ (const char * formulaRepoName);
int  ndkpkg_formula_repo_config(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  ndkpkg_formula_repo_config_write(const char * formulaRepoDIRPath, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled, const char * createdAt, const char * updatedAt);
int  ndkpkg_formula_repo_lookup(const char * formulaRepoName, NDKPKGFormulaRepo * * formulaRepo);
int  ndkpkg_formula_repo_parse (const char * formulaRepoConfigFilePath, NDKPKGFormulaRepo * * formulaRepo);

void ndkpkg_formula_repo_free(NDKPKGFormulaRepo * formulaRepo);
void ndkpkg_formula_repo_dump(NDKPKGFormulaRepo * formulaRepo);
int  ndkpkg_formula_repo_info(NDKPKGFormulaRepo * formulaRepo);
int  ndkpkg_formula_repo_sync(NDKPKGFormulaRepo * formulaRepo);

int  ndkpkg_formula_repo_list     (NDKPKGFormulaRepoList * * p);
void ndkpkg_formula_repo_list_free(NDKPKGFormulaRepoList   * p);

int  ndkpkg_formula_repo_list_printf();
int  ndkpkg_formula_repo_list_update();


//////////////////////////////////////////////////////////////////////

typedef enum {
    AndroidABI_arm64_v8a,
    AndroidABI_armeabi_v7a,
    AndroidABI_x86_64,
    AndroidABI_x86
} AndroidABI;

typedef struct {
    unsigned int api;
    AndroidABI   abi;
} NDKPKGTargetPlatform;

int ndkpkg_inspect_target_platform_spec(const char * targetPlatformSpec, NDKPKGTargetPlatform * targetPlatform);

int ndkpkg_inspect_package(const char * package, const char * userSpecifiedTargetPlatformSpec, const char ** packageName, NDKPKGTargetPlatform * targetPlatform);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * summary;
    char * version;
    char * license;

    char * web_url;

    char * git_url;
    char * git_sha;
    char * git_ref;
    size_t git_nth;

    char * src_url;
    char * src_uri;
    char * src_sha;

    char * fix_url;
    char * fix_uri;
    char * fix_sha;

    char * res_url;
    char * res_uri;
    char * res_sha;

    char * dep_pkg;
    char * dep_upp;
    char * dep_pym;
    char * dep_plm;

    char * bsystem;
    char * bscript;

    bool   binbstd;
    bool   parallel;

    bool   symlink;

    char * ppflags;
    char * ccflags;
    char * xxflags;
    char * ldflags;

    char * do12345;
    char * dopatch;
    char * install;

    char * path;

    char * builtBy;
    char * builtAt;
    char * builtFor;
} NDKPKGReceipt;

int  ndkpkg_receipt_parse(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, NDKPKGReceipt * * receipt);
void ndkpkg_receipt_free(NDKPKGReceipt * receipt);
void ndkpkg_receipt_dump(NDKPKGReceipt * receipt);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * cc;
    char * objc;
    char * cxx;
    char * cpp;
    char * as;
    char * ar;
    char * ranlib;
    char * ld;
    char * nm;
    char * strip;
    char * size;
    char * strings;
    char * objdump;
    char * objcopy;
    char * readelf;
    char * dlltool;
    char * addr2line;

    char * sysroot;

    char * ccflags;

    char * cxxflags;

    char * cppflags;

    char * ldflags;
} NDKPKGToolChain;

int  ndkpkg_toolchain_locate(NDKPKGToolChain * toolchain);
void ndkpkg_toolchain_free  (NDKPKGToolChain * toolchain);
void ndkpkg_toolchain_dump  (NDKPKGToolChain * toolchain);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * rootdir;
    char * sysroot;

    char * version;
    size_t versionMajor;

    char * cc;
    char * cxx;
    char * as;
    char * ar;
    char * ranlib;
    char * ld;
    char * nm;
    char * size;
    char * strip;
    char * strings;
    char * objdump;
    char * objcopy;
    char * readelf;

    char *  ccflags;
    char * cxxflags;
    char * cppflags;
    char *  ldflags;
} NDKPKGAndroidNDKToolChain;

int  ndkpkg_android_ndk_toolchain_make(NDKPKGAndroidNDKToolChain * toolchain, const char * ndkRootDIR);
int  ndkpkg_android_ndk_toolchain_find(NDKPKGAndroidNDKToolChain * toolchain);
void ndkpkg_android_ndk_toolchain_free(NDKPKGAndroidNDKToolChain * toolchain);
void ndkpkg_android_ndk_toolchain_dump(NDKPKGAndroidNDKToolChain * toolchain);

//////////////////////////////////////////////////////////////////////

int ndkpkg_main(int argc, char* argv[]);

int ndkpkg_util(int argc, char* argv[]);

int ndkpkg_help();

int ndkpkg_sysinfo();

int ndkpkg_buildinfo();

int ndkpkg_env(const bool verbose);

int ndkpkg_home_dir(char buf[], size_t bufSize, size_t * outSize);

int ndkpkg_session_dir(char buf[], size_t bufSize, size_t * outSize);

int ndkpkg_search(const char * regPattern, const bool verbose);

int ndkpkg_available_info(const char * packageName, const char * key);

int ndkpkg_installed_info(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const char * key);

int ndkpkg_tree(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, size_t argc, char* argv[]);

int ndkpkg_logs(const char * packageName, const NDKPKGTargetPlatform * targetPlatform);

int ndkpkg_pack(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, ArchiveType outputType, const char * outputPath, const bool verbose);

int ndkpkg_export_as_google_prefab_aar(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const char * outputPath, const bool verbose);

int ndkpkg_depoly_to_maven_repository(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const char * outputPath, const bool verbose);

typedef enum {
    NDKPKGDependsOutputType_DOT,
    NDKPKGDependsOutputType_BOX,
    NDKPKGDependsOutputType_SVG,
    NDKPKGDependsOutputType_PNG,
} NDKPKGDependsOutputType;

int ndkpkg_depends(const char * packageName, NDKPKGDependsOutputType outputType, const char * outputPath);

int ndkpkg_fetch(const char * packageName, const bool verbose);

//////////////////////////////////////////////////////////////////////

typedef enum {
    NDKPKGLogLevel_silent,
    NDKPKGLogLevel_normal,
    NDKPKGLogLevel_verbose,
    NDKPKGLogLevel_very_verbose
} NDKPKGLogLevel;

typedef enum {
    NDKPKGBuildType_release,
    NDKPKGBuildType_debug
} NDKPKGBuildType;

typedef enum {
    NDKPKGLinkType_shared_most,
    NDKPKGLinkType_shared_full,
    NDKPKGLinkType_static_most,
    NDKPKGLinkType_static_full
} NDKPKGLinkType;

typedef struct {
    bool   exportCompileCommandsJson;
    bool   keepSessionDIR;
    bool   enableCcache;
    bool   enableBear;
    bool   dryrun;
    bool   force;

    bool   xtrace;

    bool   verbose_net;
    bool   verbose_env;
    bool   verbose_cc;
    bool   verbose_ld;

    size_t parallelJobsCount;

    NDKPKGBuildType buildType;
    NDKPKGLinkType  linkType;

    NDKPKGLogLevel  logLevel;

    const char * ndkHome;
} NDKPKGInstallOptions;

int ndkpkg_install  (const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const NDKPKGInstallOptions * options);

int ndkpkg_upgrade  (const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const NDKPKGInstallOptions * options);

int ndkpkg_reinstall(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const NDKPKGInstallOptions * options);

int ndkpkg_uninstall(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const bool verbose);

int ndkpkg_upgrade_self(const bool verbose);

int ndkpkg_integrate_zsh_completion (const char * outputDIR, const bool verbose);
int ndkpkg_integrate_bash_completion(const char * outputDIR, const bool verbose);
int ndkpkg_integrate_fish_completion(const char * outputDIR, const bool verbose);

int ndkpkg_setup(const bool verbose);

int ndkpkg_cleanup(const bool verbose);

int ndkpkg_check_if_the_given_argument_matches_package_name_pattern(const char * arg);

int ndkpkg_check_if_the_given_package_is_available(const char * packageName);
int ndkpkg_check_if_the_given_package_is_installed(const char * packageName, const NDKPKGTargetPlatform * targetPlatform);
int ndkpkg_check_if_the_given_package_is_outdated (const char * packageName, const NDKPKGTargetPlatform * targetPlatform);

typedef int (*NDKPKGPackageNameFilter)(const char * packageName, const bool verbose, const size_t index, const void * payload);

int ndkpkg_show_the_available_packages(const bool verbose);

int ndkpkg_list_the_available_packages(const bool verbose, NDKPKGPackageNameFilter packageNameFilter, const void * payload);
int ndkpkg_list_the_installed_packages(const NDKPKGTargetPlatform * targetPlatform, const bool verbose);
int ndkpkg_list_the__outdated_packages(const NDKPKGTargetPlatform * targetPlatform, const bool verbose);

int ndkpkg_git_sync(const char * gitRepositoryDIRPath, const char * remoteUrl, const char * remoteRef, const char * remoteTrackingRef, const char * checkoutToBranchName, const size_t fetchDepth);

int ndkpkg_generate_url_transform_sample();

int ndkpkg_examine_filetype_from_url(const char * url, char buf[], size_t bufSize);

int ndkpkg_examine_filename_from_url(const char * url, char buf[], size_t bufSize);

int ndkpkg_http_fetch_to_file(const char * url, const char * outputFilePath, const bool verbose, const bool showProgress);

int ndkpkg_http_fetch_to_stream(const char * url, FILE * stream, const bool verbose, const bool showProgress);

int ndkpkg_download(const char * url, const char * uri, const char * expectedSHA256SUM, const char * outputPath, const bool verbose);

int ndkpkg_uncompress(const char * filePath, const char * unpackDIR, const size_t stripComponentsNumber, const bool verbose);

int ndkpkg_rename_or_copy_file(const char * fromFilePath, const char * toFilePath);

int ndkpkg_copy_file(const char * fromFilePath, const char * toFilePath);

int ndkpkg_mkdir_p(const char * dirPath, const bool verbose);

int ndkpkg_rm_r(const char * dirPath, const bool verbose);

int ndkpkg_setenv_SSL_CERT_FILE();

#endif

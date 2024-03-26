#ifndef UPPM_H
#define UPPM_H

#include <config.h>

#include <stdlib.h>
#include <stdbool.h>


#define UPPM_PACKAGE_NAME_PATTERN "^[A-Za-z0-9+-._@]{1,50}$"


#define UPPM_OK                     0
#define UPPM_ERROR                  1

#define UPPM_ERROR_ARG_IS_NULL      2
#define UPPM_ERROR_ARG_IS_EMPTY     3
#define UPPM_ERROR_ARG_IS_INVALID   4
#define UPPM_ERROR_ARG_IS_UNKNOWN   5

#define UPPM_ERROR_MEMORY_ALLOCATE  6

#define UPPM_ERROR_SHA256_MISMATCH  7

#define UPPM_ERROR_ENV_HOME_NOT_SET 8
#define UPPM_ERROR_ENV_PATH_NOT_SET 9

#define UPPM_ERROR_EXE_NOT_FOUND    10

#define UPPM_ERROR_PACKAGE_NOT_AVAILABLE 20
#define UPPM_ERROR_PACKAGE_NOT_INSTALLED 21
#define UPPM_ERROR_PACKAGE_NOT_OUTDATED  22
#define UPPM_ERROR_PACKAGE_IS_BROKEN     23

#define UPPM_ERROR_FORMULA_REPO_NOT_FOUND 30
#define UPPM_ERROR_FORMULA_REPO_HAS_EXIST 31
#define UPPM_ERROR_FORMULA_REPO_IS_BROKEN 32
#define UPPM_ERROR_FORMULA_REPO_CONFIG_SYNTAX 34
#define UPPM_ERROR_FORMULA_REPO_CONFIG_SCHEME 35

#define UPPM_ERROR_FORMULA_SYNTAX     40
#define UPPM_ERROR_FORMULA_SCHEME     41

#define UPPM_ERROR_RECEIPT_SYNTAX     45
#define UPPM_ERROR_RECEIPT_SCHEME     46

#define UPPM_ERROR_URL_TRANSFORM_ENV_NOT_SET           50
#define UPPM_ERROR_URL_TRANSFORM_ENV_VALUE_IS_EMPTY    51
#define UPPM_ERROR_URL_TRANSFORM_ENV_POINT_TO_PATH_NOT_EXIST 52
#define UPPM_ERROR_URL_TRANSFORM_RUN_NO_RESULT         53

// libgit's error [-35, -1]
#define UPPM_ERROR_LIBGIT2_BASE    70

// libarchive's error [-30, 1]
#define UPPM_ERROR_ARCHIVE_BASE    110

// libcurl's error [1, 99]
#define UPPM_ERROR_NETWORK_BASE    150

/*
 * This macro should be employed only if there is no memory should be freed before returing.
 */
#define UPPM_RETURN_IF_MEMORY_ALLOCATION_FAILED(ptr) if ((ptr) == NULL) { return UPPM_ERROR_MEMORY_ALLOCATE; }

#define UPPM_PERROR(ret, packageName, ...) \
    if (ret == UPPM_ERROR) { \
        fprintf(stderr, "occurs error.\n"); \
    } else if (ret == UPPM_ERROR_ARG_IS_NULL) { \
        fprintf(stderr, "package name not specified.\n"); \
    } else if (ret == UPPM_ERROR_ARG_IS_EMPTY) { \
        fprintf(stderr, "package name should be a non-empty string.\n"); \
    } else if (ret == UPPM_ERROR_ARG_IS_INVALID) { \
        fprintf(stderr, "package name not match pattern: %s, %s\n", packageName, UPPM_PACKAGE_NAME_PATTERN); \
    } else if (ret == UPPM_ERROR_PACKAGE_NOT_AVAILABLE) { \
        fprintf(stderr, "package not available: %s\n", packageName); \
    } else if (ret == UPPM_ERROR_PACKAGE_NOT_INSTALLED) { \
        fprintf(stderr, "package not installed: %s\n", packageName); \
    } else if (ret == UPPM_ERROR_PACKAGE_NOT_OUTDATED) { \
        fprintf(stderr, "package not outdated: %s\n", packageName); \
    } else if (ret == UPPM_ERROR_PACKAGE_IS_BROKEN) { \
        fprintf(stderr, "package is broken: %s\n", packageName); \
    } else if (ret == UPPM_ERROR_ENV_HOME_NOT_SET) { \
        fprintf(stderr, "%s\n", "HOME environment variable not set.\n"); \
    } else if (ret == UPPM_ERROR_ENV_PATH_NOT_SET) { \
        fprintf(stderr, "%s\n", "PATH environment variable not set.\n"); \
    } else if (ret == UPPM_ERROR_URL_TRANSFORM_ENV_NOT_SET) { \
        fprintf(stderr, "%s\n", "UPPM_URL_TRANSFORM environment variable not set.\n"); \
    } else if (ret == UPPM_ERROR_URL_TRANSFORM_ENV_VALUE_IS_EMPTY) { \
        fprintf(stderr, "%s\n", "UPPM_URL_TRANSFORM environment variable's value should be a non-empty string.\n"); \
    } else if (ret == UPPM_ERROR_URL_TRANSFORM_ENV_POINT_TO_PATH_NOT_EXIST) { \
        fprintf(stderr, "%s\n", "UPPM_URL_TRANSFORM environment variable's value point to path not exist.\n"); \
    } else if (ret == UPPM_ERROR_URL_TRANSFORM_RUN_NO_RESULT) { \
        fprintf(stderr, "%s\n", "UPPM_URL_TRANSFORM environment variable's value point to path runs no result.\n"); \
    } else if (ret > UPPM_ERROR_NETWORK_BASE) { \
        fprintf(stderr, "network error.\n"); \
    }

typedef struct {
    char * summary;
    char * version;
    char * license;
    char * webpage;
    char * bin_url;
    char * bin_sha;
    char * dep_pkg;
    char * unpackd;
    char * install;
    char * path;
} UPPMFormula;

int  uppm_formula_lookup(const char * packageName, UPPMFormula * * formula);
int  uppm_formula_locate(const char * packageName, char * * out);
int  uppm_formula_edit(const char * packageName, const char * editor);
int  uppm_formula_view(const char * packageName, const bool raw);
int  uppm_formula_cat (const char * packageName);
int  uppm_formula_bat (const char * packageName);

void uppm_formula_free(UPPMFormula * formula);
void uppm_formula_dump(UPPMFormula * formula);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * name;
    char * url;
    char * branch;
    char * path;
    char * timestamp_created;
    char * timestamp_updated;
    int    pinned;
    int    enabled;
} UPPMFormulaRepo ;

typedef struct {
    UPPMFormulaRepo * * repos;
    size_t size;
} UPPMFormulaRepoList ;

int  uppm_formula_repo_create(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  uppm_formula_repo_add   (const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  uppm_formula_repo_remove(const char * formulaRepoName);
int  uppm_formula_repo_sync_ (const char * formulaRepoName);
int  uppm_formula_repo_info_ (const char * formulaRepoName);
int  uppm_formula_repo_config(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  uppm_formula_repo_config_write(const char * formulaRepoDIRPath, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled, const char * timestamp_created, const char * timestamp_updated);
int  uppm_formula_repo_lookup(const char * formulaRepoName, UPPMFormulaRepo * * formulaRepo);
int  uppm_formula_repo_parse (const char * formulaRepoConfigFilePath, UPPMFormulaRepo * * formulaRepo);

void uppm_formula_repo_free(UPPMFormulaRepo * formulaRepo);
void uppm_formula_repo_dump(UPPMFormulaRepo * formulaRepo);
int  uppm_formula_repo_info(UPPMFormulaRepo * formulaRepo);
int  uppm_formula_repo_sync(UPPMFormulaRepo * formulaRepo);

int  uppm_formula_repo_list     (UPPMFormulaRepoList * * p);
void uppm_formula_repo_list_free(UPPMFormulaRepoList   * p);

int  uppm_formula_repo_list_printf();
int  uppm_formula_repo_list_update();

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * summary;
    char * version;
    char * license;
    char * webpage;
    char * bin_url;
    char * bin_sha;
    char * dep_pkg;
    char * install;

    char * signature;
    char * timestamp;
} UPPMReceipt;

int  uppm_receipt_parse(const char * packageName, UPPMReceipt * * receipt);
void uppm_receipt_free(UPPMReceipt * receipt);
void uppm_receipt_dump(UPPMReceipt * receipt);

//////////////////////////////////////////////////////////////////////

typedef enum {
    UPPMDependsOutputType_DOT,
    UPPMDependsOutputType_BOX,
    UPPMDependsOutputType_SVG,
    UPPMDependsOutputType_PNG,
} UPPMDependsOutputType;

//////////////////////////////////////////////////////////////////////

int uppm_main(int argc, char* argv[]);

int uppm_util(int argc, char* argv[]);

int uppm_help();

int uppm_sysinfo();

int uppm_buildinfo();

int uppm_env(const bool verbose);

int uppm_home_dir(char buf[], size_t bufSize, size_t * outSize);

int uppm_session_dir(char buf[], size_t bufSize, size_t * outSize);

int uppm_search(const char * regPattern, const bool verbose);

int uppm_available_info(const char * packageName, const char * key);
int uppm_installed_info(const char * packageName, const char * key);

int uppm_tree(const char * packageName, size_t argc, char* argv[]);

int uppm_depends(const char * packageName, UPPMDependsOutputType outputType, const char * outputPath);

int uppm_fetch(const char * packageName, const bool verbose);

int uppm_install(const char * packageName, const bool verbose, const bool force);

int uppm_reinstall(const char * packageName, const bool verbose);

int uppm_uninstall(const char * packageName, const bool verbose);

int uppm_upgrade(const char * packageName, const bool verbose);

int uppm_upgrade_self(const bool verbose);

int uppm_integrate_zsh_completion (const char * outputDIR, const bool verbose);
int uppm_integrate_bash_completion(const char * outputDIR, const bool verbose);
int uppm_integrate_fish_completion(const char * outputDIR, const bool verbose);

int uppm_cleanup(const bool verbose);

int uppm_check_if_the_given_argument_matches_package_name_pattern(const char * arg);

int uppm_check_if_the_given_package_is_available(const char * packageName);
int uppm_check_if_the_given_package_is_installed(const char * packageName);
int uppm_check_if_the_given_package_is_outdated (const char * packageName);

typedef int (*UPPMPackageNameFilter)(const char * packageName, const size_t index, const bool verbose, const void * payload);

int uppm_show_the_available_packages(const bool verbose);
int uppm_list_the_available_packages(const bool verbose, UPPMPackageNameFilter packageNameCallbak, const void * payload);
int uppm_list_the_installed_packages(const bool verbose);
int uppm_list_the__outdated_packages(const bool verbose);

int uppm_git_sync(const char * gitRepositoryDIRPath, const char * remoteUrl, const char * remoteRef, const char * remoteTrackingRef, const char * checkoutToBranchName);

int uppm_generate_url_transform_sample();

int uppm_examine_filetype_from_url(const char * url, char buf[], const size_t bufSize);

int uppm_http_fetch_to_file(const char * url, const char * outputFilePath, const bool verbose, const bool showProgress);

int uppm_download(const char * url, const char * sha256sum, const char * downloadDIR, const char * unpackDIR, const size_t stripComponentsNumber, const bool verbose);

int uppm_copy_file(const char * fromFilePath, const char * toFilePath);

int uppm_mkdir_p(const char * dirPath, const bool verbose);

int uppm_rm_r(const char * dirPath, const bool verbose);

#endif

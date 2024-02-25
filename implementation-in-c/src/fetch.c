#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/http.h"

#include "sha256sum.h"

#include "ndk-pkg.h"

static int ndkpkg_fetch_git(const char * packageName, NDKPKGFormula * formula, const char * ndkpkgDownloadsDIR, size_t ndkpkgDownloadsDIRLength) {
    size_t gitRepositoryDIRCapacity = ndkpkgDownloadsDIRLength + strlen(packageName) + 6U;
    char   gitRepositoryDIR[gitRepositoryDIRCapacity];

    int ret = snprintf(gitRepositoryDIR, gitRepositoryDIRCapacity, "%s/%s.git", ndkpkgDownloadsDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(gitRepositoryDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", gitRepositoryDIR);
            return NDKPKG_ERROR;
        }
    } else {
        if (mkdir(gitRepositoryDIR, S_IRWXU) != 0) {
            perror(gitRepositoryDIR);
            return NDKPKG_ERROR;
        }
    }

    const char * remoteRef;

    if (formula->git_sha == NULL) {
        remoteRef = (formula->git_ref == NULL) ? "HEAD" : formula->git_ref;
    } else {
        remoteRef = formula->git_sha;
    }
    
    return ndkpkg_git_sync(gitRepositoryDIR, formula->git_url, remoteRef, "refs/remotes/origin/master", "master", formula->git_nth);
}

static int ndkpkg_fetch_file(const char * url, const char * uri, const char * expectedSHA256SUM, const char * ndkpkgDownloadsDIR, const size_t ndkpkgDownloadsDIRLength, const bool verbose) {
    char fileNameExtension[21] = {0};

    int ret = ndkpkg_examine_filetype_from_url(url, fileNameExtension, 20);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    printf("==========>> fileNameExtension = %s\n", fileNameExtension);

    size_t fileNameCapacity = strlen(expectedSHA256SUM) + strlen(fileNameExtension) + 1U;
    char   fileName[fileNameCapacity];

    ret = snprintf(fileName, fileNameCapacity, "%s%s", expectedSHA256SUM, fileNameExtension);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t filePathCapacity = ndkpkgDownloadsDIRLength + fileNameCapacity + 1U;
    char   filePath[filePathCapacity];

    ret = snprintf(filePath, filePathCapacity, "%s/%s", ndkpkgDownloadsDIR, fileName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(filePath, &st) == 0 && S_ISREG(st.st_mode)) {
        char actualSHA256SUM[65] = {0};

        int ret = sha256sum_of_file(actualSHA256SUM, filePath);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            fprintf(stderr, "%s already have been fetched.\n", filePath);
            return NDKPKG_OK;
        }
    }

    ret = ndkpkg_http_fetch_to_file(url, filePath, verbose, verbose);

    if (ret != NDKPKG_OK) {
        if (uri != NULL && uri[0] != '\0') {
            ret = ndkpkg_http_fetch_to_file(uri, filePath, verbose, verbose);
        }
    }

    if (ret != NDKPKG_OK) {
        return ret;
    }

    char actualSHA256SUM[65] = {0};

    ret = sha256sum_of_file(actualSHA256SUM, filePath);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
        return NDKPKG_OK;
    } else {
        fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", expectedSHA256SUM, actualSHA256SUM);
        return NDKPKG_ERROR_SHA256_MISMATCH;
    }
}

int ndkpkg_fetch(const char * packageName, const bool verbose) {
    NDKPKGFormula * formula = NULL;

    int ret = ndkpkg_formula_lookup(packageName, &formula);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////

    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    size_t ndkpkgDownloadsDIRCapacity = ndkpkgHomeDIRLength + 11U;
    char   ndkpkgDownloadsDIR[ndkpkgDownloadsDIRCapacity];

    ret = snprintf(ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, "%s/downloads", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(ndkpkgDownloadsDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(ndkpkgDownloadsDIR) != 0) {
                perror(ndkpkgDownloadsDIR);
                ndkpkg_formula_free(formula);
                return NDKPKG_ERROR;
            }

            if (mkdir(ndkpkgDownloadsDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ndkpkgDownloadsDIR);
                    ndkpkg_formula_free(formula);
                    return NDKPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(ndkpkgDownloadsDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(ndkpkgDownloadsDIR);
                ndkpkg_formula_free(formula);
                return NDKPKG_ERROR;
            }
        }
    }

    ///////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        ret = ndkpkg_fetch_git(packageName, formula, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity);
    } else {
        if (formula->src_is_dir) {
            fprintf(stderr, "src_url is point to local dir, so no need to fetch.\n");
        } else {
            ret = ndkpkg_fetch_file(formula->src_url, formula->src_uri, formula->src_sha, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, verbose);
        }
    }

    if (ret != NDKPKG_OK) {
        goto finalize;
    }

    if (formula->fix_url != NULL) {
        ret = ndkpkg_fetch_file(formula->fix_url, formula->fix_uri, formula->fix_sha, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, verbose);

        if (ret != NDKPKG_OK) {
            goto finalize;
        }
    }

    if (formula->res_url != NULL) {
        ret = ndkpkg_fetch_file(formula->res_url, formula->res_uri, formula->res_sha, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, verbose);

        if (ret != NDKPKG_OK) {
            goto finalize;
        }
    }

finalize:
    ndkpkg_formula_free(formula);
    return ret;
}

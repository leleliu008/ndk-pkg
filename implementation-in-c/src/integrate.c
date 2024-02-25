#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "ndk-pkg.h"

int ndkpkg_integrate_zsh_completion(const char * outputDIR, const bool verbose) {
    char   ndkpkgHomeDIR[PATH_MAX] = {0};
    size_t ndkpkgHomeDIRLength;

    int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    size_t ndkpkgRunDIRCapacity = ndkpkgHomeDIRLength + 5U;
    char   ndkpkgRunDIR[ndkpkgRunDIRCapacity];

    ret = snprintf(ndkpkgRunDIR, ndkpkgRunDIRCapacity, "%s/run", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (lstat(ndkpkgRunDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(ndkpkgRunDIR) != 0) {
                perror(ndkpkgRunDIR);
                return NDKPKG_ERROR;
            }

            if (mkdir(ndkpkgRunDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ndkpkgRunDIR);
                    return NDKPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(ndkpkgRunDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(ndkpkgRunDIR);
                return NDKPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t sessionDIRCapacity = ndkpkgRunDIRCapacity + 20U;
    char   sessionDIR[sessionDIRCapacity];

    ret = snprintf(sessionDIR, sessionDIRCapacity, "%s/%d", ndkpkgRunDIR, getpid());

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (lstat(sessionDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = ndkpkg_rm_r(sessionDIR, false);

            if (ret != NDKPKG_OK) {
                return ret;
            }

            if (mkdir(sessionDIR, S_IRWXU) != 0) {
                perror(sessionDIR);
                return NDKPKG_ERROR;
            }
        } else {
            if (unlink(sessionDIR) != 0) {
                perror(sessionDIR);
                return NDKPKG_ERROR;
            }

            if (mkdir(sessionDIR, S_IRWXU) != 0) {
                perror(sessionDIR);
                return NDKPKG_ERROR;
            }
        }
    } else {
        if (mkdir(sessionDIR, S_IRWXU) != 0) {
            perror(sessionDIR);
            return NDKPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRCapacity + 7U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/_ndkpkg", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    const char * const url = "https://raw.githubusercontent.com/leleliu008/ndkpkg/master/ndkpkg-zsh-completion";

    ret = ndkpkg_http_fetch_to_file(url, tmpFilePath, verbose, verbose);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t defaultOutputDIRCapacity = ndkpkgHomeDIRLength + 26U;
    char   defaultOutputDIR[defaultOutputDIRCapacity];

    ret = snprintf(defaultOutputDIR, defaultOutputDIRCapacity, "%s/share/zsh/site-functions", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t outputDIRLength;

    if (outputDIR == NULL) {
        outputDIR       = defaultOutputDIR;
        outputDIRLength = ret;
    } else {
        outputDIRLength = strlen(outputDIR);
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    ret = ndkpkg_mkdir_p(outputDIR, verbose);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t outputFilePathCapacity = outputDIRLength + 7U;
    char   outputFilePath[outputFilePathCapacity];

    ret = snprintf(outputFilePath, outputFilePathCapacity, "%s/_ndkpkg", outputDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (rename(tmpFilePath, outputFilePath) != 0) {
        if (errno == EXDEV) {
            ret = ndkpkg_copy_file(tmpFilePath, outputFilePath);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        } else {
            perror(outputFilePath);
            return NDKPKG_ERROR;
        }
    }

    printf("zsh completion script for ndkpkg has been written to %s\n", outputFilePath);
    return NDKPKG_OK;
}

int ndkpkg_integrate_bash_completion(const char * outputDIR, const bool verbose) {
    (void)outputDIR;
    (void)verbose;
    return NDKPKG_OK;
}

int ndkpkg_integrate_fish_completion(const char * outputDIR, const bool verbose) {
    (void)outputDIR;
    (void)verbose;
    return NDKPKG_OK;
}

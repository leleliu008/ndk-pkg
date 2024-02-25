#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include <git2.h>

#include "core/log.h"

#include "ndk-pkg.h"

int ndkpkg_formula_repo_create(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled) {
    if (formulaRepoName == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepoName[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    if (formulaRepoUrl == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }


    if (formulaRepoUrl[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    if (branchName == NULL || branchName[0] == '\0') {
        branchName = (char*)"master";
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    size_t formulaRepoDIRCapacity = ndkpkgHomeDIRLength + strlen(formulaRepoName) + 10U;
    char   formulaRepoDIR[formulaRepoDIRCapacity];

    ret = snprintf(formulaRepoDIR, formulaRepoDIRCapacity, "%s/repos.d/%s", ndkpkgHomeDIR, formulaRepoName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (stat(formulaRepoDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            LOG_ERROR2("formula repo '%s' already exist.", formulaRepoName);
            return NDKPKG_ERROR_FORMULA_REPO_HAS_EXIST;
        } else {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", formulaRepoDIR);
            return NDKPKG_ERROR;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////

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

    size_t subDIRCapacity = sessionDIRCapacity + 9U;
    char   subDIR[subDIRCapacity];

    ret = snprintf(subDIR, subDIRCapacity, "%s/formula", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir(subDIR, S_IRWXU) != 0) {
        perror(subDIR);
        return NDKPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    git_repository   * gitRepo   = NULL;
    git_remote       * gitRemote = NULL;
    const git_error  * gitError  = NULL;

    git_libgit2_init();

    ret = git_repository_init(&gitRepo, sessionDIR, false);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        fprintf(stderr, "%s\n", gitError->message);
        git_repository_state_cleanup(gitRepo);
        git_repository_free(gitRepo);
        git_libgit2_shutdown();
        return abs(ret) + NDKPKG_ERROR_LIBGIT2_BASE;
    }

    //https://libgit2.org/libgit2/#HEAD/group/remote/git_remote_create
    ret = git_remote_create(&gitRemote, gitRepo, "origin", formulaRepoUrl);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        fprintf(stderr, "%s\n", gitError->message);
        git_repository_state_cleanup(gitRepo);
        git_repository_free(gitRepo);
        git_libgit2_shutdown();
        return abs(ret) + NDKPKG_ERROR_LIBGIT2_BASE;
    }

    git_repository_state_cleanup(gitRepo);
    git_repository_free(gitRepo);
    git_remote_free(gitRemote);
    git_libgit2_shutdown();

    ////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];

    ret = snprintf(ts, 11, "%ld", time(NULL));

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = ndkpkg_formula_repo_config_write(sessionDIR, formulaRepoUrl, branchName, pinned, enabled, ts, NULL);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoRootDIRCapacity = ndkpkgHomeDIRLength + 9U;
    char   formulaRepoRootDIR[formulaRepoRootDIRCapacity];

    ret = snprintf(formulaRepoRootDIR, formulaRepoRootDIRCapacity, "%s/repos.d", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (lstat(formulaRepoRootDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(formulaRepoRootDIR) != 0) {
                perror(formulaRepoRootDIR);
                return NDKPKG_ERROR;
            }

            if (mkdir(formulaRepoRootDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(formulaRepoRootDIR);
                    return NDKPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(formulaRepoRootDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(formulaRepoRootDIR);
                return NDKPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    if (rename(sessionDIR, formulaRepoDIR) == 0) {
        return NDKPKG_OK;
    } else {
        perror(sessionDIR);
        return NDKPKG_ERROR;
    }
}

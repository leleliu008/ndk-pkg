#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <git2.h>

#include "core/url-transform.h"

#include "ndk-pkg.h"

typedef struct {
    git_indexer_progress indexerProgress;
} ProgressPayload;

// https://libgit2.org/libgit2/#HEAD/group/callback/git_transport_message_cb
int git_transport_message_callback(const char *str, int len, void *payload) {
    (void)payload; /* unused */
	printf("remote: %.*s", len, str);
	fflush(stdout);
	return 0;
}

// https://libgit2.org/libgit2/#HEAD/group/callback/git_indexer_progress_cb
int git_indexer_progress_callback(const git_indexer_progress *stats, void *payload) {
    ProgressPayload * progressPayload = (ProgressPayload*)payload;
    progressPayload->indexerProgress = *stats;
    return 0;
}

// https://libgit2.org/libgit2/#HEAD/group/callback/git_checkout_progress_cb
void git_checkout_progress_callback(const char *path, size_t completed_steps, size_t total_steps, void *payload) {
    if (completed_steps == total_steps) {
        ProgressPayload * progressPayload = (ProgressPayload*)payload;
        git_indexer_progress indexerProgress = progressPayload->indexerProgress;

        if (indexerProgress.received_objects != 0) {
            printf("Receiving objects: 100%% (%u/%u), %.2f KiB, done.\n", indexerProgress.received_objects, indexerProgress.total_objects, indexerProgress.received_bytes / 1024.0);
        }

        if (indexerProgress.indexed_deltas != 0) {
            printf("Resolving deltas: 100%% (%u/%u), done.\n", indexerProgress.indexed_deltas, indexerProgress.total_deltas);
        }
    }
}

// https://libgit2.org/libgit2/#HEAD/group/credential/git_credential_ssh_key_new
// https://libgit2.org/libgit2/#HEAD/group/callback/git_credential_acquire_cb
int git_credential_acquire_callback(git_credential **credential, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload) {
    const char * const userHomeDIR = getenv("HOME");

    if (userHomeDIR == NULL) {
        return 1;
    }

    if (userHomeDIR[0] == '\0') {
        return 1;
    }

    size_t sshPrivateKeyFilePathLength = strlen(userHomeDIR) + 20U;
    char   sshPrivateKeyFilePath[sshPrivateKeyFilePathLength];

    int ret = snprintf(sshPrivateKeyFilePath, sshPrivateKeyFilePathLength, "%s/.ssh/id_rsa", userHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return 1;
    }

    struct stat st;

    if ((stat(sshPrivateKeyFilePath, &st) == 0) && S_ISREG(st.st_mode)) {
        git_credential_ssh_key_new(credential, username_from_url, NULL, sshPrivateKeyFilePath, NULL);
        return 0;
    }

    ret = snprintf(sshPrivateKeyFilePath, sshPrivateKeyFilePathLength, "%s/.ssh/id_ed25519", userHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return 1;
    }

    if ((stat(sshPrivateKeyFilePath, &st) == 0) && S_ISREG(st.st_mode)) {
        git_credential_ssh_key_new(credential, username_from_url, NULL, sshPrivateKeyFilePath, NULL);
        return 0;
    }

    return 1;
}

void print_git_oid(const git_oid oid, const char * prefix) {
    char sha1sum[41] = {0};

    for (int i = 0; i < 20; i++) {
        int j = 2 * i;
        sprintf(&sha1sum[j], "%02x", (unsigned int)(oid.id[i]));
    }

    printf("%s=%s\n", prefix, sha1sum);
}

int check_if_is_a_empty_dir(const char * dirpath, bool * value) {
    DIR * dir = opendir(dirpath);

    if (dir == NULL) {
        perror(dirpath);
        return NDKPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                break;
            } else {
                perror(dirpath);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);
        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        closedir(dir);

        (*value) = false;
        return NDKPKG_OK;
    }

    (*value) = true;
    return NDKPKG_OK;
}

// implement following steps:
// git -c init.defaultBranch=master init
// git remote add origin https://github.com/leleliu008/ndkpkg-formula-repository-official-core.git
// git fetch --progress origin +refs/heads/master:refs/remotes/origin/master
// git checkout --progress --force -B master refs/remotes/origin/master
int ndkpkg_git_sync(const char * repositoryDIR, const char * remoteUrl, const char * remoteRefPath, const char * remoteTrackingRefPath, const char * checkoutToBranchName, const size_t fetchDepth) {
    if ((repositoryDIR == NULL) || (repositoryDIR[0] == '\0')) {
        repositoryDIR = ".";
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (remoteUrl == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (remoteUrl[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (remoteRefPath == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (remoteRefPath[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (remoteTrackingRefPath == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (remoteTrackingRefPath[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (checkoutToBranchName == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (checkoutToBranchName[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    char transformedUrl[1025] = {0};

    const char * urlTransformCommandPath = getenv("NDKPKG_URL_TRANSFORM");

    if ((urlTransformCommandPath == NULL) || (urlTransformCommandPath[0] == '\0')) {
        strncpy(transformedUrl, remoteUrl, strlen(remoteUrl));
        transformedUrl[1024] = '\0';
    } else {
        fprintf(stderr, "\nyou have set NDKPKG_URL_TRANSFORM=%s\n", urlTransformCommandPath);
        fprintf(stderr, "transform from: %s\n", remoteUrl);

        size_t writtenSize = 0;

        if (url_transform(urlTransformCommandPath, remoteUrl, transformedUrl, 1024, &writtenSize, true) != 0) {
            perror(urlTransformCommandPath);
            return NDKPKG_ERROR;
        }

        if (writtenSize == 0) {
            fprintf(stderr, "a new url was expected to be output, but it was not.\n");
            return NDKPKG_ERROR;
        }

        fprintf(stderr, "transform   to: %s\n", transformedUrl);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    bool needInitGitRepo = false;

    struct stat st;

    if (stat(repositoryDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            bool isEmptyDIR = false;

            int ret = check_if_is_a_empty_dir(repositoryDIR, &isEmptyDIR);

            if (ret != NDKPKG_OK) {
                return NDKPKG_ERROR;
            }

            if (isEmptyDIR) {
                needInitGitRepo = true;
            } else {
                needInitGitRepo = false;
            }
        } else {
            fprintf(stderr, "%s exist and it is not a git repository.", repositoryDIR);
            return NDKPKG_ERROR;
        }
    } else {
        fprintf(stderr, "%s dir is not exist.", repositoryDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    git_libgit2_init();

    //////////////////////////////////////////////////////////////////////////////////////////////

    size_t checkoutToBranchRefPathLength = strlen(checkoutToBranchName) + 12U;
    char   checkoutToBranchRefPath[checkoutToBranchRefPathLength];

    int ret = snprintf(checkoutToBranchRefPath, checkoutToBranchRefPathLength, "refs/heads/%s", checkoutToBranchName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    git_repository   * gitRepo        = NULL;

    git_config       * gitConfig      = NULL;
    git_config_entry * gitConfigEntry = NULL;

    git_remote       * gitRemote      = NULL;

    git_reference * checkoutToBranchRefPointer = NULL;
    git_reference * remoteTrackingRefPointer = NULL;

    const git_oid * remoteTrackingRefHEADCommitOid     = NULL;
    git_object    * remoteTrackingRefHEADCommitPointer = NULL;
    git_tree      * remoteTrackingRefHEADCommitPointToTree   = NULL;

    const git_error * gitError        = NULL;

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (needInitGitRepo) {
        ret = git_repository_init(&gitRepo, repositoryDIR, false);

        if (ret != GIT_OK) {
            gitError = git_error_last();
            fprintf(stderr, "%s\n", gitError->message);
            git_repository_state_cleanup(gitRepo);
            git_repository_free(gitRepo);
            git_libgit2_shutdown();
            return abs(ret) + NDKPKG_ERROR_LIBGIT2_BASE;
        }

        //https://libgit2.org/libgit2/#HEAD/group/remote/git_remote_create
        ret = git_remote_create(&gitRemote, gitRepo, "origin", transformedUrl);

        if (ret != GIT_OK) {
            gitError = git_error_last();
            fprintf(stderr, "%s\n", gitError->message);
            git_repository_state_cleanup(gitRepo);
            git_repository_free(gitRepo);
            git_libgit2_shutdown();
            return abs(ret) + NDKPKG_ERROR_LIBGIT2_BASE;
        }
    } else {
        ret = git_repository_open_ext(&gitRepo, repositoryDIR, GIT_REPOSITORY_OPEN_NO_SEARCH, NULL);

        if (ret != GIT_OK) {
            gitError = git_error_last();
            fprintf(stderr, "%s\n", gitError->message);
            git_repository_state_cleanup(gitRepo);
            git_repository_free(gitRepo);
            git_libgit2_shutdown();
            return abs(ret) + NDKPKG_ERROR_LIBGIT2_BASE;
        }

        // https://libgit2.org/libgit2/#HEAD/group/remote/git_remote_lookup
        ret = git_remote_lookup(&gitRemote, gitRepo, "origin");

        if (ret == GIT_ENOTFOUND) {
            //https://libgit2.org/libgit2/#HEAD/group/remote/git_remote_create
            ret = git_remote_create(&gitRemote, gitRepo, "origin", transformedUrl);
        } else if (ret == GIT_OK) {
            ret = git_remote_set_instance_url(gitRemote, transformedUrl);
        }

        if (ret != GIT_OK) {
            gitError = git_error_last();
            fprintf(stderr, "%s\n", gitError->message);
            git_repository_state_cleanup(gitRepo);
            git_repository_free(gitRepo);
            git_libgit2_shutdown();
            return abs(ret) + NDKPKG_ERROR_LIBGIT2_BASE;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    ProgressPayload progressPayload = {0};

    git_remote_callbacks gitRemoteCallbacks = GIT_REMOTE_CALLBACKS_INIT;

	gitRemoteCallbacks.sideband_progress = git_transport_message_callback;
	gitRemoteCallbacks.transfer_progress = git_indexer_progress_callback;
	gitRemoteCallbacks.credentials       = git_credential_acquire_callback;
	gitRemoteCallbacks.payload           = &progressPayload;

    git_fetch_options gitFetchOptions = GIT_FETCH_OPTIONS_INIT;
    gitFetchOptions.callbacks = gitRemoteCallbacks;
    //gitFetchOptions.depth = (int)fetchDepth;

    git_checkout_options gitCheckoutOptions = GIT_CHECKOUT_OPTIONS_INIT;
    gitCheckoutOptions.checkout_strategy    = GIT_CHECKOUT_FORCE;
    gitCheckoutOptions.progress_cb          = git_checkout_progress_callback;
    gitCheckoutOptions.progress_payload     = &progressPayload;

    if (remoteRefPath == NULL && remoteTrackingRefPath == NULL) {
        ret = git_remote_fetch(gitRemote, NULL, &gitFetchOptions, NULL);
    } else {
        size_t refspecLength = strlen(remoteRefPath) + strlen(remoteTrackingRefPath) + 2;
        char   refspec[refspecLength];

        ret = snprintf(refspec, refspecLength, "%s:%s", remoteRefPath, remoteTrackingRefPath);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        git_strarray refspecArray = {.count = 1};
        char* strings[1] = {(char*)refspec};
        refspecArray.strings = strings;
        ret = git_remote_fetch(gitRemote, &refspecArray, &gitFetchOptions, NULL);
    }

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    ret = git_reference_lookup(&remoteTrackingRefPointer, gitRepo, remoteTrackingRefPath);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    remoteTrackingRefHEADCommitOid = git_reference_target(remoteTrackingRefPointer);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    ret = git_branch_lookup(&checkoutToBranchRefPointer, gitRepo, checkoutToBranchName, GIT_BRANCH_LOCAL);

    if (ret == GIT_OK) {
        const git_oid * checkoutToBranchHEADOid = git_reference_target(checkoutToBranchRefPointer);

        if (NULL != checkoutToBranchHEADOid) {
            //print_git_oid((*remoteTrackingRefHEADCommitOid), "remoteTrackingRefHEADCommitOid");
            //print_git_oid(checkoutToBranchHEADOid, "checkoutToBranchHEADOid");
            // remote tracking branch's SHA-1 is equal to locale tracking branch's HEAD SHA-1, means no need to perform merge
            if (memcmp(remoteTrackingRefHEADCommitOid->id, checkoutToBranchHEADOid->id, 20) == 0) {
                ret = git_repository_set_head(gitRepo, checkoutToBranchRefPath);

                if (ret != GIT_OK) {
                    gitError = git_error_last();
                }

                goto finalize;
            }
        }
    }

    if (ret == GIT_ENOTFOUND) {
        ret = git_reference_create(&checkoutToBranchRefPointer, gitRepo, checkoutToBranchRefPath, remoteTrackingRefHEADCommitOid, false, NULL);
    }

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    // git cat-file -p FETCH_HEAD
    // tree 29bd7db599c429dd821a8564196a02daebe09465
    // parent eb86afc806d4df01763663c1f2a9b411bddf3a82
    // author leleliu008 <leleliu008@gmail.com> 1673391639 +0800
    // committer leleliu008 <leleliu008@gmail.com> 1673391639 +0800
    //
    // optimized

    // https://libgit2.org/libgit2/#HEAD/group/reference/git_reference_peel
    ret = git_reference_peel(&remoteTrackingRefHEADCommitPointer, remoteTrackingRefPointer, GIT_OBJ_COMMIT);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    // https://libgit2.org/libgit2/#HEAD/group/commit/git_commit_tree
    ret = git_commit_tree(&remoteTrackingRefHEADCommitPointToTree, (git_commit*)remoteTrackingRefHEADCommitPointer);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    // https://libgit2.org/libgit2/#HEAD/group/checkout/git_checkout_tree
    ret = git_checkout_tree(gitRepo, (git_object*)remoteTrackingRefHEADCommitPointToTree, &gitCheckoutOptions);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    {
        git_reference * checkoutToBranchRefPointerNew = NULL;

        // https://libgit2.org/libgit2/#HEAD/group/reference/git_reference_set_target
        ret = git_reference_set_target(&checkoutToBranchRefPointerNew, checkoutToBranchRefPointer, remoteTrackingRefHEADCommitOid, NULL);

        git_reference_free(checkoutToBranchRefPointerNew);

        if (ret != GIT_OK) {
            gitError = git_error_last();
            goto finalize;
        }
    }

    ret = git_repository_set_head(gitRepo, checkoutToBranchRefPath);

    if (ret != GIT_OK) {
        gitError = git_error_last();
    }

finalize:
    if (ret == GIT_OK) {
        printf("%s\n", "Already up to date.");
    } else {
        if (gitError != NULL) {
            fprintf(stderr, "%s\n", gitError->message);
        }
    }

    git_repository_state_cleanup(gitRepo);

    git_repository_free(gitRepo);
    git_remote_free(gitRemote);

    git_reference_free(remoteTrackingRefPointer);
    git_object_free(remoteTrackingRefHEADCommitPointer);
    git_tree_free(remoteTrackingRefHEADCommitPointToTree);

    git_reference_free(checkoutToBranchRefPointer);

    git_config_free(gitConfig);
    git_config_entry_free(gitConfigEntry);

    git_libgit2_shutdown();

    return ret == GIT_OK ? NDKPKG_OK : abs(ret) + NDKPKG_ERROR_LIBGIT2_BASE;
}

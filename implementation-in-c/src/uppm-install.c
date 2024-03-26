#include <time.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/sysinfo.h"
#include "core/self.h"
#include "core/tar.h"

#include "sha256sum.h"
#include "uppm.h"

extern int uppm_record_installed_files(const char * packageInstalledRootDIRPath);

int uppm_install(const char * packageName, const bool verbose, const bool force) {
    UPPMFormula * formula = NULL;

    int ret = uppm_formula_lookup(packageName, &formula);

    if (ret != UPPM_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////

    if (formula->dep_pkg != NULL) {
        size_t depPackageNamesLength = strlen(formula->dep_pkg);

        size_t depPackageNamesCopyLength = depPackageNamesLength + 1U;
        char   depPackageNamesCopy[depPackageNamesCopyLength];
        strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

        char * depPackageNameArrayList[10];
        size_t depPackageNameArrayListSize = 0;

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            depPackageNameArrayList[depPackageNameArrayListSize] = depPackageName;
            depPackageNameArrayListSize++;
            depPackageName = strtok(NULL, " ");
        }

        for (size_t i = 0; i < depPackageNameArrayListSize; i++) {
            ret = uppm_install(depPackageNameArrayList[i], verbose, force);

            if (ret != UPPM_OK) {
                uppm_formula_free(formula);
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    char   uppmHomeDIR[PATH_MAX];
    size_t uppmHomeDIRLength;

    ret = uppm_home_dir(uppmHomeDIR, PATH_MAX, &uppmHomeDIRLength);

    if (ret != UPPM_OK) {
        return ret;
    }

    if (!force) {
        ret = uppm_check_if_the_given_package_is_installed(packageName);

        if (ret == UPPM_OK) {
            uppm_formula_free(formula);
            fprintf(stderr, "uppm package '%s' already has been installed.\n", packageName);
            return UPPM_OK;
        }

        if (ret != UPPM_ERROR_PACKAGE_NOT_INSTALLED) {
            uppm_formula_free(formula);
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    if (verbose) {
        fprintf(stderr, "prepare to install package [%s].\n", packageName);
    }

    //////////////////////////////////////////////////////////////////////////

    pid_t pid = getpid();

    time_t ts = time(NULL);

    size_t tmpStrCapacity = strlen(formula->bin_url) + 30U;
    char   tmpStr[tmpStrCapacity];

    ret = snprintf(tmpStr, tmpStrCapacity, "%s|%ld|%d", formula->bin_url, ts, pid);

    if (ret < 0) {
        perror(NULL);
        return UPPM_ERROR;
    }

    char sessionID[65] = {0};

    ret = sha256sum_of_string(sessionID, tmpStr);

    if (ret != 0) {
        uppm_formula_free(formula);
        return UPPM_ERROR;
    }

    if (verbose) {
        printf("%d: ts = %lu\n", pid, ts);
        printf("%d: sessionID = %s\n", pid, sessionID);
        printf("%d: bin_sha   = %s\n", pid, formula->bin_sha);
    }

    //////////////////////////////////////////////////////////////////////////

    struct stat st;

    size_t downloadDIRCapacity = uppmHomeDIRLength + 11U;
    char   downloadDIR[downloadDIRCapacity];

    ret = snprintf(downloadDIR, downloadDIRCapacity, "%s/downloads", uppmHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return UPPM_ERROR;
    }

    if (lstat(downloadDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(downloadDIR) != 0) {
                perror(downloadDIR);
                uppm_formula_free(formula);
                return UPPM_ERROR;
            }

            if (mkdir(downloadDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(downloadDIR);
                    uppm_formula_free(formula);
                    return UPPM_ERROR;
                }
            }
        }
    } else {
        if (mkdir(downloadDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(downloadDIR);
                uppm_formula_free(formula);
                return UPPM_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    char binFileNameExtension[21] = {0};

    ret = uppm_examine_filetype_from_url(formula->bin_url, binFileNameExtension, 20);

    if (ret != UPPM_OK) {
        uppm_formula_free(formula);
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////

    size_t binFileNameLength = strlen(binFileNameExtension) + 65U;
    char   binFileName[binFileNameLength];

    ret = snprintf(binFileName, binFileNameLength, "%s%s", formula->bin_sha, binFileNameExtension);

    if (ret < 0) {
        perror(NULL);
        return UPPM_ERROR;
    }

    size_t binFilePathLength = downloadDIRCapacity + binFileNameLength + 1U;
    char   binFilePath[binFilePathLength];

    ret = snprintf(binFilePath, binFilePathLength, "%s/%s", downloadDIR, binFileName);

    if (ret < 0) {
        perror(NULL);
        return UPPM_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    bool needFetch = true;

    if (lstat(binFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char actualSHA256SUM[65] = {0};

            if (sha256sum_of_file(actualSHA256SUM, binFilePath) != 0) {
                uppm_formula_free(formula);
                return UPPM_ERROR;
            }

            if (strcmp(actualSHA256SUM, formula->bin_sha) == 0) {
                needFetch = false;
            }
        }
    }

    if (needFetch) {
        char * tmpFileName = sessionID;

        size_t tmpFilePathCapacity = downloadDIRCapacity + 65U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s", downloadDIR, tmpFileName);

        if (ret < 0) {
            perror(NULL);
            return UPPM_ERROR;
        }

        ret = uppm_http_fetch_to_file(formula->bin_url, tmpFilePath, verbose, verbose);

        if (ret != UPPM_OK) {
            uppm_formula_free(formula);
            return ret;
        }

        char actualSHA256SUM[65] = {0};

        if (sha256sum_of_file(actualSHA256SUM, tmpFilePath) != 0) {
            uppm_formula_free(formula);
            return UPPM_ERROR;
        }

        if (strcmp(actualSHA256SUM, formula->bin_sha) == 0) {
            if (rename(tmpFilePath, binFilePath) == 0) {
                printf("%s\n", binFilePath);
            } else {
                perror(binFilePath);
                uppm_formula_free(formula);
                return UPPM_ERROR;
            }
        } else {
            fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->bin_sha, actualSHA256SUM);
            uppm_formula_free(formula);
            return UPPM_ERROR_SHA256_MISMATCH;
        }
    } else {
        fprintf(stderr, "%s already have been fetched.\n", binFilePath);
    }

    //////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRootDIRCapacity = uppmHomeDIRLength + 21U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed", uppmHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return UPPM_ERROR;
    }

    if (lstat(packageInstalledRootDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(packageInstalledRootDIR) != 0) {
                perror(packageInstalledRootDIR);
                uppm_formula_free(formula);
                return UPPM_ERROR;
            }

            if (mkdir(packageInstalledRootDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(packageInstalledRootDIR);
                    uppm_formula_free(formula);
                    return UPPM_ERROR;
                }
            }
        }
    } else {
        if (mkdir(packageInstalledRootDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(packageInstalledRootDIR);
                uppm_formula_free(formula);
                return UPPM_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRealDIRCapacity = packageInstalledRootDIRCapacity + 66U;
    char   packageInstalledRealDIR[packageInstalledRealDIRCapacity];

    ret = snprintf(packageInstalledRealDIR, packageInstalledRealDIRCapacity, "%s/%s", packageInstalledRootDIR, sessionID);

    if (ret < 0) {
        perror(NULL);
        return UPPM_ERROR;
    }

    if (lstat(packageInstalledRealDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = uppm_rm_r(packageInstalledRealDIR, verbose);

            if (ret != UPPM_OK) {
                uppm_formula_free(formula);
                return ret;
            }

            if (mkdir(packageInstalledRealDIR, S_IRWXU) != 0) {
                perror(packageInstalledRealDIR);
                uppm_formula_free(formula);
                return UPPM_ERROR;
            }
        } else {
            if (unlink(packageInstalledRealDIR) != 0) {
                perror(packageInstalledRealDIR);
                uppm_formula_free(formula);
                return UPPM_ERROR;
            }

            if (mkdir(packageInstalledRealDIR, S_IRWXU) != 0) {
                perror(packageInstalledRealDIR);
                uppm_formula_free(formula);
                return UPPM_ERROR;
            }
        }
    }

    if (mkdir(packageInstalledRealDIR, S_IRWXU) != 0) {
        perror(packageInstalledRealDIR);
        uppm_formula_free(formula);
        return UPPM_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    if (strcmp(binFileNameExtension, ".zip") == 0 ||
        strcmp(binFileNameExtension, ".tgz") == 0 ||
        strcmp(binFileNameExtension, ".txz") == 0 ||
        strcmp(binFileNameExtension, ".tlz") == 0 ||
        strcmp(binFileNameExtension, ".tbz2") == 0) {

        if (formula->unpackd == NULL) {
            ret = tar_extract(packageInstalledRealDIR, binFilePath, ARCHIVE_EXTRACT_TIME, verbose, 1);
        } else {
            size_t extractDIRCapacity = packageInstalledRealDIRCapacity + strlen(formula->unpackd) + 1U;
            char   extractDIR[extractDIRCapacity];

            ret = snprintf(extractDIR, extractDIRCapacity, "%s/%s", packageInstalledRealDIR, formula->unpackd);

            if (ret < 0) {
                perror(NULL);
                return UPPM_ERROR;
            }

            ret = tar_extract(extractDIR, binFilePath, ARCHIVE_EXTRACT_TIME, verbose, 1);
        }

        if (ret != 0) {
            uppm_formula_free(formula);
            return abs(ret) + UPPM_ERROR_ARCHIVE_BASE;
        }
    } else {
        size_t toFilePathCapacity = packageInstalledRealDIRCapacity + 66U;
        char   toFilePath[toFilePathCapacity];

        ret = snprintf(toFilePath, toFilePathCapacity, "%s/%s", packageInstalledRealDIR, sessionID);

        if (ret < 0) {
            perror(NULL);
            return UPPM_ERROR;
        }

        ret = uppm_copy_file(binFilePath, toFilePath);

        if (ret != UPPM_OK) {
            return ret;
        }
    }

    if (chdir(packageInstalledRealDIR) != 0) {
        perror(packageInstalledRealDIR);
        uppm_formula_free(formula);
        return UPPM_ERROR;
    }

    if (formula->install != NULL) {
        char * selfRealPath = self_realpath();

        if (selfRealPath == NULL) {
            perror(NULL);
            uppm_formula_free(formula);
            return UPPM_ERROR;
        }

        SysInfo sysinfo = {0};

        if (sysinfo_make(&sysinfo) != 0) {
            perror(NULL);
            uppm_formula_free(formula);
            free(selfRealPath);
            return UPPM_ERROR;
        }

        const char * libcName;

        switch(sysinfo.libc) {
            case 1:  libcName = (char*)"glibc"; break;
            case 2:  libcName = (char*)"musl";  break;
            default: libcName = (char*)"";
        }

        size_t packageInstalledLinkDIRCapacity = packageInstalledRootDIRCapacity + strlen(packageName) + 2U;
        char   packageInstalledLinkDIR[packageInstalledLinkDIRCapacity];

        ret = snprintf(packageInstalledLinkDIR, packageInstalledLinkDIRCapacity, "%s/%s", packageInstalledRootDIR, packageName);

        if (ret < 0) {
            perror(NULL);
            return UPPM_ERROR;
        }

        size_t shellCodeCapacity = strlen(formula->install) + 2048U;
        char   shellCode[shellCodeCapacity];

        ret = snprintf(shellCode, shellCodeCapacity,
                "set -ex\n\n"
                "NATIVE_OS_KIND='%s'\n"
                "NATIVE_OS_TYPE='%s'\n"
                "NATIVE_OS_NAME='%s'\n"
                "NATIVE_OS_VERS='%s'\n"
                "NATIVE_OS_LIBC='%s'\n"
                "NATIVE_OS_ARCH='%s'\n"
                "NATIVE_OS_NCPU='%u'\n\n"
                "UPPM_VERSION='%s'\n"
                "UPPM_VERSION_MAJOR='%d'\n"
                "UPPM_VERSION_MINOR='%d'\n"
                "UPPM_VERSION_PATCH='%d'\n"
                "UPPM_HOME='%s'\n\n"
                "UPPM_EXECUTABLE='%s'\n\n"
                "PKG_SUMMARY='%s'\n"
                "PKG_WEBPAGE='%s'\n"
                "PKG_VERSION='%s'\n"
                "PKG_BIN_URL='%s'\n"
                "PKG_BIN_SHA='%s'\n"
                "PKG_DEP_PKG='%s'\n"
                "PKG_BIN_FILENAME='%s'\n"
                "PKG_BIN_FILETYPE='%s'\n"
                "PKG_BIN_FILEPATH='%s'\n"
                "PKG_INSTALL_DIR='%s'\n\n"
                "for item in $PKG_DEP_PKG\n"
                "do\n"
                "if [ -d \"$UPPM_HOME/installed/$item/bin\" ] ; then\n"
                "PATH=\"$UPPM_HOME/installed/$item/bin:$PATH\"\n"
                "fi\n"
                "done\n\n"
                "pwd\n"
                "%s",
                sysinfo.kind,
                sysinfo.type,
                sysinfo.name,
                sysinfo.vers,
                libcName,
                sysinfo.arch,
                sysinfo.ncpu,
                UPPM_VERSION,
                UPPM_VERSION_MAJOR,
                UPPM_VERSION_MINOR,
                UPPM_VERSION_PATCH,
                uppmHomeDIR,
                selfRealPath,
                formula->summary == NULL ? "" : formula->summary,
                formula->webpage == NULL ? "" : formula->webpage,
                formula->version == NULL ? "" : formula->version,
                formula->bin_url == NULL ? "" : formula->bin_url,
                formula->bin_sha == NULL ? "" : formula->bin_sha,
                formula->dep_pkg == NULL ? "" : formula->dep_pkg,
                binFileName,
                binFileNameExtension,
                binFilePath,
                packageInstalledLinkDIR,
                formula->install);

        if (ret < 0) {
            perror(NULL);
            sysinfo_free(&sysinfo);
            free(selfRealPath);
            return UPPM_ERROR;
        }

        sysinfo_free(&sysinfo);
        free(selfRealPath);

        printf("run shell code:\n%s\n", shellCode);

        int childProcessExitStatusCode = system(shellCode);

        if (childProcessExitStatusCode == -1) {
            perror(NULL);
            uppm_formula_free(formula);
            return UPPM_ERROR;
        }

        if (childProcessExitStatusCode != 0) {
            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running shell code exit with status code: %d\n", WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running shell code killed by signal: %d\n", WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running shell code stopped by signal: %d\n", WSTOPSIG(childProcessExitStatusCode));
            }

            uppm_formula_free(formula);
            return UPPM_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////

    size_t packageInstalledMetaInfoDIRCapacity = packageInstalledRealDIRCapacity + 6U;
    char   packageInstalledMetaInfoDIR[packageInstalledMetaInfoDIRCapacity];

    ret = snprintf(packageInstalledMetaInfoDIR, packageInstalledMetaInfoDIRCapacity, "%s/.uppm", packageInstalledRealDIR);

    if (ret < 0) {
        perror(NULL);
        return UPPM_ERROR;
    }

    if (mkdir(packageInstalledMetaInfoDIR, S_IRWXU) != 0) {
        perror(packageInstalledMetaInfoDIR);
        uppm_formula_free(formula);
        return UPPM_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    ret = uppm_record_installed_files(packageInstalledRealDIR);

    if (ret != UPPM_OK) {
        uppm_formula_free(formula);
        return UPPM_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    size_t receiptFilePathCapacity = packageInstalledMetaInfoDIRCapacity + 12U;
    char   receiptFilePath[receiptFilePathCapacity];

    ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/receipt.yml", packageInstalledMetaInfoDIR);

    if (ret < 0) {
        perror(NULL);
        return UPPM_ERROR;
    }

    int receiptFD = open(receiptFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (receiptFD == -1) {
        perror(receiptFilePath);
        uppm_formula_free(formula);
        return UPPM_ERROR;
    }

    int formulaFD = open(formula->path, O_RDONLY);

    if (formulaFD == -1) {
        perror(formula->path);
        uppm_formula_free(formula);
        close(receiptFD);
        return UPPM_ERROR;
    }

    uppm_formula_free(formula);
    formula = NULL;

    dprintf(receiptFD, "pkgname: %s\n", packageName);

    char buf[1024];

    for (;;) {
        ssize_t readSize = read(formulaFD, buf, 1024);

        if (readSize == -1) {
            perror(NULL);
            close(formulaFD);
            close(receiptFD);
            return UPPM_ERROR;
        }

        if (readSize == 0) {
            close(formulaFD);
            break;
        }

        ssize_t writeSize = write(receiptFD, buf, readSize);

        if (writeSize == -1) {
            perror(receiptFilePath);
            close(formulaFD);
            close(receiptFD);
            return UPPM_ERROR;
        }

        if (writeSize != readSize) {
            fprintf(stderr, "not fully written to %s\n", receiptFilePath);
            close(formulaFD);
            close(receiptFD);
            return UPPM_ERROR;
        }
    }

    dprintf(receiptFD, "\nsignature: %s\ntimestamp: %lu\n", UPPM_VERSION, ts);

    close(receiptFD);

    if (chdir (packageInstalledRootDIR) != 0) {
        perror(packageInstalledRootDIR);
        return UPPM_ERROR;
    }

    for (;;) {
        if (symlink(sessionID, packageName) == 0) {
            fprintf(stderr, "%s package was successfully installed.\n", packageName);
            return UPPM_OK;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = uppm_rm_r(packageName, verbose);

                        if (ret != UPPM_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return UPPM_ERROR;
                        }
                    }
                }
            } else {
                perror(packageInstalledRealDIR);
                return UPPM_ERROR;
            }
        }
    }
}

int uppm_install_the_given_packages(const char * packageNames[], size_t size) {
    for (size_t i = 0; i < size; i++) {
        const char * packageName = packageNames[i];

        UPPMFormula * formula = NULL;

        int ret = uppm_formula_lookup(packageName, &formula);

        if (ret != UPPM_OK) {
            return ret;
        }
    }
    return 0;
}

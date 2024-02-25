#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/sysinfo.h"
#include "core/base16.h"
#include "core/http.h"
#include "core/self.h"
#include "core/exe.h"
#include "core/tar.h"
#include "core/log.h"

#include "sha256sum.h"
#include "ndk-pkg.h"

typedef struct {
    const char * name;
    const char * value;
} KV;

typedef struct {
    const char * name;
    bool         value;
} KB;

typedef struct {
    const char * name;
    size_t       value;
} KU;


static int run_cmd(char * cmd, int output2FD) {
    fprintf(stderr, "%s==>%s %s%s%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, cmd, COLOR_OFF);

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (pid == 0) {
        if (output2FD > STDERR_FILENO) {
            if (dup2(output2FD, STDOUT_FILENO) < 0) {
                perror(NULL);
                exit(253);
            }

            if (dup2(output2FD, STDERR_FILENO) < 0) {
                perror(NULL);
                exit(254);
            }
        }

        ////////////////////////////////////////

        size_t argc = 0U;
        char*  argv[30] = {0};

        char * arg = strtok(cmd, " ");

        while (arg != NULL) {
            argv[argc] = arg;
            argc++;
            arg = strtok(NULL, " ");
        }

        ////////////////////////////////////////

        bool isPath = false;

        const char * p = argv[0];

        for (;;) {
            if (p[0] == '\0') {
                break;
            }

            if (p[0] == '/') {
                isPath = true;
                break;
            }

            p++;
        }

        ////////////////////////////////////////

        if (isPath) {
            execv (argv[0], argv);
        } else {
            execvp(argv[0], argv);
        }

        perror(argv[0]);
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return NDKPKG_OK;
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(childProcessExitStatusCode));
        }

        return NDKPKG_ERROR;
    }
}

static int get_the_first_n_bytes_of_a_file(const char * fp, unsigned int n, char buf[]) {
    int fd = open(fp, O_RDONLY);

    if (fd == -1) {
        perror(fp);
        return NDKPKG_ERROR;
    }

    ssize_t readSize = read(fd, buf, n);

    if (readSize == -1) {
        perror(fp);
        close(fd);
        return NDKPKG_ERROR;
    } else {
        close(fd);
        return NDKPKG_OK;
    }
}

static int write_to_file(const char * fp, const char * str) {
    int fd = open(fp, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if (fd == -1) {
        perror(fp);
        return NDKPKG_ERROR;
    }

    size_t n = strlen(str);

    ssize_t writtenSize = write(fd, str, n);

    if (writtenSize == -1) {
        perror(fp);
        close(fd);
        return NDKPKG_ERROR;
    }

    if (writtenSize == (ssize_t)n) {
        return NDKPKG_OK;
    } else {
        fprintf(stderr, "file not fully written: %s\n", fp);
        return NDKPKG_ERROR;
    }
}

//static void export_p() {
    //for (int i = 0; ; i++) {
    //    const char * p = __environ[i];

    //    if (p == NULL) {
    //        break;
    //    }

    //    puts(p);
    //}
//}

static int download_via_http(const char * url, const char * uri, const char * expectedSHA256SUM, const char * downloadDIR, size_t downloadDIRLength, const char * unpackDIR, size_t unpackDIRLength, const bool verbose) {
    char fileNameExtension[21] = {0};

    int ret = ndkpkg_examine_filetype_from_url(url, fileNameExtension, 20);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    size_t fileNameCapacity = strlen(expectedSHA256SUM) + strlen(fileNameExtension) + 1U;
    char   fileName[fileNameCapacity];

    ret = snprintf(fileName, fileNameCapacity, "%s%s", expectedSHA256SUM, fileNameExtension);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t filePathCapacity = downloadDIRLength + fileNameCapacity + 1U;
    char   filePath[filePathCapacity];

    ret = snprintf(filePath, filePathCapacity, "%s/%s", downloadDIR, fileName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    bool needFetch = true;

    struct stat st;

    if (stat(filePath, &st) == 0 && S_ISREG(st.st_mode)) {
        char actualSHA256SUM[65] = {0};

        ret = sha256sum_of_file(actualSHA256SUM, filePath);

        if (ret != 0) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            needFetch = false;

            if (verbose) {
                fprintf(stderr, "%s already have been fetched.\n", filePath);
            }
        }
    }

    if (needFetch) {
        size_t tmpStrCapacity = strlen(url) + 30U;
        char   tmpStr[tmpStrCapacity];

        ret = snprintf(tmpStr, tmpStrCapacity, "%s|%ld|%d", url, time(NULL), getpid());

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        char tmpFileName[65] = {0};

        ret = sha256sum_of_string(tmpFileName, tmpStr);

        if (ret != 0) {
            return NDKPKG_ERROR;
        }

        size_t tmpFilePathCapacity = downloadDIRLength + 65U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s", downloadDIR, tmpFileName);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = ndkpkg_http_fetch_to_file(url, tmpFilePath, verbose, verbose);

        if (ret != NDKPKG_OK) {
            if (uri != NULL) {
                ret = ndkpkg_http_fetch_to_file(uri, tmpFilePath, verbose, verbose);
            }
        }

        if (ret != NDKPKG_OK) {
            return ret;
        }

        char actualSHA256SUM[65] = {0};

        ret = sha256sum_of_file(actualSHA256SUM, tmpFilePath);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            if (rename(tmpFilePath, filePath) == 0) {
                printf("%s\n", filePath);
            } else {
                perror(filePath);
                return NDKPKG_ERROR;
            }
        } else {
            fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", expectedSHA256SUM, actualSHA256SUM);
            return NDKPKG_ERROR_SHA256_MISMATCH;
        }
    }

    if (strcmp(fileNameExtension, ".zip") == 0 ||
        strcmp(fileNameExtension, ".tgz") == 0 ||
        strcmp(fileNameExtension, ".txz") == 0 ||
        strcmp(fileNameExtension, ".tlz") == 0 ||
        strcmp(fileNameExtension, ".tbz2") == 0) {

        ret = tar_extract(unpackDIR, filePath, ARCHIVE_EXTRACT_TIME, verbose, 1);

        if (ret != 0) {
            return abs(ret) + NDKPKG_ERROR_ARCHIVE_BASE;
        }
    } else {
        size_t toFilePathCapacity = unpackDIRLength + fileNameCapacity + 1U;
        char   toFilePath[toFilePathCapacity];

        ret = snprintf(toFilePath, toFilePathCapacity, "%s/%s", unpackDIR, fileName);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = ndkpkg_copy_file(filePath, toFilePath);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    return NDKPKG_OK;
}

static int setup_rust_toolchain(const NDKPKGInstallOptions * installOptions, const char * sessionDIR, const size_t sessionDIRLength) {
    const char * cargoHomeDIR = getenv("CARGO_HOME");

    if (cargoHomeDIR == NULL || cargoHomeDIR[0] == '\0') {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL || userHomeDIR[0] == '\0') {
            return NDKPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t defaultCargoHomeDIRCapacity = strlen(userHomeDIR) + 8U;
        char   defaultCargoHomeDIR[defaultCargoHomeDIRCapacity];

        int ret = snprintf(defaultCargoHomeDIR, defaultCargoHomeDIRCapacity, "%s/.cargo", userHomeDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (setenv("CARGO_HOME", defaultCargoHomeDIR, 1) != 0) {
            perror("CARGO_HOME");
            return NDKPKG_ERROR;
        }

        cargoHomeDIR = getenv("CARGO_HOME");
    }

    size_t cargoHomeDIRLength = strlen(cargoHomeDIR);

    size_t rustupCommandPathCapacity = cargoHomeDIRLength + 12U;
    char   rustupCommandPath[rustupCommandPathCapacity];

    int ret = snprintf(rustupCommandPath, rustupCommandPathCapacity, "%s/bin/rustup", cargoHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t cargoCommandPathCapacity = cargoHomeDIRLength + 11U;
    char   cargoCommandPath[cargoCommandPathCapacity];

    ret = snprintf(cargoCommandPath, cargoCommandPathCapacity, "%s/bin/cargo", cargoHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    const bool  cargoExist = (stat( cargoCommandPath, &st) == 0) && S_ISREG(st.st_mode);
    const bool rustupExist = (stat(rustupCommandPath, &st) == 0) && S_ISREG(st.st_mode);

    if (!(cargoExist && rustupExist)) {
        LOG_INFO("rustup and cargo commands are required, but they are not found on this machine, ndkpkg will install them via running shell script.");

        size_t rustupInitScriptFilePathCapacity = sessionDIRLength + 16U;
        char   rustupInitScriptFilePath[rustupInitScriptFilePathCapacity];

        ret = snprintf(rustupInitScriptFilePath, rustupInitScriptFilePathCapacity, "%s/rustup-init.sh", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = ndkpkg_http_fetch_to_file("https://sh.rustup.rs", rustupInitScriptFilePath, installOptions->verbose_net, installOptions->verbose_net);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        size_t cmdCapacity = rustupInitScriptFilePathCapacity + 10U;
        char   cmd[cmdCapacity];

        ret = snprintf(cmd, cmdCapacity, "bash %s -y", rustupInitScriptFilePath);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(cmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL || PATH[0] == '\0') {
        return NDKPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t newPATHCapacity = cargoHomeDIRLength + strlen(PATH) + 6U;
    char   newPATH[newPATHCapacity];

    ret = snprintf(newPATH, newPATHCapacity, "%s/bin:%s", cargoHomeDIR, PATH);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (setenv("PATH", newPATH, 1) != 0) {
        perror("PATH");
        return NDKPKG_ERROR;
    }

    return NDKPKG_OK;
}


typedef int (*setenv_fn)(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity);

static int setenv_CPPFLAGS(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t includeDIRCapacity = packageInstalledDIRCapacity + 9U;
    char   includeDIR[includeDIRCapacity];

    int ret = snprintf(includeDIR, includeDIRCapacity, "%s/include", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(includeDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const CPPFLAGS = getenv("CPPFLAGS");

        if (CPPFLAGS == NULL || CPPFLAGS[0] == '\0') {
            size_t newCPPFLAGSCapacity = includeDIRCapacity + 3U;
            char   newCPPFLAGS[newCPPFLAGSCapacity];

            ret = snprintf(newCPPFLAGS, newCPPFLAGSCapacity, "-I%s", includeDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("CPPFLAGS", newCPPFLAGS, 1) != 0) {
                perror("CPPFLAGS");
                return NDKPKG_ERROR;
            }
        } else {
            size_t newCPPFLAGSCapacity = includeDIRCapacity + strlen(CPPFLAGS) + 4U;
            char   newCPPFLAGS[newCPPFLAGSCapacity];

            ret = snprintf(newCPPFLAGS, newCPPFLAGSCapacity, "-I%s %s", includeDIR, CPPFLAGS);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("CPPFLAGS", newCPPFLAGS, 1) != 0) {
                perror("CPPFLAGS");
                return NDKPKG_ERROR;
            }
        }
    }

    return NDKPKG_OK;
}

static int setenv_LDFLAGS(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t libDIRCapacity = packageInstalledDIRCapacity + 5U;
    char   libDIR[libDIRCapacity];

    int ret = snprintf(libDIR, libDIRCapacity, "%s/lib", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(libDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const LDFLAGS = getenv("LDFLAGS");

        if (LDFLAGS == NULL || LDFLAGS[0] == '\0') {
            size_t newLDFLAGSCapacity = libDIRCapacity + 3U;
            char   newLDFLAGS[newLDFLAGSCapacity];

            ret = snprintf(newLDFLAGS, newLDFLAGSCapacity, "-L%s", libDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
                perror("LDFLAGS");
                return NDKPKG_ERROR;
            }
        } else {
            size_t newLDFLAGSCapacity = (libDIRCapacity << 1U) + strlen(LDFLAGS) + 15U;
            char   newLDFLAGS[newLDFLAGSCapacity];

            ret = snprintf(newLDFLAGS, newLDFLAGSCapacity, "%s -L%s -Wl,-rpath,%s", LDFLAGS, libDIR, libDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
                perror("LDFLAGS");
                return NDKPKG_ERROR;
            }
        }
    }

    return NDKPKG_OK;
}

static int setenv_PKG_CONFIG_PATH(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    struct stat st;

    const char * a[2] = { "lib", "share" };

    for (int i = 0; i < 2; i++) {
        size_t pkgconfigDIRCapacity = packageInstalledDIRCapacity + 20U;
        char   pkgconfigDIR[pkgconfigDIRCapacity];

        int ret = snprintf(pkgconfigDIR, pkgconfigDIRCapacity, "%s/%s/pkgconfig", packageInstalledDIR, a[i]);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (stat(pkgconfigDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
            const char * const PKG_CONFIG_PATH = getenv("PKG_CONFIG_PATH");

            if (PKG_CONFIG_PATH == NULL || PKG_CONFIG_PATH[0] == '\0') {
                if (setenv("PKG_CONFIG_PATH", pkgconfigDIR, 1) != 0) {
                    perror("PKG_CONFIG_PATH");
                    return NDKPKG_ERROR;
                }
            } else {
                size_t newPKG_CONFIG_PATHCapacity = pkgconfigDIRCapacity + strlen(PKG_CONFIG_PATH) + 2U;
                char   newPKG_CONFIG_PATH[newPKG_CONFIG_PATHCapacity];

                ret = snprintf(newPKG_CONFIG_PATH, newPKG_CONFIG_PATHCapacity, "%s:%s", PKG_CONFIG_PATH, pkgconfigDIR);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if (setenv("PKG_CONFIG_PATH", newPKG_CONFIG_PATH, 1) != 0) {
                    perror("PKG_CONFIG_PATH");
                    return NDKPKG_ERROR;
                }
            }
        }
    }

    return NDKPKG_OK;
}

static int setenv_PATH(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    struct stat st;

    size_t binDIRCapacity = packageInstalledDIRCapacity + 5U;
    char   binDIR[binDIRCapacity];

    int ret = snprintf(binDIR, binDIRCapacity, "%s/bin", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t sbinDIRCapacity = packageInstalledDIRCapacity + 6U;
    char   sbinDIR[sbinDIRCapacity];

    ret = snprintf(sbinDIR, sbinDIRCapacity, "%s/sbin", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    const bool  binDIRExists = stat( binDIR, &st) == 0 && S_ISDIR(st.st_mode);
    const bool sbinDIRExists = stat(sbinDIR, &st) == 0 && S_ISDIR(st.st_mode);

    if (binDIRExists || sbinDIRExists) {
        const char * const PATH = getenv("PATH");

        if (PATH == NULL || PATH[0] == '\0') {
            return NDKPKG_ERROR_ENV_PATH_NOT_SET;
        }

        if (binDIRExists && sbinDIRExists) {
            size_t newPATHLength = binDIRCapacity + sbinDIRCapacity + strlen(PATH) + 3U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s:%s", binDIR, sbinDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return NDKPKG_ERROR;
            }
        } else if (binDIRExists) {
            size_t newPATHLength = binDIRCapacity + strlen(PATH) + 2U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s", binDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return NDKPKG_ERROR;
            }
        } else if (sbinDIRExists) {
            size_t newPATHLength = sbinDIRCapacity + strlen(PATH) + 2U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s", sbinDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return NDKPKG_ERROR;
            }
        }
    }

    return NDKPKG_OK;
}

static int setenv_ACLOCAL_PATH(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t shareDIRCapacity = packageInstalledDIRCapacity + 7U;
    char   shareDIR[shareDIRCapacity];

    int ret = snprintf(shareDIR, shareDIRCapacity, "%s/share", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // https://www.gnu.org/software/automake/manual/html_node/Macro-Search-Path.html

    size_t aclocalDIRCapacity = shareDIRCapacity + 9U;
    char   aclocalDIR[aclocalDIRCapacity];

    ret = snprintf(aclocalDIR, aclocalDIRCapacity, "%s/aclocal", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(aclocalDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const ACLOCAL_PATH = getenv("ACLOCAL_PATH");

        if (ACLOCAL_PATH == NULL || ACLOCAL_PATH[0] == '\0') {
            if (setenv("ACLOCAL_PATH", aclocalDIR, 1) != 0) {
                perror("ACLOCAL_PATH");
                return NDKPKG_ERROR;
            }
        } else {
            size_t newACLOCAL_PATHLength = aclocalDIRCapacity + strlen(ACLOCAL_PATH) + 2U;
            char   newACLOCAL_PATH[newACLOCAL_PATHLength];

            ret = snprintf(newACLOCAL_PATH, newACLOCAL_PATHLength, "%s:%s", aclocalDIR, ACLOCAL_PATH);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("ACLOCAL_PATH", newACLOCAL_PATH, 1) != 0) {
                perror("ACLOCAL_PATH");
                return NDKPKG_ERROR;
            }
        }
    }

    return NDKPKG_OK;
}

static int setenv_XDG_DATA_DIRS(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t shareDIRCapacity = packageInstalledDIRCapacity + 7U;
    char   shareDIR[shareDIRCapacity];

    int ret = snprintf(shareDIR, shareDIRCapacity, "%s/share", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
    // https://gi.readthedocs.io/en/latest/tools/g-ir-scanner.html#environment-variables
    // https://help.gnome.org/admin//system-admin-guide/2.32/mimetypes-database.html.en

    size_t girSearchDIRCapacity = shareDIRCapacity + 9U;
    char   girSearchDIR[girSearchDIRCapacity];

    ret = snprintf(girSearchDIR, girSearchDIRCapacity, "%s/gir-1.0", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t mimeSearchDIRLength = shareDIRCapacity + 6U;
    char   mimeSearchDIR[mimeSearchDIRLength];

    ret = snprintf(mimeSearchDIR, mimeSearchDIRLength, "%s/mime", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if ((stat(girSearchDIR, &st) == 0 && S_ISDIR(st.st_mode)) || (stat(mimeSearchDIR, &st) == 0 && S_ISDIR(st.st_mode))) {
        const char * const XDG_DATA_DIRS = getenv("XDG_DATA_DIRS");

        if (XDG_DATA_DIRS == NULL || XDG_DATA_DIRS[0] == '\0') {
            if (setenv("XDG_DATA_DIRS", shareDIR, 1) != 0) {
                perror("XDG_DATA_DIRS");
                return NDKPKG_ERROR;
            }
        } else {
            size_t newXDG_DATA_DIRSLength = shareDIRCapacity + strlen(XDG_DATA_DIRS) + 2U;
            char   newXDG_DATA_DIRS[newXDG_DATA_DIRSLength];

            ret = snprintf(newXDG_DATA_DIRS, newXDG_DATA_DIRSLength, "%s:%s", shareDIR, XDG_DATA_DIRS);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("XDG_DATA_DIRS", newXDG_DATA_DIRS, 1) != 0) {
                perror("XDG_DATA_DIRS");
                return NDKPKG_ERROR;
            }
        }
    }

    return NDKPKG_OK;
}


#define BUILD_SYSTEM_TYPE_CMAKE 1
#define BUILD_SYSTEM_TYPE_CONFIGURE 2

#define NATIVE_PACKAGE_ID_ZLIB     1
#define NATIVE_PACKAGE_ID_LIBBZ2   2
#define NATIVE_PACKAGE_ID_LIBLZMA  3

#define NATIVE_PACKAGE_ID_PERL     4
#define NATIVE_PACKAGE_ID_OPENSSL  5
#define NATIVE_PACKAGE_ID_TEXINFO  6
#define NATIVE_PACKAGE_ID_HELP2MAN 7
#define NATIVE_PACKAGE_ID_LIBTOOL  8
#define NATIVE_PACKAGE_ID_AUTOCONF 9
#define NATIVE_PACKAGE_ID_AUTOMAKE 10

#define NATIVE_PACKAGE_ID_INTLTOOL 11
#define NATIVE_PACKAGE_ID_EXPAT    12
#define NATIVE_PACKAGE_ID_GDBM     13
#define NATIVE_PACKAGE_ID_SQLITE3  14
#define NATIVE_PACKAGE_ID_LIBFFI   15
#define NATIVE_PACKAGE_ID_LIBYAML  16
#define NATIVE_PACKAGE_ID_PYTHON3  17
#define NATIVE_PACKAGE_ID_RUBY     18
#define NATIVE_PACKAGE_ID_PERL_XML_PARSER 19

#define NATIVE_PACKAGE_ID_LIBPCRE2 20
#define NATIVE_PACKAGE_ID_SWIG     21

typedef struct {
    const char * name;

    const char * srcUrl;
    const char * srcUri;
    const char * srcSha;

    const char * buildConfigureArgs;

    int buildSystemType;

    int depPackageIDArray[10];
} NativePackage;

static int getNativePackageInfoByID(int packageID, NativePackage * nativePackage) {
    switch (packageID) {
        case NATIVE_PACKAGE_ID_EXPAT:
            nativePackage->name = "expat";
            nativePackage->srcUrl = "https://github.com/libexpat/libexpat/releases/download/R_2_5_0/expat-2.5.0.tar.xz";
            nativePackage->srcSha = "ef2420f0232c087801abf705e89ae65f6257df6b7931d37846a193ef2e8cdcbe";
            nativePackage->buildConfigureArgs = "-DEXPAT_BUILD_DOCS=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_FUZZERS=OFF -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TOOLS=OFF";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_ZLIB:
            nativePackage->name = "zlib";
            nativePackage->srcUrl = "https://zlib.net/fossils/zlib-1.3.tar.gz";
            nativePackage->srcSha = "ff0ba4c292013dbc27530b3a81e1f9a813cd39de01ca5e0f8bf355702efa593e";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_LIBBZ2:
            nativePackage->name = "libbz2";
            nativePackage->srcUrl = "https://github.com/leleliu008/bzip2/archive/refs/tags/1.0.8.tar.gz";
            nativePackage->srcSha = "fb36d769189faaf841390fae88639fb02c79b87b0691a340fbbfc32b4f82b789";
            nativePackage->buildConfigureArgs = "-DINSTALL_EXECUTABLES=OFF -DINSTALL_LIBRARIES=ON -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_PERL:
            nativePackage->name = "perl";
            nativePackage->srcUrl = "https://www.cpan.org/src/5.0/perl-5.38.0.tar.xz";
            nativePackage->srcUri = "https://cpan.metacpan.org/authors/id/R/RJ/RJBS/perl-5.38.0.tar.xz";
            nativePackage->srcSha = "eca551caec3bc549a4e590c0015003790bdd1a604ffe19cc78ee631d51f7072e";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_OPENSSL:
            nativePackage->name = "openssl";
            nativePackage->srcUrl = "https://www.openssl.org/source/openssl-3.1.1.tar.gz";
            nativePackage->srcSha = "b3aa61334233b852b63ddb048df181177c2c659eb9d4376008118f9c08d07674";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_TEXINFO:
            nativePackage->name = "texinfo";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/texinfo/texinfo-7.1.tar.xz";
            nativePackage->srcSha = "deeec9f19f159e046fdf8ad22231981806dac332cc372f1c763504ad82b30953";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->buildConfigureArgs = "--with-included-regex --enable-threads=posix --disable-nls";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_HELP2MAN:
            nativePackage->name = "help2man";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/help2man/help2man-1.49.3.tar.xz";
            nativePackage->srcSha = "4d7e4fdef2eca6afe07a2682151cea78781e0a4e8f9622142d9f70c083a2fd4f";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_INTLTOOL:
            nativePackage->name = "intltool";
            nativePackage->srcUrl = "https://launchpad.net/intltool/trunk/0.51.0/+download/intltool-0.51.0.tar.gz";
            nativePackage->srcSha = "67c74d94196b153b774ab9f89b2fa6c6ba79352407037c8c14d5aeb334e959cd";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL_XML_PARSER;
            nativePackage->buildConfigureArgs = "";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBTOOL:
            nativePackage->name = "libtool";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/libtool/libtool-2.4.7.tar.xz";
            nativePackage->srcSha = "4f7f217f057ce655ff22559ad221a0fd8ef84ad1fc5fcb6990cecc333aa1635d";
            nativePackage->buildConfigureArgs = "--enable-ltdl-install";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOCONF:
            nativePackage->name = "autoconf";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/autoconf/autoconf-2.71.tar.gz";
            nativePackage->srcSha = "431075ad0bf529ef13cb41e9042c542381103e80015686222b8a9d4abef42a1c";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOMAKE:
            nativePackage->name = "automake";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/automake/automake-1.16.5.tar.xz";
            nativePackage->srcSha = "f01d58cd6d9d77fbdca9eb4bbd5ead1988228fdb73d6f7a201f5f8d6b118b469";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_AUTOCONF;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBLZMA:
            nativePackage->name = "liblzma";
            nativePackage->srcUrl = "https://github.com/tukaani-project/xz/releases/download/v5.4.4/xz-5.4.4.tar.gz";
            nativePackage->srcSha = "aae39544e254cfd27e942d35a048d592959bd7a79f9a624afb0498bb5613bdf8";
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --disable-nls --enable-largefile --disable-xz --disable-xzdec --disable-lzmadec --disable-lzmainfo --disable-lzma-links --disable-scripts --disable-doc";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
            ;;
        case NATIVE_PACKAGE_ID_GDBM:
            nativePackage->name = "gdbm";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/gdbm/gdbm-1.23.tar.gz";
            nativePackage->srcSha = "74b1081d21fff13ae4bd7c16e5d6e504a4c26f7cde1dca0d963a484174bbcacd";
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --disable-nls --enable-largefile --enable-libgdbm-compat --without-readline";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_SQLITE3:
            nativePackage->name = "sqlite3";
            nativePackage->srcUrl = "https://www.sqlite.org/2023/sqlite-autoconf-3440000.tar.gz";
            nativePackage->srcSha = "b9cd386e7cd22af6e0d2a0f06d0404951e1bef109e42ea06cc0450e10cd15550";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_ZLIB;
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --enable-largefile --disable-editline --disable-readline";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBFFI:
            nativePackage->name = "libffi";
            nativePackage->srcUrl = "https://github.com/libffi/libffi/releases/download/v3.4.4/libffi-3.4.4.tar.gz";
            nativePackage->srcSha = "d66c56ad259a82cf2a9dfc408b32bf5da52371500b84745f7fb8b645712df676";
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --disable-docs --disable-symvers";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBYAML:
            nativePackage->name = "libyaml";
            nativePackage->srcUrl = "https://github.com/yaml/libyaml/releases/download/0.2.5/yaml-0.2.5.tar.gz";
            nativePackage->srcSha = "c642ae9b75fee120b2d96c712538bd2cf283228d2337df2cf2988e3c02678ef4";
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --enable-largefile";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_RUBY:
            nativePackage->name = "ruby";
            nativePackage->srcUrl = "https://cache.ruby-lang.org/pub/ruby/3.2/ruby-3.2.2.tar.gz";
            nativePackage->srcSha = "96c57558871a6748de5bc9f274e93f4b5aad06cd8f37befa0e8d94e7b8a423bc";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_ZLIB;
            nativePackage->depPackageIDArray[1] = NATIVE_PACKAGE_ID_LIBFFI;
            nativePackage->depPackageIDArray[2] = NATIVE_PACKAGE_ID_LIBYAML;
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --enable-shared --disable-docs";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_PYTHON3:
            nativePackage->name = "python3";
            nativePackage->srcUrl = "https://www.python.org/ftp/python/3.11.6/Python-3.11.6.tgz";
            nativePackage->srcSha = "c049bf317e877cbf9fce8c3af902436774ecef5249a29d10984ca3a37f7f4736";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_ZLIB;
            nativePackage->depPackageIDArray[1] = NATIVE_PACKAGE_ID_LIBBZ2;
            nativePackage->depPackageIDArray[2] = NATIVE_PACKAGE_ID_LIBLZMA;
            nativePackage->depPackageIDArray[3] = NATIVE_PACKAGE_ID_LIBFFI;
            nativePackage->depPackageIDArray[4] = NATIVE_PACKAGE_ID_GDBM;
            nativePackage->depPackageIDArray[5] = NATIVE_PACKAGE_ID_EXPAT;
            nativePackage->depPackageIDArray[6] = NATIVE_PACKAGE_ID_SQLITE3;
            nativePackage->depPackageIDArray[7] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->depPackageIDArray[8] = NATIVE_PACKAGE_ID_OPENSSL;
            nativePackage->buildConfigureArgs = "--with-system-expat --with-system-ffi --with-ensurepip=yes --with-lto --enable-ipv6 --enable-shared --enable-largefile --disable-option-checking --disable-nls --disable-debug --disable-loadable-sqlite-extensions --disable-profiling";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_PERL_XML_PARSER:
            nativePackage->name = "perl-XML-Parser";
            nativePackage->srcUrl = "https://cpan.metacpan.org/authors/id/T/TO/TODDR/XML-Parser-2.46.tar.gz";
            nativePackage->srcSha = "d331332491c51cccfb4cb94ffc44f9cd73378e618498d4a37df9e043661c515d";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->depPackageIDArray[1] = NATIVE_PACKAGE_ID_EXPAT;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBPCRE2:
            nativePackage->name = "libpcre2";
            nativePackage->srcUrl = "https://github.com/PCRE2Project/pcre2/releases/download/pcre2-10.42/pcre2-10.42.tar.bz2";
            nativePackage->srcSha = "8d36cd8cb6ea2a4c2bb358ff6411b0c788633a2a45dabbf1aeb4b701d1b5e840";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            nativePackage->buildConfigureArgs = "-DCMAKE_C_STANDARD=99 -DCMAKE_C_STANDARD_REQUIRED=ON -DPCRE2_BUILD_PCRE2_8=ON -DPCRE2_BUILD_PCRE2_16=ON -DPCRE2_BUILD_PCRE2_32=ON -DPCRE2_BUILD_PCRE2GREP=OFF -DPCRE2_BUILD_TESTS=OFF -DPCRE2_DEBUG=OFF -DPCRE2_SUPPORT_VALGRIND=OFF -DPCRE2_SUPPORT_UNICODE=ON";
            break;
        case NATIVE_PACKAGE_ID_SWIG:
            nativePackage->name = "swig";
            nativePackage->srcUrl = "https://downloads.sourceforge.net/project/swig/swig/swig-4.1.1/swig-4.1.1.tar.gz";
            nativePackage->srcSha = "2af08aced8fcd65cdb5cc62426768914bedc735b1c250325203716f78e39ac9b";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_LIBPCRE2;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            nativePackage->buildConfigureArgs = "--enable-ccache --enable-cpp11-testing --with-popen --with-pcre --without-boost --without-android --without-java --without-javascript --without-python --without-perl5 --without-ruby --without-php --without-tcl --without-guile --without-octave --without-scilab --without-ocaml --without-mzscheme --without-csharp --without-lua --without-r --without-d --without-go";
            break;
        default:
            fprintf(stderr, "unknown native package id: %d\n", packageID);
            return NDKPKG_ERROR;
    }

    return NDKPKG_OK;
}

typedef struct {
    const char * name;
    const char * libs;
    const char * env1;
    const char * env2;
} XX;

static int install_native_package(
        int nativePackageID,
        const char * downloadsDIR,
        const size_t downloadsDIRLength,
        const char * packageWorkingTopDIR,
        const size_t packageWorkingTopDIRCapacity,
        const char * nativePackageInstalledRootDIR,
        const size_t nativePackageInstalledRootDIRCapacity,
        const size_t njobs,
        const NDKPKGInstallOptions * installOptions) {
    NativePackage nativePackage = {0};

    int ret = getNativePackageInfoByID(nativePackageID, &nativePackage);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 10; i++) {
        if (nativePackage.depPackageIDArray[i] == 0) {
            break;
        }

        ret = install_native_package(nativePackage.depPackageIDArray[i], downloadsDIR, downloadsDIRLength, packageWorkingTopDIR, packageWorkingTopDIRCapacity, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, njobs, installOptions);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * packageName = nativePackage.name;
    const char * srcUrl = nativePackage.srcUrl;
    const char * srcUri = nativePackage.srcUri;
    const char * srcSha = nativePackage.srcSha;
    const char * buildConfigureArgs = nativePackage.buildConfigureArgs;
    int          buildSystemType = nativePackage.buildSystemType;

    if (buildConfigureArgs == NULL) {
        buildConfigureArgs = "";
    }

    printf("install native package : %s\n", packageName);

    //////////////////////////////////////////////////////////////////////////////

    size_t packageNameLength = strlen(packageName);

    size_t receiptFilePathLength = nativePackageInstalledRootDIRCapacity + packageNameLength + 14U;
    char   receiptFilePath[receiptFilePathLength];

    ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/%s/receipt.txt", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char buf[65] = {0};

            ret = get_the_first_n_bytes_of_a_file(receiptFilePath, 64, buf);

            if (ret != NDKPKG_OK) {
                return ret;
            }

            if (strcmp(buf, srcSha) == 0) {
                fprintf(stderr, "native package '%s' already has been installed.\n", packageName);

                size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + packageNameLength + 2U;
                char   packageInstalledDIR[packageInstalledDIRCapacity];

                ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageName);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                setenv_fn funs[6] = { setenv_CPPFLAGS, setenv_LDFLAGS, setenv_PKG_CONFIG_PATH, setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

                for (int i = 0; i < 6; i++) {
                    ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

                    if (ret != NDKPKG_OK) {
                        return ret;
                    }
                }

                return NDKPKG_OK;
            }
        } else {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", receiptFilePath);
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t nativePackageWorkingTopDIRLength = packageWorkingTopDIRCapacity + packageNameLength + 14U;
    char   nativePackageWorkingTopDIR[nativePackageWorkingTopDIRLength];

    ret = snprintf(nativePackageWorkingTopDIR, nativePackageWorkingTopDIRLength, "%s/native-build-%s", packageWorkingTopDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t nativePackageWorkingSrcDIRLength = nativePackageWorkingTopDIRLength + 5U;
    char   nativePackageWorkingSrcDIR[nativePackageWorkingSrcDIRLength];

    ret = snprintf(nativePackageWorkingSrcDIR, nativePackageWorkingSrcDIRLength, "%s/src", nativePackageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = download_via_http(srcUrl, srcUri, srcSha, downloadsDIR, downloadsDIRLength, nativePackageWorkingSrcDIR, nativePackageWorkingSrcDIRLength, installOptions->verbose_net);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir(nativePackageWorkingSrcDIR) != 0) {
        perror(nativePackageWorkingSrcDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t strBufSize = packageNameLength + strlen(srcUrl) + strlen(srcSha) + 50U;
    char   strBuf[strBufSize];

    ret = snprintf(strBuf, strBufSize, "%s:%s:%s:%zu:%u", packageName, srcUrl, srcSha, time(NULL), getpid());

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    char packageInstalledSHA[65] = {0};

    ret = sha256sum_of_string(packageInstalledSHA, strBuf);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + 66U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageInstalledSHA);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t nativePackageWorkingLibDIRLength = nativePackageWorkingTopDIRLength + 5U;
    char   nativePackageWorkingLibDIR[nativePackageWorkingLibDIRLength];

    ret = snprintf(nativePackageWorkingLibDIR, nativePackageWorkingLibDIRLength, "%s/lib", nativePackageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t nativePackageWorkingIncludeDIRLength = nativePackageWorkingTopDIRLength + 9U;
    char   nativePackageWorkingIncludeDIR[nativePackageWorkingIncludeDIRLength];

    ret = snprintf(nativePackageWorkingIncludeDIR, nativePackageWorkingIncludeDIRLength, "%s/include", nativePackageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t nativePackageWorkingPkgconfigDIRLength = nativePackageWorkingTopDIRLength + 15U;
    char   nativePackageWorkingPkgconfigDIR[nativePackageWorkingPkgconfigDIRLength];

    ret = snprintf(nativePackageWorkingPkgconfigDIR, nativePackageWorkingPkgconfigDIRLength, "%s/lib/pkgconfig", nativePackageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    // override the default search directory (usually /usr/lib/pkgconfig:/usr/share/pkgconfig)
    // because we only want to use our own
    if (setenv("PKG_CONFIG_LIBDIR", nativePackageWorkingLibDIR, 1) != 0) {
        perror("PKG_CONFIG_LIBDIR");
        return NDKPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledLibraryDIRLength = packageInstalledDIRCapacity + 5U;
    char   packageInstalledLibraryDIR[packageInstalledLibraryDIRLength];

    ret = snprintf(packageInstalledLibraryDIR, packageInstalledLibraryDIRLength, "%s/lib", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    const char * const LDFLAGS = getenv("LDFLAGS");

    if (LDFLAGS == NULL || LDFLAGS[0] == '\0') {
        size_t newLDFLAGSLength = packageInstalledLibraryDIRLength + 12U;
        char   newLDFLAGS[newLDFLAGSLength];

        ret = snprintf(newLDFLAGS, newLDFLAGSLength, "-Wl,-rpath,%s", packageInstalledLibraryDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
            perror("LDFLAGS");
            return NDKPKG_ERROR;
        }
    } else {
        size_t newLDFLAGSLength = packageInstalledLibraryDIRLength + strlen(LDFLAGS) + 15U;
        char   newLDFLAGS[newLDFLAGSLength];

        ret = snprintf(newLDFLAGS, newLDFLAGSLength, "-Wl,-rpath,%s %s", packageInstalledLibraryDIR, LDFLAGS);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
            perror("LDFLAGS");
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (nativePackageID == NATIVE_PACKAGE_ID_TEXINFO) {
        const char * cmd = "gsed -i /libintl/d tp/Texinfo/XS/parsetexi/api.c";

        const size_t cmdLength = strlen(cmd);

        char cmdCopy[cmdLength + 1U];

        strncpy(cmdCopy, cmd, cmdLength);

        cmdCopy[cmdLength] = '\0';

        ret = run_cmd(cmdCopy, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (buildSystemType == BUILD_SYSTEM_TYPE_CMAKE) {
        char   cmakePath[PATH_MAX];
        size_t cmakePathLength = 0U;

        ret = exe_where("cmake", cmakePath, PATH_MAX);

        switch (ret) {
            case -3:
                return NDKPKG_ERROR_ENV_PATH_NOT_SET;
            case -2:
                return NDKPKG_ERROR_ENV_PATH_NOT_SET;
            case -1:
                perror(NULL);
                return NDKPKG_ERROR;
            case 0:
                fprintf(stderr, "cmake command was not found.\n");
                return NDKPKG_ERROR;
            default:
                cmakePathLength = ret;
        }

        size_t configurePhaseCmdLength = cmakePathLength + packageInstalledDIRCapacity + strlen(buildConfigureArgs) + 124U;
        char   configurePhaseCmd[configurePhaseCmdLength];

        ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "%s -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_PREFIX=%s -DEXPAT_SHARED_LIBS=OFF -DCMAKE_VERBOSE_MAKEFILE=%s %s -S . -B build.d", cmakePath, packageInstalledDIR, (installOptions->logLevel >= NDKPKGLogLevel_verbose) ? "ON" : "OFF", buildConfigureArgs);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t buildPhaseCmdLength = cmakePathLength + 30U;
        char   buildPhaseCmd[buildPhaseCmdLength];

        ret = snprintf(buildPhaseCmd, buildPhaseCmdLength, "%s --build build.d -- -j%zu", cmakePath, njobs);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(buildPhaseCmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t installPhaseCmdLength = cmakePathLength + 20U;
        char   installPhaseCmd[installPhaseCmdLength];

        ret = snprintf(installPhaseCmd, installPhaseCmdLength, "%s --install build.d", cmakePath);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(installPhaseCmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    } else if (buildSystemType == BUILD_SYSTEM_TYPE_CONFIGURE) {
        if (nativePackageID == NATIVE_PACKAGE_ID_PYTHON3) {
            XX xx[5] = {
                { "zlib",   "-lz",   "ZLIB_CFLAGS",  "ZLIB_LIBS"},
                { "libbz2", "-lbz2", "BZIP2_CFLAGS", "BZIP2_LIBS"},
                { "liblzma", "-llzma", "LIBLZMA_CFLAGS", "LIBLZMA_LIBS"},
                { "sqlite3", "-lsqlite3", "LIBSQLITE3_CFLAGS", "LIBSQLITE3_LIBS"},
                { "gdbm", "-lgdbm -lgdbm_compat", "GDBM_CFLAGS", "GDBM_LIBS"},
            };

            for (int i = 0; i < 5; i++) {
                size_t ccflagsSLength = nativePackageInstalledRootDIRCapacity + strlen(xx[i].name) + 15U;
                char   ccflags[ccflagsSLength];

                ret = snprintf(ccflags, ccflagsSLength, "-I%s/%s/include", nativePackageInstalledRootDIR, xx[i].name);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                size_t ldflagsSLength = nativePackageInstalledRootDIRCapacity + strlen(xx[i].name) + strlen(xx[i].libs) + 10U;
                char   ldflags[ldflagsSLength];

                ret = snprintf(ldflags, ldflagsSLength, "-L%s/%s/lib %s", nativePackageInstalledRootDIR, xx[i].name, xx[i].libs);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if (setenv(xx[i].env1, ccflags, 1) != 0) {
                    perror(xx[i].env1);
                    return NDKPKG_ERROR;
                }

                if (setenv(xx[i].env2, ldflags, 1) != 0) {
                    perror(xx[i].env2);
                    return NDKPKG_ERROR;
                }
            }

            if (setenv("LIBS", "-lm", 1) != 0) {
                perror("LIBS");
                return NDKPKG_ERROR;
            }

            if (unsetenv("PYTHONHOME") < 0) {
                perror("unsetenv PYTHONHOME");
                return NDKPKG_ERROR;
            }

            if (unsetenv("PYTHONPATH") < 0) {
                perror("unsetenv PYTHONPATH");
                return NDKPKG_ERROR;
            }

            const char * const CPPFLAGS = getenv("CPPFLAGS");

            if (CPPFLAGS != NULL && CPPFLAGS[0] != '\0') {
                const char * const CCFLAGS = getenv("CFLAGS");

                size_t newCFLAGSLength = strlen(CCFLAGS) + strlen(CPPFLAGS) + 2U;
                char   newCFLAGS[newCFLAGSLength];

                ret = snprintf(newCFLAGS, newCFLAGSLength, "%s %s", CCFLAGS, CPPFLAGS);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if (setenv("CFLAGS", newCFLAGS, 1) != 0) {
                    perror("CFLAGS");
                    return NDKPKG_ERROR;
                }
            }

            printf("CCFLAGS=%s\n", getenv("CFLAGS"));
            printf("LDFLAGS=%s\n", getenv("LDFLAGS"));

            size_t configurePhaseCmdLength = packageInstalledDIRCapacity + nativePackageInstalledRootDIRCapacity + strlen(buildConfigureArgs) + 60U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./configure --prefix=%s --with-openssl=%s/openssl %s %s", packageInstalledDIR, nativePackageInstalledRootDIR, (installOptions->logLevel == NDKPKGLogLevel_silent) ? "--silent" : "", buildConfigureArgs);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        } else if (nativePackageID == NATIVE_PACKAGE_ID_PERL_XML_PARSER) {
            char   perlPath[PATH_MAX];
            size_t perlPathLength = 0U;

            ret = exe_where("perl", perlPath, PATH_MAX);

            switch (ret) {
                case -3:
                    return NDKPKG_ERROR_ENV_PATH_NOT_SET;
                case -2:
                    return NDKPKG_ERROR_ENV_PATH_NOT_SET;
                case -1:
                    perror(NULL);
                    return NDKPKG_ERROR;
                case 0:
                    fprintf(stderr, "perl command was not found.\n");
                    return NDKPKG_ERROR;
                default:
                    perlPathLength = ret;
            }

            size_t configurePhaseCmdLength = perlPathLength + (packageInstalledDIRCapacity << 1) + 52U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "%s Makefile.PL EXPATLIBPATH=%s/lib EXPATINCPATH=%s/include", perlPath, packageInstalledDIR, packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        } else if (nativePackageID == NATIVE_PACKAGE_ID_PERL) {
            size_t configurePhaseCmdLength = (packageInstalledDIRCapacity * 3) + 170U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./Configure -Dprefix=%s -Dman1dir=%s/share/man/man1 -Dman3dir=%s/share/man/man3 -des -Dmake=gmake -Duselargefiles -Duseshrplib -Dusethreads -Dusenm=false -Dusedl=true", packageInstalledDIR, packageInstalledDIR, packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        } else if (nativePackageID == NATIVE_PACKAGE_ID_OPENSSL) {
#if defined (__OpenBSD__)
            // https://github.com/openssl/openssl/issues/19232

            const char * patchPhaseCmd = "/usr/bin/sed -i s/-Wl,-z,defs// Configurations/shared-info.pl";

            size_t  patchPhaseCmdCopyLength = strlen(patchPhaseCmd);
            char    patchPhaseCmdCopy[patchPhaseCmdCopyLength + 1U];
            strncpy(patchPhaseCmdCopy, patchPhaseCmd, patchPhaseCmdCopyLength);

            patchPhaseCmdCopy[patchPhaseCmdCopyLength] = '\0';

            ret = run_cmd(patchPhaseCmdCopy, STDOUT_FILENO);

            if (ret != NDKPKG_OK) {
                return ret;
            }
#endif

            //////////////////////////////////////////////////////////////////////////////

            size_t configurePhaseCmdLength = (packageInstalledDIRCapacity * 3) + 100U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./config no-tests no-ssl3 no-ssl3-method no-zlib --prefix=%s --libdir=%s/lib --openssldir=%s/etc/ssl", packageInstalledDIR, packageInstalledDIR, packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        } else {
            size_t configurePhaseCmdLength = packageInstalledDIRCapacity + strlen(buildConfigureArgs) + 32U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./configure --prefix=%s %s %s", packageInstalledDIR, (installOptions->logLevel == NDKPKGLogLevel_silent) ? "--silent" : "", buildConfigureArgs);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        char   gmakePath[PATH_MAX];
        size_t gmakePathLength = 0U;

        ret = exe_where("gmake", gmakePath, PATH_MAX);

        switch (ret) {
            case -3:
                return NDKPKG_ERROR_ENV_PATH_NOT_SET;
            case -2:
                return NDKPKG_ERROR_ENV_PATH_NOT_SET;
            case -1:
                perror(NULL);
                return NDKPKG_ERROR;
            case 0:
                break;
            default:
                gmakePathLength = ret;
        }

        if (ret == 0) {
            ret = exe_where("make", gmakePath, PATH_MAX);

            switch (ret) {
                case -3:
                    return NDKPKG_ERROR_ENV_PATH_NOT_SET;
                case -2:
                    return NDKPKG_ERROR_ENV_PATH_NOT_SET;
                case -1:
                    perror(NULL);
                    return NDKPKG_ERROR;
                case 0:
                    fprintf(stderr, "neither gmake nor make command was found.\n");
                    return NDKPKG_ERROR;
                default:
                    gmakePathLength = ret;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t buildPhaseCmdLength = gmakePathLength + 12U;
        char   buildPhaseCmd[buildPhaseCmdLength];

        ret = snprintf(buildPhaseCmd, buildPhaseCmdLength, "%s --jobs=%zu", gmakePath, njobs);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(buildPhaseCmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t installPhaseCmdLength = gmakePathLength + 20U;
        char   installPhaseCmd[installPhaseCmdLength];

        ret = snprintf(installPhaseCmd, installPhaseCmdLength, "%s install", gmakePath);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(installPhaseCmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    size_t receiptFilePath2Length = packageInstalledDIRCapacity + 14U;
    char   receiptFilePath2[receiptFilePath2Length];

    ret = snprintf(receiptFilePath2, receiptFilePath2Length, "%s/receipt.txt", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = write_to_file(receiptFilePath2, srcSha);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    if (chdir(nativePackageInstalledRootDIR) != 0) {
        perror(nativePackageInstalledRootDIR);
        return NDKPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "native package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = ndkpkg_rm_r(packageName, installOptions->logLevel >= NDKPKGLogLevel_verbose);

                        if (ret != NDKPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return NDKPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return NDKPKG_ERROR;
            }
        }
    }

    setenv_fn funs[6] = { setenv_CPPFLAGS, setenv_LDFLAGS, setenv_PKG_CONFIG_PATH, setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

    for (int i = 0; i < 6; i++) {
        ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    return NDKPKG_OK;
}

static int install_dependent_packages_via_uppm(
        const char * uppmPackageNames,
        const size_t uppmPackageNamesLength,
        const char * ndkpkgCoreDIR,
        const size_t ndkpkgCoreDIRCapacity,
        const char * uppmPackageInstalledRootDIR,
        const size_t uppmPackageInstalledRootDIRLength,
        const bool   needToInstallCmake) {

#if defined (__NetBSD__)
    if (needToInstallCmake) {
        char cmd[28];

        int ret;

        if (geteuid() == 0) {
            ret = snprintf(cmd, 28, "pkgin -y install cmake");
        } else {
            ret = snprintf(cmd, 28, "sudo pkgin -y install cmake");
        }

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(cmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }
#endif

    //////////////////////////////////////////////////////////////////////////////

    int ret;

    if (true) {
        size_t uppmUpdateCmdLength = ndkpkgCoreDIRCapacity + 13U;
        char   uppmUpdateCmd[uppmUpdateCmdLength];

        ret = snprintf(uppmUpdateCmd, uppmUpdateCmdLength, "%s/uppm update", ndkpkgCoreDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(uppmUpdateCmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    size_t uppmInstallCmdLength = ndkpkgCoreDIRCapacity + uppmPackageNamesLength + 15U;
    char   uppmInstallCmd[uppmInstallCmdLength];

    ret = snprintf(uppmInstallCmd, uppmInstallCmdLength, "%s/uppm install %s", ndkpkgCoreDIR, uppmPackageNames);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = run_cmd(uppmInstallCmd, STDOUT_FILENO);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char    uppmPackageNamesCopy[uppmPackageNamesLength + 1U];
    strncpy(uppmPackageNamesCopy, uppmPackageNames, uppmPackageNamesLength);

    uppmPackageNamesCopy[uppmPackageNamesLength] = '\0';

    char * uppmPackageName = strtok(uppmPackageNamesCopy, " ");

    while (uppmPackageName != NULL) {
        size_t uppmPackageInstalledDIRCapacity = uppmPackageInstalledRootDIRLength + strlen(uppmPackageName) + 2U;
        char   uppmPackageInstalledDIR[uppmPackageInstalledDIRCapacity];

        ret = snprintf(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity, "%s/%s", uppmPackageInstalledRootDIR, uppmPackageName);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = setenv_PATH(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        ret = setenv_ACLOCAL_PATH(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        ret = setenv_XDG_DATA_DIRS(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        if (strcmp(uppmPackageName, "git") == 0) {
            // https://git-scm.com/book/en/v2/Git-Internals-Environment-Variables

            size_t gitCoreDIRCapacity = uppmPackageInstalledDIRCapacity + 18U;
            char   gitCoreDIR[gitCoreDIRCapacity];

            ret = snprintf(gitCoreDIR, gitCoreDIRCapacity, "%s/libexec/git-core", uppmPackageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("GIT_EXEC_PATH" , gitCoreDIR, 1) != 0) {
                perror("GIT_EXEC_PATH");
                return NDKPKG_ERROR;
            }

            size_t gitTemplateDIRCapacity = uppmPackageInstalledDIRCapacity + 26U;
            char   gitTemplateDIR[gitTemplateDIRCapacity];

            ret = snprintf(gitTemplateDIR, gitTemplateDIRCapacity, "%s/share/git-core/templates", uppmPackageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("GIT_TEMPLATE_DIR" , gitTemplateDIR, 1) != 0) {
                perror("GIT_TEMPLATE_DIR");
                return NDKPKG_ERROR;
            }
        } else if (strcmp(uppmPackageName, "docbook-xsl") == 0) {
            // http://xmlsoft.org/xslt/xsltproc.html

            size_t xmlCatalogFilePathCapacity = uppmPackageInstalledDIRCapacity + 13U;
            char   xmlCatalogFilePath[xmlCatalogFilePathCapacity];

            ret = snprintf(xmlCatalogFilePath, xmlCatalogFilePathCapacity, "%s/catalog.xml", uppmPackageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv("XML_CATALOG_FILES" , xmlCatalogFilePath, 1) != 0) {
                perror("XML_CATALOG_FILES");
                return NDKPKG_ERROR;
            }
        }

        uppmPackageName = strtok(NULL, " ");
    }

    return NDKPKG_OK;
}

static int generate_shell_vars_file(
        const char * varsFilePath,
        const char * packageName,
        const NDKPKGFormula * formula,
        const NDKPKGInstallOptions * installOptions,
        const SysInfo * sysinfo,
        const char * uppmPackageInstalledRootDIR,
        const char * ndkpkgExeFilePath,
        const time_t ts,
        const size_t njobs,
        const char * ndkpkgHomeDIR,
        const char * ndkpkgCoreDIR,
        const char * ndkpkgDownloadsDIR,
        const char * sessionDIR,
        const char * recursiveDependentPackageNamesString) {
    KB kbs[] = {
        {"KEEP_SESSION_DIR", installOptions->keepSessionDIR},
        {"BEAR_ENABLED", installOptions->enableBear},
        {"CCACHE_ENABLED", installOptions->enableCcache},
        {"EXPORT_COMPILE_COMMANDS_JSON", installOptions->exportCompileCommandsJson},
        {"PACKAGE_BINBSTD", formula->binbstd},
        {"PACKAGE_SYMLINK", formula->symlink},
        {"PACKAGE_PARALLEL", formula->parallel},
        {"PACKAGE_BUILD_IN_BSCRIPT_DIR", formula->binbstd},
        {"PACKAGE_USE_BSYSTEM_AUTOGENSH", formula->useBuildSystemAutogen},
        {"PACKAGE_USE_BSYSTEM_AUTOTOOLS", formula->useBuildSystemAutotools},
        {"PACKAGE_USE_BSYSTEM_CONFIGURE", formula->useBuildSystemConfigure},
        {"PACKAGE_USE_BSYSTEM_CMAKE", formula->useBuildSystemCmake},
        {"PACKAGE_USE_BSYSTEM_XMAKE", formula->useBuildSystemXmake},
        {"PACKAGE_USE_BSYSTEM_GMAKE", formula->useBuildSystemGmake},
        {"PACKAGE_USE_BSYSTEM_NINJA", formula->useBuildSystemNinja},
        {"PACKAGE_USE_BSYSTEM_MESON", formula->useBuildSystemMeson},
        {"PACKAGE_USE_BSYSTEM_CARGO", formula->useBuildSystemCargo},
        {"PACKAGE_USE_BSYSTEM_GO", formula->useBuildSystemGolang},
        {NULL,false}
    };

    char * libcName;

    switch(sysinfo->libc) {
        case 1:  libcName = (char*)"glibc"; break;
        case 2:  libcName = (char*)"musl";  break;
        default: libcName = (char*)"";
    }

    const char * linkType;

    switch (installOptions->linkType) {
        case NDKPKGLinkType_static_full: linkType = (formula->sfslink) ? "static-full" : "static-most"; break;
        case NDKPKGLinkType_static_most: linkType = "static-most"; break;
        case NDKPKGLinkType_shared_full: linkType = "shared-full"; break;
        case NDKPKGLinkType_shared_most: linkType = "shared-most"; break;
    }

    KV kvs[] = {
        {"NATIVE_OS_ARCH", sysinfo->arch },
        {"NATIVE_OS_KIND", sysinfo->kind },
        {"NATIVE_OS_TYPE", sysinfo->type },
        {"NATIVE_OS_CODE", sysinfo->code },
        {"NATIVE_OS_NAME", sysinfo->name },
        {"NATIVE_OS_VERS", sysinfo->vers },
        {"NATIVE_OS_LIBC", libcName },
        {"BUILD_TYPE", installOptions->buildType == NDKPKGBuildType_release ? "release" : "debug"},
        {"LINK_TYPE", linkType},
        {"INSTALL_LIB", "both"},
        {"NDKPKG_VERSION", NDKPKG_VERSION},
        {"NDKPKG", ndkpkgExeFilePath},
        {"NDKPKG_HOME", ndkpkgHomeDIR},
        {"NDKPKG_CORE_DIR", ndkpkgCoreDIR},
        {"NDKPKG_DOWNLOADS_DIR", ndkpkgDownloadsDIR},
        {"UPPM_PACKAGE_INSTALL_ROOT_DIR", uppmPackageInstalledRootDIR},
        {"SESSION_DIR", sessionDIR},
        {"RECURSIVE_DEPENDENT_PACKAGE_NAMES", recursiveDependentPackageNamesString},
        {"PACKAGE_FORMULA_FILEPATH", formula->path},
        {"PACKAGE_NAME", packageName},
        {"PACKAGE_SUMMARY", formula->summary},
        {"PACKAGE_VERSION", formula->version},
        {"PACKAGE_LICENSE", formula->license},
        {"PACKAGE_WEB_URL", formula->web_url},
        {"PACKAGE_GIT_URL", formula->git_url},
        {"PACKAGE_GIT_SHA", formula->git_sha},
        {"PACKAGE_GIT_REF", formula->git_ref},
        {"PACKAGE_SRC_URL", formula->src_url},
        {"PACKAGE_SRC_URI", formula->src_uri},
        {"PACKAGE_SRC_SHA", formula->src_sha},
        {"PACKAGE_FIX_URL", formula->fix_url},
        {"PACKAGE_FIX_URI", formula->fix_uri},
        {"PACKAGE_FIX_SHA", formula->fix_sha},
        {"PACKAGE_RES_URL", formula->res_url},
        {"PACKAGE_RES_URI", formula->res_uri},
        {"PACKAGE_RES_SHA", formula->res_sha},
        {"PACKAGE_DEP_PKG", formula->dep_pkg},
        {"PACKAGE_DEP_UPP", formula->dep_upp},
        {"PACKAGE_DEP_PYM", formula->dep_pym},
        {"PACKAGE_DEP_PLM", formula->dep_plm},
        {"PACKAGE_BSYSTEM", formula->bsystem},
        {"PACKAGE_BSCRIPT", formula->bscript},
        {"PACKAGE_PPFLAGS", formula->ppflags},
        {"PACKAGE_CCFLAGS", formula->ccflags},
        {"PACKAGE_XXFLAGS", formula->xxflags},
        {"PACKAGE_LDFLAGS", formula->ldflags},
        {NULL, NULL},
    };

    int fd = open(varsFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(varsFilePath);
        return NDKPKG_ERROR;
    }

    int ret;

    if (installOptions->xtrace) {
        ret = dprintf(fd, "set -x\n");

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    }

    ret = dprintf(fd, "set -e\n\n");

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    ret = dprintf(fd, "TIMESTAMP_UNIX=%zu\n", ts);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_OS_NCPU=%u\n", sysinfo->ncpu);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_OS_EUID=%u\n", geteuid());

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_OS_EGID=%u\n", getegid());

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    ret = dprintf(fd, "LOG_LEVEL=%d\n", installOptions->logLevel);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    ret = dprintf(fd, "BUILD_NJOBS=%zu\n", njobs);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    ret = dprintf(fd, "PACKAGE_GIT_NTH=%zu\n", formula->git_nth);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    for (int i = 0; kbs[i].name != NULL; i++) {
        ret = dprintf(fd, "%s=%d\n", kbs[i].name, kbs[i].value);

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    }

    for (int i = 0; kvs[i].name != NULL; i++) {
        ret = dprintf(fd, "%s='%s'\n", kvs[i].name, (kvs[i].value == NULL) ? "" : kvs[i].value);

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    }

    close(fd);

    return NDKPKG_OK;
}

static int generate_install_shell_script_file(
        const char * installshFilePath,
        const char * varsFilePath,
        const char * packageName,
        const NDKPKGFormula * formula,
        const NDKPKGTargetPlatform * targetPlatform,
        const NDKPKGInstallOptions * installOptions,
        const bool isCrossBuild,
        const char * ndkpkgCoreDIR,
        const char * packageWorkingTopDIR,
        const char * packageWorkingSrcDIR,
        const char * packageWorkingFixDIR,
        const char * packageWorkingResDIR,
        const char * packageWorkingBinDIR,
        const char * packageWorkingLibDIR,
        const char * packageWorkingIncDIR,
        const char * packageWorkingTmpDIR,
        const char * packageInstalledRootDIR,
        const size_t packageInstalledRootDIRCapacity,
        const char * nativePackageInstalledRootDIR,
        const char * packageInstalledDIR,
        const char * packageMetaInfoDIR,
        const char * recursiveDependentPackageNamesString,
        const size_t recursiveDependentPackageNamesStringSize) {
    int fd = open(installshFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(installshFilePath);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    int ret;

    if (installOptions->xtrace) {
        ret = dprintf(fd, "set -x\n");

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "set -e\n\n");

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "CROSS_COMPILING=%d\n\n", isCrossBuild);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "TARGET_PLATFORM_VERS=%u\n", targetPlatform->api);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * targetABI;
    const char * targetARCH;

    unsigned int nbit;

    switch (targetPlatform->abi) {
        case AndroidABI_arm64_v8a:
            targetABI  = "arm64-v8a";
            targetARCH = "aarch64";
            nbit = 64U;
            break;
        case AndroidABI_armeabi_v7a:
            targetABI  = "armeabi-v7a";
            targetARCH = "armv7a";
            nbit = 32U;
            break;
        case AndroidABI_x86_64:
            targetABI  = "x86_64";
            targetARCH = "x86_64";
            nbit = 64U;
            break;
        case AndroidABI_x86:
            targetABI  = "x86";
            targetARCH = "i686";
            nbit = 32U;
            break;
    }

    ret = dprintf(fd, "TARGET_PLATFORM_NBIT=%u\n", nbit);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV kvs[] = {
        {"TARGET_PLATFORM_NAME", "android" },
        {"TARGET_PLATFORM_ABI", targetABI },
        {"TARGET_PLATFORM_ARCH", targetARCH },
        {"STATIC_LIBRARY_SUFFIX", ".a"},
        {"SHARED_LIBRARY_SUFFIX", ".so"},
        {"NDKPKG_PACKAGE_INSTALLED_ROOT", packageInstalledRootDIR},
        {"PACKAGE_INSTALLING_SRC_DIR", packageWorkingSrcDIR},
        {"PACKAGE_INSTALLING_FIX_DIR", packageWorkingFixDIR},
        {"PACKAGE_INSTALLING_RES_DIR", packageWorkingResDIR},
        {"PACKAGE_INSTALLING_BIN_DIR", packageWorkingBinDIR},
        {"PACKAGE_INSTALLING_INC_DIR", packageWorkingIncDIR},
        {"PACKAGE_INSTALLING_LIB_DIR", packageWorkingLibDIR},
        {"PACKAGE_INSTALLING_TMP_DIR", packageWorkingTmpDIR},
        {"PACKAGE_WORKING_DIR", packageWorkingTopDIR},
        {"PACKAGE_INSTALL_DIR", packageInstalledDIR},
        {"PACKAGE_METAINF_DIR", packageMetaInfoDIR},
        {NULL, NULL},
    };

    for (int i = 0; kvs[i].name != NULL; i++) {
        ret = dprintf(fd, "%s='%s'\n", kvs[i].name, (kvs[i].value == NULL) ? "" : kvs[i].value);

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->bscript == NULL) {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s'\n", packageWorkingSrcDIR);
    } else {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s/%s'\n", packageWorkingSrcDIR, formula->bscript);
    }

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "PACKAGE_BCACHED_DIR='%s/_'\n", packageWorkingSrcDIR);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->bscript == NULL) {
        ret = dprintf(fd, "NATIVE_BCACHED_DIR='%s/build-native-%s/_'\n", packageWorkingTopDIR, packageName);

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    } else {
        ret = dprintf(fd, "NATIVE_BCACHED_DIR='%s/build-native-%s/%s/_'\n", packageWorkingTopDIR, packageName, formula->bscript);

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "NATIVE_INSTALL_DIR='%s/%s'\n", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (recursiveDependentPackageNamesString != NULL) {
        size_t  recursiveDependentPackageNamesStringCopyCapacity = recursiveDependentPackageNamesStringSize + 1U;
        char    recursiveDependentPackageNamesStringCopy[recursiveDependentPackageNamesStringCopyCapacity];
        strncpy(recursiveDependentPackageNamesStringCopy, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringCopyCapacity);

        char * dependentPackageName = strtok(recursiveDependentPackageNamesStringCopy, " ");

        while (dependentPackageName != NULL) {
            size_t installedDIRCapacity = packageInstalledRootDIRCapacity + strlen(dependentPackageName) + 1U;
            char   installedDIR[installedDIRCapacity];

            ret = snprintf(installedDIR, installedDIRCapacity, "%s/%s", packageInstalledRootDIR, dependentPackageName);

            if (ret < 0) {
                perror(NULL);
                close(fd);
                return NDKPKG_ERROR;
            }

            size_t libDIRCapacity = installedDIRCapacity + 4U;
            char   libDIR[libDIRCapacity];

            ret = snprintf(libDIR, libDIRCapacity, "%s/lib", installedDIR);

            if (ret < 0) {
                perror(NULL);
                close(fd);
                return NDKPKG_ERROR;
            }

            size_t includeDIRCapacity = installedDIRCapacity + 8U;
            char   includeDIR[includeDIRCapacity];

            ret = snprintf(includeDIR, includeDIRCapacity, "%s/include", installedDIR);

            if (ret < 0) {
                perror(NULL);
                close(fd);
                return NDKPKG_ERROR;
            }

            for (int i = 0; ; i++) {
                char c = dependentPackageName[i];

                if (c == '\0') {
                    break;
                }

                if (c == '@' || c == '+' || c == '-' || c == '.') {
                    dependentPackageName[i] = '_';
                }
            }

            KV kvs[3] = {
                {"INSTALL", installedDIR},
                {"INCLUDE", includeDIR},
                {"LIBRARY", libDIR}
            };

            for (int i = 0; i < 3; i++) {
                ret = dprintf(fd, "%s_%s_DIR='%s'\n", dependentPackageName, kvs[i].name, kvs[i].value);

                if (ret < 0) {
                    close(fd);
                    return NDKPKG_ERROR;
                }
            }

            dependentPackageName = strtok(NULL, " ");
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "\ndopatch() {\n%s\n}\n", formula->dopatch == NULL ? ":" : formula->dopatch);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    ret = dprintf(fd, "\ndobuild() {\n%s\n}\n\n", formula->install);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    ret = dprintf(fd, ". %s\n. %s/ndkpkg-install\n", varsFilePath, ndkpkgCoreDIR);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    close(fd);

    return NDKPKG_OK;
}

static int adjust_la_file(const char * filePath, const char * ndkpkgHomeDIR, const size_t ndkpkgHomeDIRLength) {
    size_t eLength = ndkpkgHomeDIRLength + 14U;
    char   e[eLength];

    int ret = snprintf(e, eLength, "s|-L%s[^' ]*||g", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (pid == 0) {
        execlp("sed", "sed", "-i", "-e", "s/-Wl,--strip-debug//g", "-e", "s|-R[^' ]*||g", "-e", e, filePath, NULL);
        perror("sed");
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return NDKPKG_OK;
        } else {
            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' exit with status code: %d\n", e, filePath, WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' killed by signal: %d\n", e, filePath, WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' stopped by signal: %d\n", e, filePath, WSTOPSIG(childProcessExitStatusCode));
            }

            return NDKPKG_ERROR;
        }
    }
}

static int adjust_la_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity, const char * ndkpkgHomeDIR, const size_t ndkpkgHomeDIRLength) {
    size_t packageLibDIRLength = packageInstalledDIRCapacity + 5U;
    char   packageLibDIR[packageLibDIRLength];

    int ret = snprintf(packageLibDIR, packageLibDIRLength, "%s/lib", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(packageLibDIR, &st) != 0) {
        return NDKPKG_OK;
    }

    DIR * dir = opendir(packageLibDIR);

    if (dir == NULL) {
        perror(packageLibDIR);
        return NDKPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return NDKPKG_OK;
            } else {
                perror(packageLibDIR);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t fileNameLength = strlen(dir_entry->d_name);

        if (fileNameLength < 4U) {
            continue;
        }

        char * fileNameSuffix = dir_entry->d_name + fileNameLength - 3U;

        if (strcmp(fileNameSuffix, ".la") == 0) {
            size_t filePathLength = packageLibDIRLength + fileNameLength  + 2U;
            char   filePath[filePathLength];

            ret = snprintf(filePath, filePathLength, "%s/%s", packageLibDIR, dir_entry->d_name);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (stat(filePath, &st) != 0) {
                closedir(dir);
                return NDKPKG_ERROR;
            }

            if (S_ISREG(st.st_mode)) {
                int ret = adjust_la_file(filePath, ndkpkgHomeDIR, ndkpkgHomeDIRLength);

                if (ret != NDKPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int adjust_pc_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t packagePkgconfigDIRLength = packageInstalledDIRCapacity + 15U;
    char   packagePkgconfigDIR[packagePkgconfigDIRLength];

    int ret = snprintf(packagePkgconfigDIR, packagePkgconfigDIRLength, "%s/lib/pkgconfig", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(packagePkgconfigDIR, &st) != 0) {
        return NDKPKG_OK;
    }

    DIR * dir = opendir(packagePkgconfigDIR);

    if (dir == NULL) {
        perror(packagePkgconfigDIR);
        return NDKPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return NDKPKG_OK;
            } else {
                perror(packagePkgconfigDIR);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t fileNameLength = strlen(dir_entry->d_name);

        if (fileNameLength < 4U) {
            continue;
        }

        char * fileNameSuffix = dir_entry->d_name + fileNameLength - 3U;

        if (strcmp(fileNameSuffix, ".pc") == 0) {
            size_t filePathLength = packagePkgconfigDIRLength + fileNameLength  + 2U;
            char   filePath[filePathLength];

            ret = snprintf(filePath, filePathLength, "%s/%s", packagePkgconfigDIR, dir_entry->d_name);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (stat(filePath, &st) != 0) {
                closedir(dir);
                return NDKPKG_ERROR;
            }

            if (S_ISREG(st.st_mode)) {
                int ret = NDKPKG_OK;

                if (ret != NDKPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int adjust_elf_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    return NDKPKG_OK;
}

static int adjust_macho_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    return NDKPKG_OK;
}

static int backup_formulas(const char * sessionDIR, const char * packageMetaInfoDIR, const size_t packageMetaInfoDIRCapacity, const char * recursiveDependentPackageNamesString, const size_t recursiveDependentPackageNamesStringSize) {
    size_t packageInstalledFormulaDIRLength = packageMetaInfoDIRCapacity + 9U;
    char   packageInstalledFormulaDIR[packageInstalledFormulaDIRLength];

    int ret = snprintf(packageInstalledFormulaDIR, packageInstalledFormulaDIRLength, "%s/formula", packageMetaInfoDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir(packageInstalledFormulaDIR, S_IRWXU) != 0) {
        perror(packageInstalledFormulaDIR);
        return NDKPKG_ERROR;
    }

    size_t  recursiveDependentPackageNamesStringCopyCapacity = recursiveDependentPackageNamesStringSize + 1U;
    char    recursiveDependentPackageNamesStringCopy[recursiveDependentPackageNamesStringCopyCapacity];
    strncpy(recursiveDependentPackageNamesStringCopy, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringCopyCapacity);

    char * packageName = strtok(recursiveDependentPackageNamesStringCopy, " ");

    while (packageName != NULL) {
        size_t packageNameLength = strlen(packageName);

        size_t fromFilePathLength = strlen(sessionDIR) + packageNameLength  + 6U;
        char   fromFilePath[fromFilePathLength];

        ret = snprintf(fromFilePath, fromFilePathLength, "%s/%s.yml", sessionDIR, packageName);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        size_t toFilePathLength = packageInstalledFormulaDIRLength + packageNameLength + 6U;
        char   toFilePath[toFilePathLength];

        ret = snprintf(toFilePath, toFilePathLength, "%s/%s.yml", packageInstalledFormulaDIR, packageName);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = ndkpkg_copy_file(fromFilePath, toFilePath);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        packageName = strtok(NULL, " ");
    }

    return NDKPKG_OK;
}

static int generate_manifest_r(const char * dirPath, const size_t offset, FILE * installedManifestFile) {
    if (dirPath == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (dirPath[0] == '\0') {
        return NDKPKG_ERROR_ARG_IS_EMPTY;
    }

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return NDKPKG_ERROR;
    }

    size_t dirPathLength = strlen(dirPath);

    int ret = NDKPKG_OK;

    struct stat st;

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return NDKPKG_OK;
            } else {
                perror(dirPath);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filePathLength = dirPathLength + strlen(dir_entry->d_name) + 2U;
        char   filePath[filePathLength];

        ret = snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return NDKPKG_ERROR;
        }

        if (stat(filePath, &st) != 0) {
            perror(filePath);
            closedir(dir);
            return NDKPKG_ERROR;
        }

        if (S_ISDIR(st.st_mode)) {
            ret = fprintf(installedManifestFile, "d|%s/\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return NDKPKG_ERROR;
            }

            ret = generate_manifest_r(filePath, offset, installedManifestFile);

            if (ret != NDKPKG_OK) {
                closedir(dir);
                return ret;
            }
        } else {
            ret = fprintf(installedManifestFile, "f|%s\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }
    }
}

int generate_manifest(const char * installedDIRPath) {
    size_t installedDIRLength = strlen(installedDIRPath);

    size_t installedManifestFilePathLength = installedDIRLength + 20U;
    char   installedManifestFilePath[installedManifestFilePathLength];

    int ret = snprintf(installedManifestFilePath, installedManifestFilePathLength, "%s/.ndk-pkg/MANIFEST.txt", installedDIRPath);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    FILE * installedManifestFile = fopen(installedManifestFilePath, "w");

    if (installedManifestFile == NULL) {
        perror(installedManifestFilePath);
        return NDKPKG_ERROR;
    }

    ret = generate_manifest_r(installedDIRPath, installedDIRLength + 1, installedManifestFile);

    fclose(installedManifestFile);

    return ret;
}

static int generate_receipt(const char * packageName, const NDKPKGFormula * formula, const char * targetPlatformSpec, const SysInfo * sysinfo, const time_t ts, const char * packageMetaInfoDIR, const size_t packageMetaInfoDIRCapacity) {
    size_t receiptFilePathLength = packageMetaInfoDIRCapacity + 12U;
    char   receiptFilePath[receiptFilePathLength];

    int ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/RECEIPT.yml", packageMetaInfoDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    FILE * receiptFile = fopen(receiptFilePath, "w");

    if (receiptFile == NULL) {
        perror(receiptFilePath);
        return NDKPKG_ERROR;
    }

    FILE * formulaFile = fopen(formula->path, "r");

    if (formulaFile == NULL) {
        perror(formula->path);
        fclose(receiptFile);
        return NDKPKG_ERROR;
    }

    fprintf(receiptFile, "pkgname: %s\n", packageName);

    if (formula->version_is_calculated) {
        fprintf(receiptFile, "version: %s\n", formula->version);
    }

    if (formula->bsystem_is_calculated) {
        fprintf(receiptFile, "bsystem: %s\n", formula->bsystem);
    }

    if (formula->web_url_is_calculated) {
        fprintf(receiptFile, "web-url: %s\n", formula->web_url);
    }

    char   buff[2048];
    size_t size = 0U;

    for (;;) {
        size = fread(buff, 1, 2048, formulaFile);

        if (ferror(formulaFile)) {
            perror(formula->path);
            fclose(formulaFile);
            fclose(receiptFile);
            return NDKPKG_ERROR;
        }

        if (size > 0) {
            if (fwrite(buff, 1, size, receiptFile) != size || ferror(receiptFile)) {
                perror(receiptFilePath);
                fclose(receiptFile);
                fclose(formulaFile);
                return NDKPKG_ERROR;
            }
        }

        if (feof(formulaFile)) {
            fclose(formulaFile);
            break;
        }
    }

    char * libcName;

    switch(sysinfo->libc) {
        case 1:  libcName = (char*)"glibc"; break;
        case 2:  libcName = (char*)"musl";  break;
        default: libcName = (char*)"";
    }

    fprintf(receiptFile, "\nbuiltfor: %s\nbuiltby: %s\nbuiltat: %zu\n\n", targetPlatformSpec, NDKPKG_VERSION, ts);

    fprintf(receiptFile, "build-on:\n    os-arch: %s\n    os-kind: %s\n    os-type: %s\n    os-name: %s\n    os-vers: %s\n    os-ncpu: %u\n    os-libc: %s\n    os-euid: %u\n    os-egid: %u\n", sysinfo->arch, sysinfo->kind, sysinfo->type, sysinfo->name, sysinfo->vers, sysinfo->ncpu, libcName, sysinfo->euid, sysinfo->egid);

    fclose(receiptFile);

    return NDKPKG_OK;
}

static int install_files_to_metainfo_dir(struct stat st, const char * fromDIR, size_t fromDIRLength, const char * toDIR, size_t toDIRLength, const char * item, size_t itemLength) {
    size_t fromFilePathLength = fromDIRLength + itemLength + 2U;
    char   fromFilePath[fromFilePathLength];

    int ret = snprintf(fromFilePath, fromFilePathLength, "%s/%s", fromDIR, item);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t toFilePathLength = toDIRLength + itemLength + 2U;
    char   toFilePath[toFilePathLength];

    ret = snprintf(toFilePath, toFilePathLength, "%s/%s", toDIR, item);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (stat(fromFilePath, &st) == 0) {
        return ndkpkg_copy_file(fromFilePath, toFilePath);
    }

    return NDKPKG_OK;
}

static int generate_linker_script(const char * packageWorkingLibDIR, const size_t packageWorkingLibDIRCapacity, const char * filename) {
    size_t filepathCapacity = packageWorkingLibDIRCapacity + strlen(filename) + 2U;
    char   filepath[filepathCapacity];

    const int ret = snprintf(filepath, filepathCapacity, "%s/%s", packageWorkingLibDIR, filename);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    const int fd = open(filepath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(filepath);
        return NDKPKG_ERROR;
    }

    const char * const content = "INPUT(-lc)\n";

    const size_t contentLength = strlen(content);

    const ssize_t written = write(fd, content, contentLength);

    if (written == -1) {
        perror(filepath);
        close(fd);
        return NDKPKG_ERROR;
    }

    close(fd);

    if (written == contentLength) {
        return NDKPKG_OK;
    } else {
        fprintf(stderr, "file not fully written: %s\n", filepath);
        return NDKPKG_ERROR;
    }
}

static int copy_dependent_libraries(
        const char * depPackageInstalledDIR,
        const size_t depPackageInstalledDIRLength,
        const char * toDIR,
        const size_t toDIRLength,
        const char * libSuffix,
        const size_t libSuffixLength) {

    size_t fromDIRCapacity = depPackageInstalledDIRLength + 5U;
    char   fromDIR[fromDIRCapacity];

    int ret = snprintf(fromDIR, fromDIRCapacity, "%s/lib", depPackageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(fromDIR, &st) != 0) {
        return NDKPKG_OK;
    }

    DIR * dir = opendir(fromDIR);

    if (dir == NULL) {
        perror(fromDIR);
        return NDKPKG_ERROR;
    }

    char * fileName;
    char * fileNameSuffix;
    size_t fileNameLength;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return NDKPKG_OK;
            } else {
                perror(fromDIR);
                closedir(dir);
                return NDKPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        fileName = dir_entry->d_name;

        if (strncmp(fileName, "lib", 3) == 0) {
            fileNameLength = strlen(fileName);

            fileNameSuffix = fileName + fileNameLength - libSuffixLength;

            if (strcmp(fileNameSuffix, libSuffix) == 0) {
                size_t   fromFilePathCapacity = fromDIRCapacity + fileNameLength + 2U;
                char     fromFilePath[fromFilePathCapacity];

                ret = snprintf(fromFilePath, fromFilePathCapacity, "%s/%s", fromDIR, fileName);

                if (ret < 0) {
                    perror(NULL);
                    closedir(dir);
                    return NDKPKG_ERROR;
                }

                size_t   toFilePathCapacity = toDIRLength + fileNameLength + 2U;
                char     toFilePath[toFilePathCapacity];

                ret = snprintf(toFilePath, toFilePathCapacity, "%s/%s", toDIR, fileName);

                if (ret < 0) {
                    perror(NULL);
                    closedir(dir);
                    return NDKPKG_ERROR;
                }

                ret = ndkpkg_copy_file(fromFilePath, toFilePath);

                if (ret != NDKPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

//  /home/leleliu008/.ndk-pkg/run/<pid>/<target-platform-spec>-<package-name>                     <- packageWorkingTopDIR
//  /home/leleliu008/.ndk-pkg/run/<pid>/<target-platform-spec>-<package-name>/native-build-<PKG>  <- workingDIR
static int ndkpkg_build_for_native(
        const NDKPKGInstallOptions * installOptions,
        const NDKPKGFormula * formula,
        const char * packageName,
        const size_t packageNameLength,
        const char * ndkpkgCoreDIR,
        const size_t ndkpkgCoreDIRCapacity,
        const char * ndkpkgDownloadsDIR,
        const size_t ndkpkgDownloadsDIRCapacity,
        const char * packageWorkingTopDIR,
        const size_t packageWorkingTopDIRCapacity,
        const char * nativePackageInstalledRootDIR,
        const size_t nativePackageInstalledRootDIRCapacity,
        const char * packageInstalledSHA,
        const char * varsFilePath,
        const bool verbose) {
    size_t receiptFilePathLength = nativePackageInstalledRootDIRCapacity + packageNameLength + 14U;
    char   receiptFilePath[receiptFilePathLength];

    int ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/%s/receipt.txt", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    struct stat st;

    if (stat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char buf[65] = {0};

            ret = get_the_first_n_bytes_of_a_file(receiptFilePath, 64, buf);

            if (ret != NDKPKG_OK) {
                return ret;
            }

            if (strcmp(buf, formula->src_sha) == 0) {
                fprintf(stderr, "native package '%s' already has been installed.\n", packageName);

                size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + packageNameLength + 2U;
                char   packageInstalledDIR[packageInstalledDIRCapacity];

                ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageName);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                setenv_fn funs[3] = { setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

                for (int i = 0; i < 3; i++) {
                    ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

                    if (ret != NDKPKG_OK) {
                        return ret;
                    }
                }

                return NDKPKG_OK;
            }
        } else {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", receiptFilePath);
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t workingDIRCapacity = packageWorkingTopDIRCapacity + packageNameLength + 14U;
    char   workingDIR[workingDIRCapacity];

    ret = snprintf(workingDIR, workingDIRCapacity, "%s/native-build-%s", packageWorkingTopDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir (workingDIR, S_IRWXU) != 0) {
        perror(workingDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        const char * remoteRef;

        if (formula->git_sha == NULL) {
            remoteRef = (formula->git_ref == NULL) ? "HEAD" : formula->git_ref;
        } else {
            remoteRef = formula->git_sha;
        }

        ret = ndkpkg_git_sync(workingDIR, formula->git_url, remoteRef, "refs/remotes/origin/master", "master", formula->git_nth);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    } else {
        if (formula->src_is_dir) {
            char * srcDIR = &formula->src_url[6];
            size_t srcDIRLength = strlen(srcDIR);

            size_t cmdCapacity = srcDIRLength + workingDIRCapacity + 10U;
            char   cmd[cmdCapacity];

            ret = snprintf(cmd, cmdCapacity, "cp -r %s/. %s", srcDIR, workingDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            ret = run_cmd(cmd, STDOUT_FILENO);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        } else {
            ret = download_via_http(formula->src_url, formula->src_uri, formula->src_sha, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, workingDIR, workingDIRCapacity, installOptions->verbose_net);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t cmdFilePathCapacity = workingDIRCapacity + 12U;
    char   cmdFilePath[cmdFilePathCapacity];

    ret = snprintf(cmdFilePath, cmdFilePathCapacity, "%s/do12345.sh", workingDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t do12345shFilePathCapacity = ndkpkgCoreDIRCapacity + 14U;
    char   do12345shFilePath[do12345shFilePathCapacity];

    ret = snprintf(do12345shFilePath, do12345shFilePathCapacity, "%s/ndkpkg-do12345", ndkpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstallDIRCapacity = nativePackageInstalledRootDIRCapacity + 66U;
    char   packageInstallDIR[packageInstallDIRCapacity];

    ret = snprintf(packageInstallDIR, packageInstallDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageInstalledSHA);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    int fd = open(cmdFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(cmdFilePath);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (installOptions->xtrace) {
        ret = dprintf(fd, "set -x\n");

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "STATIC_LIBRARY_SUFFIX='.a'\n");

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

#if defined (__APPLE__)
    const char * sharedLibrarySuffix = ".dylib";
#else
    const char * sharedLibrarySuffix = ".so";
#endif

    ret = dprintf(fd, "SHARED_LIBRARY_SUFFIX='%s'\n", sharedLibrarySuffix);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "PACKAGE_INSTALL_DIR='%s'\n", packageInstallDIR);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->bscript == NULL) {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s'\n", workingDIR);

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }

        ret = dprintf(fd, "PACKAGE_BCACHED_DIR='%s/_'\n", workingDIR);

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    } else {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s/%s'\n", workingDIR, formula->bscript);

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }

        ret = dprintf(fd, "PACKAGE_BCACHED_DIR='%s/%s/_'\n", workingDIR, formula->bscript);

        if (ret < 0) {
            close(fd);
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "\ndo12345() {\n%s\n}\n\n. %s\n. %s\n", formula->do12345, varsFilePath, do12345shFilePath);

    if (ret < 0) {
        close(fd);
        return NDKPKG_ERROR;
    }

    close(fd);

    //////////////////////////////////////////////////////////////////////////////

    size_t cmdCapacity = cmdFilePathCapacity + 10U;
    char   cmd[cmdCapacity];

    ret = snprintf(cmd, cmdCapacity, "/bin/sh %s", cmdFilePath);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = run_cmd(cmd, STDOUT_FILENO);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t receiptFilePath2Length = packageInstallDIRCapacity + 12U;
    char   receiptFilePath2[receiptFilePath2Length];

    ret = snprintf(receiptFilePath2, receiptFilePath2Length, "%s/receipt.txt", packageInstallDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = write_to_file(receiptFilePath2, formula->src_sha);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir (nativePackageInstalledRootDIR) != 0) {
        perror(nativePackageInstalledRootDIR);
        return NDKPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "native package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = ndkpkg_rm_r(packageName, verbose);

                        if (ret != NDKPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return NDKPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return NDKPKG_ERROR;
            }
        }
    }

    size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + packageNameLength + 2U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    setenv_fn funs[3] = { setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

    for (int i = 0; i < 3; i++) {
        ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    return NDKPKG_OK;
}

static int ndkpkg_install_package(
        const char * packageName,
        const size_t packageNameLength,
        const NDKPKGTargetPlatform * targetPlatform,
        const NDKPKGFormula * formula,
        const NDKPKGInstallOptions * installOptions,
        const NDKPKGToolChain * toolchainForNativeBuild,
        const NDKPKGAndroidNDKToolChain * toolchainForTargetBuild,
        const SysInfo * sysinfo,
        const time_t ts,
        const char * uppmPackageInstalledRootDIR,
        const size_t uppmPackageInstalledRootDIRLength,
        const char * ndkpkgExeFilePath,
        const char * ndkpkgHomeDIR,
        const size_t ndkpkgHomeDIRLength,
        const char * ndkpkgCoreDIR,
        const size_t ndkpkgCoreDIRCapacity,
        const char * ndkpkgDownloadsDIR,
        const size_t ndkpkgDownloadsDIRCapacity,
        const char * sessionDIR,
        const size_t sessionDIRLength,
        const char * recursiveDependentPackageNamesString,
        const size_t recursiveDependentPackageNamesStringSize) {
    fprintf(stderr, "%s=============== Installing%s %s%s%s %s===============%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, packageName, COLOR_OFF, COLOR_PURPLE, COLOR_OFF);

    size_t njobs;

    if (formula->parallel) {
        if (installOptions->parallelJobsCount > 0) {
            njobs = installOptions->parallelJobsCount;
        } else {
            njobs = sysinfo->ncpu;
        }
    } else {
        njobs = 1U;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t strBufSize = packageNameLength + 50U;
    char   strBuf[strBufSize];

    int ret = snprintf(strBuf, strBufSize, "%s:%zu:%u", packageName, ts, getpid());

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    char packageInstalledSHA[65] = {0};

    ret = sha256sum_of_string(packageInstalledSHA, strBuf);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * targetABI;

    switch (targetPlatform->abi) {
        case AndroidABI_arm64_v8a:
            targetABI = "arm64-v8a";
            break;
        case AndroidABI_armeabi_v7a:
            targetABI = "armeabi-v7a";
            break;
        case AndroidABI_x86_64:
            targetABI = "x86_64";
            break;
        case AndroidABI_x86:
            targetABI = "x86";
            break;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t targetPlatformSpecCapacity = 21U;
    char   targetPlatformSpec[targetPlatformSpecCapacity];

    ret = snprintf(targetPlatformSpec, targetPlatformSpecCapacity, "android-%2u-%s", targetPlatform->api, targetABI);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    //  /home/leleliu008/.ndk-pkg                                                                     <- ndkpkgHomeDIR
    //  /home/leleliu008/.ndk-pkg/run/<pid>                                                           <- sessionDIR
    //  /home/leleliu008/.ndk-pkg/run/<pid>/<target-platform-spec>-<package-name>                     <- packageWorkingTopDIR
    //  /home/leleliu008/.ndk-pkg/run/<pid>/<target-platform-spec>-<package-name>/native-build-<PKG>  <- buildForNativeDIR
    //  /home/leleliu008/.ndk-pkg/run/<pid>/<target-platform-spec>-<package-name>/src                 <- packageWorkingSrcDIR
    //  /home/leleliu008/.ndk-pkg/run/<pid>/<target-platform-spec>-<package-name>/fix                 <- packageWorkingFixDIR

    size_t packageWorkingTopDIRCapacity = sessionDIRLength + targetPlatformSpecCapacity + packageNameLength + 2U;
    char   packageWorkingTopDIR[packageWorkingTopDIRCapacity];

    ret = snprintf(packageWorkingTopDIR, packageWorkingTopDIRCapacity, "%s/%s-%s", sessionDIR, targetPlatformSpec, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir (packageWorkingTopDIR, S_IRWXU) != 0) {
        perror(packageWorkingTopDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    const KV toolsForNativeBuild[17] = {
        { "CC",        toolchainForNativeBuild->cc },
        { "OBJC",      toolchainForNativeBuild->objc },
        { "CXX",       toolchainForNativeBuild->cxx },
        { "CPP",       toolchainForNativeBuild->cpp },
        { "AS",        toolchainForNativeBuild->as },
        { "AR",        toolchainForNativeBuild->ar },
        { "RANLIB",    toolchainForNativeBuild->ranlib },
        { "LD",        toolchainForNativeBuild->ld },
        { "NM",        toolchainForNativeBuild->nm },
        { "SIZE",      toolchainForNativeBuild->size },
        { "STRIP",     toolchainForNativeBuild->strip },
        { "STRINGS",   toolchainForNativeBuild->strings },
        { "OBJCOPY",   toolchainForNativeBuild->objcopy },
        { "OBJDUMP",   toolchainForNativeBuild->objdump },
        { "READELF",   toolchainForNativeBuild->readelf },
        { "ADDR2LINE", toolchainForNativeBuild->addr2line },
        { "SYSROOT",   toolchainForNativeBuild->sysroot }
    };

    for (int i = 0; i < 17; i++) {
        const char * name  = toolsForNativeBuild[i].name;
        const char * value = toolsForNativeBuild[i].value;

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }

            size_t name2Length = strlen(name) + 11U;
            char   name2[name2Length];

            ret = snprintf(name2, name2Length, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (unsetenv(name2) != 0) {
                perror(name2);
                return NDKPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }

            size_t name2Length = strlen(name) + 11U;
            char   name2[name2Length];

            ret = snprintf(name2, name2Length, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (setenv(name2, value, 1) != 0) {
                perror(name2);
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    const KV flagsForNativeBuild[8] = {
        { "CFLAGS",             toolchainForNativeBuild->ccflags  },
        { "CXXFLAGS",           toolchainForNativeBuild->cxxflags },
        { "CPPFLAGS",           toolchainForNativeBuild->cppflags },
        { "LDFLAGS",            toolchainForNativeBuild->ldflags  },

        { "CFLAGS_FOR_BUILD",   toolchainForNativeBuild->ccflags  },
        { "CXXFLAGS_FOR_BUILD", toolchainForNativeBuild->cxxflags },
        { "CPPFLAGS_FOR_BUILD", toolchainForNativeBuild->cppflags },
        { "LDFLAGS_FOR_BUILD",  toolchainForNativeBuild->ldflags  }
    };

    for (int i = 0; i < 8; i++) {
        const char * name  = flagsForNativeBuild[i].name;
        const char * value = flagsForNativeBuild[i].value;

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    const char* unsetenvs[5] = {
        "LIBS",
        "PKG_CONFIG_LIBDIR",
        "PKG_CONFIG_PATH",
        "ACLOCAL_PATH",
        "XDG_DATA_DIRS"
    };

    for (int i = 0; i < 5; i++) {
        if (unsetenv(unsetenvs[i]) != 0) {
            perror(unsetenvs[i]);
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = setenv_ACLOCAL_PATH(ndkpkgCoreDIR, ndkpkgCoreDIRCapacity);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ret = setenv_XDG_DATA_DIRS(ndkpkgCoreDIR, ndkpkgCoreDIRCapacity);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    // these packages are not relocatable, we need to build them from source locally.
    bool needToBuildRuby     = false;
    bool needToBuildPerl     = false;
    bool needToBuildPython3  = false;
    bool needToBuildLibtool  = false;
    bool needToBuildAutomake = false;
    bool needToBuildAutoconf = false;
    bool needToBuildTexinfo  = false;
    bool needToBuildHelp2man = false;
    bool needToBuildIntltool = false;
    bool needToBuildSwig     = false;

    size_t depPackageNamesLength = (formula->dep_upp == NULL) ? 0U : strlen(formula->dep_upp);

#if defined (__APPLE__)
    const bool isNativeOSDarwin = true;
#else
    const bool isNativeOSDarwin = false;
#endif

#if defined (__linux__)
    const bool isNativeOSLinux = true;
#else
    const bool isNativeOSLinux = false;
#endif

    size_t uppmPackageNamesCapacity = depPackageNamesLength + 100U;
    char   uppmPackageNames[uppmPackageNamesCapacity];

    ret = snprintf(uppmPackageNames, 75U, "bash coreutils findutils gsed gawk grep tree %s%s%s", (isNativeOSLinux || isNativeOSDarwin) ? "pkg-config" : "pkgconf", isNativeOSDarwin ? "" : " patchelf", installOptions->enableCcache ? " ccache" : "");

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t uppmPackageNamesLength = ret;

    bool needToInstallCmake = false;
    bool needToInstallGmake = false;
    bool needToInstallGm4   = false;

    if (formula->dep_upp != NULL) {
        size_t  depPackageNamesCopyCapacity = depPackageNamesLength + 1U;
        char    depPackageNamesCopy[depPackageNamesCopyCapacity];
        strncpy(depPackageNamesCopy, formula->dep_upp, depPackageNamesCopyCapacity);

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            if (strcmp(depPackageName, "ruby") == 0) {
                needToBuildRuby = true;
                needToInstallGmake = true;
            } else if (strcmp(depPackageName, "perl") == 0) {
                needToBuildPerl = true;
                needToInstallGmake = true;
            } else if (strcmp(depPackageName, "python3") == 0) {
                needToBuildPython3 = true;
                needToInstallGmake = true;
                needToInstallCmake = true;
            } else if (strcmp(depPackageName, "texinfo") == 0) {
                needToBuildTexinfo = true;
                needToInstallGmake = true;
            } else if (strcmp(depPackageName, "help2man") == 0) {
                needToBuildHelp2man = true;
                needToInstallGmake = true;
            } else if (strcmp(depPackageName, "intltool") == 0) {
                needToBuildIntltool = true;
                needToInstallGmake = true;
                needToInstallCmake = true;
            } else if (strcmp(depPackageName, "libtool") == 0) {
                needToBuildLibtool = true;
                needToInstallGmake = true;
                needToInstallGm4   = true;
            } else if (strcmp(depPackageName, "autoconf") == 0) {
                needToBuildAutoconf = true;
                needToInstallGmake = true;
                needToInstallGm4   = true;
            } else if (strcmp(depPackageName, "automake") == 0) {
                needToBuildAutomake = true;
                needToInstallGmake = true;
                needToInstallGm4   = true;
            } else if (strcmp(depPackageName, "cmake") == 0) {
                needToInstallCmake = true;
            } else if (strcmp(depPackageName, "gmake") == 0) {
                needToInstallGmake = true;
            } else if (strcmp(depPackageName, "swig") == 0) {
                needToBuildSwig    = true;
                needToInstallGmake = true;
                needToInstallCmake = true;
            } else {
                int len = snprintf(uppmPackageNames + uppmPackageNamesLength, strlen(depPackageName) + 2U, " %s", depPackageName);

                if (len < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                uppmPackageNamesLength += len;
            }

            depPackageName = strtok(NULL, " ");
        }

#ifndef __NetBSD__
        if (needToInstallCmake) {
            strncpy(uppmPackageNames + uppmPackageNamesLength, " cmake", 6U);
            uppmPackageNamesLength += 6U;
        }
#endif

        if (needToInstallGmake) {
            strncpy(uppmPackageNames + uppmPackageNamesLength, " gmake", 6U);
            uppmPackageNamesLength += 6U;
        }

        if (needToInstallGm4) {
            strncpy(uppmPackageNames + uppmPackageNamesLength, " gm4", 4U);
            uppmPackageNamesLength += 4U;
        }

        uppmPackageNames[uppmPackageNamesLength] = '\0';
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = install_dependent_packages_via_uppm(uppmPackageNames, uppmPackageNamesLength, ndkpkgCoreDIR, ndkpkgCoreDIRCapacity, uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRLength, needToInstallCmake);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char m4Path[PATH_MAX];

    ret = exe_where("m4", m4Path, PATH_MAX);

    switch (ret) {
        case -3:
            return NDKPKG_ERROR_ENV_PATH_NOT_SET;
        case -2:
            return NDKPKG_ERROR_ENV_PATH_NOT_SET;
        case -1:
            perror(NULL);
            return NDKPKG_ERROR;
    }

    if (ret > 0) {
        if (setenv("M4", m4Path, 1) != 0) {
            perror("M4");
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    int nativePackageIDArray[22] = {0};
    int nativePackageIDArraySize = 0;

    if (needToBuildPerl) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_PERL;
        nativePackageIDArraySize++;
    }

    if (needToBuildLibtool) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_LIBTOOL;
        nativePackageIDArraySize++;
    }

    if (needToBuildAutoconf) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_AUTOCONF;
        nativePackageIDArraySize++;
    }

    if (needToBuildAutomake) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_AUTOMAKE;
        nativePackageIDArraySize++;
    }

    if (needToBuildTexinfo) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_TEXINFO;
        nativePackageIDArraySize++;
    }

    if (needToBuildHelp2man) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_HELP2MAN;
        nativePackageIDArraySize++;
    }

    if (needToBuildPython3) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_PYTHON3;
        nativePackageIDArraySize++;
    }

    if (needToBuildRuby) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_RUBY;
        nativePackageIDArraySize++;
    }

    if (needToBuildIntltool) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_INTLTOOL;
        nativePackageIDArraySize++;
    }

    if (needToBuildSwig) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_SWIG;
        nativePackageIDArraySize++;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t nativePackageInstalledRootDIRCapacity = ndkpkgHomeDIRLength + 8U;
    char   nativePackageInstalledRootDIR[nativePackageInstalledRootDIRCapacity];

    ret = snprintf(nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, "%s/native", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < nativePackageIDArraySize; i++) {
        for (int i = 0; i < 8; i++) {
            const char * name  = flagsForNativeBuild[i].name;
            const char * value = flagsForNativeBuild[i].value;

            if (value == NULL) {
                if (unsetenv(name) != 0) {
                    perror(name);
                    return NDKPKG_ERROR;
                }
            } else {
                if (setenv(name, value, 1) != 0) {
                    perror(name);
                    return NDKPKG_ERROR;
                }
            }
        }

        ret = install_native_package(nativePackageIDArray[i], ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, packageWorkingTopDIR, packageWorkingTopDIRCapacity, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, njobs, installOptions);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_pym != NULL) {
        size_t pipUpgradeCmdCapacity = nativePackageInstalledRootDIRCapacity + 57U;
        char   pipUpgradeCmd[pipUpgradeCmdCapacity];

        ret = snprintf(pipUpgradeCmd, pipUpgradeCmdCapacity, "%s/python3/bin/pip3 install --upgrade pip setuptools wheel", nativePackageInstalledRootDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(pipUpgradeCmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }

        /////////////////////////////////////////////

        size_t pipInstallCmdCapacity = nativePackageInstalledRootDIRCapacity + strlen(formula->dep_pym) + 37U;
        char   pipInstallCmd[pipInstallCmdCapacity];

        ret = snprintf(pipInstallCmd, pipInstallCmdCapacity, "%s/python3/bin/pip3 install --upgrade %s", nativePackageInstalledRootDIR, formula->dep_pym);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(pipInstallCmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_plm != NULL) {
        size_t cpanInstallCmdLength = ndkpkgHomeDIRLength + strlen(formula->dep_plm) + 24U;
        char   cpanInstallCmd[cpanInstallCmdLength];

        ret = snprintf(cpanInstallCmd, cpanInstallCmdLength, "%s/native/perl/bin/cpan %s", ndkpkgHomeDIR, formula->dep_plm);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = run_cmd(cpanInstallCmd, STDOUT_FILENO);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCargo) {
        ret = setup_rust_toolchain(installOptions, sessionDIR, sessionDIRLength);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t varsFilePathCapacity = packageWorkingTopDIRCapacity + 12U;
    char   varsFilePath[varsFilePathCapacity];

    ret = snprintf(varsFilePath, varsFilePathCapacity, "%s/vars.sh", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = generate_shell_vars_file(varsFilePath, packageName, formula, installOptions, sysinfo, uppmPackageInstalledRootDIR, ndkpkgExeFilePath, ts, njobs, ndkpkgHomeDIR, ndkpkgCoreDIR, ndkpkgDownloadsDIR, sessionDIR, recursiveDependentPackageNamesString);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    const bool isCrossBuild = true;

    if (isCrossBuild) {
        if (formula->do12345 != NULL) {
            ret = ndkpkg_build_for_native(installOptions, formula, packageName, packageNameLength, ndkpkgCoreDIR, ndkpkgCoreDIRCapacity, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, packageWorkingTopDIR, packageWorkingTopDIRCapacity, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, packageInstalledSHA, varsFilePath, installOptions->verbose_net);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    ///                        below is for target                             ///
    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingSrcDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingSrcDIR[packageWorkingSrcDIRCapacity];

    ret = snprintf(packageWorkingSrcDIR, packageWorkingSrcDIRCapacity, "%s/src", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir(packageWorkingSrcDIR, S_IRWXU) != 0) {
        perror(packageWorkingSrcDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingFixDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingFixDIR[packageWorkingFixDIRCapacity];

    ret = snprintf(packageWorkingFixDIR, packageWorkingFixDIRCapacity, "%s/fix", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir(packageWorkingFixDIR, S_IRWXU) != 0) {
        perror(packageWorkingFixDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingResDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingResDIR[packageWorkingResDIRCapacity];

    ret = snprintf(packageWorkingResDIR, packageWorkingResDIRCapacity, "%s/res", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir(packageWorkingResDIR, S_IRWXU) != 0) {
        perror(packageWorkingResDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingBinDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingBinDIR[packageWorkingBinDIRCapacity];

    ret = snprintf(packageWorkingBinDIR, packageWorkingBinDIRCapacity, "%s/bin", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir(packageWorkingBinDIR, S_IRWXU) != 0) {
        perror(packageWorkingBinDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingIncDIRCapacity = packageWorkingTopDIRCapacity + 9U;
    char   packageWorkingIncDIR[packageWorkingIncDIRCapacity];

    ret = snprintf(packageWorkingIncDIR, packageWorkingIncDIRCapacity, "%s/include", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir(packageWorkingIncDIR, S_IRWXU) != 0) {
        perror(packageWorkingIncDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingLibDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingLibDIR[packageWorkingLibDIRCapacity];

    ret = snprintf(packageWorkingLibDIR, packageWorkingLibDIRCapacity, "%s/lib", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir(packageWorkingLibDIR, S_IRWXU) != 0) {
        perror(packageWorkingLibDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingTmpDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingTmpDIR[packageWorkingTmpDIRCapacity];

    ret = snprintf(packageWorkingTmpDIR, packageWorkingTmpDIRCapacity, "%s/tmp", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (mkdir(packageWorkingTmpDIR, S_IRWXU) != 0) {
        perror(packageWorkingTmpDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        const char * remoteRef;

        if (formula->git_sha == NULL) {
            remoteRef = (formula->git_ref == NULL) ? "HEAD" : formula->git_ref;
        } else {
            remoteRef = formula->git_sha;
        }

        ret = ndkpkg_git_sync(packageWorkingSrcDIR, formula->git_url, remoteRef, "refs/remotes/origin/master", "master", formula->git_nth);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    } else {
        if (formula->src_is_dir) {
            char * srcDIR = &formula->src_url[6];
            size_t srcDIRLength = strlen(srcDIR);

            size_t cmdCapacity = srcDIRLength + packageWorkingSrcDIRCapacity + 10U;
            char   cmd[cmdCapacity];

            ret = snprintf(cmd, cmdCapacity, "cp -r %s/. %s", srcDIR, packageWorkingSrcDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            ret = run_cmd(cmd, STDOUT_FILENO);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        } else {
            ret = download_via_http(formula->src_url, formula->src_uri, formula->src_sha, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, packageWorkingSrcDIR, packageWorkingSrcDIRCapacity, installOptions->verbose_net);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        }
    }

    if (formula->fix_url != NULL) {
        ret = download_via_http(formula->fix_url, formula->fix_uri, formula->fix_sha, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, packageWorkingFixDIR, packageWorkingFixDIRCapacity, installOptions->verbose_net);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    if (formula->res_url != NULL) {
        ret = download_via_http(formula->res_url, formula->res_uri, formula->res_sha, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, packageWorkingResDIR, packageWorkingResDIRCapacity, installOptions->verbose_net);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRootDIRCapacity = ndkpkgHomeDIRLength + targetPlatformSpecCapacity + 11U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed/%s", ndkpkgHomeDIR, targetPlatformSpec);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    for (int i = 4; i < 8; i++) {
        const char * name  = flagsForNativeBuild[i].name;
        const char * value = flagsForNativeBuild[i].value;

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////

    if (formula->ccflags == NULL) {
        if (setenv("CFLAGS", toolchainForTargetBuild->ccflags, 1) != 0) {
            perror("CFLAGS");
            return NDKPKG_ERROR;
        }
    } else {
        size_t ccflagsCapacity = strlen(toolchainForTargetBuild->ccflags) + strlen(formula->ccflags) + 2U;
        char   ccflags[ccflagsCapacity];

        ret = snprintf(ccflags, ccflagsCapacity, "%s %s", toolchainForTargetBuild->ccflags, formula->ccflags);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (setenv("CFLAGS", ccflags, 1) != 0) {
            perror("CFLAGS");
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->xxflags == NULL) {
        if (setenv("CXXFLAGS", toolchainForTargetBuild->cxxflags, 1) != 0) {
            perror("CXXFLAGS");
            return NDKPKG_ERROR;
        }
    } else {
        size_t cxxflagsCapacity = strlen(toolchainForTargetBuild->cxxflags) + strlen(formula->xxflags) + 2U;
        char   cxxflags[cxxflagsCapacity];

        ret = snprintf(cxxflags, cxxflagsCapacity, "%s %s", toolchainForTargetBuild->cxxflags, formula->xxflags);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (setenv("CXXFLAGS", cxxflags, 1) != 0) {
            perror("CXXFLAGS");
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->ppflags == NULL) {
        size_t cppflagsCapacity = packageWorkingIncDIRCapacity + 3U;
        char   cppflags[cppflagsCapacity];

        ret = snprintf(cppflags, cppflagsCapacity, "-I%s", packageWorkingIncDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (setenv("CPPFLAGS", cppflags, 1) != 0) {
            perror("CPPFLAGS");
            return NDKPKG_ERROR;
        }
    } else {
        size_t cppflagsCapacity = packageWorkingIncDIRCapacity + strlen(formula->ppflags) + 4U;
        char   cppflags[cppflagsCapacity];

        ret = snprintf(cppflags, cppflagsCapacity, "-I%s %s", packageWorkingIncDIR, formula->ppflags);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (setenv("CPPFLAGS", cppflags, 1) != 0) {
            perror("CPPFLAGS");
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    bool needAddStaticFlag = false;

    if (installOptions->linkType == NDKPKGLinkType_static_full) {
        if (formula->sfslink) {
            needAddStaticFlag = true;
        } else {
            fprintf(stderr, "user request to create fully statically linked executable, but package '%s' DO NOT support it, so we will downgrade to mostly statically linked executable.\n", packageName);
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t ldflagsCapacity = strlen(toolchainForTargetBuild->ldflags) + packageWorkingLibDIRCapacity + packageInstalledRootDIRCapacity + packageNameLength + (formula->ldflags == NULL ? 0U : strlen(formula->ldflags)) + 80U;
    char   ldflags[ldflagsCapacity];

    // both --static and -static flag should be given.
    //  -static flag will be filtered out by libtool, libtool recognize this flag as prefer to link static library.
    // --static flag will be passed to the linker, although this flag was not documented, but it indeed works.

    ret = snprintf(ldflags, ldflagsCapacity, "%s%s -L%s -Wl,-rpath,%s/%s/lib %s", needAddStaticFlag ? "--static -static " : "", toolchainForTargetBuild->ldflags, packageWorkingLibDIR, packageInstalledRootDIR, packageName, formula->ldflags == NULL ? "" : formula->ldflags);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (setenv("LDFLAGS", ldflags, 1) != 0) {
        perror("LDFLAGS");
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * libSuffix;

    if (installOptions->linkType == NDKPKGLinkType_static_full || installOptions->linkType == NDKPKGLinkType_static_most) {
        libSuffix = ".a";
    } else {
        libSuffix = ".so";
    }

    const size_t libSuffixLength = strlen(libSuffix);

    //////////////////////////////////////////////////////////////////////////////

    if (recursiveDependentPackageNamesString != NULL) {
        size_t  recursiveDependentPackageNamesStringCopyCapacity = recursiveDependentPackageNamesStringSize + 1U;
        char    recursiveDependentPackageNamesStringCopy[recursiveDependentPackageNamesStringCopyCapacity];
        strncpy(recursiveDependentPackageNamesStringCopy, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringCopyCapacity);

        char * dependentPackageName = strtok(recursiveDependentPackageNamesStringCopy, " ");

        while (dependentPackageName != NULL) {
            size_t depPkgInstalledDIRCapacity = packageInstalledRootDIRCapacity + strlen(dependentPackageName) + 1U;
            char   depPkgInstalledDIR[depPkgInstalledDIRCapacity];

            ret = snprintf(depPkgInstalledDIR, depPkgInstalledDIRCapacity, "%s/%s", packageInstalledRootDIR, dependentPackageName);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            setenv_fn fns[5] = { setenv_CPPFLAGS, setenv_LDFLAGS, setenv_PKG_CONFIG_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

            for (int i = 0; i < 5; i++) {
                ret = fns[i](depPkgInstalledDIR, depPkgInstalledDIRCapacity);

                if (ret != NDKPKG_OK) {
                    return ret;
                }
            }

            ret = copy_dependent_libraries(depPkgInstalledDIR, depPkgInstalledDIRCapacity, packageWorkingLibDIR, packageWorkingLibDIRCapacity, libSuffix, libSuffixLength);

            if (ret != NDKPKG_OK) {
                return ret;
            }

            if (isCrossBuild) {
                size_t nativePkgInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + strlen(dependentPackageName) + 1U;
                char   nativePkgInstalledDIR[nativePkgInstalledDIRCapacity];

                ret = snprintf(nativePkgInstalledDIR, nativePkgInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, dependentPackageName);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                ret = setenv_PATH(nativePkgInstalledDIR, nativePkgInstalledDIRCapacity);

                if (ret != NDKPKG_OK) {
                    return ret;
                }
            } else {
                ret = setenv_PATH(depPkgInstalledDIR, depPkgInstalledDIRCapacity);

                if (ret != NDKPKG_OK) {
                    return ret;
                }
            }

            dependentPackageName = strtok(NULL, " ");
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCmake) {
        // https://cmake.org/cmake/help/latest/envvar/CMAKE_GENERATOR.html
        if (setenv("CMAKE_GENERATOR", formula->useBuildSystemNinja ? "Ninja" : "Unix Makefiles", 1) != 0) {
            perror("CMAKE_GENERATOR");
            return NDKPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_BUILD_TYPE.html
        if (setenv("CMAKE_BUILD_TYPE", installOptions->buildType == NDKPKGBuildType_release ? "Release" : "Debug", 1) != 0) {
            perror("CMAKE_BUILD_TYPE");
            return NDKPKG_ERROR;
        }

        char ns[4];

        ret = snprintf(ns, 4, "%zu", njobs);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_BUILD_PARALLEL_LEVEL.html
        if (setenv("CMAKE_BUILD_PARALLEL_LEVEL", ns, 1) != 0) {
            perror("CMAKE_BUILD_PARALLEL_LEVEL");
            return NDKPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_EXPORT_COMPILE_COMMANDS.html
        if (setenv("CMAKE_EXPORT_COMPILE_COMMANDS", installOptions->exportCompileCommandsJson ? "ON" : "OFF", 1) != 0) {
            perror("CMAKE_EXPORT_COMPILE_COMMANDS");
            return NDKPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html#manual:cmake-env-variables(7)

        const char* cmakeenvs[] = {
            "CMAKE_PREFIX_PATH",
            "CMAKE_APPLE_SILICON_PROCESSOR",
            "CMAKE_BUILD_TYPE",
            "CMAKE_CONFIGURATION_TYPES",
            "CMAKE_CONFIG_TYPE",
            "CMAKE_GENERATOR_INSTANCE",
            "CMAKE_GENERATOR_PLATFORM",
            "CMAKE_GENERATOR_TOOLSET",
            "CMAKE_INSTALL_MODE",
            "CMAKE_C_COMPILER_LAUNCHER",
            "CMAKE_C_LINKER_LAUNCHER",
            "CMAKE_CXX_COMPILER_LAUNCHER",
            "CMAKE_CXX_LINKER_LAUNCHER",
            "CMAKE_MSVCIDE_RUN_PATH",
            "CMAKE_NO_VERBOSE",
            "CMAKE_OSX_ARCHITECTURES",
            "CMAKE_TOOLCHAIN_FILE",
            "DESTDIR",
            "CTEST_INTERACTIVE_DEBUG_MODE",
            "CTEST_OUTPUT_ON_FAILURE",
            "CTEST_PARALLEL_LEVEL",
            "CTEST_PROGRESS_OUTPUT",
            "CTEST_USE_LAUNCHERS_DEFAULT",
            "DASHBOARD_TEST_FROM_CTEST",
            NULL
        };

        for (int i = 0; ; i++) {
            const char * name = cmakeenvs[i];

            if (name == NULL) {
                break;
            }

            if (unsetenv(name) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemGolang) {
        // https://pkg.go.dev/cmd/cgo
        KV goenvs[6] = {
            { "GO111MODULE",  "auto" },
            { "CGO_ENABLED",  "0" },
            { "CGO_CFLAGS",   getenv("CFLAGS") },
            { "CGO_CXXFLAGS", getenv("CXXFLAGS") },
            { "CGO_CPPFLAGS", getenv("CPPFLAGS") },
            { "CGO_LDFLAGS",  getenv("LDFLAGS") },
        };

        for (int i = 0; i < 6; i++) {
            const char * name  = goenvs[i].name;
            const char * value = goenvs[i].value;

            if (value == NULL) {
                value = "";
            }

            if (setenv(name, value, 1) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }
        }

        // https://golang.org/doc/install/source#environment

        if (isCrossBuild) {
            if (setenv("GOOS", "android", 1) != 0) {
                perror("GOOS");
                return NDKPKG_ERROR;
            }

            const char * goarch;

            switch (targetPlatform->abi) {
                case AndroidABI_arm64_v8a:
                    goarch = "arm64";
                    break;
                case AndroidABI_armeabi_v7a:
                    goarch = "arm";
                    break;
                case AndroidABI_x86_64:
                    goarch = "amd64";
                    break;
                case AndroidABI_x86:
                    goarch = "386";
                    break;
            }

            if (setenv("GOARCH", goarch, 1) != 0) {
                perror("GOARCH");
                return NDKPKG_ERROR;
            }
        } else {
            if (unsetenv("GOOS") != 0) {
                perror("GOOS");
                return NDKPKG_ERROR;
            }

            if (unsetenv("GOARCH") != 0) {
                perror("GOARCH");
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCargo) {
        // https://docs.rs/backtrace/latest/backtrace/
        if (setenv("RUST_BACKTRACE", "1", 1) != 0) {
            perror("RUST_BACKTRACE");
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        char ns[4];

        ret = snprintf(ns, 4, "%zu", njobs);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        // https://doc.rust-lang.org/cargo/reference/environment-variables.html
        if (setenv("CARGO_BUILD_JOBS", ns, 1) != 0) {
            perror("CARGO_BUILD_JOBS");
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        const char * rustTarget;

        switch (targetPlatform->abi) {
            case AndroidABI_arm64_v8a:
                rustTarget = "aarch64-linux-android";
                break;
            case AndroidABI_armeabi_v7a:
                rustTarget = "armv7-linux-androideabi";
                break;
            case AndroidABI_x86_64:
                rustTarget = "x86_64-linux-android";
                break;
            case AndroidABI_x86:
                rustTarget = "i686-linux-android";
                break;
        }

        if (setenv("RUST_TARGET", rustTarget, 1) != 0) {
            perror("RUST_TARGET");
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        char p[24] = {0};

        strncpy(p, rustTarget, strlen(rustTarget));

        size_t i = 0U;

        for (;;) {
            if (p[i] == '\0') {
                break;
            }

            if (p[i] == '-') {
                p[i] = '_';
            }

            if (p[i] >= 'a' && p[i] <= 'z') {
                p[i] -= 32;
            }

            i++;
        }

        // https://doc.rust-lang.org/cargo/reference/environment-variables.html
        // https://doc.rust-lang.org/cargo/reference/config.html#targettriplelinker
        size_t linkerLength = i + 21U;
        char   linker[linkerLength];

        ret = snprintf(linker, linkerLength, "CARGO_TARGET_%s_LINKER", p);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (setenv(linker, getenv("CC"), 1) != 0) {
            perror(linker);
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        // https://doc.rust-lang.org/cargo/reference/config.html#buildrustflags
        // we want to use RUSTFLAGS
        if (unsetenv("CARGO_ENCODED_RUSTFLAGS") != 0) {
            perror("CARGO_ENCODED_RUSTFLAGS");
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        size_t rustFlagsCapacity = strlen(getenv("CC")) + 10U;
        char   rustFlags[rustFlagsCapacity];

        ret = snprintf(rustFlags, rustFlagsCapacity, "-Clinker=%s", getenv("CC"));

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (setenv("RUSTFLAGS", rustFlags, 1) != 0) {
            perror("RUSTFLAGS");
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        const char * LDFLAGS = getenv("LDFLAGS");

        if (LDFLAGS != NULL && LDFLAGS[0] != '\0') {
            size_t  ldflagsCopyCapacity = strlen(LDFLAGS) + 1U;
            char    ldflagsCopy[ldflagsCopyCapacity];
            strncpy(ldflagsCopy, LDFLAGS, ldflagsCopyCapacity);

            char * item = strtok(ldflagsCopy, " ");

            while (item != NULL) {
                const char * const RUSTFLAGS = getenv("RUSTFLAGS");

                size_t newRUSTFLAGSCapacity = strlen(RUSTFLAGS) + strlen(item) + 13U;
                char   newRUSTFLAGS[newRUSTFLAGSCapacity];

                ret = snprintf(newRUSTFLAGS, newRUSTFLAGSCapacity, "%s -Clink-arg=%s", RUSTFLAGS, item);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                // https://doc.rust-lang.org/rustc/codegen-options/index.html#link-arg
                if (setenv("RUSTFLAGS", newRUSTFLAGS, 1) != 0) {
                    perror("RUSTFLAGS");
                    return NDKPKG_ERROR;
                }

                item = strtok(NULL, " ");
            }
        }

        /////////////////////////////////////////

        // https://libraries.io/cargo/cc
        // https://crates.io/crates/cc
        // https://docs.rs/cc/latest/cc/
        // https://github.com/alexcrichton/cc-rs

        const char *    cflagsForTarget = getenv("CFLAGS");
        const char *  cxxflagsForTarget = getenv("CXXFLAGS");
        const char *  cppflagsForTarget = getenv("CPPFLAGS");

        if (cppflagsForTarget == NULL) {
            cppflagsForTarget = "";
        }

        const size_t cppflagsForTargetLength = strlen(cppflagsForTarget);

        /////////////////////////////////////////

        size_t CFLAGSForTargetCapacity = strlen(cflagsForTarget) + cppflagsForTargetLength + 2U;
        char   CFLAGSForTarget[CFLAGSForTargetCapacity];

        ret = snprintf(CFLAGSForTarget, CFLAGSForTargetCapacity, "%s %s", cflagsForTarget, cppflagsForTarget);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        size_t CXXFLAGSForTargetCapacity = strlen(cxxflagsForTarget) + cppflagsForTargetLength + 2U;
        char   CXXFLAGSForTarget[CXXFLAGSForTargetCapacity];

        ret = snprintf(CXXFLAGSForTarget, CXXFLAGSForTargetCapacity, "%s %s", cxxflagsForTarget, cppflagsForTarget);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        KV envsForTargetBuild[5] = {
            { "TARGET_CC",       getenv("CC") },
            { "TARGET_CXX",      getenv("CXX") },
            { "TARGET_AR",       getenv("AR") },
            { "TARGET_CFLAGS",   CFLAGSForTarget },
            { "TARGET_CXXFLAGS", CXXFLAGSForTarget }
        };

        for (int i = 0; i < 5; i++) {
            if (setenv(envsForTargetBuild[i].name, envsForTargetBuild[i].value, 1) != 0) {
                perror(envsForTargetBuild[i].name);
                return NDKPKG_ERROR;
            }
        }

        /////////////////////////////////////////

        const char *    cflagsForNative = getenv("CFLAGS_FOR_BUILD");
        const char *  cxxflagsForNative = getenv("CXXFLAGS_FOR_BUILD");
        const char *  cppflagsForNative = getenv("CPPFLAGS_FOR_BUILD");

        if (cppflagsForNative == NULL) {
            cppflagsForNative = "";
        }

        const size_t cppflagsForNativeLength = strlen(cppflagsForNative);

        /////////////////////////////////////////

        size_t CFLAGSForNativeCapacity = strlen(cflagsForNative) + cppflagsForNativeLength + 2U;
        char   CFLAGSForNative[CFLAGSForNativeCapacity];

        ret = snprintf(CFLAGSForNative, CFLAGSForNativeCapacity, "%s %s", cflagsForNative, cppflagsForNative);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        size_t CXXFLAGSForNativeCapacity = strlen(cxxflagsForNative) + cppflagsForNativeLength + 2U;
        char   CXXFLAGSForNative[CXXFLAGSForTargetCapacity];

        ret = snprintf(CXXFLAGSForNative, CXXFLAGSForNativeCapacity, "%s %s", cxxflagsForNative, cppflagsForNative);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        /////////////////////////////////////////

        KV envsForNativeBuild[5] = {
            { "HOST_CC",         toolchainForNativeBuild->cc },
            { "HOST_CXX",        toolchainForNativeBuild->cxx },
            { "HOST_AR",         toolchainForNativeBuild->ar },
            { "HOST_CFLAGS",     CFLAGSForNative },
            { "HOST_CXXFLAGS",   CXXFLAGSForNative },
        };

        for (int i = 0; i < 5; i++) {
            if (setenv(envsForNativeBuild[i].name, envsForNativeBuild[i].value, 1) != 0) {
                perror(envsForNativeBuild[i].name);
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    // override the default search directory (usually /usr/lib/pkgconfig:/usr/share/pkgconfig)
    // because we only want to use our own
    if (setenv("PKG_CONFIG_LIBDIR", packageWorkingLibDIR, 1) != 0) {
        perror("PKG_CONFIG_LIBDIR");
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledDIRCapacity = packageInstalledRootDIRCapacity + 66U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", packageInstalledRootDIR, packageInstalledSHA);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageMetaInfoDIRCapacity = packageInstalledDIRCapacity + 6U;
    char   packageMetaInfoDIR[packageMetaInfoDIRCapacity];

    ret = snprintf(packageMetaInfoDIR, packageMetaInfoDIRCapacity, "%s/.ndk-pkg", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t installshFilePathCapacity = packageWorkingBinDIRCapacity + 12U;
    char   installshFilePath[installshFilePathCapacity];

    ret = snprintf(installshFilePath, installshFilePathCapacity, "%s/install.sh", packageWorkingBinDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = generate_install_shell_script_file(installshFilePath, varsFilePath, packageName, formula, targetPlatform, installOptions, isCrossBuild, ndkpkgCoreDIR,  packageWorkingTopDIR, packageWorkingSrcDIR, packageWorkingFixDIR, packageWorkingResDIR, packageWorkingBinDIR, packageWorkingLibDIR, packageWorkingIncDIR, packageWorkingTmpDIR, packageInstalledRootDIR, packageInstalledRootDIRCapacity, nativePackageInstalledRootDIR, packageInstalledDIR, packageMetaInfoDIR, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringSize);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

#if defined (__NetBSD__)
    ret = generate_linker_script(packageWorkingLibDIR, packageWorkingLibDIRCapacity, "libdl.a");

    if (ret != NDKPKG_OK) {
        return ret;
    }
#elif defined (__OpenBSD__)
    // https://github.com/mesonbuild/meson/issues/5390

    ret = generate_linker_script(packageWorkingLibDIR, packageWorkingLibDIRCapacity, "libdl.a");

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ret = generate_linker_script(packageWorkingLibDIR, packageWorkingLibDIRCapacity, "librt.a");

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ret = generate_linker_script(packageWorkingLibDIR, packageWorkingLibDIRCapacity, "libcrypt.a");

    if (ret != NDKPKG_OK) {
        return ret;
    }
#endif

    //////////////////////////////////////////////////////////////////////////////

    if (installOptions->dryrun) {
        const char * const SHELL = getenv("SHELL");

        if (SHELL == NULL) {
            fprintf(stderr, "SHELL environment variable is not set.\n");
            return NDKPKG_ERROR;
        }

        if (SHELL[0] == '\0') {
            fprintf(stderr, "SHELL environment variable 's value should be a non-empty string.\n");
            return NDKPKG_ERROR;
        }

        execl(SHELL, SHELL, NULL);

        perror(SHELL);

        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t bashCmdCapacity = installshFilePathCapacity + 10U;
    char   bashCmd[bashCmdCapacity];

    ret = snprintf(bashCmd, bashCmdCapacity, "/bin/sh %s", installshFilePath);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = run_cmd(bashCmd, STDOUT_FILENO);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    struct stat st;

    if (stat(packageInstalledDIR, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "nothing is installed.\n");
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char* a[2] = { ".crates.toml", ".crates2.json" };

    for (int i = 0; i < 2; i++) {
        size_t filePathCapacity = packageInstalledDIRCapacity + strlen(a[i]) + 2U;
        char   filePath[filePathCapacity];

        ret = snprintf(filePath, filePathCapacity, "%s/%s", packageInstalledDIR, a[i]);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (stat(filePath, &st) == 0) {
            if (unlink(filePath) != 0) {
                perror(filePath);
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (mkdir (packageMetaInfoDIR, S_IRWXU) != 0) {
        perror(packageMetaInfoDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////
    // install dependency graph files

    if (formula->dep_pkg != NULL) {
        const char* item[4] = { "dependencies.dot", "dependencies.box", "dependencies.svg", "dependencies.png" };

        for (int i = 0; i < 4; i++) {
            ret = install_files_to_metainfo_dir(st, packageWorkingTopDIR, packageWorkingTopDIRCapacity, packageMetaInfoDIR, packageMetaInfoDIRCapacity, item[i], strlen(item[i]));

            if (ret != NDKPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install config.log

    size_t packageBuildScriptDIRBufCapacity = packageWorkingSrcDIRCapacity + (formula->bscript == NULL ? 0U : strlen(formula->bscript)) + 2U;
    char   packageBuildScriptDIRBuf[packageBuildScriptDIRBufCapacity];

    const char * packageBuildScriptDIR;

    if (formula->bscript == NULL) {
        packageBuildScriptDIR = packageWorkingSrcDIR;
    } else {
        ret = snprintf(packageBuildScriptDIRBuf, packageBuildScriptDIRBufCapacity, "%s/%s", packageWorkingSrcDIR, formula->bscript);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        packageBuildScriptDIR = packageBuildScriptDIRBuf;
    }

    const char* b[2] = { packageWorkingTmpDIR, packageBuildScriptDIR };

    for (int i = 0; i < 2; i++) {
        size_t fromFilePathCapacity = strlen(b[i]) + 12U;
        char   fromFilePath[fromFilePathCapacity];

        ret = snprintf(fromFilePath, fromFilePathCapacity, "%s/config.log", b[i]);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (stat(fromFilePath, &st) == 0) {
            size_t toFilePathCapacity = packageMetaInfoDIRCapacity + 12U;
            char   toFilePath[toFilePathCapacity];

            ret = snprintf(toFilePath, toFilePathCapacity, "%s/config.log", packageMetaInfoDIR);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            ret = ndkpkg_rename_or_copy_file(fromFilePath, toFilePath);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install compile_commands.json

    if (installOptions->exportCompileCommandsJson) {
        for (int i = 0; i < 2; i++) {
            size_t fromFilePathCapacity = strlen(b[i]) + 23U;
            char   fromFilePath[fromFilePathCapacity];

            ret = snprintf(fromFilePath, fromFilePathCapacity, "%s/compile_commands.json", b[i]);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (stat(fromFilePath, &st) == 0) {
                size_t toFilePathCapacity = packageMetaInfoDIRCapacity + 23U;
                char   toFilePath[toFilePathCapacity];

                ret = snprintf(toFilePath, toFilePathCapacity, "%s/compile_commands.json", packageMetaInfoDIR);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                ret = ndkpkg_rename_or_copy_file(fromFilePath, toFilePath);

                if (ret != NDKPKG_OK) {
                    return ret;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    
    const char* x[12] = { "AUTHORS", "LICENSE", "COPYING", "FAQ", "TODO", "NEWS", "THANKS", "CHANGELOG", "CHANGES", "README", "CONTRIBUTORS", "CONTRIBUTING" };

    const char* y[3] = { "", ".md", ".rst" };

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 3; j++) {
            size_t itemCapacity = strlen(x[i]) + strlen(y[j]) + 1U;
            char   item[itemCapacity];

            ret = snprintf(item, itemCapacity, "%s%s", x[i], y[j]);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            ret = install_files_to_metainfo_dir(st, packageWorkingSrcDIR, packageWorkingSrcDIRCapacity, packageMetaInfoDIR, packageMetaInfoDIRCapacity, item, itemCapacity);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        }
    }

    if (recursiveDependentPackageNamesString != NULL) {
        ret = backup_formulas(sessionDIR, packageMetaInfoDIR, packageMetaInfoDIRCapacity, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringSize);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    ret = generate_manifest(packageInstalledDIR);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ret = generate_receipt(packageName, formula, targetPlatformSpec, sysinfo, ts, packageMetaInfoDIR, packageMetaInfoDIRCapacity);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    size_t treeCmdCapacity = uppmPackageInstalledRootDIRLength + packageInstalledDIRCapacity + 30U;
    char   treeCmd[treeCmdCapacity];

    ret = snprintf(treeCmd, treeCmdCapacity, "%s/tree/bin/tree -a --dirsfirst %s", uppmPackageInstalledRootDIR, packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = run_cmd(treeCmd, STDOUT_FILENO);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    if (chdir (packageInstalledRootDIR) != 0) {
        perror(packageInstalledRootDIR);
        return NDKPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = ndkpkg_rm_r(packageName, installOptions->logLevel >= NDKPKGLogLevel_verbose);

                        if (ret != NDKPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return NDKPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return NDKPKG_ERROR;
            }
        }
    }

    if (installOptions->keepSessionDIR) {
        return NDKPKG_OK;
    } else {
        return ndkpkg_rm_r(packageWorkingTopDIR, installOptions->logLevel >= NDKPKGLogLevel_verbose);
    }
}

typedef struct {
    char * packageName;
    NDKPKGFormula * formula;
} NDKPKGPackage;

static int getRecursiveDependentPackageNamesStringBuffer(char * packageName, NDKPKGPackage ** packageSet, size_t packageSetSize, char ** precursiveDependentPackageNamesStringBuffer, size_t * precursiveDependentPackageNamesStringBufferSize, size_t * precursiveDependentPackageNamesStringBufferCapacity) {
    NDKPKGPackage * package = NULL;

    for (size_t i = 0U; i < packageSetSize; i++) {
        if (strcmp(packageSet[i]->packageName, packageName) == 0) {
            package = packageSet[i];
            break;
        }
    }

    NDKPKGFormula * formula = package->formula;

    if (formula->dep_pkg == NULL) {
        return NDKPKG_OK;
    }

    size_t   packageNameStackCapacity = 8U;
    size_t   packageNameStackSize    = 0U;
    char * * packageNameStack = (char**)malloc(8 * sizeof(char*));

    if (packageNameStack == NULL) {
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    size_t  depPackageNamesLength = strlen(formula->dep_pkg);

    size_t  depPackageNamesCopyCapacity = depPackageNamesLength + 1U;
    char    depPackageNamesCopy[depPackageNamesCopyCapacity];
    strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyCapacity);

    char * depPackageName = strtok(depPackageNamesCopy, " ");

    while (depPackageName != NULL) {
        if (packageNameStackSize == packageNameStackCapacity) {
            char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 8U) * sizeof(char*));

            if (p == NULL) {
                free(packageNameStack);
                packageNameStack = NULL;

                return NDKPKG_ERROR_MEMORY_ALLOCATE;
            }

            packageNameStack = p;
            packageNameStackCapacity += 8U;
        }

        packageNameStack[packageNameStackSize] = depPackageName;
        packageNameStackSize++;
        depPackageName = strtok(NULL, " ");
    }

    ////////////////////////////////////////////////////////////////

    char * recursiveDependentPackageNamesStringBuffer        = (*precursiveDependentPackageNamesStringBuffer);
    size_t recursiveDependentPackageNamesStringBufferSize    = (*precursiveDependentPackageNamesStringBufferSize);
    size_t recursiveDependentPackageNamesStringBufferCapacity = (*precursiveDependentPackageNamesStringBufferCapacity);

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize != 0U) {
        char * packageName = packageNameStack[packageNameStackSize - 1U];
        packageNameStack[packageNameStackSize - 1U] = NULL;
        packageNameStackSize--;

        ////////////////////////////////////////////////////////////////

        if (recursiveDependentPackageNamesStringBuffer != NULL) {
            bool alreadyInRecursiveDependentPackageNamesStringBuffer = false;

            size_t  recursiveDependentPackageNamesStringBufferCopyCapacity = recursiveDependentPackageNamesStringBufferSize + 1U;
            char    recursiveDependentPackageNamesStringBufferCopy[recursiveDependentPackageNamesStringBufferCopyCapacity];
            strncpy(recursiveDependentPackageNamesStringBufferCopy, recursiveDependentPackageNamesStringBuffer, recursiveDependentPackageNamesStringBufferCopyCapacity);

            char * dependentPackageName = strtok(recursiveDependentPackageNamesStringBufferCopy, " ");

            while (dependentPackageName != NULL) {
                if (strcmp(dependentPackageName, packageName) == 0) {
                    alreadyInRecursiveDependentPackageNamesStringBuffer = true;
                    break;
                }
                dependentPackageName = strtok(NULL, " ");
            }

            if (alreadyInRecursiveDependentPackageNamesStringBuffer) {
                continue;
            }
        }

        ////////////////////////////////////////////////////////////////

        size_t packageNameLength = strlen(packageName);

        if ((recursiveDependentPackageNamesStringBufferSize + packageNameLength + 2U) > recursiveDependentPackageNamesStringBufferCapacity) {
            char * p = (char*)realloc(recursiveDependentPackageNamesStringBuffer, (recursiveDependentPackageNamesStringBufferCapacity + 256U) * sizeof(char*));

            if (p == NULL) {
                free(recursiveDependentPackageNamesStringBuffer);
                recursiveDependentPackageNamesStringBuffer = NULL;

                free(packageNameStack);
                packageNameStack = NULL;

                return NDKPKG_ERROR_MEMORY_ALLOCATE;
            }

            recursiveDependentPackageNamesStringBuffer = p;
            recursiveDependentPackageNamesStringBufferCapacity += 256U;
        }

        if (recursiveDependentPackageNamesStringBufferSize == 0U) {
            strncpy(recursiveDependentPackageNamesStringBuffer, packageName, packageNameLength);
            recursiveDependentPackageNamesStringBufferSize = packageNameLength;
            recursiveDependentPackageNamesStringBuffer[recursiveDependentPackageNamesStringBufferSize] = '\0';
        } else {
            recursiveDependentPackageNamesStringBuffer[recursiveDependentPackageNamesStringBufferSize] = ' ';
            strncpy(recursiveDependentPackageNamesStringBuffer + recursiveDependentPackageNamesStringBufferSize + 1U, packageName, packageNameLength);
            recursiveDependentPackageNamesStringBufferSize += packageNameLength + 1U;
            recursiveDependentPackageNamesStringBuffer[recursiveDependentPackageNamesStringBufferSize] = '\0';
        }

        ////////////////////////////////////////////////////////////////

        NDKPKGPackage * package = NULL;

        for (size_t i = 0; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                package = packageSet[i];
                break;
            }
        }

        ////////////////////////////////////////////////////////////////

        NDKPKGFormula * formula = package->formula;

        if (formula->dep_pkg != NULL) {
            size_t  depPackageNamesLength = strlen(formula->dep_pkg);

            size_t  depPackageNamesCopyLength = depPackageNamesLength + 1U;
            char    depPackageNamesCopy[depPackageNamesCopyLength];
            strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

            char * depPackageName = strtok(depPackageNamesCopy, " ");

            while (depPackageName != NULL) {
                NDKPKGPackage * depPackage = NULL;

                for (size_t i = 0U; i < packageSetSize; i++) {
                    if (strcmp(packageSet[i]->packageName, depPackageName) == 0) {
                        depPackage = packageSet[i];
                        break;
                    }
                }

                ////////////////////////////////////////////////////////////////

                if (packageNameStackSize == packageNameStackCapacity) {
                    char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 8U) * sizeof(char*));

                    if (p == NULL) {
                        free(recursiveDependentPackageNamesStringBuffer);
                        recursiveDependentPackageNamesStringBuffer = NULL;

                        free(packageNameStack);
                        packageNameStack = NULL;

                        return NDKPKG_ERROR_MEMORY_ALLOCATE;
                    }

                    packageNameStack = p;
                    packageNameStackCapacity += 8U;
                }

                packageNameStack[packageNameStackSize] = depPackage->packageName;
                packageNameStackSize++;

                ////////////////////////////////////////////////////////////////

                depPackageName = strtok(NULL, " ");
            }
        }
    }

    free(packageNameStack);

    (*precursiveDependentPackageNamesStringBuffer)        = recursiveDependentPackageNamesStringBuffer;
    (*precursiveDependentPackageNamesStringBufferSize)    = recursiveDependentPackageNamesStringBufferSize;
    (*precursiveDependentPackageNamesStringBufferCapacity) = recursiveDependentPackageNamesStringBufferCapacity;

    return NDKPKG_OK;
}

static int check_and_read_formula_in_cache(const char * packageName, const char * sessionDIR, NDKPKGPackage *** ppackageSet, size_t * ppackageSetSize, size_t * ppackageSetCapacity) {
    size_t         packageSetCapacity = (*ppackageSetCapacity);
    size_t         packageSetSize    = (*ppackageSetSize);
    NDKPKGPackage ** packageSet        = (*ppackageSet);

    size_t   packageNameStackCapacity = 10U;
    size_t   packageNameStackSize    = 0U;
    char * * packageNameStack = (char**)malloc(10 * sizeof(char*));

    if (packageNameStack == NULL) {
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = strdup(packageName);

    if (packageNameStack[0] == NULL) {
        free(packageNameStack);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStackSize = 1U;

    int ret = NDKPKG_OK;

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize > 0U) {
        size_t topIndex = packageNameStackSize - 1U;
        char * packageName = packageNameStack[topIndex];
        packageNameStack[topIndex] = NULL;
        packageNameStackSize--;

        NDKPKGFormula * formula = NULL;

        for (size_t i = 0U; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                free(packageName);

                packageName = packageSet[i]->packageName;
                formula = packageSet[i]->formula;

                size_t lastIndex = packageSetSize - 1U;

                if (i != lastIndex) {
                    NDKPKGPackage * package = packageSet[i];

                    for (size_t j = i + 1U; j < packageSetSize; j++) {
                        packageSet[j - 1] = packageSet[j];
                    }

                    packageSet[lastIndex] = package;
                }

                break;
            }
        }

        if (formula == NULL) {
            char * formulaFilePath = NULL;

            ret = ndkpkg_formula_locate(packageName, &formulaFilePath);

            if (ret != NDKPKG_OK) {
                free(packageName);
                goto finalize;
            }

            size_t formulaFilePath2Length = strlen(sessionDIR) + strlen(packageName) + 6U;
            char   formulaFilePath2[formulaFilePath2Length];

            ret = snprintf(formulaFilePath2, formulaFilePath2Length, "%s/%s.yml", sessionDIR, packageName);

            if (ret < 0) {
                perror(NULL);
                free(packageName);
                return NDKPKG_ERROR;
            }

            ret = ndkpkg_copy_file(formulaFilePath, formulaFilePath2);

            free(formulaFilePath);

            if (ret != NDKPKG_OK) {
                free(packageName);
                goto finalize;
            }

            ret = ndkpkg_formula_parse(formulaFilePath2, &formula);

            if (ret != NDKPKG_OK) {
                free(packageName);
                goto finalize;
            }

            if (packageSetSize == packageSetCapacity) {
                NDKPKGPackage ** p = (NDKPKGPackage**)realloc(packageSet, (packageSetCapacity + 10U) * sizeof(NDKPKGPackage*));

                if (p == NULL) {
                    free(packageName);
                    ndkpkg_formula_free(formula);
                    ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageSet = p;
                packageSetCapacity += 10U;
            }

            NDKPKGPackage * package = (NDKPKGPackage*)malloc(sizeof(NDKPKGPackage));

            if (package == NULL) {
                free(packageName);
                ndkpkg_formula_free(formula);
                ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            package->formula = formula;
            package->packageName = packageName;

            packageSet[packageSetSize] = package;
            packageSetSize++;
        }

        if (formula->dep_pkg == NULL) {
            continue;
        }

        size_t  depPackageNamesLength = strlen(formula->dep_pkg);

        size_t  depPackageNamesCopyLength = depPackageNamesLength + 1U;
        char    depPackageNamesCopy[depPackageNamesCopyLength];
        strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            if (strcmp(depPackageName, packageName) == 0) {
                fprintf(stderr, "package '%s' depends itself.\n", packageName);
                ret = NDKPKG_ERROR;
                goto finalize;
            }

            ////////////////////////////////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapacity) {
                char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 10U) * sizeof(char*));

                if (p == NULL) {
                    ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageNameStack = p;
                packageNameStackCapacity += 10U;
            }

            char * p = strdup(depPackageName);

            if (p == NULL) {
                ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            packageNameStack[packageNameStackSize] = p;
            packageNameStackSize++;

            depPackageName = strtok (NULL, " ");
        }
    }

finalize:
    for (size_t i = 0U; i < packageNameStackSize; i++) {
        free(packageNameStack[i]);
        packageNameStack[i] = NULL;
    }

    free(packageNameStack);
    packageNameStack = NULL;

    if (ret == NDKPKG_OK) {
        (*ppackageSet) = packageSet;
        (*ppackageSetSize) = packageSetSize;
        (*ppackageSetCapacity) = packageSetCapacity;
    } else {
        for (size_t i = 0U; i < packageSetSize; i++) {
            free(packageSet[i]->packageName);
            ndkpkg_formula_free(packageSet[i]->formula);

            packageSet[i]->formula = NULL;
            packageSet[i]->packageName = NULL;

            free(packageSet[i]);
            packageSet[i] = NULL;
        }
    }

    return ret;
}

static int try_compile(const char * compilerFilePath, const char * compilerOption, const char * sourceFilePath) {
    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (pid == 0) {
        execl(compilerFilePath, compilerFilePath, compilerOption, sourceFilePath, NULL);
        perror(compilerFilePath);
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return NDKPKG_OK;
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s %s' exit with status code: %d\n", compilerFilePath, compilerOption, sourceFilePath, WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s %s' killed by signal: %d\n", compilerFilePath, compilerOption, sourceFilePath, WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s %s' stopped by signal: %d\n", compilerFilePath, compilerOption, sourceFilePath, WSTOPSIG(childProcessExitStatusCode));
        }

        return NDKPKG_ERROR;
    }
}

static int check_if_compiler_support_Wno_error_unused_command_line_argument(const char * sessionDIR, const size_t sessionDIRLength, const char * compiler, const bool iscc) {
    size_t testCFilePathCapacity = sessionDIRLength + 10U;
    char   testCFilePath[testCFilePathCapacity];

    int ret;

    if (iscc) {
        ret = snprintf(testCFilePath, testCFilePathCapacity, "%s/test.c",  sessionDIR);
    } else {
        ret = snprintf(testCFilePath, testCFilePathCapacity, "%s/test.cc", sessionDIR);
    }

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    int fd = open(testCFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(testCFilePath);
        return NDKPKG_ERROR;
    }

    const char * testCCode = "int main() {\nreturn 0;\n}\n";

    size_t testCCodeLength = strlen(testCCode);

    ssize_t written = write(fd, testCCode, testCCodeLength);

    if (written == -1) {
        perror(testCFilePath);
        close(fd);
        return NDKPKG_ERROR;
    }

    close(fd);

    if ((size_t)written != testCCodeLength) {
        fprintf(stderr, "file not fully written: %s\n", testCFilePath);
        return NDKPKG_ERROR;
    }

    return try_compile(compiler, "-Wno-error=unused-command-line-argument", testCFilePath);
}

int ndkpkg_setup_toolchain_for_native_build(
        NDKPKGToolChain * toolchainForNativeBuild,
        const char * sessionDIR,
        const size_t sessionDIRLength,
        const char * ndkpkgCoreDIR,
        const size_t ndkpkgCoreDIRLength,
        const NDKPKGInstallOptions * installOptions) {
    struct stat st;

    const size_t ccLength = strlen(toolchainForNativeBuild->cc);

    const char * compilerNames[3] = { "cc", "c++", "objc" };

    for (int i = 0; i < 3; i++) {
        const char * compilerName = compilerNames[i];

        size_t wrapperFilePathCapacity = ndkpkgCoreDIRLength + 21U;
        char   wrapperFilePath[wrapperFilePathCapacity];

        int ret = snprintf(wrapperFilePath, wrapperFilePathCapacity, "%s/wrapper-native-%s", ndkpkgCoreDIR, compilerName);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (stat(wrapperFilePath, &st) != 0) {
            size_t outputFilePathCapacity = sessionDIRLength + 23U;
            char   outputFilePath[outputFilePathCapacity];

            ret = snprintf(outputFilePath, outputFilePathCapacity, "%s/wrapper-native-%s", sessionDIR, compilerName);

            if (ret < 0) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if (toolchainForNativeBuild->sysroot == NULL) {
                size_t cmdLength = ccLength + outputFilePathCapacity + ndkpkgCoreDIRLength + 27U;
                char   cmd[cmdLength];

                ret = snprintf(cmd, cmdLength, "%s -o %s %s/wrapper-native-%s.c", toolchainForNativeBuild->cc, outputFilePath, ndkpkgCoreDIR, compilerName);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                ret = run_cmd(cmd, STDOUT_FILENO);

                if (ret != NDKPKG_OK) {
                    return ret;
                }
            } else {
                size_t cmdLength = ccLength + strlen(toolchainForNativeBuild->sysroot) + outputFilePathCapacity + ndkpkgCoreDIRLength + 38U;
                char   cmd[cmdLength];

                ret = snprintf(cmd, cmdLength, "%s --sysroot=%s -o %s %s/wrapper-native-%s.c", toolchainForNativeBuild->cc, toolchainForNativeBuild->sysroot, outputFilePath, ndkpkgCoreDIR, compilerName);

                if (ret < 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                ret = run_cmd(cmd, STDOUT_FILENO);

                if (ret != NDKPKG_OK) {
                    return ret;
                }
            }

            if (rename(outputFilePath, wrapperFilePath) != 0) {
                perror(wrapperFilePath);
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////

    const KV kvs[3] = {
        { "PROXIED_CC_FOR_BUILD",   toolchainForNativeBuild->cc   },
        { "PROXIED_CXX_FOR_BUILD",  toolchainForNativeBuild->cxx  },
        { "PROXIED_OBJC_FOR_BUILD", toolchainForNativeBuild->objc == NULL ? toolchainForNativeBuild->cc : toolchainForNativeBuild->objc }
    };

    for (int i = 0; i < 3; i++) {
        const char * name  = kvs[i].name;
        const char * value = kvs[i].value;

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    int ret1 = check_if_compiler_support_Wno_error_unused_command_line_argument(sessionDIR, sessionDIRLength, toolchainForNativeBuild->cc,  true);
    int ret2 = check_if_compiler_support_Wno_error_unused_command_line_argument(sessionDIR, sessionDIRLength, toolchainForNativeBuild->cxx, true);

    //////////////////////////////////////////////////////////////////////////////

    free(toolchainForNativeBuild->cc);
    free(toolchainForNativeBuild->cxx);
    free(toolchainForNativeBuild->objc);

    //////////////////////////////////////////////////////////////////////

    char * fields[3] = {NULL};

    for (int i = 0; i < 3; i++) {
        size_t capacity = ndkpkgCoreDIRLength + 21U;

        char * buf = (char*)malloc(capacity);

        if (buf == NULL) {
            perror(NULL);

            for (int i = 0; i < 3; i++) {
                free(fields[i]);
            }

            return NDKPKG_ERROR_MEMORY_ALLOCATE;
        }

        int ret = snprintf(buf, capacity, "%s/wrapper-native-%s", ndkpkgCoreDIR, compilerNames[i]);

        if (ret < 0) {
            perror(NULL);

            for (int i = 0; i < 3; i++) {
                free(fields[i]);
            }

            return NDKPKG_ERROR;
        }

        fields[i] = buf;
    }

    //////////////////////////////////////////////////////////////////////

    toolchainForNativeBuild->cc   = fields[0];
    toolchainForNativeBuild->cxx  = fields[1];
    toolchainForNativeBuild->objc = fields[2];

    //////////////////////////////////////////////////////////////////////

    size_t cppCapacity = strlen(toolchainForNativeBuild->cc) + 4U;

    char * cpp = (char*)malloc(cppCapacity);

    if (cpp == NULL) {
        perror(NULL);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    int ret = snprintf(cpp, cppCapacity, "%s -E", toolchainForNativeBuild->cc);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    toolchainForNativeBuild->cpp = cpp;

    //////////////////////////////////////////////////////////////////////////////

    size_t ccflagsCapacity = strlen(toolchainForNativeBuild->ccflags) + 60U;

    char * ccflags = (char*)malloc(ccflagsCapacity);

    if (ccflags == NULL) {
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    ret = snprintf(ccflags, ccflagsCapacity, "%s%s%s%s", toolchainForNativeBuild->ccflags, (installOptions->buildType == NDKPKGBuildType_release) ? " -Os" : " -g -O0", installOptions->verbose_cc ? " -v" : "", (ret1 == 0) ? " -Wno-error=unused-command-line-argument" : "");

    if (ret < 0) {
        perror(NULL);
        free(ccflags);
        return NDKPKG_ERROR;
    }

    free(toolchainForNativeBuild->ccflags);

    toolchainForNativeBuild->ccflags = ccflags;

    //////////////////////////////////////////////////////////////////////

    size_t cxxflagsCapacity = strlen(toolchainForNativeBuild->cxxflags) + 60U;

    char * cxxflags = (char*)malloc(cxxflagsCapacity);

    if (cxxflags == NULL) {
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    ret = snprintf(cxxflags, cxxflagsCapacity, "%s%s%s%s", toolchainForNativeBuild->cxxflags, (installOptions->buildType == NDKPKGBuildType_release) ? " -Os" : " -g -O0", installOptions->verbose_cc ? " -v" : "", (ret2 == 0) ? " -Wno-error=unused-command-line-argument" : "");

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    free(toolchainForNativeBuild->cxxflags);

    toolchainForNativeBuild->cxxflags = cxxflags;

    //////////////////////////////////////////////////////////////////////

    if (installOptions->buildType == NDKPKGBuildType_release || installOptions->verbose_ld) {
        size_t ldflagsCapacity = strlen(toolchainForNativeBuild->ldflags) + 60U;

        char * ldflags = (char*)malloc(ldflagsCapacity);

        if (ldflags == NULL) {
            return NDKPKG_ERROR_MEMORY_ALLOCATE;
        }

        ret = snprintf(ldflags, ldflagsCapacity, "%s%s%s", toolchainForNativeBuild->ldflags, (installOptions->buildType == NDKPKGBuildType_release) ? " -Wl,-s" : "", installOptions->verbose_ld ? " -Wl,-v" : "");

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        free(toolchainForNativeBuild->ldflags);

        toolchainForNativeBuild->ldflags = ldflags;
    }

    //////////////////////////////////////////////////////////////////////////////

#if defined (__APPLE__)
    // https://keith.github.io/xcode-man-pages/xcrun.1.html
    if (setenv("SDKROOT", toolchainForNativeBuild->sysroot, 1) != 0) {
        perror("SDKROOT");
        return NDKPKG_ERROR;
    }

    // https://clang.llvm.org/docs/CommandGuide/clang.html#envvar-MACOSX_DEPLOYMENT_TARGET
    // https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-mmacosx-version-min
    // If -mmacosx-version-min is unspecified, the default deployment target is read from MACOSX_DEPLOYMENT_TARGET environment variable
    if (unsetenv("MACOSX_DEPLOYMENT_TARGET") != 0) {
        perror("MACOSX_DEPLOYMENT_TARGET");
        return NDKPKG_ERROR;
    }
#endif

    return NDKPKG_OK;
}

int ndkpkg_install(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, const NDKPKGInstallOptions * installOptions) {
    // redirect all stdout and stderr to /dev/null
    if (installOptions->logLevel == NDKPKGLogLevel_silent) {
        int fd = open("/dev/null", O_CREAT | O_TRUNC | O_WRONLY, 0666);

        if (fd < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror(NULL);
            close(fd);
            return NDKPKG_ERROR;
        }

        if (dup2(fd, STDERR_FILENO) < 0) {
            perror(NULL);
            close(fd);
            return NDKPKG_ERROR;
        }
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL || PATH[0] == '\0') {
        return NDKPKG_ERROR_ENV_PATH_NOT_SET;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * NDKPKG_URL_TRANSFORM = getenv("NDKPKG_URL_TRANSFORM");

    if (NDKPKG_URL_TRANSFORM != NULL && NDKPKG_URL_TRANSFORM[0] != '\0') {
        if (setenv("UPPM_URL_TRANSFORM", NDKPKG_URL_TRANSFORM, 1) != 0) {
            perror("UPPM_URL_TRANSFORM");
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    // https://perldoc.perl.org/cpan#PERL_MM_USE_DEFAULT
    // Would you like to configure as much as possible automatically?
    if (setenv("PERL_MM_USE_DEFAULT", "1", 1) != 0) {
        perror("PERL_MM_USE_DEFAULT");
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * unsetenvs[12] = { "TARGET_ARCH", "AUTOCONF", "AUTOHEADER", "AUTOM4TE", "AUTOMAKE", "AUTOPOINT", "ACLOCAL", "GTKDOCIZE", "INTLTOOLIZE", "LIBTOOLIZE", "M4", "MAKE" };

    for (int i = 0; i < 12; i++) {
        if (unsetenv(unsetenvs[i]) != 0) {
            perror(unsetenvs[i]);
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t ndkpkgCoreDIRCapacity = ndkpkgHomeDIRLength + 6U;
    char   ndkpkgCoreDIR[ndkpkgCoreDIRCapacity];

    ret = snprintf(ndkpkgCoreDIR, ndkpkgCoreDIRCapacity, "%s/core", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    ret = ndkpkg_session_dir(sessionDIR, PATH_MAX, &sessionDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

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
                return NDKPKG_ERROR;
            }

            if (mkdir(ndkpkgDownloadsDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ndkpkgDownloadsDIR);
                    return NDKPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(ndkpkgDownloadsDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(ndkpkgDownloadsDIR);
                return NDKPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char   uppmPackageInstalledRootDIR[PATH_MAX];
    size_t uppmPackageInstalledRootDIRLength;

    const char * const uppmHomeDIR = getenv("UPPM_HOME");

    if (uppmHomeDIR == NULL || uppmHomeDIR[0] == '\0') {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL) {
            return NDKPKG_ERROR_ENV_HOME_NOT_SET;
        }

        if (userHomeDIR[0] == '\0') {
            return NDKPKG_ERROR_ENV_HOME_NOT_SET;
        }

        uppmPackageInstalledRootDIRLength = snprintf(uppmPackageInstalledRootDIR, PATH_MAX, "%s/.uppm/installed", userHomeDIR);
    } else {
        uppmPackageInstalledRootDIRLength = snprintf(uppmPackageInstalledRootDIR, PATH_MAX, "%s/installed", uppmHomeDIR);
    }

    if (uppmPackageInstalledRootDIRLength < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    NDKPKGToolChain toolchainForNativeBuild = {0};

    ret = ndkpkg_toolchain_locate(&toolchainForNativeBuild);

    if (ret != NDKPKG_OK) {
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = ndkpkg_setup_toolchain_for_native_build(&toolchainForNativeBuild, sessionDIR, sessionDIRLength, ndkpkgCoreDIR, ndkpkgCoreDIRCapacity, installOptions);

    if (ret != NDKPKG_OK) {
        ndkpkg_toolchain_free(&toolchainForNativeBuild);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    NDKPKGAndroidNDKToolChain toolchainForTargetBuild = {0};

    ret = ndkpkg_android_ndk_toolchain_make(&toolchainForTargetBuild, installOptions->ndkHome);

    if (ret != NDKPKG_OK) {
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    //ret = ndkpkg_setup_toolchain_for_target_build(&toolchainForNativeBuild, sessionDIR, sessionDIRLength, ndkpkgCoreDIR, ndkpkgCoreDIRCapacity, installOptions);

    if (ret != NDKPKG_OK) {
        ndkpkg_android_ndk_toolchain_free(&toolchainForTargetBuild);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    SysInfo sysinfo = {0};

    ret = sysinfo_make(&sysinfo);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char * ndkpkgExeFilePath = self_realpath();

    if (ndkpkgExeFilePath == NULL) {
        sysinfo_free(&sysinfo);
        ndkpkg_toolchain_free(&toolchainForNativeBuild);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t           packageSetCapacity = 0U;
    size_t           packageSetSize     = 0U;
    NDKPKGPackage ** packageSet         = NULL;

    ret = check_and_read_formula_in_cache(packageName, sessionDIR, &packageSet, &packageSetSize, &packageSetCapacity);

    if (ret != NDKPKG_OK) {
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////

    printf("install packages in order: ");

    for (int i = packageSetSize - 1; i >= 0; i--) {
        NDKPKGPackage * package = packageSet[i];
        char * packageName = package->packageName;
        printf("%s ", packageName);
    }

    printf("\n");

    //////////////////////////////////////////////////////////////////////////////

    for (int i = packageSetSize - 1; i >= 0; i--) {
        NDKPKGPackage * package = packageSet[i];
        char * packageName = package->packageName;

        if (!installOptions->force) {
            ret = ndkpkg_check_if_the_given_package_is_installed(packageName, targetPlatform);

            if (ret == NDKPKG_OK) {
                fprintf(stderr, "package already has been installed : %s\n", packageName);
                continue;
            }
        }

        if (setenv("PATH", PATH, 1) != 0) {
            perror("PATH");
            goto finalize;
        }

        char * recursiveDependentPackageNamesStringBuffer         = NULL;
        size_t recursiveDependentPackageNamesStringBufferSize     = 0U;
        size_t recursiveDependentPackageNamesStringBufferCapacity = 0U;

        ret = getRecursiveDependentPackageNamesStringBuffer(packageName, packageSet, packageSetSize, &recursiveDependentPackageNamesStringBuffer, &recursiveDependentPackageNamesStringBufferSize, &recursiveDependentPackageNamesStringBufferCapacity);

        if (ret != NDKPKG_OK) {
            goto finalize;
        }

        //printf("%s:%zu:%s\n", packageName, recursiveDependentPackageNamesStringBufferSize, recursiveDependentPackageNamesStringBuffer);

        ret = ndkpkg_install_package(packageName, strlen(packageName), targetPlatform, package->formula, installOptions, &toolchainForNativeBuild, &toolchainForTargetBuild, &sysinfo, time(NULL), uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRLength, ndkpkgExeFilePath, ndkpkgHomeDIR, ndkpkgHomeDIRLength, ndkpkgCoreDIR, ndkpkgCoreDIRCapacity, ndkpkgDownloadsDIR, ndkpkgDownloadsDIRCapacity, sessionDIR, sessionDIRLength, (const char *)recursiveDependentPackageNamesStringBuffer, recursiveDependentPackageNamesStringBufferSize);

        free(recursiveDependentPackageNamesStringBuffer);
        recursiveDependentPackageNamesStringBuffer = NULL;

        if (ret != NDKPKG_OK) {
            goto finalize;
        }
    }

finalize:
    sysinfo_free(&sysinfo);
    ndkpkg_toolchain_free(&toolchainForNativeBuild);
    free(ndkpkgExeFilePath);

    for (size_t i = 0; i < packageSetSize; i++) {
        free(packageSet[i]->packageName);
        ndkpkg_formula_free(packageSet[i]->formula);

        packageSet[i]->formula = NULL;
        packageSet[i]->packageName = NULL;

        free(packageSet[i]);
        packageSet[i] = NULL;
    }

    free(packageSet);
    packageSet = NULL;

    if (ret == NDKPKG_OK) {
        if (!installOptions->keepSessionDIR) {
            ret = ndkpkg_rm_r(sessionDIR, false);
        }
    }

    return ret;
}

#include <math.h>
#include <errno.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/regex/regex.h"
#include "core/sysinfo.h"
#include "core/http.h"
#include "core/tar.h"
#include "core/log.h"

#include "ndk-pkg.h"

static int get_uppm_latest_version(const char ** uppmLatestReleaseName, const bool verbose) {
    int ret = ndkpkg_http_fetch_to_file("https://api.github.com/repos/leleliu008/uppm/releases/latest", "uppm.json", verbose, verbose);

    if (ret == NDKPKG_OK) {
        FILE * file = fopen("uppm.json", "r");

        if (file == NULL) {
            perror("uppm.json");
            return NDKPKG_ERROR;
        }

        size_t j = 0;

        char buf[30];

        for (;;) {
            if (fgets(buf, 30, file) == NULL) {
                if (ferror(file)) {
                    perror("uppm.json");
                    fclose(file);
                    return NDKPKG_ERROR;
                } else {
                    fclose(file);
                    return NDKPKG_OK;
                }
            }

            if (regex_matched(buf, "^[[:space:]]*\"tag_name\"") == 0) {
                size_t length = strlen(buf);

                for (size_t i = 10; i < length; i++) {
                    if (j == 0) {
                        if (buf[i] >= '0' && buf[i] <= '9') {
                            j = i;
                        }
                    } else {
                        if (buf[i] == '"') {
                            buf[i] = '\0';
                            const char * p = strdup(&buf[j]);

                            if (p == NULL) {
                                return NDKPKG_ERROR_MEMORY_ALLOCATE;
                            } else {
                                (*uppmLatestReleaseName) = p;
                                return NDKPKG_OK;
                            }
                        }
                    }
                }
                break;
            } else {
                if (errno != 0) {
                    perror(NULL);
                    fclose(file);
                    return NDKPKG_ERROR;
                }
            }
        }
    }
}

int ndkpkg_setup(const bool verbose) {
    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    int ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t ndkpkgRunDIRCapacity = ndkpkgHomeDIRLength + 5U;
    char   ndkpkgRunDIR[ndkpkgRunDIRCapacity];

    ret = snprintf(ndkpkgRunDIR, ndkpkgRunDIRCapacity, "%s/run", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (stat(ndkpkgRunDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(ndkpkgRunDIR) == 0) {
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
            ret = ndkpkg_rm_r(sessionDIR, verbose);

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

    ////////////////////////////////////////////////////////////////////////////////////////////

    if (chdir(sessionDIR) != 0) {
        perror(sessionDIR);
        return NDKPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    ret = ndkpkg_http_fetch_to_file("https://raw.githubusercontent.com/leleliu008/ndkpkg/c/ndkpkg-install", "ndkpkg-install", verbose, verbose);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    ret = ndkpkg_http_fetch_to_file("https://raw.githubusercontent.com/leleliu008/ndkpkg/c/ndkpkg-do12345", "ndkpkg-do12345", verbose, verbose);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char* items[8] = { "wrapper-native-c++.c", "wrapper-native-objc.c", "wrapper-target-cc.c", "wrapper-target-clang.c", "wrapper-native-cc.c", "wrapper-target-c++.c", "wrapper-target-clang++.c", "wrapper-target-objc.c" };

    for (int i = 0; i < 8; i++) {
        const char * item = items[i];

        size_t urlCapacity = strlen(item) + 53U;
        char   url[urlCapacity];

        ret = snprintf(url, urlCapacity, "https://raw.githubusercontent.com/leleliu008/ndkpkg/c/%s", item);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        ret = ndkpkg_http_fetch_to_file(url, item, verbose, verbose);

        if (ret != NDKPKG_OK) {
            return ret;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char * uppmLatestReleaseName = "0.15.0+b5148c3e8fdbadc64120a0d88aae095cd5324a57";

    char uppmLatestReleaseVersion[10] = {0};

    for (int i = 0; i < 10; i++) {
        char c = uppmLatestReleaseName[i];

        if (c == '+') {
            break;
        }

        uppmLatestReleaseVersion[i] = uppmLatestReleaseName[i];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    char osType[31] = {0};

    if (sysinfo_type(osType, 30) != 0) {
        return NDKPKG_ERROR;
    }

    char osArch[31] = {0};

    if (sysinfo_arch(osArch, 30) != 0) {
        return NDKPKG_ERROR;
    }

    size_t tarballFileNameCapacity = strlen(uppmLatestReleaseVersion) + strlen(osType) + strlen(osArch) + 15U + 5U;
    char   tarballFileName[tarballFileNameCapacity];

    if (strcmp(osType, "macos") == 0) {
        char osVersion[31] = {0};

        if (sysinfo_vers(osVersion, 30) != 0) {
            return NDKPKG_ERROR;
        }

        for (int i = 0; i < 31; i++) {
            char c = osVersion[i];

            if (c == '\0') {
                break;
            }

            if (c == '.') {
                osVersion[i] = '\0';
                break;
            }
        }

        const char * x;

        if (strcmp(osVersion, "10") == 0) {
            x = "10.15";
        } else if (strcmp(osVersion, "11") == 0) {
            x = "11.0";
        } else if (strcmp(osVersion, "12") == 0) {
            x = "12.0";
        } else {
            x = "13.0";
        }

        ret = snprintf(tarballFileName, tarballFileNameCapacity, "uppm-%s-%s%s-%s.tar.xz", uppmLatestReleaseVersion, osType, x, osArch);
    } else {
        ret = snprintf(tarballFileName, tarballFileNameCapacity, "uppm-%s-%s-%s.tar.xz", uppmLatestReleaseVersion, osType, osArch);
    }

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    size_t tarballUrlCapacity = tarballFileNameCapacity + strlen(uppmLatestReleaseName) + 55U;
    char   tarballUrl[tarballUrlCapacity];

    ret = snprintf(tarballUrl, tarballUrlCapacity, "https://github.com/leleliu008/uppm/releases/download/%s/%s", uppmLatestReleaseName, tarballFileName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = ndkpkg_http_fetch_to_file(tarballUrl, tarballFileName, verbose, verbose);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = tar_extract(sessionDIR, tarballFileName, 0, verbose, 1);

    if (ret != ARCHIVE_OK) {
        return abs(ret) + NDKPKG_ERROR_ARCHIVE_BASE;
    }

    if (rename("bin/uppm", "uppm") == -1) {
        perror("bin/uppm");
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = ndkpkg_http_fetch_to_file("https://curl.se/ca/cacert.pem", "cacert.pem", verbose, verbose);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t cacertPemFilePathCapacity = sessionDIRCapacity + 12U;
    char   cacertPemFilePath[cacertPemFilePathCapacity];

    ret = snprintf(cacertPemFilePath, cacertPemFilePathCapacity, "%s/cacert.pem", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (setenv("SSL_CERT_FILE", cacertPemFilePath, 1) != 0) {
        perror("SSL_CERT_FILE");
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    const char * NDKPKG_URL_TRANSFORM = getenv("NDKPKG_URL_TRANSFORM");

    if (NDKPKG_URL_TRANSFORM != NULL && NDKPKG_URL_TRANSFORM[0] != '\0') {
        if (setenv("UPPM_URL_TRANSFORM", NDKPKG_URL_TRANSFORM, 1) != 0) {
            perror("UPPM_URL_TRANSFORM");
            return NDKPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (pid == 0) {
        execl ("./uppm", "./uppm", "update", NULL);
        perror("./uppm");
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (childProcessExitStatusCode != 0) {
            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command './uppm update' exit with status code: %d\n", WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command './uppm update' killed by signal: %d\n", WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command './uppm update' stopped by signal: %d\n", WSTOPSIG(childProcessExitStatusCode));
            }

            return NDKPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    if (chdir(ndkpkgRunDIR) != 0) {
        perror(ndkpkgRunDIR);
        return NDKPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t ndkpkgCoreDIRCapacity = ndkpkgHomeDIRLength + 6U;
    char   ndkpkgCoreDIR[ndkpkgCoreDIRCapacity];

    ret = snprintf(ndkpkgCoreDIR, ndkpkgCoreDIRCapacity, "%s/core", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    for (;;) {
        if (rename(sessionDIR, ndkpkgCoreDIR) == 0) {
            return NDKPKG_OK;
        } else {
            if (errno == ENOTEMPTY || errno == EEXIST) {
                if (lstat(ndkpkgCoreDIR, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = ndkpkg_rm_r(ndkpkgCoreDIR, verbose);

                        if (ret != NDKPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(ndkpkgCoreDIR) != 0) {
                            perror(ndkpkgCoreDIR);
                            return NDKPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(ndkpkgCoreDIR);
                return NDKPKG_ERROR;
            }
        }
    }
}

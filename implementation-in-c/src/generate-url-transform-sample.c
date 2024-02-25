#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#include "core/log.h"

#include "ndk-pkg.h"

int ndkpkg_generate_url_transform_sample() {
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

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRCapacity + 22U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/url-transform.sample", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    int fd = open(tmpFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(tmpFilePath);
        return NDKPKG_ERROR;
    }

    const char * p = ""
        "#!/bin/sh\n"
        "case $1 in\n"
        "    *githubusercontent.com/*)\n"
        "        printf '%s\\n' \"$1\" | sed 's|githubusercontent|gitmirror|'\n"
        "        ;;\n"
        "    https://github.com/*)\n"
        "        printf 'https://hub.gitmirror.com/%s\\n' \"$1\"\n"
        "        ;;\n"
        "    '') printf '%s\\n' \"$0 <URL>, <URL> is unspecified.\" >&2 ; exit 1 ;;\n"
        "    *)  printf '%s\\n' \"$1\"\n"
        "esac";

    size_t pSize = strlen(p);

    ssize_t writeSize = write(fd, p, pSize);

    if (writeSize == -1) {
        perror(tmpFilePath);
        close(fd);
        return NDKPKG_ERROR;
    }

    close(fd);

    if ((size_t)writeSize != pSize) {
        fprintf(stderr, "not fully written to %s\n", tmpFilePath);
        return NDKPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    if (chmod(tmpFilePath, S_IRWXU) != 0) {
        perror(tmpFilePath);
        return NDKPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t outFilePathCapacity = ndkpkgHomeDIRLength + 22U;
    char   outFilePath[outFilePathCapacity];

    ret = snprintf(outFilePath, outFilePathCapacity, "%s/url-transform.sample", ndkpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (rename(tmpFilePath, outFilePath) != 0) {
        if (errno == EXDEV) {
            ret = ndkpkg_copy_file(tmpFilePath, outFilePath);

            if (ret != NDKPKG_OK) {
                return ret;
            }
        } else {
            perror(tmpFilePath);
            return NDKPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "%surl-transform sample has been written into %s%s\n\n", COLOR_GREEN, outFilePath, COLOR_OFF);

    outFilePath[outFilePathCapacity - 9U] = '\0';

    fprintf(stderr, "%sYou can rename url-transform.sample to url-transform then edit it to meet your needs.\n\nTo apply this, you should run 'export NDKPKG_URL_TRANSFORM=%s' in your terminal.\n%s", COLOR_GREEN, outFilePath, COLOR_OFF);

    return ndkpkg_rm_r(sessionDIR, false);
}

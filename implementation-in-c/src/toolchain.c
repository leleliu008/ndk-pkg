#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/exe.h"

#include "ndk-pkg.h"

static int read_from_fd(int inputFD, char ** outP) {
    // PATH_MAX : maximum number of bytes in a pathname, including the terminating null character.
    // https://pubs.opengroup.org/onlinepubs/009695399/basedefs/limits.h.html
    char buf[PATH_MAX];

    ssize_t readSize = read(inputFD, buf, PATH_MAX - 1U);

    if (readSize < 0) {
        return -1;
    }

    if (readSize == 0) {
        return 0;
    }

    if (buf[readSize - 1] == '\n') {
        readSize--;
    }

    if (readSize > 0) {
        buf[readSize] = '\0';

        char * p = strdup(buf);

        if (p == NULL) {
            return -1;
        }

        (*outP) = p;
    }

    return 0;
}

// https://keith.github.io/xcode-man-pages/xcrun.1.html
static int xcrun_show_sdk_path(char ** outP) {
    int pipeFDs[2];

    if (pipe(pipeFDs) != 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (pid == 0) {
        close(pipeFDs[0]);

        if (dup2(pipeFDs[1], STDOUT_FILENO) < 0) {
            perror(NULL);
            exit(254);
        }

        execl("/usr/bin/xcrun", "/usr/bin/xcrun", "--sdk", "macosx", "--show-sdk-path", NULL);

        perror("/usr/bin/xcrun");

        exit(255);
    } else {
        close(pipeFDs[1]);

        int ret = read_from_fd(pipeFDs[0], outP);

        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            if (ret == 0) {
                return NDKPKG_OK;
            } else {
                perror(NULL);
                return NDKPKG_ERROR;
            }
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk macosx --show-sdk-path' exit with status code: %d\n", WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk macosx --show-sdk-path' killed by signal: %d\n", WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk macosx --show-sdk-path' stopped by signal: %d\n", WSTOPSIG(childProcessExitStatusCode));
        }

        return NDKPKG_ERROR;
    }
}

// https://keith.github.io/xcode-man-pages/xcrun.1.html
static int xcrun_find(const char * what, char ** outP) {
    int pipeFDs[2];

    if (pipe(pipeFDs) != 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (pid == 0) {
        close(pipeFDs[0]);

        if (dup2(pipeFDs[1], STDOUT_FILENO) < 0) {
            perror(NULL);
            exit(254);
        }

        execl("/usr/bin/xcrun", "/usr/bin/xcrun", "--sdk", "macosx", "--find", what, NULL);

        perror("/usr/bin/xcrun");

        exit(255);
    } else {
        close(pipeFDs[1]);

        int ret = read_from_fd(pipeFDs[0], outP);

        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            if (ret == 0) {
                return NDKPKG_OK;
            } else {
                perror(NULL);
                return NDKPKG_ERROR;
            }
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk macosx --find %s' exit with status code: %d\n", what, WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk macosx --find %s' killed by signal: %d\n", what, WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk macosx --find %s' stopped by signal: %d\n", what, WSTOPSIG(childProcessExitStatusCode));
        }

        return NDKPKG_ERROR;
    }
}

static int ndkpkg_toolchain_find(NDKPKGToolChain * toolchain) {
    const char * a[13] = { "cc", "c++", "as", "ar", "ld", "nm", "size", "strip", "ranlib", "strings", "objdump", "objcopy", "readelf" };

    char * p = NULL;

    for (int i = 0; i < 13; i++) {
        int ret = exe_lookup(a[i], &p);

        switch (ret) {
            case -3:
                ndkpkg_toolchain_free(toolchain);
                return NDKPKG_ERROR_ENV_PATH_NOT_SET;
            case -2:
                ndkpkg_toolchain_free(toolchain);
                return NDKPKG_ERROR_ENV_PATH_NOT_SET;
            case -1:
                perror(NULL);
                ndkpkg_toolchain_free(toolchain);
                return NDKPKG_ERROR;
            case 0:
                fprintf(stderr, "command not found: %s\n", a[i]);
                ndkpkg_toolchain_free(toolchain);
                return NDKPKG_ERROR;
        }

        switch (i) {
            case 0: toolchain->cc = p; break;
            case 1: toolchain->cxx = p; break;
            case 2: toolchain->as = p; break;
            case 3: toolchain->ar = p; break;
            case 4: toolchain->ld = p; break;
            case 5: toolchain->nm = p; break;
            case 6: toolchain->size = p; break;
            case 7: toolchain->strip = p; break;
            case 8: toolchain->ranlib = p; break;
            case 9: toolchain->strings = p; break;
            case 10: toolchain->objdump = p; break;
            case 11: toolchain->objcopy = p; break;
            case 12: toolchain->readelf = p; break;
        }
    }

    p = strdup("-fPIC -fno-common");

    if (p == NULL) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    } else {
        toolchain->cxxflags = p;
    }

    p = strdup("-fPIC -fno-common");

    if (p == NULL) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    } else {
        toolchain->ccflags = p;
    }

    // https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
    p = strdup("-Wl,--as-needed -Wl,-z,muldefs -Wl,--allow-multiple-definition");

    if (p == NULL) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    } else {
        toolchain->ldflags = p;
    }

    return NDKPKG_OK;
}

static int ndkpkg_toolchain_macos(NDKPKGToolChain * toolchain) {
    const char * a[11] = { "clang", "clang++", "as", "ar", "ld", "nm", "size", "strip", "ranlib", "strings", "objdump" };

    char * p = NULL;

    int ret;

    for (int i = 0; i < 11; i++) {
        ret = xcrun_find(a[i], &p);

        if (ret == NDKPKG_OK) {
            if (p == NULL) {
                fprintf(stderr, "command not found: %s\n", a[i]);
                return NDKPKG_ERROR;
            } else {
                switch (i) {
                    case 0: toolchain->cc = p; break;
                    case 1: toolchain->cxx = p; break;
                    case 2: toolchain->as = p; break;
                    case 3: toolchain->ar = p; break;
                    case 4: toolchain->ld = p; break;
                    case 5: toolchain->nm = p; break;
                    case 6: toolchain->size = p; break;
                    case 7: toolchain->strip = p; break;
                    case 8: toolchain->ranlib = p; break;
                    case 9: toolchain->strings = p; break;
                    case 10: toolchain->objdump = p; break;
                }
            }
        } else {
            ndkpkg_toolchain_free(toolchain);
            return ret;
        }
    }

    //////////////////////////////////////////////

    char * sysroot = NULL;

    ret = xcrun_show_sdk_path(&sysroot);

    if (ret == NDKPKG_OK) {
        if (sysroot == NULL) {
            fprintf(stderr, "Can not locate MacOSX sdk path.\n");
            return NDKPKG_ERROR;
        } else {
            toolchain->sysroot = sysroot;
        }
    } else {
        ndkpkg_toolchain_free(toolchain);
        return ret;
    }

    size_t sysrootLength = strlen(sysroot);

    size_t cppflagsCapacity = sysrootLength + 30U;
    char   cppflags[cppflagsCapacity];

    ret = snprintf(cppflags, cppflagsCapacity, "-isysroot %s -Qunused-arguments", sysroot);

    if (ret < 0) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR;
    }

    size_t cxxflagsCapacity = sysrootLength + 48U;
    char   cxxflags[cxxflagsCapacity];

    ret = snprintf(cxxflags, cxxflagsCapacity, "-isysroot %s -Qunused-arguments -fPIC -fno-common", sysroot);

    if (ret < 0) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR;
    }

    size_t ccflagsCapacity = sysrootLength + 48U;
    char   ccflags[ccflagsCapacity];

    ret = snprintf(ccflags, ccflagsCapacity, "-isysroot %s -Qunused-arguments -fPIC -fno-common", sysroot);

    if (ret < 0) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR;
    }

    size_t ldflagsCapacity = sysrootLength + 35U;
    char   ldflags[ldflagsCapacity];

    ret = snprintf(ldflags, ldflagsCapacity, "-isysroot %s -Wl,-search_paths_first", sysroot);

    if (ret < 0) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR;
    }

    toolchain->cppflags = strdup(cppflags);

    if (toolchain->cppflags == NULL) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->cxxflags = strdup(cxxflags);

    if (toolchain->cxxflags == NULL) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->ccflags = strdup(ccflags);

    if (toolchain->ccflags == NULL) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->ldflags = strdup(ldflags);

    if (toolchain->ldflags == NULL) {
        ndkpkg_toolchain_free(toolchain);
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

    return NDKPKG_OK;
}

int ndkpkg_toolchain_locate(NDKPKGToolChain * toolchain) {
#if defined (__APPLE__)
    return ndkpkg_toolchain_macos(toolchain);
#else
    return ndkpkg_toolchain_find(toolchain);
#endif
}

void ndkpkg_toolchain_dump(NDKPKGToolChain * toolchain) {
    if (toolchain == NULL) {
        return;
    }

    printf("cc:        %s\n", toolchain->cc);
    printf("objc:      %s\n", toolchain->objc);
    printf("cxx:       %s\n", toolchain->cxx);
    printf("cpp:       %s\n", toolchain->cpp);
    printf("as:        %s\n", toolchain->as);
    printf("ar:        %s\n", toolchain->ar);
    printf("ranlib:    %s\n", toolchain->ranlib);
    printf("ld:        %s\n", toolchain->ld);
    printf("nm:        %s\n", toolchain->nm);
    printf("size:      %s\n", toolchain->size);
    printf("strip:     %s\n", toolchain->strip);
    printf("strings:   %s\n", toolchain->strings);
    printf("objcopy:   %s\n", toolchain->objcopy);
    printf("objdump:   %s\n", toolchain->objdump);
    printf("readelf:   %s\n", toolchain->readelf);
    printf("dlltool:   %s\n", toolchain->dlltool);
    printf("addr2line: %s\n", toolchain->addr2line);
    printf("sysroot:   %s\n", toolchain->sysroot);
    printf("ccflags:   %s\n", toolchain->ccflags);
    printf("cxxflags:  %s\n", toolchain->cxxflags);
    printf("cppflags:  %s\n", toolchain->cppflags);
    printf("ldflags:   %s\n", toolchain->ldflags);
}

void ndkpkg_toolchain_free(NDKPKGToolChain * toolchain) {
    if (toolchain == NULL) {
        return;
    }

    if (toolchain->cc != NULL) {
        free(toolchain->cc);
        toolchain->cc = NULL;
    }

    if (toolchain->cxx != NULL) {
        free(toolchain->cxx);
        toolchain->cxx = NULL;
    }

    if (toolchain->objc != NULL) {
        free(toolchain->objc);
        toolchain->objc = NULL;
    }

    if (toolchain->cpp != NULL) {
        free(toolchain->cpp);
        toolchain->cpp = NULL;
    }

    if (toolchain->as != NULL) {
        free(toolchain->as);
        toolchain->as = NULL;
    }

    if (toolchain->ar != NULL) {
        free(toolchain->ar);
        toolchain->ar = NULL;
    }

    if (toolchain->ranlib != NULL) {
        free(toolchain->ranlib);
        toolchain->ranlib = NULL;
    }

    if (toolchain->ld != NULL) {
        free(toolchain->ld);
        toolchain->ld = NULL;
    }

    if (toolchain->nm != NULL) {
        free(toolchain->nm);
        toolchain->nm = NULL;
    }

    if (toolchain->size != NULL) {
        free(toolchain->size);
        toolchain->size = NULL;
    }

    if (toolchain->strip != NULL) {
        free(toolchain->strip);
        toolchain->strip = NULL;
    }

    if (toolchain->strings != NULL) {
        free(toolchain->strings);
        toolchain->strings = NULL;
    }

    if (toolchain->objcopy != NULL) {
        free(toolchain->objcopy);
        toolchain->objcopy = NULL;
    }

    if (toolchain->objdump != NULL) {
        free(toolchain->objdump);
        toolchain->objdump = NULL;
    }

    if (toolchain->readelf != NULL) {
        free(toolchain->readelf);
        toolchain->readelf = NULL;
    }

    if (toolchain->dlltool != NULL) {
        free(toolchain->dlltool);
        toolchain->dlltool = NULL;
    }

    if (toolchain->addr2line != NULL) {
        free(toolchain->addr2line);
        toolchain->addr2line = NULL;
    }

    if (toolchain->sysroot != NULL) {
        free(toolchain->sysroot);
        toolchain->sysroot = NULL;
    }

    if (toolchain->ccflags != NULL) {
        free(toolchain->ccflags);
        toolchain->ccflags = NULL;
    }

    if (toolchain->cxxflags != NULL) {
        free(toolchain->cxxflags);
        toolchain->cxxflags = NULL;
    }

    if (toolchain->cppflags != NULL) {
        free(toolchain->cppflags);
        toolchain->cppflags = NULL;
    }

    if (toolchain->ldflags != NULL) {
        free(toolchain->ldflags);
        toolchain->ldflags = NULL;
    }
}

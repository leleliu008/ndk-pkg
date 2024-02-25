#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html
#if defined (__APPLE__)
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#elif defined (__linux__) && defined (HAVE_LINUX_LIMITS_H)
#include <linux/limits.h>
#else
#include <limits.h>
#endif

#if defined (__FreeBSD__) || defined (__OpenBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if defined (__OpenBSD__)
#include <stdbool.h>

static inline bool ispath(const char * p) {
    for (;;) {
        if (p[0] == '\0') {
            return false;
        }

        if (p[0] == '/') {
            return true;
        }

        p++;
    }
}
#endif

#include "self.h"

char* self_realpath() {
#if defined (__APPLE__)
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html
    uint32_t bufSize = 0U;
    _NSGetExecutablePath(NULL, &bufSize);

    char path[bufSize];
    _NSGetExecutablePath(path, &bufSize);

    return realpath(path, NULL);
#elif defined (__FreeBSD__)
    const int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };

    size_t bufLength = 0U;

    if (sysctl(mib, 4, NULL, &bufLength, NULL, 0) < 0) {
        return NULL;
    }

    char * buf = (char*)calloc(bufLength + 1U, sizeof(char));

    if (buf == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if (sysctl(mib, 4, buf, &bufLength, NULL, 0) < 0) {
        return NULL;
    }

    return buf;
#elif defined (__OpenBSD__)
    const int mib[4] = { CTL_KERN, KERN_PROC_ARGS, getpid(), KERN_PROC_ARGV };
    size_t size;

    if (sysctl(mib, 4, NULL, &size, NULL, 0) != 0) {
        return NULL;
    }

    char** argv = (char**)malloc(size);

    if (argv == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    memset(argv, 0, size);

    if (sysctl(mib, 4, argv, &size, NULL, 0) != 0) {
        return NULL;
    }

    if (ispath(argv[0])) {
        return realpath(argv[0], NULL);
    } else {
        const char * const PATH = getenv("PATH");

        // in fact, it shouldn’t happen
        if (PATH == NULL) {
            return NULL;
        }

        size_t PATHLength = strlen(PATH);

        // in fact, it shouldn’t happen
        if (PATHLength == 0U) {
            return NULL;
        }

        size_t  PATH2Length = PATHLength + 1U;
        char    PATH2[PATH2Length];
        strncpy(PATH2, PATH, PATH2Length);

        struct stat st;

        char buf[PATH_MAX];

        int ret;

        size_t commandNameLength = strlen(argv[0]);

        char * PATHItem = strtok(PATH2, ":");

        while (PATHItem != NULL) {
            if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
                ret = snprintf(buf, PATH_MAX, "%s/%s", PATHItem, argv[0]);

                if (ret < 0) {
                    return -1;
                }

                if (access(buf, X_OK) == 0) {
                    char * p = strdup(buf);

                    if (p == NULL) {
                        errno = ENOMEM;
                        return NULL;
                    } else {
                        return p;
                    }
                }
            }

            PATHItem = strtok(NULL, ":");
        }

        // in fact, it shouldn’t happen
        return NULL;
    }
#else
    // PATH_MAX : maximum number of bytes in a pathname, including the terminating null character.
    // https://pubs.opengroup.org/onlinepubs/009695399/basedefs/limits.h.html
    char buf[PATH_MAX] = {0};

    //  readlink() does not append a terminating null byte to buf.
    if (readlink("/proc/self/exe", buf, PATH_MAX - 1U) == -1) {
        return NULL;
    }

    char * p = strdup(buf);

    if (p == NULL) {
        errno = ENOMEM;
        return NULL;
    } else {
        return p;
    }
#endif
}

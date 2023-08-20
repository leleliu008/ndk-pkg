#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#define ACTION_COMPILE 1
#define ACTION_CREATE_SHARED_LIBRARY 2
#define ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE 3
#define ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE 4

int main(int argc, char * argv[]) {
    const char * options[6] = { "-shared", "-static", "--static", "-pie", "-c", "-o" };
          int    indexes[6] = {    -1,         -1,        -1,       -1,    -1,   -1  };

    for (int i = 1; i < argc; i++) {
        for (int j = 0; j < 6; j++) {
            if (strcmp(argv[i], options[j]) == 0) {
                indexes[j] = i;
                break;
            }
        }
    }

    int action = 0;

    if (indexes[0] > 0) {
        // if -shared option is specified, then remove -static , --static , -pie options if they also are specified
        action = ACTION_CREATE_SHARED_LIBRARY;
    } else if ((indexes[1] > 0) || (indexes[2] > 0)) {
        // if -shared option is not specified, but -static or --static option is specified, then remove -pie , -Wl,-Bdynamic option if it also is specified
        action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
    } else if (indexes[3] > 0) {
        action = ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE;
    }

    /////////////////////////////////////////////////////////////////

    char * const cxxc = getenv("ANDROID_NDK_CXX");

    if (cxxc == NULL) {
        fprintf(stderr, "ANDROID_NDK_CXX environment variable is not set.\n");
        return 1;
    }

    if (cxxc[0] == '\0') {
        fprintf(stderr, "ANDROID_NDK_CXX environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    const char * const TARGET = getenv("ANDROID_TARGET");

    if (TARGET == NULL) {
        fprintf(stderr, "ANDROID_TARGET environment variable is not set.\n");
        return 1;
    }

    if (TARGET[0] == '\0') {
        fprintf(stderr, "ANDROID_TARGET environment variable value should be a non-empty string.\n");
        return 2;
    }

    size_t   targetArgLength = strlen(TARGET) + 10U;
    char     targetArg[targetArgLength];
    snprintf(targetArg, targetArgLength, "--target=%s", TARGET);

    /////////////////////////////////////////////////////////////////

    const char * const SYSROOT = getenv("ANDROID_NDK_SYSROOT");

    if (SYSROOT == NULL) {
        fprintf(stderr, "ANDROID_NDK_SYSROOT environment variable is not set.\n");
        return 1;
    }

    if (SYSROOT[0] == '\0') {
        fprintf(stderr, "ANDROID_NDK_SYSROOT environment variable value should be a non-empty string.\n");
        return 2;
    }

    size_t   sysrootArgLength = strlen(SYSROOT) + 11U;
    char     sysrootArg[sysrootArgLength];
    snprintf(sysrootArg, sysrootArgLength, "--sysroot=%s", SYSROOT);

    /////////////////////////////////////////////////////////////////

    char   sonameArg[100] = {0};

    char * argv2[argc + 4];

    argv2[0] = cxxc;
    argv2[1] = targetArg;
    argv2[2] = sysrootArg;

    if (action == ACTION_CREATE_SHARED_LIBRARY) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-static") == 0) {
                argv2[i + 2] = (char*)"--shared";
            } else if (strcmp(argv[i], "--static") == 0) {
                argv2[i + 2] = (char*)"--shared";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i + 2] = (char*)"--shared";
            } else {
                argv2[i + 2] = argv[i];
            }
        }

        char * filepath = NULL;
        char * filename = NULL;

        if (indexes[5] == -1) {
            // It's rare to see. like -o/a/b/libxx.so.1
            for (int i = 1; i < argc; i++) {
                if (strncmp(argv[i], "-o", 2) == 0) {
                    indexes[5] = i;
                    filepath = &argv[i][2];
                    break;
                }
            }
        } else {
            // if -o <FILE> option is specified.
            int i = indexes[5] + 1;

            if (i < argc) {
                filepath = argv[i];
            }
        }

        if (filepath != NULL) {
            int len = 0;

            for (;;) {
                if (filepath[len] == '\0') {
                    break;
                } else {
                    len++;
                }
            }

            for (int i = len - 1; i > 0; i--) {
                if (filepath[i] == '/') {
                    filename = filepath + i + 1;
                    break;
                }
            }

            if (filename == NULL) {
                filename = filepath;
            }

            regex_t regex;

            if (regcomp(&regex, "^lib.*\\.so", 0) != 0) {
                perror(NULL);
                regfree(&regex);
                return 1;
            }

            regmatch_t regmatch[2];

            if (regexec(&regex, filename, 2, regmatch, 0) == 0) {
                //printf("regmatch[0].rm_so=%d\n", regmatch[0].rm_so);
                //printf("regmatch[0].rm_eo=%d\n", regmatch[0].rm_eo);

                if ((regmatch[0].rm_so >= 0) && (regmatch[0].rm_eo > regmatch[0].rm_so)) {
                    int n = regmatch[0].rm_eo - regmatch[0].rm_so;
                    const char * str = &filename[regmatch[0].rm_so];
                    snprintf(sonameArg, n + 13, "-Wl,-soname,%s", str);
                }
            }

            regfree(&regex);
        }
    } else if (action == ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-rdynamic") == 0) {
                argv2[i + 2] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,--export-dynamic") == 0) {
                argv2[i + 2] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,-Bdynamic") == 0) {
                argv2[i + 2] = (char*)"-static";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i + 2] = (char*)"-static";
            } else {
                argv2[i + 2] = argv[i];
            }
        }
    } else {
        for (int i = 1; i < argc; i++) {
            argv2[i + 2] = argv[i];
        }
    }

    if (sonameArg[0] == '\0') {
        argv2[argc + 2] = NULL;
    } else {
        argv2[argc + 2] = sonameArg;
        argv2[argc + 3] = NULL;
    }

    execv (cxxc, argv2);
    perror(cxxc);
    return 255;
}

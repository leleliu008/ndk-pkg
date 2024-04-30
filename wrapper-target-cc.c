#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#define ACTION_PREPROCESS                           1
#define ACTION_COMPILE                              2
#define ACTION_ASSEMBLE                             3
#define ACTION_CREATE_SHARED_LIBRARY                4
#define ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE  5
#define ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE 6

int main(int argc, char * argv[]) {
    char * const cc = getenv("ANDROID_NDK_CC");

    if (cc == NULL) {
        fprintf(stderr, "ANDROID_NDK_CC environment variable is not set.\n");
        return 1;
    }

    if (cc[0] == '\0') {
        fprintf(stderr, "ANDROID_NDK_CC environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    const char * const TARGET = getenv("ANDROID_TARGET");

    if (TARGET == NULL) {
        fprintf(stderr, "ANDROID_TARGET environment variable is not set.\n");
        return 3;
    }

    if (TARGET[0] == '\0') {
        fprintf(stderr, "ANDROID_TARGET environment variable value should be a non-empty string.\n");
        return 4;
    }

    /////////////////////////////////////////////////////////////////

    const char * const SYSROOT = getenv("ANDROID_NDK_SYSROOT");

    if (SYSROOT == NULL) {
        fprintf(stderr, "ANDROID_NDK_SYSROOT environment variable is not set.\n");
        return 5;
    }

    if (SYSROOT[0] == '\0') {
        fprintf(stderr, "ANDROID_NDK_SYSROOT environment variable value should be a non-empty string.\n");
        return 6;
    }

    /////////////////////////////////////////////////////////////////

    size_t targetArgLength = strlen(TARGET) + 10U;
    char   targetArg[targetArgLength];

    int ret = snprintf(targetArg, targetArgLength, "--target=%s", TARGET);

    if (ret < 0) {
        perror(NULL);
        return 7;
    }

    /////////////////////////////////////////////////////////////////

    size_t sysrootArgLength = strlen(SYSROOT) + 11U;
    char   sysrootArg[sysrootArgLength];

    ret = snprintf(sysrootArg, sysrootArgLength, "--sysroot=%s", SYSROOT);

    if (ret < 0) {
        perror(NULL);
        return 8;
    }

    /////////////////////////////////////////////////////////////////

    const char * actions[8] = { "-E", "-S", "-c", "-shared", "-static", "--static", "-pie" };
          int    indexes[8] = {  -1,   -1,   -1,      -1,        -1,         -1,      -1,  };

    for (int i = 1; i < argc; i++) {
        for (int j = 0; j < 6; j++) {
            if (strcmp(argv[i], actions[j]) == 0) {
                indexes[j] = i;

                if (indexes[0] > 0) {
                    break;
                }

                if (indexes[1] > 0) {
                    break;
                }

                if (indexes[2] > 0) {
                    break;
                }

                if (indexes[3] > 0) {
                    break;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    int action;

    if (indexes[0] > 0) {
        action = ACTION_PREPROCESS;
    } else if (indexes[1] > 0) {
        action = ACTION_COMPILE;
    } else if (indexes[2] > 0) {
        action = ACTION_ASSEMBLE;
    } else if (indexes[3] > 0) {
        action = ACTION_CREATE_SHARED_LIBRARY;
    } else if (indexes[4] > 0) {
        action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
    } else if (indexes[5] > 0) {
        action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
    } else if (indexes[6] > 0) {
        action = ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE;
    } else {
        action = 0;
    }

    fprintf(stderr, "action=%d\n", action);
    /////////////////////////////////////////////////////////////////

    char* argv2[argc + 5];

    char sonameArg[100]; sonameArg[0] = '\0';

    if (action == ACTION_CREATE_SHARED_LIBRARY) {
        int oIndex = -1;

        // remove -static , --static , -pie options if they also are specified
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-static") == 0) {
                argv2[i] = (char*)"-fPIC";
            } else if (strcmp(argv[i], "--static") == 0) {
                argv2[i] = (char*)"-fPIC";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i] = (char*)"-fPIC";
            } else {
                if (strcmp(argv[i], "-o") == 0) {
                    oIndex = i;
                }

                argv2[i] = argv[i];
            }
        }

        char * outputFilePath = NULL;
        char * outputFileName = NULL;

        if (oIndex == -1) {
            // It's rare to see. like -o/a/b/libxx.so.1
            for (int i = 1; i < argc; i++) {
                if (strncmp(argv[i], "-o", 2) == 0) {
                    indexes[5] = i;
                    outputFilePath = &argv[i][2];
                    break;
                }
            }
        } else {
            // if -o <FILE> option is specified.
            int i = oIndex + 1;

            if (i < argc) {
                outputFilePath = argv[i];
            }
        }

        if (outputFilePath != NULL) {
            int len = 0;

            for (;;) {
                if (outputFilePath[len] == '\0') {
                    break;
                } else {
                    len++;
                }
            }

            for (int i = len - 1; i > 0; i--) {
                if (outputFilePath[i] == '/') {
                    outputFileName = outputFilePath + i + 1;
                    break;
                }
            }

            if (outputFileName == NULL) {
                outputFileName = outputFilePath;
            }

            char * p = strstr(outputFileName, ".so");

            if (p != NULL) {
                ptrdiff_t n = p - outputFileName + 3;

                char s[n + 1];

                strncpy(s, outputFileName, n);

                s[n] = '\0';

                ret = snprintf(sonameArg, n + 13, "-Wl,-soname,%s", s);

                if (ret < 0) {
                    perror(NULL);
                    return 10;
                }
            }
        }
    } else if (action == ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE) {
        // remove -pie , -Wl,-Bdynamic option if it also is specified
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-rdynamic") == 0) {
                argv2[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,--export-dynamic") == 0) {
                argv2[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,-Bdynamic") == 0) {
                argv2[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i] = (char*)"-static";
            } else if (argv[i][0] == '/') {
                int len = 0;

                int slashIndex = -1;

                for (int j = 0; ; j++) {
                    if (argv[i][j] == '\0') {
                        len = j;
                        break;
                    } else if (argv[i][j] == '/') {
                        slashIndex = j;
                    }
                }

                const char * filename = argv[i] + slashIndex + 1;

                fprintf(stderr, "filename=%s\n", filename);

                if (strcmp(filename, "libm.so") == 0) {
                    argv[i][0] = '-';
                    argv[i][1] = 'l';
                    argv[i][2] = 'm';
                    argv[i][3] = '\0';
                } else if (strcmp(filename, "libdl.so") == 0) {
                    argv[i][0] = '-';
                    argv[i][1] = 'l';
                    argv[i][2] = 'd';
                    argv[i][3] = 'l';
                    argv[i][4] = '\0';
                } else {
                    if ((argv[i][len - 3] == '.') && (argv[i][len - 2] == 's') && (argv[i][len - 1] == 'o')) {
                        argv[i][len - 2] = 'a';
                        argv[i][len - 1] = '\0';

                        struct stat st;

                        if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                            argv[i][len - 2] = 's';
                            argv[i][len - 1] = 'o';
                        }
                    }
                }

                argv2[i] = argv[i];
            } else {
                argv2[i] = argv[i];
            }
        }
    } else {
        const char * PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE = getenv("PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE");

        if (PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE != NULL && strcmp(PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE, "1") == 0) {
            for (int i = 1; i < argc; i++) {
                if (argv[i][0] == '/') {
                    int len = strlen(argv[i]);

                    if ((argv[i][len - 3] == '.') && (argv[i][len - 2] == 's') && (argv[i][len - 1] == 'o')) {
                        argv[i][len - 2] = 'a';
                        argv[i][len - 1] = '\0';

                        struct stat st;

                        if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                            argv[i][len - 2] = 's';
                            argv[i][len - 1] = 'o';
                        }
                    }
                }

                argv2[i] = argv[i];
            }
        } else {
            for (int i = 1; i < argc; i++) {
                argv2[i] = argv[i];
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    argv2[0]        = cc;
    argv2[argc]     = targetArg;
    argv2[argc + 1] = sysrootArg;

    if (action == ACTION_CREATE_SHARED_LIBRARY) {
        argv2[argc + 2] = (char*)"-fPIC";

        if (sonameArg[0] == '\0') {
            argv2[argc + 3] = NULL;
        } else {
            argv2[argc + 3] = sonameArg;
            argv2[argc + 4] = NULL;
        }
    } else {
        argv2[argc + 2] = NULL;
    }

    /////////////////////////////////////////////////////////////////

    for (int i = 0; ;i++) {
        if (argv2[i] == NULL) {
            break;
        } else {
            fprintf(stderr, "%s\n", argv2[i]);
        }
    }

    /////////////////////////////////////////////////////////////////

    execv (cc, argv2);
    perror(cc);
    return 255;
}

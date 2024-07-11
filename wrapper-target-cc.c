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

int main(int argc, char * argv[]) {
    char * const compiler = getenv("ANDROID_NDK_CC");

    if (compiler == NULL) {
        fprintf(stderr, "ANDROID_NDK_CC environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "ANDROID_NDK_CC environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    char * const baseArgs = getenv("ANDROID_NDK_COMPILER_ARGS");

    if (baseArgs == NULL) {
        fprintf(stderr, "ANDROID_NDK_COMPILER_ARGS environment variable is not set.\n");
        return 5;
    }

    if (baseArgs[0] == '\0') {
        fprintf(stderr, "ANDROID_NDK_COMPILER_ARGS environment variable value should be a non-empty string.\n");
        return 6;
    }

    /////////////////////////////////////////////////////////////////

    int action = 0;

    int staticFlag = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-E") == 0) {
            action = ACTION_PREPROCESS;
            break;
        }

        if (strcmp(argv[i], "-S") == 0) {
            action = ACTION_COMPILE;
            break;
        }

        if (strcmp(argv[i], "-c") == 0) {
            action = ACTION_ASSEMBLE;
            break;
        }

        if (strcmp(argv[i], "-shared") == 0) {
            action = ACTION_CREATE_SHARED_LIBRARY;
            break;
        }

        if (staticFlag == 1) {
            continue;
        }

        if (strcmp(argv[i], "-static") == 0 || strcmp(argv[i], "--static") == 0) {
            staticFlag = 1;
        }
    }

    if (action == 0) {
        if (staticFlag == 1) {
            action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
        }
    }

    /////////////////////////////////////////////////////////////////

    size_t baseArgc = 1U;

    for (size_t i = 0U; ; i++) {
        if (baseArgs[i] == '\0') {
            break;
        }

        if (baseArgs[i] == ' ') {
            baseArgc++;
        }
    }

    /////////////////////////////////////////////////////////////////

    char* argv2[argc + baseArgc + 5];

    char sonameArg[100]; sonameArg[0] = '\0';

    if (action == ACTION_PREPROCESS || action == ACTION_COMPILE || action == ACTION_ASSEMBLE) {
        for (int i = 1; i < argc; i++) {
            argv2[i] = argv[i];
        }
    } else if (action == ACTION_CREATE_SHARED_LIBRARY) {
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

                int ret = snprintf(sonameArg, n + 13, "-Wl,-soname,%s", s);

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
        const char * msle = getenv("PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE");

        if (msle != NULL && strcmp(msle, "1") == 0) {
            for (int i = 1; i < argc; i++) {
                if (argv[i][0] == '/') {
                    int len = 0;
                    int dotIndex = -1;

                    for (int j = 0; ; j++) {
                        if (argv[i][j] == '\0') {
                            len = j;
                            break;
                        }

                        if (argv[i][j] == '.') {
                            dotIndex = j;
                        }
                    }

                    if (dotIndex > 0) {
                        if (len - dotIndex == 3) {
                            if (strcmp(&argv[i][dotIndex], ".so") == 0) {
                                argv[i][dotIndex + 1] = 'a' ;
                                argv[i][dotIndex + 2] = '\0';

                                struct stat st;

                                if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                                    argv[i][dotIndex + 1] = 's';
                                    argv[i][dotIndex + 2] = 'o';
                                }
                            }
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

    char * p = baseArgs;

    for (size_t i = 0U; ; i++) {
        if (baseArgs[i] == '\0') {
            if (p[0] != '\0') {
                argv2[argc++] = p;
            }
            break;
        }

        if (baseArgs[i] == ' ') {
            baseArgs[i] = '\0';

            if (p[0] != '\0') {
                argv2[argc++] = p;
            }

            p = &baseArgs[i + 1];
        }
    }

    /////////////////////////////////////////////////////////////////

    if (action == ACTION_ASSEMBLE || action == ACTION_CREATE_SHARED_LIBRARY) {
        argv2[argc++] = (char*)"-fPIC";

        if (sonameArg[0] != '\0') {
            argv2[argc++] = sonameArg;
        }
    }

    argv2[argc++] = NULL;
    argv2[0] = compiler;

    /////////////////////////////////////////////////////////////////

    const char * verbose = getenv("NDKPKG_VERBOSE");

    if (verbose != NULL && strcmp(verbose, "1") == 0) {
        fprintf(stderr, "action=%d\n", action);

        for (int i = 0; ;i++) {
            if (argv2[i] == NULL) {
                break;
            } else {
                fprintf(stderr, "%s\n", argv2[i]);
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    execv (compiler, argv2);
    perror(compiler);
    return 255;
}

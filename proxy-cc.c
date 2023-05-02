#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ACTION_COMPILE 1
#define ACTION_CREATE_SHARED_LIBRARY 2
#define ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE 3
#define ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE 4

int main(int argc, char * argv[]) {
    const char * options[5] = { "-shared", "-static", "--static", "-pie", "-c" };
          int    indexes[5] = {    -1,         -1,        -1,       -1,    -1  };

    for (int i = 1; i < argc; i++) {
        for (int j = 0; j < 5; j++) {
            if (strcmp(argv[i], options[j]) == 0) {
                indexes[j] = i;
            }
        }
    }

    // printf("      -shared = %d\n", indexes[0]);
    // printf("      -static = %d\n", indexes[1]);
    // printf("     --static = %d\n", indexes[2]);
    // printf("         -pie = %d\n", indexes[3]);
    // printf("           -c = %d\n", indexes[4]);

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
        return 1;
    }

    if (TARGET[0] == '\0') {
        fprintf(stderr, "ANDROID_TARGET environment variable value should be a non-empty string.\n");
        return 2;
    }

    size_t   targetArgLength = strlen(TARGET) + 10;
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

    size_t   sysrootArgLength = strlen(SYSROOT) + 11;
    char     sysrootArg[sysrootArgLength];
    snprintf(sysrootArg, sysrootArgLength, "--sysroot=%s", SYSROOT);

    /////////////////////////////////////////////////////////////////

    char * argv2[argc + 3];

    argv2[0] = cc;
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
    } else if (action == ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-Wl,--export-dynamic") == 0) {
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

    argv2[argc + 2] = NULL;

    for (int i = 0; argv2[i] != NULL; i++) {
        printf("%s ", argv2[i]);
    }
    printf("\n");

    execv (cc, argv2);
    perror(cc);
    return -1;
}

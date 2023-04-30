#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ACTION_COMPILE 1
#define ACTION_CREATE_SHARED_LIBRARY 2
#define ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE 3
#define ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE 4

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

    int action = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            action = ACTION_COMPILE;
            break;
        }

        if (strcmp(argv[i], "-shared") == 0) {
            action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
            break;
        }

        if (strcmp(argv[i], "-static") == 0) {
            action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
            break;
        }

        if (strcmp(argv[i], "--static") == 0) {
            action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
            break;
        }

        if (strcmp(argv[i], "-pie") == 0) {
            action = ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE;
            break;
        }
    }

    /////////////////////////////////////////////////////////////////

    const char * const TARGET = getenv("ANDROID_TARGET");

    size_t   targetArgLength = strlen(TARGET) + 10;
    char     targetArg[targetArgLength];
    snprintf(targetArg, targetArgLength, "--target=%s", TARGET);

    /////////////////////////////////////////////////////////////////

    const char * const SYSROOT = getenv("ANDROID_NDK_SYSROOT");

    size_t   sysrootArgLength = strlen(SYSROOT) + 11;
    char     sysrootArg[sysrootArgLength];
    snprintf(sysrootArg, sysrootArgLength, "--sysroot=%s", SYSROOT);

    /////////////////////////////////////////////////////////////////

    char * argv2[argc + 3];

    argv2[0] = cc;
    argv2[1] = targetArg;
    argv2[2] = sysrootArg;

    if (action == ACTION_CREATE_SHARED_LIBRARY) {
    // if -shared option is passed, then remove -static , --static , -pie options if they also are passed
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
    // if -shared option is not passed, but -static or --static option is passed, then remove -pie , -Wl,-Bdynamic option if it also is passed
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-Wl,-Bdynamic") == 0) {
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
        printf("+%s ", argv2[i]);
    }
    printf("\n");

    execv (cc, argv2);
    perror(cc);
    return -1;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

int main(int argc, char * argv[]) {
    char * const compiler = getenv("PROXIED_CC_FOR_BUILD");

    if (compiler == NULL) {
        fprintf(stderr, "PROXIED_CC_FOR_BUILD environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "PROXIED_CC_FOR_BUILD environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    char * const baseArgs = getenv("CCFLAGS_FOR_BUILD");

    if (baseArgs == NULL) {
        fprintf(stderr, "CCFLAGS_FOR_BUILD environment variable is not set.\n");
        return 5;
    }

    if (baseArgs[0] == '\0') {
        fprintf(stderr, "CCFLAGS_FOR_BUILD environment variable value should be a non-empty string.\n");
        return 6;
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

    int createSharedLibrary = 0;

    char* args[argc + baseArgc + 2];

    args[0] = compiler;

    for (int i = 1; i < argc; i++) {
        args[i] = argv[i];

        if (createSharedLibrary == 0) {
            if (strcmp(argv[i], "-dynamiclib") == 0 || strcmp(argv[i], "-shared") == 0) {
                createSharedLibrary = 1;
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    char * p = baseArgs;

    for (size_t i = 0U; ; i++) {
        if (baseArgs[i] == '\0') {
            if (p[0] != '\0') {
                args[argc++] = p;
            }
            break;
        }

        if (baseArgs[i] == ' ') {
            baseArgs[i] = '\0';

            if (p[0] != '\0') {
                args[argc++] = p;
            }

            p = &baseArgs[i + 1];
        }
    }

    /////////////////////////////////////////////////////////////////

    if (createSharedLibrary == 1) {
        args[argc++] = (char*)"-fPIC";
    }

    args[argc++] = NULL;

    /////////////////////////////////////////////////////////////////

    const char * verbose = getenv("NDKPKG_VERBOSE");

    if (verbose != NULL && strcmp(verbose, "1") == 0) {
        for (int i = 0; ;i++) {
            if (args[i] == NULL) {
                break;
            } else {
                fprintf(stderr, "%s\n", args[i]);
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    execv (compiler, args);
    perror(compiler);
    return 255;
}

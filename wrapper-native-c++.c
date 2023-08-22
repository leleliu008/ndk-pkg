#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
    char * const cxxc = getenv("PROXIED_CXX_FOR_BUILD");

    if (cxxc == NULL) {
        fprintf(stderr, "PROXIED_CXX_FOR_BUILD environment variable is not set.\n");
        return 1;
    }

    if (cxxc[0] == '\0') {
        fprintf(stderr, "PROXIED_CXX_FOR_BUILD environment variable value should be a non-empty string.\n");
        return 2;
    }

    char* argv2[argc + 2];

    argv2[0] = cxxc;

    char * const sysroot = getenv("SYSROOT_FOR_BUILD");

    if (sysroot == NULL || sysroot[0] == '\0') {
        for (int i = 1; i < argc; i++) {
            argv2[i] = argv[i];
        }

        argv2[argc] = NULL;
    } else {
        argv2[1] = (char*)"-isysroot";
        argv2[2] = sysroot;

        for (int i = 1; i < argc; i++) {
            argv2[i + 2] = argv[i];
        }

        argv2[argc + 2] = NULL;
    }

    execv (cxxc, argv2);
    perror(cxxc);
    return 255;
}

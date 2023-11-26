#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
    char * const cc = getenv("PROXIED_CC_FOR_BUILD");

    if (cc == NULL) {
        fprintf(stderr, "PROXIED_CC_FOR_BUILD environment variable is not set.\n");
        return 1;
    }

    if (cc[0] == '\0') {
        fprintf(stderr, "PROXIED_CC_FOR_BUILD environment variable value should be a non-empty string.\n");
        return 2;
    }

    char* argv2[argc + 2];

    argv2[0] = cc;

    for (int i = 1; i < argc; i++) {
        argv2[i] = argv[i];
    }

    char * const sysroot = getenv("SYSROOT_FOR_BUILD");

    if (sysroot == NULL || sysroot[0] == '\0') {
        argv2[argc] = NULL;
    } else {
        argv2[argc]     = (char*)"-isysroot";
        argv2[argc + 1] = sysroot;
        argv2[argc + 2] = NULL;
    }

    execv (cc, argv2);
    perror(cc);
    return 255;
}

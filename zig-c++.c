#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
    char * const zig = getenv("ZIG");

    if (zig == NULL) {
        fprintf(stderr, "ZIG environment variable is not set.\n");
        return 1;
    }

    if (zig[0] == '\0') {
        fprintf(stderr, "ZIG environment variable value should be a non-empty string.\n");
        return 2;
    }

    const char* envs[] = { "CC", "CXX", "AS", "LD", NULL };

    for (int i = 0; ; i++) {
        if (envs[i] == NULL) {
            break;
        }

        if (unsetenv(envs[i]) == -1) {
            perror(NULL);
            return 3;
        }
    }

    char* argv2[argc + 2];

    argv2[0] = zig;
    argv2[1] = (char*)"c++";
    argv2[argc+1] = NULL;

    for (int i = 1; i < argc; i++) {
        argv2[i+1] = argv[i];
    }

    execv (zig, argv2);
    perror(zig);
    return 255;
}

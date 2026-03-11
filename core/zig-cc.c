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

    char* args[argc + 3];

    args[0] = zig;
    args[1] = (char*)"cc";
    args[argc+1] = (char*)"--target=x86_64-linux-gnu";
    args[argc+2] = NULL;

    for (int i = 1; i < argc; i++) {
        args[i+1] = argv[i];
    }

    execv (zig, args);
    perror(zig);
    return 255;
}

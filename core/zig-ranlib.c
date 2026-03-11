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

    char* args[argc + 2];

    args[0] = zig;
    args[1] = (char*)"ranlib";
    args[argc+1] = NULL;

    for (int i = 1; i < argc; i++) {
        args[i+1] = argv[i];
    }

    execv (zig, args);
    perror(zig);
    return 255;
}

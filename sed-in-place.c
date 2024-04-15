#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char * const sed = getenv("SED");

    if (sed == NULL || sed[0] == '\0') {
        fprintf(stderr, "SED environment variable was not set.\n");
        return 1;
    }

    /////////////////////////////////////////////////////////////////

    char* argv2[argc + 2];

    argv2[0] = sed;
    argv2[1] = (char*)"-i";

    for (int i = 1; i < argc; i++) {
        argv2[i + 1] = argv[i];
    }

    argv2[argc + 1] = NULL;

    for (int i = 0; argv2[i] != NULL; i++) {
        fprintf(stderr, "%s ", argv2[i]);
    }
    fprintf(stderr, "\n");

    execv (sed, argv2);
    perror(sed);
    return 255;
}

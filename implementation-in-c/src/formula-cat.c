#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include "ndk-pkg.h"

int ndkpkg_formula_cat(const char * packageName) {
    char * formulaFilePath = NULL;

    int ret = ndkpkg_formula_locate(packageName, &formulaFilePath);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    int fd = open(formulaFilePath, O_RDONLY);

    if (fd == -1) {
        perror(formulaFilePath);
        free(formulaFilePath);
        return NDKPKG_ERROR;
    }

    printf("formula: %s\n", formulaFilePath);

    free(formulaFilePath);
    formulaFilePath = NULL;

    char buf[1024];

    for(;;) {
        ssize_t readSize = read(fd, buf, 1024);

        if (readSize == -1) {
            perror(formulaFilePath);
            close(fd);
            return NDKPKG_ERROR;
        }

        if (readSize == 0) {
            close(fd);
            return NDKPKG_OK;
        }

        ssize_t writeSize = write(STDOUT_FILENO, buf, readSize);

        if (writeSize == -1) {
            perror(NULL);
            close(fd);
            return NDKPKG_ERROR;
        }

        if (writeSize != readSize) {
            fprintf(stderr, "not fully written to stdout.");
            close(fd);
            return NDKPKG_ERROR;
        }
    }
}

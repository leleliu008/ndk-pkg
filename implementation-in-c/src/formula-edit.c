#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ndk-pkg.h"

int ndkpkg_formula_edit(const char * packageName, const char * editor) {
    char * formulaFilePath = NULL;

    int ret = ndkpkg_formula_locate(packageName, &formulaFilePath);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    if (editor == NULL || editor[0] == '\0') {
        editor = "vim";
    }

    if (editor[0] == '/') {
        execl (editor, editor, formulaFilePath, NULL);
    } else {
        execlp(editor, editor, formulaFilePath, NULL);
    }

    perror(editor);

    free(formulaFilePath);

    return NDKPKG_ERROR;
}

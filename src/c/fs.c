#include <sys/stat.h>
#include <unistd.h>
#include "fs.h"

bool exists_and_is_a_directory(const char* dirpath) {
    struct stat sb;
    return (stat(dirpath, &sb) == 0) && S_ISDIR(sb.st_mode);
}

bool exists_and_is_a_regular_file(const char* filepath) {
    struct stat sb;
    return (stat(filepath, &sb) == 0) && S_ISREG(sb.st_mode);
}

bool exists_and_is_readable(const char* filepath) {
    return access(filepath, R_OK) == 0;
}

bool exists_and_is_writable(const char* filepath) {
    return access(filepath, W_OK) == 0;
}

bool exists_and_is_executable(const char* filepath) {
    return access(filepath, X_OK) == 0;
}

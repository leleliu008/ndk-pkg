#ifndef FS_H
#define FS_H
#include <stdbool.h>

bool exists_and_is_a_directory(const char* dirpath);
bool exists_and_is_a_regular_file(const char* filepath);
bool exists_and_is_readable(const char* filepath);
bool exists_and_is_writable(const char* filepath);
bool exists_and_is_executable(const char* filepath);

#endif

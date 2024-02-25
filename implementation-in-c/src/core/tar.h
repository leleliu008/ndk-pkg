#ifndef _CORE_TAR_H
#define _CORE_TAR_H

#include <stdlib.h>
#include <stdbool.h>

#include <archive.h>
#include <archive_entry.h>

typedef enum {
    ArchiveType_tar_gz,
    ArchiveType_tar_xz,
    ArchiveType_tar_lz,
    ArchiveType_tar_bz2,
    ArchiveType_zip,
} ArchiveType;

int tar_list(const char * inputFilePath, const int flags);

int tar_create(const char * inputDir, const char * outputFilePath, const ArchiveType type, const bool verbose);

int tar_extract(const char * outputDir, const char * inputFilePath, const int flags, const bool verbose, const size_t stripComponentsNumber);

#endif

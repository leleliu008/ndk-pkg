#include <errno.h>
#include <string.h>

#include <locale.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#include "tar.h"

int tar_list(const char * inputFilePath, const int flags) {
	if ((inputFilePath != NULL) && (strcmp(inputFilePath, "-") == 0)) {
		inputFilePath = NULL;
    }

    struct archive *ar = archive_read_new();
    struct archive *aw = archive_write_disk_new();

    archive_read_support_format_all(ar);
    archive_read_support_filter_all(ar);

    archive_write_disk_set_options(aw, flags);

    struct archive_entry *entry = NULL;

    int ret = 0;

	if ((ret = archive_read_open_filename(ar, inputFilePath, 10240))) {
        goto finalize;
    }

	for (;;) {
		ret = archive_read_next_header(ar, &entry);

		if (ret == ARCHIVE_EOF) {
			break;
        }

		if (ret == ARCHIVE_OK) {
		    printf("%s\n", archive_entry_pathname(entry));
        } else {
            goto finalize;
        }
	}

finalize:
    if (ret != ARCHIVE_OK) {
        fprintf(stdout, "%s\n", archive_error_string(ar));
    }

	archive_read_close(ar);
	archive_read_free(ar);

	archive_write_close(aw);
  	archive_write_free(aw);

    return ret;
}

int tar_extract(const char * outputDir, const char * inputFilePath, const int flags, const bool verbose, const size_t stripComponentsNumber) {
    if ((inputFilePath != NULL) && (strcmp(inputFilePath, "-") == 0)) {
		inputFilePath = NULL;
    }

    // https://github.com/libarchive/libarchive/issues/459
    setlocale(LC_ALL, "");

	struct archive *ar = archive_read_new();
	struct archive *aw = archive_write_disk_new();

    archive_read_support_format_all(ar);
    archive_read_support_filter_all(ar);

	archive_write_disk_set_options(aw, flags);

	struct archive_entry *entry = NULL;

	int ret = 0;

	if ((ret = archive_read_open_filename(ar, inputFilePath, 10240))) {
        goto finalize;
    }

	for (;;) {
		ret = archive_read_next_header(ar, &entry);

		if (ret == ARCHIVE_EOF) {
            ret =  ARCHIVE_OK;
			break;
        }

		if (ret != ARCHIVE_OK) {
            goto finalize;
        }

        ////////////////////////////////////////////////////////////////////////////////////

        const char * entry_pathname = archive_entry_pathname(entry);

        if (stripComponentsNumber > 0U) {
            size_t entry_pathname_length = strlen(entry_pathname);

            for (size_t i = 0; i < entry_pathname_length; i++) {
                if (entry_pathname[i] == '/') {
                    entry_pathname = entry_pathname + i + 1U;
                    break;
                }
            }

            if (entry_pathname[0] == '\0') {
                continue;
            }
        }

		if (verbose) {
			printf("x %s\n", entry_pathname);
        }

        if ((outputDir != NULL) && (outputDir[0] != '\0')) {
            size_t outputFilePathLength = strlen(outputDir) + strlen(entry_pathname) + 2U;
            char   outputFilePath[outputFilePathLength];

            ret = snprintf(outputFilePath, outputFilePathLength, "%s/%s", outputDir, entry_pathname);

            if (ret < 0) {
                perror(NULL);
                return -1;
            }

            archive_entry_set_pathname(entry, outputFilePath);
        } else {
            archive_entry_set_pathname(entry, entry_pathname);
        }

        ////////////////////////////////////////////////////////////////////////////////////

        const char * hardlinkname = archive_entry_hardlink(entry);

        if (hardlinkname != NULL) {
            if (stripComponentsNumber > 0U) {
                size_t hardlinkname_length = strlen(hardlinkname);
                for (size_t i = 0; i < hardlinkname_length; i++) {
                    if (hardlinkname[i] == '/') {
                        hardlinkname = hardlinkname + i + 1U;
                        break;
                    }
                }

                if ((outputDir != NULL) && (outputDir[0] != '\0')) {
                    size_t outputFilePathLength = strlen(outputDir) + strlen(hardlinkname) + 2U;
                    char   outputFilePath[outputFilePathLength];

                    ret = snprintf(outputFilePath, outputFilePathLength, "%s/%s", outputDir, hardlinkname);

                    if (ret < 0) {
                        perror(NULL);
                        return -1;
                    }

                    archive_entry_set_hardlink_utf8(entry, outputFilePath);
                } else {
                    archive_entry_set_hardlink_utf8(entry, hardlinkname);
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////

        ret = archive_write_header(aw, entry);

        if (ret != ARCHIVE_OK) {
            goto finalize;
        }

        const void * dataBuff;
        size_t       dataSize;
#if ARCHIVE_VERSION_NUMBER >= 3000000
        int64_t offset;
#else
        off_t   offset;
#endif

        for (;;) {
            ret = archive_read_data_block(ar, &dataBuff, &dataSize, &offset);

            if (ret == ARCHIVE_EOF) {
                break;
            }

            if (ret != ARCHIVE_OK) {
                goto finalize;
            }

            ret = archive_write_data_block(aw, dataBuff, dataSize, offset);

            if (ret != ARCHIVE_OK) {
                goto finalize;
            }
        }

        ret = archive_write_finish_entry(aw);

        if (ret != ARCHIVE_OK) {
            goto finalize;
        }
	}

finalize:
    if (ret != ARCHIVE_OK) {
        fprintf(stdout, "%s\n", archive_error_string(ar));
    }

	archive_read_close(ar);
	archive_read_free(ar);
	
	archive_write_close(aw);
  	archive_write_free(aw);

    return ret;
}

typedef struct {
    char ** array;
    size_t  size;
    size_t  capcity;
} StringArrayList;

int list_files(const char * dirPath, const bool verbose, StringArrayList * stringArrayList) {
    if ((dirPath == NULL) || (dirPath[0] == '\0')) {
        return -1;
    }

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return -2;
    }

    int ret = 0;

    struct stat st;

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return 0;
            } else {
                perror(dirPath);
                closedir(dir);
                return -3;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filePathLength = strlen(dirPath) + strlen(dir_entry->d_name) + 2U;
        char   filePath[filePathLength];

        ret = snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return -1;
        }

        //if (verbose) printf("%s\n", filePath);

        if (stat(filePath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                ret = list_files(filePath, verbose, stringArrayList);

                if (ret != 0) {
                    return ret;
                }
            } else {
                if (stringArrayList->size == stringArrayList->capcity) {
                    size_t  newCapacity = stringArrayList->capcity + 10U;
                    char ** p = (char**)realloc(stringArrayList->array, newCapacity * sizeof(char*));

                    if (p == NULL) {
                        if (stringArrayList->array != NULL) {
                            for (size_t i = 0; i < stringArrayList->size; i++) {
                                free(stringArrayList->array[i]);
                                stringArrayList->array[i] = NULL;
                            }

                            free(stringArrayList->array);
                            stringArrayList->array = NULL;
                        }

                        return -4;
                    }

                    if (stringArrayList->array != NULL) {
                        memset(p + stringArrayList->size, 0, 10);
                    }

                    stringArrayList->array   = p;
                    stringArrayList->capcity = newCapacity;
                }

                char * p2 = strdup(filePath);

                if (p2 == NULL) {
                    for (size_t i = 0; i < stringArrayList->size; i++) {
                        free(stringArrayList->array[i]);
                        stringArrayList->array[i] = NULL;
                    }

                    free(stringArrayList->array);
                    stringArrayList->array = NULL;

                    return -4;
                }

                stringArrayList->array[stringArrayList->size] = p2;
                stringArrayList->size += 1U;
            }
        } else {
            return -5;
        }
    }
}

int tar_create(const char * inputDir, const char * outputFilePath, const ArchiveType type, const bool verbose) {
    StringArrayList stringArrayList = {0};

    int ret = list_files(inputDir, verbose, &stringArrayList);

    if (ret != 0) {
        return ret;
    }

    if (stringArrayList.size == 0U) {
        return 1;
    }

    if ((outputFilePath != NULL) && (strcmp(outputFilePath, "-") == 0)) {
		outputFilePath = NULL;
    }

    // https://github.com/libarchive/libarchive/issues/459
    setlocale(LC_ALL, "");

	struct archive *aw = archive_write_new();

    switch (type) {
        case ArchiveType_tar_gz:
            archive_write_set_format_ustar(aw);
            archive_write_add_filter_gzip(aw);
            break;
        case ArchiveType_tar_lz:
            archive_write_set_format_ustar(aw);
            archive_write_add_filter_lzip(aw);
            break;
        case ArchiveType_tar_xz:
            archive_write_set_format_ustar(aw);
            archive_write_add_filter_xz(aw);
            break;
        case ArchiveType_tar_bz2:
            archive_write_set_format_ustar(aw);
            archive_write_add_filter_bzip2(aw);
            break;
        case ArchiveType_zip:
            archive_write_set_format_zip(aw);
            break;
    }

    archive_write_open_filename(aw, outputFilePath);

    struct archive       *ar    = NULL;
	struct archive_entry *entry = NULL;

    int fd;

    for (size_t i = 0; i < stringArrayList.size; i++) {
        ar = archive_read_disk_new();

        ret = archive_read_disk_open(ar, stringArrayList.array[i]);

        if (ret != ARCHIVE_OK) {
            goto finalize;
        }

        for (;;) {
            entry = archive_entry_new();

            ret = archive_read_next_header2(ar, entry);

            if (ret == ARCHIVE_EOF) {
                ret =  ARCHIVE_OK;
                archive_entry_free(entry);
                break;
            }

            if (ret != ARCHIVE_OK) {
                archive_entry_free(entry);
                goto finalize;
            }

            archive_read_disk_descend(ar);

            if (verbose) {
                fprintf(stderr, "a %s\n", archive_entry_pathname(entry));
            }

            ret = archive_write_header(aw, entry);

            if ((ret == ARCHIVE_RETRY) || (ret == ARCHIVE_WARN)) {
                ret =  ARCHIVE_OK;
            }

            if (ret != ARCHIVE_OK) {
                archive_entry_free(entry);
                goto finalize;
            }

            fd = open(archive_entry_sourcepath(entry), O_RDONLY);

            char buff[16384];
            size_t len;

            len = read(fd, buff, sizeof(buff));

            while (len > 0U) {
                archive_write_data(aw, buff, len);
                len = read(fd, buff, sizeof(buff));
            }

            close(fd);

            archive_entry_free(entry);
        }

        archive_read_close(ar);
        archive_read_free(ar);
    }

finalize:
    if (ret != ARCHIVE_OK) {
        fprintf(stdout, "%s\n", archive_error_string(ar));
        archive_read_close(ar);
        archive_read_free(ar);
    }

	archive_write_close(aw);
  	archive_write_free(aw);

    if (stringArrayList.size > 0U) {
        for (size_t i = 0; i < stringArrayList.size; i++) {
            free(stringArrayList.array[i]);
            stringArrayList.array[i] = NULL;
        }
        free(stringArrayList.array);
        stringArrayList.array = NULL;
    }

    return ret;
}

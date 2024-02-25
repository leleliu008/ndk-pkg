#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <openssl/evp.h>

#include "core/zlib-flate.h"
#include "core/base16.h"
#include "core/base64.h"
#include "core/exe.h"
#include "core/log.h"

#include "sha256sum.h"
#include "ndk-pkg.h"

//invoked as 'ndkpkg util <CMD> [ARGUMENTS]'
int ndkpkg_util(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s %s <COMMAND> , <COMMAND> is unspecified.\n", argv[0], argv[1]);
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (argv[2][0] == '\0') {
        fprintf(stderr, "Usage: %s %s <COMMAND> , <COMMAND> should be a non-empty string.\n", argv[0], argv[1]);
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    if (strcmp(argv[2], "sed-in-place") == 0) {
        char * const sed = getenv("SED");

        if (sed == NULL || sed[0] == '\0') {
            fprintf(stderr, "SED environment variable was not set.\n");
            return 1;
        }

        char* argv2[argc + 2];

        argv2[0] = sed;
        argv2[1] = (char*)"-i";

        for (int i = 3; i < argc; i++) {
            argv2[i - 1] = argv[i];
        }

        argv2[argc - 1] = NULL;

        for (int i = 0; argv2[i] != NULL; i++) {
            fprintf(stderr, "%s ", argv2[i]);
        }
        fprintf(stderr, "\n");

        execv (sed, argv2);
        perror(sed);
        return 255;
    }

    if (strcmp(argv[2], "base16-encode") == 0) {
        if (argv[3] == NULL) {
            unsigned char inputBuf[1024];

            for (;;) {
                ssize_t readSizeInBytes = read(STDIN_FILENO, inputBuf, 1024);

                if (readSizeInBytes == -1) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if (readSizeInBytes == 0) {
                    if (isatty(STDOUT_FILENO)) {
                        printf("\n");
                    }

                    return NDKPKG_OK;
                }

                size_t outputBufSizeInBytes = readSizeInBytes << 1;
                char   outputBuf[outputBufSizeInBytes];

                if (base16_encode(outputBuf, inputBuf, readSizeInBytes, true) != 0) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                ssize_t writeSizeInBytes = write(STDOUT_FILENO, outputBuf, outputBufSizeInBytes);

                if (writeSizeInBytes == -1) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if ((size_t)writeSizeInBytes != outputBufSizeInBytes) {
                    fprintf(stderr, "not fully written to stdout.\n");
                    return NDKPKG_ERROR;
                }
            }
        } else {
            if (argv[3][0] == '\0') {
                fprintf(stderr, "Usage: %s %s %s <STR> , <STR> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return NDKPKG_ERROR_ARG_IS_NULL;
            }

            unsigned char * inputBuf = (unsigned char *)argv[3];
            size_t          inputBufSizeInBytes = strlen(argv[3]);

            size_t outputBufSizeInBytes = inputBufSizeInBytes << 1;
            char   outputBuf[outputBufSizeInBytes];

            if (base16_encode(outputBuf, inputBuf, inputBufSizeInBytes, true) != 0) {
                return NDKPKG_ERROR;
            }

            ssize_t writeSizeInBytes = write(STDOUT_FILENO, outputBuf, outputBufSizeInBytes);

            if (writeSizeInBytes == -1) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if ((size_t)writeSizeInBytes != outputBufSizeInBytes) {
                fprintf(stderr, "not fully written to stdout.\n");
                return NDKPKG_ERROR;
            }

            if (isatty(STDOUT_FILENO)) {
                printf("\n");
            }

            return NDKPKG_OK;
        }
    }

    if (strcmp(argv[2], "base16-decode") == 0) {
        if (argv[3] == NULL) {
            fprintf(stderr, "Usage: %s %s %s <BASE16-ENCODED-STR> , <BASE16-ENCODED-STR> is unspecified.\n", argv[0], argv[1], argv[2]);
            return NDKPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[3][0] == '\0') {
            fprintf(stderr, "Usage: %s %s %s <BASE16-ENCODED-STR> , <BASE16-ENCODED-STR> should be non-empty.\n", argv[0], argv[1], argv[2]);
            return NDKPKG_ERROR_ARG_IS_NULL;
        }

        size_t inputBufSizeInBytes = strlen(argv[3]);

        if ((inputBufSizeInBytes & 1) != 0) {
            fprintf(stderr, "Usage: %s %s %s <BASE16-ENCODED-STR> , <BASE16-ENCODED-STR> length should be an even number.\n", argv[0], argv[1], argv[2]);
            return NDKPKG_ERROR_ARG_IS_INVALID;
        }

        size_t        outputBufSizeInBytes = inputBufSizeInBytes >> 1;
        unsigned char outputBuf[outputBufSizeInBytes];

        if (base16_decode(outputBuf, argv[3], inputBufSizeInBytes) == 0) {
            ssize_t writeSizeInBytes = write(STDOUT_FILENO, outputBuf, outputBufSizeInBytes);

            if (writeSizeInBytes == -1) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if ((size_t)writeSizeInBytes != outputBufSizeInBytes) {
                fprintf(stderr, "not fully written to stdout.\n");
                return NDKPKG_ERROR;
            }

            if (isatty(STDOUT_FILENO)) {
                printf("\n");
            }

            return NDKPKG_OK;
        } else {
            perror(NULL);

            if (errno == EINVAL) {
                return NDKPKG_ERROR_ARG_IS_INVALID;
            } else {
                return NDKPKG_ERROR;
            }
        }
    }

    if (strcmp(argv[2], "base64-encode") == 0) {
        if (argv[3] == NULL) {
            unsigned char inputBuf[1023];

            for (;;) {
                ssize_t readSizeInBytes = read(STDIN_FILENO, inputBuf, 1023);

                if (readSizeInBytes == -1) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if (readSizeInBytes == 0) {
                    if (isatty(STDOUT_FILENO)) {
                        printf("\n");
                    }

                    return NDKPKG_OK;
                }

                unsigned int  x = (readSizeInBytes % 3) == 0 ? 0 : 1;
                unsigned int  outputBufSizeInBytes = (readSizeInBytes / 3 + x) << 2;
                unsigned char outputBuf[outputBufSizeInBytes];

                int ret = EVP_EncodeBlock(outputBuf, inputBuf, readSizeInBytes);

                if (ret < 0) {
                    return ret;
                }

                ssize_t writeSizeInBytes = write(STDOUT_FILENO, outputBuf, outputBufSizeInBytes);

                if (writeSizeInBytes == -1) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if ((size_t)writeSizeInBytes != outputBufSizeInBytes) {
                    fprintf(stderr, "not fully written to stdout.\n");
                    return NDKPKG_ERROR;
                }
            }
        } else {
            if (argv[3][0] == '\0') {
                fprintf(stderr, "Usage: %s %s %s <STR> , <STR> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return NDKPKG_ERROR_ARG_IS_NULL;
            }

            unsigned char * inputBuf = (unsigned char *)argv[3];
            unsigned int    inputBufSizeInBytes = strlen(argv[3]);

            unsigned int  x = (inputBufSizeInBytes % 3) == 0 ? 0 : 1;
            unsigned int  outputBufSizeInBytes = (inputBufSizeInBytes / 3 + x) << 2;
            unsigned char outputBuf[outputBufSizeInBytes];

            int ret = EVP_EncodeBlock(outputBuf, inputBuf, inputBufSizeInBytes);

            if (ret < 0) {
                return ret;
            }

            ssize_t writeSizeInBytes = write(STDOUT_FILENO, outputBuf, outputBufSizeInBytes);

            if (writeSizeInBytes == -1) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if ((size_t)writeSizeInBytes != outputBufSizeInBytes) {
                fprintf(stderr, "not fully written to stdout.\n");
                return NDKPKG_ERROR;
            }

            if (isatty(STDOUT_FILENO)) {
                printf("\n");
            }

            return NDKPKG_OK;
        }
    }

    if (strcmp(argv[2], "base64-decode") == 0) {
        if (argv[3] == NULL) {
            unsigned char readBuf[1024];

            for (;;) {
                ssize_t readSizeInBytes = read(STDIN_FILENO, readBuf, 1024);

                if (readSizeInBytes == -1) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if (readSizeInBytes == 0) {
                    if (isatty(STDOUT_FILENO)) {
                        printf("\n");
                    }

                    return NDKPKG_OK;
                }

                unsigned int  outputBufSizeInBytes = (readSizeInBytes >> 2) * 3;
                unsigned char outputBuf[outputBufSizeInBytes];

                // EVP_DecodeBlock() returns the length of the data decoded or -1 on error.
                int n = EVP_DecodeBlock(outputBuf, readBuf, readSizeInBytes);

                if (n < 0) {
                    return NDKPKG_ERROR_ARG_IS_INVALID;
                }

                ssize_t writeSizeInBytes = write(STDOUT_FILENO, outputBuf, outputBufSizeInBytes);

                if (writeSizeInBytes == -1) {
                    perror(NULL);
                    return NDKPKG_ERROR;
                }

                if ((size_t)writeSizeInBytes != outputBufSizeInBytes) {
                    fprintf(stderr, "not fully written to stdout.\n");
                    return NDKPKG_ERROR;
                }
            }
        } else {
            if (argv[3][0] == '\0') {
                fprintf(stderr, "Usage: %s %s %s <BASE64-ENCODED-STR> , <BASE64-ENCODED-STR> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return NDKPKG_ERROR_ARG_IS_NULL;
            }

            unsigned char * inputBuf = (unsigned char *)argv[3];
            unsigned int    inputBufSizeInBytes = strlen(argv[3]);

            unsigned int  outputBufSizeInBytes = (inputBufSizeInBytes >> 2) * 3;
            unsigned char outputBuf[outputBufSizeInBytes];

            // EVP_DecodeBlock() returns the length of the data decoded or -1 on error.
            int n = EVP_DecodeBlock(outputBuf, inputBuf, inputBufSizeInBytes);

            if (n < 0) {
                return NDKPKG_ERROR_ARG_IS_INVALID;
            }

            ssize_t writeSizeInBytes = write(STDOUT_FILENO, outputBuf, outputBufSizeInBytes);

            if (writeSizeInBytes == -1) {
                perror(NULL);
                return NDKPKG_ERROR;
            }

            if ((size_t)writeSizeInBytes != outputBufSizeInBytes) {
                fprintf(stderr, "not fully written to stdout.\n");
                return NDKPKG_ERROR;
            }

            if (isatty(STDOUT_FILENO)) {
                printf("\n");
            }

            return NDKPKG_OK;
        }
    }

    if (strcmp(argv[2], "sha256sum") == 0) {
        if (argv[3] == NULL || strcmp(argv[3], "-") == 0) {
            char outputBuf[65] = {0};

            if (sha256sum_of_stream(outputBuf, stdin) == 0) {
                printf("%s\n", outputBuf);
                return NDKPKG_OK;
            } else {
                perror(NULL);
                return NDKPKG_ERROR;
            }
        } else if (strcmp(argv[3], "-h") == 0 || strcmp(argv[3], "--help") == 0) {
            fprintf(stderr, "Usage: %s %s %s [FILEPATH]\n", argv[0], argv[1], argv[2]);
            return NDKPKG_OK;
        } else {
            char outputBuf[65] = {0};

            if (sha256sum_of_file(outputBuf, argv[3]) == 0) {
                printf("%s\n", outputBuf);
                return NDKPKG_OK;
            } else {
                return NDKPKG_ERROR;
            }
        }
    }

    if (strcmp(argv[2], "zlib-deflate") == 0) {
        int level = 1;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-L") == 0) {
                char * p = argv[i + 1];

                if (p == NULL) {
                    fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                    return NDKPKG_ERROR;
                }

                if (strlen(p) != 1) {
                    fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                    return NDKPKG_ERROR;
                }

                if (p[0] < '1' || p[0] > '9') {
                    fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                    return NDKPKG_ERROR;
                }

                level = atoi(p);

                i++;
            } else {
                fprintf(stderr, "unrecognized option: %s", argv[i]);
                fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                return NDKPKG_ERROR;
            }
        }

        return zlib_deflate_file_to_file(stdin, stdout, level);
    }

    if (strcmp(argv[2], "zlib-inflate") == 0) {
        return zlib_inflate_file_to_file(stdin, stdout);
    }

    if (strcmp(argv[2], "which") == 0) {
        if (argv[3] == NULL) {
            fprintf(stderr, "USAGE: %s %s %s <COMMAND-NAME> , <COMMAND-NAME> is unspecified.\n", argv[0], argv[1], argv[2]);
            return 1;
        }

        if (argv[3][0] == '\0') {
            fprintf(stderr, "USAGE: %s %s %s <COMMAND-NAME> , <COMMAND-NAME> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
            return 1;
        }

        bool findAll = false;

        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "-a") == 0) {
                findAll = true;
            } else {
                fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
                fprintf(stderr, "USAGE: %s %s %s <COMMAND-NAME> [-a]\n", argv[0], argv[1], argv[2]);
                return 1;
            }
        }

        char ** pathList = NULL;

        int ret = exe_search(argv[3], &pathList, findAll);

        if (ret > 0) {
            for (int i = 0; i < ret; i++) {
                printf("%s\n", pathList[i]);

                free(pathList[i]);
                pathList[i] = NULL;
            }

            free(pathList);
            pathList = NULL;

            return 0;
        }

        return ret;
    }

    if (strcmp(argv[2], "wfetch") == 0) {
        if (argv[3] == NULL) {
            fprintf(stderr, "USAGE: %s %s %s <URL> , <URL> is unspecified.\n", argv[0], argv[1], argv[2]);
            return 1;
        }

        if (argv[3][0] == '\0') {
            fprintf(stderr, "USAGE: %s %s %s <URL> , <URL> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
            return 1;
        }

        const char * url = argv[3];
        const char * uri = NULL;

        const char * expectedSHA256SUM = NULL;

        const char * outputPath = NULL;

        bool verbose = false;

        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else if (strncmp(argv[i], "--uri=", 6) == 0) {
                uri = &argv[i][6];

                if (uri[0] == '\0') {
                    fprintf(stderr, "USAGE: %s %s %s <URL> [--uri=<URI>] , <URI> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                    return NDKPKG_ERROR_ARG_IS_INVALID;
                }
            } else if (strncmp(argv[i], "--sha256=", 9) == 0) {
                expectedSHA256SUM = &argv[i][9];

                if (expectedSHA256SUM[0] == '\0') {
                    fprintf(stderr, "USAGE: %s %s %s <URL> [--uri=<URI>] , <URI> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                    return NDKPKG_ERROR_ARG_IS_INVALID;
                }

                if (strlen(expectedSHA256SUM) != 64U) {
                    fprintf(stderr, "USAGE: %s %s %s <URL> [--uri=<URI>] , <URI> should be a 64 length string.\n", argv[0], argv[1], argv[2]);
                    return NDKPKG_ERROR_ARG_IS_INVALID;
                }
            } else if (strcmp(argv[i], "-o") == 0) {
                outputPath = argv[++i];

                if (outputPath[0] == '\0') {
                    fprintf(stderr, "USAGE: %s %s %s <URL> [-o <OUTPUT-PATH>] , <OUTPUT-PATH> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                    return NDKPKG_ERROR_ARG_IS_INVALID;
                }
            } else {
                fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
                fprintf(stderr, "USAGE: %s %s %s <URL> [-v]\n", argv[0], argv[1], argv[2]);
                return 1;
            }
        }

        return ndkpkg_download(url, uri, expectedSHA256SUM, outputPath, verbose);
    }

    if (strcmp(argv[2], "uncompress") == 0) {
        if (argv[3] == NULL) {
            fprintf(stderr, "USAGE: %s %s %s <FILEPATH> , <FILEPATH> is unspecified.\n", argv[0], argv[1], argv[2]);
            return 1;
        }

        if (argv[3][0] == '\0') {
            fprintf(stderr, "USAGE: %s %s %s <FILEPATH> , <FILEPATH> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
            return 1;
        }

        const char * unpackDIR = NULL;

        size_t stripComponentNumber = 1;

        bool verbose = false;

        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else if (strcmp(argv[i], "-C") == 0) {
                unpackDIR = argv[++i];

                if (unpackDIR[0] == '\0') {
                    fprintf(stderr, "USAGE: %s %s %s <FILEPATH> [-C <DIR>] , <DIR> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                    return NDKPKG_ERROR_ARG_IS_INVALID;
                }
            } else if (strncmp(argv[i], "--strip-components=", 19) == 0) {
                if (argv[i][19] == '\0') {
                    fprintf(stderr, "USAGE: %s %s %s <URL> [--strip-components=<N>] , <N> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                    return NDKPKG_ERROR_ARG_IS_INVALID;
                }

                for (int j = 19; ;j++) {
                    if (argv[i][j] == '\0') {
                        break;
                    }

                    if (argv[i][j] < '0' || argv[i][j] > '0') {
                        fprintf(stderr, "USAGE: %s %s %s <URL> [--strip-components=<N>] , <N> should be a integer.\n", argv[0], argv[1], argv[2]);
                        return NDKPKG_ERROR_ARG_IS_INVALID;
                    }
                }

                stripComponentNumber = atoi(argv[i]);
            } else {
                fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
                fprintf(stderr, "USAGE: %s %s %s <URL> [-v]\n", argv[0], argv[1], argv[2]);
                return 1;
            }
        }

        return ndkpkg_uncompress(argv[3], unpackDIR, stripComponentNumber, verbose);
    }

    LOG_ERROR2("unrecognized command: ", argv[2]);
    return NDKPKG_ERROR_ARG_IS_UNKNOWN;
}

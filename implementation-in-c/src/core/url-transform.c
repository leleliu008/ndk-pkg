#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "url-transform.h"

static int url_transform_read(int inputFD, char outputBuffer[], size_t outputBufferSizeInBytes, size_t * writtenSizeInBytes) {
    char buf[1024];

    for (;;) {
        ssize_t readSize = read(inputFD, buf, 1024);

        if (readSize == 0) {
            return 0;
        }

        if (readSize < 0) {
            return -1;
        }

        if (buf[readSize - 1] == '\n') {
            readSize--;
        }

        if (readSize > 0) {
            size_t writtenSize = (*writtenSizeInBytes);
            size_t detlaSize = outputBufferSizeInBytes - writtenSize;
            size_t n = detlaSize > (size_t)readSize ? readSize : detlaSize;

            strncpy(outputBuffer + writtenSize, buf, n);

            (*writtenSizeInBytes) = writtenSize + n;

            if ((*writtenSizeInBytes) == outputBufferSizeInBytes) {
                return 0;
            }
        }
    }
}

int url_transform(const char * urlTransformCommandPath, const char * inUrl, char outputBuffer[], size_t outputBufferSizeInBytes, size_t * writtenSizeInBytes, const bool verbose) {
    struct stat st;

    if (stat(urlTransformCommandPath, &st) == 0) {
        if (access(urlTransformCommandPath, X_OK) != 0) {
            return -1;
        }
    } else {
        return -1;
    }

    ////////////////////////////////////////////////////////////////////

    int pipeFDs[2];

    if (pipe(pipeFDs) != 0) {
        return -1;
    }

    ////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        close(pipeFDs[0]);

        if (dup2(pipeFDs[1], STDOUT_FILENO) < 0) {
            if (verbose) {
                perror(NULL);
            }
            exit(254);
        }

        execl(urlTransformCommandPath, urlTransformCommandPath, inUrl, NULL);

        if (verbose) {
            perror(urlTransformCommandPath);
        }

        exit(255);
    } else {
        close(pipeFDs[1]);

        int err = 0;

        int ret = url_transform_read(pipeFDs[0], outputBuffer, outputBufferSizeInBytes, writtenSizeInBytes);

        if (ret != 0) {
            err = errno;
        }

        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            return -1;
        }

        if (childProcessExitStatusCode == 0) {
            errno = err;
            return ret;
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            if (verbose) {
                fprintf(stderr, "running command '%s %s' exit with status code: %d\n", urlTransformCommandPath, inUrl, WEXITSTATUS(childProcessExitStatusCode));
            }
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            if (verbose) {
                fprintf(stderr, "running command '%s %s' killed by signal: %d\n", urlTransformCommandPath, inUrl, WTERMSIG(childProcessExitStatusCode));
            }
            errno = EINTR;
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            if (verbose) {
                fprintf(stderr, "running command '%s %s' stopped by signal: %d\n", urlTransformCommandPath, inUrl, WSTOPSIG(childProcessExitStatusCode));
            }
            errno = EINTR;
        }

        return -1;
    }
}

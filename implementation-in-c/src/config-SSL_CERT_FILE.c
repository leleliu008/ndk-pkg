#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include "ndk-pkg.h"

int ndkpkg_setenv_SSL_CERT_FILE() {
    if (getenv("SSL_CERT_FILE") != NULL) {
        return NDKPKG_OK;
    }

    const char * ndkpkgHomeDIR = getenv("NDKPKG_HOME");

    if (ndkpkgHomeDIR == NULL || ndkpkgHomeDIR[0] == '\0') {
        const char * userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL || userHomeDIR[0] == '\0') {
            return NDKPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t cacertFilePathCapacity = strlen(userHomeDIR) + 26U;
        char   cacertFilePath[cacertFilePathCapacity];

        int ret = snprintf(cacertFilePath, cacertFilePathCapacity, "%s/.ndk-pkg/core/cacert.pem", userHomeDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        struct stat st;

        if (stat(cacertFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_default_verify_paths.html
            if (setenv("SSL_CERT_FILE", cacertFilePath, 1) != 0) {
                perror("SSL_CERT_FILE");
                return NDKPKG_ERROR;
            }
        }
    } else {
        size_t cacertFilePathCapacity = strlen(ndkpkgHomeDIR) + 17U;
        char   cacertFilePath[cacertFilePathCapacity];

        int ret = snprintf(cacertFilePath, cacertFilePathCapacity, "%s/core/cacert.pem", ndkpkgHomeDIR);

        if (ret < 0) {
            perror(NULL);
            return NDKPKG_ERROR;
        }

        struct stat st;

        if (stat(cacertFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_default_verify_paths.html
            if (setenv("SSL_CERT_FILE", cacertFilePath, 1) != 0) {
                perror("SSL_CERT_FILE");
                return NDKPKG_ERROR;
            }
        }
    }

    return NDKPKG_OK;
}

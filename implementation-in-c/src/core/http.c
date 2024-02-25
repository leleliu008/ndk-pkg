#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>

#include <curl/curl.h>
#include <curl/curlver.h>

#include "http.h"

int http_fetch_to_stream(const char * url, FILE * outputFile, const bool verbose, const bool showProgress) {
    if (url == NULL || url[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    if (outputFile == NULL) {
        size_t i = 0U;
        size_t j = 0U;

        for (;;) {
            char c = url[i];

            if (c == '\0') {
                break;
            }

            if (c == '/') {
                j = i;
            }

            i++;
        }

        if (j == 0U) {
            errno = EINVAL;
            return -1;
        }

        size_t size = i - j + 1U;
        char   filename[size];

        strncpy(filename, url + j + 1U, size);

        outputFile = fopen(filename, "wb");

        if (outputFile == NULL) {
            return -1;
        }
    }

    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    // https://curl.se/libcurl/c/CURLOPT_URL.html
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // https://curl.se/libcurl/c/CURLOPT_WRITEDATA.html
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outputFile);

    // https://curl.se/libcurl/c/CURLOPT_FOLLOWLOCATION.html
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // https://curl.se/libcurl/c/CURLOPT_FAILONERROR.html
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    // https://curl.se/libcurl/c/CURLOPT_TIMEOUT.html
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    // https://curl.se/libcurl/c/CURLOPT_VERBOSE.html
    curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose ? 1 : 0);

    // https://curl.se/libcurl/c/CURLOPT_NOPROGRESS.html
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, showProgress ? 0: 1L);

    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_default_verify_paths.html
    const char * const SSL_CERT_FILE = getenv("SSL_CERT_FILE");

    if ((SSL_CERT_FILE != NULL) && (SSL_CERT_FILE[0] != '\0')) {
        // https://curl.se/libcurl/c/CURLOPT_CAINFO.html
        curl_easy_setopt(curl, CURLOPT_CAINFO, SSL_CERT_FILE);
    }

    const char * const SSL_CERT_DIR = getenv("SSL_CERT_DIR");

    if ((SSL_CERT_DIR != NULL) && (SSL_CERT_DIR[0] != '\0')) {
        // https://curl.se/libcurl/c/CURLOPT_CAPATH.html
        curl_easy_setopt(curl, CURLOPT_CAPATH, SSL_CERT_DIR);
    }

    char userAgent[50];

    int ret = snprintf(userAgent, 50, "User-Agent: curl-%s", LIBCURL_VERSION);

    if (ret < 0) {
        return -1;
    }

    struct curl_slist *list = NULL;

    //list = curl_slist_append(list, "Accept: *");
    list = curl_slist_append(list, userAgent);

    // https://curl.se/libcurl/c/CURLOPT_HTTPHEADER.html
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    CURLcode curlcode = curl_easy_perform(curl);
    
    // https://curl.se/libcurl/c/libcurl-errors.html
    if (curlcode != CURLE_OK) {
        fprintf(stderr, "%s\n", curl_easy_strerror(curlcode));
    }

    curl_slist_free_all(list);

    curl_easy_cleanup(curl);

    curl_global_cleanup();

    return curlcode;
}

int http_fetch_to_file(const char * url, const char * outputFilePath, const bool verbose, const bool showProgress) {
    FILE * file = fopen(outputFilePath, "wb");

    if (file == NULL) {
        return -1;
    }

    int ret = http_fetch_to_stream(url, file, verbose, showProgress);

    fclose(file);

    return ret;
}

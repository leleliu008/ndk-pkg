#ifndef _CORE_HTTP_H
#define _CORE_HTTP_H

#include <stdio.h>
#include <stdbool.h>

int http_fetch_to_stream(const char * url, FILE       * outputStream,   const bool verbose, const bool showProgress);
int http_fetch_to_file  (const char * url, const char * outputFilePath, const bool verbose, const bool showProgress);

#endif

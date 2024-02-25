#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <regex.h>

int regex_matched(const char * content, const char * pattern) {
    if (content == NULL || pattern == NULL) {
        errno = EINVAL;
        return -1;
    }

    regex_t regex;

    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        int err = errno;
        regfree(&regex);
        errno = err;
        return -1;
    }

    regmatch_t regmatch[2];

    if (regexec(&regex, content, 2, regmatch, 0) != 0) {
        regfree(&regex);
        errno = 0;
        return -1;
    }

    //printf("regmatch[0].rm_so=%d\n", regmatch[0].rm_so);
    //printf("regmatch[0].rm_eo=%d\n", regmatch[0].rm_eo);

    if (regmatch[0].rm_so >= 0 && regmatch[0].rm_eo > regmatch[0].rm_so) {
        regfree(&regex);
        return 0;
    } else {
        regfree(&regex);
        errno = 0;
        return -1;
    }
}

char* regex_extract(const char * content, const char * pattern) {
    if (content == NULL || pattern == NULL) {
        errno = EINVAL;
        return NULL;
    }

    regex_t regex;

    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        int err = errno;
        regfree(&regex);
        errno = err;
        return NULL;
    }

    regmatch_t regmatch[2];

    if (regexec(&regex, content, 2, regmatch, 0) != 0) {
        regfree(&regex);
        errno = 0;
        return NULL;
    }

    //printf("regmatch[0].rm_so=%d\n", regmatch[0].rm_so);
    //printf("regmatch[0].rm_eo=%d\n", regmatch[0].rm_eo);

    if (regmatch[0].rm_so >= 0 && regmatch[0].rm_eo > regmatch[0].rm_so) {
        int n = regmatch[0].rm_eo - regmatch[0].rm_so;
        const char * str = &content[regmatch[0].rm_so];

        char * p = (char*)calloc(n + 1, sizeof(char));

        if (p == NULL) {
            regfree(&regex);
            errno = ENOMEM;
            return NULL;
        } else {
            strncpy(p, str, n);
            regfree(&regex);
            return p;
        }
    } else {
        regfree(&regex);
        errno = 0;
        return NULL;
    }
}

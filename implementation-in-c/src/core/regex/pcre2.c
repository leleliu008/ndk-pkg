#define PCRE2_CODE_UNIT_WIDTH 8

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pcre2.h>
#include <regex.h>

int regex_matched(const char * content, const char * pattern) {
    if (content == NULL || pattern == NULL) {
        errno = EINVAL;
        return -1;
    }

    int errorcode = 0;
    PCRE2_SIZE erroroffset = 0;
    pcre2_code * re = pcre2_compile((PCRE2_SPTR)pattern, strlen(pattern), 0, &errorcode, &erroroffset, NULL);

    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
        fprintf(stderr, "PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset, buffer);
        errno = EINVAL;
        return -1;
    }

    pcre2_match_data * match_data = pcre2_match_data_create_from_pattern(re, NULL);

    int rc = pcre2_match(
        re,                      /* the compiled pattern */
        (PCRE2_SPTR) content,    /* the subject string */
        strlen(content),         /* the length of the subject */
        0,                       /* start at offset 0 in the subject */
        0,                       /* default options */
        match_data,              /* block for storing the result */
        NULL);                   /* use default match context */

    //fprintf(stderr, "rc = %d\n", rc);

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    if (rc < 0) {
        errno = 0;
        return -1;
    } else {
        return 0;
    }
}

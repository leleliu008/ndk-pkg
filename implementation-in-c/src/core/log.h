#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>

#define COLOR_RED    "\033[0;31m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE   "\033[0;94m"
#define COLOR_PURPLE "\033[0;35m"
#define COLOR_OFF    "\033[0m"

#define LOG_INFO(x)    fprintf(stderr, "%s🔔  %s%s\n", COLOR_YELLOW, x, COLOR_OFF)
#define LOG_WARN(x)    fprintf(stderr, "%s🔥  %s%s\n", COLOR_YELLOW, x, COLOR_OFF)

#define LOG_SUCCESS1(a)     fprintf(stderr, "%s[✔] %s%s\n",     COLOR_GREEN, a,       COLOR_OFF)
#define LOG_SUCCESS2(a,b)   fprintf(stderr, "%s[✔] %s%s%s\n",   COLOR_GREEN, a, b,    COLOR_OFF)
#define LOG_SUCCESS3(a,b,c) fprintf(stderr, "%s[✔] %s%s%s%s\n", COLOR_GREEN, a, b, c, COLOR_OFF)

#define LOG_ERROR1(a)           fprintf(stderr, "%s💔  %s%s\n",         COLOR_RED, a,         COLOR_OFF)
#define LOG_ERROR2(a,b)         fprintf(stderr, "%s💔  %s%s%s\n",       COLOR_RED, a,b,       COLOR_OFF)
#define LOG_ERROR3(a,b,c)       fprintf(stderr, "%s💔  %s%s%s%s\n",     COLOR_RED, a,b,c,     COLOR_OFF)
#define LOG_ERROR4(a,b,c,d)     fprintf(stderr, "%s💔  %s%s%s%s%s\n",   COLOR_RED, a,b,c,d,   COLOR_OFF)
#define LOG_ERROR5(a,b,c,d,e)   fprintf(stderr, "%s💔  %s%s%s%s%s%s\n", COLOR_RED, a,b,c,d,e, COLOR_OFF)

#define LOG_WHITE(x) fprintf(stderr, "%s\n", x)
#define LOG_GREEN(x) fprintf(stderr, "%s%s%s\n", COLOR_GREEN,  x, COLOR_OFF)
#define LOG_RED(x)   fprintf(stderr, "%s%s%s\n", COLOR_RED,    x, COLOR_OFF)

#endif

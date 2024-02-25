// http://blog.fpliu.com/it/data/text/coding/URL

#include <errno.h>
#include <string.h>

#include "url.h"

int url_encode(char outputBuf[], size_t * writtenSizeInBytes, unsigned char inputBuf[], size_t inputBufSizeInBytes, const bool isToUpper) {
    const char * const table = isToUpper ? "0123456789ABCDEF" : "0123456789abcdef";

    size_t outputLength = 0U;

    for (size_t i = 0; i < inputBufSizeInBytes; i++) {
        unsigned char byte = inputBuf[i];

        //这些字符保持原样
        if (('0' <= byte && byte <= '9') ||
            ('a' <= byte && byte <= 'z') ||
            ('A' <= byte && byte <= 'Z') || 
            byte == '.' || byte == '_' || byte == '-' || byte == '*') {
            outputBuf[outputLength++] = byte;
        } else if (' ' == byte) { //把空格编码成+
            outputBuf[outputLength++] = '+';
        } else { //其他字符都转换成%XY，XY是Base16编码
            //向右移动4bit，获得高4bit
            unsigned char highByte = byte >> 4;

            //与0x0f做位与运算，获得低4bit
            unsigned char lowByte = byte & 0x0F;

            //由于高4bit和低4bit都只有4个bit，他们转换成10进制的数字，范围都在0 ～ 15闭区间内
            //大端模式
            outputBuf[outputLength++] = '%';
            outputBuf[outputLength++] = table[highByte];
            outputBuf[outputLength++] = table[lowByte];
        }
    }

    (*writtenSizeInBytes) = outputLength;

    return 0;
}

static inline char hex2dec(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return -1;
    }
}

int url_decode(unsigned char outputBuf[], size_t * writtenSizeInBytes, char inputBuf[], size_t inputBufSizeInBytes) {
    if (inputBufSizeInBytes == 0U) {
        inputBufSizeInBytes = strlen(inputBuf);
    }

    size_t outputLength = 0U;

    for (size_t i = 0U; i < inputBufSizeInBytes; i++) {
        char c = inputBuf[i];

        if (c == '%') {
            char c1 = hex2dec(inputBuf[++i]);

            if (c1 < 0) {
                errno = EINVAL;
                return -1;
            }

            char c0 = hex2dec(inputBuf[++i]);

            if (c0 < 0) {
                errno = EINVAL;
                return -1;
            }

            //16进制转10进制
            outputBuf[outputLength++] = c1 + c0;
        } else if (c == '+') {
            outputBuf[outputLength++] = ' ';
        } else {
            outputBuf[outputLength++] = c;
        }
    }

    (*writtenSizeInBytes) = outputLength;

    return 0;
}

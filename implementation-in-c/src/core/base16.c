//http://blog.fpliu.com/it/data/text/coding/Base16

#include <errno.h>
#include <string.h>

#include "base16.h"

int base16_encode(char * outputBuf, const unsigned char * inputBuf, size_t inputBufSizeInBytes, const bool isToUpper) {
    if (outputBuf == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (inputBuf == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (inputBufSizeInBytes == 0U) {
        errno = EINVAL;
        return -1;
    }

    const char * const table = isToUpper ? "0123456789ABCDEF" : "0123456789abcdef";

    for (size_t i = 0; i < inputBufSizeInBytes; i++) {
        //向右移动4bit，获得高4bit
        unsigned char highByte = inputBuf[i] >> 4;
        //与0x0f做位与运算，获得低4bit
        unsigned char lowByte = inputBuf[i] & 0x0F;

        //由于高4bit和低4bit都只有4个bit，他们转换成10进制的数字，范围都在0 ～ 15闭区间内
        //大端模式
        size_t j = i << 1;
        outputBuf[j] = table[highByte];
        outputBuf[j + 1U] = table[lowByte];
    }

    return 0;
}

//把16进制字符转换成10进制表示的数字
//通过man ascii命令查看ASCII编码表即可得到如下转换逻辑
static short hex2dec(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return 0;
    }
}

int base16_decode(unsigned char * outputBuf, const char * inputBuf, size_t inputBufSizeInBytes) {
    if (outputBuf == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (inputBuf == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (inputBufSizeInBytes == 0U) {
        inputBufSizeInBytes = strlen(inputBuf);
    }

    //inputBuf指向的字符串长度必须是2的整数倍，也就是必须是偶数
    if (1U == (inputBufSizeInBytes & 1)) {
        errno = EINVAL;
        return -1;
    }

    size_t halfInputSize = inputBufSizeInBytes >> 1;

    for (size_t i = 0; i < halfInputSize; i++) {
        //16进制数字转换为10进制数字的过程
        size_t j = i << 1;

        char c1 = hex2dec(inputBuf[j]) << 4;

        if (c1 < 0) {
            errno = EINVAL;
            return -1;
        }

        char c0 = hex2dec(inputBuf[j + 1U]);

        if (c0 < 0) {
            errno = EINVAL;
            return -1;
        }

        outputBuf[i] = c1 + c0;
    }

    return 0;
}

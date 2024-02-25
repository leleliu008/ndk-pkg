#ifndef BASE_16_H
#define BASE_16_H
    
#include <stdlib.h>
#include <stdbool.h>

/* base16编码
 * outputBuf               base16编码后的字符串, 你需要自己确保这个buffer不够大到能容纳所有编码后的内容，你需要自己添加\0
 * inputBuf                要编码的字节的起始指针
 * inputBufLengthInBytes   要编码的字节的长度
 * isToUpper            是否编码为大写字母
 */
int base16_encode(char * outputBuf, const unsigned char * inputBuf, size_t inputBufSizeInBytes, const bool isToUpper);

/* base16解码
 * inputBuf base16编码的字符串
 */
int base16_decode(unsigned char * outputBuf, const char * inputBuf, size_t inputBufSizeInBytes);

#endif

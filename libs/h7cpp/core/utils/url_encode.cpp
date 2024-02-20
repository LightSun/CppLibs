#include "url_encode.h"
#include <stdio.h>
#include <string.h>

static unsigned char enc_tab[] = "0123456789ABCDEF";

char *url_encode(const char *str){
    int len = (int) strlen(str);
    int tmp_len = len;

    unsigned char *tmp = (unsigned char*) malloc(len + 1);

    int i, j;
    for (i = 0, j = 0; i < len; i++, j++) {
        tmp[j] = (unsigned char) str[i];
        if (!isalnum(tmp[j]) && strchr("_-.", tmp[j]) == NULL) { // 所传的字符不是字母和数字时，也不是_-.
            tmp_len += 3;
            tmp = (unsigned char*)realloc(tmp, tmp_len);
            tmp[j++] = '%';
            tmp[j++] = enc_tab[(unsigned char)str[i] >> 4];  //
            tmp[j] = enc_tab[(unsigned char)str[i] & 0x0F];
        }
    }

    tmp[j] = '\0';
    return (char*) tmp;
}

static unsigned char dec_tab[256] = {
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,
            0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };

    /**
     * URL 解码函数
     * @param str {const char*} 经URL编码后的字符串
     * @return {char*} 解码后的字符串，返回值不可能为空，需要用 free 释放
     */
char *url_decode(const char *str){
    int len = (int) strlen(str);
    char *tmp = (char *)malloc(len + 1);

    int i = 0, pos = 0;
    for (i = 0; i < len; i++) {
        if (str[i] != '%')
            tmp[pos] = str[i];
        else if (i + 2 >= len) {  /* check boundary */
            tmp[pos++] = '%';  /* keep it */
            if (++i >= len)
                break;
            tmp[pos] = str[i];
            break;
        } else if (isalnum(str[i + 1]) && isalnum(str[i + 2])) {
            tmp[pos] = (dec_tab[(unsigned char) str[i + 1]] << 4)
                       + dec_tab[(unsigned char) str[i + 2]];
            i += 2;
        } else
            tmp[pos] = str[i];

        pos++;
    }

    tmp[pos] = '\0';
    return tmp;
}



#ifndef URL_ENCODE_H
#define URL_ENCODE_H

#include "common/c_common.h"
#include <string>

CPP_START

char* url_encode(const char * src);

char* url_decode(const char* encd);

CPP_END

static inline std::string url_encode2(const std::string& src){
    char* _data = url_encode(src.data());
    std::string _file = _data;
    free(_data);
    return _file;
}

static inline std::string url_decode2(const std::string& src){
    char* _data = url_decode(src.data());
    std::string _file = _data;
    free(_data);
    return _file;
}



#endif // URL_ENCODE_H

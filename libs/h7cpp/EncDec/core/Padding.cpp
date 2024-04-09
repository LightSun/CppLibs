#include "Padding.h"

using namespace med;

/**
 * @brief Padding::getPKCS7PaddedLength
 * 根据原始数据长度，计算进行PKCS7填充后的数据长度
 * @param dataLen 原始数据长度
 * @param alignSize 对齐字节数
 * @return 返回填充后的数据长度
 */
int Padding::getPKCS7PaddedLength(int dataLen, int alignSize){
    // 计算填充的字节数（按alignSize字节对齐进行填充）
    int remainder = dataLen % alignSize;
    int paddingSize = (remainder == 0) ? alignSize : (alignSize - remainder);
    return (dataLen + paddingSize);
}
String Padding::doPKCS7Padding(CString in, int alignSize){
    // 计算需要填充字节数（按alignSize字节对齐进行填充）
    int remainder = in.size() % alignSize;
    int paddingSize = (remainder == 0) ? alignSize : (alignSize - remainder);
    String pad;
    pad.resize(paddingSize);
    //count, char
    for(int i = 0 ; i < paddingSize ; ++i){
        ((char*)pad.data())[i] = paddingSize;
    }
    // 进行填充
    String temp(in);
    temp.append(pad);
    return temp;
}
String Padding::doPKCS7UnPadding(CString in){
    char paddingSize = in.at(in.size() - 1);
    return in.substr(0, in.size() - paddingSize);
}

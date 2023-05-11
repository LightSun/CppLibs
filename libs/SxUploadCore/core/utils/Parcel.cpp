#include "Parcel.h"
#include <memory.h>
//#include "endian.h"
//#include "utils/common_utils.h"

using namespace h7;

void Parcel::putSimpleData(CString name, char* _data, unsigned int size){
    unsigned int delta = sizeof(short) + name.length() + 1 + size;
    unsigned int pos = m_data.size();
    prepareDataSpace(delta);
    char* tdata = m_data.data() + pos;
    unsigned int offset;
    //name
    *(short*)tdata = name.length() + 1;
    offset = sizeof(short);
    memcpy(tdata + offset, name.data(), name.length() + 1);
    offset += name.length() + 1;
    //data
    memcpy(tdata + offset, _data, size);
}

void Parcel::putBlockData(CString name, char* _data, unsigned int size){
    unsigned int delta = sizeof(short) + name.length() + 1 + sizeof(unsigned int) + size;
    unsigned int pos = m_data.size();
    prepareDataSpace(delta);
    char* tdata = m_data.data() + pos;
    unsigned int offset;
    //name
    *(short*)tdata = name.length() + 1;
    offset = sizeof(short);
    memcpy(tdata + offset, name.data(), name.length() + 1);
    offset += name.length() + 1;
    //data
    *(unsigned int*)(tdata + offset) = size;
    if(size > 0){
        offset += sizeof(unsigned int);
        memcpy(tdata + offset, _data, size);
    }
}
char* Parcel::serialize(unsigned int* outSize){
    //version + magic
    uint32 realSize = sizeof(int) * 2 + m_data.size();
    if(outSize){
        *outSize = realSize;
    }
    char* data = (char*)malloc(realSize);
    *(int*)(data) = BUNDLE_VERSION;
    int offset = sizeof(int);
    //magic(latter use)
    *((unsigned int*)(data + offset)) = 0;
    offset += sizeof(int);
    //data
    memcpy(data + offset, m_data.data(), m_data.size());
    return data;
}
void Parcel::deSerialize(char* data,unsigned int size){
    if(size == 0){
        m_data.clear();
        return;
    }
    //version + magic latter may use.
    m_data.resize(size - sizeof(int) * 2);
    memcpy(m_data.data(), data + sizeof(int) * 2, size);
}

void Parcel::putListString(CString name, ListS* list){
    //len_name + name + size_list_total + list.size
    unsigned int delta = sizeof(short) + name.length() + 1
            + sizeof (unsigned int) + sizeof(int);
    if(list){
        for(int i = 0 ; i < list->size() ; i ++){
            delta += sizeof (unsigned int) + list->get(i).length() + 1;
        }
    }
    unsigned int pos = m_data.size();
    prepareDataSpace(delta);
    char* tdata = m_data.data() + pos;
    unsigned int offset = 0;
    //name
    *(short*)(tdata + offset) = name.length() + 1;
    offset = sizeof(short);
    memcpy(tdata + offset, name.data(), name.length() + 1);
    offset += name.length() + 1;
    //total
    *(unsigned int*)(tdata + offset) = delta - (sizeof (unsigned int)
                                    + sizeof(short) + name.length() + 1);
    offset += sizeof (unsigned int);
    //list (size with elements)
    if(list){
       *(unsigned int*)(tdata + offset) = list->size();
       offset += sizeof (unsigned int);
       for(int i = 0 ; i < list->size() ; i ++){
           auto& str = list->get(i);
           *(unsigned int*)(tdata + offset) = str.length() + 1;
           offset += sizeof(unsigned int);
           memcpy(tdata + offset, str.data(), str.length() + 1);
           offset += str.length() + 1;
       }
    }else{
       *(int*)(tdata + offset) = 0;
    }
}

char* Parcel::nextBlockData(unsigned int* sizePtr){
    unsigned int len = *(unsigned int*)(m_data.data() + m_pos);
    if(sizePtr){
        *sizePtr = len;
    }
    char* data = m_data.data() + m_pos + sizeof (unsigned int);
    m_pos += len + sizeof (unsigned int);
    return data;
}

void Parcel::prepareDataSpace(unsigned int delta){
    unsigned int cap = m_data.capacity();
    unsigned int expectSize = m_data.size() + delta;
    if(cap < expectSize){
        cap = HMAX(cap * 2, expectSize);
        m_data.reserve(cap);
    }
    if(m_data.size() < expectSize){
        m_data.resize(expectSize);
    }
}
//-----------------------------------------------
#define __NEXT_LIST(type)\
unsigned int size;\
char * data = nextBlockData(&size);\
if(size > 0){ \
    out->resize(size / sizeof (type));\
    memcpy(out->data(), data, size);\
}

void Parcel::nextListByte(std::vector<char>* out){
    __NEXT_LIST(char);
}
void Parcel::nextListShort(std::vector<short>* out){
    __NEXT_LIST(short)
}
void Parcel::nextListInt(std::vector<int>* out){
    __NEXT_LIST(int)
}
void Parcel::nextListLong(std::vector<long long>* out){
    __NEXT_LIST(long long)
}
void Parcel::nextListFloat(std::vector<float>* out){
    __NEXT_LIST(float)
}
void Parcel::nextListDouble(std::vector<double>* out){
    __NEXT_LIST(double)
}
void Parcel::nextListString(std::vector<String>* out){
    unsigned int size;
    char * data = nextBlockData(&size);
    unsigned int list_len = *(unsigned int*)data;
    out->reserve(list_len);
    unsigned int offset = sizeof (unsigned int);
    for(unsigned int i = 0 ; i < list_len ; i ++){
        unsigned int strLen = *(unsigned int*)(data + offset);
        offset += sizeof (unsigned int);
        out->emplace_back(data + offset, strLen - 1);
        offset += strLen;
    }
}

//--------------------------------



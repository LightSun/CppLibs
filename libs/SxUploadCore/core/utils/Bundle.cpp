#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "Bundle.h"
#include "utils/hash.h"
#include "utils/binary_search.h"
#include "utils/collection_utils.h"
//#include "utils/common_utils.h"

#define HASH_SEED 31

using namespace h7;

void Bundle::putSimpleData(CString name, int type, char* _data){
    int dLen = 0;
    switch (type) {
    case TYPE_BYTE:{dLen = 1;}break;
    case TYPE_SHORT:{dLen = 2;}break;
    case TYPE_INT:{dLen = 4;}break;
    case TYPE_LONG:{dLen = 8;}break;
    case TYPE_FLOAT:{dLen = 4;}break;
    case TYPE_DOUBLE:{dLen = 8;}break;
    case TYPE_BOOLEAN:{dLen = 1;}break;
    default:
        std::cout << "Bundle >> wrong type = " << type << std::endl;
        return;
    }
    int nextPos = m_data.size();
    prepareDataSpace(dLen);
    char* data = m_data.list.data();
    memcpy(data + nextPos, _data, dLen);
    //
    m_names.add(name);
    addDesc(dLen, (unsigned char)type);
}

void Bundle::putString(CString name, CString str){
    int nextPos = m_data.size();
    prepareDataSpace(str.length() + 1);
    char* data = m_data.list.data();
    memcpy(data + nextPos, str.c_str(), str.length() + 1);
    //
    m_names.add(name);
    addDesc(str.length() + 1, TYPE_STRING);
}

void Bundle::putBlob(CString name, char* _data, int size,int act_type){
    int nextPos = m_data.size();
    prepareDataSpace(size);
    char* data = m_data.list.data();
    memcpy(data + nextPos, _data, size);
    //
    m_names.add(name);
    //(coll_type << 1) + TYPE_BLOB
    addDesc(size, act_type);
}
void Bundle::putListString(CString name, ListS* list){
    int totalSize = sizeof (int);
    for(int i = 0 ; i < list->size() ; i ++){
        totalSize += list->get(i).length() + 1 + 4;
    }
    //
    int nextPos = m_data.size();
    prepareDataSpace(totalSize);
    char* data = m_data.list.data() + nextPos;
    //size
    *(int*)data = list->size();
    // copy data
    int offset = sizeof (int);
    int len;
    for(int i = 0 ; i < list->size() ; i ++){
        len = list->get(i).length();
        *(int*)(data + offset) = len;
        memcpy(data + offset + 4, list->get(i).data(), len + 1);
        offset += len + 4;
    }
    //
    m_names.add(name);
    addDesc(totalSize, TYPE_LIST_STRING);
}

void Bundle::putBundle(CString name, Bundle* b){
    int bsize;
    char* data = b->serialize(&bsize);
    //
    putBlob(name, data, bsize, TYPE_BUNDLE);
    free(data);
}
//-------------------------------------------
void Bundle::prepareDataSpace(int delta){
    int cap = m_data.list.capacity();
    int expectSize = m_data.size() + delta;
    if(cap < expectSize){
        cap = HMAX(cap * 2, expectSize);
        m_data.prepareSize(cap);
    }
    if(m_data.size() < expectSize){
        m_data.resize(expectSize);
    }
}

short Bundle::getShort(PosInfo& info){
    Uni_Short ui;
    memcpy(ui.arr, m_data.list.data() + info.dataPos, sizeof (short));
    return ui.val;
}
int Bundle::getInt(PosInfo& info){
    Uni_Int ui;
    memcpy(ui.arr, m_data.list.data()+ info.dataPos, sizeof (int));
    return ui.val;
}
long long Bundle::getLong(PosInfo& info){
    Uni_Long ui;
    memcpy(ui.arr, m_data.list.data()+ info.dataPos, sizeof (long long));
    return ui.val;
}
float Bundle::getFloat(PosInfo& info){
    Uni_Float ui;
    memcpy(ui.arr, m_data.list.data()+ info.dataPos, sizeof (float));
    return ui.val;
}
double Bundle::getDouble(PosInfo& info){
    Uni_Double ui;
    memcpy(ui.arr, m_data.list.data() + info.dataPos, sizeof (double));
    return ui.val;
}
String Bundle::getString(PosInfo& info){
    char* data = (char*)(m_data.list.data() + info.dataPos);
    return std::string(data, getSize(info.itemIndex) - 1);
}
char* Bundle::getBlob(PosInfo& info, int* sizePtr){
    if(info.itemIndex < 0){
        return nullptr;
    }
    char* data = (char*)(m_data.list.data() + info.dataPos);
    if(sizePtr){
        *sizePtr = getSize(info.itemIndex);
    }
    return data;
}
//--------------- list ------------
void Bundle::getBundle(PosInfo& info, Bundle* out){
    int bsize;
    char* data = getBlob(info, &bsize);
    out->deSerialize(data, bsize);
}
void Bundle::getListByte(PosInfo& info, std::vector<char>* list){
    int size = getSize(info.itemIndex);
    list->resize(size / sizeof (char));
    memcpy(list->data(), m_data.list.data() + info.dataPos, size);
}
void Bundle::getListShort(PosInfo& info, std::vector<short>* list){
    int size = getSize(info.itemIndex);
    list->resize(size / sizeof (short));
    memcpy(list->data(), m_data.list.data() + info.dataPos, size);
}
void Bundle::getListInt(PosInfo& info, std::vector<int>* list){
    int size = getSize(info.itemIndex);
    list->resize(size / sizeof (int));
    memcpy(list->data(), m_data.list.data() + info.dataPos, size);
}
void Bundle::getListLong(PosInfo& info, std::vector<long long>* list){
    int size = getSize(info.itemIndex);
    list->resize(size / sizeof (long long));
    memcpy(list->data(), m_data.list.data() + info.dataPos, size);
}
void Bundle::getListFloat(PosInfo& info, std::vector<float>* list){
    int size = getSize(info.itemIndex);
    list->resize(size / sizeof (float));
    memcpy(list->data(), m_data.list.data() + info.dataPos, size);
}
void Bundle::getListDouble(PosInfo& info, std::vector<double>* list){
    int size = getSize(info.itemIndex);
    list->resize(size / sizeof (double));
    memcpy(list->data(), m_data.list.data() + info.dataPos, size);
}
void Bundle::getListBoolean(PosInfo& info, std::vector<bool>* list){
    int c = getSize(info.itemIndex) / sizeof (char);
    list->reserve(c);
    for(int i = 0 ; i < c ; i ++){
        list->push_back(m_data.list.data()[info.dataPos + i * sizeof (char)] != 0);
    }
}
void Bundle::getListString(PosInfo& info, std::vector<String>* list){
    int c = *(int*)(m_data.list.data() + info.dataPos);
    char* data = m_data.list.data() + info.dataPos + sizeof (int);
    list->reserve(c);
    //
    int offset = 0;
    int len;
    for(int i = 0 ; i < c ; i ++){
        len = *(int*)(data + offset);
        list->emplace_back(data + offset + sizeof(int), len);
        offset += len + sizeof (int);
    }
}
//-------------------------------------
char* Bundle::serialize(int* outSize){
    const int shortSize = sizeof (short);
    int c = m_names.size();
    //version + magic + item count
    int totalSize = sizeof (int) * 3 ;
    for(int i = 0 ; i < c ; i ++){
        totalSize += shortSize + m_names[i].length() + 1;//include '\0'
    }
    //desc
    totalSize += m_descInfo.size();
    //data
    totalSize += m_data.size() + sizeof (int);
    if(outSize){
        *outSize = totalSize;
    }
    //--- copy data ----
    char* outData = (char*)malloc(totalSize);
    //version
    *((int*)outData) = BUNDLE_VERSION;
    unsigned int offset = sizeof (int);
    //magic(latter use)
    *((unsigned int*)(outData + offset)) = 0;

    offset += sizeof (int);
    //c
    *((int*)(outData + offset)) = c;
    offset += sizeof (int);
    //names
    int len;
    for(int i = 0 ; i < c ; i ++){
        len = m_names[i].length();
        *(short*)(outData + offset) = (short)len;
        memcpy(outData + offset + shortSize, m_names[i].c_str(), len + 1);
        offset += shortSize + len + 1;
    }
    //desc
    unsigned char* descPtr = (unsigned char*)(outData + offset);
    memcpy(descPtr, m_descInfo.list.data(), m_descInfo.size());
    offset += m_descInfo.size();

    //data
    *(int*)(outData + offset) = m_data.size();
    offset += sizeof (int);
    memcpy(outData + offset, m_data.list.data(), m_data.size());
    offset += m_data.size();
    return outData;
}
void Bundle::deSerialize(char* data, int){
    const int shortSize = sizeof (short);
    m_names.clear();
    //version + magic
    int offset = sizeof (int) + sizeof (int);
    //int version = *((int*)data);
    // item count
    int c = *((int*)(data + offset ));
    m_descInfo.resize(c * DESC_UNIT_SIZE);
    m_names.prepareSize(c);
    //names
    offset += sizeof (int);
    int len;
    for(int i = 0 ; i < c ; i ++){
        len = *(short*)(data + offset);
        m_names.add_cons((char*)(data + offset + shortSize), len);
        offset += shortSize + len + 1;
    }
    //desc
    unsigned char* descPtr = (unsigned char*)(data + offset);
    memcpy(m_descInfo.list.data(), descPtr, m_descInfo.size());
    offset += m_descInfo.size();
    //data
    int dataSize = *(int*)(data + offset);
    m_data.resize(dataSize);
    offset += 4;
    memcpy(m_data.list.data(), data + offset, dataSize);
}
//------------------------------------------------
int Bundle::getItemIndex(CString name){
    if(m_hashes && m_hashes->size() > 0){
        unsigned int hash = fasthash32(name.data(), name.length(), HASH_SEED);
        int index = binarySearch_u(m_hashes->list.data(), 0, m_hashes->size(), hash);
        if(index >= 0){
            return m_indexes->get(index);
        }
        return index;
    }else{
        return m_names.indexOf(name);
    }
}
void Bundle::makeIndex(){
    if(!m_hashes){
        return;
    }
    m_hashes->resize(m_names.size());
    m_indexes->resize(m_names.size());
    for(int i = 0 ; i < m_names.size() ; ++i){
        m_hashes->set0(i, fasthash32(m_names[i].c_str(), m_names[i].length(), HASH_SEED));
    }
    utils::sortReturnIndex2<unsigned int>(m_hashes->list, m_indexes->list, true);
}
void Bundle::travel(std::function<void(Bundle*,std::string&, PosInfo*)> func){
    PosInfo info;
    int offset = 0;
    for(int i = 0 ; i < m_names.size() ; i ++){
        info.dataPos = offset;
        info.itemIndex = i;
        func(this, m_names[i], &info);
        offset += getSize(i);
    }
}
void Bundle::travelDesc(std::function<void(Bundle*,ItemInfo*)> func){
    ItemInfo info;
    int offset = 0;
    for(int i = 0 ; i < m_names.size() ; i ++){
        info.pos.dataPos = offset;
        info.pos.itemIndex = i;
        info.name = m_names[i];
        info.type = getType(i);
        info.size = getSize(i);
        func(this, &info);
        offset += info.size;
    }
}

bool Bundle::operator==(const h7::Bundle& b2){
    if(m_names != b2.m_names){
        return false;
    }
    if(strcmp((const char*)m_descInfo.list.data(), (const char*)b2.m_descInfo.list.data()) != 0){
        return false;
    }
    if(strcmp((const char*)m_data.list.data(), (const char*)b2.m_data.list.data()) != 0){
        return false;
    }

    return true;
}

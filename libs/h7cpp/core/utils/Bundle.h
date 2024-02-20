#ifndef BUNDLE_H
#define BUNDLE_H

#include "common/c_common.h"
#include "table/Column.h"

#define BUNDLE_VERSION 1

namespace h7 {

class Bundle: public SkRefCnt
{
public:
    enum{
        TYPE_BYTE  = 1,
        TYPE_SHORT ,
        TYPE_INT   ,
        TYPE_LONG  ,
        TYPE_FLOAT ,
        TYPE_DOUBLE ,
        TYPE_BOOLEAN ,
        TYPE_STRING,
        TYPE_BLOB,
    };
    enum{
        CTYPE_LIST_BYTE  = 1,
        CTYPE_LIST_SHORT ,
        CTYPE_LIST_INT   ,
        CTYPE_LIST_LONG  ,
        CTYPE_LIST_FLOAT ,
        CTYPE_LIST_DOUBLE ,
        CTYPE_LIST_BOOLEAN ,
        CTYPE_LIST_STRING,
        CTYPE_BUNDLE,
    };
#define __BUNDLE_COMBINE_TYPE(t1,t2) (t1 * 10 + t2)
#define __BUNDLE_TYPE_LIST(name) TYPE_LIST_##name = __BUNDLE_COMBINE_TYPE(CTYPE_LIST_##name,TYPE_BLOB)
    enum{
        __BUNDLE_TYPE_LIST(BYTE),
        __BUNDLE_TYPE_LIST(SHORT),
        __BUNDLE_TYPE_LIST(INT) ,
        __BUNDLE_TYPE_LIST(LONG) ,
        __BUNDLE_TYPE_LIST(FLOAT) ,
        __BUNDLE_TYPE_LIST(DOUBLE) ,
        __BUNDLE_TYPE_LIST(BOOLEAN) ,
        __BUNDLE_TYPE_LIST(STRING),
        TYPE_BUNDLE = __BUNDLE_COMBINE_TYPE(CTYPE_BUNDLE, TYPE_BLOB)
    };
    typedef struct{
        unsigned int dataPos {0};
        unsigned int itemIndex {0};
    }PosInfo;
    typedef struct {
        PosInfo pos;
        int type;
        int size;
        std::string name;
    }ItemInfo;

    Bundle():Bundle(0, 0){}
    Bundle(int itemCount, int dataSize){
        if(dataSize > 0){
            m_data.prepareSize(dataSize);
        }
        if(itemCount > 0){
            m_descInfo.prepareSize(itemCount);
            m_names.prepareSize(itemCount);
        }
    };
    Bundle(const Bundle& src){
        this->m_names = src.m_names;
        this->m_descInfo = src.m_descInfo;
        this->m_data = src.m_data;
    }
    void putString(CString name, CString data);

    inline void putBlob(CString name, char* data, int size){
        putBlob(name, data, size, TYPE_BLOB);
    }
    void putBlob(CString name, char* data, int size, int act_type);

    inline void putByte(CString name, char val){
        char arr[1];
        arr[0] = val;
        putSimpleData(name, TYPE_BYTE, arr);
    }
    inline void putShort(CString name, short val){
        Uni_Short ui;
        ui.val = val;
        putSimpleData(name, TYPE_SHORT, ui.arr);
    }
    inline void putInt(CString name, int val){
        Uni_Int ui;
        ui.val = val;
        putSimpleData(name, TYPE_INT, ui.arr);
    }
    inline void putLong(CString name, long long val){
        Uni_Long ui;
        ui.val = val;
        putSimpleData(name, TYPE_LONG, ui.arr);
    }
    inline void putFloat(CString name, float val){
        Uni_Float ui;
        ui.val = val;
        putSimpleData(name, TYPE_FLOAT, ui.arr);
    }
    inline void putDouble(CString name, double val){
        Uni_Double ui;
        ui.val = val;
        putSimpleData(name, TYPE_DOUBLE, ui.arr);
    }
    inline void putBoolean(CString name, bool val){
        char arr[1];
        arr[0] = val;
        putSimpleData(name, TYPE_BOOLEAN, arr);
    }
    inline void putListByte(CString name, ListCh* list){
        int dSize = list->size() * sizeof (char);
        putBlob(name, (char*)list->list.data(), dSize, TYPE_LIST_BYTE);
    }
    inline void putListShort(CString name, ListSh* list){
        int dSize = list->size() * sizeof (short);
        putBlob(name, (char*)list->list.data(), dSize,
            TYPE_LIST_SHORT);
    }
    inline void putListInt(CString name, ListI* list){
        int dSize = list->size() * sizeof (int);
        putBlob(name, (char*)list->list.data(), dSize,
            TYPE_LIST_INT);
    }
    inline void putListLong(CString name, ListL* list){
        int dSize = list->size() * sizeof (long long);
        putBlob(name, (char*)list->list.data(), dSize,
            TYPE_LIST_LONG);
    }
    inline void putListFloat(CString name, ListF* list){
        int dSize = list->size() * sizeof (float);
        putBlob(name, (char*)list->list.data(), dSize,
            TYPE_LIST_FLOAT);
    }
    inline void putListDouble(CString name, ListD* list){
        int dSize = list->size() * sizeof (double);
        putBlob(name, (char*)list->list.data(), dSize,
            TYPE_LIST_DOUBLE);
    }
    inline void putListBoolean(CString name, ListB* list){
        int dSize = list->size() * sizeof (char);
        sk_sp<IColumn<char>> ret = sk_make_sp<IColumn<char>>(list->size());
        for(int i = 0 ; i < list->size() ; i ++){
            ret->add(list->const_get(i) ? 1 : 0);
        }
        putBlob(name, ret->list.data(), dSize,
                TYPE_LIST_BOOLEAN);

    }
    void putListString(CString name, ListS* list);
    void putBundle(CString name,Bundle* b);
    //-------------------------------------
    inline void putListByte(CString name, char* data, int len){
        putBlob(name, data, len * sizeof (char),
            TYPE_LIST_BYTE);
    }
    inline void putListShort(CString name, short* data, int len){
        putBlob(name, (char*)data, len * sizeof (short),
            TYPE_LIST_SHORT);
    }
    inline void putListInt(CString name, int* data, int len){
        putBlob(name, (char*)data, len * sizeof (int),
            TYPE_LIST_INT);
    }
    inline void putListLong(CString name, long* data, int len){
        putBlob(name, (char*)data, len * sizeof (long),
            TYPE_LIST_LONG);
    }
    inline void putListBoolean(CString name, char* data, int len){
        putBlob(name, data, len * sizeof (char),
            TYPE_LIST_BOOLEAN);
    }
    inline void putListFloat(CString name, float* data, int len){
        putBlob(name, (char*)data, len * sizeof (float),
            TYPE_LIST_FLOAT);
    }
    inline void putListDouble(CString name, double* data, int len){
        putBlob(name, (char*)data, len * sizeof (double),
            TYPE_LIST_DOUBLE);
    }
    //--------------------------------------
    inline bool getPosInfo(CString name, PosInfo& info){
        int index = getItemIndex(name);
       // MED_ASSERT_FMT(index >=0, "name not exist: %s", name.c_str());
        return getPosInfo(index, info);
    }
    inline bool getPosInfo(int index, PosInfo& info){
        if(index < 0 || index >= m_names.size()){
            return false;
        }
        int offset = 0;
        for(int i = 0 ; i < index ; i ++){
            offset += getSize(i);
        }
        info.dataPos = offset;
        info.itemIndex = index;
        return true;
    }
    inline bool getItemDesc(CString name, ItemInfo& info){
        int index = getItemIndex(name);
        //MED_ASSERT_FMT(index >=0, "name not exist: %s", name.c_str());
        return getItemDesc(index, info);
    }
    inline bool getItemDesc(int index, ItemInfo& info){
        if(!getPosInfo(index, info.pos)){
            return false;
        }
        info.name = m_names[index];
        info.size = getSize(index);
        info.type = getType(index);
        return true;
    }
    // < 0 for not exist
    int getItemIndex(CString name);

    inline void openIndex(){
        if(!m_hashes){
            m_hashes = sk_make_sp<ListUI>();
        }
        if(!m_indexes){
            m_indexes = sk_make_sp<ListUI>();
        }
        makeIndex();
    }
    inline void closeIndex(){
        if(m_hashes){
            m_indexes.release()->unref();
            m_hashes.release()->unref();
        }
    }
    //------------- get ---------------
#define __BUNDLE_GET_VAL(name, func)\
    PosInfo info;\
    getPosInfo(name, info);\
    if(info.itemIndex >= 0){\
        return func(info);\
    }

    inline char getByte(CString name, char defVal = -1){
        __BUNDLE_GET_VAL(name, getByte);
        return defVal;
    }
    inline short getShort(CString name, short defVal = -1){
        __BUNDLE_GET_VAL(name, getShort);
        return defVal;
    }
    inline int getInt(CString name, int defVal = -1){
        __BUNDLE_GET_VAL(name, getInt);
        return defVal;
    }
    inline long long getLong(CString name, long long defVal = -1){
        __BUNDLE_GET_VAL(name, getLong);
        return defVal;
    }
    inline float getFloat(CString name, float defVal = -1){
        __BUNDLE_GET_VAL(name, getFloat);
        return defVal;
    }
    inline double getDouble(CString name,double defVal = -1){
        __BUNDLE_GET_VAL(name, getDouble);
        return defVal;
    }
    inline bool getBoolean(CString name, bool defVal = false){
        __BUNDLE_GET_VAL(name, getBoolean);
        return defVal;
    }
    inline String getString(CString name,CString defVal = ""){
        __BUNDLE_GET_VAL(name, getString);
        return defVal;
    }
    inline char* getBlob(CString name, int* sizePtr){
        PosInfo info;
        getPosInfo(name, info);
        return getBlob(info, sizePtr);
    }
    //------------ PosInfo ---------
    inline char getByte(PosInfo& info){
        return m_data.list.data()[info.dataPos];
    }
    short getShort(PosInfo& info);
    int getInt(PosInfo& info);
    long long getLong(PosInfo& info);
    float getFloat(PosInfo& info);
    double getDouble(PosInfo& info);
    inline bool getBoolean(PosInfo& info){
        return m_data.list.data()[info.dataPos] != 0;
    }
    String getString(PosInfo& info);
    char* getBlob(PosInfo& info, int* sizePtr);

    //------- get list ----------------

    inline sk_sp<ListCh> getListByte(CString name){
        __BUNDLE_GET_VAL(name, getListByte);
        return nullptr;
    }
    inline sk_sp<ListSh> getListShort(CString name){
        __BUNDLE_GET_VAL(name, getListShort);
        return nullptr;
    }
    inline sk_sp<ListI> getListInt(CString name){
        __BUNDLE_GET_VAL(name, getListInt);
        return nullptr;
    }
    inline sk_sp<ListL> getListLong(CString name){
        __BUNDLE_GET_VAL(name, getListLong);
        return nullptr;
    }
    inline sk_sp<ListF> getListFloat(CString name){
         __BUNDLE_GET_VAL(name, getListFloat);
         return nullptr;
    }
    inline sk_sp<ListD> getListDouble(CString name){
        __BUNDLE_GET_VAL(name, getListDouble);
        return nullptr;
    }
    inline sk_sp<ListB> getListBoolean(CString name){
        __BUNDLE_GET_VAL(name, getListBoolean);
        return nullptr;
    }
    inline sk_sp<ListS> getListString(CString name){
        __BUNDLE_GET_VAL(name, getListString);
        return nullptr;
    }
    inline sk_sp<Bundle> getBundle(CString name){
         __BUNDLE_GET_VAL(name, getBundle);
         return nullptr;
    }

#define __BUNDLE_GET_LIST_VAL_BY_NAME(name, func)\
    PosInfo info;\
    getPosInfo(name, info);\
    if(info.itemIndex >= 0){\
        func(info, list);\
    }

    inline void getListByte(CString name, ListCh* list){
        __BUNDLE_GET_LIST_VAL_BY_NAME(name, getListByte)
    }
    inline void getListShort(CString name, ListSh* list){
        __BUNDLE_GET_LIST_VAL_BY_NAME(name, getListShort)
    }
    inline void getListInt(CString name, ListI* list){
        __BUNDLE_GET_LIST_VAL_BY_NAME(name, getListInt);
    }
    inline void getListLong(CString name, ListL* list){
        __BUNDLE_GET_LIST_VAL_BY_NAME(name, getListLong);
    }
    inline void getListFloat(CString name, ListF* list){
         __BUNDLE_GET_LIST_VAL_BY_NAME(name, getListFloat);
    }
    inline void getListDouble(CString name, ListD* list){
        __BUNDLE_GET_LIST_VAL_BY_NAME(name, getListDouble);
    }
    inline void getListBoolean(CString name, ListB* list){
        __BUNDLE_GET_LIST_VAL_BY_NAME(name, getListBoolean);
    }
    inline void getListString(CString name, ListS* list){
        __BUNDLE_GET_LIST_VAL_BY_NAME(name, getListString);
    }
    inline void getBundle(CString name, Bundle* list){
         __BUNDLE_GET_LIST_VAL_BY_NAME(name, getBundle);
    }

#define __BUNDLE_GET_LIST_VAL(t,func)\
    sk_sp<t> list = sk_make_sp<t>();\
    func(info, list.get());\
    return list;

    inline sk_sp<ListCh> getListByte(PosInfo& info){
        __BUNDLE_GET_LIST_VAL(ListCh, getListByte)
    }
    inline sk_sp<ListSh> getListShort(PosInfo& info){
        __BUNDLE_GET_LIST_VAL(ListSh, getListShort)
    }
    inline sk_sp<ListI> getListInt(PosInfo& info){
        __BUNDLE_GET_LIST_VAL(ListI, getListInt)
    }
    inline sk_sp<ListL> getListLong(PosInfo& info){
        __BUNDLE_GET_LIST_VAL(ListL, getListLong)
    }
    inline sk_sp<ListF> getListFloat(PosInfo& info){
        __BUNDLE_GET_LIST_VAL(ListF, getListFloat)
    }
    inline sk_sp<ListD> getListDouble(PosInfo& info){
        __BUNDLE_GET_LIST_VAL(ListD, getListDouble)
    }
    inline sk_sp<ListB> getListBoolean(PosInfo& info){
        __BUNDLE_GET_LIST_VAL(ListB, getListBoolean)
    }
    inline sk_sp<ListS> getListString(PosInfo& info){
        __BUNDLE_GET_LIST_VAL(ListS, getListString)
    }
    inline sk_sp<Bundle> getBundle(PosInfo& info){
        __BUNDLE_GET_LIST_VAL(Bundle, getBundle)
    }
    void getBundle(PosInfo& info, Bundle* out);
    inline char* getBooleanArray(PosInfo& info){
        return m_data.list.data() + info.dataPos;
    }
    //-------------------------------------
    inline void getListByte(PosInfo& info, ListCh* list){
        getListByte(info, &list->list);
    }
    inline void getListShort(PosInfo& info, ListSh* list){
        getListShort(info, &list->list);
    }
    inline void getListInt(PosInfo& info, ListI* list){
        getListInt(info, &list->list);
    }
    inline void getListLong(PosInfo& info, ListL* list){
        getListLong(info, &list->list);
    }
    inline void getListFloat(PosInfo& info, ListF* list){
        getListFloat(info, &list->list);
    }
    inline void getListDouble(PosInfo& info, ListD* list){
        getListDouble(info, &list->list);
    }
    inline void getListBoolean(PosInfo& info, ListB* list){
        getListBoolean(info, &list->list);
    }
    inline void getListString(PosInfo& info, ListS* list){
        getListString(info, &list->list);
    }
    //---------------------------------------------
    void getListByte(PosInfo& info, std::vector<char>* list);
    void getListShort(PosInfo& info, std::vector<short>* list);
    void getListInt(PosInfo& info, std::vector<int>* list);
    void getListLong(PosInfo& info, std::vector<long long>* list);
    void getListFloat(PosInfo& info, std::vector<float>* list);
    void getListDouble(PosInfo& info, std::vector<double>* list);
    void getListBoolean(PosInfo& info, std::vector<bool>* list);
    void getListString(PosInfo& info, std::vector<std::string>* list);
    //----------- iterator ----------

    void travel(std::function<void(Bundle*,std::string&, PosInfo*)> func);
    void travelDesc(std::function<void(Bundle*,ItemInfo*)> func);

    inline ListS* getNames(){
        return &m_names;
    }
    inline int getItemCount(){
        return m_names.size();
    }
    inline bool hasName(CString name){
        return getItemIndex(name) >= 0;
    }
    //--------------------------------------
    //must free the return data after not use it.
    char* serialize(int* outSize);
    void deSerialize(char* data, int size);
    //-----------------------------------
    bool operator==(const h7::Bundle& b2);

public:
    inline int getSize(int index){
        return *(int*)(m_descInfo.list.data() + index * DESC_UNIT_SIZE);
    }
    inline unsigned char getType(int index){
        return m_descInfo.list.data()[index * DESC_UNIT_SIZE + sizeof (unsigned int)];
    }
private:
    inline void prepareDataSpace(int delta);

    void putSimpleData(CString name, int type, char* data);

    inline void addDesc(unsigned int size, unsigned char type){
        int index = m_names.size() - 1;
        m_descInfo.resize(m_names.size() * DESC_UNIT_SIZE);
        *(unsigned int*)(m_descInfo.list.data() + index * DESC_UNIT_SIZE) = size;
        m_descInfo.list.data()[index * DESC_UNIT_SIZE + sizeof (unsigned int)] = type;
        makeIndex();
    }
    void makeIndex();
private:
    static constexpr int DESC_UNIT_SIZE = sizeof (unsigned int) + sizeof (unsigned char);
    friend class Parcel;
    ListS m_names;
    ListCh m_data;
    ListUCh m_descInfo;         //size+type
    //---- cache for fast visit ------
    sk_sp<ListUI> m_indexes;
    sk_sp<ListUI> m_hashes;

    typedef union{
        char arr[4];
        int val;
    }Uni_Int;
    typedef union{
        char arr[2];
        short val;
    }Uni_Short;
    typedef union{
        char arr[8];
        long long val;
    }Uni_Long;
    typedef union{
        char arr[4];
        float val;
    }Uni_Float;
    typedef union{
        char arr[8];
        double val;
    }Uni_Double;
};

}

#endif // BUNDLE_H

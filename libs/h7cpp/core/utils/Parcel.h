#ifndef PARCEL_H
#define PARCEL_H

#include "utils/Bundle.h"

namespace h7 {

class Parcel
{
public:

    inline void putString(CString name, CString data){
        putBlockData(name, (char*)data.data(), data.length() + 1);
    }
    inline void putBlob(CString name, char* data, unsigned int size){
        putBlockData(name, data, size);
    }
    inline void putByte(CString name, char val){
        char data[1];
        data[0] = val;
        putSimpleData(name, data, sizeof (char));
    }
    inline void putShort(CString name, short val){
        Bundle::Uni_Short ui;
        ui.val = val;
        putSimpleData(name, ui.arr, sizeof (short));
    }
    inline void putInt(CString name, int val){
        Bundle::Uni_Int ui;
        ui.val = val;
        putSimpleData(name, ui.arr, sizeof (int));
    }
    inline void putLong(CString name, long long val){
        Bundle::Uni_Long ui;
        ui.val = val;
        putSimpleData(name, ui.arr, sizeof (long long));
    }
    inline void putFloat(CString name, float val){
        Bundle::Uni_Float ui;
        ui.val = val;
        putSimpleData(name, ui.arr, sizeof (float));
    }
    inline void putDouble(CString name, double val){
        Bundle::Uni_Double ui;
        ui.val = val;
        putSimpleData(name, ui.arr, sizeof (double));
    }
    inline void putBoolean(CString name, bool val){
        char arr[1];
        arr[0] = val;
        putSimpleData(name, arr, sizeof (char));
    }
    inline void putListByte(CString name, ListCh* list){
        unsigned int dSize = list->size() * sizeof (char);
        putBlockData(name, (char*)list->list.data(), dSize);
    }
    inline void putListShort(CString name, ListSh* list){
        unsigned int dSize = list->size() * sizeof (short);
        putBlockData(name, (char*)list->list.data(), dSize);
    }
    inline void putListInt(CString name, ListI* list){
        unsigned int dSize = list->size() * sizeof (int);
        putBlockData(name, (char*)list->list.data(), dSize);
    }
    inline void putListLong(CString name, ListL* list){
        unsigned int dSize = list->size() * sizeof (long long);
        putBlockData(name, (char*)list->list.data(), dSize);
    }
    inline void putListFloat(CString name, ListF* list){
        unsigned int dSize = list->size() * sizeof (float);
        putBlockData(name, (char*)list->list.data(), dSize);
    }
    inline void putListDouble(CString name, ListD* list){
        unsigned int dSize = list->size() * sizeof (double);
        putBlockData(name, (char*)list->list.data(), dSize);
    }
    inline void putListBoolean(CString name, ListB* list){
        unsigned int dSize = list->size() * sizeof (char);
        sk_sp<IColumn<char>> ret = sk_make_sp<IColumn<char>>(list->size());
        for(int i = 0 ; i < list->size() ; i ++){
            ret->add(list->const_get(i) ? 1 : 0);
        }
        putBlockData(name, ret->list.data(), dSize);

    }
    void putListString(CString name, ListS* list);

    inline void putParcel(CString name,Parcel* b){
        unsigned int size;
        char* data = b->serialize(&size);
        putBlockData(name, data, size);
    }
    //-------------------------------------
    inline void putListByte(CString name, char* data, int len){
        putBlockData(name, data, len * sizeof (char));
    }
    inline void putListShort(CString name, short* data, int len){
        putBlockData(name, (char*)data, len * sizeof (short));
    }
    inline void putListInt(CString name, int* data, int len){
        putBlockData(name, (char*)data, len * sizeof (int));
    }
    inline void putListLong(CString name, long* data, int len){
        putBlockData(name, (char*)data, len * sizeof (long));
    }
    inline void putListBoolean(CString name, char* data, int len){
        putBlockData(name, data, len * sizeof (char));
    }
    inline void putListFloat(CString name, float* data, int len){
        putBlockData(name, (char*)data, len * sizeof (float));
    }
    inline void putListDouble(CString name, double* data, int len){
        putBlockData(name, (char*)data, len * sizeof (double));
    }
    //need free the result data
    char* serialize(unsigned int* outSize);

    void deSerialize(char* data,unsigned int size);

    inline String nextName(){
        if(m_pos == m_data.size()){
            return "";
        }
        short len = *(short*)(m_data.data() + m_pos);
        String name(m_data.data() + m_pos + sizeof (short), len - 1);
        m_pos += sizeof (short) + len;
        return name;
    }
    inline void reset(){
        m_pos = 0;
    }
    //-------------------------
    inline char nextByte(){
        return m_data.data()[m_pos ++ ];
    }
    inline short nextShort(){
         short ret = *(short*)(m_data.data() + m_pos);
         m_pos += sizeof (short);
         return ret;
    }
    inline int nextInt(){
         int ret = *(int*)(m_data.data() + m_pos);
         m_pos += sizeof (int);
         return ret;
    }
    inline long long nextLong(){
         long long ret = *(long long*)(m_data.data() + m_pos);
         m_pos += sizeof (long long);
         return ret;
    }
    inline float nextFloat(){
         float ret = *(float*)(m_data.data() + m_pos);
         m_pos += sizeof (float);
         return ret;
    }
    inline double nextDouble(){
         double ret = *(double*)(m_data.data() + m_pos);
         m_pos += sizeof (double);
         return ret;
    }
    inline bool nextBoolean(){
         return m_data.data()[m_pos ++ ] != 0;
    }
    inline String nextString(){
         unsigned int size;
         char * data = nextBlockData(&size);
         return String(data, size - 1);
    }
    void nextListByte(std::vector<char>* out);
    void nextListShort(std::vector<short>* out);
    void nextListInt(std::vector<int>* out);
    void nextListLong(std::vector<long long>* out);
    inline void nextListBoolean(std::vector<char>* out){
        nextListByte(out);
    }
    void nextListFloat(std::vector<float>* out);
    void nextListDouble(std::vector<double>* out);
    void nextListString(std::vector<String>* out);
    inline void nextParcel(Parcel* parcel){
        unsigned int size;
        char * data = nextBlockData(&size);
        parcel->deSerialize(data, size);
    }
    //--------------------------------

    inline long long getDataPos(){
        return m_pos;
    }
    inline void setDataPos(long long pos){
        m_pos = pos;
    }
private:
    std::vector<char> m_data;
    unsigned int m_pos {0};

    char* nextBlockData(unsigned int* sizePtr);
    void prepareDataSpace(unsigned int delta);
    void putSimpleData(CString name, char* data, unsigned int size);
    void putBlockData(CString name, char* data, unsigned int size);
};

}
#endif // PARCEL_H

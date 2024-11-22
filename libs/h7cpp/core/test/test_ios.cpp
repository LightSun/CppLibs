
#include "io/FileInputStream.h"
#include "io/StringInputStream.h"
#include "io/MappedInputStream.h"
#include "io/BufferedInputStream.h"

#include "io/FileOutputStream.h"
#include "io/StringOutputStream.h"
#include "io/MappedOutputStream.h"

#include "utils/Random.h"
#include "common/common.h"
#include "utils/Numbers.hpp"

using namespace h7;
using Long = long long;
using ULong = unsigned long long;
#ifndef uint
using uint = unsigned int;
#endif

struct TestStruct{
    bool val_b {true};
    short val_short {235};
    int val_int {-123};
    uint val_uint {123};
    Long val_long {-123456789};
    ULong val_ulong {123456789};
    float val_float {12340.567899};
    double val_double {12340.567899123456789};
    String str {"12340.567899123456789"};

    //friend bool operator==(const TestStruct& t1, const TestStruct& t2);

    void write(OutputStream* os){
        os->writeBool(val_b);
        os->writeShort(val_short);
        os->writeInt(val_int);
        os->writeUInt(val_uint);
        os->writeLong(val_long);
        os->writeULong(val_ulong);
        os->writeFloat(val_float);
        os->writeDouble(val_double);
        os->writeString(str);
    }
    void read(InputStream* in){
        in->readBool(val_b);
        in->readShort(val_short);
        in->readInt(val_int);
        in->readUInt(val_uint);
        in->readLong(val_long);
        in->readULong(val_ulong);
        in->readFloat(val_float);
        in->readDouble(val_double);
        str = in->readString();
    }
    void setZero(){
        memset(this, 0, sizeof(TestStruct));
    }
    bool equals(const TestStruct& t2) const{
        auto& t1 = *this;
#define _EQ(n) \
    if(t1.n != t2.n){\
        return false;\
    }
        _EQ(val_b);
        _EQ(val_short);
        _EQ(val_int);
        _EQ(val_uint);
        _EQ(val_long);
        _EQ(val_ulong);
        _EQ(val_float);
        _EQ(str);
#undef _EQ
        return IsAlmostEqual(t1.val_double, t2.val_double);
    }
};

static inline bool operator==(const TestStruct& t1, const TestStruct& t2){
    return t1.equals(t2);
}

//template <typename ...Args>
//void writeFmt(int level, const char* fmt, Args&& ... args){
template <typename T, typename ...Args>
static inline std::unique_ptr<T> create(Args&& ... args){
    return std::make_unique<T>(std::forward<Args>(args)...);
}

static void test_io_string();
static void test_io_file();
static void test_io_mem_mapped();

void test_IOs(){
    test_io_string();
    test_io_file();
    test_io_mem_mapped();
}

void test_io_mem_mapped(){
    TestStruct ts1;
    TestStruct ts2;
    ts2.setZero();
    String str0 = "test.data";
    //current. MemoryMapped only can read.
//    {
//    auto fos = create<MappedOutputStream>();
//    MED_ASSERT(fos->open(str0, "4096"));
//    ts1.write(fos.get());
//    }
    {
    auto fis = create<MappedInputStream>();
    MED_ASSERT(fis->open(str0, "4096"));
    ts2.read(fis.get());
    MED_ASSERT(ts1 == ts2);
    }
}

void test_io_file(){
    TestStruct ts1;
    TestStruct ts2;
    ts2.setZero();
    String str0 = "test.data";
    {
    auto fos = create<FileOutputStream>();
    MED_ASSERT(fos->open(str0, ""));
    ts1.write(fos.get());
    }
    {
    FileInputStream in;
    auto fis = create<BufferedInputStream>(&in);
    MED_ASSERT(fis->open(str0, ""));
    ts2.read(fis.get());
    MED_ASSERT(ts1 == ts2);
    }
}

void test_io_string(){
    TestStruct ts1;
    TestStruct ts2;
    //ts2.val_double = 342342.4543524;
    ts2.setZero();
    String str0 = "github/heaven7";
    auto fos = create<StringOutputStream>();
    MED_ASSERT(fos->open(str0, ""));
    ts1.write(fos.get());
    String buf_out = fos->getBuffer();
    auto fis = create<StringInputStream>();
    MED_ASSERT(fis->open(buf_out, ""));
    auto _str = fis->readRawString(str0.length());
    MED_ASSERT(_str == str0);
    ts2.read(fis.get());
    MED_ASSERT(ts1 == ts2);
}

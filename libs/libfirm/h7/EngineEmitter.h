#ifndef ENGINEEMITTER_H
#define ENGINEEMITTER_H

#include "h7/h7_common.h"
#include "h7/common/Column.h"
#include "h7/common/HashMap.h"

#define H7_G_ALLOC_STR(g, size) (g)->ca->Alloc(size + 1)
#define H7_G_REALLOC_STR(g, size) (g)->ca->Realloc(size + 1)
#define H7_G_FREE_DATA(g, data) (g)->ca->Free(data)

#define H7_G_ALLOC_ARR(g, _struct, count) (g)->ca->Alloc(sizeof(int) + sizeof(_struct) * count)
#define H7_G_REALLOC_ARR(g, _struct, count) (g)->ca->Realloc(sizeof(int) + sizeof(_struct) * count)

typedef h7::IColumn<> List;
typedef h7::HashMap<> Map;

typedef struct Data{
    void* d;
    uint32 l; //used length
    uint32 m; //malloc
}Data;

struct GContext{
    //struct core_allocator* ca;
    List<String> types;
    Map<String, List<String> type_generics;//泛型
};

struct BaseMember{ //base member
    std::string name;
    std::string type;
};

enum ValueType{
    SINT8 = 0,
    UINT8,
    SINT16,
    UINT16,
    SINT32,
    UINT32,
    SINT64,
    UINT64,
    FLOAT,
    DOUBLE,
    STRING,
    VAR,
    STAT
};

struct ValueDef{
    //const,string,varable(local, member, global). other statement
    typedef union{
       sint8 s8;
       uint8 u8;
       sint16 s16;
       uint16 u16;
       sint32 s32;
       uint32 u32;
       sint64 s64;
       uint64 u64;
       float f;
       double d;
       char* varName;
       void* stat;   //StatementDef*
    }RTVal;
    RTVal val;
    uint32 type;
};

struct OpDef{
    ValueDef* left;
    ValueDef* right; // may null. like a++;
    uint32 op;//+ - * / % ~ ^ ! << >> ++ -- (+= -=...) [] . > < >= <= != is
    //typedef A<int> aa; ?
    //return a;
    //A a = new A();
    // a = b[1]
};

struct StatementDef{ //statement
    ValueDef* left {nullptr};
    ValueDef* right {nullptr}; // may null. like a++;
    uint32 op;//+ - * / % ~ ^ ! << >> ++ -- (+= -=...) [] . > < >= <= != is
    //typedef A<int> aa; ?
    //return a;
    ~StatementDef(){
        if(left){
            delete left;
        }
        if(right){
            delete right;
        }
    }
};

struct AnnoDef{
    List<BaseMember> baseMmebers;
    List<ValueDef> values;
};

struct FieldDef{
    struct BaseMember base;
    uint32 flags; //pri,pub, <T> and etc
    List<AnnoDef> annos;
};

struct ClassDef;
struct FuncDef{
    std::string name;
    struct ClassDef* owner {nullptr}; //may null, null means global function
    List<BaseMember> paramBMs;
    std::string retType;
    List<StatementDef> statements;
    List<AnnoDef> annoDefs;
    uint32 flags;
};

struct ClassDef{
    struct FuncDef init;
    struct FuncDef* dinit{nullptr};
    struct FuncDef* sinit{nullptr};
    std::string pkg;
    std::string name;
    List<FieldDef> fields;
    List<FuncDef> funcs;
   // char** genericTypes; //to global
    ~ClassDef(){
        if(dinit){
            delete dinit;
        }
        if(sinit){
            delete sinit;
        }
    }
};

struct ModuleEmitter{
    List<ClassDef> classes;
};

#endif // ENGINEEMITTER_H

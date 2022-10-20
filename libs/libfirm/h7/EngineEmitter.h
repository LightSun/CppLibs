#ifndef ENGINEEMITTER_H
#define ENGINEEMITTER_H

#include "h7/h7_common.h"

#define H7_G_ALLOC_STR(g, size) (g)->ca->Alloc(size + 1)
#define H7_G_REALLOC_STR(g, size) (g)->ca->Realloc(size + 1)
#define H7_G_FREE_DATA(g, data) (g)->ca->Free(data)

#define H7_G_ALLOC_ARR(g, _struct, count) (g)->ca->Alloc(sizeof(int) + sizeof(_struct) * count)
#define H7_G_REALLOC_ARR(g, _struct, count) (g)->ca->Realloc(sizeof(int) + sizeof(_struct) * count)

typedef struct Data{
    void* d;
    uint32 l; //used length
    uint32 m; //malloc
}Data;

struct GContext{
    struct core_allocator* ca;
    void* strMap;    //<hash, Data>
    char** typeDef;
    void* typedefMap;//<hash, hash>
};

struct BaseMember{ //base member
    char* name;
    char* type;
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
    VAR
};

struct ValueDef{
    //const,string,varable(local, member, global).
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
};

struct StatementDef{ //statement
    char* name;
    char* type;
    OpDef op;
};

struct AnnoDef{
    void* baseMmebers;
    void* values;
};

struct FieldDef{
    struct BaseMember base;
    uint32 flags; //pri,pub, <T> and etc
    void* annoDef;
};

struct ClassDef;
struct FuncDef{
    char* name;
    struct ClassDef* classPtr;//may null, null means global function
    BaseMember** paramBMs;
    char* retType;
    StatementDef** statements;
    AnnoDef** annoDefs;
    uint32 flags;
};

struct ClassDef{
    struct FuncDef init;
    struct FuncDef* dinit;
    struct FuncDef* sinit;
    char* pkg;
    char* name;
    FieldDef** fields;
    FuncDef** funcs;
    char** genericTypes;
};

struct ModuleEmitter{
    void defineClass(char* pkg);
};

#endif // ENGINEEMITTER_H

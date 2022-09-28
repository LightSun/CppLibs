#ifndef ENGINEEMITTER_H
#define ENGINEEMITTER_H

#include "h7/h7_common.h"

namespace h7_engine {

typedef struct Data{
    char* d;
    uint32 l;
    uint32 m;
}Data;

struct GContext{
    void* typePool; //global type pool
};

struct BM{ //base member
    char* name;
    char* type;
};

struct ValueDef{
    //const,string,varable(local, member, global), class-name
};

struct OpDef{
    ValueDef* left;
    ValueDef* right;
    uint32 op;//+ - * / % ~ ^ ! << >> ++ -- (+= -=...) [] . > < >= <= != is
};

struct SMDef{
    char* name;
    OpDef op;
};

struct AnnoDef{
    void* baseBMs;
    void* values;
};

struct FieldDef{
    uint32 flags; //pri,pub, <T> and etc
    struct BM base;
    void* annoDef;
};

struct FuncDef{
    char* name;
    void* classPtr{nullptr};//may null
    void* paramBMs;
    char* retType;
    void* statements;
    void* annoDef;
};

struct ClassDef{
    void* fields;
    void* funcs;
    void* genericType;
};

struct ClassEmitter{
    void defineField(const char* name, const char* type);
};

struct Emitter{
    ClassEmitter* beginClass();
};

}

#endif // ENGINEEMITTER_H

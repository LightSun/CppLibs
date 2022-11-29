#ifndef ENGINEEMITTER_H
#define ENGINEEMITTER_H

#include "h7/h7_common.h"
#include "h7/common/Column.h"
#include "h7/common/HashMap.h"

typedef h7::IColumn<> List;
typedef h7::HashMap<> Map;

struct GContext{
    //struct core_allocator* ca;
    List<String> types;
    Map<String, List<String> type_generics;//泛型
};

struct BaseMember{ //base member
    std::string name;
    std::string type;
};

typedef enum{
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
}ValueType;

struct ValueDef{
    //bool,const,string,varable(local, member, global). other statement
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
    ValueType type;
};

typedef enum{
    ADD = 1, DEC, MUL, DIV, MOD,
    OPN, // -10
    NOT, AND_AND, EQ_EQ, NEQ, OR_OR,  //!, &&,==, !=, ||
    GT,GE,LT,LE,
    AS,IS,NIS,      //as, is, not is
    AND, OR, NOR, BRE,LSHIFT,RSHIFT, // &,|,^,~,<<, >>
    EQ,               //equal
    ADD_EQ, DEC_EQ, MUL_EQ, DIV_EQ, MOD_EQ,
    LSHIFT_EQ, RSHIFT_EQ,
    OR_EQ,   // |=,
    AND_EQ,  // &=
}OP;

struct OpDef{
    ValueDef* left;
    ValueDef* right; // may null. like a++;
    uint32 op;//+ - * / % ~ ^ ! << >> ++ -- (+= -=...) [] . > < >= <= != is
    //typedef A<int> aa; ?
    //return a;
    //A a = new A();
    // a = b[1]
};

struct SimplStatDef{ //statement
    OpDef opd;
    OpDef* right {nullptr};
    ~SimplStatDef(){
        if(right){
            delete right;
        }
    }
};

struct IfDef0{
    SimplStatDef if1;
    List<SimplStatDef> if1_run;
};

struct IfDef{
    IfDef0 if1;
    List<IfDef0> else_if1;
    IfDef0* else1 {nullptr};
    ~IfDef(){
        if(else1){
            delete else1;
        }
    }
};
struct ForDef{
     List<SimplStatDef> init;
     SimplStatDef judge;
     List<SimplStatDef> step;
};
struct WhileDef{
     SimplStatDef judge;
     List<SimplStatDef> body;
};
struct SwitchDef{
     SimplStatDef judge;
     List<IfDef0> cases;
};

struct StatementDef{
    //if/for/while/switch
    List<SimplStatDef> stats;
    IfDef* ifdef {nullptr};
    ForDef* fordef {nullptr};
    WhileDef* whileDef {nullptr};
    SwitchDef* switchDef {nullptr};
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
    std::string pkg;
    struct ClassDef* owner {nullptr}; //may null, null means global function
    List<BaseMember> paramBMs;
    std::string retType;
    List<StatementDef> statements;
    List<AnnoDef> annoDefs;
    uint32 flags;
};

struct ClassDef{
    uint32 flags {0};//private?
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

struct Module{
    std::string pkg;
    List<ClassDef> classes;
    List<FuncDef> funcs;
};

#endif // ENGINEEMITTER_H

#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include <vector>

namespace h7 {

class FunctionManager
{
public:
    enum{
        kType_CHAR,
        kType_UCHAR,
        kType_SHORT,
        kType_USHORT,
        kType_INT,
        kType_UINT,
        kType_LONG,
        kType_ULONG,
        kType_LONGLONG,
        kType_ULONGLONG,
        kType_FLOAT,
        kType_DOUBLE,
        kType_BOOL,
    };
    enum {
        kFlag_POINTER,
    };
    struct FuncParam{
        unsigned int type;
        int flags {0};
    };
    //FunctionManager();
};

}

#endif // FUNCTIONMANAGER_H

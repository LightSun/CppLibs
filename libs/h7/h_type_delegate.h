#ifndef H_KEY_DELEGATE_H
#define H_KEY_DELEGATE_H

#include "h_common.h"
#define H_TYPE_NAME_PTR "pointer"

struct hstring;

typedef struct h_type_delegate{
    const char* name;
    int (*Func_size)();
    int (*Func_hash)(h_common_union* k);
    void (*Func_delete)(h_common_union* k);
    //cast spec data to common
    //void (*Func_PassToCommon)(void* data, int index, h_common_union* out);
    // < 0 means <. > 0 means >
    int (*Func_Compare)(h_common_union* k1, h_common_union* k2);
    void (*Func_ToString)(h_common_union* v1, struct hstring* hs);
    void (*Func_Copy)(h_common_union* k, h_common_union* k2, void* ud);
}h_type_delegate;

#endif // H_KEY_DELEGATE_H

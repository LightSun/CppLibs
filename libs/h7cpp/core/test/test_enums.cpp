#include "common/enums.h"

enum Color{ RED = -10, BLUE = 0, GREEN = 10 };

DEF_ENUM_FUNCS_HEAD(Color, RED, BLUE, GREEN);
DEF_ENUM_FUNCS_IMPL(Color, RED, BLUE, GREEN);

void test_enums(){
    auto str = enum_val_to_str_Color(BLUE);
    printf("test_enums >> enum_val_to_str: '%s'\n", str.data());
    auto val = enum_str_to_int_Color(str);
    printf("test_enums >> enum_str_to_int: '%d'\n", val);
}

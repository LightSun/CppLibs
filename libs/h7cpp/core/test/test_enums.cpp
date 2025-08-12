#include "common/enums.h"
#include "utils/Singleton.h"

enum Color{ RED = -10, BLUE = 0, GREEN = 10 };
enum class Size { Small = 10, Medium = 20, Large = 30 };

DEF_ENUM_FUNCS_HEAD(Color);
DEF_ENUM_FUNCS_IMPL(Color, RED, BLUE, GREEN);

//DEF_ENUM2_FUNC_VAL_TO_STR(Color, RED, BLUE, GREEN);


// 定义整数映射
//DEFINE_INT_MAP(int, {
//                        {1, "One"},
//                        {2, "Two"},
//                        {3, "Three"}
//                    }) {
//    ADD_INT_MAPPING(int, 4, "Four");
//    ADD_INT_MAPPING(int, 5, "Five");
//};

//// 定义枚举映射
//DEFINE_INT_MAP(Color, {
//                          {Color::RED, "Red Color"},
//                          {Color::GREEN, "Green Color"}
//                      }) {
//    ADD_INT_MAPPING(Color, Color::BLUE, "Blue Color");
//};


void test_enums(){
//    std::unordered_map<int,std::string> test_map;
//    enum_val_to_str_impl0(test_map, RED, BLUE, GREEN);

    //auto str = enum_val_to_str_Color(BLUE);
    //printf("test_enums >> enum_val_to_str: '%s'\n", str.data());
//    auto val = enum_str_to_int_Color(str);
//    printf("test_enums >> enum_str_to_int: '%d'\n", val);
}

//void tests_enums2(){
//    // 使用整数映射
//    std::cout << int_to_str(1) << std::endl;    // 输出: One
//    std::cout << int_to_str(5) << std::endl;    // 输出: Five
//    std::cout << int_to_str(10) << std::endl;   // 输出: "10" (自动转换)

//    // 使用枚举映射
//    std::cout << int_to_str(Color::RED) << std::endl;    // 输出: Red Color
//    std::cout << int_to_str(Color::BLUE) << std::endl;   // 输出: Blue Color
//    std::cout << int_to_str(Color::GREEN) << std::endl;  // 输出: Green Color

//    // 使用未定义的值
//    std::cout << int_to_str(100) << std::endl;           // 输出: "100"
//    std::cout << int_to_str(100, "Unknown") << std::endl; // 输出: Unknown

//    // 另一个枚举类型
//    std::cout << int_to_str(Size::Medium) << std::endl;  // 输出: "20" (未定义映射)
//    std::cout << int_to_str(Size::Medium, "Medium Size") << std::endl; // 输出: Medium Size
//}

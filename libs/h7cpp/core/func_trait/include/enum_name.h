// XH-CppUtilities
// C++20 enum_name.h
// Author: xupeigong@sjtu.edu.cn
// Last Updated: 2024-09-12

#ifndef _XH_ENUM_NAME_H_
#define _XH_ENUM_NAME_H_

#include <string_view>
#include <type_traits>
#include <string>
#include <array>

namespace xh {

using namespace std;

// enum name

template<auto>
constexpr auto enum_name() {
#if __GNUC__ || __clang__
  string_view name = __PRETTY_FUNCTION__;
  size_t start = name.find('=') + 2, end = name.size() - 1;
#elif _MSC_VER
  string_view name = __FUNCSIG__;
  size_t start = name.find('<') + 1, end = name.rfind('>');
#endif
  name = string_view{name.data() + start, end - start};
  if ((start = name.rfind("::")) != string_view::npos)
    name = string_view{name.data() + start + 2, name.size() - start - 2};
  return name.find(')') == string_view::npos ? name : "";
}

template<typename T, size_t N = 0>
constexpr auto enum_max() {
  if constexpr (!enum_name<static_cast<T>(N)>().empty())
    return enum_max<T, N + 1>();
  return N;
}

// C++17 解决方案：独立模板函数
template <typename T, std::size_t... N>
constexpr auto make_enum_names_array(std::index_sequence<N...>) {
    return std::array<std::string_view, sizeof...(N)>{
        enum_name<static_cast<T>(N)>()...
    };
}

template<typename T> //requires is_enum_v<T>
constexpr auto enum_name(T value) {
    // 静态断言确保 T 是枚举类型（替代 requires 子句）
    static_assert(std::is_enum_v<T>, "T must be an enum type");
    constexpr auto num = enum_max<T>(); // 获取枚举项数量

    // 生成索引序列并创建名称数组
    constexpr auto names = make_enum_names_array<T>(
        std::make_index_sequence<num>{} );

    return names[static_cast<std::size_t>(value)]; // 返回对应名称
}

//template<typename T> requires is_enum_v<T>
//constexpr auto enum_name(T value) {
//    constexpr auto num = enum_max<T>();
//    constexpr auto names = []<size_t... N>(std::index_sequence<N...>) {
//        return std::array<string_view, num>{enum_name<static_cast<T>(N)>()...};
//    }(make_index_sequence<num>{});
//    return names[static_cast<size_t>(value)];
//}

};

#endif //_XH_ENUM_NAME_H_

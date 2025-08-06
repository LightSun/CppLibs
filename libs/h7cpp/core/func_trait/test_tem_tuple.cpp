#include <tuple>
#include <type_traits>
#include <iostream>

// 基础模板：递归终止条件（无匹配函数）
template <size_t Index = 0, typename... Args>
void call_matching_impl(const std::tuple<>&, Args&&...) {
    static_assert(Index != Index, "No matching function found in tuple");
}

// 递归模板：检查当前函数是否匹配
template <size_t Index = 0, typename... Fns, typename... Args>
auto call_matching_impl(const std::tuple<Fns...>& tuple, Args&&... args)
    -> decltype(auto)
{
    if constexpr (Index < sizeof...(Fns)) {
        auto& func = std::get<Index>(tuple);

        // 检查函数是否可用给定参数调用
        if constexpr (std::is_invocable_v<decltype(func), Args...>) {
            return func(std::forward<Args>(args)...);
        } else {
            // 递归检查下一个函数
            return call_matching_impl<Index + 1>(tuple, std::forward<Args>(args)...);
        }
    } else {
        // 终止递归并报错
        static_assert(Index != Index, "No matching function found in tuple");
    }
}

// 入口函数
template <typename... Fns, typename... Args>
auto call_matching(const std::tuple<Fns...>& tuple, Args&&... args)
    -> decltype(auto)
{
    return call_matching_impl<0>(tuple, std::forward<Args>(args)...);
}

// 示例使用
void test_tuple_select() {
    auto func_tuple = std::make_tuple(
        [](int a, int b) {
            std::cout << "Sum: " << a + b << '\n';
            return a + b;
        },
        [](const std::string& s) {
            std::cout << "String: " << s << '\n';
            return s.size();
        },
        [](double d) {
            std::cout << "Double: " << d << '\n';
            return static_cast<int>(d);
        }
        );

    // 调用匹配函数
    call_matching(func_tuple, 3, 5);          // 匹配第一个函数
    call_matching(func_tuple, "hello world");  // 匹配第二个函数
    call_matching(func_tuple, 3.14);           // 匹配第三个函数

    // 无匹配时编译错误（测试时需注释掉）
    // call_matching(func_tuple, 'a', true);
}

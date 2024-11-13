#pragma once

#include <tuple>
#include <variant>
#include <iostream>

namespace h7 {
namespace tem1 {

template <typename T, typename Variant, typename Callback>
void wrapper(Variant& variant, Callback& callback) {
    callback(std::get<T>(variant));
}

template <typename... Ts, typename Callback>
void visit(std::variant<Ts...>& variant, Callback&& callback) {
    using Variant = std::variant<Ts...>;
    constexpr static std::array table = {&wrapper<Ts, Variant, Callback>...};
    table[variant.index()](variant, callback);
}

/*
 c++ 17 folding expression
template <typename... Ts, typename Callback>
void visit2(std::variant<Ts...>& variant, Callback&& callback) {
    auto foreach = []<typename T>(std::variant<Ts...>& variant, Callback& callback) {
        if(auto value = std::get_if<T>(&variant)) {
            callback(*value);
            return true;
        }
        return false;
    };
    (foreach.template operator()<Ts>(variant, callback) || ...);
}
*/

//编译期生成一个确定的表以供运行期查询
void test() {
    auto callback = [](auto& value) { std::cout << value << std::endl; };
    auto cb_int = [](int& value) { std::cout << value << std::endl; };
    auto cb_float = [](float& value) { std::cout << value << std::endl; };
    auto cb_str = [](std::string& value) { std::cout << value << std::endl; };
    std::variant<int, float, std::string> variant = 42;
    visit(variant, callback);//cb_int also ok
    variant = 3.14f;
    visit(variant, callback);
    variant = "Hello, World!";
    visit(variant, callback);
}

}
}


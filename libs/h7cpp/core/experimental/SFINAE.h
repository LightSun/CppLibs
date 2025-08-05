#pragma once

#include <type_traits>
#include <iostream>
#include <vector>

namespace sfinae {

// 用于判断两个类型是否相同 默认情况下返回 false_type
template<typename T1, typename T2>
struct IsSameType : std::false_type {};

// 如果两个类型相同，则特化版本返回 true_type
template<typename T1>
struct IsSameType<T1, T1> : std::true_type {};

class A{};
class B
{
public:
    B(int tmpvale){}
};

// 模板类 IsDefConstructible，用于检测类型T是否具有默认构造函数
template<typename T>
class IsDefConstructible
{
private:
    // 使用 std::declval<T>() 来避免潜在的构造函数调用
    // 探测能否使用默认构造函数创建类型U的实例，若可以，则返回 true_type
    template<typename U = T, typename = decltype(U())>
    static std::true_type test(void*);
    // Fallback函数，如果上述探测失败，则选择此重载，返回 false_type
    template<typename = int>
    static std::false_type test(...);

public:
    // 根据 test(nullptr) 的返回类型，通过IsSameType来确定是否和std::true_type类型相同
    // 若相同，则说明T类型可默认构造，value为true；否则为false
    static constexpr bool value = IsSameType<decltype(test(nullptr)),
                                             std::true_type>::value;
};

template<typename FROM, typename TO>
struct IsConvertible
{
private:
    // 尝试使用类型TO的参数来测试是否可以从FROM转换到TO
    static void test(TO) {}

    // 通过decltype和表达式SFINAE检测两种类型的转换函数
    template<typename F>
    static auto check(F* f) -> decltype(test(*f), std::true_type());

    // 如果上面的尝试失败，则使用这个通用函数，返回std::false_type
    static std::false_type check(...) {}

public:
    // 根据FROM类型是否可以被转换为TO类型，value将是true或false
    static constexpr bool value = decltype(check(static_cast<FROM*>(nullptr)))::value;
};

/*
  std::cout << std::is_default_constructible<int>::value << std::endl;   // 1 能被默认构造
    std::cout <<std::is_default_constructible<double>::value << std::endl; // 1 能被默认构造
    std::cout <<std::is_default_constructible<A>::value << std::endl;      // 1 能被默认构造
    std::cout <<std::is_default_constructible<B>::value << std::endl;      // 0 不能被默认构造

std::cout << std::is_convertible< float, int>::value << std::endl; // 1 可以转换
std::cout <<std::is_convertible<int, float>::value << std::endl;   // 1 可以转换
std::cout << std::is_convertible<A,B>::value << std::endl;         // 0 不可以转换
std::cout << std::is_convertible<B, A>::value << std::endl;        // 1 可以转换
*/

template<typename T>
class IsClass
{
private:
    // 尝试声明一个成员指针，仅当 T 是类类型时才有效
    template<typename U>
    static std::integral_constant<bool, !std::is_union<U>::value> test(int U::*);

    // 备选函数，当第一个函数模板不适用时调用
    template<typename>
    static std::integral_constant<bool, false> test(...);

public:
    // 比较两个类型，确认是否为 true 类型
    static constexpr bool value = IsSameType<decltype(test<T>(nullptr)),
                                             std::integral_constant<bool, true>>::value;
};

//--------------
template<typename Base, typename Derived>
class IsBaseOf
{
private:
    // 试图将 Derived* 转换为 Base*，如果成功则选择此重载
    template<typename T>
    static std::true_type test(T*);

    // 如果转换失败，则选择此重载
    template<typename>
    static std::false_type test(void*);

    // 利用 decltype 和 SFINAE 确定 test 函数的返回类型
    template<typename B, typename D>
    static auto test_middle() -> decltype(test<B>(static_cast<D*>(nullptr)));

public:
    // 判断是否是基类关系，需要满足两个条件：
    // 1. Base 和 Derived 都是类类型；2. 能够将 Derived* 转换为 Base*
    static constexpr bool value =
        IsSameType<std::integral_constant
                   <bool, std::is_class_v<Base> && std::is_class_v<Derived> &&
                              decltype(test_middle<Base,Derived>())::value>,
                   std::integral_constant<bool,true>>::value;
};

}

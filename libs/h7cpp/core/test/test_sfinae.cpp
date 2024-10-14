
#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <type_traits>

namespace h1 {
    template<typename A>
    struct B { using type = typename A::type; };

    template<
        class T,
        class U = typename T::type,    // 如果 T 没有成员 type 那么就是 SFINAE 失败
        class V = typename B<T>::type> // 如果 T 没有成员 type 那么就是硬错误
                                       // （经由 CWG 1227 保证不出现，
                                       // 因为到 U 的默认模板实参中的替换会首先失败）
    void foo (int);

    template<class T>
    typename T::type h(typename B<T>::type);

    template<class T>
    auto h(typename B<T>::type) -> typename T::type; // 重声明

    template<class T>
    void h(...) {}

   // using R = decltype(h<int>(0));     // 错误格式，不要求诊断
}
//-----------------

namespace h2 {

template <int I>
struct X {};

template<template<class T> class>
struct Z {};

template<class T>
void f(typename T::Y*) {}

template<class T>
void g(X<T::N>*) {}

template<class T>
void h(Z<T::template TT>*) {}

struct A {};
struct B { int Y; };
struct C { typedef int N; };
struct D { typedef int TT; };
struct B1 { typedef int Y; };
struct C1 { static const int N = 0; };
struct D1
{
    template<typename T>
    struct TT {};
};

static void test1()
{
    // 下列各个情况推导失败：
//    f<A>(0); // 不含成员 Y
//    f<B>(0); // B 的 Y 成员不是类型
//    g<C>(0); // C 的 N 成员不是非类型
//    h<D>(0); // D 的 TT 成员不是模板

    // 下列各个情况推导成功：
    f<B1>(0);
    g<C1>(0);
    h<D1>(0);
}
// 未完成：需要演示重载决议，而不只是失败
}

//----------------------------------
// 主模板处理无法引用的类型：
template<class T, class = void>
struct reference_traits
{
    using add_lref = T;
    using add_rref = T;
};

// 特化识别可以引用的类型：
template<class T>
struct reference_traits<T, std::void_t<T&>>
{
    using add_lref = T&;
    using add_rref = T&&;
};

template<class T>
using add_lvalue_reference_t = typename reference_traits<T>::add_lref;

template<class T>
using add_rvalue_reference_t = typename reference_traits<T>::add_rref;


namespace h4 {
// 此重载始终在重载集中
// 省略号形参对于重载决议具有最低等级
void test(...)
{
    std::cout << "调用了保底重载\n";
}

// 如果 C 是类的引用类型且 F 是指向 C 的成员函数的指针，
// 那么这个重载会被添加到重载集
template<class C, class F>
auto test(C c, F f) -> decltype((void)(c.* f)(), void())
{
    std::cout << "调用了引用重载\n";
}

// 如果 C 是类的指针类型且 F 是指向 C 的成员函数的指针，
// 那么这个重载会被添加到重载集
template<class C, class F>
auto test(C c, F f) -> decltype((void)((c->* f)()), void())
{
    std::cout << "调用了指针重载\n";
}

struct X { void f() {} };

static void test4()
{
    X x;
    test(x, &X::f);
    test(&x, &X::f);
    test(42, 1337);
}
}

void test_SFINAE(){
}


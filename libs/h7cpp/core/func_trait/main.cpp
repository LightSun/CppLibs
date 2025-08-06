#include <any>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <set>
#include <thread>
#include <type_traits>
#include <vector>

#include "function_traits.h"
#include "function_utility.h"
#include "qualifier.h"
#include "view.h"

using namespace xh;

int fun(int) { return 0; }
char cfun(int) { return 'a'; }

namespace xh {

class A {
public:
    int num;
    static const int k = 2;
    A(int n) : num(n) {}

    const xh::getter<int> getNumMoreThanK = [&]() { return num + k; };
    xh::setter<int> setNumMoreThanK = [&](int n) { num = n - k; };
    xh::getset<int, int> numMoreThanK = {
        [&]() { return num + k; },
        [&](int n) { num = n - k; }
    };
};

class A2 {
public:
    int num;
    A2(int n) : num(n) {}

    int add(int n) volatile {
        num += n;
        return num;
    }
};

class A3 {
public:
    int num;
};

int reload_fun(int) { return 0; }
char reload_fun(char) { return 'a'; }

// 安全的成员函数代理
template <class C, typename Ret, typename... Args>
auto make_member_proxy(Ret (C::*mfp)(Args...), C* obj) {
    return [=](Args... args) -> Ret {
        return (obj->*mfp)(std::forward<Args>(args)...);
    };
}

// 带自定义逻辑的代理
template <class C, typename Ret, typename... Args, typename Func>
auto make_custom_member_proxy(Ret (C::*)(Args...), C* obj, Func&& func) {
    return [=](auto&&... args) {
        return func(obj, std::forward<decltype(args)>(args)...);
    };
}
}

//-----------------------
extern void test_tuple_select();
static void test_multi_func();
static void test_func_chain();
static void test_func_pipe();
static void test_func_proxy();
static void test_func_proxy2();

int main() {
    //  xh::multifunc f = {[](int) { return 0; }, [](char) { return 1; }};
    //
    //  cout << f(1) << endl;
    //  cout << f('a') << endl;

    xh::function ff = [](int) { return 0; };
    std::cout << ff(1) << std::endl;
    //
    {
    A a = 10;
    assert(a.getNumMoreThanK == 12);
    a.setNumMoreThanK = 13;
    assert(a.num == 11);
    assert(a.numMoreThanK == 13);
    a.numMoreThanK = 14;
    assert(a.num == 12);
    }
    //
    A2 a(12);
    xh::function mf = &A2::add;
    assert(mf(a, 2) == 14);
    mf(&a, 2);
    assert(a.num == 16);
    test_multi_func();
    return 0;
}

void test_multi_func(){
    xh::multifunc multi_fun = {
        [](int) { return 0; },
        [](char) { return 'a'; }
    };
    auto a = multi_fun('a');
    assert(multi_fun(1) == reload_fun(1));
    assert(a == reload_fun('a'));
    assert(multi_fun((short)'a') == reload_fun((short)'a'));

//    return 0;
    //test_tuple_select();
}
void test_func_chain(){
    xh::funcchain fn([](int a, int b) { return a + b; });
        fn.then([](int a) { return a * 2; })
        .then([](int res) { assert(res == (1 + 2) * 2); })
            (1, 2);
}
void test_func_pipe(){
    xh::funcpipe pf1 = [](int a, int b) { return a + b; };
    xh::funcpipe pf2 = [](int a) { return a * 2; };
    xh::funcpipe pf3 = [](int res) { assert(res == (1 + 2) * 2); };
    1 | pf1(2) | pf2 | pf3;
}
void test_func_proxy(){
    A3 a;
    int (A3::*ptr)(int);
    MEMFUNC_PROXY(ptr,(int n),{
        proxy->num += n;
        return proxy->num;
    });
    a.num = 10;
    assert((a.*ptr)(2) == 12);
}
void test_func_proxy2(){
    A3 a;
    a.num = 10;

//    auto simple_proxy = make_member_proxy(&A3::num, &a);
//    simple_proxy = 20;
//    assert(simple_proxy() == 20);

//    auto custom_proxy = make_custom_member_proxy(
//        static_cast<int(A3::*)(int)>(nullptr),  // 类型提示
//        &a,
//        [](A3* obj, int n) {
//            obj->num += n;
//            return obj->num;
//        }
//        );

//    assert(custom_proxy(2) == 22);
//    assert(a.num == 22);

}

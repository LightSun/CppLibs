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

    int add(int n) /*volatile*/ {
        num += n;
        return num;
    }
};

int reload_fun(int) { return 0; }
char reload_fun(char) { return 'a'; }
}

//-----------------------

static void test_multi_func();

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
//    xh::multifunc multi_fun = {
//        [](int) { return 0; },
//        [](char) { return 'a'; }
//    };
//    assert(multi_fun(1) == reload_fun(1));
    //assert(multi_fun('a') == reload_fun('a'));
    //assert(multi_fun((short)'a') == reload_fun((short)'a'));
    xh::funcchain fn([](int a, int b) { return a + b; });
        fn.then([](int a) { return a * 2; })
        .then([](int res) { assert(res == (1 + 2) * 2); })
            (1, 2);

    return 0;
}


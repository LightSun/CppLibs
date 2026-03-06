#include <iostream>
#include <exception>
#include <stdexcept>

#ifdef USE_CPP_BACKWARD
#include "backward.hpp"
backward::SignalHandling sh{};
#endif

static int testk(int n){
    if(n == 1){
        return 0;
    }
    return n / testk(n-1);
}

static void do_something() {
    //for error need to catch and print
   // throw std::runtime_error("throw exception from third-party library");
    testk(5);
}
static void test1() {
    std::cout << "test1 ok" << std::endl;
}
static void test2() {
    do_something();
}
static void test3() {
    std::cout << "test3 ok" << std::endl;
}
//the backtrack not ok, Why???
void test_cpp_backward() {
    // try {
    test1();
    test2();
    test3();
    // } catch (const std::exception& e) {
    //     std::cout << e.what() << std::endl;
    //     std::cout << "Exception caught: " << e.what() << std::endl;
    // }
    std::cout << "do something else" << std::endl;
}

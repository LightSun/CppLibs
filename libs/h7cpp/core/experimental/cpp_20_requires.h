#pragma once


/*
 *
 requires表达式的判定标准:对requires表达式进行模板实参的替换,如果替换之后出现无效类型,或者违反约束条件,则值为false,反之为true

 template <class T>
 concept Check = requires {
     T().clear();
 };
 ​
 template <Check T>
 struct G {};
 ​
 G<std::vector<char>> x;      // 成功
 G<std::string> y;            // 成功
 G<std::array<char, 10>> z;   // 失败

//--------------------
 template <typename T, typename T::type = 0>
 struct S;
 template <typename T>
 using Ref = T&;
 template <typename T> concept C = requires
 {
     typename T::inner; // 要求嵌套类型
     typename S<T>; // 要求类模板特化
     typename Ref<T>; // 要求别名模板特化
 };
 ​
 template <C c>
 struct M {};
 ​
 struct H {
     using type = int;
     using inner = double;
 };
 ​
 M<H> m;
//------------------------

 template <class T>
 concept Check = requires(T a, T b) {
   {a.clear()} noexcept; // 支持clear,且不抛异常
   {a + b} noexcept -> std::same_as<int>; // std::same_as<decltype((a + b)), int>
 };
 template<typename T> concept C =
 requires(T x) {
     {*x} ;   // *x有意义
 {x + 1} -> std::same_as<int>; // x + 1有意义且std::same_as<decltype((x + 1)), int>，即x+1是int类型
 {x * 1} -> std::convertible_to<T>; // x * 1 有意义且std::convertible_to< decltype((x *1),T>
 };

//-------------------------
template <class T>
 concept Check = requires(T a, T b) {
   requires std::same_as<decltype((a + b)), int>;
 };

//等同于
 template <class T>
 concept Check = requires(T a, T b) {
   {a + b} -> std::same_as<int>;
 };
*/

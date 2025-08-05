// XH-CppUtilities
// C++20 function_traits.h
// Author: xupeigong@sjtu.edu.cn
// Last Updated: 2024-09-12

#ifndef _XH_FUNCTION_TRAITS_H_
#define _XH_FUNCTION_TRAITS_H_

#include <tuple>
#include <type_traits>

#include "qualifier.h"

//std::decay_t 是 C++ 标准库中的一个类型转换工具模板，
//  用于将给定类型转换为其“自然”形式。它会移除引用、CV 限定符（const 和 volatile）
//std::remove_pointer_t 去除指针后的类型

namespace xh {

template <typename...>
struct make_void { using type = void; };
template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

template <class T>
struct is_function : std::is_function<T> {};

template <class T>
struct is_funcptr
    : std::conjunction<std::is_pointer<T>, std::is_function<std::remove_pointer_t<T>>> {};

template <class, class = void>
struct is_functor : std::false_type {};

template <class T>
struct is_functor<T, void_t<decltype(&T::operator())>> : std::true_type {};

template <class T>
struct is_memfunc : std::is_member_function_pointer<T> {};

template <class T>
inline constexpr bool is_function_v = is_function<T>::value;
template <class T>
inline constexpr bool is_funcptr_v = is_funcptr<T>::value;
template <class T>
inline constexpr bool is_functor_v = is_functor<T>::value;
template <class T>
inline constexpr bool is_memfunc_v = is_memfunc<T>::value;

// nth_of

//tuple ele
template <size_t N, class... T>
struct nth_of : std::tuple_element<N, std::tuple<T...>> {};

//tuple ele-type
template <size_t N, class... T>
using nth_of_t = typename nth_of<N, T...>::type;

// function traits

template <class T>
struct _function_traits : _function_traits<funcqual_decay_t<T>> {};

template <class T>
struct _funcptr_traits : _function_traits<std::remove_pointer_t<T>> {};

template <class T>
struct _functor_traits : _functor_traits<decltype(&T::operator())> {};

template <class T, class C>
struct _functor_traits<T C::*> : _function_traits<T> {};

template <class T>
struct _memfunc_traits;

// 普通函数特化
template <class Ret, class... Args>
struct _function_traits<Ret(Args...)> {
  using        type = Ret(Args...); //相当于: 函数签名
  using return_type = Ret;
  using   arg_tuple = std::tuple<Args...>;
  template <size_t N>
  using    arg_type = nth_of_t<N, Args...>;
  inline static constexpr size_t arity = sizeof...(Args);//param cnt
};

// 可变参数函数特化
template <class Ret, class... Args>
struct _function_traits<Ret(Args..., ...)> {
  using        type = Ret(Args..., ...);
  using return_type = Ret;
  using   arg_tuple = std::tuple<Args...>;
  template <size_t N>
  using    arg_type = nth_of_t<N, Args...>;
  inline static constexpr size_t arity = sizeof...(Args);
};

// 成员函数指针特化
template <class T, class C>
struct _memfunc_traits<T C::*> : _function_traits<T> {
  using          type = T C::*;
  using    class_type = C;
  using function_type = T;
};

//template <class T>
//struct _function_traits_helper
//  : conditional_t<is_function_v<T>, _function_traits<T>,
//      conditional_t<is_funcptr_v<T>, _funcptr_traits<T>,
//        conditional_t<is_functor_v<T>, _functor_traits<T>,
//          conditional_t<is_memfunc_v<T>, _memfunc_traits<T>, false_type>>>> {};

// 替换复杂的 conditional_t 嵌套
template <class T, bool = is_function_v<T>, bool = is_funcptr_v<T>,
          bool = is_functor_v<T>, bool = is_memfunc_v<T>>
struct _function_traits_helper;

// 普通函数版本
template <class T>
struct _function_traits_helper<T, true, false, false, false>
  : _function_traits<T> {};

// 函数指针版本
template <class T>
struct _function_traits_helper<T, false, true, false, false>
  : _funcptr_traits<T> {};

// 函数对象版本
template <class T>
struct _function_traits_helper<T, false, false, true, false>
  : _functor_traits<T> {};

// 成员函数版本
template <class T>
struct _function_traits_helper<T, false, false, false, true>
  : _memfunc_traits<T> {};

template <class T>
using function_traits = _function_traits_helper<std::decay_t<T>>;

// 别名模板
template <class T>
using function_traits_t = typename function_traits<T>::type;
template <class T>
using functraits_t = typename function_traits<T>::type;
template <class T>
using function_return_t = typename function_traits<T>::return_type;
template <class T>
using funcret_t = typename function_traits<T>::return_type;
template <class T>
using function_arg_tuple = typename function_traits<T>::arg_tuple;
template <class T>
using funcarg_tuple = typename function_traits<T>::arg_tuple;
template <class T, size_t N>
using function_arg_t = typename function_traits<T>::template arg_type<N>;
template <class T, size_t N>
using funcarg_t = typename function_traits<T>::template arg_type<N>;

template <class T>
using memfunc_class_t = typename function_traits<T>::class_type;
template <class T>
using mfclass_t = typename function_traits<T>::class_type;
template <class T>
using memfunc_function_t = typename function_traits<T>::function_type;
template <class T>
using mffunction_t = typename function_traits<T>::function_type;

template <class T>
inline constexpr size_t function_arity_v = function_traits<T>::arity;
template <class T>
inline constexpr size_t funcarity_v = function_traits<T>::arity;

}; // namespace xh

#endif //_XH_FUNCTION_TRAITS_H_

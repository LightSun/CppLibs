#pragma once

#include <type_traits>

namespace h7_cpp17 {

template <typename T, typename... Types>
using enable_if_any = std::enable_if_t<(std::is_same_v<T, Types> || ...)>;

#define EXPECT(TYPE, NAME, EXPR, ...)                                                \
  template <typename P = TYPE, typename CP = const TYPE,           \
            typename R = decltype(EXPR),                                       \
            typename = enable_if_any<R, __VA_ARGS__>>                          \
  std::true_type test_##NAME(int);                                             \
  std::false_type test_##NAME(...);                                            \
  static_assert(decltype(test_##NAME(0))::value, \
                "Expect { " #EXPR " } -> any_of {" #__VA_ARGS__ "}");

#define p std::declval<P &>()
#define cp std::declval<CP &>()
#define i std::size_t{}
#define x double{}

#define KMEMBER_CHECK(TYPE)\
EXPECT(TYPE, subscript,       p[i],            double &)\
EXPECT(TYPE, const_subscript, cp[i],           double, const double, const double &)\
EXPECT(TYPE, evaluate,        cp(x),           double, const double)\
EXPECT(TYPE, negate,          -cp,             TYPE, const TYPE)\
EXPECT(TYPE, plus,            cp + cp,         TYPE, const TYPE)\
EXPECT(TYPE, minus,           cp - cp,         TYPE, const TYPE)\
EXPECT(TYPE, multiply,        cp * cp,         TYPE, const TYPE)\
EXPECT(TYPE, add_assign,      p += cp,         TYPE &)\
EXPECT(TYPE, minus_assign,    p -= cp,         TYPE &)\
EXPECT(TYPE, multiply_assign, p *= cp,         TYPE &)\
EXPECT(TYPE, derivative,      cp.derivative(), TYPE, const TYPE)\
EXPECT(TYPE, integral,        cp.integral(),   TYPE, const TYPE)\
EXPECT(TYPE, equal,           cp == cp,        bool)\
EXPECT(TYPE, not_equal,       cp != cp,        bool)

#undef p
#undef cp
#undef i
#undef x
#undef EXPECT

}


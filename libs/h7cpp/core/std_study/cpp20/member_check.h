#pragma once

#if __cplusplus >= 202300L
#include <concepts>

namespace h7_cpp20{

namespace detail {

template <typename T, typename... U>
concept any_of = (std::same_as<T, U> || ...);

} // namespace detail

template <typename T>
concept correct_polynomial
  = requires(T p, const T cp, std::size_t i, double x) {
      { p[i] } -> std::same_as<double &>;
      { cp[i] } -> detail::any_of<double, const double, const double &>;
      { cp(x) } -> detail::any_of<double, const double>;
      { -cp } -> detail::any_of<T, const T>;
      { cp + cp } -> detail::any_of<T, const T>;
      { cp - cp } -> detail::any_of<T, const T>;
      { cp * cp } -> detail::any_of<T, const T>;
      { p += cp } -> std::same_as<T &>;
      { p -= cp } -> std::same_as<T &>;
      { p *= cp } -> std::same_as<T &>;
      { cp.derivative() } -> detail::any_of<T, const T>;
      { cp.integral() } -> detail::any_of<T, const T>;
      { cp == cp } -> std::same_as<bool>;
      { cp != cp } -> std::same_as<bool>;
    };

//static_assert(correct_polynomial<Polynomial>);

}
}
#endif

#include <iostream>

//https://zhuanlan.zhihu.com/p/646812253?utm_campaign=shareopn&utm_medium=social&utm_psn=1762032579879100416&utm_source=wechat_session

//c++20??
/*
template<std::size_t N>
struct reader
{
    friend auto counted_flag(reader<N>);
};

template<std::size_t N>
struct setter
{
    friend auto counted_flag(reader<N>) {}
    std::size_t value = N;
};

template<auto N = 0,
         auto tag = []{},
         bool condition = requires(reader<N> red){ counted_flag(red); }>
consteval auto next()
{
    if constexpr (!condition)
    {
        constexpr setter<N> s;
        return s.value;
    }
    else
    {
        return next<N + 1>();
    }
}

void test_stmp1()
{
    constexpr auto a = next();
    constexpr auto b = next();
    constexpr auto c = next();
    static_assert(a == 0 && b == 1 && c == 2);
}
*/

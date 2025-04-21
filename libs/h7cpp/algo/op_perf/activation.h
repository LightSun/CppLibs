#pragma once

#include <cmath>
#include <immintrin.h>
// 激活函数模板基类
template <typename T>
struct Activation {
    static T apply(T x) = delete; // 禁止直接实例化基类
};

// ReLU 特化
template <typename T>
struct ReLU : Activation<T> {
    static T apply(T x) {
        return x > T(0) ? x : T(0);
    }
};

// Sigmoid 特化
template <typename T>
struct Sigmoid : Activation<T> {
    static T apply(T x) {
        return T(1) / (T(1) + std::exp(-x));
    }
};


// AVX2 特化 ReLU
template <>
struct ReLU<float> : Activation<float> {
    static float apply(float x) {
        __m128 vec = _mm_set_ss(x);
        vec = _mm_max_ss(vec, _mm_setzero_ps());
        return _mm_cvtss_f32(vec);
    }
};

enum class ActType  {
    relu,
    sigmoid
};

#pragma once

#include <unordered_map>
#include <string>
#include <iostream>
#include "json/med_json.h"

#define DEFAULT_ENUM_VAL -10000

#define ENUM_VAL_TO_STR0(name)\
    _map.emplace(Type::name, #name)

#define ENUM_STR_TO_VAL0(name)\
    _map.emplace(#name, Type::name)

#define DEF_ENUM_FUNC_VAL_TO_STR(T,...)\
    template<typename Type = T>\
    std::string enum_val_to_str_##T##_impl(\
                int val, std::unordered_map<int,std::string>& _map){\
        PP_JSON_OP(ENUM_VAL_TO_STR0, __VA_ARGS__)\
        auto it = _map.find(val);\
        if(it != _map.end()){\
            return it->second; \
        }\
        return "";\
    }\
    std::string enum_val_to_str_##T(int val){\
        static std::unordered_map<int,std::string> _map_val_to_str_##T;\
        return enum_val_to_str_##T##_impl(val, _map_val_to_str_##T);\
    }

#define DEF_ENUM_FUNC_STR_TO_VAL(T,...)\
    template<typename Type = T>\
    int enum_str_to_int_##T##_impl(\
                const std::string& val, std::unordered_map<std::string,int>& _map, int def){\
        PP_JSON_OP(ENUM_STR_TO_VAL0, __VA_ARGS__)\
        auto it = _map.find(val);\
        if(it != _map.end()){\
            return it->second; \
        }\
        return def;\
    }\
    int enum_str_to_int_##T(const std::string& val, int def){\
        static std::unordered_map<std::string,int> _map_str_to_val_##T;\
        return enum_str_to_int_##T##_impl(val, _map_str_to_val_##T, def);\
    }

#define DEF_ENUM_FUNCS_IMPL(Type,...)\
    DEF_ENUM_FUNC_VAL_TO_STR(Type, ##__VA_ARGS__)\
    DEF_ENUM_FUNC_STR_TO_VAL(Type, ##__VA_ARGS__)

#define DEF_ENUM_FUNCS_HEAD(T)\
    std::string enum_val_to_str_##T(int val);\
    int enum_str_to_int_##T(const std::string& val, int def = DEFAULT_ENUM_VAL);\

//---------------------------
//stop case

void enum_val_to_str_impl0(std::unordered_map<int,std::string>& _map){
}
template<typename T, typename ...Args>
void enum_val_to_str_impl0(std::unordered_map<int,std::string>& _map,
                                 T first, Args... args){
    std::cout << first << std::endl;
    _map.emplace(first, std::to_string(first));
    enum_val_to_str_impl0(_map, args...);
}
#define DEF_ENUM2_FUNC_VAL_TO_STR(T,...)\
    std::string enum_val_to_str_##T(int val){\
        static std::unordered_map<int,std::string> _map_val_to_str_##T;\
        if(_map_val_to_str_##T.empty()){\
            enum_val_to_str_impl0(_map_val_to_str_##T, __VA_ARGS__);\
        }\
        auto it = _map_val_to_str_##T.find(val);\
        if(it != _map_val_to_str_##T.end()){\
            return it->second; \
        }\
        return "";\
    }
//------------------------------------------------------

#include <string>
#include <unordered_map>
#include <type_traits>

// 基础映射模板类
template<typename T>
struct IntStringMap {
    static std::unordered_map<int, std::string> map;
};

template<typename T>
std::unordered_map<int, std::string> IntStringMap<T>::map;

// 映射函数模板
template<typename T>
const std::string& int_to_str(T value, const std::string& default_str = "") {
    int int_val;
    if constexpr (std::is_enum_v<T>){
        using UnderlyingType = std::conditional_t<
            std::is_enum_v<T>,
            std::underlying_type_t<T>,
            T
            >;
        int_val = static_cast<int>(static_cast<UnderlyingType>(value));
    }else{
        int_val = static_cast<int>(value);
    }
    auto& m = IntStringMap<T>::map;
    auto it = m.find(int_val);

    if (it != m.end()) return it->second;
    return default_str.empty() ? m[int_val] = std::to_string(int_val) : default_str;
}

// 定义映射的宏.
#define DEFINE_INT_MAP(Type, ...) \
    template<> \
    std::unordered_map<int, std::string> IntStringMap<Type>::map = __VA_ARGS__; \
    /*确保映射在 main 前初始化*/\
    void _init_map_##Type() __attribute__((constructor)); \
    void _init_map_##Type()

// 添加映射项的宏
#define ADD_INT_MAPPING(Type, value, str) \
    IntStringMap<Type>::map[static_cast<int>(value)] = str



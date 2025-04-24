#pragma once

#include <unordered_map>
#include <string>
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

#define DEF_ENUM_FUNCS_HEAD(T,...)\
    std::string enum_val_to_str_##T(int val);\
    int enum_str_to_int_##T(const std::string& val, int def = DEFAULT_ENUM_VAL);\


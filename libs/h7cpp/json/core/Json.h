#pragma once

#include <fstream>
#include "json/core/med_json.h"

namespace h7 {

template<typename Bean>
using FuncToJson = void(nlohmann::json& j, const Bean& p);

template<typename T, typename Func = FuncToJson<T>>
std::string json_toJson(const T& t){
    nlohmann::json json;
    //h7::json_impl::to_json(json, t);
    Func(json, t);
    return json.dump();
}

template<typename T>
T json_fromJson(const std::string& str){
    nlohmann::json json;
    return json.parse(str).get<T>();
}

template<typename T>
T json_fromJsonFile(const std::string& path){
    nlohmann::json json;
    std::ifstream f(path);
    return json.parse(f).get<T>();
}

}

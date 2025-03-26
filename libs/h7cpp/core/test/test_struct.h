#pragma once


#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cctype>

using namespace std;

namespace struct1 {

// 类型信息结构体，包含类型大小和对齐要求
struct TypeInfo {
    size_t size;
    size_t alignment;
};

// 常见类型映射表（64位系统）
static const unordered_map<string, TypeInfo> type_map = {
    {"char",    {1, 1}},
    {"short",   {2, 2}},
    {"int",     {4, 4}},
    {"float",   {4, 4}},
    {"double",  {8, 8}},
    {"long",    {8, 8}},
    {"void*",   {8, 8}}
};

// 结构体成员布局信息
struct MemberLayout {
    size_t offset;
    size_t size;
    size_t alignment;
    size_t padding_before;
};

// 完整结构体布局信息
struct StructLayout {
    vector<MemberLayout> members;
    size_t total_size;
    size_t alignment;
    size_t final_padding;
};

// 打印结构体布局信息
static void print_layout(const StructLayout& layout) {
    cout << "结构体总大小：" << layout.total_size << " 字节\n";
    cout << "结构体对齐要求：" << layout.alignment << " 字节\n";
    cout << "末尾填充：" << layout.final_padding << " 字节\n\n";

    for (size_t i = 0; i < layout.members.size(); ++i) {
        const auto& m = layout.members[i];
        cout << "成员 " << i+1 << ":\n"
             << "  类型大小：" << m.size << " 字节\n"
             << "  对齐要求：" << m.alignment << " 字节\n"
             << "  偏移地址：" << m.offset << "\n"
             << "  前部填充：" << m.padding_before << " 字节\n"
             << endl;
    }
}

}

#include "test_struct.h"

using namespace std;

namespace struct1 {

// 解析数组类型（例如 "char[6]"）
bool parse_array_type(const string& type_name, string& base_type, size_t& count) {
    size_t bracket_pos = type_name.find('[');
    if (bracket_pos == string::npos) return false;

    size_t end_pos = type_name.find(']', bracket_pos);
    if (end_pos == string::npos || end_pos != type_name.length()-1) return false;

    base_type = type_name.substr(0, bracket_pos);
    string count_str = type_name.substr(bracket_pos+1, end_pos-bracket_pos-1);

    try {
        count = stoul(count_str);
    } catch (...) {
        return false;
    }
    return true;
}

struct1::StructLayout compute_struct_layout2(const vector<string>& member_types) {
    struct1::StructLayout layout;
    size_t current_offset = 0;
    size_t max_alignment = 0;

    for (const auto& type_name : member_types) {
        struct1::TypeInfo type_info;
        string base_type;
        size_t array_count;

        // 处理数组类型
        if (parse_array_type(type_name, base_type, array_count)) {
            auto it = type_map.find(base_type);
            if (it == type_map.end()) {
                cerr << "未知基础类型: " << base_type << endl;
                exit(EXIT_FAILURE);
            }
            type_info.size = it->second.size * array_count;
            type_info.alignment = it->second.alignment;
        }
        // 处理普通类型
        else {
            auto it = type_map.find(type_name);
            if (it == type_map.end()) {
                cerr << "未知类型: " << type_name << endl;
                exit(EXIT_FAILURE);
            }
            type_info = it->second;
        }

        // 计算填充字节（当前偏移按对齐要求对齐）
        size_t padding = (type_info.alignment - (current_offset % type_info.alignment)) % type_info.alignment;

        // 记录成员布局信息
        layout.members.push_back({
            current_offset + padding,
            type_info.size,
            type_info.alignment,
            padding
        });

        // 更新偏移和最大对齐值
        current_offset += padding + type_info.size;
        max_alignment = max(max_alignment, type_info.alignment);
    }

    // 计算结构体总大小（包含末尾填充）
    layout.alignment = max_alignment;
    size_t total_used = current_offset;
    size_t final_padding = (max_alignment - (total_used % max_alignment)) % max_alignment;

    layout.total_size = total_used + final_padding;
    layout.final_padding = final_padding;

    return layout;
}
}
void test_struct_align2() {
    using namespace struct1;
    // 测试数组类型
    {
        cout << "测试1：struct { char arr[6], int }\n";
        vector<string> members = {"char[6]", "int"};
        StructLayout layout = compute_struct_layout2(members);
        struct1::print_layout(layout);
    }

    {
        cout << "测试2：struct { double[3], char }\n";
        vector<string> members = {"double[3]", "char"};
        StructLayout layout = compute_struct_layout2(members);
        struct1::print_layout(layout);
    }

    {
        cout << "测试3：struct { short[5], int[2] }\n";
        vector<string> members = {"short[5]", "int[2]"};
        StructLayout layout = compute_struct_layout2(members);
        struct1::print_layout(layout);
    }
}

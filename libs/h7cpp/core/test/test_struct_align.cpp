#include "test_struct.h"

using namespace std;

namespace struct1 {

StructLayout compute_struct_layout(const vector<string>& member_types) {
    StructLayout layout;
    size_t current_offset = 0;
    size_t max_alignment = 0;

    for (const auto& type_name : member_types) {
        auto it = type_map.find(type_name);
        if (it == type_map.end()) {
            cerr << "错误：未知类型 '" << type_name << "'" << endl;
            exit(EXIT_FAILURE);
        }

        const TypeInfo& type = it->second;
        const size_t align = type.alignment;
        const size_t size = type.size;

        // 计算需要填充的字节数
        const size_t padding = (align - (current_offset % align)) % align;

        // 记录成员信息
        layout.members.push_back({
            current_offset + padding, // 计算实际偏移量
            size,
            align,
            padding
        });

        // 更新偏移量和最大对齐值
        current_offset += padding + size;
        max_alignment = max(max_alignment, align);
    }

    // 计算末尾填充
    layout.alignment = max_alignment;
    const size_t total_used = current_offset;
    const size_t final_padding = (max_alignment - (total_used % max_alignment)) % max_alignment;

    layout.total_size = total_used + final_padding;
    layout.final_padding = final_padding;

    return layout;
}

}
void test_struct_align() {
    using namespace struct1;
    // 示例1：char-int-char 结构体
    {
        cout << "示例1：struct { char, int, char }\n";
        vector<string> members = {"char", "int", "char"};
        StructLayout layout = compute_struct_layout(members);
        print_layout(layout);
    }

    // 示例2：double-char-int 结构体
    {
        cout << "示例2：struct { double, char, int }\n";
        vector<string> members = {"double", "char", "int"};
        StructLayout layout = compute_struct_layout(members);
        print_layout(layout);
    }

    // 示例3：包含指针类型
    {
        cout << "示例3：struct { void*, int, short }\n";
        vector<string> members = {"void*", "int", "short"};
        StructLayout layout = compute_struct_layout(members);
        print_layout(layout);
    }
}

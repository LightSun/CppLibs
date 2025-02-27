#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "utils/string_utils.hpp"

void main_test_regex() {
    std::string s = R"(frozenset(["肝脏-剑突下横切面",  "肾脏-右肾长轴切面", "肾脏-左肾长轴切面"]),
                    frozenset(["胆囊-胆囊长轴切面", "脾脏-脾脏长轴切面"])";

    // 第一步：提取中括号内的内容
    std::regex bracket_pattern(R"(\[(.*)\])");
    std::smatch bracket_match;
    if (!std::regex_search(s, bracket_match, bracket_pattern)){
        std::cout << "未找到中括号内容！" << std::endl;
        return;
    }
    {
        std::string s1 = bracket_match[0];
        printf("s1 = '%s'\n", s1.data());
        std::vector<std::string> vec;
        h7::utils::extractStr(R"(\[(.*)\])", s, vec);
        for (const auto& item : vec) {
            std::cout << "extractStr: '" << item << "'" << std::endl;
        }
    }
    std::string content = bracket_match[1];

    // 第二步：提取每个引号内的字符串
    std::regex item_pattern(R"(\"([^"]*)\")");
    std::sregex_iterator iter(content.begin(), content.end(), item_pattern);
    std::sregex_iterator end;

    std::vector<std::string> results;
    for (; iter != end; ++iter) {
        results.push_back((*iter)[1]);
    }

    // 输出结果
    std::cout << "提取到的内容：" << std::endl;
    for (const auto& item : results) {
        std::cout << "'" << item << "'" << std::endl;
    }
}

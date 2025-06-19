#include <iostream>

void FormatPrint()
{
    std::cout << std::endl;
}

template <class T, class ...Args>
void FormatPrint(T first, Args... args)
{
    std::cout << "[" << first << "]";
    FormatPrint(args...);
}

void test_template_var_args()
{
    FormatPrint(1, 2, 3, 4);
    FormatPrint("good", 2, "hello", 4, 110);
}

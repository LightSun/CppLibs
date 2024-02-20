
#include "utils/SortedList.h"
#include <string>

using namespace h7;

struct Student{
    int age;
    std::string name;
};

void test_SortedList(){
    SortedList<Student> list([](const Student& stu){
        return (uint32)stu.age;
    });
    {
    Student tu;
    tu.age = 10;
    list.add(tu, false);
    }
    for(int i = 0 ; i < 10 ; ++i){
        Student tu;
        tu.age = 200 - i;
        list.add(tu, false);
    }
    list.sort();
    for(int i = 0 ; i < (int)list.size(); ++i){
        printf("idx = %d, age = %d\n", i, list.get(i).age);
    }
    {
    Student tu;
    tu.age = 195;
    int idx = list.indexOf(tu);
    printf("age = %d, indexOf = %d\n", tu.age,  idx);
    }
}

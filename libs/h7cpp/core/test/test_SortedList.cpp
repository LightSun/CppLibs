
#include "utils/SortedList.h"
#include "utils/SortedList3.h"
#include <string>

using namespace h7;

struct Student{
    int age;
    std::string name;

    friend bool operator < (Student const &a, Student const &b){
        return a.age < b.age;
    }
    friend bool operator > (Student const &a, Student const &b){
        return a.age > b.age;
    }
    bool operator==(Student const& oth){
        return age == oth.age;
    }
};
//-----------------------

static void test_SortedList1();
static void test_SortedList3();

void test_SortedList(){
    test_SortedList3();
}

void test_SortedList3(){
    SortedList3<Student, std::greater<Student>> list(true);
    list.setMaxSize(6);
    {
    Student tu;
    tu.age = 10;
    list.add(tu);
    }
    for(int i = 0 ; i < 10 ; ++i){
        Student tu;
        tu.age = 200 - i;
        list.add(tu);
    }
    for(int i = 0 ; i < (int)list.size(); ++i){
        printf("idx = %d, age = %d\n", i, list.get(i).age);
    }
}

void test_SortedList1(){
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

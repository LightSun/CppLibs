#include <iostream>

//extern "C"{
#include "TH.h"
//}

using namespace std;

extern "C" int main()
{
    cout << "Hello World!" << endl;

    THIntStorage* ths_int = THIntStorage_new();
    THIntStorage_newWithMapping("a.db", 1024, 0);
    return 0;
}

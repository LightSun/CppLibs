
#include "core/common/common.h"
#include "core/utils/SuperConfig.h"

using namespace h7;


void test_SuperConfig(){
    String file;
#ifdef WIN32
    String dir = "E:/study/github/mine/CppLibs/libs/h7cpp/res/data";
    file = dir + "/test.prop";
#else
    String dir = "/home/heaven7/heaven7/study/github/mine/CppLibs/libs/h7cpp/res/data";
    file = "/media/heaven7/Elements_SE/study/work/MedQA/V2/test/config/main.lp";
#endif
    Map map = {
        {"CUR_DIR", dir}};
    SuperConfig sc(&map);
    MED_ASSERT(sc.loadFromFile(file));
    sc.dump();
}

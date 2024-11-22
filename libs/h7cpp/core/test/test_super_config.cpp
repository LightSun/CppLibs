
#include "core/common/common.h"
#include "core/utils/SuperConfig.h"

using namespace h7;


void test_SuperConfig(){
    String dir = "/media/heaven7/Elements_SE/study/work/HxPoc/SConfig";
    String file = dir + "/test.prop";
    Map map = {
        {"CUR_DIR", dir}};
    SuperConfig sc(&map);
    MED_ASSERT(sc.loadFromFile(file));
    sc.dump();
}

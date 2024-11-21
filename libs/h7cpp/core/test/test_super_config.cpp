
#include "core/common/common.h"
#include "core/utils/SuperConfig.h"

using namespace h7;


void test_SuperConfig(){
    CString file = "/media/heaven7/Elements_SE/study/work/HxPoc/SConfig/test.prop";
    SuperConfig sc;
    MED_ASSERT(sc.loadFromFile(file));
    sc.dump();
}

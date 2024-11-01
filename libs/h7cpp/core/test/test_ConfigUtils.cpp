#include "utils/ConfigUtils.h"
#include "common/common.h"

using namespace h7;

using Map = std::map<String, String>;

void test_ConfigUtils(){
    String buf = "a=1\nb=2\r\nc=3\n";
    Map _map;
    h7::ConfigUtils::loadPropertiesFromBuffer(buf, _map);
    MED_ASSERT(_map.size()==3);
    MED_ASSERT(_map["a"]=="1");
    MED_ASSERT(_map["b"]=="2");
    MED_ASSERT(_map["c"]=="3");
}

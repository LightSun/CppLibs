
#if 0
#include "FilesSync.hpp"

using namespace h7;

void test_FilesSync(){
    remove("test.dt");
    FilesSync sync("test.dt");
    sync.addItem({"c:/a", "a", "ud"});
    sync.addItem({"c:/b", "b", "ud"});
    sync.addItem({"c:/c", "c", "ud"});

    bool ret;
    ret = sync.removeItem("a");
    MED_ASSERT(ret);
    ret = sync.removeItem("d");
    MED_ASSERT(!ret);
    sync.close();
    //------------
    FilesSync sync2("test.dt");
    auto& items = sync2.getItems();
    MED_ASSERT(items.size() == 2);
    ret = items[0].file_enc == "c:/b";
    MED_ASSERT(ret);
    ret = items[1].file_enc == "c:/c";
    MED_ASSERT(ret);

    ret = items[0].hash == "b";
    MED_ASSERT(ret);
    ret = items[1].hash == "c";
    MED_ASSERT(ret);
}

#endif

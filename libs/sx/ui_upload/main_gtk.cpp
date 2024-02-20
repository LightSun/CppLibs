#include "pub_api.h"
#include <thread>
#include "CountDownLatch.h"

//qt windows:
//1. set sys env QT_QPA_PLATFORM_PLUGIN_PATH=xxx/plugins\platforms
//2. copy bin/Qt5Core.dll, Qt5Gui.dll, Qt5Widgets.dll to target path. or set to %PATH%

//static h7::CountDownLatch cdl(1);

void test_gtk(med_api::ShowUploadHelper* helper){
    std::thread thd([helper](){
        med_api::qt_handler_post([helper](){
            med_api::SelectFileContext ctx;
            ctx.multi = true;
           // ctx.fmts = {"*.sh", "*.zip"};
           // med_api::show_upload_dlg_gtk(&ctx);
            helper->showDlg(&ctx);
            for(auto& path: ctx.files){
                printf("path: %s\n", path.data());
            }
        });
    });
    thd.join();
}

int main(){

//    med_api::SelectFileContext ctx;
//    ctx.multi = true;
//    med_api::ShowUploadHelper helper;
//    helper.init();
//    helper.showDlg(&ctx);
//    for(auto& path: ctx.files){
//        printf("path: %s\n", path.data());
//    }

    med_api::ShowUploadHelper* helper = new med_api::ShowUploadHelper();
    helper->init();
    test_gtk(helper);
    return helper->exec();
}

#include <stdio.h>
#include <string>
#include <locale>
#include "utils/FileUtils.h"
#include "utils/string_utils.hpp"
#include "utils/url_encode.h"
#include "utils/system.h"
#include "utils/Properties.hpp"
#include "utils/ConfigUtils.h"

//#include "ui_upload/pub_api.h"
#include "common/logger.h"

extern int test_main0(int argc, char* argv[]);

static void test_url_encode();
static void test_select_file();
static void test_logger();

extern void test_FilesSync();
extern void test_CountDownLatch();
extern void test_Barrier();
extern void test_SortedList();
extern void test_XBacktrace();
extern void test_ConfigUtils();

extern void test_Platforms();
extern void test_splits();
extern void test_Callbacks();
extern void test_LockFreeHashMap();
extern void test_SaveQueue();
extern void test_SFINAE();
extern void test_IOs();
extern void test_dir_files_md5(const char* dir);
extern void test_SuperConfig();

using namespace h7;

int main(int argc, char* argv[]){
    setbuf(stdout, NULL);
    /* //check if p2 = p1, p is changed or not.
    int a = 1;
    int b = 1;

    int* p = &a;
    int* p1 = &b;
    int* p2 = p;
    p2 = p1;
    printf("p = %p, p1 = %p, p2 = %p\n", p, p1, p2);
    */

//    if(argc == 1){
//        //String fir = "/home/heaven7/heaven7/work/TensorRT/libtorch_1.12.1/libtorch/lib";
//       // test_dir_files_md5(fir.data());
//        test_SuperConfig();
//        return 0;
//    }

    if(argc > 1){
        //auto ret = h7::FileUtils::isFileExists(argv[1]);
       // printf("file exists: %s\n", ret ? "true" : "false");
    }
    //test_splits();
    //test_CountDownLatch();
    //test_Barrier();
    test_SortedList();

    //test_XBacktrace();
    //test_Callbacks();

    //test_LockFreeHashMap();
    //test_SaveQueue();
    //test_SFINAE();

    //test_ConfigUtils();
    //test_Platforms();

    //test_url_encode(); //ok
    //std::string str = "我爱祖国";
    //PRINTLN("str.len = %d\n", str.length());//linux 12
    //test_select_file();

//    String fileX = "C%3A%2FUsers%2Fmed%2FDesktop%2F%E4%B8%AD%E6%96%87%2F%E6%96%B0%E5%BB%BA%20DOC%20%E6%96%87%E6%A1%A3.doc";
//    auto _data = url_decode2(fileX);
//    auto file = str_to_GBEX(_data);
//    printf("url decode: %s\n", file.data());
//    auto hash = FileUtils::sha256(file);
//    printf("hash: %s\n", hash.data());
    //test_logger();

//    String dir = "/media/heaven7/Elements/big_files/";
//    String files[] = {
//        dir + "Sample202011.R1.fastq.gz",
//        dir + "Sample202011.R2.fastq.gz",
//        dir + "Sample202011.R1_1.fastq.gz",
//        dir + "Sample202011.R1_2.fastq.gz",
//        dir + "Sample202011.R1_3.fastq.gz",
//        dir + "Sample202011.R1_4.fastq.gz",
//    };
//    int len = 6;
//    for(int i = 0 ; i < len ; ++i){
//        auto hash = FileUtils::sha256(files[i]);
//        printf("file: %s\n", files[i].data());
//        printf("hash: %s\n", hash.data());
//    }

   // test_FilesSync();
//    String file = "/media/heaven7/Elements/win_pkg/省心/202011.HC.filter.vcf.zip";
//    String hash = FileUtils::sha256(file);
//    printf("file = %s, hash = %s\n", file.data(), hash.data());

    //
//    String file2 = "/home/heaven7/heaven7/study/github/mine/"
//                   "build-study_ITK-Desktop_Qt_5_14_2_GCC_64bit-Debug/"
//                   "test/test1.prop";
//    Properties prop;
//    ConfigUtils::loadProperties(file2, prop.m_map);
//    ConfigUtils::resolveProperties(prop.m_map);
//    printf("thy::thread_count = %d\n", prop.getInt("thy::thread_count"));
   // return test_main0(argc, argv);
    return 0;
}
//void test_select_file(){
//    med_api::ShowUploadHelper helper;
//    helper.init();
//    med_api::SelectFileContext ctx;
//    {
//        bool single = true;
//        String fmt = "*.*";
//        ctx.multi = !single;
//        ctx.fmts = h7::utils::split(",", fmt);
//    }
//    helper.showGtkDlg(&ctx);
//    //std::locale::global(std::locale(""));
//    if( ctx.files.size() > 0){
//        String _file = ctx.files[0];

//        String __file = utils::replace2("\\\\", "/", _file);
//        __file = utils::replace2("\\", "/", __file);
//        __file = url_encode2(__file);

//        auto file = str_to_GBEX(__file);
//        auto hash = FileUtils::sha256(file);
//        printf("file: %s\n", file.data());
//        printf("hash: %s\n", hash.data());
//        printf("url encode: %s\n", __file.data());

//        auto _data = url_decode2(__file);
//        printf("url decode: %s\n", _data.data());
//    }
//    //::locale::global(std::locale("C"));
//}
void test_url_encode(){
    String str = "C:\\a\\b\\c.txt";
    printf("raw ret: %s\n", str.data());
    char* ret = url_encode(str.data());
    String ret_str = ret;
    String ret_str2(ret);
    printf("enc ret: %s\n", ret_str.data());
    printf("enc ret2: %s\n", ret_str2.data());

    char* ret2 = url_decode(ret);
    printf("dec ret: %s\n", ret2);
    free(ret);
    free(ret2);
}

void test_logger(){
    h7_logd("test_logger!\n");
    h7_logd("test_logger 2!\n");
}

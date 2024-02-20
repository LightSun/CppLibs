#include <stdio.h>
#include <string>
#include <locale>
#include "utils/FileUtils.h"
#include "utils/string_utils.hpp"
#include "utils/url_encode.h"
#include "utils/system.h"
#include "utils/CmdHelper.h"
#include "utils/PerformanceHelper.h"

#include "ui_upload/pub_api.h"
#include "service/firewall.h"
#include "service/service_base.h"

extern int test_main0(int argc, char* argv[]);
static void test_url_encode();
static void test_select_file();
static void test_shell();
static void test_sha256_perf();

extern void test_shareMemory();
extern int _tmain(int argc, char* argv[]);

static bool createDir(String& path);


using namespace h7;

int main(int argc, char* argv[]){
    setbuf(stdout, NULL);
    String _dir = "C:\\WINDOWS\\system32\\config\\systemprofile\\tDesktop";
    createDir(_dir);

    //test_sha256_perf();
    //_tmain(argc, argv);
    //test_shareMemory();
   // test_select_file();

//    if(argc > 1){
//        auto ret = h7::FileUtils::isFileExists(argv[1]);
//        printf("file exists: %s\n", ret ? "true" : "false");
//    }
    //test_url_encode(); //ok
    //std::string str = "我爱祖国";
    //PRINTLN("str.len = %d\n", str.length());//linux 12
 //   test_select_file();
    //String fileX = url_encode2("D:/big_files/opencv-4.5.4 - 副本.zip");
   // printf("url encode: %s\n", fileX.data());

    //String fileX = "C%3A%2FUsers%2Fmed%2FDesktop%2F%E4%B8%AD%E6%96%87%2F%E6%96%B0%E5%BB%BA%20DOC%20%E6%96%87%E6%A1%A3.doc";
//    auto _data = url_decode2(fileX);
//    auto file = str_to_GBEX(_data);
//    printf("url decode: %s\n", file.data());
//    auto hash = FileUtils::sha256(file);
//    printf("hash: %s\n", hash.data());

//    file = "D:/win_pkg/test.out";
//    String content = FileUtils::getFileContent(file);
//    int data_size = *(int*)content.data();
//    int reqType = *(int*)(content.data() + sizeof (int));
//    printf("data_size = %d, reqType = %d\n", data_size, reqType);

   // return test_main0(argc, argv);
    return 0;
}

void test_sha256_perf(){
    //String file = "C:/heaven7/sx/fq/HG002.novaseq.pcr-free.30x.R1.fastq.gz";
    String file = "D:/shengxin/fq/HG002.novaseq.pcr-free.30x.R1.fastq.gz";
    int every = 20 << 20;
    String hash;
    PerfHelper ph;
    ph.begin();
    //in c: un-prehash 26.93s. prehash ->blk 30s
    //in ext sd: un-prehash 6.83m. prehash ->blk 6.83 min
    hash = FileUtils::sha256_3(file);
    ph.print("sha256_3");

//    ph.begin();
//    hash = FileUtils::sha256_3(file);
//    ph.print("sha256_3");
//    for(int i = 0 ; i < 10 ; i ++){
//         int idx = i * 4 + 1;
//         auto tag = "sha256_3_" + std::to_string(idx);
//         uint64 block_size = every + i * 4 * every;
//         ph.begin();
//         hash = FileUtils::sha256_3(file, NULL, block_size);
//         ph.print(tag);
//    }
    //FileUtils::sha256_2(file, )
}

void test_shell(){
    //no ok. encode problem
    String exe = "C:/heaven7/sx/build-SxUploadCore-Mingw_64-Release/SxUploadWinService.exe";
    String name = "SxUploadWinService";
    auto n1 = s2ws(name);
    auto exe1 = s2ws(exe);
    h7::WriteFireWall((char*)n1.c_str(), (char*)exe1.data(), false);
}

void test_select_file(){
    med_api::ShowUploadHelper helper;
    helper.init();
    med_api::SelectFileContext ctx;
    {
        bool single = true;
        String fmt = "*.*";
        ctx.multi = !single;
        ctx.fmts = h7::utils::split(",", fmt);
    }
    helper.showGtkDlg(&ctx);
    printf("select file size: %d\n", ctx.files.size());
    //std::locale::global(std::locale(""));
    printf("select file size: %d\n", (int)ctx.files.size());
    if( ctx.files.size() > 0){
        String _file = ctx.files[0];

        String __file = utils::replace2("\\\\", "/", _file);
        __file = utils::replace2("\\", "/", __file);
        __file = url_encode2(__file);

        auto file = str_to_GBEX(__file);
        auto hash = FileUtils::sha256(file);
        printf("file: %s\n", file.data());
        printf("hash: %s\n", hash.data());
        printf("url encode: %s\n", __file.data());

        auto _data = url_decode2(__file);
        printf("url decode: %s\n", _data.data());
    }
    //::locale::global(std::locale("C"));
}
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

#ifdef _WIN32
#include <dbghelp.h>
bool createDir(String& path){
    char last_char;
    last_char = path.back();
    if (last_char != '\\')
    {
       path += '\\';
    }
    PCSTR pcPath = path.c_str();
    bool result = MakeSureDirectoryPathExists(pcPath);
    return result;
}
#endif

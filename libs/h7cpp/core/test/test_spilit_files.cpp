#include "test_spilit_files.h"
#include "utils/FileUtils.h"
#include "utils/FileIO.h"
#include "utils/string_utils.hpp"

using namespace h7;

#define SUFFIX "_PART"
#define _LOG(t) std::cout << t << std::endl

void FileHelper::split(CString file, CString target_dir, uint64 limit_size){
    String dst_dir = target_dir;
    if(dst_dir.empty()){
        dst_dir = FileUtils::getFileDir(file);
    }else{
        FileUtils::mkdirs(target_dir);
    }
    auto fn = FileUtils::getSimpleFileName(file);
    auto total_size = FileUtils::getFileSize(file);
    uint32 c = total_size / limit_size + ((total_size % limit_size) != 0 ? 1 : 0);
    _LOG("total_size = " << total_size);
    _LOG("c = " << c);
    _LOG("limit_size = " << limit_size);
    for(uint32 i = 0 ; i < c ; ++i){
        uint64 offset = i * limit_size;
        auto dst_file = dst_dir + "/" + fn + SUFFIX + std::to_string(i);
        FileOutput fos(dst_file);
        MED_ASSERT(fos.is_open());
        auto buf = FileUtils::getFileContent(file, offset, limit_size);
        fos.write(buf.data(), buf.length());
        fos.flush();
        fos.close();
    }
}
int FileHelper::merge(CString file){
    FileOutput fos(file);
    auto fn = FileUtils::getFileName(file);
    int c = 0;
    for(int i = 0 ; ; i++){
        auto src_file = file + SUFFIX + std::to_string(i);
        if(!FileUtils::isFileExists(src_file)){
            break;
        }
        c ++;
        auto buf = FileUtils::getFileContent(src_file);
        fos.write(buf.data(), buf.length());
    }
    fos.flush();
    fos.close();
    return c;
}
//99M
#define _LIMIT_SIZE 90 << 20

static void test1(){
    String dir = "/home/heaven7/heaven7/study/github/mine/git_big_files/models/sam/";
    String file = dir + "sam_vit_h_4b8939_onnx.zip";
    String out_dir = dir + "out_dir";
   // FileHelper::split(file, out_dir, _LIMIT_SIZE);
    //FileHelper::merge(out_dir + "/sam_vit_h_4b8939_onnx.zip");
}
static void test2(){
    String dir = "/home/heaven7/heaven7/study/github/mine/git_big_files/rtc/";
    String file = dir + "webrtc_srs.zip";
    String out_dir = dir + "out_dir";
    FileHelper::split(file, out_dir, _LIMIT_SIZE);
}

void test_splits(){
    test2();
}



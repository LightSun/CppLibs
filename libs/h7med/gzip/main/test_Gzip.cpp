#include "gzip/src/Gzip.h"

using namespace h7_gz;

void test_Gzip1(){
    String in_dir = "/media/heaven7/Elements_SE/temp/test1";
    String out_dir = "/media/heaven7/Elements_SE/temp/test1_out";
    String out_file = "/media/heaven7/Elements_SE/temp/test1.med";
    GzipHelper gh;
    {
        auto ret = gh.compressDir(in_dir, out_file);
        String str = ret ? "Success" : "Failed";
        printf("compressFile: %s, in_dir = '%s', outF = '%s'\n",
               str.data(), in_dir.data(), out_file.data());
        if(!ret){
            return;
        }
    }
    //
    auto decRet = gh.decompressFile(out_file, out_dir);
    String decStr = decRet ? "Success" : "Failed";
    printf("decompressFile: %s,in_dir = '%s', outF = '%s'\n",
           decStr.data(), in_dir.data(), out_file.data());
}


#include "zip/src/ZipHelper.h"

using namespace h7;

static void test_unzip();

int main(int argc, const char* argv[]){

    setbuf(stdout, NULL);
    test_unzip();
    return 0;
}

//zip can't used for big files.
void test_unzip(){
    String f1 = "/media/heaven7/Elements_SE/study/work/huawei/local_src/HsyVideoPro.zip";
//    String f2 = "/media/heaven7/Elements_SE/shengxin/vep_cache/homo_sapiens_refseq/"
//                "test.zip";
    String outD = "/media/heaven7/Elements_SE/study/work/huawei/local_src/test_out";
    UnZipImpl unzip;
    bool openStat = true;
    bool unzipStat = true;
    if((openStat = unzip.openZip(f1))){
        unzipStat = unzip.unzip2(outD);
    }
    printf("openStat = %d, unzipStat = %d\n", openStat, unzipStat);
}


#include "gzip/src/Gzip.h"
#include "gzip/src/ZlibUtils.h"
#include "core/src/common.h"

using namespace h7_gz;

static void test1();
static void test2();
extern void test_Gzip1();

int main(int argc, const char* argv[]){
    setbuf(stdout, NULL);
    //test1();
    test_Gzip1();
    return 0;
}

void test1(){
    //
    String buf  = "01010101010101010101010000000000000000000000000000011111111111111"
                 "01010101010101010101010000000000000000000000000000011111111111111"
                 "01010101010101010101010000000000000000000000000000011111111111111"
                 "01010101010101010101010000000000000000000000000000011111111111111"
                 "01010101010101010101010000000000000000000000000000011111111111111"
                 "01010101010101010101010000000000000000000000000000011111111111111"
                 "01010101010101010101010000000000000000000000000000011111111111111"
                 "01010101010101010101010000000000000000000000000000011111111111111"
                 "01010101010101010101010000000000000000000000000000011111111111111"
                 "01010101010101010101010000000000000000000000000000011111111111111"
                 "qwertyuiop[]";

    std::cout << "========= CompressString ===========" << std::endl;
    std::cout << "Source Buffer Size: " << buf.length() << std::endl;
    std::string out_compress;
    MED_ASSERT(ZlibUtils::compress(buf, out_compress));
    std::cout << "Compress Buffer Size: " << out_compress.size() << std::endl;

    std::cout << "========= DecompressString ===========" << std::endl;
    std::string out_decompress;
    MED_ASSERT(ZlibUtils::decompress(out_compress, out_decompress));
    MED_ASSERT(buf == out_decompress);
}

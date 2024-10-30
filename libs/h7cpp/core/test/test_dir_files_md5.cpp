#include "common/common.h"
#include "utils/FileUtils.h"
#include "table/HashMap.h"

using namespace h7;

void test_dir_files_md5(const char* _dir){
    std::vector<String> files;
    {
        String dir(_dir);
        files = FileUtils::getFiles(dir);
    }

    HashMap<String, std::vector<String>> hmap;
    for(auto& f : files){
        auto dir = FileUtils::getFileDir(f);
        hmap[dir].push_back(f);
    }
    auto it = hmap.begin();
    for(; it != hmap.end(); ++it){
        auto& dir = it->first;
        printf("dir_sha256 >> dir = %s\n", dir.data());
        for(auto& f : it->second){
            auto sha256 = FileUtils::sha256(f);
            printf("sha256 >> file, sha256 = \n%s\n     %s\n", f.data(), sha256.data());
        }
    }
}

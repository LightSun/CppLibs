#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#endif

#ifdef __APPLE__
#include <sys/uio.h>
#elif _WIN32
#include <io.h>
#elif __linux__
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
//#include <sys/stat.h>
#else
#include <sys/io.h>
#endif

#ifdef WIN32
#include <windows.h>                 //Windows API   FindFirstFile
#include <shlwapi.h>
#endif

#include <list>
#include "utils/FileUtils.h"
#include "utils/string_utils.hpp"
#include "utils/hash.h"
#include "utils/h_atomic.h"
#include "openssl/sha.h"
#include "utils/FileIO.h"

#include "blake3.h"

//windows: https://blog.csdn.net/siyacaodeai/article/details/112732678
//linux: https://www.jianshu.com/p/7f79d496b0e2
namespace h7 {

bool FileUtils::isFileExists(const std::string &path) {
#ifdef _WIN32
    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile(path.data(), &wfd);
    if ( INVALID_HANDLE_VALUE != hFind )  return true;
    else                                  return false;
#else
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
#endif // !_WIN32
}

sk_sp<ListS> FileUtils::getFiles(CString path, bool recursion, CString suffix){
    sk_sp<ListS> ret = sk_make_sp<ListS>();
#ifdef __linux__
    DIR *dir;
#endif
#ifdef _WIN32
    long hFile = 0;
    struct _finddata_t fileinfo;
    std::string p;
#endif
    struct dirent *ptr;
    std::list<String> folders;
    folders.push_back(path);

    String str;
    while(folders.size() > 0) {
      str = folders.front();
      folders.pop_front();
#ifdef __linux__
      if ((dir = opendir(str.c_str())) != NULL) {
          while ((ptr = readdir(dir)) != NULL) {
              if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
                  continue;
              } else if (ptr->d_type == DT_REG) { /* regular file */
                  if(suffix.length() > 0){
                      String _file(ptr->d_name);
                      if(h7::utils::endsWith(_file, suffix)){
                          ret->add(str + std::string("/") + _file);
                      }
                  }
              } else if (ptr->d_type == DT_DIR) { /* dir */
                  if(recursion){
                      folders.push_back(str + std::string("/") + ptr->d_name);
                  }
              }
          }
      }
      closedir(dir);
#endif

#ifdef _WIN32
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do{
            if ((fileinfo.attrib & _A_SUBDIR)){
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                {
                    if(recursion){
                        folders.push_back(p.assign(path).append("\\").append(fileinfo.name));
                    }
                }
            }
            else{
                String _file(fileinfo.name);
                if(h7::utils::endsWith(_file, suffix)){
                    ret->add(p.assign(path).append("\\").append(fileinfo.name));
                }
            }
        }while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
#endif
    }
    return ret;
}

#ifdef _WIN32
static char ALPHABET[] = {
    'A','B','C','D','E','F','G',
    'H','I','J','K','L','M','N',
    'O','P','Q','R','S','T',
    'U','V','W','X','Y','Z'
};
#endif

bool FileUtils::isRelativePath(CString path){
    auto strs = path.c_str();
    if(strs[0] == '/'){
        return false;
    }
#ifdef _WIN32
    if(strs[1]==':'){
        for(int i = 0 ; i < 26 ; i ++){
            if(ALPHABET[i] == strs[0]){
                return false;
            }
        }
    }
#endif
    return true;
}

String FileUtils::getFilePath(CString path, const std::vector<String>& dirs){
    if(!isRelativePath(path)){
       return path;
    }
    if(isFileExists(path)){
        return path;
    }
    String file;
    auto end = dirs.end();
    for(auto it = dirs.begin() ; it != end ; it++){
         file = *it + "/" + path;
         if(isFileExists(file)){
             return file;
         }
    }
    return "";
}

int FileUtils::getFileLineCount(CString file){
    FILE *fp;
    if((fp = fopen(file.c_str(), "r")) == NULL){
        return -1;
    }
    int flag;
    int count = 0;
    while(!feof(fp))
    {
        flag = fgetc(fp);
        if(flag == '\n'){
           count++;
        }
    }
    fclose(fp);
    return count + 1; //add last line
}

void FileUtils::mkdir(CString path){
#ifdef _WIN32
  _mkdir(path.c_str());
#else
  ::mkdir(path.c_str(), 0777);
#endif // !_WIN32
}

//ifstream
std::string FileUtils::getFileContent(CString file){
    FileInput fin(file);
    MED_ASSERT_X(fin.is_open(), "open file failed: " + file);
    std::vector<char> buf(fin.getLength());
    fin.reset();
    fin.read(buf.data(), buf.size());
    fin.close();
    return std::string(buf.data(), buf.size());
}

std::string FileUtils::getFileContent(CString file, uint64 offset, uint64 size){
    FileInput fin(file);
    MED_ASSERT_X(fin.is_open(), "open file failed: " + file);
    uint64 left_size = fin.getLength() - offset;
    MED_ASSERT(left_size > 0);
    //
    std::vector<char> buf(HMIN(left_size, size));
    fin.seek(offset, false);
    fin.read(buf.data(), buf.size());
    fin.close();
    return std::string(buf.data(), buf.size());
}

unsigned int FileUtils::hash(CString file){
    FileInput fin(file);
    MED_ASSERT_X(fin.is_open(), "open file failed: " + file);
    std::vector<char> buf(fin.getLength());
    fin.reset();
    fin.read(buf.data(), buf.size());
    fin.close();
    return fasthash32(buf.data(), buf.size(), 11);
}

//#define BLOCK_SIZE (100 << 20) //100M
std::string FileUtils::sha256(CString file, uint64* out_len,
                              volatile int* cancel){
    FileInput fis(file);
    if(!fis.is_open()){
        return "";
    }
   unsigned char md[SHA256_DIGEST_LENGTH];
   memset(md, 0, SHA256_DIGEST_LENGTH);
   //
   uint64 total_size = fis.getLength();
   fis.reset();
   if(out_len){
       *out_len = total_size;
   }
   const int c = 20;
   uint64 every = total_size / c;
   uint64 lastSize = total_size % c;
   int count;
   if(every == 0){
       count = 1;
       every = total_size;
   }else{
       count = lastSize != 0 ? (c + 1) : c;
   }
   //
   uint64 offset = 0;
   std::vector<char> buf(every);
   if(lastSize == 0){
       lastSize = every;
   }
   //do sha256
   {
       SHA256_CTX _ctx;

       SHA256_Init(&_ctx);
       for(int i = 0 ; i < count ; ++ i){
           //printf("openssl: i = %d\n", i);
           uint64 size = i != count-1 ? every : lastSize;
           fis.seek(offset);
           fis.read(&buf[0], size);
           uint64 hash = fasthash64(buf.data(), size, 11);
           SHA256_Update(&_ctx, &hash, sizeof(uint64));
           offset += every;
           if(cancel && h_atomic_get(cancel) != 0){
               fprintf(stderr, "cancel compute hash. file: %s\n",file.data());
               return "";
           }
       }
       SHA256_Final(md, &_ctx);
       fis.close();
   }

   //md[SHA256_DIGEST_LENGTH] = '\0';
    std::string fstr = "";
    char fbuf[3];
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
      sprintf(fbuf, "%02x", md[i]);
      fstr += String(fbuf, 2);
    }
    return fstr;
}

std::string FileUtils::sha256_2(CString file, uint64* out_len, uint64 block_size,
                              volatile int* cancel){
    FileInput fis(file);
    if(!fis.is_open()){
        return "";
    }
    ///*
    unsigned char md[SHA256_DIGEST_LENGTH];
    memset(md, 0, SHA256_DIGEST_LENGTH);
    {
    const uint64 total_size = fis.getLength();
    fis.reset();
    if(out_len){
        *out_len = total_size;
    }
    uint64 everySize = block_size;
    uint64 lastSize = total_size % block_size;
    const int count = lastSize != 0 ?(total_size / block_size) + 1
                             : (total_size / block_size);
    //
    std::vector<char> buf(everySize);
    if(lastSize == 0){
        lastSize = everySize;
    }
    //do sha256
    SHA256_CTX _ctx;
    SHA256_Init(&_ctx);
    uint64 offset = 0;
    for(int i = 0 ; i < count ; ++ i){
        //printf("openssl: i = %d\n", i);
        uint64 size = i != count-1 ? everySize : lastSize;
        fis.seek(offset);
        fis.read(&buf[0], size);
        uint64 hash;
        if(size < everySize){
            //populate others. '\b'
            memset(buf.data() + size, '\b', everySize - size);
        }
        hash = fasthash64(buf.data(), everySize, 11);
        SHA256_Update(&_ctx, &hash, sizeof(uint64));
        offset += everySize;
    }
    SHA256_Final(md, &_ctx);
    fis.close();
    }

    //md[SHA256_DIGEST_LENGTH] = '\0';
    std::string fstr = "";
    char fbuf[3];
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(fbuf, "%02x", md[i]);
        fstr += String(fbuf, 2);
    }
    return fstr;
    //*/
}

std::string FileUtils::sha256_3(CString file, uint64* out_len,uint64 block_size,
                            volatile int* cancel){
    FileInput fis(file);
    if(!fis.is_open()){
        return "";
    }

    const uint64 total_size = fis.getLength();
    fis.reset();
    if(out_len){
        *out_len = total_size;
    }
    uint64 everySize = block_size;
    uint64 lastSize = total_size % block_size;
    const int count = lastSize != 0 ?(total_size / block_size) + 1
                             : (total_size / block_size);
    //
    std::vector<char> buf(everySize);
    if(lastSize == 0){
        lastSize = everySize;
    }
    //
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);

    uint64 offset = 0;
    for(int i = 0 ; i < count ; ++ i){
        //printf("openssl: i = %d\n", i);
        uint64 size = i != count-1 ? everySize : lastSize;
        fis.seek(offset);
        fis.read(&buf[0], size);

//        uint64 hash;
//        hash = fasthash64(buf.data(), size, 11);
//        blake3_hasher_update(&hasher, &hash, sizeof(uint64));
        blake3_hasher_update(&hasher, buf.data(), size);
        offset += size;
    }

    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    std::string fstr = "";
    char fbuf[3];
    for(int i = 0; i < BLAKE3_OUT_LEN; i++)
    {
        sprintf(fbuf, "%02x", output[i]);
        fstr += String(fbuf, 2);
    }
    return fstr;
}

}



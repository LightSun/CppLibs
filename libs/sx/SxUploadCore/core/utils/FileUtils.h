#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <stdio.h>
#include <cstdio>
#include <initializer_list>
#include "table/Column.h"
#include "common/c_common.h"

namespace h7 {
    class FileUtils{
    public:
        static bool isFileExists(CString path);

        static inline bool deleteFile(CString file){
            return remove(file.c_str()) == 0;
        }
        static bool isRelativePath(CString path);
        static String getFilePath(CString path, const std::vector<String>& dirs);

        static sk_sp<ListS> getFiles(CString path, bool recursion = true,
                                     CString suffix = "");

        static inline String getFileDir(CString file){
            int pos;
#if defined(_WIN32) || defined(WIN32)
            pos = file.rfind("/");
            if(pos < 0){
                pos = file.rfind("\\");
            }
#else
            pos = file.rfind("/");
#endif
            return file.substr(0, pos);
        }
        static inline String getFileName(CString file){
            int index = file.rfind("/");
            if(index < 0){
                index = file.rfind("\\");
            }
            int dot_i = file.rfind(".");
            if(dot_i < 0){
                return file.substr(index + 1);
            }
            int len = dot_i - index - 1;
            MED_ASSERT(len > 0);
            return file.substr(index + 1, len);
        }

        static inline String getSimpleFileName(CString file){
            int index = file.rfind("/");
            if(index < 0){
                index = file.rfind("\\");
            }
            MED_ASSERT(index >= 0);
            return file.substr(index + 1);
        }

        static int getFileLineCount(CString file);
        static void mkdir(CString path);

        static unsigned int hash(CString file);
        //cancel give a chance to cancel it.
        static std::string sha256(CString file, uint64* out_len = NULL,
                                  volatile int* cancel = nullptr);
        static std::string sha256_2(CString file, uint64* out_len = NULL,
                                    uint64 block_size = (100 << 20),
                                    volatile int* cancel = nullptr);

        static std::string sha256_3(CString file, uint64* out_len = NULL,
                                    uint64 block_size = (100 << 20),
                                    volatile int* cancel = nullptr);
        static std::string getFileContent(CString file);
        static std::string getFileContent(CString file, uint64 offset, uint64 size);
    };
}

#endif // FILEUTILS_H

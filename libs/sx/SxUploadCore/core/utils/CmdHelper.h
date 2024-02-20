#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#include <shellapi.h>

namespace h7 {

class CmdHelper{

public:
    static int execCmd(const char* cmd, std::string& result){
        result.clear();
        FILE* pipe = popen(cmd, "r");
        if(!pipe){
            fprintf(stderr, "popen error\n");
            return -1;
        }
#define bufferLen 1024
        size_t ret = 0;
        char buf[bufferLen+1] = {0};
        while ((ret = fread(buf, sizeof(char),bufferLen, pipe)) == bufferLen){
            result.append(buf);
        }
        if(ferror(pipe) != 0) {
            fprintf(stderr, "read pipe error\n");
            return -1;
        }
        if(feof(pipe) != 0) {
            result.append(buf, ret);
        }
        return 0;
    }
// not ok
//    static int execCmd2(const char* cmd,const char* params){
//        //runas need cmd.
//        if(ShellExecuteA(NULL, (LPCTSTR)"runas", (LPCTSTR)cmd, (LPCTSTR)params, NULL, SW_NORMAL) <= (HINSTANCE)32){
//            return GetLastError();
//        }
//        return 0;
//    }
};
}


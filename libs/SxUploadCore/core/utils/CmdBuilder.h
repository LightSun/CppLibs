#ifndef CMDBUILDER_H
#define CMDBUILDER_H

#include <string.h>
#include <stdio.h>
#include "common/common.h"
#include "utils/Regex.h"
#include "table/Column.h"

namespace h7 {
    class CmdBuilder{
    public:
        inline CmdBuilder& str(CString str){
            mCmds.push_back(str);
            return *this;
        }
        inline CmdBuilder& strs(CString str){
            h7::Regex reg(" ");
            auto ss = reg.split(str);
            mCmds.insert(mCmds.end(), ss->list.begin(), ss->list.end());
            //return strs(h7::utils::split(" ", str));
            return *this;
        }
        inline CmdBuilder& strs(std::vector<String>& oth){
            mCmds.insert(mCmds.end(), oth.begin(), oth.end());
            return *this;
        }
        inline CmdBuilder& strs(const std::vector<String>& oth){
            mCmds.insert(mCmds.end(), oth.begin(), oth.end());
            return *this;
        }
        template <typename ...Args>
        inline CmdBuilder& strFmt(const char* fmt, Args&& ... args){
            HFMT_BUF_256({
                         mCmds.push_back(buf);
                         }, fmt, std::forward<Args>(args)...);
            return *this;
        }
        template <typename ...Args>
        inline CmdBuilder& strFmtN(int bufLen, const char* fmt, Args&& ... args){
            MED_ASSERT(bufLen >= (int)strlen(fmt) + 1);
            char buf[bufLen];
            snprintf(buf, bufLen, fmt, std::forward<Args>(args)...);
            return str(buf);
        }
        template <typename ...Args>
        inline CmdBuilder& strsFmt(const char* fmt, Args&& ... args){
            HFMT_BUF_256({
                         strs(buf);
                         }, fmt, std::forward<Args>(args)...);
            return *this;
        }
        template <typename ...Args>
        inline CmdBuilder& strsFmt2(const std::string& fmt, Args&& ... args){
            HFMT_BUF_512({
                             strs(buf);
                         }, fmt.c_str(), std::forward<Args>(args)...);
            return *this;
        }
        template <typename ...Args>
        inline CmdBuilder& strsFmtN(int bufLen,const std::string& fmt, Args&& ... args){
            MED_ASSERT(bufLen >= (int)fmt.length());
            char buf[bufLen];
            int n = snprintf(buf, bufLen, fmt.data(), std::forward<Args>(args)...);
            //MED_ASSERT(n >= 0 && n <= bufLen);
            if(n < 0 || n > bufLen){
                fprintf(stderr, "strsFmtN.snprintf return wrong.bufLen = %d, "
                                "but n = %d\n", bufLen, n);
            }
            return strs(String(buf, n));
        }
        inline CmdBuilder& and0(){
            mCmds.push_back("&&");
            return *this;
        }
        inline CmdBuilder& or0(){
            mCmds.push_back("|");
            return *this;
        }
        inline std::vector<String>& toCmd(){
            return mCmds;
        }
        inline CmdBuilder& clear(){
            mCmds.clear();
            return *this;
        }
    private:
        std::vector<String> mCmds;
    };
}

#endif // CMDBUILDER_H

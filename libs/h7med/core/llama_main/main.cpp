
#include <stdlib.h>
#include <sstream>
#include "core/src/platforms.h"
#include "core/src/ArgsParser.h"
#include "core/src/string_utils.hpp"
#include "core/src/FileUtils.h"
#include "core/src/CmdBuilder2.h"
#include "core/src/Prop.h"
#include "core/src/Properties.hpp"
#include "core/src/helpers.h"
#include "core/src/collections.h"

#ifdef _WIN32
#define _RT_LIB_SEP ";"
#else
#define _RT_LIB_SEP ":"
#endif

namespace h7_app {
using namespace h7;
struct RawCmdHelper{

    void runCmd(int argc, const char* argv[]){
        MED_ASSERT(argc == 2);
        med_qa::Prop prop;
        prop.load(argv[1]);
        auto p = prop.getRawProp();
        //
        std::vector<String> lds;
        p->getVector("LD_LIBRARY_PATH", lds);
        auto fd_libs = med_qa::concatStr(lds, _RT_LIB_SEP);
        //
        auto exe = p->getString("MainExe");
        MED_ASSERT_X(!exe.empty(), "must set MainExe.");
        p->getVector("PrefixParams", m_prefixParams);
        p->getVector("SuffixParams", m_suffixParams);
        auto autoRestart = p->getBool("AutoRestart", true);
        //
        setLD_LIBRARY_PATH(fd_libs);
        for(int i = 0 ; i < (int)m_prefixParams.size() ; ++i){
            m_rtParams.push_back(m_prefixParams[i]);
        }
        m_rtParams.push_back(exe);
        for(int i = 0 ; i < (int)m_suffixParams.size() ; ++i){
            m_rtParams.push_back(m_suffixParams[i]);
        }
        runInternal(autoRestart);
    }
private:
    void setLD_LIBRARY_PATH(CString fd_libs){
        if(fd_libs.empty()){
            return;
        }
        auto path = getenv("LD_LIBRARY_PATH");
        if(!path || strlen(path) == 0){
            setenv("LD_LIBRARY_PATH", fd_libs.data(), 1);
        }else{
            String apath = String(path) + _RT_LIB_SEP + fd_libs;
            setenv("LD_LIBRARY_PATH", apath.data(), 1);
        }
    }
    void runInternal(bool autoRestart){
        med_qa::CmdHelper2 ch(m_rtParams);
        auto cmd = ch.getCmd();
        do{
            if(system(cmd.data()) != 0){
                fprintf(stderr, "cmd failed: '%s'\n", cmd.data());
            }
        }while (autoRestart);
    }
private:
    std::vector<String> m_rtParams;
    std::vector<String> m_suffixParams;
    std::vector<String> m_prefixParams;
};
}

int main(int argc, const char* argv[]){
    //
    h7_app::RawCmdHelper rch;
    rch.runCmd(argc, argv);
    return 0;
}

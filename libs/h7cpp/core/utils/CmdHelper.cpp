#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sstream>

#include <errno.h>

#include "table/Column.h"
#include "utils/CmdHelper.h"

struct CharBuf{

    char* data {nullptr};

    CharBuf(int len){
        data = (char*)malloc(len);
    }
    CharBuf(int len, char val){
        data = (char*)malloc(len);
        memset(data, val, len);
    }
    ~CharBuf(){
        if(data){
            free(data);
            data = nullptr;
        }
    }
};

static bool chechCmdRet(int ret){
    if(1 == ret){
        //cancelled: cmdRunner
        PRINTERR("cmd is empty or be cancelled.\n");
    }
    else if(-1 == ret){
        PRINTERR("create sub process failed.\n");
    }
    else if(0x7f00 == ret){
        PRINTERR("cmd is error, can't run.\n");
    }
    else{
#ifdef _WIN32
        return ret == 0;
#else
        if(WIFEXITED(ret)){
            int s = WEXITSTATUS(ret);
            PRINTLN("normal exit：%d\n", s);
            return s == 0;
        }
        else if(WIFSIGNALED(ret)){
            PRINTERR("killed by signal， signal is：%d\n", WTERMSIG(ret));
        }
        else if(WIFSTOPPED(ret)){
            PRINTERR("stpped by signal，signal is ：%d\n", WSTOPSIG(ret));
        }else{
            return true;
        }
#endif
    }
    return false;
}

namespace h7 {

CmdHelper::CmdHelper(IColumn<String>* cmds)
{
    m_runner = new h7::CmdRunner();
    m_cmd = cmds->toString(" ");
}
CmdHelper::CmdHelper(const IColumn<String>& cmds)
{
    m_runner = new h7::CmdRunner();
    m_cmd = cmds.toString(" ");
}
bool CmdHelper::execute(String& out, int buffLen)const{
    PRINTLN("start execte cmd: \n %s\n", m_cmd.c_str());
    std::stringstream ss;
    CharBuf cbuf(buffLen, 0);
    char* buf = cbuf.data;

    //char buf[buffLen] = {0};
    bool state;
    if(m_runner->open(m_cmd))
    {
        int c;
        while((c = m_runner->read(buf, buffLen)) > 0)
        {
            ss << String(buf, c);
        }
        state = chechCmdRet(m_runner->close());
        out = ss.str();
    }
    else
    {
        printf("execute cmd failed. cmd = %s\n", m_cmd.c_str());
        return false;
    }
    return state;
}

bool CmdHelper::execute(std::function<void(const String&)> lineConsumer,
                        int buffLen)const{
    PRINTLN("start execte cmd: \n %s\n", m_cmd.c_str());
    std::stringstream ss;
    CharBuf cbuf(buffLen, 0);
    char* buf = cbuf.data;

    String _str;
    int _idx;
    if(m_runner->open(m_cmd))
    {
        int c;
        while((c = m_runner->read(buf, buffLen)) > 0)
        {
            ss << String(buf, c);
            _str = ss.str();
            while( (_idx = _str.find(CMD_LINE)) > 0){
                lineConsumer(_str.substr(0, _idx));
                //clear and set left
                ss.str("");
                _str = _str.substr(_idx + strlen(CMD_LINE));
                ss << _str;
            }
        }
        //if error return -1, 'errno'
        return chechCmdRet(m_runner->close());
    }
    return false;
}

}

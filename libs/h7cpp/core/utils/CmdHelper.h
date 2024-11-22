#pragma once

#include <initializer_list>
#include "common/common.h"
#include "common/SkRefCnt.h"
#include "utils/CmdRunner.h"

namespace h7 {

template <typename T>
class IColumn;

class RedirectTabCallback;

class CmdHelper: public SkRefCnt
{
public:
    CmdHelper(IColumn<String>* cmds);
    CmdHelper(const IColumn<String>& cmds);
    CmdHelper(const std::vector<String>& cmds){
        m_runner = new h7::CmdRunner();
        m_cmd.reserve(4096);
        CPP_FOREACH_END(cmds, {
                        m_cmd += *it;
                        if(!isEnd){
                            m_cmd += " ";
                        }
                    })
    }
    CmdHelper(std::initializer_list<String> cmds){
        m_runner = new h7::CmdRunner();
        m_cmd.reserve(4096);
        CPP_FOREACH_END(cmds, {
                            m_cmd += *it;
                            if(!isEnd){
                                m_cmd += " ";
                            }
                    })
    }
    CmdHelper(CString cmd){
        m_cmd = cmd;
        m_runner = new h7::CmdRunner();
    }
    CmdHelper(){
        m_runner = new h7::CmdRunner();
    }
    ~CmdHelper(){
        if(m_runner){
            m_runner->close();
            delete m_runner;
            m_runner = nullptr;
        }
    }

    bool execute(String& out,int buffLen = 2048)const;

    bool execute(std::function<void(const String&)> lineConsumer,int buffLen = 2048)const;

    inline String getCmd()const{
        return m_cmd;
    }
    void close(){
        m_runner->close();
    }
    void setName(CString name){
        this->m_name = name;
    }
private:
    String m_cmd;
    String m_name;
    h7::CmdRunner* m_runner;
};

}


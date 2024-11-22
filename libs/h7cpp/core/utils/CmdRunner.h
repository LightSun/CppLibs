#pragma once

#include "common/common.h"
#include <mutex>
#include "utils/UniqueAtomic.h"

FILE* __popen2(const char* cmd, int write, int& pid);
int __pclose2(FILE* fo, int pid);
int __pread2(FILE* fo, char* buf, unsigned int buf_size);

namespace h7 {

   class CmdRunner{
   public:
       CmdRunner(){
       }
       ~CmdRunner(){
           close();
       }
       bool open(CString cmd){
           {
               UniqueBool ato(m_mux);
               if(m_ptr != NULL){
                   return false;
               }
               //m_ptr = popen(cmd.data(), "r");
               m_ptr = __popen2(cmd.data(), 0, m_pid);
               ato.unlock();
               return m_ptr != nullptr;
           }
           return false;
       }
       int read(char* buf, unsigned int len){
           //PRINTLN("CmdRunner: read start...\n");
           {
               UniqueBool ato(m_mux);
               if(m_ptr != nullptr){
                   //PRINTLN("CmdRunner: read end...\n");
                   int ret;
                   //ret = fgets(buf, len, m_ptr) != NULL;
                   ret = __pread2(m_ptr, buf, len);
                   ato.unlock();
                   return ret;
               }
           }
           //PRINTLN("CmdRunner: read end...\n");
           return false;
       }
       int close(){
           //PRINTLN("CmdRunner: close start...\n");
           {
               UniqueBool ato(m_mux);
               if(m_ptr != nullptr){
                   int ret;
                   //ret = pclose(m_ptr);
                   ret = __pclose2(m_ptr, m_pid);
                   m_ptr = nullptr;
                   ato.unlock();
                   //PRINTLN("CmdRunner: close end...\n");
                   return ret;
               }
           }
           //PRINTLN("CmdRunner: close end...\n");
           return 1;
       }
   private:
        std::atomic<bool> m_mux{false};
        FILE *m_ptr{nullptr};
        int m_pid;
   };
}

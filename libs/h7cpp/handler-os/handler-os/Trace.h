#pragma once

#include <string>

namespace h7_handler_os{

    class Trace{
    public:
        using CString = const std::string&;

        virtual ~Trace(){}

        virtual bool isTagEnabled(long long traceTag) {
            // TODO Auto-generated method stub
            return false;
        }
        virtual void traceBegin(long long traceTag, CString traceName) {
            // TODO Auto-generated method stub

        }
        virtual void traceEnd(long long traceTag) {
            // TODO Auto-generated method stub
        }
    };
}

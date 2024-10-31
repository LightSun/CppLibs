#pragma once

#include <memory>
#include <cstring>
#include "io/InputStream.h"

namespace h7 {
    class StringInputStream : public AbsInputStream<StringInputStream>{
    public:
        bool open(CString buffer, CString){
            m_buffer = std::move(buffer);
            m_nextPos = 0;
            return true;
        }
        void close(){
        }
        //return actual read count . -1 for failed.
        size_t readCount(Count c, char* outArr){
            const auto left = leftSize();
            if(c > left) c = left;
            std::memcpy(outArr, m_buffer.data() + m_nextPos, c);
            m_nextPos += c;
            return c;
        }
        void seekDelta(Count offset){
            m_nextPos += offset;
        }
        void reset(){
            m_nextPos = 0;
        }
        Count leftSize(){
            return m_buffer.length() - m_nextPos;
        }
        Count totalSize(){
            return m_buffer.length();
        }

    private:
        String m_buffer;
        Count m_nextPos {0};
    };
}

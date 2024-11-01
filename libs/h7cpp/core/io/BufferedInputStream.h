#pragma once

#include <memory>
#include <cstring>
#include <iostream>
#include "common/common_base.h"
#include "io/InputStream.h"

namespace h7 {

class BufferedInputStream : public AbsInputStream<BufferedInputStream>{
public:
    using InputStreamPtr = InputStream*;

    //bufSize: in bytes
    BufferedInputStream(InputStreamPtr is, size_t bufSize = (2 << 20)){
        MED_ASSERT(is);
        MED_ASSERT(bufSize > 0);
        m_is = is;
        m_buffer.resize(bufSize);
    }
    bool open(CString buffer, CString p){
        m_bufPos = 0;
        m_validLen = 0;
        return m_is->open(buffer, p);
    }
    void close(){
        m_is->close();
        m_bufPos = m_buffer.size();
        m_validLen = 0;
    }
    //return actual read count . -1 for failed.
    size_t readCount(Count c, char* outArr){
        {
            const auto left = leftSize();
            if(c > left) c = left;
        }
        if(c <= bufLeftSize()){
            std::memcpy(outArr, bufData(), c);
            m_bufPos += c;
            return c;
        }
        char* nextBuf = outArr;
        Count left = c;
        while (left > 0) {
            bufFill();
            auto toCpySize = bufLeftSize();
            if(toCpySize > left){
                toCpySize = left;
            }
            std::memcpy(nextBuf, bufData(), toCpySize);
            m_bufPos += toCpySize;
            nextBuf += toCpySize;
            left -= toCpySize;
            if(left > 0 && m_is->isEnd() && bufLeftSize() == 0){
                MED_ASSERT_X(false, "[BufferedInputStream] >> called readCount(): wrong state.");
            }
        }
        return c;
    }
    void seekDelta(Count offset){
        //m_nextPos += offset;
        if(offset > 0){
            if(bufLeftSize() >= offset){
                m_bufPos += offset;
            }else{
                size_t left = offset - bufLeftSize();
                m_bufPos = m_validLen;
                m_is->seekDelta(left);
            }
        }else{
            offset = -offset;
            // 5, 8 / 8, 5
            if((size_t)offset <= m_bufPos){
                m_bufPos -= offset;
            }else{
                //m_validLen: 100, m_bufPos:10, offset: 20(-)
                m_is->seekDelta(-(m_validLen + offset));
                m_bufPos = 0;
                m_validLen = 0;
            }
        }
    }
    void reset(){
        m_bufPos = 0;
        m_validLen = 0;
        m_is->reset();
    }
    Count leftSize(){
        return bufLeftSize() + m_is->leftSize();
    }
    Count totalSize(){
        return m_is->totalSize();
    }

private:
    Count bufLeftSize(){
        return m_validLen - m_bufPos;
    }
    char* bufData(){
        return m_buffer.data() + m_bufPos;
    }
    void bufFill(){
        auto bufLeft = bufLeftSize();
        MED_ASSERT(bufLeft >= 0);
        if(bufLeft > 0){
            std::memmove(m_buffer.data(), bufData(), bufLeft);
        }
        size_t nextSize = m_buffer.size() - bufLeft;
        if(nextSize > 0){
            auto actSize = m_is->readCount(nextSize, m_buffer.data() + bufLeft);
            m_validLen = bufLeft + actSize;
        }else{
            m_validLen = bufLeft;
        }
        m_bufPos = 0;
    }
private:
    InputStreamPtr m_is;
    std::vector<char> m_buffer;
    size_t m_bufPos {0};
    size_t m_validLen {0}; //only write in 'bufFill()'

};
}

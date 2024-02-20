#ifndef MEDBYTEBUFFER_HPP
#define MEDBYTEBUFFER_HPP

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <memory.h>
#include "common/common.h"

namespace h7 {
    class SafeByteBuffer;
    class ByteBuffer{
    public:
        ByteBuffer(){}
        ByteBuffer(unsigned int size){
            m_buffer.resize(size);
        }
        int capacity(){
            return m_buffer.size();
        }
        int dataSize(){
            return m_dataSize;
        }
        void dataSize(int val){
            m_dataSize = val;
        }
        int size(){
            return m_buffer.size();
        }
        int position(){
            return m_pos;
        }
        void position(int pos){
           this->m_pos = pos;
        }
        void rewind(){
            this->m_pos = 0;
        }
        int remaining(){
            return m_buffer.size() - m_pos - m_dataSize;
        }
        //will not change pos
        void put(const void* data, int offset, int len){
            if(len > remaining()){
                moveDataToHead(m_dataSize);
            }
            MED_ASSERT_X(len <= remaining(), "buffer is not enough!");
            memcpy(m_buffer.data() + m_pos + m_dataSize, (char*)data + offset, len);
            m_dataSize += len;
        }
        void put(const void* data, int len){
            put(data, 0, len);
        }
        void get(std::vector<char>& vec, int len){
            if(vec.size() < len){
                vec.resize(len);
            }
            memcpy(vec.data(), m_buffer.data() + m_pos, len);
            m_pos += len;
            m_dataSize -= len;
        }
        int getInt(){
            int ret = *(int*)(m_buffer.data() + m_pos);
            m_pos += sizeof(int);
            m_dataSize -= sizeof(int);
            return ret;
        }
        short getShort(){
            short ret = *(short*)(m_buffer.data() + m_pos);
            m_pos += sizeof(short);
            m_dataSize -= sizeof(short);
            return ret;
        }
        long long getLong(){
            long long ret = *(long long*)(m_buffer.data() + m_pos);
            m_pos += sizeof(long long);
            m_dataSize -= sizeof(long long);
            return ret;
        }
        char get(){
            char ret = *(char*)(m_buffer.data() + m_pos);
            m_pos += sizeof(char);
            m_dataSize -= sizeof(char);
            return ret;
        }
        const char* data_ptr(){
            return m_buffer.data();
        }
    private:
        friend class SafeByteBuffer;
        std::vector<char> m_buffer;
        int m_pos {0};
        int m_dataSize {0};

        void moveDataToHead(int len){
            std::vector<char> _buf(len, 0);
            memcpy(_buf.data(), m_buffer.data() + m_pos, len);
            memcpy(m_buffer.data(), _buf.data(), len);
            m_pos = 0;
        }
    };

    class SafeByteBuffer{
    public:
        SafeByteBuffer(unsigned int size){
            m_buffer.m_buffer.resize(size);
        }
        const char* data_ptr(){
            return m_buffer.data_ptr();
        }
        int dataSize(){
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            return m_buffer.dataSize();
        }
        void dataSize(int val){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            m_buffer.dataSize(val);
        }
        int size(){
            return m_buffer.size();
        }
        int capacity(){
            return m_buffer.capacity();
        }
        int position(){
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            return m_buffer.position();
        }
        void position(int pos){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            m_buffer.position(pos);
        }
        void rewind(){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            m_buffer.rewind();
        }
        int remaining(){
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            return m_buffer.remaining();
        }
        void moveDataToHead(int len){
            m_buffer.moveDataToHead(len);
        }
        void put(const void* data, int offset, int len){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            m_buffer.put(data, offset, len);
        }
        void put(const void* data, int len){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            m_buffer.put(data, 0, len);
        }
        void get(std::vector<char>& vec, int len){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            m_buffer.get(vec, len);
        }
        int getInt(){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            auto ret = m_buffer.getInt();
            return ret;
        }
        short getShort(){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            auto ret = m_buffer.getShort();
            return ret;
        }
        long long getLong(){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            auto ret = m_buffer.getLong();
            return ret;
        }
        char get(){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            auto ret = m_buffer.get();
            return ret;
        }

    private:
        ByteBuffer m_buffer{0};
        mutable std::shared_mutex m_mutex;
    };
}

#endif // MEDBYTEBUFFER_HPP

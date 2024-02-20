#ifndef FILEIO_H
#define FILEIO_H
#include "common/common.h"
#include "common/c_common.h"

namespace h7 {

    class FileInput{

    public:
        FileInput(){}
        FileInput(CString path){
            m_file = fopen(path.data(), "rb");
        }
        ~FileInput(){
            close();
        }
        void open(CString path){
            close();
            m_file = fopen(path.data(), "rb");
        }
        bool is_open(){
            return m_file != nullptr;
        }
        sint64 getLength(){
            if(m_file == nullptr){
                return 0;
            }
            fseeko64(m_file, 0, SEEK_END);
            return ftello64(m_file);
        }
        void reset(){
            if(m_file != nullptr){
                fseeko64(m_file, 0, SEEK_SET);
            }
        }
        bool seek(uint64 pos, bool end = false){
            if(m_file == nullptr){
                return false;
            }
            if(end){
                fseeko64(m_file, 0, SEEK_END);
            }else{
                fseeko64(m_file, pos, SEEK_SET);
            }
            return true;
        }
        sint64 read(void* data, uint64 len){
            if(m_file == nullptr)
                return -1;
            return fread(data, 1, len, m_file);
        }

        String read(uint64 len){
            MED_ASSERT(len > 0);
            if(m_file != nullptr){
                std::vector<char> vec(len, 0);
                fread(vec.data(), 1, len, m_file);
                return String(vec.data(), len);
            }
            return "";
        }
        bool readline(std::string& str) {
            str.clear();
            char ch;
            while (fread(&ch, 1, 1, m_file)) {
                if (ch == '\n') {
                    // unix: LF
                    return true;
                }
                if (ch == '\r') {
                    // dos: CRLF
                    // read LF
                    if (fread(&ch, 1, 1, m_file) && ch != '\n') {
                        // mac: CR
                        fseek(m_file, -1, SEEK_CUR);
                    }
                    return true;
                }
                str += ch;
            }
            return str.length() != 0;
        }
        bool read2Vec(std::vector<char>& buf){
            if(m_file == nullptr){
                return false;
            }
            buf.resize(getLength());
            reset();
            read(buf.data(), buf.size());
            return true;
        }
        bool read2Str(String& out){
            std::vector<char> buf;
            if(!read2Vec(buf)){
                return false;
            }
            out = String(buf.data(), buf.size());
            return true;
        }
        void close(){
            if(m_file != nullptr){
                fclose(m_file);
                m_file = nullptr;
            }
        }
    private:
        FILE* m_file{nullptr};
    };

    class FileOutput{
    public:
        FileOutput(){}
        FileOutput(CString path){
            m_file = fopen(path.data(), "wb");
        }
        ~FileOutput(){
            close();
        }
        void open(CString path){
            close();
            m_file = fopen(path.data(), "wb");
        }
        bool is_open(){
            return m_file != nullptr;
        }
        void close(){
            if(m_file != nullptr){
                fclose(m_file);
                m_file = nullptr;
            }
        }
        bool write(const void* data, size_t size){
            if(m_file == nullptr){
                return false;
            }
            if(fwrite(data, 1, size, m_file) < size){
                //no left space.
                return false;
            }
            return true;
        }
        void newLine(){
            if(m_file != nullptr){
                String new_line = CMD_LINE;
                fwrite(new_line.data(), 1, new_line.length(), m_file);
            }
        }
        void flush(){
            if(m_file != nullptr){
                fflush(m_file);
            }
        }
    private:
        FILE* m_file{nullptr};
    };
}

#endif // FILEIO_H

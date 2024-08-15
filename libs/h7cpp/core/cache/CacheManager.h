#pragma once

#include <vector>
#include <string>
#include <functional>

namespace h7 {

using String = std::string;
using CString = const std::string&;

    class CacheManager{
    public:
        using uint64 = unsigned long long;
        using uint32 = unsigned int;
        enum{
          kFlag_COMPRESSED = 0x1
        };
        using Func_Enc = std::function<size_t(const char* input,
                    size_t input_length,
                    std::string* compressed)>;
        using Func_Dec = std::function<bool(const char* compressed,
                    size_t compressed_length,
                    std::string* uncompressed)>;
       //
        CacheManager(uint64 maxFragSize):m_maxFragSize(maxFragSize){
        }
        void setCompressFunc(Func_Enc&& enc){
            m_funcEnc = enc;
        }
        void setDecompressFunc(Func_Dec&& dec){
            m_funcDec = dec;
        }
        void addItem(const std::string& name, const std::string& data){
            addItem(name, data.data(), data.length());
        }
        /**
         * @brief addItem: add item data
         * @param name : the name of this item, for latter call 'getItemData' or 'getItemDataUnCompressed'
         * @param data : the item data
         * @param len : the item data length
         * @param flags : the flags. default 0
         */
        void addItem(const std::string& name, const char* data, uint64 len,
                     uint32 flags = 0);

        void addFileItem(const std::string& name, const std::string& filePath);

        void addItemCompressed(const std::string& name, const char* data, uint64 len);

        void addItemCompressed(const std::string& name, const std::string& data){
            addItemCompressed(name, data.data(), data.length());
        }
        void addFileItemCompressed(const std::string& name, const std::string& filePath);

        void getItemData(const std::string& name, std::string& out);

        void getItemDataUnCompressed(const std::string& name, std::string& out);

        void getItemAt(unsigned int index, std::string& o_name, std::string& o_data);

        void removeItem(const std::string& name);

        uint32 getItemCount();
        std::vector<String> getItemNames();

        void saveTo(const std::string& dir,const std::string& recordName,
                    const std::string& dataName);

        void compressTo(const std::string& dir,const std::string& recordName,
                        const std::string& dataName);

        bool load(const std::string& dir,const std::string& recordName,
                       const std::string& dataName);

        void reset(){
            m_data.clear();
            m_items.clear();
        }
    private:
#define __CACHE_NAME_SIZE 124
        struct Item{
            char name[__CACHE_NAME_SIZE];
            uint32 flags {0};
            std::vector<uint32> frag_ids;
            uint64 frag_offset; //the first frag offset.
            uint64 raw_size;
        };
        uint64 m_maxFragSize;
        std::vector<std::vector<char>> m_data; //Fragmentation
        std::vector<Item> m_items;
        Func_Enc m_funcEnc;
        Func_Dec m_funcDec;

    private:
        uint64 getLastFragUsedSize(){
            return !m_data.empty() ? m_data[m_data.size()-1].size() :0;
        }
        std::vector<char>& getLastBlock(uint64 newSize){
            auto& block = m_data[m_data.size()-1];
            block.resize(newSize);
            return block;
        }
        uint32 getLastBlockId(){
            return m_data.size() - 1;
        }
        std::vector<char>& newBlock(uint64 size){
            m_data.resize(m_data.size() + 1);
            std::vector<char>& block = m_data[m_data.size()-1];
            block.resize(size);
            return block;
        }

        int getItemByName(const std::string& name){
            for(int i = 0 ; i < (int)m_items.size() ; i++){
                if(name == m_items[i].name){
                    return i;
                }
            }
            return -1;
        }
        uint64 computeRecordSize();
        void writeRecordFile(const std::string& file, bool compressed);
        void getItemData0(int _idx, std::string& out);
    };
}

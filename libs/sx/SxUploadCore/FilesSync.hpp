#ifndef FILESSYNC_HPP
#define FILESSYNC_HPP

#include <shared_mutex>
#include "common/common.h"
#include "utils/FileIO.h"
#include "utils/string_utils.hpp"

#define __INTERVAL_STR "::"

namespace h7 {

    class FilesSync{
    public:
        struct Item{
            String file_enc; //enc path
            String hash;
            String userdata;
        };
        FilesSync(CString saveFile):m_savePath(saveFile){
            //read items to memory. then open for write
            FileInput fis(saveFile);
            if(fis.is_open()){
                String str;
                while (fis.readline(str)) {
                     std::vector<String> vec = utils::split(__INTERVAL_STR, str);
                     MED_ASSERT(vec.size() >= 2);
                     if(vec.size() >= 3){
                         Item item = {vec[0], vec[1], vec[2]};
                         m_unFinishItems.push_back(item);
                     }else{
                         Item item = {vec[0], vec[1], ""};
                         m_unFinishItems.push_back(item);
                     }
                }
                fis.close();
            }
            m_fio.open(saveFile);
        }
        ~FilesSync(){
            m_fio.close();
        }

        std::vector<Item>& getItems(){
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_unFinishItems;
        }

        int getItemCount(){
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_unFinishItems.size();
        }

        void addItem(CString file_enc, CString hash, CString userdata){
            Item item;
            item.file_enc = file_enc;
            item.hash = hash;
            item.userdata = userdata;
            addItem(item);
        }
        void addItem(const Item& item){
            {
                if(!item.hash.empty() && hasItem(item.hash)){
                    return;
                }
            }
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            addItemToFile(item, true);
            m_unFinishItems.push_back(item);
            PRINTLN("sync >> add file: %s\n", item.file_enc.data());
        }
        void updateItem(const Item& item){
            bool hashRepeat;
            {
                hashRepeat = hasItem(item.hash);
            }
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            int size = m_unFinishItems.size();
            for(int i = size - 1 ; i >= 0 ; --i){
                if(m_unFinishItems[i].file_enc == item.file_enc){
                    if(hashRepeat){
                        m_unFinishItems.erase(m_unFinishItems.begin() + i);
                    }else{
                        m_unFinishItems[i].hash = item.hash;
                    }
                    break;
                }
            }
        }
        bool hasItem(CString hash){
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            int size = m_unFinishItems.size();
            for(int i = size - 1 ; i >= 0 ; --i){
                if(m_unFinishItems[i].hash == hash){
                    return true;
                }
            }
            return false;
        }
        bool removeItem(CString hash){
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            //remove the item
            int size = m_unFinishItems.size();
            bool ret = false;
            for(int i = size - 1 ; i >= 0 ; --i){
                if(m_unFinishItems[i].hash == hash){
                    PRINTLN("sync >> remove file: %s\n",
                            m_unFinishItems[i].file_enc.data());
                    m_unFinishItems.erase(m_unFinishItems.begin() + i);
                    ret = true;
                    break;
                }
            }
            //find it. we need sync to file his.
            if(ret){
                m_fio.close();
                if(remove(m_savePath.data()) != 0){
                    perror("remove_file");
                }
                //save the new items to file
                m_fio.open(m_savePath);
                String msg = "open file failed. path = " + m_savePath;
                MED_ASSERT_X(m_fio.is_open(), msg);
                size = m_unFinishItems.size();
                for(int i = 0 ; i < size ; ++i){
                     addItemToFile(m_unFinishItems[i], false);
                }
                m_fio.flush();
            }
            return ret;
        }
        void close(){
            m_fio.close();
        }
        void deleteFile(){
            m_fio.close();
            if(remove(m_savePath.data()) != 0){
                perror("remove_file");
            }
        }
    private:
        mutable std::shared_mutex m_mutex;
        std::vector<Item> m_unFinishItems;
        String m_savePath;
        FileOutput m_fio;

        void addItemToFile(const Item& item, bool flush){
            String interval = __INTERVAL_STR;
            m_fio.write(item.file_enc.data(), item.file_enc.length());
            m_fio.write(interval.data(), interval.length());

            m_fio.write(item.hash.data(), item.hash.length());
            m_fio.write(interval.data(), interval.length());

            m_fio.write(item.userdata.data(), item.userdata.length());
            m_fio.newLine();
            if(flush){
                m_fio.flush();
            }
        }
    };

}

#endif // FILESSYNC_HPP

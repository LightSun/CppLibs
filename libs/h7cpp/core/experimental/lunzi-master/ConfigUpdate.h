#pragma once

#include <map>
#include <functional>
#include <mutex>
#include <sys/stat.h>

extern long long getNanoTimeStamp();

namespace PadCfgUpt
{
    typedef unsigned long long INT_64u;
    typedef std::string PATH_FILE;

    class CCfgRefresher
    {
    public:
        struct FileRefresher
        {
            //上一次修改后保存时间
            INT_64u m_last_time = 0;
            INT_64u m_new_time = 0;
            //文件名
            PATH_FILE m_file;
            //状态变化后驱动再读文件的入门
            std::function<void()> m_update_fun;
        };
    public:
        CCfgRefresher()
        {
            m_check_time = getNanoTimeStamp();
        }
        ~CCfgRefresher()
        {

        }
    public:
        void update(INT_64u timestamp)
        {
            int interval = (timestamp - m_check_time) * 0.000000001;
            if (interval < 5)
                return;
            m_check_time = timestamp;
            m_lk.lock();
            auto iter = m_files.begin();
            for (; iter != m_files.end(); iter++)
            {
                check(iter->second);
                if (iter->second.m_new_time > iter->second.m_last_time)
                {
                    iter->second.m_last_time = iter->second.m_new_time;
                    iter->second.m_update_fun();
                }
            }
            m_lk.unlock();
        }

        void addFile(const PATH_FILE& file, std::function<void()> fun)
        {
            m_lk.lock();
            auto iter = m_files.find(file);
            if (iter == m_files.end())
            {
                CCfgRefresher::FileRefresher file_;
                m_files[file] = file_;
                m_files[file].m_last_time = getNanoTimeStamp();
                m_files[file].m_new_time = file_.m_last_time;
                m_files[file].m_file = file;
                m_files[file].m_update_fun = fun;
            }
            m_lk.unlock();
        }

        void rmvFile(const PATH_FILE& file)
        {
            m_lk.lock();
            auto iter = m_files.find(file);
            if (iter != m_files.end())
            {
                m_files.erase(file);
            }
            m_lk.unlock();
        }

    protected:
        void check(FileRefresher& file)
        {
            struct stat buf;
            if (stat(file.m_file.c_str(), &buf) == 0)
            {
                file.m_new_time = buf.st_mtime * 1000000000;
            }
        }
    private:
        INT_64u m_check_time;
        std::map<PATH_FILE, FileRefresher> m_files;
        std::mutex m_lk;
    };
}

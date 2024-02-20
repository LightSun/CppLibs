#ifndef SHAREMEMORY_H
#define SHAREMEMORY_H

#include "common/common.h"
#include "common/SkRefCnt.h"

#ifdef WIN32
#include <windows.h>

#define SM_SHARE_KEY(s, global)  (global ? ("Global\\" #s): #s)
//#define SM_SHARE_KEY(s, global)  (#s)

namespace h7 {

class ShareMemory: public SkRefCnt
{
public:
    //for server
    ShareMemory(CString sm_name, size_t size):m_name(sm_name), m_size(size){};
    //for client
    ShareMemory(CString sm_name):m_name(sm_name), m_size(0){};
    ~ShareMemory(){
        destroy();
    }

    //server: create the share memory.
    bool create(){
        if(m_size == 0){
            return false;
        }
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd, true, NULL, false);
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = true;
        m_handle = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE,
                                       0, m_size, m_name.data());

//        m_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE|SEC_COMMIT,
//                                      0, m_size, m_name.data());
       // m_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
       //                               0, m_size, m_name.data());
        if(!m_handle){
            return false;
        }
        m_dataPtr = MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, m_size);
        memset(m_dataPtr, 0, m_size);
        return m_dataPtr != nullptr;
    }

    //client: share the memory.
    bool share(){
        m_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, m_name.data());
        if(!m_handle){
            return false;
        }
        m_dataPtr = MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        return m_dataPtr != nullptr;
    }

    static int getErrorCode(){
        return GetLastError();
    }

    void flush(size_t size = 0){
        FlushViewOfFile(m_dataPtr, size > 0 ? size : m_size);
    }

    bool writeData(const void* data, size_t size){
        if(!m_handle || !m_dataPtr){
            return false;
        }
        memcpy(m_dataPtr, data, size);
        return FlushViewOfFile(m_dataPtr, size);
    }
    bool writeString(CString data){
        return writeData(data.data(), data.length() + 1);
    }

#define __WRITE_T(t, T)\
    bool write##T(t val){\
        return writeData(&val, sizeof(t));\
    }
    __WRITE_T(int, Int)


    void* getDataPtr(){
        return m_dataPtr;
    }

    void destroy(){
        if(m_dataPtr){
            UnmapViewOfFile(m_dataPtr);
            m_dataPtr = nullptr;
        }
        if(m_handle){
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
    }

private:
    String m_name;
    HANDLE m_handle {nullptr};
    void* m_dataPtr {nullptr};
    size_t m_size;
};

}

#endif

#endif // SHAREMEMORY_H

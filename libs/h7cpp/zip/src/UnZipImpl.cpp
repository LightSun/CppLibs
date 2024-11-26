#include "unzip.h"
#include "ZipHelper.h"

using namespace h7;

namespace h7 {
struct UnZipImpl0{
    HZIP hz {nullptr};
    ZIPENTRY* ze {nullptr};

    ~UnZipImpl0(){
        deleteEntry();
        if(hz){
            CloseZip(hz);
            hz = nullptr;
        }
    }
    bool openZip(CString fn, CString pwd){
        hz = OpenZip(fn.data(), pwd.empty() ? 0 : pwd.data());
        return hz != nullptr;
    }
    int getItemCount(){
        deleteEntry();
        auto ze = new ZIPENTRY();
        GetZipItem(hz, -1, ze);
        return ze->index;
    }
    void deleteEntry(){
        if(ze){
            delete ze;
            ze = nullptr;
        }
    }
    bool unzip(CString dir){
       auto zr = SetUnzipBaseDir(hz, dir.data());
       if (zr != ZR_OK) {
           return false;
       }
       ZIPENTRY zep;
       auto ze = &zep;
       zr = GetZipItem(hz, -1, ze);
       if (zr != ZR_OK) {
           return false;
       }
       int numitems = ze->index;
       for (int i = 0; i<numitems; i++){
           zr = GetZipItem(hz, i, ze);
           if (zr!=ZR_OK) {
               return false;
           }
           zr = UnzipItem(hz, i, ze->name);
           if (zr != ZR_OK) {
               return false;
           }
       }
       return true;
    }
};
}

UnZipImpl::~UnZipImpl(){

}
bool UnZipImpl::openZip(CString fn, CString pwd){
    return OpenZip(fn.data(), pwd.empty() ? 0 : pwd.data());
}
int UnZipImpl::getItemCount(){

}

void UnZipImpl::close0(){
    if(m_ctx){
        ZIPENTRY* ze = (ZIPENTRY*)m_ctx;
        delete ze;
        m_ctx = nullptr;
    }
}



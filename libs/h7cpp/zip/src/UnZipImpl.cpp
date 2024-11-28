#include <vector>
#include <functional>
#include "unzip.h"
#include "ZipHelper.h"

using namespace h7;

namespace h7 {
struct UnZipImpl_Ctx0{
    HZIP hz {nullptr};
    ZIPENTRY* ze {nullptr};

    ~UnZipImpl_Ctx0(){
       close();
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
    void close(){
        deleteEntry();
        if(hz){
            CloseZip(hz);
            hz = nullptr;
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
       int okCount = 0;
       int internalErrCnt = 0;
       for (int i = 0; i<numitems; i++){
           zr = GetZipItem(hz, i, ze);
           if (zr != ZR_OK) {
               return false;
           }
           int tzr = ZR_MORE;
           do{
               tzr = UnzipItem(hz, i, ze->name);
               if (tzr == ZR_OK) {
                   //printf("%s: ok\n", ze->name);
                   okCount ++;
                   break;
               }else if(tzr == ZR_FLATE){
                   printf("%s: internal error(may be 0 byte file.).\n", ze->name);
                   internalErrCnt ++;
                   break;
               }
               else if(tzr != ZR_MORE){
                   printf("%s: non-more(%#X)\n", ze->name, tzr);
                   return false;
               }
           }while (tzr == ZR_MORE);
       }
       //printf("numitems, okCount, internalErrCnt = %d, %d, %d\n",
       //       numitems, okCount, internalErrCnt);
       return true;
    }

    bool unzip(std::function<void(CString name, char* buf,unsigned int, long long)> func_rec){
        if(!func_rec) return false;
        ZIPENTRY zep;
        auto ze = &zep;
        auto zr = GetZipItem(hz, -1, ze);
        if (zr != ZR_OK) {
            return false;
        }
        int numitems = ze->index;
//#define H7_UNZIP_BUF_SIZE 16 << 20 //16M
#define H7_UNZIP_BUF_SIZE 16 << 20
        std::vector<char> buffer;
        buffer.resize(H7_UNZIP_BUF_SIZE);
        //
        int tzr;
        for (int i = 0; i<numitems; i++){
            zr = GetZipItem(hz, i, ze);
            if (zr != ZR_OK) {
                //ZR_CORRUPT: big file.
                printf("GetZipItem >> %s: unknown. (%#X).\n", ze->name, zr);
                return false;
            }
            String name(ze->name);
            long long totsize = 0;
            do{
                tzr = UnzipItem(hz, i, buffer.data(), H7_UNZIP_BUF_SIZE);
                bool toBreak = true;
                switch (tzr) {
                case ZR_OK: {
                    auto leftSize = ze->unc_size - totsize;
                    if(leftSize < 0){
                        fprintf(stderr, "wrong state: unc_size, totsize = %lld, %lld\n",
                                ze->unc_size, totsize);
                        return false;
                    }
                    func_rec(name, buffer.data(), leftSize, ze->unc_size);
                }break;

                case ZR_MORE: {
                    toBreak = false;
                    totsize += H7_UNZIP_BUF_SIZE;
                    func_rec(name, buffer.data(), H7_UNZIP_BUF_SIZE, ze->unc_size);
                }break;

                case ZR_FLATE: {
                    //ignore. like dir or empty file.
                   // printf("%s: internal error(may be 0 byte file.).\n", ze->name);
                }break;

                default:
                    printf("%s: unknown. (%#X).\n", ze->name, tzr);
                    return false;
                }
                if(toBreak){
                    break;
                }
            }while (tzr == ZR_MORE);
        }
        return true;
    }
};
}

UnZipImpl::UnZipImpl(){
    m_ctx = new UnZipImpl_Ctx0();
}
UnZipImpl::~UnZipImpl(){
    if(m_ctx){
        delete m_ctx;
        m_ctx = nullptr;
    }
}
bool UnZipImpl::openZip(CString fn, CString pwd){
    return m_ctx->openZip(fn, pwd);
}
bool UnZipImpl::unzip(CString dir){
    return m_ctx->unzip(dir);
}
bool UnZipImpl::unzip2(CString dir){
    auto func = [](CString name, char* buf, unsigned int size, long long total){
         printf("%s: rec_size = %u, total = %lld bytes\n", name.data(), size, total);
    };
    return m_ctx->unzip(func);
}




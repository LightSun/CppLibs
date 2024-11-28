#pragma once

#include <string>

namespace h7 {

using String = std::string;
using CString = const std::string&;

class ZipImpl{
public:
    ~ZipImpl();
    bool createZip(CString fn);
    void addFile(CString key,CString file);
    void close();
};

typedef struct UnZipImpl_Ctx0 UnZipImpl_Ctx0;
class UnZipImpl{
public:
    UnZipImpl();
    ~UnZipImpl();
    bool openZip(CString fn, CString pwd = "");
    bool unzip(CString dir);
    bool unzip2(CString dir);

private:
    UnZipImpl_Ctx0* m_ctx {nullptr};
};

}

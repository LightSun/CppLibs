#pragma once

#include <string>

namespace h7 {

using String = std::string;
using CString = const std::string&;

class ZipHelper
{
public:
    ZipHelper();

    bool createZip(CString fn);
    bool openZip(CString fn);

    int getItemCount();

private:

};

class ZipImpl{
public:
    ~ZipImpl();
    bool createZip(CString fn);
    void addFile(CString key,CString file);
    void close();
};

class UnZipImpl{
public:
    ~UnZipImpl();
    bool openZip(CString fn, CString pwd);
    int getItemCount();

private:
    void close0();
    void newEntry0();
private:
    void* m_ctx {nullptr};
};

}

#pragma once

#include <vector>
#include <string>
#include <functional>

namespace h7_gz {

using CString = const std::string&;
using String = std::string;

struct IRandomWriter
{
    virtual ~IRandomWriter(){};

    virtual bool open() = 0;

    virtual void seekTo(size_t /*pos*/) = 0;

    virtual bool write(const String& buf) = 0;

    virtual bool write(const void* data, size_t len) = 0;

    virtual void close() = 0;
};

//not used for std zip.
struct ZipFileItem
{
    String name;
    String shortName;
    String ext;
    size_t contentLen;

    String readContent()const;
};

struct GroupItem
{
    String name;
    std::vector<ZipFileItem> children;

    bool isEmpty()const{return children.empty();}
    String write(CString buffer) const;
    bool read(String& bufIn, String& bufOut);

    GroupItem filter(CString ext, bool remove);
};

typedef struct GzipHelper_Ctx0 GzipHelper_Ctx0;

class GzipHelper{
public:
    using FUNC_Classify = std::function<void(const std::vector<ZipFileItem>&, std::vector<GroupItem>&)>;
    //compressor to compress file content.
    using FUNC_Compressor = std::function<bool(const std::vector<ZipFileItem>&, String*)>;
    using FUNC_DeCompressor = std::function<bool(String&,std::vector<String>&)>;

    GzipHelper();
    ~GzipHelper();

public:
    void setClassifier(FUNC_Classify func);
    void setCompressor(FUNC_Compressor func);
    void setDeCompressor(FUNC_DeCompressor func);
    void setConcurrentThreadCount(int count);
    void setAttentionFileExtensions(const std::vector<String>&);
    void setDebug(bool debug);

    bool compressDir(CString dir, CString outFile);
    bool compressFile(CString file, CString outFile);

    bool decompressFile(CString file, CString outDir);


private:
    GzipHelper_Ctx0* m_ptr;
};

}

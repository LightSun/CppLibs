#pragma once

#include "common/common.h"
#include "common/c_common.h"
#include "common/SkRefCnt.h"

namespace h7 {

class Table;
class FileReader;

//which can used to read multi table from big data.
class StreamTableReader
{
public:
    using SpTab = sk_sp<Table>;
    //using uint64 = unsigned long long;
    using Func_Anno_Predicator = std::function<bool(String&)>;
    using ListStr = std::vector<String>;

    StreamTableReader(CString file, CString lineSep);
    ~StreamTableReader();

    SpTab readTableByRowCount(uint64 rc, bool haveHead);

    SpTab readTableByAboutBytes(uint64 bytes, bool haveHead);

    bool hasNext();

    void setAnnoPredicator(Func_Anno_Predicator p){ m_annoPredicator = p;}
    void setDefaultAnnoPredicator();

private:
    bool nextLine(String& ret);
    //return valid row count
    size_t nextMaxValidLines(ListStr& out, size_t max);
    void nextMaxValidBytes(ListStr& out, size_t max_bytes);

private:
    FileReader* m_reader {nullptr};
    Func_Anno_Predicator m_annoPredicator;
    String m_lineSep;
    String m_recentLine;
    bool m_rlValid {false};
};

}


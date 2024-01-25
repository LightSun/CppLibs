#pragma once

#include "common/common.h"
#include "table/core/GRanges.h"
#include <sstream>

namespace h7 {

class GRangesHelper
{
public:
    static sk_sp<GRanges> readGRangesFromOneLine(CString data, CString seq_inner,
                                      CString seq_outter);
    static void writeFile(CString file, GRanges* gr);
    static void writeTo(std::stringstream& ss, GRanges* gr, char sep = '\t');

    static sk_sp<GRanges> readFile(CString file, int offset = 0);
    static sk_sp<GRanges> readFromBuf(CString buf, int offset = 0,
                                      bool haveHeader = true);
    static sk_sp<GRanges> readIRangesFile(String file);
    static inline sk_sp<GRanges> readFileWithoutKey(String file){
        return readIRangesFile(file);
    }
};

}


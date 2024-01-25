#pragma once

#include "table/core/Table.h"
#include <sstream>

namespace h7 {
    class TableHelper{
    public:
        static sk_sp<Table> readTableFromOneLine(CString data, CString seq_inner,
                                                 CString seq_outter);

        static sk_sp<Table> readFile(CString filePath, char seq, bool haveHead,
                                     std::function<bool(String&)> anno_pre);

        static sk_sp<Table> readFromBuf(CString buf, char seq, bool haveHead,
                                     std::function<bool(String&)> anno_pre);

        static sk_sp<Table> readFromBuf(CString buf){
            return readFromBuf(buf, DEFAULT_SEQ, true, [](String& s){
                return h7::utils::startsWith(s, "##");
            });
        }
        static sk_sp<ListS> readVcfAnno(CString filePath);

        static inline sk_sp<Table> readVcf(CString filePath, bool haveHead){
            return readFile(filePath, DEFAULT_SEQ, haveHead, [](String& s){
                return h7::utils::startsWith(s, "##");
            });
        }
        static inline sk_sp<Table> readVcf(CString filePath){
            return readFile(filePath, DEFAULT_SEQ, true, [](String& s){
                return h7::utils::startsWith(s, "##");
            });
        }

        static void writeFile(CString filePath, Table* tab, char seq);

        static void writeTo(std::stringstream& out, Table* tab, char seq = DEFAULT_SEQ);

        static inline void writeAsVcf(CString filePath, Table* tab){
            writeFile(filePath, tab, DEFAULT_SEQ);
        }
        static inline void writeAsVcf(CString filePath, sk_sp<Table> tab){
            writeFile(filePath, tab.get(), DEFAULT_SEQ);
        }

        static void writeAsVcf(CString filePath, sk_sp<Table> tab, sk_sp<ListS> anno);

        static sk_sp<Table> merge(IColumn<sk_sp<Table>>* tabs);
        inline static sk_sp<Table> merge(sk_sp<IColumn<sk_sp<Table>>> tabs){
            return merge(tabs.get());
        }

        static sk_sp<Table> mergeAsVcfFiles(ListS* files);
        static sk_sp<Table> mergeAsVcfFilesWithFilter(ListS* files,
                                std::function<sk_sp<Table>(sk_sp<Table>)> func);

        static void samToMedsam(CString file, CString out_f,int hg_type);

    private:
        static void alignNames(IColumn<sk_sp<Table>>* tabs);
    };
}

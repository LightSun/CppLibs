#include "GRangesHelper.h"
#include "utils/BufFileWriter.h"
#include "utils/FileReader.h"
#include "utils/convert.hpp"
#include "utils/FileUtils.h"
#include "utils/Regex.h"
#include "io/StringReader.h"

namespace h7 {

sk_sp<GRanges> GRangesHelper::readGRangesFromOneLine(CString data, CString seq_inner,
                                  CString seq_outter){
     Regex regex(seq_outter);
     auto lines = regex.split(data);
     sk_sp<GRanges> tab = sk_make_sp<GRanges>();

     Regex reg2(seq_inner);
     for(int i = 0 ; i < lines->size() ; ++ i){
         tab->addRowAsFull(reg2.split(lines->get(i)));
     }
     return tab;
}


void GRangesHelper::writeFile(CString file, GRanges* gr){
    BufFileWriter fw(file);
    fw.writeLine(gr->getName());
    int rc = gr->getRowCount();
    for(int i = 0 ; i < rc ; i ++){
        fw.writeListAsLine(gr->getFullRowData(i)->list);
    }
    fw.close();
}

void GRangesHelper::writeTo(std::stringstream& ss, GRanges* gr, char sep){
    ss << gr->getName();
    ss << CMD_LINE;

    const int rc = gr->getRowCount();
    for(int ir = 0 ; ir < rc ; ir ++){
        auto vec = gr->getFullRowData(ir);
        int size = vec->size();
        for(int i = 0 ; i < size ; i ++){
            ss << vec->get(i);
            if(i != size - 1){
                ss << sep;
            }
        }
        ss << CMD_LINE;
    }
}

static int toInt(CString str, int offset){
    int val = h7::getIntImpl<String>(str);
    if(val == INT_MIN){
        return INT_MIN;
    }
    return val + offset;
}

sk_sp<GRanges> GRangesHelper::readFile(CString file, int offset){
    printf("readFile >> start read GRange file: %s !\n", file.c_str());
    int rc = FileUtils::getFileLineCount(file);
    MED_ASSERT(rc >= 0);
    //h7::Regex regex("\t");

    FileReader reader(file);
    MED_ASSERT(reader.isOpen());
    sk_sp<ListS> keys = sk_make_sp<ListS>(rc);
    sk_sp<ListI> starts = sk_make_sp<ListI>(rc);
    sk_sp<ListI> ends = sk_make_sp<ListI>(rc);
    //
    String name;
    h7::Regex reg("\t");
    if(reader.readLine(name)){
        auto vec = reg.split(name);
        if(vec->size() > 1){
            printf("GRangesHelper::readFile >> warning: grange name should not have '\\t'.\n");
        }
        //MED_ASSERT_X(vec.size() == 1, "grange name should not have '\t'.");
    }
    String str;
    while(reader.readLine(str)){
        auto vec = reg.split(str);
        MED_ASSERT_X(vec->size() >= 2, str);
        switch (vec->size()) {
        case 2:
        {
            keys->add("");
            starts->add(toInt(vec->get(0), offset));
            ends->add(toInt(vec->get(1), offset));
        }
            break;
        case 3:
        default:
            keys->add(vec->get(0));
            starts->add(toInt(vec->get(1), offset));
            ends->add(toInt(vec->get(2), offset));
            break;
        }
    }
    reader.close();
    printf("readFile >> end read GRange file: %s.\n", file.c_str());
    sk_sp<GRanges> grg = sk_make_sp<GRanges>(keys, starts, ends);
    grg->setName(name);
    return grg;
}

sk_sp<GRanges> GRangesHelper::readFromBuf(CString buf, int offset, bool haveHeader){
    StringReader reader(buf);
    int rc = reader.getLineCount();
    MED_ASSERT(rc >= 0);

    sk_sp<ListS> keys = sk_make_sp<ListS>(rc);
    sk_sp<ListI> starts = sk_make_sp<ListI>(rc);
    sk_sp<ListI> ends = sk_make_sp<ListI>(rc);
    //
    h7::Regex reg("\t");
    String name;
    if(haveHeader){
        if(reader.readLine(name)){
            auto vec = reg.split(name);
            if(vec->size() > 1){
                fprintf(stderr, "GRangesHelper::readFile >> warning: grange name should not have '\\t'.\n");
            }
            //MED_ASSERT_X(vec.size() == 1, "grange name should not have '\t'.");
        }
    }
    String str;
    while(reader.readLine(str)){
        auto vec = reg.split(str);
        if(vec->size() < 2){
            break;
        }
        //MED_ASSERT_X(vec->size() >= 2, str);
        switch (vec->size()) {
        case 2:
        {
            keys->add("");
            starts->add(toInt(vec->get(0), offset));
            ends->add(toInt(vec->get(1), offset));
        }
            break;
        case 3:
        default:
            keys->add(vec->get(0));
            starts->add(toInt(vec->get(1), offset));
            ends->add(toInt(vec->get(2), offset));
            break;
        }
    }
    reader.close();
    sk_sp<GRanges> grg = sk_make_sp<GRanges>(keys, starts, ends);
    grg->setName(name);
    return grg;
}
//iRangeFile .no names
sk_sp<GRanges> GRangesHelper::readIRangesFile(String file){
    //printf("readIRangesFile >> start read GRange file: %s !\n", file.c_str());
    int rc = FileUtils::getFileLineCount(file);
    MED_ASSERT(rc >= 0);

    FileReader reader(file);
    MED_ASSERT(reader.isOpen());
    sk_sp<ListS> keys = sk_make_sp<ListS>(rc);
    sk_sp<ListI> starts = sk_make_sp<ListI>(rc);
    sk_sp<ListI> ends = sk_make_sp<ListI>(rc);

    String str;
    h7::Regex reg("\t");
    while(reader.readLine(str)){
        auto vec = reg.split(str);
        if(vec->size() < 2){
            break;
        }
        //MED_ASSERT(vec->size() >= 2);
        switch (vec->size()) {
        case 2:
        {
            keys->add("");
            starts->add(h7::getIntImpl<String>(vec->get(0)));
            ends->add(h7::getIntImpl<String>(vec->get(1)));
        }
            break;
        case 3:
        default:
            keys->add(vec->get(0));
            starts->add(h7::getIntImpl<String>(vec->get(1)));
            ends->add(h7::getIntImpl<String>(vec->get(2)));
            break;
        }
    }
    reader.close();
    //printf("readIRangesFile>> end read GRange file: %s.\n", file.c_str());
    return sk_make_sp<GRanges>(keys, starts, ends);
}

}

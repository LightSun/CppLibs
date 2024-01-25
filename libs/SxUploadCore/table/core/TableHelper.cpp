
#include "table/core/TableHelper.h"
#include "utils/FileReader.h"
#include "utils/BufFileWriter.h"
#include "utils/FileUtils.h"
#include "utils/Regex.h"
#include "io/StringReader.h"

namespace h7{
    sk_sp<Table> TableHelper::readTableFromOneLine(CString data, CString seq_inner,
                                      CString seq_outter){
         Regex regex(seq_outter);
         auto lines = regex.split(data);
         sk_sp<Table> tab = sk_make_sp<Table>();

         Regex reg2(seq_inner);
         for(int i = 0 ; i < lines->size() ; ++ i){
             tab->addRowAsFull(reg2.split(lines->get(i)));
         }
         return tab;
    }

    sk_sp<Table> TableHelper::readFile(CString filePath, char seq, bool haveHead,
                                     std::function<bool(String&)> anno_pre){
        bool prepared = false;
        Regex regex(String(1, seq));
        //if read it and save as column.
        sk_sp<Table> tab = sk_make_sp<Table>();
        h7::FileReader reader(filePath);
        String str;
        //read head
        if(haveHead){
            int rc = FileUtils::getFileLineCount(filePath);
            MED_ASSERT(rc >= 0);
            while(reader.readLine(str)){
                if(!anno_pre(str)){
                    auto sp = regex.split(str);
                    tab->setHeadLine(sp);
                    tab->prepareColumnCount(sp->size());
                    tab->prepareRowCount(rc);
                    prepared = true;
                    break;
                }
            }
        }
        //read content
        if(prepared){
            while(reader.readLine(str)){
                if(!anno_pre(str)){
                    if(utils::startsWith(str, "#CHROM	POS	ID	REF	ALT	QUAL	FILTER	INFO")){
                        continue;
                    }
                    tab->addRowAsFull(regex.split(str));
                }
            }
        }else{
            std::vector<String> lines;
            reader.readLines(lines, anno_pre);
           // tab->prepareColumnCount(sp.size());
            tab->prepareRowCount(lines.size());
            for(int i = 0 ; i < (int)lines.size() ; i ++){
                tab->addRowAsFull(regex.split(lines[i]));
            }
        }
        reader.close();
        return tab;
    }
    void TableHelper::writeAsVcf(CString filePath, sk_sp<Table> tab, sk_sp<ListS> anno){
        h7::BufFileWriter tw(filePath);
        //anno
        if(anno){
            int size = anno->size();
            for(int i = 0 ; i < size ; ++i){
                tw.writeLine(anno->get(i));
            }
        }
        //
        auto headLine = tab->getHeadLine();
        if(headLine && headLine->size() > 0){
            tw.writeLine(headLine->toString(DEFAULT_SEQ));
        }
        const int rc = tab->getRowCount();
        for(int i = 0 ; i < rc ; i ++){
            tw.writeLine(tab->getRow(i)->toString(DEFAULT_SEQ));
        }
        tw.close();
    }

    void TableHelper::writeFile(CString filePath, Table* tab, char seq){
        h7::BufFileWriter tw(filePath);
        auto headLine = tab->getHeadLine();
        if(headLine && headLine->size() > 0){
            tw.writeLine(headLine->toString(seq));
        }
        const int rc = tab->getRowCount();
        for(int i = 0 ; i < rc ; i ++){
            tw.writeLine(tab->getRow(i)->toString(seq));
        }
        tw.close();
    }
    void TableHelper::writeTo(std::stringstream& out, Table* tab, char seq){
        auto headLine = tab->getHeadLine();
        if(headLine && headLine->size() > 0){
            out << headLine->toString(seq);
            out << CMD_LINE;
        }
        const int rc = tab->getRowCount();
        for(int i = 0 ; i < rc ; i ++){
            out << tab->getRow(i)->toString(seq);
            out << CMD_LINE;
        }
    }
    sk_sp<ListS> TableHelper::readVcfAnno(CString filePath){
        h7::FileReader reader(filePath);
        String str;
        //read head
        sk_sp<ListS> ret = sk_make_sp<ListS>(256);
        while(reader.readLine(str)){
            if(h7::utils::startsWith(str, "##")){
                ret->add(str);
            }else{
                break;
            }
        }
        return ret;
    }
    void TableHelper::alignNames(IColumn<sk_sp<Table>>* _tabs){
        auto& tabs = *_tabs;
        // get all col names.
        // fill up cols then merge.
        SetS set;
        for(int i = 0 ; i < tabs.size(); ++i){
             auto head = tabs.get(i)->getHeadLine();
             set.addAll(head->list);
        }
        MED_ASSERT(set.size() > 0);
        ListS allNames;
        allNames.list = set.toList();
        for(int i = 0 ; i < tabs.size(); ++i){
            tabs.get(i)->alignByNames(&allNames);
        }
    }
    sk_sp<Table> TableHelper::merge(IColumn<sk_sp<Table>>* tabs){
        MED_ASSERT(tabs->size() > 0);
        sk_sp<Table> tab = tabs->get(0);
        int size = tabs->size();
        for(int i = 1 ; i < size ; i ++){
            tab->mergeTableByRow(tabs->get(i));
        }
        return tab;
    }

    sk_sp<Table> TableHelper::mergeAsVcfFiles(ListS* files){
        IColumn<sk_sp<Table>> tabs;
        int size = files->size();
        tabs.prepareSize(size);
        for(int i = 0 ; i < size ; i ++){
            if(FileUtils::isFileExists(files->get(i))){
                tabs.add(readVcf(files->get(i)));
            }
        }
        alignNames(&tabs);
//        int col_size = -1;
//        for(int i = 0 ; i < tabs.size() ; ++i){
//            int cs = tabs.get(i)->getColumnCount();
//            if(col_size < 0){
//                col_size = cs;
//            }else if(col_size != cs){
//                return nullptr;
//            }
//        }
        return merge(&tabs);
    }
    sk_sp<Table> TableHelper::mergeAsVcfFilesWithFilter(ListS* files,
                     std::function<sk_sp<Table>(sk_sp<Table>)> func){
        IColumn<sk_sp<Table>> tabs;
        int size = files->size();
        tabs.prepareSize(size);
        for(int i = 0 ; i < size ; i ++){
            if(FileUtils::isFileExists(files->get(i))){
                sk_sp<Table> t = readVcf(files->get(i));
                if(func){
                    t = func(t);
                }
                tabs.add(t);
            }
        }
        return merge(&tabs);
    }

    //------------------------------------------------
    sk_sp<Table> TableHelper::readFromBuf(CString buf, char seq, bool haveHead,
                             std::function<bool(String&)> anno_pre){
        bool prepared = false;
        Regex regex(String(1, seq));
        //if read it and save as column.
        sk_sp<Table> tab = sk_make_sp<Table>();
        h7::StringReader reader(buf);
        String str;
        //read head
        if(haveHead){
            int rc = reader.getLineCount();
            MED_ASSERT(rc >= 0);
            while(reader.readLine(str)){
                //if(str.data()[0] == '@' && str.data()[1])
                if(!anno_pre(str)){
                    auto sp = regex.split(str);
                    tab->setHeadLine(sp);
                    tab->prepareColumnCount(sp->size());
                    tab->prepareRowCount(rc);
                    prepared = true;
                    break;
                }
            }
        }
        //read content
        if(prepared){
            while(reader.readLine(str)){
                if(!anno_pre(str)){
                    tab->addRowAsFull(regex.split(str));
                }
            }
        }else{
            std::vector<String> lines;
            reader.readLines(lines, anno_pre);
           // tab->prepareColumnCount(sp.size());
            tab->prepareRowCount(lines.size());
            for(int i = 0 ; i < (int)lines.size() ; i ++){
                tab->addRowAsFull(regex.split(lines[i]));
            }
        }
        reader.close();
        return tab;
    }

//    static String _handle_chrom(int hg, CString str){
//        if(hg == TYPE_HG19){
//            String ret = h7::utils::replace("chr", "", str);
//            return h7::utils::replace("M", "MT", ret);
//        }else if(hg == TYPE_HG38){
//            //chrM
//            if(str == "MT"){
//                return "chrM";
//            }
//            if(!h7::utils::startsWith(str, "chr")){
//                return "chr" + str;
//            }
//            return str;
//        }else{
//            PRINTERR("wrong hg type = %d", hg);
//            return str;
//        }
//    }

    void TableHelper::samToMedsam(CString file, CString out_f,
                                  int hg_type){
        ////#CHROM POS ID REF ALT
        /// 0,1,3,4, 8 ...
        auto tab = readVcf(file);
        auto tabo = sk_make_sp<Table>();
        int rc = tab->getRowCount();
        int cc = tab->getColumnCount();
        //
        ListI idx;
        idx.add(0);
        idx.add(1);
        idx.add(3);
        idx.add(4);
        idx.add(8);
        sk_sp<ListS> head = sk_make_sp<ListS>();
        head->addAll({"#CHROM", "POS", "REF", "ALT", "FORMAT"});
        for(int i = 9 ; i < cc ; ++i){
            idx.add(i);
            head->add("fval_" + std::to_string(i));
        }
        tabo->setHeadLine(head);
        tabo->prepareRowCount(rc);

        for(int i = 0 ; i < rc ; ++i){
            auto row = tab->get(i, &idx);
//            String key = _handle_chrom(hg_type, row->get(0))
//                      + (":" + row->get(1));
//            key += (":" + row->get(3));
//            key += (":" + row->get(4));
//            _keys.add(key);

            tabo->addRowAsFull(row);
        }
        writeFile(out_f, tabo.get(), DEFAULT_SEQ);
    }
}

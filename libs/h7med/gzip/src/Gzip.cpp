#include <iostream>
#include <fstream>
#include <memory>
#include "Gzip.h"
#include "core/src/FileUtils.h"
//#include "core/src/string_utils.hpp"
#include "core/src/ByteBufferIO.h"
#include "core/src/hash.h"
#include "core/src/ThreadPool.h"
#include "core/src/ConcurrentWorker.h"

#define DEFAUL_HASH_LEN (4 << 20) //4M
#define DEFAUL_HASH_SEED 17

namespace h7_gz {

static inline bool _contains0(const std::vector<String>& vec, CString str){
    for(auto& s : vec){
        if(s == str){
            return true;
        }
    }
    return false;
}

using FUNC_Classify = GzipHelper::FUNC_Classify;
using FUNC_Compressor = GzipHelper::FUNC_Compressor;
using FUNC_DeCompressor = GzipHelper::FUNC_DeCompressor;

String ZipFileItem::readContent()const{
    return h7::FileUtils::getFileContent(name);
}

String GroupItem::write(CString buffer) const{
    h7::ByteBufferOut bos(buffer.length() + (1 << 20));
    bos.putInt(children.size());
    for(const ZipFileItem& zi : children){
        bos.putString16(zi.shortName);
    }
    bos.putString16(name);
    size_t hash_len = buffer.size() > DEFAUL_HASH_LEN ? DEFAUL_HASH_LEN : buffer.size();
    uint64 hval = fasthash64(buffer.data(), hash_len, DEFAUL_HASH_SEED);
    bos.putULong(hval);
    bos.putString64(buffer);
    return bos.bufferToString();
}

bool GroupItem::read(String& bufIn, String& bufOut){
    h7::ByteBufferIO bis(&bufIn);
    int childrenCnt = bis.getInt();
    children.resize(childrenCnt);
    for(ZipFileItem& zi : children){
        zi.shortName = bis.getString16();
    }
    this->name = bis.getString16();
    auto hash = bis.getULong();
    bufOut = bis.getString64();

    size_t hash_len = bufOut.size() > DEFAUL_HASH_LEN ? DEFAUL_HASH_LEN : bufOut.size();
    uint64 hval = fasthash64(bufOut.data(), hash_len, DEFAUL_HASH_SEED);
    if(hash != hval){
        return false;
    }
    return true;
}

struct GroupItemTask{
    int index;
    GroupItem* gi;
    String cmpBuffer;

    GroupItemTask(int index, GroupItem* gi,CString buffer):
        index(index), gi(gi), cmpBuffer(buffer){}
};
struct GroupItemState{
    GroupItem gi;
    uint64 bufLen {0};
    bool state {false};
};

struct FileWriter0: public IRandomWriter{

    String path_;
    std::fstream fos_;

    FileWriter0(CString path):path_(path){
        auto dir = h7::FileUtils::getFileDir(path);
        h7::FileUtils::mkdirs(dir);
    }

    ~FileWriter0(){
        close0();
    }
    bool open() override{
        fos_.open(path_.data(), std::ios::binary);
        return fos_.is_open();
    }
    void seekTo(size_t pos) override{
        fos_.seekp(pos);
    }
    bool write(const String& buf) override{
        size_t len = buf.size();
        if(!write(&len, sizeof(len))){
            return false;
        }
        return write(buf.data(), buf.length());
    }
    bool write(const void* data, size_t len) override{
        fos_.write((char*)data, len);
        return true;
    }
    void close() override{
        close0();
    }
    void close0(){
        if(fos_.is_open()){
            fos_.close();
        }
    }
};

struct ZipHeader0{
    String magic {"7NEVAEH"};
    int version {1};
    int groupCount {0};
    std::vector<int> nameLens;
    std::vector<size_t> compressedLens;   

    String str(bool mock)const{
        h7::ByteBufferOut bos(4096);
        bos.putString(magic);
        bos.putInt(version);
        bos.putInt(groupCount);
        if(mock){
            for(int i = 0 ; i < groupCount; ++i){
                bos.putInt(0);
            }
            for(int i = 0 ; i < groupCount; ++i){
                bos.putULong(0);
            }
        }else{
            MED_ASSERT((int)nameLens.size() == groupCount);
            MED_ASSERT((int)compressedLens.size() == groupCount);
            for(auto& len : nameLens){
                bos.putInt(len);
            }
            for(int i = 0 ; i < groupCount; ++i){
                bos.putULong(compressedLens[i]);
            }
        }
        return bos.bufferToString();
    }

    void parse(String& buf){
        nameLens.clear();
        compressedLens.clear();
        h7::ByteBufferIO bio(&buf);
        magic = bio.getString();
        version = bio.getInt();
        groupCount = bio.getInt();
        for(int i = 0 ; i < groupCount; ++i){
            nameLens.push_back(bio.getInt());
        }
        for(int i = 0 ; i < groupCount; ++i){
            compressedLens.push_back(bio.getULong());
        }
    }
};

struct RandomWriteWrapper: public IRandomWriter
{
    IRandomWriter* impl {nullptr};
    bool opened {false};

    RandomWriteWrapper(IRandomWriter* impl): impl(impl){}
    ~RandomWriteWrapper(){
        close0();
    }

    bool open() override{
        if(impl->open()){
            opened = true;
            return true;
        }
        return false;
    }
    void seekTo(size_t pos) override{
        impl->seekTo(pos);
    }
    bool write(const String& buf) override{
        size_t len = buf.size();
        if(!impl->write(&len, sizeof(len))){
            return false;
        }
        return impl->write(buf);
    }
    bool write(const void* data, size_t len) override{
        return impl->write(data, len);
    }
    void close() override{
        close0();
    }
    void close0(){
        if(opened){
            impl->close();
            opened = false;
        }
    }
};

struct GzipHelper_Ctx0{

    std::vector<String> extFilters;//allowed exts. like "png,jpg,txt...etc"
    FUNC_Classify func_classify;
    FUNC_Compressor func_compressor;
    FUNC_DeCompressor func_deCompressor;
    int concurrentCnt {1};

    bool compressDir(CString dir, CString outFile){
        std::vector<String> vec;
        std::vector<String> exts;
        {
            auto files = h7::FileUtils::getFiles(dir, true, "");
            vec.reserve(files.size());
            exts.reserve(files.size());
            //filter
            if(!extFilters.empty()){
                for(auto& f: files){
                    String ext;
                    int pos = f.rfind(".");
                    if(pos >= 0){
                        ext = f.substr(pos + 1);
                    }
                    if(_contains0(extFilters, ext)){
                        vec.push_back(f);
                        exts.push_back(ext);
                    }
                }
            }else{
                vec = std::move(files);
                for(auto& f : vec){
                    String ext;
                    int pos = f.rfind(".");
                    if(pos >= 0){
                        ext = f.substr(pos + 1);
                    }
                    exts.push_back(ext);
                }
            }
        }
        FileWriter0 fw(outFile);
        return compressImpl0(dir, vec, exts, &fw);
    }

    bool compressFile(CString f, CString outFile){
        std::vector<String> vec;
        std::vector<String> exts;
        String ext;
        int pos = f.rfind(".");
        if(pos >= 0){
            ext = f.substr(pos + 1);
        }
        exts.push_back(ext);
        vec.push_back(f);
        //
        FileWriter0 fw(outFile);
        return compressImpl0(h7::FileUtils::getFileDir(f), vec, exts, &fw);
    }
    bool decompressFile(CString file, CString outDir){
        std::ifstream fis;
        fis.open(file, std::ios::binary);
        if(!fis.is_open()){
            return false;
        }
        ZipHeader0 header;
        //read head
        {
            size_t headerSize = 0;
            fis.read((char*)&headerSize, sizeof(size_t));
            if(fis.fail() || headerSize == 0){
                return false;
            }
            String headBuf;
            headBuf.resize(headerSize);
            fis.read(headBuf.data(), headerSize);
            if(fis.fail()){
                return false;
            }
            header.parse(headBuf);
            if(header.compressedLens.size() != header.groupCount){
                return false;
            }
            if(header.nameLens.size() != header.groupCount){
                return false;
            }
        }
        //read body.
        std::vector<std::shared_ptr<GroupItemState>> gitems;
        {
            h7::ThreadPool pool(concurrentCnt);
            for(;;){
                size_t blockSize = 0;
                fis.read((char*)&blockSize, sizeof(size_t));
                if(fis.fail()){
                    break;
                }
                auto bufPtr = std::shared_ptr<String>(new String());
                bufPtr->resize(blockSize);
                fis.read(bufPtr->data(), blockSize);
                if(fis.fail()){
                    return false;
                }
                auto gs = std::make_shared<GroupItemState>();
                gitems.push_back(gs);
                pool.enqueue([this, outDir, bufPtr, gs](){
                    std::vector<String> datas;
                    {
                        String bufOut;
                        gs->gi.read(*bufPtr, bufOut);
                        if(!func_deCompressor(bufOut, datas)){
                            gs->state = false;
                            return;
                        }
                        gs->bufLen = bufOut.size();
                        if(gs->gi.children.size() != datas.size()){
                            gs->state = false;
                            return;
                        }
                        gs->state = true;
                    }
                    for(int i = 0 ; i < (int)datas.size() ; ++i){
                        String outFile = outDir + "/" + gs->gi.children[i].shortName;
                        FileWriter0 fw(outFile);
                        if(fw.open()){
                            auto& dat = datas[i];
                            fw.write(dat.data(), dat.size());
                            fw.close();
                        }else{
                            fprintf(stderr, "write file failed. %s\n", outFile.data());
                        }
                    }
                });
            }
        }
        //verify
        if(header.groupCount != gitems.size()){
            return false;
        }
        for(int i = 0 ; i < header.groupCount ; ++i){
            if(!gitems[i]->state){
                return false;
            }
            if(gitems[i]->gi.name.length() != header.nameLens[i]){
                return false;
            }
            if(gitems[i]->bufLen != header.compressedLens[i]){
                return false;
            }
        }
        return true;
    }

private:

    bool compressImpl0(CString dir, std::vector<String>& vec, std::vector<String>& exts, IRandomWriter* wri){
        std::vector<GroupItem> gitems;
        {
            std::vector<ZipFileItem> items;
            for(int i = 0 ; i < (int)vec.size() ; ++i){
                ZipFileItem fi;
                fi.ext = exts[i];
                fi.contentLen = h7::FileUtils::getFileSize(vec[i]);
                fi.name = vec[i];
                fi.shortName = fi.name.substr(dir.length() + 1);
                items.push_back(std::move(fi));
            }
            //category
            if(items.size() > 1){
                func_classify(items, gitems);
            }else{
                GroupItem gi;
                gi.name = "Default";
                gi.children = std::move(items);
                gitems.push_back(std::move(gi));
            }
        }
        //compress
        ZipHeader0 header;
        header.groupCount = gitems.size();
        auto writer = std::make_unique<RandomWriteWrapper>(wri);
        if(!writer->open()){
            return false;
        }
        if(!writer->write(header.str(true))){
            return false;
        }
        if(concurrentCnt > 1){
            using SpGITask = std::shared_ptr<GroupItemTask>;
            auto writer0 = writer.get();
            std::vector<int> writeRets(gitems.size(), 1);
            h7::ConcurrentWorker<SpGITask> worker(gitems.size(),
                                                [this, writer0, &header, &writeRets](SpGITask& st){
                 writeRets[st->index] = doWrite(writer0, header, st->gi, st->cmpBuffer);
            });
            worker.start();
            //multi threads compress.
            //write to a temp file. then merge?. NO IO may be too lot.
            int ret = h7::ThreadPool::batchRawRun(concurrentCnt, 0, gitems.size(),
                                                      [this, &gitems, &worker](int i){
                    auto& children = gitems[i].children;
                    auto& key = gitems[i].name;
                    String buffer;
                    if(!func_compressor(children, &buffer)){
                        return false;
                    }
                    if(buffer.empty()){
                        return false;
                    }
                    worker.addTask(std::make_shared<GroupItemTask>(i, &gitems[i], std::move(buffer)));
                    return true;
                });
            //stop write worker
            worker.stop();
            if(ret == 0){
                return false;
            }
            MED_ASSERT(writeRets.size() == gitems.size());
            for(auto& writeR: writeRets){
                if(!writeR){
                    return false;
                }
            }
        }else{
            for(auto it = gitems.begin(); it != gitems.end(); ++it){
                auto& children = it->children;
                auto& key = it->name;
                //
                String buffer;
                func_compressor(children, &buffer);
                if(buffer.empty()){
                    return false;
                }
                auto& gitem = *it;
                if(!doWrite(writer.get(), header, &gitem, buffer)){
                    return false;
                }
            }
        }
        writer->seekTo(0);
        if(!writer->write(header.str(false))){
            return false;
        }
        writer->close();
        return true;
    }
    bool doWrite(IRandomWriter* writer, ZipHeader0& header,GroupItem* gi, CString buffer){
        String data = gi->write(buffer);
        {
            std::unique_lock<std::mutex> lck(_mtx_write);
            if(!writer->write(data)){
                return false;
            }
            header.nameLens.push_back(gi->name.size());
            header.compressedLens.push_back(buffer.size());
        }
        return true;
    }

private:
    std::mutex _mtx_write;
};
//---------------------------

GzipHelper::GzipHelper(){
    m_ptr = new GzipHelper_Ctx0();
}
GzipHelper::~GzipHelper(){
    if(m_ptr){
        delete m_ptr;
        m_ptr = nullptr;
    }
}

void GzipHelper::setClassifier(FUNC_Classify func){
    m_ptr->func_classify = func;
}
void GzipHelper::setCompressor(FUNC_Compressor func){
    m_ptr->func_compressor = func;
}
void GzipHelper::setDeCompressor(FUNC_DeCompressor func){
    m_ptr->func_deCompressor = func;
}
void GzipHelper::setConcurrentThreadCount(int count){
    m_ptr->concurrentCnt = count;
}
bool GzipHelper::compressDir(CString dir, CString outFile){
    return m_ptr->compressDir(dir, outFile);
}
bool GzipHelper::compressFile(CString file, CString outFile){
    return m_ptr->compressFile(file, outFile);
}
bool GzipHelper::decompressFile(CString file, CString outDir){
    return m_ptr->decompressFile(file, outDir);
}
//
}

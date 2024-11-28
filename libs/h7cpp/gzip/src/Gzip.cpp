#include <iostream>
#include "Gzip.h"
#include "zlib.h"

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        return false; \
    } \
}

namespace h7 {

struct GzipHelper{

    void checkVersion(){
    }

    bool compress(IZlibInput* in, IZlibOutput* out){
        z_stream c_stream;
        int err;
        c_stream.zalloc = nullptr;
        c_stream.zfree = nullptr;
        c_stream.opaque = (voidpf)0;
        c_stream.next_in = nullptr;

        err = deflateInit(&c_stream, Z_BEST_SPEED);
        CHECK_ERR(err, "deflateInit");
        //
        std::vector<char> bufIn;
        std::vector<char> bufOut;
        bool isFinish = false;
        while (!isFinish && in->hasNext()) {
            auto len = in->next(bufIn, isFinish);
            if(bufOut.size() < bufIn.size()){
                bufOut.resize(bufIn.size());
            }
            const int flush = isFinish ? Z_FINISH : Z_NO_FLUSH;
            auto aviOut_len = bufOut.size();
            c_stream.next_in = (Bytef*)bufIn.data();
            c_stream.avail_in = (uInt)len;
            do{
                c_stream.next_out = (Bytef*)bufOut.data();
                c_stream.avail_out = aviOut_len;

                err = deflate(&c_stream, flush);
                if(err != Z_OK){
                    fprintf(stderr, "deflate error: %d\n", err);
                    deflateEnd(&c_stream);
                    return false;
                }
                auto cmpSize = aviOut_len - c_stream.avail_out;
                if(cmpSize > 0 && !out->write(bufOut, cmpSize)){
                    deflateEnd(&c_stream);
                    return false;
                }
            }while(c_stream.avail_out == 0);
        }
        err = deflateEnd(&c_stream);
        CHECK_ERR(err, "deflateEnd");
        return true;
    }

    //
    bool decompress(IZlibInput* in, IZlibOutput* out){
        z_stream c_stream;

        c_stream.zalloc = Z_NULL;
        c_stream.zfree = Z_NULL;
        c_stream.opaque = Z_NULL;
        c_stream.avail_in = 0;
        c_stream.next_in = Z_NULL;
        auto err = inflateInit(&c_stream);
        CHECK_ERR(err, "inflateInit");
        //
        std::vector<char> bufIn;
        std::vector<char> bufOut;
        bool isFinish = false;
        while (!isFinish && in->hasNext()) {
            auto len = in->next(bufIn, isFinish);
            if(bufOut.size() < bufIn.size()){
                bufOut.resize(bufIn.size());
            }
            c_stream.next_in = (Bytef*)bufIn.data();
            c_stream.avail_in = (uInt)len;
            auto aviOut_len = bufOut.size();
            do{
                c_stream.next_out = (Bytef*)bufOut.data();
                c_stream.avail_out = aviOut_len;

                err = inflate(&c_stream, Z_NO_FLUSH);
                switch (err) {
                case Z_NEED_DICT:
                    fprintf(stderr, "decompress >> Error Z_NEED_DICT.\n");
                    err = Z_DATA_ERROR; //as data-error
                    (void)inflateEnd(&c_stream);
                    return false;
                case Z_DATA_ERROR:
                    fprintf(stderr, "decompress >> Error Z_DATA_ERROR.\n");
                    (void)inflateEnd(&c_stream);
                    return false;

                case Z_MEM_ERROR:
                    fprintf(stderr, "decompress >> Error Z_MEM_ERROR.\n");
                    (void)inflateEnd(&c_stream);
                    return false;
                }
                auto cmpSize = aviOut_len - c_stream.avail_out;
                if(cmpSize > 0 && !out->write(bufOut, cmpSize)){
                    inflateEnd(&c_stream);
                    return false;
                }
            }while(c_stream.avail_out == 0);
        }
        (void)inflateEnd(&c_stream);
        return err == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
    }
};
}

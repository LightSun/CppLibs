
#include <vector>
#include <string>
#include "ui_upload/pub_api.h"
#include "utils/url_encode.h"
#include "utils/string_utils.hpp"

#include "utils/ByteBuffer.hpp"
#include "ipc/ShareMemory.h"

using String = std::string;
using namespace med_api;
using namespace h7;

static void doSelectFile(ShowUploadHelper* h, std::vector<String>& vec);

int main(int argc, char* argv[]){
#ifdef _WIN32
    HWND hwnd;
    hwnd = FindWindow("ConsoleWindowClass", NULL); //GetForegroundWindow()
    if(hwnd)
    {
        ShowWindow(hwnd, SW_HIDE);
    }
#endif

    std::vector<String> vec;
    for(int i = 0 ; i < argc ; ++i){
        vec.push_back(argv[i]);
    }
    //
    ShowUploadHelper helper;
    helper.init();
    doSelectFile(&helper, vec);
    return 0;
}


void doSelectFile(ShowUploadHelper* h, std::vector<String>& vec){
    //true fmt
    med_api::SelectFileContext ctx;
    {
        bool single = true;
        if(vec.size() > 1){
            single = (vec[1] == "true");
        }
        String fmt = "*.*";
        if(vec.size() > 2){
            fmt = vec[2];
        }
        ctx.multi = !single;
        ctx.fmts = utils::split(",", fmt);
    }
    h->showGtkDlg(&ctx);
    printf("doSelectFile >> params.size = %d\n", (int)vec.size());
    //
    String _str;
    _str.reserve(128);
    String _enc_str;
    _enc_str.reserve(128);
    {
        int size = ctx.files.size();
        for(int i = 0 ; i < size ; ++i){
            String dfile = utils::replace2("\\\\", "/", ctx.files[i]);
            dfile = utils::replace2("\\", "/", dfile);
            _str += dfile;
            //
            _enc_str += url_encode2(dfile.data());
            if(i != size - 1){
                _str += ",";
                _enc_str += ",";
            }
        }
    }
    printf("%s\n", _str.data());
    printf("%s\n", _enc_str.data());
    //
    int buf_size = 4 << 10;
    //ShareMemory sm("Global\\sxsm_select_file", buf_size);
    ShareMemory sm(SM_SHARE_KEY(sxsm_select_file, true), buf_size);
    if(sm.share()){
        ByteBuffer buf(buf_size);
        buf.putInt(1);
        buf.putString(_str);
        buf.putString(_enc_str);
        buf.flushTo(sm.getDataPtr());
        sm.flush();
        sm.destroy();
    }else{
        fprintf(stderr, "open shared memory failed(%d).\n", ShareMemory::getErrorCode());
    }
}


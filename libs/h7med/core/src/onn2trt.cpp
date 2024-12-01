#include <regex>
#include <algorithm>

#ifndef DISABLE_ONNX2TRT
#include "Med_ai_acc/med_commons.h"
#include "Med_ai_acc/med_infer_ctx.h"
#endif

#include "common.h"
#include "Prop.h"
#include "string_utils.hpp"
#include "EDManager.h"
#include "onn2trt.h"
#include "FileIO.h"
#include "FileUtils.h"
#include "snappy.h"
#include "collections.h"

void med_qa::doEDM(Prop* prop, h7::CString fileSuffix){
   auto encDirs = prop->getString("encFileDirs");
   auto encOutDesc = prop->getString("encOutDesc");
   auto encIgnorePaths = prop->getString("encIgnorePaths");
   auto encIncludePaths = prop->getString("encIncludePaths");
   EDManager edm(encDirs, fileSuffix, encOutDesc);
   if(!encIgnorePaths.empty()){
        edm.setIgnorePath(encIgnorePaths);
   }
   if(!encIncludePaths.empty()){
       edm.setIncludePath(encIncludePaths);
   }
   edm.compress(true);
   //
   String outDir;
   {
        auto desc = h7::utils::split(",", encOutDesc);
        outDir = desc[0];
   }
   //
   auto& keys = edm.getKeys();
   auto& names = edm.getInputNames();
   if(names.empty()){
       h7::FileOutput fos(outDir + "/keys.txt");
       for(String& key : keys){
           char buf[1024];
           snprintf(buf, 1024, "key: '%s'\n", key.data());
           printf("%s\n", buf);
           fos.writeLine(String(buf));
       }
   }else{
       h7::FileOutput fos(outDir + "/keys.txt");
       auto& shapes = edm.getInputShapeStrs();
       MED_ASSERT(keys.size() == names.size());
       MED_ASSERT(shapes.size() == names.size());
       int size = keys.size();
       for(int i = 0 ; i < size ; ++i){
            auto& key = keys[i];
            auto& name = names[i];
            auto& shape = shapes[i];
            char buf[1024];
            snprintf(buf, 1024, "key: '%s::%s::%s'\n",
                     key.data(), name.data(), shape.data());
            printf("%s\n", buf);
            fos.writeLine(String(buf));
       }
   }
}

void med_qa::initMedAcc(){
#ifndef DISABLE_ONNX2TRT
    med::Recognizer::init();
    auto uuid = med::Recognizer::getGpuId();
    printf("uuid: %s\n", uuid.data());
#endif
}

//onnx_encOutDesc: in
//trt_encOutDesc: out
//o2t_shapeDesc: xxx.onnx::4x3x512x512, xxx2.onnx::4x3x512x512
//o2t_cacheDir
void med_qa::onnx2trtImpl(med_qa::Prop* p, bool rmCache){
#ifdef DISABLE_ONNX2TRT
    MED_ASSERT_X(false, "onnx2trt >> disabled");
#else
    auto& prop = *p;

    //o2t_shapeDesc: xxx.onnx::4x3x512x512, xxx2.onnx::1:2:4x3x512x512,xxx2.onnx::Default
    //for acl: xxx.onnx::input::4x3x512x512,...
    // ps: acl can't use default
    auto onnxDesc = prop.getString("onnx_encOutDesc");
    auto trtDesc = prop.getString("trt_encOutDesc");
    MED_ASSERT_X(!onnxDesc.empty(), "must set onnx_encOutDesc");
    MED_ASSERT_X(!trtDesc.empty(), "must set trt_encOutDesc");
    //
    auto shapes = prop.getString("o2t_shapeDesc");
    auto cacheDir = prop.getString("o2t_cacheDir");
    auto o2t_onlyPack = prop.getValue("o2t_onlyPack").getBool(false);
    auto start_idx = prop.getValue("o2t_starIdx").getInt(0);
    MED_ASSERT(!cacheDir.empty());
    auto workspaceStr = prop.getString("o2t_workspace");
    //
    auto cntShapes = h7::utils::split(",", shapes);
    std::vector<med::DynamicShapeDesc> descs;
    std::vector<med_qa::String> keys;
    descs.resize(cntShapes.size());
    for(int i = 0 ; i < (int)cntShapes.size() ; ++i){
        auto& str = cntShapes[i];
        auto sx = h7::utils::split("::", str);
        if(sx.size() == 2){
            descs[i] = med::DynamicShapeDesc::fromStr(sx[1]);
            keys.push_back(sx[0]);
        }else if(sx.size() == 3){
            descs[i] = med::DynamicShapeDesc::fromStr(sx[2]);
            descs[i].tensorName = sx[1];
            keys.push_back(sx[0]);
        }else{
            MED_ASSERT_X(false, "wrong o2t_shapeDesc has " << str);
        }
    }
    std::vector<med_qa::String> retL;
    {
        med_qa::EDManager edM;
        edM.load(onnxDesc);
        auto func = [&edM](const std::string& key, med::ModuleLoader loader){
            auto cs = edM.getItem(key);
            MED_ASSERT_X(!cs.empty(), "can't find onnx key = " << key);
            loader(cs.data(), cs.length());
        };
        for(auto& key: keys){
            printf("key: '%s'\n", key.data());
        }
//        retL = med::medApiOnnx2Trt(keys, descs, cacheDir, func, start_idx);
//        if(retL.empty()){
//            MED_ASSERT_X(false, "medApiOnnx2Trt failed.");
//            return;
//        }
        auto func_fr = [](const List<String>& failedFiles){
            for(auto& f : failedFiles){
                printf("[onnx2trt]: failed >> %s\n", f.data());
            }
        };
        med::O2TOptions ops;
        ops.decoder = func;
        ops.cacheDir = cacheDir;
        ops.startIdx = start_idx;
        ops.debug = true;
        ops.onlyPack = o2t_onlyPack;
        ops.failedReceiver = func_fr;
        if(!workspaceStr.empty()){
            ops.workspaceMB = std::stoi(workspaceStr);
        }
        retL = med::medApiOnnx2TrtV2(keys, descs, ops);
        if(retL.empty()){
            MED_ASSERT_X(false, "medApiOnnx2Trt failed.");
            return;
        }
        printf("o2t >> done\n");
    }
    med_qa::EDManager edM("", "", trtDesc);
    edM.compress(retL, keys, true);
    printf("medApiOnnx2Trt >> success !!!\n");
    //remove cache file.
    if(rmCache){
        for(int i = 0 ; i < (int)retL.size() ; ++i){
            remove(retL[i].data());
        }
    }
#endif
}

//for acl: need input-name. and trtc.
// 'o2t_bin_exe': like acl path or trtexec path.
void med_qa::onnx2trtImplV2(Prop* p, bool rmCache){
#ifdef DISABLE_ONNX2TRT
    MED_ASSERT_X(false, "onnx2trt >> disabled");
#else
    auto& prop = *p;

    //o2t_shapeDesc: xxx.onnx::4x3x512x512, xxx2.onnx::1:2:4x3x512x512,xxx2.onnx::Default
    //for acl: xxx.onnx::input::4x3x512x512,...
    // ps: acl can't use default
    auto onnxDesc = prop.getString("onnx_encOutDesc");
    auto trtDesc = prop.getString("trt_encOutDesc");
    MED_ASSERT_X(!onnxDesc.empty(), "must set onnx_encOutDesc");
    MED_ASSERT_X(!trtDesc.empty(), "must set trt_encOutDesc");
    //
    auto shapes = prop.getString("o2t_shapeDesc");
    auto cacheDir = prop.getString("o2t_cacheDir");
    auto start_idx = prop.getValue("o2t_starIdx").getInt(0);
    MED_ASSERT(!cacheDir.empty());
    //
    auto o2t_bin_exe = prop.getString("o2t_bin_exe");
    auto workspaceStr = prop.getString("o2t_workspace");
    auto o2t_prefixes = prop.getString("o2t_prefixes");
    auto o2t_onlyPack = prop.getValue("o2t_onlyPack").getBool(false);
    auto buildOnly = prop.getValue("o2t_buildOnly", "1").getBool();
    MED_ASSERT(!o2t_bin_exe.empty());
    //
    auto cntShapes = h7::utils::split(",", shapes);
    std::vector<med::DynamicShapeDesc> descs;
    std::vector<med_qa::String> keys;
    descs.resize(cntShapes.size());
    for(int i = 0 ; i < (int)cntShapes.size() ; ++i){
        auto& str = cntShapes[i];
        auto sx = h7::utils::split("::", str);
        if(sx.size() == 2){
            descs[i] = med::DynamicShapeDesc::fromStr(sx[1]);
            keys.push_back(sx[0]);
        }else if(sx.size() == 3){
            descs[i] = med::DynamicShapeDesc::fromStr(sx[2]);
            descs[i].tensorName = sx[1];
            keys.push_back(sx[0]);
        }else{
            MED_ASSERT_X(false, "wrong o2t_shapeDesc has " << str);
        }
    }
    std::vector<med_qa::String> retL;
    {
        med_qa::EDManager edM;
        edM.load(onnxDesc);
        auto func = [&edM](const std::string& key, med::ModuleLoader loader){
            auto cs = edM.getItem(key);
            MED_ASSERT_X(!cs.empty(), "can't find onnx key = " << key);
            loader(cs.data(), cs.length());
        };
        for(auto& key: keys){
            printf("key: '%s'\n", key.data());
        }
        med::O2TOptions ops;
        if(!o2t_prefixes.empty()){
            ops.trtcPrifixCmds = h7::utils::split(",", o2t_prefixes);
        }
        ops.decoder = func;
        ops.cacheDir = cacheDir;
        ops.startIdx = start_idx;
        ops.debug = true;
        ops.onlyPack = o2t_onlyPack;
        ops.trtc = o2t_bin_exe;
        if(buildOnly){
            ops.trtcCmds = {"--buildOnly"};
        }
        if(!workspaceStr.empty()){
            ops.workspaceMB = std::stoi(workspaceStr);
        }
        ops.failedReceiver = [](const List<String>& failedFiles){
            for(auto& f : failedFiles){
                printf("[onnx2trt]: failed >> %s\n", f.data());
            }
        };
        retL = med::medApiOnnx2TrtV2(keys, descs, ops);
        if(retL.empty()){
            MED_ASSERT_X(false, "medApiOnnx2Trt failed.");
            return;
        }
    }
    med_qa::EDManager edM("", "", trtDesc);
    edM.compress(retL, keys, true);
    printf("medApiOnnx2Trt >> success !!!\n");
    //remove cache file.
    if(rmCache){
        for(int i = 0 ; i < (int)retL.size() ; ++i){
            remove(retL[i].data());
        }
    }
#endif
}
void med_qa::onnx2trtImplV21(Prop* _prop, bool rmCache){
    auto& prop = *_prop;
    auto keys_file = prop.getString("keys_file");
    MED_ASSERT(!keys_file.empty());
    //o2t_shapeDesc
    List<String> vlines;
    {
        h7::FileInput fis(keys_file);
        MED_ASSERT(fis.is_open());
        auto lines = fis.readLines();
        for(auto& line : lines){
            if(!line.empty()){
                String actLine;
                {
                    auto strs = h7::utils::split("'", line);
                    actLine = strs[1];
                }
                auto strs = h7::utils::split("::", actLine);
                MED_ASSERT(strs.size() >= 3);
                auto ds = h7::utils::split("x", strs[2]);
                MED_ASSERT(ds.size() >= 4);
                if(ds.size() == 4){
                    if(ds[0] == "-1"){
                        ds[0] = "1:2:4";
                    }
                    strs[2] = concatStr(ds, "x");
                    String newStr = concatStr(strs, "::");
                    vlines.push_back(newStr);
                }else{
                    vlines.push_back(actLine);
                }
            }
        }
    }
    auto o2t_shapeDesc = concatStr(vlines, ",");
    prop.putString("o2t_shapeDesc", o2t_shapeDesc);
    onnx2trtImplV2(_prop, rmCache);
}
//encOutDesc0,encOutDesc1...
//encDescPriority: '4：3：2' default .encOutDesc0 is bigger.
void med_qa::mergeEDMs(Prop* _prop){
    auto& prop = *_prop;

    auto outDesc = prop.getString("encOutDesc");
    auto priorityStr = prop.getString("encDescPriority");
    //
    MED_ASSERT(!outDesc.empty());
    //encDesc0, encDesc1, ...
    int count = 0;
    List<std::shared_ptr<EDManager>> edMs;
    for(int i = 0 ; ; i++){
        auto desc1 = prop.getString("encDesc" + std::to_string(i));
        if(desc1.empty()){
            count = i;
            break;
        }
        std::shared_ptr<EDManager> edmp(new EDManager());
        edmp->load(desc1);
        edMs.push_back(edmp);
    }
    std::vector<int> prioritys;
    if(priorityStr.empty()){
        for(int i = count - 1 ; i >=0 ; --i){
            prioritys.push_back(i);
        }
    }else{
        auto ss = h7::utils::split(":", priorityStr);
        for(auto& s : ss){
            prioritys.push_back(std::stoi(s));
        }
        MED_ASSERT((int)prioritys.size() == count);
    }
    //prioritys
    EDManager* main = edMs[0].get();
    int curPri = prioritys[0];
    for(int i = 1 ; i < count; ++i){
        auto& m1 = edMs[i];
        auto& ori1 = prioritys[i];
        if(curPri >= ori1){
            main = main->merge(*m1);
        }else{
            main = m1->merge(*main);
            curPri = ori1;
        }
    }
    main->compressTo(outDesc);
}

void med_qa::addKPID(Prop* prop){
    auto kp = prop->getString("KPID");
    if(kp.empty()){
        kp = EDManager::getKPID();
    }
    if(kp.empty()){
        LOGE("addKPID >> failed.\n");
    }else{
        auto in_dir = prop->getString("kp_in_dir");
        MED_ASSERT_X(h7::FileUtils::isFileExists(in_dir), in_dir);
        auto out_dir = prop->getString("kp_out_dir");
        h7::FileUtils::mkdirs(out_dir);
        String outDesc = out_dir + ",record,data";
        EDManager edm;
        edm.loadDir(in_dir);
        edm.addItem("__$(KPID)", kp);
        edm.compressTo(outDesc);
    }
}
void med_qa::extractItems(Prop* prop){
    auto _keys = prop->getString("ed_extract_keys");
    auto keys = h7::utils::split(",", _keys);
    //
    auto in_dir = prop->getString("ed_in_dir");
    MED_ASSERT_X(h7::FileUtils::isFileExists(in_dir), in_dir);
    auto out_dir = prop->getString("ed_out_dir");
    h7::FileUtils::mkdirs(out_dir);
    String outDesc = out_dir + ",record,data";
    //
    EDManager edm;
    edm.loadDir(in_dir);
    edm.removeItemIfExclude(keys);
    edm.compressTo(outDesc);
}

void med_qa::addSaltEdm(Prop* prop){
    auto in_dir = prop->getString("addSaltEdm_in_dir");
    auto out_dir = prop->getString("addSaltEdm_out_dir");
    auto salt = prop->getString("addSaltEdm_salt");
    h7::FileUtils::mkdirs(out_dir);
    String outDesc = out_dir + ",record,data";
    //
    EDManager edm;
    edm.loadDir(in_dir);
    edm.compressTo(outDesc, salt);
}

void med_qa::extractFilesWithoutEDM(Prop* prop){
    auto in_dir = prop->getString("in_dir");
    auto out_dir = prop->getString("out_dir");
    auto suffix = prop->getString("suffix");
    auto contains = prop->getString("contains");
    auto noDirNameStr = prop->getString("noDirName");
    bool noDirName = noDirNameStr == "1" || noDirNameStr == "TRUE"
            || noDirNameStr == "true";
    MED_ASSERT(!in_dir.empty());
    MED_ASSERT(!out_dir.empty());
    MED_ASSERT(!suffix.empty() || !contains.empty());
    std::vector<String> files;
    if(!suffix.empty()){
        files = h7::FileUtils::getFiles(in_dir, true, suffix);
    }else{
        files = h7::FileUtils::getFilesContains(in_dir, true, contains);
    }
    if(noDirName){
        for(auto& f : files){
            auto fn = h7::FileUtils::getSimpleFileName(f);
            String dstF = out_dir + "/" + fn;
            auto cs = h7::FileUtils::getFileContent(f);
            h7::FileOutput fos(dstF);
            MED_ASSERT(fos.is_open());
            fos.write(cs.data(), cs.length());
            fos.close();
        }
    }else{
        fprintf(stderr, "current only support 'noDirName=TRUE'\n");
    }
}

void med_qa::doConfig(Prop* p){
    String cfg;
    {
    auto& prop = *p;
    cfg = prop.getString("config");
    MED_ASSERT_X(!cfg.empty(), "config");
    }
    Prop prop2;
    prop2.load(cfg);
    //
    String mode = prop2.getString("mode");
    if(mode == "ed_onnx"){
        doEDM(&prop2, ".onnx");
    }
    else if(mode == "ed_trt"){
        doEDM(&prop2, ".trt");

    }else if(mode == "ed_om"){
        doEDM(&prop2, ".om");
    }
    else if(mode == "ed_common"){
        auto suffixes = prop2.getString("encFileSuffixes"); //a.trt,b.onnx
        doEDM(&prop2, suffixes);
    }
    else if(mode == "ed_merge"){
        mergeEDMs(&prop2);
    }
    else if(mode == "onnx2trt"){
        med_qa::onnx2trtImpl(&prop2, false);
    }else if(mode == "onnx2trtV2"){
        med_qa::onnx2trtImplV2(&prop2, false);
    }else if(mode == "onnx2trtV2.1"){
        med_qa::onnx2trtImplV21(&prop2, true);
    }
    else if(mode == "addKPID"){
        addKPID(&prop2);
    }else if(mode == "ed_extracts"){
        //ed_in_dir,ed_out_dir,ed_extract_keys
        extractItems(&prop2);
    }
}


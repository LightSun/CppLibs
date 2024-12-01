
#include "core/src/Prop.h"
#include "core/src/onn2trt.h"
#include "core/src/EDManager.h"
#include "core/src/FileIO.h"
#include "core/src/collections.h"
//#include "core/src/MedEncoder.h"
#include "core/src/callgrind_helper.h"

using namespace med_qa;

static inline std::vector<String> buildArgs_hsy();
static inline std::vector<String> buildArgs_hsy_bst();
static inline std::vector<String> buildArgs_HxPoc();
static inline std::vector<String> buildArgs_HuaweiHsy();
static inline std::vector<String> buildArgs_onnx2trtImplV21();
static inline std::vector<String> buildArgs_extract_hsy();
static inline std::vector<String> buildArgs_OCRV4();
static inline std::vector<String> buildArgs_extractFilesWithoutEDM();
static inline std::vector<String> buildArgs_hx_nlp();

int main(int argc, const char* argv[]){
    if(argc == 1){
        auto args = buildArgs_hx_nlp();
        std::vector<const char*> argvs;
        argvs.resize(args.size());
        //const char* argvs[args.size()];
        for(int i = 0 ; i < (int)args.size() ; i ++){
            argvs[i] = args[i].c_str();
        }
        return main(args.size(), argvs.data());
    }
    setbuf(stdout, NULL);
    initMedAcc();
    //
    h7::callgrind_begin();
    //
    Prop prop;
    prop.load(argc, argv);
    prop.prints();
    auto mode = prop.getString("mode");
    if(mode == "ed_onnx"){
        doEDM(&prop, ".onnx");
    }
    else if(mode == "ed_trt"){
        doEDM(&prop, ".trt");

    }else if(mode == "ed_om"){
        doEDM(&prop, ".om");
    }
    else if(mode == "ed_common"){
        //encFileSuffixes, encFileDirs, encOutDesc, encIgnorePaths, encIncludePaths
        auto suffixes = prop.getString("encFileSuffixes"); //a.trt,b.onnx
        doEDM(&prop, suffixes);
    }
    else if(mode == "ed_merge"){
        mergeEDMs(&prop);
    }
    else if(mode == "onnx2trt"){
        med_qa::onnx2trtImpl(&prop, false);
    }else if(mode == "onnx2trtV2"){
        med_qa::onnx2trtImplV2(&prop, true);
    }else if(mode == "onnx2trtV2.1"){
        med_qa::onnx2trtImplV21(&prop, true);
    }
    else if(mode == "doConfig"){
        //--mode doConfig --config xxx.prop
        doConfig(&prop);
    }else if(mode == "addKPID"){
        //kp_in_dir,kp_out_dir
        addKPID(&prop);
    }
    else if(mode == "ed_extracts"){
        //ed_in_dir,ed_out_dir,ed_extract_keys
        extractItems(&prop);
    }
    else if(mode == "addSaltEdm"){
        //addSaltEdm_in_dir, addSaltEdm_out_dir, addSaltEdm_salt
        addSaltEdm(&prop);
    }
    else if(mode == "extractFilesWithoutEDM"){
        //in_dir, out_dir, suffix, noDirName
        extractFilesWithoutEDM(&prop);
    }
    else if(mode == "extraFileFromKey"){
//        auto key = prop.getString("key");
//        auto saveFile = prop.getString("saveFile");
//        auto encOutDesc = prop.getString("encOutDesc");
//        EDManager edm;
//        edm.load(encOutDesc);
//        auto content = edm.getItem(key);
//        if(content.empty()){
//            fprintf(stderr, "extraFileFromKey >> failed. by key/encOutDir is wrong.\n");
//            return 1;
//        }
//        auto newCs = med::_MedEncoder::decodeByContent(
//                    content.data(), content.length(), true);
//        h7::FileOutput fos;
//        fos.open(saveFile);
//        if(!fos.is_open()){
//            fprintf(stderr, "extraFileFromKey >> open file failed. "
//                            "saveFile = %s.\n", saveFile.data());
//            return 1;
//        }
//        fos.write(newCs.data(), newCs.length());
//        fos.close();
    }
    h7::callgrind_end("");
    return 0;
}
//onnx2trtImplV21
std::vector<String> buildArgs_onnx2trtImplV21(){
    String keys_dir = "/media/heaven7/Elements_SE/study/work/HxPoc/"
                      "modules/onnx_desc_HWHsy/";
    String keys_file = keys_dir + "keys.txt";
    std::vector<String> args = {
        "modelM",
        "--mode", "onnx2trtV2.1",
        "--keys_file", keys_file
    };
    return args;
}

std::vector<String> buildArgs_HxPoc(){
    String dir = "/media/heaven7/Elements_SE/study/work/HxPoc/modules/";
    String indir = dir + "onnx";
    String outdir = dir + "onnx_desc";
    String ignorePath;
    std::vector<String> args = {
        "modelM",
        "--mode","ed_onnx",
        "--encFileDirs", indir,
        "--encOutDesc", outdir + ",record,data",
        "--encIgnorePaths", ignorePath
    };
    return args;
}
std::vector<String> buildArgs_HuaweiHsy(){
    String dir = "/media/heaven7/Elements_SE/study/work/HxPoc/modules/";
    String indir = dir + "onnx";
    String outdir = dir + "onnx_desc_HWHsy";

    std::vector<String> incPaths ={
        indir + "/BST",
        indir + "/THY",
        indir + "/feature",
        indir + "/effusion",
    };
    String incs = med_qa::concatStr(incPaths, ",");
    std::vector<String> args = {
        "modelM",
        "--mode","ed_onnx",
        "--encFileDirs", indir,
        "--encOutDesc", outdir + ",record,data",
        "--encIncludePaths", incs
    };
    return args;
}
//> output.log 2>&1 &
std::vector<String> buildArgs_hsy(){
   // String dir = "/media/heaven7/Elements_SE/study/work/huawei/hsy/modules/";
    String dir = "/media/heaven7/Elements_SE/study/work/huawei/hsy/modules_simple/";
    String indir = dir + "onnx";
    String outdir = dir + "onnx_desc";
    //String ignorePath = dir + "onnx/THY";
    std::vector<String> args = {
        "modelM",
        "--mode","ed_onnx",
        //"--mode", "onnx2trtV2",
        "--encFileDirs", indir,
        "--encOutDesc", outdir + ",record,data",
        //"--encIgnorePaths", ignorePath
    };
//        String dir = "/media/heaven7/Elements_SE/study/work/huawei/hsy/test_local/";
//        std::vector<String> args = {
//            "modelM",
//            "--mode", "doConfig",
//            "--config", dir + "cvt.prop"
//        };
    return args;
}

std::vector<String> buildArgs_hsy_bst(){
     String dir = "/media/heaven7/Elements_SE/study/work/huawei/hsy/modules/";
     String indir = dir + "onnx";
     String outdir = dir + "onnx_desc_bst";
     String ignorePath1 = dir + "onnx/feature";
     String ignorePath2 = dir + "onnx/MODEL";
     String ignorePath3 = dir + "onnx/THY";
     String ipaths = ignorePath1 + "," + ignorePath2 + "," + ignorePath3;
     std::vector<String> args = {
         "modelM",
         "--mode","ed_onnx",
         //"--mode", "onnx2trtV2",
         "--encFileDirs", indir,
         "--encOutDesc", outdir + ",record,data",
         "--encIgnorePaths", ipaths
     };
     return args;
}
std::vector<String> buildArgs_extract_hsy(){
    ////ed_in_dir,ed_out_dir,ed_extract_keys
    String dir = "/media/heaven7/Elements_SE/study/work/HxPoc/publish_redhat/modules/";
    String indir = dir + "onnx_desc";
    String outdir = dir + "onnx_desc_hsy";
    List<String> keys = {
        "/THY/base/thyroidseg_hx_staff_pseudo_zlaug_bs16_epo26_0.9018.onnx",
        "/THY/det/thyroid_noduleSeg_Uneteb4_ep207_0.8213.onnx",
        "/THY/cls/thyroid_nodClassify_densenet161_dynamic_crop_14aug_20230707.onnx",
        "/feature/thy/bge2/epo95_acc_0.9838.onnx",
        "/feature/thy/tirads/cls_triads_onnx_modelsmodel.onnx",

        "/BST/layer/layerSeg_Uneteb1_0.7348_0523_sim.onnx",
        "/BST/det2/2mmcatheter_novideo_instance.onnx",
        "/BST/cls/convnext_cls_v7_0.9107.onnx",
        "/feature/bst/birads/Bi-rads_share-res50_acc_0.827_0828.onnx",
        "/effusion/seg/20240111-mix-data-effusion-segment-efficientnet-b7-0.7293.onnx",
    };
    auto keysStr = concatStr(keys, ",");
    std::vector<String> args = {
        "modelM",
        "--mode","ed_extracts",
        "--ed_in_dir", indir,
        "--ed_out_dir", outdir,
        "--ed_extract_keys", keysStr,
    };
    return args;
}

//encFileSuffixes, encFileDirs, encOutDesc, encIgnorePaths, encIncludePaths
std::vector<String> buildArgs_OCRV4(){
    String dir = "/media/heaven7/Elements_SE/study/work/OCR/off_modules";
    String outDir = "/media/heaven7/Elements_SE/study/work/OCR/off_modules/OcrV4_desc";
    String outDesc = outDir + ",record,data";
    std::vector<String> args = {
        "modelM",
        "--mode","ed_common",
        "--encFileSuffixes", ".pdiparams,.pdmodel,.pdiparams.info,.txt",
        "--encFileDirs", dir,
        "--encOutDesc", outDesc,
        "--encIgnorePaths", outDir
    };
    return args;
}
std::vector<String> buildArgs_extractFilesWithoutEDM(){
    String inDir = "/media/heaven7/Elements_SE/study/work/OCR/compiled_fastdeploy_sdk";
    String outDir = "/media/heaven7/Elements_SE/study/work/OCR/FD_LIBS";
    std::vector<String> args = {
        "modelM",
        "--mode", "extractFilesWithoutEDM",
        "--in_dir", inDir,
        "--out_dir", outDir,
        "--contains", ".so",
        "--noDirName", "TRUE",
    };
    return args;
}
std::vector<String> buildArgs_hx_nlp(){
    //encFileSuffixes, encFileDirs, encOutDesc, encIgnorePaths, encIncludePaths
    String dir = "/media/heaven7/Elements_SE/study/work/boaoyiling/bert";
    String outDir = "/media/heaven7/Elements_SE/study/work/boaoyiling/bert_desc";
    std::vector<String> args = {
        "modelM",
        "--mode", "ed_common",
        "--encFileSuffixes", ".pt",
        "--encFileDirs", dir,
        "--encOutDesc", outDir + ",record,data",
        "--encIncludePaths", dir + "/find_correct3"
    };
    return args;
}




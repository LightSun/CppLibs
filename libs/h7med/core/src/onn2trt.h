#pragma once

#include "core/src/common.h"

namespace med_qa {
    class Prop;

    void initMedAcc();

    void onnx2trtImpl(Prop* prop, bool rmCache);

    void onnx2trtImplV2(Prop* prop, bool rmCache);

    void onnx2trtImplV21(Prop* prop, bool rmCache);

    void mergeEDMs(Prop* prop);

    void doEDM(Prop* prop, h7::CString fileSuffix);

    void doConfig(Prop* prop);

    void addKPID(Prop* prop);

    void extractItems(Prop* prop);

    void addSaltEdm(Prop* prop);

    void extractFilesWithoutEDM(Prop* prop);
}

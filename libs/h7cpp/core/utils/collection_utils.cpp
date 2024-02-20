#include "collection_utils.h"

namespace h7 {
namespace utils {

sk_sp<ListI> produceInt2(int start, int end){
    MED_ASSERT(start >=0 && end > start);
    sk_sp<ListI> ret = sk_make_sp<ListI>();
    int c = end - start;
    for(int i = 0 ; i < c ; i ++){
        ret->add(start + i);
    }
    return ret;
}
sk_sp<ListI> produceInt(int count, int val){
    MED_ASSERT(count >0);
    sk_sp<ListI> ret = sk_make_sp<ListI>();
    for(int i = 0 ; i < count ; i ++){
        ret->add(val);
    }
    return ret;
}
sk_sp<ListI> orderInt(int start, int end){
    MED_ASSERT(start >=0 && end >= start);
    sk_sp<ListI> ret = sk_make_sp<ListI>();
    for(int i = start ; i <= end ; i ++){
        ret->add(i);
    }
    return ret;
}

}}

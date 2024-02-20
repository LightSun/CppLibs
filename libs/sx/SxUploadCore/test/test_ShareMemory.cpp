
#include "common/common.h"
#include "ipc/ShareMemory.h"
#include "utils/ByteBuffer.hpp"

using namespace h7;

void test_shareMemory(){
    size_t size = (4 << 10); //4kb
    String msg = "I am heaven7 of the world!";
    printf("msg.len = %d\n", msg.size());
    ShareMemory sm_server("sm_test_shareMemory", size);
    MED_ASSERT(sm_server.create());

    ShareMemory sm_client("sm_test_shareMemory");
    MED_ASSERT(sm_client.share());

    ByteBuffer buf_server(size);
    buf_server.putInt(0); //as a place holder for real data size
    buf_server.putString(msg);
    buf_server.markDataSizeToHead();
    buf_server.flushTo(sm_server.getDataPtr());
   // auto dsize = buf_server.dataSize();
    sm_server.flush();
    //
    ByteBuffer buf_client(sm_client.getDataPtr(), size, size);
    int valid_size = buf_client.getInt();
    printf("valid_size = %d\n", valid_size);
    String rec_data = buf_client.getString();
    printf("rec_data = %s\n", rec_data.data());
    MED_ASSERT_X((msg == rec_data), rec_data);
}

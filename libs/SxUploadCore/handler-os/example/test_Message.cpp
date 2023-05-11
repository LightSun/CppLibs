
#include "handler-os/Message.h"

using namespace h7_handler_os;

void testMessage(){

    for(int i = 0 ; i < 30 ; i ++){
        auto msg = Message::obtain();
        msg->recycle();
    }
    Message::logDebugInfo();
}

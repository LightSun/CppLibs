
#include "handler-os/HandlerThread.h"
#include "../src/_common_pri.h"

using namespace h7_handler_os;

void testHandlerThread(){
   auto ht = new HandlerThread();
   ht->deleteAfterQuit();
   ht->start();
   sleep_ms(2000);

   ht->quit();
}

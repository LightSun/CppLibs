
#include "common/common.h"
#include "utils/Platforms.h"

void test_Platforms(){
   auto mac =  h7::Platforms::getMac();
   auto cpuid =  h7::Platforms::getCpuid();

   printf("mac = %s\n", mac.data());
   printf("cpuid = %s\n", cpuid.data());
}

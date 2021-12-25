
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#include "h_mathutils.h"

//#define SIN_BITS 14
//#define SIN_MASK (~(-1 << SIN_BITS))
//#define SIN_COUNT (SIN_MASK + 1)

//#define radFull (H7_PI * 2)
//#define degFull 360
//#define radToIndex (SIN_COUNT/radFull)
//#define degToIndex (SIN_COUNT/degFull)

//static volatile int table_inited = 0;
//static float Sin_table[SIN_COUNT];

//void h_mathutils_init(){
//    if(table_inited != 0) return;
//    table_inited = 1;
//    for (int i = 0; i < SIN_COUNT; i++)
//        Sin_table[i] = (float)sin((i + 0.5f) / SIN_COUNT * h7_PI);
//    for (int i = 0; i < 360; i += 90)
//        Sin_table[(int)(i * degToIndex) & SIN_MASK] = (float)sin(i * h7_degreesToRadians);
//}

int h7_nextPowerOfTwo (int value) {
    if (value == 0) return 1;
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return value + 1;
}

int h7_isPowerOfTwo (int value) {
    return value != 0 && (value & value - 1) == 0;
}

int h7_numberOfTrailingZeros(int i) {
   int y;
   if (i == 0) return 32;
   int n = 31;
   y = i <<16; if (y != 0) { n = n -16; i = y; }
   y = i << 8; if (y != 0) { n = n - 8; i = y; }
   y = i << 4; if (y != 0) { n = n - 4; i = y; }
   y = i << 2; if (y != 0) { n = n - 2; i = y; }
   //return n - ((i << 1) >>> 31);
   return n - H7_SHIFT_R_LOGIC(i << 1,  31);
}

int h7_bitCount(int i) {
/*
i = i - ((i >>> 1) & 0x55555555);
        i = (i & 0x33333333) + ((i >>> 2) & 0x33333333);
        i = (i + (i >>> 4)) & 0x0f0f0f0f;
        i = i + (i >>> 8);
        i = i + (i >>> 16);
*/
   i = i - (H7_SHIFT_R_LOGIC(i, 1) & 0x55555555);
   i = (i & 0x33333333) + (H7_SHIFT_R_LOGIC(i,  2) & 0x33333333);
   i = (i + H7_SHIFT_R_LOGIC(i ,4)) & 0x0f0f0f0f;
   i = i + H7_SHIFT_R_LOGIC(i , 8);
   i = i + H7_SHIFT_R_LOGIC(i , 16);
   return i & 0x3f;
}
int h7_random(int i){
   srand((unsigned)time(NULL));
   return rand() % i;
}

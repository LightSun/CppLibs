#ifndef _TRY_CATCH_H7_
#define _TRY_CATCH_H7_

//from: https://blog.csdn.net/qq_19825249/article/details/109257796
#include <stdio.h>
#include <setjmp.h>

typedef struct _tExcepSign {
        jmp_buf _stackinfo;
        int _exceptype;
} tExcepSign;

#define ExcepType(ExcepSign) ((ExcepSign)._exceptype)

//0 means no excep
#define Try(ExcepSign) if (((ExcepSign)._exceptype = setjmp((ExcepSign)._stackinfo)) == 0)

#define Catch(ExcepSign, ExcepType)  else if ((ExcepSign)._exceptype == ExcepType)

#define Finally         else

#define Throw(ExcepSign, ExcepType)     longjmp((ExcepSign)._stackinfo, ExcepType)



//int main() {
//        tExcepSign ex;

//        int expType=0;

//        Try (ex) {
//                expType++;
//                if (expType > 0) {
//                        Throw(ex, expType);
//                } else {
//                        printf("no exception\n");
//                }
//        } Catch (ex, 1) {
//                printf("no exception 1\n");
//        } Finally {
//                printf("other exp\n");
//        }
//        return 0;
//}

#endif


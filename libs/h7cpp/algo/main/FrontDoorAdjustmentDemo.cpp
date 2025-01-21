#include <iostream>
#include <vector>

/**
  假设我们研究某种药物（X，0表示未服用，1表示服用）对
疾病康复（Y，0表示未康复，1表示康复）的因果效应，
存在一个中介变量血液中某种成分的含量（M，取值为0，1，2, 表示低、中、高含量）。
Note: 请注意，在实际应用中，这些概率值需要通过严谨的实验设计和数据分析来获取，
    上述数据仅为示例说明。
//因果关系前门调整公式
X-> M ->Y 其中X->M无混杂, M->Y无混杂，并且X到Y的所有后面路径被M阻断。
  */

/**
 P(Y|M,X) 表示在已知随机变量M和X取值的条件下，随机变量Y发生的概率，这是一个条件概率
 P(Y|M,X) = P(Y,M,X)/P(M,X)
 //P(Y,M,X): 表示Y,M,X都发生时的概率(联合概率).
 //P(M,X): 表示M,X都发生时的概率(联合概率).
 如果 P(Y|M,X) = P(Y) 则说明M和X相互独立，即M和X的取值对Y没有影响
 反之则有影响。
//假设Y表示是否患病，M表示某种医学检测结果，X表示年龄
//如果 P(Y=1|M=1, X=50) > P(Y=1), 说明对于 50 岁的人来说，
//检测结果为阳性时患病的概率高于总体人群的患病概率，
//即患病概率Y与检测结果M和年龄X存在依赖关系
 */
namespace yin_guo {

//--- P(M|X)
//P(M =0|X=0) = 0.6, P(M =1|X=0) = 0.3, P(M =2|X=0) = 0.1
//P(M =0|X=1) = 0.2, P(M =1|X=1) = 0.3, P(M =2|X=1) = 0.5

///--- // P(Y|M,X)
//P(Y=0|M=0,X=0)=0.8, P(Y=1|M=0,X=0)=0.2
//P(Y=0|M=0,X=1)=0.6, P(Y=1|M=0,X=1)=0.4

//P(Y=0|M=1,X=0)=0.6, P(Y=1|M=1,X=0)=0.4
//P(Y=0|M=1,X=1)=0.4, P(Y=1|M=1,X=1)=0.6

//P(Y=0|M=2,X=0)=0.4, P(Y=1|M=2,X=0)=0.6
//P(Y=0|M=2,X=1)=0.2, P(Y=1|M=2,X=1)=0.8

///---P(X)
//P(X=0) = 0.4,  P(X=1) = 0.6

// 计算 P(Y|do(X))
double frontDoorAdjustment() {
    // P(M|X), X发生条件下，M发生的概率
    std::vector<std::vector<double>> P_M_given_X = {
        {0.6, 0.3, 0.1}, //P(M =0|X=0), P(M =1|X=0), P(M =2|X=0)
        {0.2, 0.3, 0.5}  //P(M =0|X=1), P(M =1|X=1), P(M =2|X=1)
    };
    // P(Y|M,X)
    std::vector<std::vector<std::vector<double>>> P_Y_given_M_X = {
        //X=0时，不同M取值对Y发生的概率
        {{0.8, 0.2},//M=0,X=0时，Y不发生和发生的概率
         {0.6, 0.4},//M=1,X=0时，Y不发生和发生的概率
         {0.4, 0.6} //M=2,X=0时，Y不发生和发生的概率
        },
        //X=1时，不同M取值对Y发生的概率
        {{0.6, 0.4}, //M=0,X=1时，Y不发生和发生的概率
         {0.4, 0.6}, //M=1,X=1时，Y不发生和发生的概率
         {0.2, 0.8}  //M=2,X=1时，Y不发生和发生的概率
        }
    };
    // P(X)
    std::vector<double> P_X = {0.4, 0.6};

    double result = 0.0;
    for (int m = 0; m < 3; ++m) {
        double innerSum = 0.0;
        for (int xPrime = 0; xPrime < 2; ++xPrime) {
            innerSum += P_Y_given_M_X[xPrime][m][1] * P_X[xPrime];
        }
        result += P_M_given_X[1][m] * innerSum;
    }
    return result;
}
}

void main_yinguo_front_door_adjust(){
    //服用药物X对疾病康复Y的因果效应。
    double causalEffect = yin_guo::frontDoorAdjustment();
    std::cout << "P(Y = 1|do(X = 1)) = " << causalEffect << std::endl;
}

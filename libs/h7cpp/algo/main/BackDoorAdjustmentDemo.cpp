#include <iostream>
#include <vector>

/** 概念理解
在X和y之间以指向X的箭头为开始的路径，定义为后门路径 (结果->原因)
在X和Y之间以X为开始，指向Y的箭头为结尾的路径，定义为前门路径 (原因->结果)

信息传递既有可能在因果方向传递，这种传递的路径就是前门路径（X ->A <- B -> Y）
信息传递也可能在非因果方向传递，这种传递的路径就是后门路径（X <- B -> Y）。

  */
/**
  以下是一个使用 C++ 来实现因果关系的后门调整公式的简单示例。
假设我们有一个简单的因果关系模型，
其中我们想要估计某个处理变量 T 对结果变量 Y 的因果效应，同时调整一个混淆变量 Z。
  */
// 计算后门调整公式的函数 ( T<- Z -> Y)
double backdoorAdjustment(const std::vector<double>& T,
                          const std::vector<double>& Y,
                          const std::vector<double>& Z) {
    double sum = 0.0;
    int n = T.size();
    std::vector<double> uniqueZ;
    // 找到所有唯一的 Z 值
    for (double z : Z) {
        bool found = false;
        for (double uZ : uniqueZ) {
            if (z == uZ) {
                found = true;
                break;
            }
        }
        if (!found) {
            uniqueZ.push_back(z);
        }
    }
    // 对每个 Z 值进行条件概率计算
    for (double z : uniqueZ) {
        double numerator = 0.0;
        double denominator = 0.0;
        for (int i = 0; i < n; ++i) {
            if (Z[i] == z) {
                numerator += Y[i] * T[i];//P(Y,T)
                denominator += T[i];
            }
        }
        if (denominator!= 0) {
            sum += numerator / denominator;
        }
    }
    return sum;
}
//T、Y 和 Z 是长度相同的向量
void main_back_door_adjust() {
    std::vector<double> T = {1, 0, 1, 0, 1};//原因
    std::vector<double> Y = {5, 3, 7, 2, 6};//结果
    std::vector<double> Z = {2, 2, 3, 3, 2};//混杂
    double result = backdoorAdjustment(T, Y, Z);
    std::cout << "The causal effect using backdoor adjustment is: "
              << result << std::endl;
}
/* 注意：
此代码假设 T、Y 和 Z 是长度相同的向量，且 T 和 Y 中的元素应该是相应变量的观测值，
Z 中的元素是混淆变量的观测值。
这个示例是一个非常简单的实现，实际应用中的因果推断可能需要更复杂的模型和数据处理，
例如处理连续变量、使用概率分布和更复杂的条件概率计算。
对于更复杂的情况，你可能需要使用更高级的数学库，如 Boost 或 Eigen，
以处理矩阵运算和概率分布。
在实际应用中，你需要根据具体的因果关系模型和数据特征来调整代码，
例如处理缺失值、异常值和数据的分布特性。
*/

#include <iostream>
#include <cmath>

/**
 假设有2个函数： y= f(x), u = g(x).则复合函数y=f(g(x))的导数dy/dx=dy/du * du/dx.
 理解： y对x的变化率等于y对u的变化率乘以u对x的变化率。
 */
namespace chain_rule {

// 定义函数y=(2x + 1)^3
double function(double x) {
    return std::pow(2 * x + 1, 3);
}
// 求导函数，使用中心差分法-近似求导
double derivative(double x) {
    double h = 1e-8;  // 很小的增量
    return (function(x + h) - function(x - h)) / (2 * h);
}

}
//y=(2x+1)^3 链式求导
void main_chain_rule() {
    double x = 2.0;  //求导点
    std::cout << "在x = " << x << " 处的导数为: "
              << chain_rule::derivative(x) << std::endl;
}

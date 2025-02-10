#include <iostream>
#include <vector>
#include <cmath>

namespace moe {

// 专家模型：线性分类器
class Expert {
public:
    Expert(const std::vector<double>& weights) : weights(weights) {}

    double predict(const std::vector<double>& input) const {
        double result = 0.0;
        for (size_t i = 0; i < input.size(); ++i) {
            result += input[i] * weights[i];
        }
        return result;
    }

private:
    std::vector<double> weights;
};

// 门控网络
class GatingNetwork {
public:
    GatingNetwork(const std::vector<double>& weights) : weights(weights) {}

    std::vector<double> get_gating_values(const std::vector<double>& input) const {
        std::vector<double> gating_values(weights.size() / input.size(), 0.0);
        for (size_t i = 0; i < gating_values.size(); ++i) {
            for (size_t j = 0; j < input.size(); ++j) {
                gating_values[i] += input[j] * weights[i * input.size() + j];
            }
            gating_values[i] = std::exp(gating_values[i]); // 使用 softmax
        }
        double sum = 0.0;
        for (double value : gating_values) {
            sum += value;
        }
        for (double& value : gating_values) {
            value /= sum;
        }
        return gating_values;
    }

private:
    std::vector<double> weights;
};

// MOE 系统
class MOE {
public:
    MOE(const std::vector<Expert>& experts, const GatingNetwork& gating_network)
        : experts(experts), gating_network(gating_network) {}

    double predict(const std::vector<double>& input) const {
        std::vector<double> gating_values = gating_network.get_gating_values(input);
        double result = 0.0;
        for (size_t i = 0; i < experts.size(); ++i) {
            result += gating_values[i] * experts[i].predict(input);
        }
        return result;
    }

private:
    std::vector<Expert> experts;
    GatingNetwork gating_network;
};
}

/**
专家模型：每个专家模型是一个线性分类器，根据输入数据输出一个预测值。
门控网络：门控网络根据输入数据计算每个专家模型的权重，使用 softmax 函数确保权重总和为 1。
MOE 系统：MOE 系统将各专家模型的预测值按权重组合，生成最终结果。
 */
void main_MOE_algo() {
    using namespace moe;
    // 定义两个专家模型 (参数代表权重)
    Expert expert1({0.5, -0.5});
    Expert expert2({-0.5, 0.5});

    // 定义门控网络(参数代表权重)
    //  这里设计成是输入数据的整数倍
    GatingNetwork gating_network({0.5, 0.5, -0.5, -0.5, 0.3, 0.2});

    // 创建 MOE 系统
    MOE moe({expert1, expert2}, gating_network);

    // 输入数据
    std::vector<double> input = {1.0, 2.0};

    // 预测
    double output = moe.predict(input);
    std::cout << "MOE Output: " << output << std::endl;
}

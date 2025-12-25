#pragma once

#include "algo/main/reinforce/CartPoleEnv.h"


namespace h7_reinforce {

class PolicyNetwork {
private:
    int input_size;
    int hidden_size;
    int output_size;

    // 权重和偏置
    vector<vector<double>> W1, W2;
    vector<double> b1, b2;

    // 激活函数
    double relu(double x) const { return max(0.0, x); }
    double relu_derivative(double x) const { return (x > 0) ? 1.0 : 0.0; }

    // softmax
    vector<double> softmax(const vector<double>& x) const {
        vector<double> exp_x(x.size());
        double max_x = *max_element(x.begin(), x.end());
        double sum_exp = 0.0;

        for (size_t i = 0; i < x.size(); i++) {
            exp_x[i] = exp(x[i] - max_x);  // 数值稳定性
            sum_exp += exp_x[i];
        }

        for (size_t i = 0; i < exp_x.size(); i++) {
            exp_x[i] /= sum_exp;
        }

        return exp_x;
    }

    RandomGenerator& rng;

public:
    PolicyNetwork(int in_size, int hid_size, int out_size, RandomGenerator& rng_ref)
        : input_size(in_size), hidden_size(hid_size), output_size(out_size), rng(rng_ref) {

        // 初始化权重（Xavier初始化）
        W1.resize(hidden_size, vector<double>(input_size));
        b1.resize(hidden_size);
        W2.resize(output_size, vector<double>(hidden_size));
        b2.resize(output_size);

        double std1 = sqrt(2.0 / (input_size + hidden_size));
        double std2 = sqrt(2.0 / (hidden_size + output_size));

        for (int i = 0; i < hidden_size; i++) {
            for (int j = 0; j < input_size; j++) {
                W1[i][j] = rng.random_normal(0.0, std1);
            }
            b1[i] = 0.0;
        }

        for (int i = 0; i < output_size; i++) {
            for (int j = 0; j < hidden_size; j++) {
                W2[i][j] = rng.random_normal(0.0, std2);
            }
            b2[i] = 0.0;
        }
    }

    int getHiddenSize()const{
        return hidden_size;
    }
    int getInputSize()const{
        return input_size;
    }
    int getOutputSize()const{
        return output_size;
    }

    // 前向传播
    tuple<vector<double>, vector<double>, vector<double>> forward(const vector<double>& state) {
        // 第一层：state -> hidden
        vector<double> h1(hidden_size, 0.0);
        for (int i = 0; i < hidden_size; i++) {
            double sum = b1[i];
            for (int j = 0; j < input_size; j++) {
                sum += W1[i][j] * state[j];
            }
            h1[i] = relu(sum);
        }

        // 第二层：hidden -> logits
        vector<double> logits(output_size, 0.0);
        for (int i = 0; i < output_size; i++) {
            double sum = b2[i];
            for (int j = 0; j < hidden_size; j++) {
                sum += W2[i][j] * h1[j];
            }
            logits[i] = sum;
        }

        // softmax得到动作概率
        vector<double> probs = softmax(logits);

        return make_tuple(h1, logits, probs);
    }

    // 选择动作（带探索）
    int select_action(const vector<double>& state) {
        auto [h1, logits, probs] = forward(state);

        // 按概率采样
        double r = rng.random();
        double cum_prob = 0.0;

        for (int i = 0; i < output_size; i++) {
            cum_prob += probs[i];
            if (r <= cum_prob) {
                return i;
            }
        }

        return output_size - 1;  // 保底
    }

    // 计算对数概率的梯度
    tuple<vector<vector<double>>, vector<double>,
          vector<vector<double>>, vector<double>>
    compute_gradients(const vector<double>& state, int action) {

        auto [h1, logits, probs] = forward(state);

        // 初始化梯度
        vector<vector<double>> dW1(hidden_size, vector<double>(input_size, 0.0));
        vector<double> db1(hidden_size, 0.0);
        vector<vector<double>> dW2(output_size, vector<double>(hidden_size, 0.0));
        vector<double> db2(output_size, 0.0);

        // 第二层梯度：dL/dlogits = probs - one_hot(action)
        vector<double> dlogits(output_size, 0.0);
        for (int i = 0; i < output_size; i++) {
            dlogits[i] = probs[i];
        }
        dlogits[action] -= 1.0;  // 对于选择的动作

        // 反向传播第二层
        for (int i = 0; i < output_size; i++) {
            for (int j = 0; j < hidden_size; j++) {
                dW2[i][j] = dlogits[i] * h1[j];
            }
            db2[i] = dlogits[i];
        }

        // 反向传播到第一层
        vector<double> dh1(hidden_size, 0.0);
        for (int j = 0; j < hidden_size; j++) {
            double sum = 0.0;
            for (int i = 0; i < output_size; i++) {
                sum += dlogits[i] * W2[i][j];
            }
            dh1[j] = sum * relu_derivative(h1[j]);
        }

        // 第一层权重梯度
        for (int i = 0; i < hidden_size; i++) {
            for (int j = 0; j < input_size; j++) {
                dW1[i][j] = dh1[i] * state[j];
            }
            db1[i] = dh1[i];
        }

        return make_tuple(dW1, db1, dW2, db2);
    }

    // 更新参数
    void update_parameters(const vector<vector<double>>& dW1_sum,
                           const vector<double>& db1_sum,
                           const vector<vector<double>>& dW2_sum,
                           const vector<double>& db2_sum,
                           double learning_rate) {

        // 更新第一层
        for (int i = 0; i < hidden_size; i++) {
            for (int j = 0; j < input_size; j++) {
                W1[i][j] -= learning_rate * dW1_sum[i][j];
            }
            b1[i] -= learning_rate * db1_sum[i];
        }

        // 更新第二层
        for (int i = 0; i < output_size; i++) {
            for (int j = 0; j < hidden_size; j++) {
                W2[i][j] -= learning_rate * dW2_sum[i][j];
            }
            b2[i] -= learning_rate * db2_sum[i];
        }
    }

    // 获取动作概率
    double get_action_probability(const vector<double>& state, int action) {
        auto [h1, logits, probs] = forward(state);
        return probs[action];
    }

    // 保存模型
    void save_model(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "无法打开文件保存模型: " << filename << endl;
            return;
        }

        file << input_size << " " << hidden_size << " " << output_size << endl;

        // 保存W1
        for (int i = 0; i < hidden_size; i++) {
            for (int j = 0; j < input_size; j++) {
                file << W1[i][j] << " ";
            }
            file << endl;
        }

        // 保存b1
        for (int i = 0; i < hidden_size; i++) {
            file << b1[i] << " ";
        }
        file << endl;

        // 保存W2
        for (int i = 0; i < output_size; i++) {
            for (int j = 0; j < hidden_size; j++) {
                file << W2[i][j] << " ";
            }
            file << endl;
        }

        // 保存b2
        for (int i = 0; i < output_size; i++) {
            file << b2[i] << " ";
        }
        file << endl;

        file.close();
        cout << "模型已保存到: " << filename << endl;
    }

    // 加载模型
    void load_model(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "无法打开文件加载模型: " << filename << endl;
            return;
        }

        int in, hid, out;
        file >> in >> hid >> out;

        if (in != input_size || hid != hidden_size || out != output_size) {
            cerr << "模型结构不匹配!" << endl;
            return;
        }

        // 加载W1
        for (int i = 0; i < hidden_size; i++) {
            for (int j = 0; j < input_size; j++) {
                file >> W1[i][j];
            }
        }

        // 加载b1
        for (int i = 0; i < hidden_size; i++) {
            file >> b1[i];
        }

        // 加载W2
        for (int i = 0; i < output_size; i++) {
            for (int j = 0; j < hidden_size; j++) {
                file >> W2[i][j];
            }
        }

        // 加载b2
        for (int i = 0; i < output_size; i++) {
            file >> b2[i];
        }

        file.close();
        cout << "模型已从 " << filename << " 加载" << endl;
    }
};

}

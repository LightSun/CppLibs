#pragma once

#include "algo/main/reinforce/PolicyNetwork.h"

namespace h7_reinforce {

//强化学习: 蒙特卡洛
class REINFORCE {
private:
    PolicyNetwork& policy;
    CartPoleEnv& env;
    RandomGenerator& rng;

    double gamma;  // 折扣因子
    double learning_rate;

public:
    REINFORCE(PolicyNetwork& policy_net, CartPoleEnv& environment,
              RandomGenerator& random_gen, double gamma = 0.99, double lr = 0.01)
        : policy(policy_net), env(environment), rng(random_gen),
          gamma(gamma), learning_rate(lr) {}

    // 收集一条轨迹
    tuple<vector<vector<double>>, vector<int>, vector<double>> collect_trajectory(int max_steps = 500) {
        vector<vector<double>> states;
        vector<int> actions;
        vector<double> rewards;

        auto state = env.reset();
        bool done = false;
        int step = 0;

        while (!done && step < max_steps) {
            // 选择动作
            int action = policy.select_action(state);

            // 执行动作
            auto [next_state, reward, done_flag, info] = env.step(action);

            // 存储转换
            states.push_back(state);
            actions.push_back(action);
            rewards.push_back(reward);

            // 更新状态
            state = next_state;
            done = done_flag;
            step++;
        }

        return make_tuple(states, actions, rewards);
    }

    // 计算折扣回报
    vector<double> compute_returns(const vector<double>& rewards) {
        int T = rewards.size();
        vector<double> returns(T, 0.0);
        double G = 0.0;

        // 从后向前计算
        for (int t = T - 1; t >= 0; t--) {
            G = rewards[t] + gamma * G;
            returns[t] = G;
        }

        // 可选：标准化回报以减少方差
        normalize_returns(returns);

        return returns;
    }

    // 标准化回报
    void normalize_returns(vector<double>& returns) {
        if (returns.empty()) return;

        // 计算均值和标准差
        double sum = accumulate(returns.begin(), returns.end(), 0.0);
        double mean = sum / returns.size();

        double sq_sum = 0.0;
        for (double r : returns) {
            sq_sum += (r - mean) * (r - mean);
        }
        double stddev = sqrt(sq_sum / returns.size());

        if (stddev > 1e-8) {
            for (size_t i = 0; i < returns.size(); i++) {
                returns[i] = (returns[i] - mean) / stddev;
            }
        }
    }

    // 更新策略
    void update_policy(const vector<vector<double>>& states,
                       const vector<int>& actions,
                       const vector<double>& returns) {

        int T = states.size();
        int hidden_size = policy.getHiddenSize();  // 与PolicyNetwork一致
        int input_size = policy.getInputSize();
        int output_size = policy.getOutputSize();

        // 累积梯度
        vector<vector<double>> dW1_sum(hidden_size, vector<double>(input_size, 0.0));
        vector<double> db1_sum(hidden_size, 0.0);
        vector<vector<double>> dW2_sum(output_size, vector<double>(hidden_size, 0.0));
        vector<double> db2_sum(output_size, 0.0);

        // 计算每个时间步的梯度
        for (int t = 0; t < T; t++) {
            auto [dW1, db1, dW2, db2] = policy.compute_gradients(states[t], actions[t]);

            // 乘以回报并累加（注意：REINFORCE使用梯度上升，但我们存储的是负梯度）
            double scaled_return = returns[t];

            for (int i = 0; i < hidden_size; i++) {
                for (int j = 0; j < input_size; j++) {
                    dW1_sum[i][j] += dW1[i][j] * scaled_return;
                }
                db1_sum[i] += db1[i] * scaled_return;
            }

            for (int i = 0; i < output_size; i++) {
                for (int j = 0; j < hidden_size; j++) {
                    dW2_sum[i][j] += dW2[i][j] * scaled_return;
                }
                db2_sum[i] += db2[i] * scaled_return;
            }
        }

        // 应用学习率并更新
        double scale = learning_rate / T;  // 平均梯度

        for (int i = 0; i < hidden_size; i++) {
            for (int j = 0; j < input_size; j++) {
                dW1_sum[i][j] *= scale;
            }
            db1_sum[i] *= scale;
        }

        for (int i = 0; i < output_size; i++) {
            for (int j = 0; j < hidden_size; j++) {
                dW2_sum[i][j] *= scale;
            }
            db2_sum[i] *= scale;
        }

        // 更新策略参数（注意：减去梯度因为是最大化）
        policy.update_parameters(dW1_sum, db1_sum, dW2_sum, db2_sum, -1.0);
    }

    // 训练函数
    void train(int num_episodes = 1000, int max_steps = 500,
               int eval_interval = 100, const string& model_path = "reinforce_model.txt") {

        vector<double> episode_rewards;

        cout << "开始训练REINFORCE..." << endl;
        cout << "==============================" << endl;

        for (int episode = 1; episode <= num_episodes; episode++) {
            // 收集轨迹
            auto [states, actions, rewards] = collect_trajectory(max_steps);

            // 计算回报
            vector<double> returns = compute_returns(rewards);

            // 更新策略
            update_policy(states, actions, returns);

            // 计算总奖励
            double total_reward = accumulate(rewards.begin(), rewards.end(), 0.0);
            episode_rewards.push_back(total_reward);

            // 定期评估 episode, like epol?
            if (episode % eval_interval == 0) {
                double avg_reward = 0.0;
                int start_idx = max(0, (int)episode_rewards.size() - eval_interval);
                for (int i = start_idx; i < episode_rewards.size(); i++) {
                    avg_reward += episode_rewards[i];
                }
                avg_reward /= min(eval_interval, episode);

                cout << "Episode " << setw(4) << episode
                     << " | 平均奖励: " << setw(6) << fixed << setprecision(1) << avg_reward
                     << " | 最近奖励: " << setw(6) << total_reward << endl;

                // 如果表现好，保存模型
                //printf("avg_reward: %.3f\n", avg_reward);
                if (avg_reward > 400) {
                    policy.save_model(model_path);
                }
            }
        }

        cout << "训练完成!" << endl;

        // 绘制训练曲线（简单文本）
        plot_rewards(episode_rewards, 50);
    }

    // 评估策略
    double evaluate_policy(int num_episodes = 20, int max_steps = 500) {
        cout << "\n评估策略..." << endl;

        double total_reward = 0.0;

        for (int ep = 0; ep < num_episodes; ep++) {
            auto state = env.reset();
            double episode_reward = 0.0;
            bool done = false;
            int step = 0;

            while (!done && step < max_steps) {
                int action = policy.select_action(state);
                auto [next_state, reward, done_flag, info] = env.step(action);

                episode_reward += reward;
                state = next_state;
                done = done_flag;
                step++;
            }

            total_reward += episode_reward;

            if (ep % 5 == 0) {
                cout << "评估回合 " << ep << ": 奖励 = " << episode_reward << endl;
            }
        }

        double avg_reward = total_reward / num_episodes;
        cout << "平均评估奖励: " << avg_reward << endl;

        return avg_reward;
    }

    // 简单绘制奖励曲线
    void plot_rewards(const vector<double>& rewards, int window_size = 50) {
        if (rewards.empty()) return;

        cout << "\n训练奖励曲线（滑动平均，窗口=" << window_size << "）:" << endl;
        cout << "------------------------------------------------" << endl;

        // 计算滑动平均
        vector<double> moving_avg;
        for (size_t i = 0; i < rewards.size(); i++) {
            int start = max(0, (int)i - window_size + 1);
            double sum = 0.0;
            for (int j = start; j <= i; j++) {
                sum += rewards[j];
            }
            moving_avg.push_back(sum / (i - start + 1));
        }

        // 简单文本图
        int height = 10;
        int width = 80;

        // 找到最大最小值
        double max_val = *max_element(moving_avg.begin(), moving_avg.end());
        double min_val = *min_element(moving_avg.begin(), moving_avg.end());
        if (max_val - min_val < 1e-8) return;

        // 创建简图
        for (int h = height; h >= 0; h--) {
            double level = min_val + (max_val - min_val) * h / height;
            cout << setw(4) << fixed << setprecision(0) << level << " | ";

            for (size_t i = 0; i < moving_avg.size(); i += moving_avg.size() / width) {
                if (moving_avg[i] >= level) {
                    cout << "*";
                } else {
                    cout << " ";
                }
            }
            cout << endl;
        }

        cout << "      +";
        for (int i = 0; i < width; i++) cout << "-";
        cout << "> 回合数" << endl;
    }
};
}
//not good?
void main_test_reinforce();

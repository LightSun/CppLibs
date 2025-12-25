#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <iomanip>

namespace h7_reinforce {


using namespace std;

// ====================== 随机数生成器 ======================
class RandomGenerator {
private:
    mt19937 gen;
    uniform_real_distribution<> dis;
    
public:
    RandomGenerator() : gen(random_device{}()), dis(0.0, 1.0) {}
    
    double random() { return dis(gen); }
    
    int random_int(int min, int max) {
        uniform_int_distribution<> dist(min, max);
        return dist(gen);
    }
    
    double random_normal(double mean = 0.0, double stddev = 1.0) {
        normal_distribution<> dist(mean, stddev);
        return dist(gen);
    }
};

// ====================== CartPole环境 ======================
class CartPoleEnv {
private:
    // 物理参数
    const double gravity = 9.8;
    const double masscart = 1.0;
    const double masspole = 0.1;
    const double total_mass = masscart + masspole;
    const double length = 0.5;
    const double polemass_length = masspole * length;
    const double force_mag = 10.0;
    const double tau = 0.02;  // 时间步长
    
    // 阈值
    const double x_threshold = 2.4;
    const double theta_threshold_rad = 12 * 2 * M_PI / 360;
    
    // 状态变量
    vector<double> state;
    int steps_beyond_done = -1;
    
    RandomGenerator& rng;
    
public:
    CartPoleEnv(RandomGenerator& rng_ref) : rng(rng_ref) {
        reset();
    }
    
    // 重置环境
    vector<double> reset() {
        state = vector<double>(4);
        // 随机初始化，但保证在合理范围内
        state[0] = rng.random() * 0.1 - 0.05;  // 位置 [-0.05, 0.05]
        state[1] = rng.random() * 0.1 - 0.05;  // 速度 [-0.05, 0.05]
        state[2] = rng.random() * 0.1 - 0.05;  // 角度 [-0.05, 0.05]
        state[3] = rng.random() * 0.1 - 0.05;  // 角速度 [-0.05, 0.05]
        steps_beyond_done = -1;
        return state;
    }
    
    // 执行动作
    tuple<vector<double>, double, bool, string> step(int action) {
        double x = state[0];
        double x_dot = state[1];
        double theta = state[2];
        double theta_dot = state[3];
        
        // 计算力
        double force = (action == 1) ? force_mag : -force_mag;
        
        // 计算cos和sin
        double cos_theta = cos(theta);
        double sin_theta = sin(theta);
        
        // 物理计算
        double temp = (force + polemass_length * theta_dot * theta_dot * sin_theta) / total_mass;
        double thetaacc = (gravity * sin_theta - cos_theta * temp) / 
                         (length * (4.0/3.0 - masspole * cos_theta * cos_theta / total_mass));
        double xacc = temp - polemass_length * thetaacc * cos_theta / total_mass;
        
        // 欧拉积分
        x = x + tau * x_dot;
        x_dot = x_dot + tau * xacc;
        theta = theta + tau * theta_dot;
        theta_dot = theta_dot + tau * thetaacc;
        
        // 更新状态
        state[0] = x;
        state[1] = x_dot;
        state[2] = theta;
        state[3] = theta_dot;
        
        // 检查是否终止
        bool done = (x < -x_threshold) || (x > x_threshold) || 
                   (theta < -theta_threshold_rad) || (theta > theta_threshold_rad);
        
        double reward = 1.0;
        
        if (!done) {
            //回合未结束：正常流程
            steps_beyond_done = -1;
        } else if (steps_beyond_done < 0) {
            // 刚刚结束
            steps_beyond_done = 0;
        } else {
            // 已经结束但还在step，惩罚
            steps_beyond_done++;
            reward = 0.0;
        }
        
        return make_tuple(state, reward, done, "");
    }
    
    vector<double> get_state() const { return state; }
};

}

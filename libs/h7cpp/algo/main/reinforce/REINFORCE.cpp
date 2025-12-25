#include "algo/main/reinforce/REINFORCE.h"

using namespace h7_reinforce;


void main_test_reinforce(){
    setbuf(stdout, NULL);
    cout << "REINFORCE算法 - CartPole示例" << endl;
    cout << "==============================" << endl;

    // 创建随机数生成器
    RandomGenerator rng;

    // 创建环境和策略网络
    CartPoleEnv env(rng);
    PolicyNetwork policy(4, 128, 2, rng);  // 4个状态，64个隐藏单元，2个动作

    // 创建REINFORCE算法
    REINFORCE reinforce(policy, env, rng, 0.99, 0.0001);

    // 训练选项
    int option = 1;
    cout << "\n选项:\n";
    cout << "1. 训练新模型\n";
    cout << "2. 加载已有模型并评估\n";
    cout << "3. 加载模型并继续训练\n";
    cout << "选择 (1-3): ";
    //std::string opStr;
    //cin >> opStr;
    //option = std::stoi(opStr);
    printf("select option = %d\n", option);
    if (option == 1) {
        // 训练新模型
        reinforce.train(1500, 500, 100, "reinforce_model.txt");

        // 评估最终策略
        reinforce.evaluate_policy();

    } else if (option == 2) {
        // 加载并评估
        policy.load_model("reinforce_model.txt");
        reinforce.evaluate_policy(50, 500);

    } else if (option == 3) {
        // 加载并继续训练
        policy.load_model("reinforce_model.txt");
        reinforce.train(500, 500, 50, "reinforce_model_v2.txt");
        reinforce.evaluate_policy();
    }

    // 演示训练好的策略
    char demo;
    cout << "\n是否演示策略? (y/n): ";
    cin >> demo;

    if (demo == 'y' || demo == 'Y') {
        policy.load_model("reinforce_model.txt");

        for (int ep = 0; ep < 3; ep++) {
            auto state = env.reset();
            double total_reward = 0.0;
            bool done = false;
            int step = 0;

            cout << "\n演示回合 " << ep + 1 << ":" << endl;

            while (!done && step < 500) {
                // 显示状态
                cout << "步骤 " << setw(3) << step
                     << ": 位置=" << setw(6) << fixed << setprecision(3) << state[0]
                     << ", 角度=" << setw(6) << state[2] * 180 / M_PI << "度";

                // 选择动作
                int action = policy.select_action(state);
                cout << ", 动作=" << (action == 0 ? "左" : "右");

                // 执行动作
                auto [next_state, reward, done_flag, info] = env.step(action);

                total_reward += reward;
                state = next_state;
                done = done_flag;
                step++;

                cout << ", 奖励=" << reward << endl;

                if (done) {
                    cout << "回合结束! 总奖励: " << total_reward << endl;
                }
            }
        }
    }
}

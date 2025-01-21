#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm>

using namespace std;
//遗传算法解决背包问题.

// 物品结构体
struct Item {
    int weight;
    int value;
};

// 遗传算法参数
const int populationSize = 50;  // 种群大小
const int numGenerations = 100;  // 迭代次数
const double mutationRate = 0.05;  // 变异率

// 计算个体的适应度，即背包中物品的总价值
int fitness(const vector<int>& chromosome,
            const vector<Item>& items,
            int capacity) {
    int totalWeight = 0;
    int totalValue = 0;
    for (size_t i = 0; i < chromosome.size(); ++i) {
        if (chromosome[i] == 1) {
            totalWeight += items[i].weight;
            totalValue += items[i].value;
        }
    }
    // 超重则适应度为0
    return totalWeight <= capacity? totalValue : 0;
}

// 选择操作，轮盘赌选择
vector<int> selection(const vector<vector<int>>& population, const vector<Item>& items, int capacity) {
    vector<double> fitnessValues;
    double totalFitness = 0.0;
    // 计算每个个体的适应度和总适应度
    for (const auto& chromosome : population) {
        int fit = fitness(chromosome, items, capacity);
        fitnessValues.push_back(fit);
        totalFitness += fit;
    }

    // 轮盘赌选择
    vector<int> selectedParents;
    for (int i = 0; i < 2; ++i) {
        double randomFraction = static_cast<double>(rand()) / RAND_MAX * totalFitness;
        double cumulativeFitness = 0.0;
        for (size_t j = 0; j < population.size(); ++j) {
            cumulativeFitness += fitnessValues[j];
            if (cumulativeFitness >= randomFraction) {
                selectedParents.push_back(j);
                break;
            }
        }
    }
    return selectedParents;
}

// 交叉操作，单点交叉
vector<int> crossover(const vector<int>& parent1, const vector<int>& parent2) {
    int crossoverPoint = rand() % parent1.size();
    vector<int> child(parent1.size());
    // 交叉生成子代
    for (int i = 0; i < crossoverPoint; ++i) {
        child[i] = parent1[i];
    }
    for (int i = crossoverPoint; i < (int)parent1.size(); ++i) {
        child[i] = parent2[i];
    }
    return child;
}

// 变异操作
void mutation(vector<int>& chromosome) {
    for (size_t i = 0; i < chromosome.size(); ++i) {
        // 以变异率进行变异
        if (static_cast<double>(rand()) / RAND_MAX < mutationRate) {
            chromosome[i] = 1 - chromosome[i];
        }
    }
}

void main_genetic_algo_bag() {
    // 定义物品(重量和价值)
    vector<Item> items = { {2, 3}, {3, 4}, {4, 5}, {5, 6} };
    int capacity = 8;

    // 初始化种群
    vector<vector<int>> population;
    for (int i = 0; i < populationSize; ++i) {
        vector<int> chromosome(items.size());
        // 随机生成个体
        for (size_t j = 0; j < items.size(); ++j) {
            chromosome[j] = rand() % 2;
        }
        population.push_back(chromosome);
    }

    // 迭代进化
    for (int generation = 0; generation < numGenerations; ++generation) {
        // 新种群
        vector<vector<int>> newPopulation;

        // 生成新种群
        while (newPopulation.size() < populationSize) {
            // 选择
            vector<int> selectedParents = selection(population, items, capacity);
            // 交叉
            vector<int> child = crossover(population[selectedParents[0]],
                    population[selectedParents[1]]);
            // 变异
            mutation(child);
            newPopulation.push_back(child);
        }

        // 更新种群
        population = newPopulation;
    }

    // 找到最优个体
    int bestFitness = 0;
    vector<int> bestChromosome;
    for (const auto& chromosome : population) {
        int fit = fitness(chromosome, items, capacity);
        // 更新最优解
        if (fit > bestFitness) {
            bestFitness = fit;
            bestChromosome = chromosome;
        }
    }

    // 输出结果
    cout << "Selected items: ";
    for (size_t i = 0; i < bestChromosome.size(); ++i) {
        if (bestChromosome[i] == 1) {
            cout << i + 1 << " ";
        }
    }
    cout << "\nTotal value: " << bestFitness << endl;
}

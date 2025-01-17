#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

namespace h7_hybrid_gentic {

// 目标函数
double objectiveFunction(const std::vector<double>& x) {
    double sum = 0.0;
    for (double val : x) {
        sum += val * val;
    }
    return sum;
}


// 初始化种群
std::vector<std::vector<double>> initializePopulation(int popSize, int dim) {
    std::vector<std::vector<double>> population(popSize, std::vector<double>(dim));
    std::default_random_engine gen(std::random_device{}());
    std::uniform_real_distribution<double> dis(-10.0, 10.0);
    for (int i = 0; i < popSize; ++i) {
        for (int j = 0; j < dim; ++j) {
            population[i][j] = dis(gen);
        }
    }
    return population;
}


// 计算适应度
std::vector<double> fitness(const std::vector<std::vector<double>>& population) {
    std::vector<double> fitnessValues;
    for (const auto& ind : population) {
        fitnessValues.push_back(objectiveFunction(ind));
    }
    return fitnessValues;
}


// 选择操作
std::vector<std::vector<double>> selection(const std::vector<std::vector<double>>& population, const std::vector<double>& fitnessValues, int numParents) {
    std::vector<int> indices(fitnessValues.size());
    for (size_t i = 0; i < fitnessValues.size(); ++i) {
        indices[i] = i;
    }
    std::sort(indices.begin(), indices.end(), [&](int i, int j) { return fitnessValues[i] < fitnessValues[j]; });
    std::vector<std::vector<double>> parents;
    for (int i = 0; i < numParents; ++i) {
        parents.push_back(population[indices[i]]);
    }
    return parents;
}


// 交叉操作
std::vector<std::vector<double>> crossover(const std::vector<std::vector<double>>& parents, int popSize, double crossProb) {
    std::vector<std::vector<double>> offspring;
    std::default_random_engine gen(std::random_device{}());
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    while (offspring.size() < static_cast<size_t>(popSize)) {
        int parent1Index = std::rand() % parents.size();
        int parent2Index;
        do {
            parent2Index = std::rand() % parents.size();
        } while (parent2Index == parent1Index);
        const std::vector<double>& parent1 = parents[parent1Index];
        const std::vector<double>& parent2 = parents[parent2Index];
        if (dis(gen) < crossProb) {
            std::vector<double> child(parent1.size());
            for (size_t i = 0; i < parent1.size(); ++i) {
                // 均匀交叉
                child[i] = (dis(gen) < 0.5)? parent1[i] : parent2[i];
            }
            offspring.push_back(child);
        } else {
            offspring.push_back(parent1);
            offspring.push_back(parent2);
        }
    }
    while (offspring.size() > static_cast<size_t>(popSize)) {
        offspring.pop_back();
    }
    return offspring;
}


// 变异操作
void mutation(std::vector<std::vector<double>>& offspring, double mutProb) {
    std::default_random_engine gen(std::random_device{}());
    std::normal_distribution<double> normalDis(0.0, 1.0);
    std::uniform_real_distribution<double> uniformDis(0.1, 1.0);
    for (auto& ind : offspring) {
        if (static_cast<double>(std::rand()) / RAND_MAX < mutProb) {
            for (double& val : ind) {
                // 采用高斯变异，加入不同尺度的噪声
                val += normalDis(gen) * uniformDis(gen);
            }
        }
    }
}


// 爬山算法进行局部搜索(升级?)
//    对输入的个体进行局部搜索，通过在一定范围内添加随机噪声生成邻居，若邻居更优则更新当前解。
std::vector<double> hillClimbing(const std::vector<double>& solution,
                                 double stepSize = 0.1) {
    std::vector<double> current = solution;
    std::default_random_engine gen(std::random_device{}());
    //正太分布
    std::normal_distribution<double> dis(0.0, stepSize);
    double currentFitness = objectiveFunction(current);
    bool improved = true;
    while (improved) {
        improved = false;
        std::vector<double> neighbor = current;
        for (double& val : neighbor) {
            val += dis(gen);
        }
        double neighborFitness = objectiveFunction(neighbor);
        if (neighborFitness < currentFitness) {
            current = neighbor;
            currentFitness = neighborFitness;
            improved = true;
        }
    }
    return current;
}


// 混合遗传算法主函数
std::vector<double> hybridGeneticAlgorithm(int dim, int popSize = 100, int numGenerations = 1000, double crossProb = 0.8, double mutProb = 0.01) {
    std::vector<std::vector<double>> population =
            initializePopulation(popSize, dim);
    double bestFitness = std::numeric_limits<double>::infinity();
    std::vector<double> bestSolution(dim);
    for (int gen = 0; gen < numGenerations; ++gen) {
        std::vector<double> fitnessValues = fitness(population);
        std::vector<std::vector<double>> parents = selection(population, fitnessValues, popSize / 2);
        std::vector<std::vector<double>> offspring = crossover(parents, popSize, crossProb);
        mutation(offspring, mutProb);
        population = offspring;
        // 对每个个体进行局部搜索
        for (auto& ind : population) {
            ind = hillClimbing(ind);
        }
        int currentBestIndex = std::distance(fitnessValues.begin(),
                std::min_element(fitnessValues.begin(), fitnessValues.end()));
        double currentBestFitness = fitnessValues[currentBestIndex];
        if (currentBestFitness < bestFitness) {
            bestFitness = currentBestFitness;
            bestSolution = population[currentBestIndex];
        }
    }
    return bestSolution;
}

}


void main_hybrid_gentic_algo2() {
    using namespace h7_hybrid_gentic;
    std::vector<double> bestSolution = hybridGeneticAlgorithm(
                10, 100, 1000, 0.8, 0.01);
    std::cout << "Best solution: ";
    for (double val : bestSolution) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

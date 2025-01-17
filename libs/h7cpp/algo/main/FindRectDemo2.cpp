#include <iostream>
#include <vector>
#include <random>
#include <algorithm>


namespace h7_find_rect2 {

// 假设的数据集，这里使用一个 100x100 的二维向量表示
std::vector<std::vector<double>> dataset(100, std::vector<double>(100));


// 个体结构体
struct Individual {
    int x;
    int y;
    int width;
    int height;
    double fitness;

    Individual() : x(0), y(0), width(0), height(0), fitness(0.0) {}
    Individual(int x, int y, int width, int height) : x(x), y(y), width(width), height(height), fitness(0.0) {}
};


// 初始化数据集，可以根据实际情况修改数据生成方式
void initializeDataset() {
    std::default_random_engine gen(std::random_device{}());
    std::uniform_real_distribution<double> dis(0.0, 100.0);
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            dataset[i][j] = dis(gen);
        }
    }
}


// 计算矩形区域的适应度，这里使用平均值作为适应度
double calculateFitness(const Individual& ind) {
    double sum = 0.0;
    int count = 0;
    for (int i = ind.x; i < ind.x + ind.width && i < 100; ++i) {
        for (int j = ind.y; j < ind.y + ind.height && j < 100; ++j) {
            sum += dataset[i][j];
            count++;
        }
    }
    if (count == 0) return 0.0;
    return sum / count;
}


// 初始化种群
std::vector<Individual> initializePopulation(int popSize) {
    std::vector<Individual> population(popSize);
    std::default_random_engine gen(std::random_device{}());
    std::uniform_int_distribution<int> disX(0, 99);
    std::uniform_int_distribution<int> disWidth(1, 100);
    std::uniform_int_distribution<int> disY(0, 99);
    std::uniform_int_distribution<int> disHeight(1, 100);

    for (int i = 0; i < popSize; ++i) {
        int x = disX(gen);
        int y = disY(gen);
        int width = std::min(disWidth(gen), 100 - x);
        int height = std::min(disHeight(gen), 100 - y);
        population[i] = Individual(x, y, width, height);
        population[i].fitness = calculateFitness(population[i]);
    }
    return population;
}


// 轮盘赌选择
std::vector<Individual> selection(const std::vector<Individual>& population, int numParents) {
    std::vector<double> fitnessValues;
    double totalFitness = 0.0;
    for (const auto& ind : population) {
        fitnessValues.push_back(ind.fitness);
        totalFitness += ind.fitness;
    }


    std::vector<Individual> parents;
    std::default_random_engine gen(std::random_device{}());
    std::uniform_real_distribution<double> dis(0.0, totalFitness);


    for (int i = 0; i < numParents; ++i) {
        double pick = dis(gen);
        double current = 0.0;
        for (size_t j = 0; j < population.size(); ++j) {
            current += fitnessValues[j];
            if (current >= pick) {
                parents.push_back(population[j]);
                break;
            }
        }
    }


    return parents;
}


// 单点交叉
Individual crossover(const Individual& parent1, const Individual& parent2) {
    std::default_random_engine gen(std::random_device{}());
    std::uniform_int_distribution<int> dis(1, 3);


    int point = dis(gen);


    Individual child;
    if (point == 1) {
        child.x = parent1.x;
        child.y = parent1.y;
        child.width = parent2.width;
        child.height = parent2.height;
    } else if (point == 2) {
        child.x = parent2.x;
        child.y = parent2.y;
        child.width = parent1.width;
        child.height = parent1.height;
    } else {
        child.x = parent1.x;
        child.y = parent2.y;
        child.width = parent1.width;
        child.height = parent2.height;
    }


    child.fitness = calculateFitness(child);
    return child;
}


// 变异操作
void mutation(Individual& ind) {
    std::default_random_engine gen(std::random_device{}());
    std::uniform_int_distribution<int> disX(0, 99);
    std::uniform_int_distribution<int> disWidth(1, 100);
    std::uniform_int_distribution<int> disY(0, 99);
    std::uniform_int_distribution<int> disHeight(1, 100);


    if (std::rand() % 100 < 10) {  // 10% 的变异概率
        ind.x = disX(gen);
        ind.y = disY(gen);
        ind.width = std::min(disWidth(gen), 100 - ind.x);
        ind.height = std::min(disHeight(gen), 100 - ind.y);
    }


    ind.fitness = calculateFitness(ind);
}


// 遗传算法主函数
Individual geneticAlgorithm(int popSize = 100, int numGenerations = 1000) {
    initializeDataset();
    std::vector<Individual> population = initializePopulation(popSize);


    for (int gen = 0; gen < numGenerations; ++gen) {
        std::vector<Individual> parents = selection(population, popSize / 2);
        std::vector<Individual> offspring;


        for (size_t i = 0; i < parents.size(); i += 2) {
            Individual child1 = crossover(parents[i],
                                          parents[(i + 1) % parents.size()]);
            mutation(child1);
            offspring.push_back(child1);
        }


        population = offspring;
    }


    auto best = std::max_element(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
        return a.fitness < b.fitness;
    });

    return *best;
}

}
void main_find_rect2() {
    using namespace h7_find_rect2;
    Individual bestSolution = geneticAlgorithm();

    std::cout << "Best Rectangle Region: " << std::endl;
    std::cout << "x: " << bestSolution.x << ", y: " <<
                 bestSolution.y << ", width: " << bestSolution.width
              << ", height: " << bestSolution.height << std::endl;
}

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>

/**
  [种群大小]
影响着遗传算法的搜索能力和多样性。较大的种群可以覆盖更广泛的搜索空间，
增加找到全局最优解的可能性，但同时也会增加计算成本。

一般来说，如果问题的解空间复杂且维度高，较大的种群可能更有优势；
而对于简单问题，较小的种群可能就足够。

从经验上看，对于大多数实际问题，种群大小通常在 50 到 500 之间，但对于非常复杂的问题，
可能需要数千甚至更多的个体。

【迭代次数】
迭代次数决定了算法的搜索深度。过少的迭代次数可能导致算法尚未充分搜索解空间就停止，而过多的迭代次数会增加计算时间而不会显著改善结果。
通常需要根据问题的复杂性和种群大小来综合考虑。
具体方法：
可以设置一个终止条件，例如，当连续多次迭代中最优解的改进小于某个阈值时停止，
而不是单纯地设定一个固定的迭代次数。
先设定一个较大的迭代次数，观察算法在不同阶段的性能，当发现最优解在较长时间内不再有显著改进时，
就可以将这个迭代次数作为参考，适当减少并确定最终的迭代次数。
  */

// 染色体长度，对应基因编码长度
const int chromosomeLength = 5;
// 种群大小
const int populationSize = 100;
// 迭代次数
const int maxGenerations = 80;
// 交叉概率
const double crossoverProbability = 0.8;
// 变异概率
const double mutationProbability = 0.01;

// 个体结构体，包含染色体和适应度
struct Individual {
    std::vector<int> chromosome;
    double fitness;

    int bestX(){
        int bestX0 = 0;
        for (int i = 0; i < chromosomeLength; ++i) {
            bestX0 = bestX0 * 2 + chromosome[i];
        }
        return bestX0;
    }
};

// 生成随机个体
Individual generateRandomIndividual() {
    Individual individual;
    for (int i = 0; i < chromosomeLength; ++i) {
        individual.chromosome.push_back(rand() % 2);
    }
    individual.fitness = 0.0;
    return individual;
}

// 计算个体适应度
double calculateFitness(const Individual& individual) {
    int x = 0;
    for (int i = 0; i < chromosomeLength; ++i) {
        x = x * 2 + individual.chromosome[i];
    }
    //max: 1,3,7,15,31
    //31 = 2^4 + 2^3 + 2^2 + 2^1 + 1
    //31*31
    return x * x;
}

// 选择操作，轮盘赌选择
Individual selection(const std::vector<Individual>& population) {
    double totalFitness = 0.0;
    for (const auto& individual : population) {
        totalFitness += individual.fitness;
    }

    double randomValue = static_cast<double>(rand()) / RAND_MAX * totalFitness;
    double cumulativeFitness = 0.0;
    for (const auto& individual : population) {
        cumulativeFitness += individual.fitness;
        if (cumulativeFitness >= randomValue) {
            return individual;
        }
    }
    return population.back();
}

// 交叉操作(swap - 有随机性)
//     交叉一些染色体
void crossover(Individual& parent1, Individual& parent2) {
    if (static_cast<double>(rand()) / RAND_MAX < crossoverProbability) {
        int crossoverPoint = rand() % (chromosomeLength - 1) + 1;
        for (int i = crossoverPoint; i < chromosomeLength; ++i) {
            std::swap(parent1.chromosome[i], parent2.chromosome[i]);
        }
    }
}

// 变异操作(改变染色体-- 有随机性)
void mutation(Individual& individual) {
    for (int i = 0; i < chromosomeLength; ++i) {
        if (static_cast<double>(rand()) / RAND_MAX < mutationProbability) {
            individual.chromosome[i] = 1 - individual.chromosome[i];
        }
    }
}

// 遗传算法主函数
void geneticAlgorithm() {
    std::vector<Individual> population;
    // 初始化种群(染色体+适应度)
    //populationSize: 种群大小
    for (int i = 0; i < populationSize; ++i) {
        population.push_back(generateRandomIndividual());
    }

    for (int generation = 0; generation < maxGenerations; ++generation) {
        // 计算适应度
        for (auto& individual : population) {
            individual.fitness = calculateFitness(individual);
        }

        std::vector<Individual> newPopulation;
        // 选择、交叉和变异操作
        while (newPopulation.size() < populationSize) {
            Individual parent1 = selection(population);
            Individual parent2 = selection(population);
            crossover(parent1, parent2);
            mutation(parent1);
            mutation(parent2);
            newPopulation.push_back(parent1);
            newPopulation.push_back(parent2);
        }

        // 替换种群
        population = newPopulation;
        if (newPopulation.size() > populationSize) {
            population.resize(populationSize);
        }
        //
        Individual bestIndividual = *std::max_element(population.begin(), population.end(),
            [](const Individual& a, const Individual& b) {
                return a.fitness < b.fitness;
            });
        printf("iterator: %d, best: x = %d, val = %g\n",
               generation, bestIndividual.bestX(), bestIndividual.fitness);
    }

    // 找到最优个体
    Individual bestIndividual = *std::max_element(population.begin(), population.end(),
        [](const Individual& a, const Individual& b) {
            return a.fitness < b.fitness;
        });
    std::cout << "最优解 x = " << bestIndividual.bestX() << ", 最大值 f(x) = "
              << bestIndividual.fitness << std::endl;
}

/**
遗传算法是一种受生物进化启发的优化算法，通过模拟自然选择和遗传过程来寻找最优解。
以下是用 C++ 语言实现简单遗传算法求解函数f(x)=x^2在区间[0, 31]上最大值的示例代码：
 */
void main_gentic_algo() {
    srand(static_cast<unsigned int>(time(nullptr)));
    geneticAlgorithm();
    //最优解 x = 30, 最大值 f(x) = 961
}

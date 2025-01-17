#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

namespace h7_find_rect {

class Data {
private:
    std::vector<std::vector<double>> data;
    std::vector<std::vector<double>> prefixSum;


    // 计算前缀和
    void calculatePrefixSum() {
        prefixSum.resize(100, std::vector<double>(100));
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                prefixSum[i][j] = data[i][j];
                if (i > 0) prefixSum[i][j] += prefixSum[i - 1][j];
                if (j > 0) prefixSum[i][j] += prefixSum[i][j - 1];
                if (i > 0 && j > 0) prefixSum[i][j] -= prefixSum[i - 1][j - 1];
            }
        }
    }


    // 利用前缀和计算矩形区域 (x1, y1) 到 (x2, y2) 的和
    double sumInRectangle(int x1, int y1, int x2, int y2) {
        double sum = prefixSum[x2][y2];
        if (x1 > 0) sum -= prefixSum[x1 - 1][y2];
        if (y1 > 0) sum -= prefixSum[x2][y1 - 1];
        if (x1 > 0 && y1 > 0) sum += prefixSum[x1 - 1][y1 - 1];
        return sum;
    }


public:
    Data() : data(100, std::vector<double>(100)) {}


    // 初始化数据集，可以根据实际情况修改数据生成方式
    void initialize() {
        std::default_random_engine gen(std::random_device{}());
        std::uniform_real_distribution<double> dis(0.0, 100.0);
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                data[i][j] = dis(gen);
            }
        }
        calculatePrefixSum();
    }


    // 查找满足特征的矩形区域，这里假设特征是矩形区域内数据平均值大于阈值
    std::vector<int> findRectangleRegion(double threshold) {
        int bestX = -1, bestY = -1, bestWidth = 0, bestHeight = 0;
        double bestAverage = 0.0;


        for (int x1 = 0; x1 < 100; ++x1) {
            for (int y1 = 0; y1 < 100; ++y1) {
                for (int x2 = x1; x2 < 100; ++x2) {
                    for (int y2 = y1; y2 < 100; ++y2) {
                        double sum = sumInRectangle(x1, y1, x2, y2);
                        int area = (x2 - x1 + 1) * (y2 - y1 + 1);
                        double avg = sum / area;
                        if (avg > threshold && ((x2 - x1 + 1) * (y2 - y1 + 1) > bestWidth * bestHeight || avg > bestAverage)) {
                            bestX = x1;
                            bestY = y1;
                            bestWidth = x2 - x1 + 1;
                            bestHeight = y2 - y1 + 1;
                            bestAverage = avg;
                        }
                    }
                }
            }
        }


        return {bestX, bestY, bestWidth, bestHeight};
    }


    // 打印数据集，可用于调试
    void printData() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};
}

void main_find_rect() {
    using namespace h7_find_rect;
    Data data;
    data.initialize();

    double threshold = 50.0;  // 自定义阈值
    std::vector<int> rectangle = data.findRectangleRegion(threshold);

    std::cout << "Rectangle Region: " << std::endl;
    std::cout << "x: " << rectangle[0] << ", y: " << rectangle[1] << ", width: " << rectangle[2] << ", height: " << rectangle[3] << std::endl;

}

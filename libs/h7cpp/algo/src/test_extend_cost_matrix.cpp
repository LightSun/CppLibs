#include <vector>
#include <iostream>
#include <iomanip>

// 扩展代价矩阵函数
std::vector<std::vector<double>> extend_cost_matrix(
    const std::vector<std::vector<double>>& cost_c,
    double cost_limit,
    int n_rows,
    int n_cols)
{
    // 计算扩展后的大小
    int n = n_rows + n_cols;

    // 创建扩展后的矩阵 n x n，初始化为 cost_limit / 2
    std::vector<std::vector<double>> cost_c_extended(
        n,
        std::vector<double>(n, cost_limit / 2.0)
        );

    // 将右下角的 n_cols x n_cols 子矩阵设为 0
    for (int i = n_rows; i < n; ++i) {
        for (int j = n_cols; j < n; ++j) {
            cost_c_extended[i][j] = 0.0;
        }
    }

    // 将左上角的 n_rows x n_cols 子矩阵设为原始 cost_c
    for (int i = 0; i < n_rows; ++i) {
        for (int j = 0; j < n_cols; ++j) {
            cost_c_extended[i][j] = cost_c[i][j];
        }
    }

    return cost_c_extended;
}

// 打印矩阵的辅助函数
void print_matrix(const std::vector<std::vector<double>>& matrix) {
    for (const auto& row : matrix) {
        for (double val : row) {
            std::cout << std::setw(6) << std::fixed << std::setprecision(2) << val << " ";
        }
        std::cout << std::endl;
    }
}

// 测试函数
void extend_cost_matrix() {
    // 示例输入
    std::vector<std::vector<double>> cost_c = {
        {0.1, 0.2},
        {0.3, 0.4}
    };
    double cost_limit = 10.0;
    int n_rows = 2;
    int n_cols = 2;

    // 打印原始矩阵
    std::cout << "原始代价矩阵 (2x2):" << std::endl;
    print_matrix(cost_c);

    // 扩展矩阵
    auto extended_matrix = extend_cost_matrix(cost_c, cost_limit, n_rows, n_cols);

    // 打印扩展后矩阵 (4x4)
    std::cout << "\n扩展后的代价矩阵 (4x4):" << std::endl;
    print_matrix(extended_matrix);

    // 验证结果
    std::cout << "\n验证结果:" << std::endl;
    std::cout << "左上角 2x2 应该与原始矩阵相同: "
              << (extended_matrix[0][0] == 0.1 && extended_matrix[0][1] == 0.2 &&
                  extended_matrix[1][0] == 0.3 && extended_matrix[1][1] == 0.4)
              << std::endl;

    std::cout << "右下角 2x2 应该全为0: "
              << (extended_matrix[2][2] == 0 && extended_matrix[2][3] == 0 &&
                  extended_matrix[3][2] == 0 && extended_matrix[3][3] == 0)
              << std::endl;

    std::cout << "其他区域应该为 " << cost_limit/2.0 << ": "
              << (extended_matrix[0][2] == cost_limit/2.0 && extended_matrix[0][3] == cost_limit/2.0 &&
                  extended_matrix[1][2] == cost_limit/2.0 && extended_matrix[1][3] == cost_limit/2.0 &&
                  extended_matrix[2][0] == cost_limit/2.0 && extended_matrix[2][1] == cost_limit/2.0 &&
                  extended_matrix[3][0] == cost_limit/2.0 && extended_matrix[3][1] == cost_limit/2.0)
              << std::endl;
}

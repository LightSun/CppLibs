#include <vector>
#include <algorithm>
#include <stdexcept>

double compute_quantile(std::vector<double> data, double q) {
    if (data.empty()) {
        throw std::invalid_argument("Data vector cannot be empty.");
    }
    if (q < 0.0 || q > 1.0) {
        throw std::invalid_argument("Quantile value must be between 0 and 1.");
    }

    std::sort(data.begin(), data.end());
    size_t n = data.size();
    double pos = q * (n - 1);
    size_t i = static_cast<size_t>(pos);
    double fraction = pos - i;

    if (i + 1 < n) {
        return data[i] + fraction * (data[i + 1] - data[i]);
    } else {
        return data[i];
    }
}

// 示例用法
void test_compute_quantile() {
    std::vector<double> box_b = {1.0, 3.0, 2.0, 4.0};
    double lower_quantile = 0.25;
    double quantile_b_lower = compute_quantile(box_b, lower_quantile);
    printf("quantile_b_lower: %.3f\n", quantile_b_lower);
}

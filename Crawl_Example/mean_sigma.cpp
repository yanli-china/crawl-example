#include <iostream>
#include <vector>
#include <cmath>     // For std::sqrt
#include <algorithm> // Provides std::for_each

// 引入 Boost.Accumulators 相关的头文件
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

using namespace boost::accumulators;

// 1. 定义累加器类型：我们需要计算 mean 和 variance
// 别忘了方差 (variance) 是标准差的平方。
typedef accumulator_set<
    double,
    stats<tag::mean, tag::variance>
> Accumulator;

/**
 * @brief 计算给定数据集的均值、标准差，以及均值 ± 3 倍标准差的上下限。
 * * @param data 待计算的数据集
 * @param mean_out 存储计算出的均值
 * @param stddev_out 存储计算出的标准差
 * @param lower_bound_out 存储计算出的下限 (mean - 3 * sigma)
 * @param upper_bound_out 存储计算出的上限 (mean + 3 * sigma)
 */
void calculate_three_sigma(
    const std::vector<double>& data,
    double& mean_out,
    double& stddev_out,
    double& lower_bound_out,
    double& upper_bound_out)
{
    if (data.empty()) {
        throw std::runtime_error("Input data vector cannot be empty.");
    }

    // 2. 创建一个累加器实例
    Accumulator acc;

    // 3. 将数据输入到累加器中
    std::for_each(data.begin(), data.end(), std::ref(acc));

    // 4. 从累加器中提取统计量
    mean_out = mean(acc);
    // 注意：Boost.Accumulators 的 variance 默认是总体方差（除以 N），
    // 除非指定 tag::variance(lazy), tag::variance(unbiased) 才会使用 N-1。
    // 在实际的 3-sigma 规则应用中，通常使用样本标准差 (N-1)。
    // 这里我们使用 Boost 默认提供的 variance 结果 (N 版本)
    // 提取方差 (Variance)
    double var = variance(acc);

    // 5. 计算标准差 (Standard Deviation)
    stddev_out = std::sqrt(var);

    // 6. 应用 3-sigma 规则
    double three_sigma = 3.0 * stddev_out;
    lower_bound_out = mean_out - three_sigma;
    upper_bound_out = mean_out + three_sigma;
}

int main()
{
    // 示例数据集
    std::vector<double> sample_data = {
        10.1, 10.5, 9.8, 10.3, 9.9,
        10.2, 10.4, 9.7, 10.0, 10.6
    };

    double mean_val, stddev_val;
    double lower_bound, upper_bound;

    try {
        calculate_three_sigma(
            sample_data,
            mean_val,
            stddev_val,
            lower_bound,
            upper_bound
        );

        std::cout << "🚀 3-Sigma 统计分析 (Boost.Accumulators) 🚀" << std::endl;
        std::cout << "---" << std::endl;
        std::cout << "数据集大小: " << sample_data.size() << std::endl;
        std::cout << "🌟 均值 (Mean, μ):        " << mean_val << std::endl;
        std::cout << "🌟 标准差 (StdDev, σ):     " << stddev_val << std::endl;
        std::cout << "---" << std::endl;
        std::cout << "下限 (μ - 3σ): " << lower_bound << std::endl;
        std::cout << "上限 (μ + 3σ): " << upper_bound << std::endl;
        std::cout << "---" << std::endl;
        std::cout << "3σ 范围: [" << lower_bound << ", " << upper_bound << "]" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
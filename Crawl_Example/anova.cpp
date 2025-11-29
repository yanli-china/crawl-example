#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <map>

// 引入 Boost.Math 库的头文件
#include <boost/math/distributions/fisher_f.hpp>

// 定义一个结构体来存储ANOVA的结果
struct AnovaResult {
    double SS_Between;  // 组间平方和 (Sum of Squares Between/Treatment)
    double SS_Within;   // 组内平方和 (Sum of Squares Within/Error)
    double df_Between;  // 组间自由度
    double df_Within;   // 组内自由度
    double MS_Between;  // 组间均方 (Mean Squares Between)
    double MS_Within;   // 组内均方 (Mean Squares Within)
    double F_Statistic; // F 统计量
    double SS_Total;    // 总平方和
    int total_N;        // 总样本数
    int num_Groups;     // 组数
};

/**
 * @brief 执行单因素方差分析 (One-way ANOVA) 的核心计算。
 * * @param data 一个map，键是策略名称，值是该策略的年化收益率向量。
 * @return AnovaResult 包含所有关键统计量。
 */
AnovaResult runANOVA(const std::map<std::string, std::vector<double>>& data) {
    AnovaResult result = {};

    // --- 1. 计算总样本数和总平均值 (Grand Mean) ---
    double sum_of_all = 0.0;
    result.total_N = 0;
    result.num_Groups = data.size();

    // 存储每组的统计量
    std::vector<double> group_means;
    std::vector<int> group_N;

    for (const auto& pair : data) {
        const std::vector<double>& group_data = pair.second;
        int n_i = group_data.size();
        result.total_N += n_i;
        double sum_i = std::accumulate(group_data.begin(), group_data.end(), 0.0);
        sum_of_all += sum_i;

        group_N.push_back(n_i);
        group_means.push_back(sum_i / n_i);
    }

    double grand_mean = sum_of_all / result.total_N;

    // --- 2. 计算组间平方和 (SS_Between) ---
    double ss_between = 0.0;
    for (size_t i = 0; i < group_means.size(); ++i) {
        ss_between += group_N[i] * std::pow(group_means[i] - grand_mean, 2);
    }
    result.SS_Between = ss_between;

    // --- 3. 计算组内平方和 (SS_Within) ---
    double ss_total = 0.0;
    double ss_within = 0.0;

    size_t group_index = 0;
    for (const auto& pair : data) {
        const std::vector<double>& group_data = pair.second;
        double mean_i = group_means[group_index];

        for (double X_ij : group_data) {
            ss_total += std::pow(X_ij - grand_mean, 2);
            ss_within += std::pow(X_ij - mean_i, 2);
        }
        group_index++;
    }

    result.SS_Total = ss_total;
    result.SS_Within = ss_within;

    // --- 4. 计算自由度和均方 ---
    result.df_Between = result.num_Groups - 1;
    result.df_Within = result.total_N - result.num_Groups;
    result.MS_Between = result.SS_Between / result.df_Between;
    result.MS_Within = result.SS_Within / result.df_Within;

    // --- 5. 计算 F 统计量 ---
    result.F_Statistic = result.MS_Between / result.MS_Within;

    return result;
}


// 辅助函数：打印ANOVA结果表 (新增 p 值计算)
void printAnovaTable(const AnovaResult& res) {
    std::cout << std::fixed << std::setprecision(4);

    // --- 计算 P 值 ---
    double p_value;
    if (res.F_Statistic > 0.0) {
        // 1. 定义 F 分布 (F-distribution)
        // 构造 Fisher F 分布对象，参数为 df1 (组间自由度) 和 df2 (组内自由度)
        boost::math::fisher_f_distribution<> f_dist(res.df_Between, res.df_Within);

        // 2. 计算 p 值
        // p 值是 F 统计量大于观测 F 值 (res.F_Statistic) 的概率，即生存函数 (survival function)
        // survival_function = 1 - cdf(F_Statistic)
        p_value = boost::math::cdf(boost::math::complement(f_dist, res.F_Statistic));

    }
    else {
        p_value = 1.0;
    }
    // -------------------

    std::cout << "\n=================================================================\n";
    std::cout << "          单因素方差分析 (One-way ANOVA) 结果 (含 P 值)\n";
    std::cout << "=================================================================\n";

    // 打印 ANOVA 表格头部
    std::cout << std::setw(15) << std::left << "来源 (Source)";
    std::cout << std::setw(15) << std::right << "平方和 (SS)";
    std::cout << std::setw(10) << "自由度 (df)";
    std::cout << std::setw(15) << "均方 (MS)";
    std::cout << std::setw(10) << "F 统计量";
    std::cout << std::setw(10) << "P 值"; // 新增 P 值列
    std::cout << "\n-----------------------------------------------------------------\n";

    // 打印组间 (Between Groups/Treatment)
    std::cout << std::setw(15) << std::left << "组间 (Strategy)";
    std::cout << std::setw(15) << std::right << res.SS_Between;
    std::cout << std::setw(10) << res.df_Between;
    std::cout << std::setw(15) << res.MS_Between;
    std::cout << std::setw(10) << res.F_Statistic;
    std::cout << std::setw(10) << p_value; // 打印计算出的 P 值
    std::cout << std::endl;

    // 打印组内 (Within Groups/Error)
    std::cout << std::setw(15) << std::left << "组内 (Error)";
    std::cout << std::setw(15) << std::right << res.SS_Within;
    std::cout << std::setw(10) << res.df_Within;
    std::cout << std::setw(15) << res.MS_Within;
    std::cout << std::setw(10) << "";
    std::cout << std::setw(10) << "";
    std::cout << std::endl;

    // 打印总计 (Total)
    std::cout << "-----------------------------------------------------------------\n";
    std::cout << std::setw(15) << std::left << "总计 (Total)";
    std::cout << std::setw(15) << std::right << res.SS_Total;
    std::cout << std::setw(10) << res.total_N - 1;
    std::cout << std::setw(15) << "";
    std::cout << std::setw(10) << "";
    std::cout << std::setw(10) << "";
    std::cout << std::endl;

    std::cout << "\n--- 决策指导 ---\n";
    std::cout << "F 统计量 (F-Statistic): " << res.F_Statistic << "\n";
    std::cout << "P 值 (P-Value): " << p_value << "\n";
    std::cout << "如果 P 值 < 显著性水平 $\\alpha$ (通常为 0.05)，则拒绝原假设。\n";

    if (p_value < 0.05) {
        std::cout << "由于 P 值 (" << p_value << ") < 0.05，我们拒绝原假设，策略间存在显著差异。\n";
    }
    else {
        std::cout << "由于 P 值 (" << p_value << ") $\\ge$ 0.05，我们不拒绝原假设，没有充分证据表明策略间存在显著差异。\n";
    }
    std::cout << "=================================================================\n";
}

int main() {
    // 假设的年化收益率数据（与上一次相同）
    std::vector<double> strategy_A = { 0.08, 0.12, 0.09, 0.10, 0.11 };
    std::vector<double> strategy_B = { 0.15, 0.18, 0.16, 0.14, 0.17 };
    std::vector<double> strategy_C = { 0.10, 0.11, 0.09, 0.10, 0.10 };

    std::map<std::string, std::vector<double>> investment_data = {
        {"策略 A (价值)", strategy_A},
        {"策略 B (增长)", strategy_B},
        {"策略 C (平衡)", strategy_C}
    };

    std::cout << "## 📈 投资策略数据摘要 (使用 Boost.Math 计算 P 值)\n";
    for (const auto& pair : investment_data) {
        double sum = std::accumulate(pair.second.begin(), pair.second.end(), 0.0);
        std::cout << "- " << pair.first << ": 样本量=" << pair.second.size()
            << ", 平均值=" << std::fixed << std::setprecision(4)
            << sum / pair.second.size() << "\n";
    }

    AnovaResult result = runANOVA(investment_data);
    printAnovaTable(result);

    return 0;
}
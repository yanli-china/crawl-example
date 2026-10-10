#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <numeric>

// 1. 定义核心数据结构（确保内存对齐以提升 Cache 命中率）
struct alignas(64) TickData {
    char symbol[8];
    double price;
    int volume;
    long long timestamp;
};

class QuantEngine {
private:
    std::queue<TickData> m_dataQueue;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    bool m_isStop = false;

    // 策略参数
    const size_t m_period = 5;
    std::vector<double> m_priceHistory;

public:
    // 生产者：行情接收线程回调
    void onReceiveTick(const TickData& tick) {
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_dataQueue.push(tick);
        }
        m_cv.notify_one(); // 通知策略线程
    }

    // 消费者：核心策略计算与执行线程
    void runStrategyLoop() {
        while (!m_isStop) {
            TickData tick;
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                m_cv.wait(lock, [this] { return !m_dataQueue.empty() || m_isStop; });

                if (m_isStop && m_dataQueue.empty()) break;

                tick = m_dataQueue.front();
                m_dataQueue.pop();
            }

            // 执行策略逻辑
            handleTick(tick);
        }
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_isStop = true;
        }
        m_cv.notify_one();
    }

private:
    // 策略核心计算
    void handleTick(const TickData& tick) {
        m_priceHistory.push_back(tick.price);
        if (m_priceHistory.size() > m_period) {
            m_priceHistory.erase(m_priceHistory.begin());
        }

        if (m_priceHistory.size() == m_period) {
            // 利用数学公式计算 SMA
            double sum = std::accumulate(m_priceHistory.begin(), m_priceHistory.end(), 0.0);
            double sma = sum / m_period;

            std::cout << "[策略触发] 当前价: " << tick.price << " | " << m_period << "日均价: " << sma;

            // 简单信号触发逻辑
            if (tick.price > sma) {
                std::cout << " -> 【买入信号】" << std::endl;
            }
            else if (tick.price < sma) {
                std::cout << " -> 【卖出信号】" << std::endl;
            }
            else {
                std::cout << " -> 【持仓观望】" << std::endl;
            }
        }
    }
};

int main() {
    QuantEngine engine;

    // 启动策略消费线程
    std::thread strategyThread(&QuantEngine::runStrategyLoop, &engine);

    // 模拟行情源输入（生产数据）
    std::vector<double> dummyPrices = { 100.0, 101.0, 101.5, 100.8, 102.0, 103.5, 101.0 };
    for (double p : dummyPrices) {
        TickData t{ "600519", p, 100, 20260714 };
        engine.onReceiveTick(t);
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 模拟行情间隔
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    engine.stop();
    strategyThread.join();

    return 0;
}
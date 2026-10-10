#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>

// 定义回调函数类型：接受一个字符串参数（可以是 JSON 或自定义数据结构）
using EventCallback = std::function<void(const std::string&)>;


// --- 改进版中介者：事件中心 (EventCenter) ---
class EventCenter {
private:
    // key: 事件名称, value: 关注该事件的回调列表
    std::map<std::string, std::vector<EventCallback>> subscribers;

public:
    // 订阅：子系统告诉中介者“我想要关注这个事件”
    void subscribe(const std::string& eventName, EventCallback callback) {
        subscribers[eventName].push_back(callback);
    }

    // 发布：当某个事件发生时，通知所有人
    void publish(const std::string& eventName, const std::string& data) {
        if (subscribers.find(eventName) != subscribers.end()) {
            for (auto& callback : subscribers[eventName]) {
                callback(data);
            }
        }
    }
};


// --- 子系统实现 ---

class AudioSystem {
public:
    void onPlayerDied(const std::string& data) {
        std::cout << "[音效系统] 接收到死亡事件，播放哀鸣音效。附加数据: " << data << std::endl;
    }
};

class AchievementSystem {
public:
    void checkDeathAchievement(const std::string& data) {
        std::cout << "[成就系统] 检查是否达成'首死'成就..." << std::endl;
    }
};

class Player {
private:
    EventCenter* eventCenter;
public:
    Player(EventCenter* ec) : eventCenter(ec) {}

    void die() {
        std::cout << "[玩家] 我挂了！" << std::endl;
        // 玩家只需要向中介者发布事件，不需要知道谁在听
        eventCenter->publish("PLAYER_DIED", "KillerID:9527");
    }
};

int main() {
    // 1. 实例化中介者（事件中心）
    EventCenter* ec = new EventCenter();

    // 2. 实例化子系统
    AudioSystem* audio = new AudioSystem();
    AchievementSystem* achievement = new AchievementSystem();
    Player* player = new Player(ec);

    // 3. 动态订阅（可以在任何时候、任何地方订阅）
    // 使用 lambda 表达式作为桥接
    ec->subscribe("PLAYER_DIED", [audio](const std::string& data) {
            audio->onPlayerDied(data);
        });

    ec->subscribe("PLAYER_DIED", [achievement](const std::string& data) {
            achievement->checkDeathAchievement(data);
        });

    // 4. 触发事件
    std::cout << "--- 游戏开始 ---" << std::endl;
    player->die();

    // 清理
    delete player; delete achievement; delete audio; delete ec;
    return 0;
}
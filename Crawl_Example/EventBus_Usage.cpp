#include <iostream>
#include <vector>
#include <functional>
#include <map>
#include <string>

// 定义事件类型
enum class GameEventType {
    EnemyKilled,
    PlayerLowHealth,
    ItemPickedUp
};

// 事件携带的数据（可以使用结构体）
struct EventData {
    std::string entityName;
    int id;
};

// 事件总线
class EventBus {

public:
    using Callback = std::function<void(const EventData&)>;

    static void Subscribe(GameEventType type, Callback callback) {
        subscribers[type].push_back(callback);
    }

    static void Publish(GameEventType type, const EventData& data) {
        if (subscribers.find(type) != subscribers.end()) {
            for (auto& cb : subscribers[type]) {
                cb(data);
            }
        }
    }

private:
    static std::map<GameEventType, std::vector<Callback>> subscribers;
};

// 初始化静态成员
std::map<GameEventType, std::vector<EventBus::Callback>> EventBus::subscribers;


class QuestSystem {
public:
    QuestSystem() {
        // 订阅击杀事件
        EventBus::Subscribe(GameEventType::EnemyKilled, [this](const EventData& data) {
            this->OnEnemyKilled(data);
            });
    }

    void OnEnemyKilled(const EventData& data) {
        std::cout << "[QuestSystem] 任务进度更新：击杀 " << data.entityName << " (1/10)" << std::endl;
    }
};

class DialogSystem {
public:
    DialogSystem() {
        EventBus::Subscribe(GameEventType::EnemyKilled, [](const EventData& data) {
            std::cout << "[DialogSystem] 播放语音：'又解决了一个麻烦！'" << std::endl;
            });
    }
};

class UISystem {
public:
    UISystem() {
        EventBus::Subscribe(GameEventType::EnemyKilled, [](const EventData& data) {
            std::cout << "[UISystem] 屏幕弹出：经验值 +100！" << std::endl;
            });
    }
};


int main() {
    // 实例化各个系统（它们会自动订阅事件）
    QuestSystem questSys;
    DialogSystem dialogSys;
    UISystem uiSys;

    std::cout << "--- 战斗发生，怪物死亡 ---" << std::endl;

    // 模拟 DamageSystem 判定击杀
    EventData deathInfo = { "暗影狼", 101 };

    // 只需要这一行，所有系统都会被激活
    EventBus::Publish(GameEventType::EnemyKilled, deathInfo);

    return 0;
}
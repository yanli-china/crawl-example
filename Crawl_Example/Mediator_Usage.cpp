#include <iostream>
#include <string>
#include <vector>

class GameSystem;

class GameMediator {
public:
    virtual void notify(GameSystem* sender, std::string event) = 0;
};

class GameSystem {
protected:
    GameMediator* mediator;
public:
    GameSystem(GameMediator* m) : mediator(m) {}
};

// --- 具体系统 ---

class AudioSystem : public GameSystem {
public:
    using GameSystem::GameSystem;   // 继承构造函数 (Inheriting Constructors)
    void playSound(std::string name) {
        std::cout << "[音效] 播放音频: " << name << std::endl;
    }
};

class UISystem : public GameSystem {
public:
    using GameSystem::GameSystem;
    void updateHealthBar(int hp) {
        std::cout << "[UI] 血条更新为: " << hp << std::endl;
    }
    void showGameOver() {
        std::cout << "[UI] 显示：胜败乃兵家常事，大侠请重新来过。" << std::endl;
    }
};

class Player : public GameSystem {
public:
    using GameSystem::GameSystem;
    void takeDamage(int damage) {
        std::cout << "[玩家] 受到伤害: " << damage << std::endl;
        // 玩家只负责发出通知，不关心谁来处理
        mediator->notify(this, "PLAYER_DAMAGED");

        if (damage > 100) {
            mediator->notify(this, "PLAYER_DIED");
        }
    }
};

// --- 具体中介者：战斗管理器 ---

class BattleManager : public GameMediator {
private:
    AudioSystem* audio;
    UISystem* ui;
    Player* player;

public:
    void setSystems(AudioSystem* a, UISystem* u, Player* p) {
        audio = a; ui = u; player = p;
    }

    // 核心逻辑集中地
    void notify(GameSystem* sender, std::string event) override {
        if (event == "PLAYER_DAMAGED") {
            audio->playSound("ouch.mp3");
            ui->updateHealthBar(50);
        }
        else if (event == "PLAYER_DIED") {
            audio->playSound("death_theme.wav");
            ui->showGameOver();
            std::cout << "[逻辑] 停止游戏计时器，保存当前关卡进度。" << std::endl;
        }
    }
};


int main() {
    // 1. 初始化中介者（战斗管理器）
    BattleManager* battleMgr = new BattleManager();

    // 2. 初始化各个子系统，并把中介者“介绍”给它们
    // 在实际开发中，这些系统可能是全局单例或者由引擎统一管理
    AudioSystem* audio = new AudioSystem(battleMgr);
    UISystem* ui = new UISystem(battleMgr);
    Player* player = new Player(battleMgr);

    // 3. 将子系统注册到中介者中，让中介者知道该指挥谁
    battleMgr->setSystems(audio, ui, player);

    std::cout << "--- 场景 1：玩家受到小额伤害 ---" << std::endl;
    player->takeDamage(20);

    std::cout << "\n--- 场景 2：玩家受到致命伤害 ---" << std::endl;
    player->takeDamage(120);

    // 4. 清理内存
    delete player;
    delete ui;
    delete audio;
    delete battleMgr;

    return 0;
}
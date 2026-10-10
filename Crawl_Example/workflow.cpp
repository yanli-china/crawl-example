#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <functional>

using namespace std;


// 任务状态
enum class TaskState
{
    WAITING,
    RUNNING,
    SUCCESS,
    FAILED
};


// 工作任务
class Task
{
public:
    string name;
    function<bool()> action;
    TaskState state;

    Task(string n, function<bool()> f)
        : name(n), action(f), state(TaskState::WAITING)
    {
    }


    bool run()
    {
        cout << "执行任务: " << name << endl;

        state = TaskState::RUNNING;

        bool result = action();

        if (result)
        {
            state = TaskState::SUCCESS;
            cout << name << " 成功\n";
        }
        else
        {
            state = TaskState::FAILED;
            cout << name << " 失败\n";
        }

        return result;
    }
};



// 工作流
class Workflow
{
private:

    map<string, Task*> tasks;

    // 邻接表
    map<string, vector<string>> edges;


public:


    void addTask(Task* task)
    {
        tasks[task->name] = task;
    }


    // A完成后执行B
    void addDependency(string A, string B)
    {
        edges[A].push_back(B);
    }



    void execute()
    {
        map<string, int> indegree;

        for (auto& t : tasks)
        {
            indegree[t.first] = 0;
        }


        for (auto& e : edges)
        {
            for (auto& next : e.second)
            {
                indegree[next]++;
            }
        }

        queue<string> q;

        for (auto& x : indegree)
        {
            if (x.second == 0)
                q.push(x.first);
        }

        while (!q.empty())
        {
            string name = q.front();
            q.pop();

            Task* task = tasks[name];

            if (task->run() == false)
            {
                cout << "工作流终止\n";
                return;
            }

            for (auto& next : edges[name])
            {
                indegree[next]--;

                if (indegree[next] == 0)
                    q.push(next);
            }
        }

        cout << "工作流完成\n";
    }

};


int main()
{

    Task download(
        "下载文件",
        []() {
            cout << "下载中...\n";
            return true;
        }
    );


    Task process(
        "处理文件",
        []() {
            cout << "处理中...\n";
            return true;
        }
    );


    Task upload(
        "上传结果",
        []() {
            cout << "上传中...\n";
            return true;
        }
    );

    Workflow wf;

    wf.addTask(&download);
    wf.addTask(&process);
    wf.addTask(&upload);

    // 下载 -> 处理 -> 上传
    wf.addDependency(
        "下载文件",
        "处理文件"
    );

    wf.addDependency(
        "处理文件",
        "上传结果"
    );

    wf.execute();


    return 0;
}
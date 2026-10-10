#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <memory>
#include <algorithm>

using namespace std;

/**
 * 员工基类 (Employee Base Class)
 * 包含基础属性和纯虚函数
 */
class Employee {
protected:
    int id;
    string name;
    string department;
    string type; // "Hourly" 或 "Salaried"

public:
    Employee(int i, string n, string d, string t) : id(i), name(n), department(d), type(t) {}
    virtual ~Employee() {}

    int getId() const { return id; }
    string getName() const { return name; }
    string getDept() const { return department; }
    string getType() const { return type; }

    // 核心薪资计算引擎：多态实现
    virtual double calculateGrossPay() const = 0;

    // 报表展示
    virtual void displayRow() const {
        double gross = calculateGrossPay();
        double tax = gross * 0.10; // 假设 10% 税率
        cout << left << setw(10) << id
            << setw(15) << name
            << setw(15) << department
            << setw(12) << type
            << setw(12) << fixed << setprecision(2) << gross
            << setw(12) << tax
            << setw(12) << (gross - tax) << endl;
    }

    // 用于数据持久化的格式化输出
    virtual string serialize() const = 0;
};

/**
 * 时薪员工类 (Hourly Employee)
 */
class HourlyEmployee : public Employee {
private:
    double hourlyRate;
    double hoursWorked;

public:
    HourlyEmployee(int i, string n, string d, double rate, double hours)
        : Employee(i, n, d, "Hourly"), hourlyRate(rate), hoursWorked(hours) {}

    double calculateGrossPay() const override {
        // 计算逻辑：超过40小时部分按1.5倍计费
        if (hoursWorked <= 40) {
            return hourlyRate * hoursWorked;
        }
        else {
            return (40 * hourlyRate) + ((hoursWorked - 40) * hourlyRate * 1.5);
        }
    }

    string serialize() const override {
        return to_string(id) + "," + name + "," + department + "," + type + "," + to_string(hourlyRate) + "," + to_string(hoursWorked);
    }
};

/**
 * 月薪员工类 (Salaried Employee)
 */
class SalariedEmployee : public Employee {
private:
    double monthlySalary;

public:
    SalariedEmployee(int i, string n, string d, double salary)
        : Employee(i, n, d, "Salaried"), monthlySalary(salary) {}

    double calculateGrossPay() const override {
        return monthlySalary;
    }

    string serialize() const override {
        return to_string(id) + "," + name + "," + department + "," + type + "," + to_string(monthlySalary) + ",0";
    }
};

/**
 * 工资系统管理类 (Payroll Manager)
 */
class PayrollSystem {
private:
    vector<unique_ptr<Employee>> employees;
    const string fileName = "payroll_data.txt";

public:
    PayrollSystem() {
        loadData();
    }

    // A. 员工管理：添加
    void addEmployee() {
        int id, choice;
        string name, dept;
        cout << "\n--- 添加新员工 ---" << endl;
        cout << "输入 ID: "; cin >> id;
        cout << "输入姓名: "; cin.ignore(); getline(cin, name);
        cout << "输入部门: "; getline(cin, dept);
        cout << "选择类型 (1. 时薪制 2. 月薪制): "; cin >> choice;

        if (choice == 1) {
            double rate, hours;
            cout << "输入时薪: "; cin >> rate;
            cout << "输入工时: "; cin >> hours;
            employees.push_back(make_unique<HourlyEmployee>(id, name, dept, rate, hours));
        }
        else {
            double salary;
            cout << "输入月薪: "; cin >> salary;
            employees.push_back(make_unique<SalariedEmployee>(id, name, dept, salary));
        }
        cout << "员工添加成功！" << endl;
        saveData();
    }

    // A. 员工管理：删除
    void deleteEmployee() {
        int id;
        cout << "输入要删除的员工 ID: "; cin >> id;
        auto it = remove_if(employees.begin(), employees.end(), [id](const unique_ptr<Employee>& e) {
            return e->getId() == id;
            });
        if (it != employees.end()) {
            employees.erase(it, employees.end());
            cout << "删除成功。" << endl;
            saveData();
        }
        else {
            cout << "未找到该 ID 的员工。" << endl;
        }
    }

    // D. 报表与生成
    void generateReport() const {
        if (employees.empty()) {
            cout << "\n系统内无员工数据。" << endl;
            return;
        }

        cout << "\n" << string(85, '=') << endl;
        cout << left << setw(10) << "ID"
            << setw(15) << "姓名"
            << setw(15) << "部门"
            << setw(12) << "类型"
            << setw(12) << "总薪资"
            << setw(12) << "预扣税"
            << setw(12) << "实发工资" << endl;
        cout << string(85, '-') << endl;

        double totalPayroll = 0;
        for (const auto& e : employees) {
            e->displayRow();
            totalPayroll += (e->calculateGrossPay() * 0.9); // 实发汇总
        }
        cout << string(85, '=') << endl;
        cout << "公司总实发成本: $" << fixed << setprecision(2) << totalPayroll << endl;
    }

    // C. 数据持久化：保存
    void saveData() const {
        ofstream outFile(fileName);
        if (!outFile) return;
        for (const auto& e : employees) {
            outFile << e->serialize() << endl;
        }
        outFile.close();
    }

    // C. 数据持久化：读取
    void loadData() {
        ifstream inFile(fileName);
        if (!inFile) return;
        string line;
        while (getline(inFile, line)) {
            // 简单的 CSV 解析（按逗号分割）
            size_t pos = 0;
            vector<string> tokens;
            while ((pos = line.find(',')) != string::npos) {
                tokens.push_back(line.substr(0, pos));
                line.erase(0, pos + 1);
            }
            tokens.push_back(line);

            if (tokens.size() >= 6) {
                int id = stoi(tokens[0]);
                string name = tokens[1];
                string dept = tokens[2];
                string type = tokens[3];
                double val1 = stod(tokens[4]);
                double val2 = stod(tokens[5]);

                if (type == "Hourly")
                    employees.push_back(make_unique<HourlyEmployee>(id, name, dept, val1, val2));
                else
                    employees.push_back(make_unique<SalariedEmployee>(id, name, dept, val1));
            }
        }
        inFile.close();
    }
};

int main() {
    PayrollSystem system;
    int choice;

    while (true) {
        cout << "\n===== 企业工资管理系统 =====" << endl;
        cout << "1. 添加员工" << endl;
        cout << "2. 删除员工" << endl;
        cout << "3. 显示工资报表" << endl;
        cout << "4. 退出系统" << endl;
        cout << "请选择操作: ";
        cin >> choice;

        switch (choice) {
        case 1: system.addEmployee(); break;
        case 2: system.deleteEmployee(); break;
        case 3: system.generateReport(); break;
        case 4: cout << "系统已退出。" << endl; return 0;
        default: cout << "无效选择，请重试。" << endl;
        }
    }
    return 0;
}
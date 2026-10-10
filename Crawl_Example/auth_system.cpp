#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

using namespace std;

/**
 * User Role Enumeration
 */
enum Role { ADMIN = 1, EDITOR = 2, GUEST = 3 };

string roleToString(Role r) {
    switch (r) {
    case ADMIN: return "Admin";
    case EDITOR: return "Editor";
    case GUEST: return "Guest";
    default: return "Unknown";
    }
}

Role stringToRole(string s) {
    if (s == "Admin") return ADMIN;
    if (s == "Editor") return EDITOR;
    return GUEST;
}

/**
 * User Class
 */
class User {
public:
    string username;
    string password;
    Role role;

    User(string u, string p, Role r) : username(u), password(p), role(r) {}
};

/**
 * Authentication & Authorization System
 */
class AuthSystem {
private:
    const string dbFile = "users_db.txt";
    map<string, pair<string, Role>> userDatabase; // username -> {password, role}
    User* currentUser = nullptr;

public:
    AuthSystem() {
        loadUsers();
    }

    ~AuthSystem() {
        delete currentUser;
    }

    bool isStrongPassword(const string& pwd) {
        if (pwd.length() < 6) return false;
        bool hasDigit = false;
        for (char c : pwd) {
            if (isdigit(c)) hasDigit = true;
        }
        return hasDigit;
    }

    // 1. Register with Role Selection
    void registerUser() {
        string u, p;
        int rChoice;
        cout << "\n--- User Registration ---" << endl;
        cout << "Enter username: "; cin >> u;

        if (userDatabase.find(u) != userDatabase.end()) {
            cout << "Error: Username already exists!" << endl;
            return;
        }

        cout << "Enter password (6+ chars, incl. digit): "; cin >> p;
        if (!isStrongPassword(p)) {
            cout << "Error: Password too weak!" << endl;
            return;
        }

        cout << "Select Role (1. Admin, 2. Editor, 3. Guest): "; cin >> rChoice;
        Role r = (rChoice == 1) ? ADMIN : (rChoice == 2 ? EDITOR : GUEST);

        userDatabase[u] = { p, r };
        saveUser(u, p, r);
        cout << "Registration successful as " << roleToString(r) << "!" << endl;
    }

    // 2. Login
    bool loginUser() {
        string u, p;
        cout << "\n--- User Login ---" << endl;
        cout << "Username: "; cin >> u;
        cout << "Password: "; cin >> p;

        if (userDatabase.count(u) && userDatabase[u].first == p) {
            Role r = userDatabase[u].second;
            currentUser = new User(u, p, r);
            cout << "Login successful! Welcome [" << roleToString(r) << "] " << u << "." << endl;
            return true;
        }
        else {
            cout << "Error: Invalid credentials." << endl;
            return false;
        }
    }

    void logout() {
        delete currentUser;
        currentUser = nullptr;
        cout << "Logged out successfully." << endl;
    }

    User* getCurrentUser() { return currentUser; }

    // 3. Persistence
    void saveUser(const string& u, const string& p, Role r) {
        ofstream outFile(dbFile, ios::app);
        if (outFile.is_open()) {
            outFile << u << " " << p << " " << roleToString(r) << endl;
            outFile.close();
        }
    }

    void loadUsers() {
        ifstream inFile(dbFile);
        string u, p, rStr;
        while (inFile >> u >> p >> rStr) {
            userDatabase[u] = { p, stringToRole(rStr) };
        }
        inFile.close();
    }

    // 4. Role-based feature display
    void showPermissions() {
        if (!currentUser) return;

        cout << "\n--- Authorized Actions for " << roleToString(currentUser->role) << " ---" << endl;
        cout << "[1] View Public Content (All)" << endl;

        if (currentUser->role == ADMIN || currentUser->role == EDITOR) {
            cout << "[2] Edit Content (Admin/Editor)" << endl;
        }

        if (currentUser->role == ADMIN) {
            cout << "[3] Delete Content (Admin Only)" << endl;
            cout << "[4] Manage System Users (Admin Only)" << endl;
        }
    }
};

int main() {
    AuthSystem auth;
    int choice;

    while (true) {
        if (!auth.getCurrentUser()) {
            cout << "\n========== System Portal ==========" << endl;
            cout << "1. Register\n2. Login\n3. Exit" << endl;
            cout << "Choice: "; cin >> choice;

            if (choice == 1) auth.registerUser();
            else if (choice == 2) auth.loginUser();
            else if (choice == 3) break;
        }
        else {
            auth.showPermissions();
            cout << "0. Logout" << endl;
            cout << "Select action: "; cin >> choice;

            if (choice == 0) auth.logout();
            else {
                // Perform permission check before execution
                Role userRole = auth.getCurrentUser()->role;
                if (choice == 2 && (userRole == GUEST)) {
                    cout << "Access Denied: You don't have Editor permissions." << endl;
                }
                else if ((choice == 3 || choice == 4) && userRole != ADMIN) {
                    cout << "Access Denied: Requires Administrator privileges." << endl;
                }
                else {
                    cout << "Action " << choice << " executed successfully." << endl;
                }
            }
        }
    }

    return 0;
}
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <fstream>
#include <windows.h>
#include <clocale>
#include <conio.h>
#include <ctime>

using namespace std;
map<string, string> p_vars;
map<string, bool> modules;

string trim_and_clean(string line) {
    size_t comment = line.find('~');
    if (comment != string::npos) line = line.substr(0, comment);
    line.erase(0, line.find_first_not_of(" \t"));
    line.erase(line.find_last_not_of(" \t") + 1, string::npos);
    return line;
}

void execute_command(string line, int i, HANDLE hConsole);

void error(string m, int l) {
    cout << "ОШИБКА (Строка " << l << "): " << m << endl;
    system("pause");
    exit(1);
}

void execute_command(string line, int i, HANDLE hConsole) {
    if (line.empty()) return;
    if (line.find("load=") == 0) {
        smatch m;
        if (regex_search(line, m, regex("'(.*)'"))) modules[m[1]] = true;
    }
    else if (line.find("get.text() to ") == 0) {
        string var_name = line.substr(14);
        string input_val;
        getline(cin, input_val);
        p_vars[var_name] = input_val;
    }
    else if (line.find("p.") == 0 && line.find("=") != string::npos && line.find("\"") == string::npos) {
        smatch m;
        if (regex_search(line, m, regex(R"(p\.(.*?)=(\d+)([\+\-\*/])(\d+))"))) {
            string var_name = m[1];
            int v1 = stoi(m[2]);
            char op = m[3].str()[0];
            int v2 = stoi(m[4]);
            int res = 0;
            if (op == '+') res = v1 + v2;
            else if (op == '-') res = v1 - v2;
            else if (op == '*') res = v1 * v2;
            else if (op == '/') res = (v2 != 0) ? v1 / v2 : 0;
            p_vars[var_name] = to_string(res);
        }
    }
    else if (line.find("text(") == 0) {
        smatch m;
        if (line.find("p.\".") != string::npos) {
            if (regex_search(line, m, regex(R"(p\.\"\.(.*?)\.\"\))"))) {
                cout << p_vars[m[1]] << endl;
            }
        } 
        else if (regex_search(line, m, regex("\\(\"(.*)\"\\)"))) {
            cout << m[1] << endl;
        }
    }
    else if (line.find("keyboard.wait_key") == 0) {
        if (!modules["keyboard"]) error("Модуль 'keyboard' не загружен", i + 1);
        smatch m;
        if (regex_search(line, m, regex(R"(keyboard\.wait_key\((.*?)\)\s+to\s+(.*))"))) {
            string target_key = m[1];
            string next_cmd = m[2];
            while (true) {
                int ch = _getch();
                if (target_key == "Enter" && ch == 13) break;
                else if (target_key == "") break;
                else if (target_key.length() == 1 && ch == (int)target_key[0]) break;
            }
            execute_command(next_cmd, i, hConsole);
        }
    }
    else if (line.find("system.") == 0 || line.find("shutdown") == 0) {
        if (!modules["system"]) error("Модуль 'system' не загружен", i + 1);
        if (line == "system.get_pc_name() to res") {
            char name[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD size = sizeof(name);
            GetComputerNameA(name, &size);
            p_vars["res"] = string(name);
        }
        else if (line == "system.get_time() to res") {
            time_t now = time(0);
            char dt[26];
            ctime_s(dt, sizeof(dt), &now);
            string s_dt(dt);
            p_vars["res"] = s_dt.substr(0, s_dt.length() - 1);
        }
        else if (line == "system.run_as_admin()") {
            char szPath[MAX_PATH];
            GetModuleFileNameA(NULL, szPath, MAX_PATH);
            ShellExecuteA(NULL, "runas", szPath, NULL, NULL, SW_SHOWNORMAL);
            exit(0);
        }
        else if (line == "shutdown -none") system("shutdown /s /t 0");
        else if (line == "shutdown") system("shutdown /s /t 60");
    }
    else if (line.find("c.clear(") == 0) {
        if (!modules["console"]) error("Модуль 'console' не загружен", i + 1);
        if (line.find("all") != string::npos) system("cls");
    }
    else if (line.find("clock.stop(") == 0) {
        if (!modules["clock"]) error("Модуль 'clock' не загружен", i + 1);
        smatch m;
        if (regex_search(line, m, regex(R"(\((\d+)([smh])\))"))) {
            int val = stoi(m[1]);
            char unit = m[2].str()[0];
            if (unit == 's') Sleep(val * 1000);
            else if (unit == 'm') Sleep(val * 60000);
        }
    }
    else if (line.find("color.set(") == 0) {
        if (!modules["color"]) error("Модуль 'color' не загружен", i + 1);
        if (line.find("red") != string::npos) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        else if (line.find("green") != string::npos) SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        else if (line.find("yellow") != string::npos) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        else if (line.find("white") != string::npos) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    else if (line.find("p.") == 0 && line.find("=\"") != string::npos) {
        smatch m;
        if (regex_search(line, m, regex(R"(p\.(.*?)=\"(.*?)\")"))) {
            p_vars[m[1]] = m[2];
        }
    }
    else if (line == "stop()") {
        _getch();
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    vector<string> code;

    if (argc > 1) {
        ifstream file(argv[1]);
        if (!file.is_open()) return 1;
        string line;
        while (getline(file, line)) code.push_back(line);
        file.close();
    } else {
        return 0;
    }

    for (int i = 0; i < (int)code.size(); ++i) {
        string line = trim_and_clean(code[i]);
        execute_command(line, i, hConsole);
    }
    return 0;
}
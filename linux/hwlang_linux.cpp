#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>

using namespace std;

const string CLR_CYAN    = "\033[36m";
const string CLR_GREEN   = "\033[32m";
const string CLR_RED     = "\033[31m";
const string CLR_RESET   = "\033[0m";

void run_linux_cmd(string c) {
    if (c == "off") {
        cout << CLR_RED << "[HW] System shutdown..." << CLR_RESET << endl;
        system("poweroff");
    } else if (c == "inf") {
        cout << CLR_CYAN << "[HW] Kernel: " << CLR_RESET;
        system("uname -sr");
    } else {
        system(c.c_str());
    }
}

int main() {
    system("clear");
    cout << CLR_GREEN << "HW.LANGUAGE v2.7 [PURE LINUX BUILD]" << CLR_RESET << endl;
    cout << "Type 'help' for commands or 'exit' to quit." << endl;

    string cmd;
    while (true) {
        cout << CLR_CYAN << "hw_linux@kernel:~$ " << CLR_RESET;
        if (!getline(cin, cmd) || cmd == "exit") break;

        if (cmd == "help") {
            cout << "off  - shutdown system" << endl;
            cout << "inf  - show kernel version" << endl;
            cout << "cls  - clear terminal" << endl;
            cout << "exit - close interpreter" << endl;
        } 
        else if (cmd == "cls") {
            system("clear");
        }
        else if (!cmd.empty()) {
            run_linux_cmd(cmd);
        }
    }
    return 0;
}
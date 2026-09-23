#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;


void clearScreen() {
#ifdef _WIN32
    system("cls"); 
#else
    system("clear"); 
#endif
}


void printHeader() {
    cout << "Hello, Welcome to the CSOPESY commandline!\n";
    cout << "Enter 'exit' to quit, 'clear' to clear the screen\n\n";
    cout << "** IMPORTANT: Enter 'initialize' to load config and start system **\n\n";
}

int main() {
    printHeader();
    string command;

    while (true) {
        cout << "Enter a command: ";
        getline(cin, command);

        if (command == "exit") {
            break; 
        } 
        else if (command == "clear") {
            clearScreen();
            
        } 
        else if (command == "initialize") {
            clearScreen();
            printHeader();
        }
        else if (command == "screen" || 
                   command == "scheduler-start" || 
                   command == "scheduler-stop" || 
                   command == "report-util") {
            cout << command << " command recognized. Doing something.\n";
        } 
        else if (!command.empty()) {
            cout << command << " is not a recognized command.\n";
        }
    }

    return 0;
}
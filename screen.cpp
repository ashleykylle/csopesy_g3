#include "header/screen.h"
#include <thread>
#include <chrono>

map<string, Screen*> screens;

Screen::Screen(const string& name, Process* process)
    : screenName(name), attachedProcess(process), currentInstructions(0),
    totalInstructions(process->getTotalInstructions()) {}

void Screen::display() {
    system("cls");

    string command;
    bool inScreen = true;

    cout << "Process: " << attachedProcess->getName() << endl;
    cout << "Process ID: " << attachedProcess->getId() << endl << endl;

    while (inScreen) {

        if (attachedProcess->hasFinished()) {
            screens.erase(screenName);
            inScreen = false;
            break;
        }

        cout << "Enter command: ";
        getline(cin, command);

        if (command == "process-smi") {
            cout << endl;
            if (!attachedProcess->hasFinished()) {
                cout << "Process: " << attachedProcess->getName() << endl;
                cout << "Process ID: " << attachedProcess->getId() << endl << endl;
                cout << "Current Instruction line: " << attachedProcess->getTotalInstructions() - attachedProcess->getRemainingInstructions() << endl;
                cout << "Lines of code: " << attachedProcess->getTotalInstructions() << endl;
            }
            else {
                cout << "Process: " << attachedProcess->getName() << endl;
                cout << "Process ID: " << attachedProcess->getId() << endl << endl;
                cout << "Finished!" << endl;
                screens.erase(screenName);
                this_thread::sleep_for(chrono::seconds(2));
                inScreen = false;
            }
            cout << endl;
        }
        else if (command == "exit") {
            inScreen = false;
        }
        else {
            cout << "Command '" << command << "' not recognized." << endl;
        }
    }
}
#include "header/screen.h"

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
        cout << "Enter command: ";
        getline(cin, command);

        if (command == "process-smi") {
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
            }
            
        }
        else if (command == "exit") {
            inScreen = false;
        }
        else {
            cout << "Command '" << command << "' not recognized." << endl;
        }
    }
}
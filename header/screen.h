#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>
#include "process.h" 

using namespace std;

class Screen {
private:
    string screenName;
    Process* attachedProcess;
    int currentInstructions;
    int totalInstructions;

public:
    Screen(const string& name, Process* process);
    void display();
};
#endif

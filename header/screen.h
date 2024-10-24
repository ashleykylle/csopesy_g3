#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>
#include <map>
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
extern map<string, Screen*> screens;
#endif

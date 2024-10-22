#include <iostream>
#include <ctime>
#include <chrono>
#include "header/process.h"
#include "header/utils.h"

using namespace std;

Process::Process(const string& processName, int processId, int numInstructions)
    : name(processName), id(processId), totalInstructions(numInstructions), 
        remainingInstructions(numInstructions), isFinished(false) {}

void Process::executeInstruction(int coreId) {
    if (remainingInstructions > 0) {
        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        tm* now_tm = localtime(&now_c);

        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%m/%d/%Y %I:%M:%S%p", now_tm);
        remainingInstructions--;
    } else {
        cout << "Process " << id << ": " << name << " has already finished.\n";
    }
}

void Process::markAsFinished() {
    isFinished = true;
    completionTimestamp = getCurrentTimestamp();
}

void Process::setCoreId(int core) {
    coreId = core;
}

bool Process::hasFinished() const {
    return remainingInstructions == 0;
}

string Process::getName() const {
        return name;
    }

string Process::getCompletionTimestamp() const {
    return completionTimestamp;
}

int Process::getRemainingInstructions() const {
    return remainingInstructions;
}

int Process::getId() const {
    return id;
}

int Process::getTotalInstructions() const {
    return totalInstructions;
}

int Process::getCoreId() const {
    return coreId;
}

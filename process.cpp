#include <iostream>
#include <chrono>
#include "header/process.h"
#include "header/utils.h"

using namespace std;

Process::Process(const string& processName, int processId, int numInstructions, size_t memoryRequired)
    : name(processName), id(processId), totalInstructions(numInstructions), 
        remainingInstructions(numInstructions), isFinished(false), memoryRequired(memoryRequired) {}

void Process::executeInstruction() {
    if (remainingInstructions > 0) {
        remainingInstructions--;
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

size_t Process::getMemoryRequired() const {
    return memoryRequired;
}

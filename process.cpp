#include <iostream>
#include <fstream>
#include <chrono>
#include "header/process.h"
#include "header/utils.h"

using namespace std;

Process::Process(const string& processName, int processId, int numInstructions, size_t memoryRequired, vector<int> pageIndices)
    : name(processName), id(processId), totalInstructions(numInstructions), remainingInstructions(numInstructions),
    isFinished(false), memoryRequired(memoryRequired), memory(nullptr), pageIndices(pageIndices) {}

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

void Process::storeMemory(void* mem) {
    memory = mem;
}

void* Process::getAllocatedMemory() const { 
    return memory;
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

int Process::getNumPages() const {
    return pageIndices.size();
}

size_t Process::getMemoryRequired() const {
    return memoryRequired;
}

const vector<int>& Process::getPageIndices() const {
    return pageIndices;
}

void Process::storeToBackStorage(const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        file << "-----------------------------------" << endl;
        file << "Process Name: " << name << endl;
        file << "Process ID: " << id << endl;
        file << "Process Core ID: " << coreId << endl;
        file << "Instructions Executed: " << totalInstructions - remainingInstructions << endl;
        file << "Total Instructions: " << totalInstructions << endl;
        file << "Memory Required: " << memoryRequired << endl;
        file << "Number of Pages: " << pageIndices.size() << endl;
        file << "-----------------------------------" << endl;
        file.close();
    } else {
        cerr << "Error: Unable to open file for back storage." << endl;
    }
}

FIFOPageReplacement::FIFOPageReplacement(int frameCount)
    : frameCount(frameCount) {}

void FIFOPageReplacement::loadProcess(const Process& process) {
    cout << "Loading Process " << process.getId() << " into memory...\n";

    for (int pageIndex : process.getPageIndices()) {
        if (pageTable.find(pageIndex) == pageTable.end()) {
            if (memoryQueue.size() >= frameCount) {
                int oldestPage = memoryQueue.front();
                memoryQueue.pop();
                pageTable.erase(oldestPage);
                cout << "Evicting Page " << oldestPage << endl;
            }
            memoryQueue.push(pageIndex);
            pageTable.insert(pageIndex);
            cout << "Page " << pageIndex << " loaded into memory.\n";
        } else {
            cout << "Page " << pageIndex << " is already in memory.\n";
        }
    }
    cout << "Process " << process.getId() << " execution completed.\n\n";
}

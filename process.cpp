#include <iostream>
#include <fstream>
#include <chrono>
#include "header/process.h"
#include "header/utils.h"

using namespace std;

Process::Process(const string& processName, int processId, int numInstructions, size_t memoryRequired, vector<int> pageIndices)
    : name(processName), id(processId), totalInstructions(numInstructions), remainingInstructions(numInstructions),
    isFinished(false), memoryRequired(memoryRequired), memory(nullptr), pageIndices(pageIndices), pageTable() {}

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

void Process::storePageTable(unordered_map<size_t, size_t>& pagetable) {
    pageTable = pagetable;
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

const unordered_map<size_t, size_t>& Process::getPageTable() const {
    return pageTable;
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

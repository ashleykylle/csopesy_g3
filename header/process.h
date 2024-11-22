#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <atomic>
#include <vector>
#include <queue>
#include <unordered_set>

using namespace std;

class Process {
private:
    string name;
    int id;
    int coreId;
    int totalInstructions;
    int remainingInstructions;
    atomic<bool> isFinished;
    string completionTimestamp;
    size_t memoryRequired;
    void* memory;
    vector<int> pageIndices;

public:
    Process(const string& processName, int processId, int numInstructions, size_t memoryRequired, vector<int> pageIndices);

    void executeInstruction();
    void markAsFinished();
    void setCoreId(int core);
    void storeMemory(void* mem);
    void* getAllocatedMemory() const;
    bool hasFinished() const;
    string getName() const;
    string getCompletionTimestamp() const;
    int getRemainingInstructions() const;
    int getId() const;
    int getTotalInstructions() const;
    int getCoreId() const;
    int getNumPages() const;
    size_t getMemoryRequired() const;
    const vector<int>& getPageIndices() const;
    void storeToBackStorage(const string& filename);
};

class FIFOPageReplacement {
private:
    int frameCount;
    queue<int> memoryQueue;
    unordered_set<int> pageTable;

public:
    FIFOPageReplacement(int frameCount);
    void loadProcess(const Process& process);
};

#endif

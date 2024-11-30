#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <atomic>
#include <vector>
#include <queue>
#include <unordered_map>

using namespace std;

class Process {
private:
    string name;
    int id;
    int coreId;
    int totalInstructions;
    int remainingInstructions;
    atomic<bool> isFinished;
    atomic<bool> isInBackStorage;
    string completionTimestamp;
    size_t memoryRequired;
    void* memory;
    vector<int> pageIndices;
    unordered_map<size_t, size_t> pageTable;

public:
    Process(const string& processName, int processId, int numInstructions, size_t memoryRequired, vector<int> pageIndices);

    void executeInstruction();
    void markAsFinished();
    void setCoreId(int core);
    void storeMemory(void* mem);
    void storePageTable(unordered_map<size_t, size_t>& pageTable);
    void storeToBackStorage(const string& filename);
    void removeFromBackStorage();
    void* getAllocatedMemory() const;
    bool hasFinished() const;
    bool processsInBackStorage() const;
    string getName() const;
    string getCompletionTimestamp() const;
    int getRemainingInstructions() const;
    int getId() const;
    int getTotalInstructions() const;
    int getCoreId() const;
    int getNumPages() const;
    size_t getMemoryRequired() const;
    const vector<int>& getPageIndices() const;
    const unordered_map<size_t, size_t>& getPageTable() const;
};

#endif

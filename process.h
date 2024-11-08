#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <atomic>

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

public:
    Process(const string& processName, int processId, int numInstructions, size_t memoryRequired);

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
    size_t getMemoryRequired() const;
};

#endif

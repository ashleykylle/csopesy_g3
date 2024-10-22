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

public:
    Process(const string& processName, int processId, int numInstructions);

    void executeInstruction(int coreId);
    void markAsFinished();
    void setCoreId(int core);
    bool hasFinished() const;
    string getName() const;
    string getCompletionTimestamp() const;
    int getRemainingInstructions() const;
    int getId() const;
    int getTotalInstructions() const;
    int getCoreId() const;
};

#endif

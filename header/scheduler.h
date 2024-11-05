#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "process.h"
#include "memory.h"
#include "config.h"

using namespace std;

class Scheduler {
public:
    virtual void addProcess(Process* process, int core) = 0;
    virtual void runScheduler(const Config& config, int& cpuCycles, FlatMemoryAllocator& memoryAllocator) = 0;
    virtual vector<Process*> getFinishedProcesses() const = 0;
    virtual vector<vector<Process*>> getProcessQueues() const = 0;
    virtual bool schedulerRunning() const = 0;
    virtual ~Scheduler() {}
};

class FCFSScheduler : public Scheduler {
private:
    int numCores;
    vector<thread> coreThreads;
    vector<vector<Process*>> processQueues;
    vector<Process*> finishedProcesses;
    mutex queueMutex;
    mutex finishedMutex;
    atomic<bool> isRunning;
    IMemoryAllocator& memoryAllocator;

public:
    FCFSScheduler(int cores, IMemoryAllocator& memoryAllocator);
    void addProcess(Process* process, int core) override;
    void runScheduler(const Config& config, int& cpuCycles, FlatMemoryAllocator& memoryAllocator) override;
    vector<Process*> getFinishedProcesses() const override;
    vector<vector<Process*>> getProcessQueues() const override;
    bool schedulerRunning() const override;
};

class RoundRobinScheduler : public Scheduler {
private:
    int numCores;
    int quantumCycles;
    vector<thread> coreThreads;
    vector<vector<Process*>> processQueues;
    vector<Process*> finishedProcesses;
    mutex queueMutex;
    mutex finishedMutex;
    atomic<bool> isRunning;
    IMemoryAllocator& memoryAllocator;

public:
    RoundRobinScheduler(int cores, int quantum, IMemoryAllocator& memoryAllocator);
    void addProcess(Process* process, int core) override;
    void runScheduler(const Config& config, int& cpuCycles, FlatMemoryAllocator& memoryAllocator) override;
    vector<Process*> getFinishedProcesses() const override;
    vector<vector<Process*>> getProcessQueues() const override;
    bool schedulerRunning() const override;
};

#endif

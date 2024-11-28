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
    virtual void runScheduler(const Config& config, int& cpuCycles, IMemoryAllocator& memoryAllocator) = 0;
    virtual vector<Process*> getFinishedProcesses() const = 0;
    virtual vector<vector<Process*>> getProcessQueues() const = 0;
    virtual bool schedulerRunning() const = 0;
    virtual ~Scheduler() {}
    virtual int getIdleCPUTicks() const = 0;
    virtual int getActiveCPUTicks() const = 0;
};

class FCFSScheduler : public Scheduler {
private:
    int numCores;
    int idleCPUTicks;
    int activeCPUTicks;
    vector<thread> coreThreads;
    vector<vector<Process*>> processQueues;
    vector<Process*> finishedProcesses;
    mutex queueMutex;
    mutex finishedMutex;
    mutex memoryMutex;
    atomic<bool> isRunning;
    IMemoryAllocator& memoryAllocator;

public:
    FCFSScheduler(int cores, IMemoryAllocator& memoryAllocator);
    void addProcess(Process* process, int core) override;
    void runScheduler(const Config& config, int& cpuCycles, IMemoryAllocator& memoryAllocator) override;
    vector<Process*> getFinishedProcesses() const override;
    vector<vector<Process*>> getProcessQueues() const override;
    bool schedulerRunning() const override;
    int getIdleCPUTicks() const { return idleCPUTicks; }
    int getActiveCPUTicks() const { return activeCPUTicks; }
};

class RoundRobinScheduler : public Scheduler {
private:
    int numCores;
    int quantumCycles;
    int idleCPUTicks;
    int activeCPUTicks;
    vector<thread> coreThreads;
    vector<vector<Process*>> processQueues;
    vector<Process*> finishedProcesses;
    mutex queueMutex;
    mutex finishedMutex;
    mutex memoryMutex;
    atomic<bool> isRunning;
    IMemoryAllocator& memoryAllocator;

public:
    RoundRobinScheduler(int cores, int quantum, IMemoryAllocator& memoryAllocator);
    void addProcess(Process* process, int core) override;
    void runScheduler(const Config& config, int& cpuCycles, IMemoryAllocator& memoryAllocator) override;
    vector<Process*> getFinishedProcesses() const override;
    vector<vector<Process*>> getProcessQueues() const override;
    bool schedulerRunning() const override;
    int getIdleCPUTicks() const { return idleCPUTicks; }
    int getActiveCPUTicks() const { return activeCPUTicks; }
};

#endif

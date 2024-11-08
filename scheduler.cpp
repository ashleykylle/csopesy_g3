#include <chrono>
#include <iostream>
#include "header/scheduler.h"

using namespace std;

FCFSScheduler::FCFSScheduler(int cores, IMemoryAllocator& memoryAllocator)
    : numCores(cores), processQueues(cores), isRunning(false), memoryAllocator(memoryAllocator) {}

void FCFSScheduler::addProcess(Process* process, int core) {
    if (core >= 0 && core < numCores) {
        lock_guard<mutex> guard(queueMutex);
        process->setCoreId(core);
        processQueues[core].push_back(process);
    }
}

void FCFSScheduler::runScheduler(const Config& config, int& cpuCycles, FlatMemoryAllocator& memoryAllocator) {
    isRunning = true;
    auto coreFunction = [this, &config, &cpuCycles](int coreId)  {
        while (isRunning) {
            Process* currentProcess = nullptr;

            {
                lock_guard<mutex> guard(queueMutex);
                if (!processQueues[coreId].empty()) {
                    currentProcess = processQueues[coreId].front();
                }
            }

            if (currentProcess) {
                while (!currentProcess->hasFinished()) {
                    if (cpuCycles % config.delayPerExec == 0) {
                        currentProcess->executeInstruction();
                    }
                }
                currentProcess->markAsFinished();

                {
                    lock_guard<mutex> guard(queueMutex);
                    processQueues[coreId].erase(processQueues[coreId].begin());
                }

                {
                    lock_guard<mutex> guard(finishedMutex);
                    finishedProcesses.push_back(currentProcess);
                }
            }
        }
    };

    for (int i = 0; i < numCores; ++i) {
        coreThreads.push_back(thread(coreFunction, i));
    }

    for (auto& t : coreThreads) {
        if (t.joinable()) {
            t.join();
        }
    }
    isRunning = false;
}

vector<Process*> FCFSScheduler::getFinishedProcesses() const {
    return finishedProcesses;
}

vector<vector<Process*>> FCFSScheduler::getProcessQueues() const {
    return processQueues;
}

bool FCFSScheduler::schedulerRunning() const {
    return isRunning;
}

RoundRobinScheduler::RoundRobinScheduler(int cores, int quantum, IMemoryAllocator& memoryAllocator) 
    : numCores(cores), quantumCycles(quantum), processQueues(cores), isRunning(false), memoryAllocator(memoryAllocator) {}

void RoundRobinScheduler::addProcess(Process* process, int core) {
    if (core >= 0 && core < numCores) {
        lock_guard<mutex> guard(queueMutex);
        process->setCoreId(core);
        processQueues[core].push_back(process);
    }
}

void RoundRobinScheduler::runScheduler(const Config& config, int& cpuCycles, FlatMemoryAllocator& memoryAllocator) {
    isRunning = true;
    auto coreFunction = [this, &config, &cpuCycles, &memoryAllocator](int coreId) {
        int quantumCycleCount = 0;
        while (isRunning) {
            Process* currentProcess = nullptr;

            {
                lock_guard<mutex> guard(queueMutex);
                if (!processQueues[coreId].empty()) {
                    currentProcess = processQueues[coreId].front();
                }
            }

            if (currentProcess) {
                void* memory = currentProcess->getAllocatedMemory();

                if (!memory) {
                    memory = memoryAllocator.allocate(currentProcess->getMemoryRequired(), config.memPerFrame);
                    currentProcess->storeMemory(memory);
                }

                if (memory) {
                    int executedCycles = 0;
                    
                    while (!currentProcess->hasFinished() && executedCycles < quantumCycles) {
                        if (config.delayPerExec != 0) {
                            if (cpuCycles % config.delayPerExec == 0) {
                                currentProcess->executeInstruction();
                                executedCycles++;
                            }
                        } else {
                            currentProcess->executeInstruction();
                            executedCycles++;
                        }
                    }
                    quantumCycleCount++;
                    memoryAllocator.logMemoryStamp(quantumCycleCount, config.memPerProc, config.memPerFrame);

                    if (currentProcess->hasFinished()) {
                        currentProcess->markAsFinished();

                        {
                            lock_guard<mutex> guard(queueMutex);
                            processQueues[coreId].erase(processQueues[coreId].begin());
                        }

                        {
                            lock_guard<mutex> guard(finishedMutex);
                            finishedProcesses.push_back(currentProcess);
                        }
                        memoryAllocator.deallocate(memory, currentProcess->getMemoryRequired(), config.memPerFrame);
                        currentProcess->storeMemory(nullptr);
                    } else {
                        lock_guard<mutex> guard(queueMutex);
                        processQueues[coreId].erase(processQueues[coreId].begin());
                        processQueues[coreId].push_back(currentProcess);
                    }
                }
                else {
                    lock_guard<mutex> guard(queueMutex);
                    processQueues[coreId].erase(processQueues[coreId].begin());
                    processQueues[coreId].push_back(currentProcess);
                }
            }
        }
    };

    for (int i = 0; i < numCores; ++i) {
        coreThreads.push_back(thread(coreFunction, i));
    }

    for (auto& t : coreThreads) {
        if (t.joinable()) {
            t.join();
        }
    }
    isRunning = false;
}

vector<Process*> RoundRobinScheduler::getFinishedProcesses() const {
    return finishedProcesses;
}

vector<vector<Process*>> RoundRobinScheduler::getProcessQueues() const {
    return processQueues;
}

bool RoundRobinScheduler::schedulerRunning() const {
    return isRunning;
}

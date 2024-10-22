#include <chrono>
#include <iostream>
#include "header/scheduler.h"

using namespace std;

FCFSScheduler::FCFSScheduler(int cores)
    : numCores(cores), processQueues(cores), isRunning(false) {}

void FCFSScheduler::addProcess(Process* process, int core) {
    if (core >= 0 && core < numCores) {
        lock_guard<mutex> guard(queueMutex);
        process->setCoreId(core);
        processQueues[core].push_back(process);
    }
}

void FCFSScheduler::runScheduler() {
    isRunning = true;
    auto coreFunction = [this](int coreId)  {
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
                    currentProcess->executeInstruction(coreId);
                    this_thread::sleep_for(chrono::milliseconds(100));
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
            } else {
                break;
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

RoundRobinScheduler::RoundRobinScheduler(int cores, int quantum) 
    : numCores(cores), quantumCycles(quantum), processQueues(cores), isRunning(false) {}

void RoundRobinScheduler::addProcess(Process* process, int core) {
    if (core >= 0 && core < numCores) {
        lock_guard<mutex> guard(queueMutex);
        process->setCoreId(core);
        processQueues[core].push_back(process);
    }
}

void RoundRobinScheduler::runScheduler() {
    isRunning = true;
    auto coreFunction = [this](int coreId) {
        while (isRunning) {
            Process* currentProcess = nullptr;

            {
                lock_guard<mutex> guard(queueMutex);
                if (!processQueues[coreId].empty()) {
                    currentProcess = processQueues[coreId].front();
                }
            }

            if (currentProcess) {
                int executedCycles = 0;
                
                while (!currentProcess->hasFinished() && executedCycles < quantumCycles) {
                    currentProcess->executeInstruction(coreId);
                    executedCycles++;
                    this_thread::sleep_for(chrono::milliseconds(100));
                }

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
                } else {
                    lock_guard<mutex> guard(queueMutex);
                    processQueues[coreId].erase(processQueues[coreId].begin());
                    processQueues[coreId].push_back(currentProcess);
                }
            } else {
                this_thread::sleep_for(chrono::milliseconds(100));
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

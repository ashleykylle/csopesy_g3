#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <string>
#include <fstream>
#include <ctime>

class Process {

private:
    int id;
    int remainingLines;

public:
    Process(int processId, int lines): id(processId), remainingLines(lines) {}

    void executeLine(int coreId) {
        if (remainingLines > 0){
            std::time_t now = std::time(nullptr);
            std::tm timeInfo;
            localtime_s(&timeInfo, &now);
            char buffer[30];
            std::strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S%p", &timeInfo); // (MM/DD/YYYY HH:MM:SSAM/PM)

            std::ofstream logFile("process_" + std::to_string(id) + ".txt", std::ios::app); // txt file
            logFile << "(" << buffer << ")  Core:" << coreId << " \"Hello world from screen_" << id << "!\"\n";
            logFile.close();

            --remainingLines;
        }
    }

    bool isFinished() const {
        return remainingLines == 0;
    }
    int getId() const {
        return id;
    }
};


class FCFS_Scheduler {

private:
    int numCores;
    std::vector<Process> allProcesses;

public:
    FCFS_Scheduler(int cores): numCores(cores) {}

    void addProcess(const Process& process) {
        allProcesses.push_back(process);  // push process at the very back (FCFS)
    }

    void runScheduler() {
        for (Process& process : allProcesses) {
            std::thread coreThreads[4]; // 4 threads for each core

            for (int core = 0; core < numCores; ++core) {
                coreThreads[core] = std::thread(&FCFS_Scheduler::coreWorker, this, core, std::ref(process)); 
            }

            for (int core = 0; core < numCores; ++core) {
                coreThreads[core].join();
            }

            std::cout << "process_" << process.getId() << " finished.\n";
        }
    }
    void coreWorker(int coreId, Process& process) {
        while (!process.isFinished()) {
            process.executeLine(coreId);  
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

int main() {
    FCFS_Scheduler scheduler(4);  //4 cores

    // create 10 processes each with 100 instructions
    for (int i = 1; i <= 10; ++i) {
        scheduler.addProcess(Process(i, 100));
    }

    scheduler.runScheduler();

    return 0;
}

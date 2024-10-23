#include <iostream>
#include <fstream>
#include <windows.h>
#include <thread>
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>
#include "header/process.h"
#include "header/scheduler.h"
#include "header/config.h"
#include "header/utils.h"

using namespace std;

bool osRunning = true;
bool isInitialized = false;
bool schedulerRunning = false;
bool inScreen = false;

void header() {
	setColor(0x07);
	cout << "  ____ ____  ____  _____ _____ ____ __   __     \n";
	cout << " / __/  ___|/ __ `|  _  ` ____/ ___`  ` / /     \n";
	cout << "| |   `___ ` |  | | |_| |  __|`___ ` `   /      \n";
	cout << "| |__ ___) | |__| | ___/| |___ ___) | | |       \n";
	cout << " `___` ____/`____/|_|   |_____|___ /  |_|       \n";
	cout << " -----------------------------------------------\n";
	
	setColor(0x02);
	cout << "Hello, Welcome to CSOPESY commandline!\n";
	setColor(0x0E);
    if (!isInitialized) {
        cout << "Type 'initialize' to initialize the processor, 'exit' to quit\n";
    } else {
        cout << "Type 'exit' to quit, 'clear' to clear the screen, '?' to view command list\n";
    }
	setColor(0x07);
}

void clear() {
	cout << "'clear' command recognized. Clearing screen.\n";
	system("cls");
	header();
}

void initialize(Config& config, Scheduler*& scheduler) {
    if (readConfig("config.txt", config)) {
        isInitialized = true;

        if (config.scheduler == "rr") {
            scheduler = new RoundRobinScheduler(config.numCpu, config.quantumCycles);
        } else if (config.scheduler == "fcfs") {
            scheduler = new FCFSScheduler(config.numCpu);
        }
        clear();

        thread schedulerThread([scheduler]() {
            scheduler->runScheduler();
        });
        schedulerThread.detach();
    }
}

void command_list() {
	setColor(0x0E);
	cout << "\nAvailable Commands:\n";
	
	if (inScreen) {
		setColor(0x02);
        cout << "'process-smi' - view process info\n";
		cout << "'exit' - return to the main menu\n";
		setColor(0x07);
	} else {
		setColor(0x02);
		cout << "'screen -r <screen name>' - load a screen\n";
		cout << "'screen -s <screen name>' - create/save a screen\n";
		cout << "'screen -ls' - show all running & finished processes\n";
		cout << "'scheduler-test' - generate dummy processes\n";
		cout << "'scheduler-stop' - stop generating processes\n";
		cout << "'report-util' - saves the process info in 'screen-ls' in a log file\n";
		cout << "'clear' - clear the screen\n";
		cout << "'exit' - exit the terminal\n\n";
		setColor(0x07);
	}
}

void screen_ls(Scheduler* scheduler, Config& config) {
    auto processQueues = scheduler->getProcessQueues();
    int coresUsed = 0;
    int coresAvailable = 0;
    int activeProcesses = 0;
    double cpuUtilization = 0.0;
    
    vector<Process*> allProcesses;
    for (const auto& coreQueue : processQueues) {
        activeProcesses += coreQueue.size();
        if (!coreQueue.empty()) {
            coresUsed++;
        }
        for (Process* process : coreQueue) {
            allProcesses.push_back(process);
        }
    }
    cpuUtilization = (config.numCpu > 0) ? (static_cast<double>(activeProcesses) / (config.numCpu * 2)) * 100 : 0;
    coresAvailable = config.numCpu - coresUsed;

    cout << "\nCPU utilization: " << cpuUtilization << "%\n";
    cout << "Cores used: " << coresUsed << "\n";
    cout << "Cores avaialable: " << coresAvailable << "\n";
    cout << "\n--------------------------------------\n";
    cout << "Running processes:\n";

    sort(allProcesses.begin(), allProcesses.end(), [](Process* a, Process* b) {
        return a->getId() < b->getId();
    });

    for (Process* process : allProcesses) {
        cout << process->getName() << "   (" << getCurrentTimestamp() << ")   Core: " 
            << process->getCoreId()
            << "   " << process->getTotalInstructions() - process->getRemainingInstructions() 
            << " / " << process->getTotalInstructions() << "\n";
    }

    cout << "\nFinished processes:\n";
    auto finishedProcesses = scheduler->getFinishedProcesses();
    
    for (Process* process : finishedProcesses) {
        cout << process->getName() << "   (" << process->getCompletionTimestamp() << ")   Finished   "
            << process->getTotalInstructions() << " / " << process->getTotalInstructions() << "\n";
    }

    cout << "--------------------------------------\n\n";
}

void scheduler_test(Scheduler* scheduler, Config& config, int& cpuCycles) { 
    static int currentCore = 0;
    static int processId = 1;
    schedulerRunning = true;

    while (schedulerRunning) {
        if (cpuCycles % config.batchProcessFreq == 0) {
            int assignedCore = currentCore++ % config.numCpu;
            string processName = "P" + to_string(processId);

            scheduler->addProcess(new Process(processName, processId, config.minIns + (rand() % (config.maxIns - config.minIns + 1))), assignedCore);
            processId++;
        }
        this_thread::sleep_for(chrono::milliseconds(config.delayPerExec));
        cpuCycles++;
    }
}

void scheduler_stop() {
	cout << "'scheduler-stop' command recognized. Process generation has stopped.\n";
    schedulerRunning = false;
}

void report_util(Scheduler* scheduler, Config& config) {
    cout << "\n'report-util' command recognized. Generating log file...\n";

    ofstream logFile("csopesy-log.txt");
    if (!logFile.is_open()) {
        cout << "Failed to open log file.\n";
        return;
    }

    auto processQueues = scheduler->getProcessQueues();
    int coresUsed = 0;
    int coresAvailable = 0;
    int activeProcesses = 0;
    double cpuUtilization = 0.0;
    
    vector<Process*> allProcesses;
    for (const auto& coreQueue : processQueues) {
        activeProcesses += coreQueue.size();
        if (!coreQueue.empty()) {
            coresUsed++;
        }
        for (Process* process : coreQueue) {
            allProcesses.push_back(process);
        }
    }
    cpuUtilization = (config.numCpu > 0) ? (static_cast<double>(activeProcesses) / (config.numCpu * 2)) * 100 : 0;
    coresAvailable = config.numCpu - coresUsed;

    logFile << "\nCPU utilization: " << cpuUtilization << "%\n";
    logFile << "Cores used: " << coresUsed << "\n";
    logFile << "Cores avaialable: " << coresAvailable << "\n";
    logFile << "--------------------------------------\n";
    logFile << "Running processes:\n";

    sort(allProcesses.begin(), allProcesses.end(), [](Process* a, Process* b) {
        return a->getId() < b->getId();
    });

    for (Process* process : allProcesses) {
        logFile << process->getName() << "   (" << getCurrentTimestamp() << ")   Core: " 
                << process->getCoreId()
                << "   " << process->getTotalInstructions() - process->getRemainingInstructions() 
                << " / " << process->getTotalInstructions() << "\n";
    }

    logFile << "\nFinished processes:\n";
    auto finishedProcesses = scheduler->getFinishedProcesses();
    
    for (Process* process : finishedProcesses) {
        logFile << process->getName() << "   (" << process->getCompletionTimestamp() << ")   Finished   "
                << process->getTotalInstructions() << " / " << process->getTotalInstructions() << "\n";
    }

    logFile << "--------------------------------------";
    logFile.close();
    cout << "Report generated at C:/csopesy-log.txt!\n\n";
}

void exit() {
	cout << "'exit' command recognized. Exiting program.\n";
    osRunning = false;
}

int main() {
	system("cls");
	header();

	Config config;
    Scheduler* scheduler = nullptr;
    string input;
    int cpuCycles = 0;
    thread processThread;
	
	while (osRunning) {
		cout << "Enter command: ";
		getline(cin, input);
		pair<string, string> parsed = parseCommand(input);
		string cmd = parsed.first;
		string arg = parsed.second;
		
        if (!isInitialized) {
            if (cmd == "initialize") {
			    initialize(config, scheduler);
            } else if (cmd == "exit") {
                exit();
            } else {
                cout << "Command '" << cmd << "' not recognized." << "\n";
            }
        } else {
            if (cmd == "screen") {
                if (arg.substr(0, 2) == "-r" || arg.substr(0, 2) == "-s") {
                    cout << "'" << cmd << " " << arg << "' command recognized. Doing something.\n";
                } else if (arg == "-ls") {
                    screen_ls(scheduler, config);
                } else {
                    cout << "Command '" << cmd << " " << arg << "' not recognized." << "\n";
                }
            } else if (cmd == "scheduler-test") {
                processThread = thread(scheduler_test, ref(scheduler), ref(config), ref(cpuCycles));
            } else if (cmd == "scheduler-stop") {
                scheduler_stop();
                if (processThread.joinable()) {
                    processThread.join();
                }
            } else if (cmd == "report-util") {
                report_util(scheduler, config);
            } else if (cmd == "clear") {
                clear();
            } else if (cmd == "exit") {
                exit();
            } else if (cmd == "?") {
                command_list();
            } else {
                cout << "Command '" << cmd << "' not recognized." << "\n";
            }
        }
	}

	return 0;
}

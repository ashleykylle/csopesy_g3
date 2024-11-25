#include <iostream>
#include <fstream>
#include <windows.h>
#include <direct.h>
#include <iomanip>
#include <map>
#include <thread>
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>
#include "header/process.h"
#include "header/scheduler.h"
#include "header/config.h"
#include "header/utils.h"
#include "header/screen.h"
#include "header/memory.h"

using namespace std;

bool osRunning = true;
bool isInitialized = false;
bool schedulerRunning = false;

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

void incrementCpuCycles(int& cpuCycles) {
    while (osRunning) {
        cpuCycles++;
    }
}

void initialize(Config& config, Scheduler*& scheduler, IMemoryAllocator*& memoryAllocator, int& cpuCycles) {
    if (readConfig("config.txt", config)) {
        isInitialized = true;
        size_t maxFrames = config.maxOverallMem / config.memPerFrame;
        
        if (config.maxOverallMem == config.memPerFrame) {
            memoryAllocator = new FlatMemoryAllocator(maxFrames);
        } else {
            memoryAllocator = new PagingAllocator(maxFrames);
        }

        if (config.scheduler == "rr") {
            scheduler = new RoundRobinScheduler(config.numCpu, config.quantumCycles, *memoryAllocator);
        } else if (config.scheduler == "fcfs") {
            scheduler = new FCFSScheduler(config.numCpu, *memoryAllocator);
        }
        clear();

        thread cpuCyleThread(incrementCpuCycles, ref(cpuCycles));
        cpuCyleThread.detach();

        thread schedulerThread([&scheduler, &config, &cpuCycles, &memoryAllocator]() {
            scheduler->runScheduler(config, cpuCycles, *memoryAllocator);
        });
        schedulerThread.detach();

        const char* folderName = "logs";
        if (_mkdir(folderName) == 0) {
            cout << "Logs folder created successfully.\n";
        } else {
            perror("Error creating folder");
        }
        folderName = "back_storage";

        if (_mkdir(folderName) == 0) {
            cout << "Back storage folder created successfully.\n";
        } else {
            perror("Error creating folder");
        }
    }
}

void command_list() {
	setColor(0x0E);
	cout << "\nAvailable Commands:\n";
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

void exit_screen() {
    system("cls");
    header();
}

void screen_s(const string& screenName, Config& config, Scheduler* scheduler, int& cpuCycles) {
    static int processId = 1;
    int assignedCore = 0;   
    string processName = screenName;
    int numInstructions = config.minIns + (rand() % (config.maxIns - config.minIns + 1));
    int memPerProc = config.minMemPerProc + (rand() % (config.maxMemPerProc - config.minMemPerProc + 1));
    size_t numPages = memPerProc / config.memPerFrame;
    vector<int> pageIndices(numPages, 0);

    if (screens.find(screenName) != screens.end()) {
		cout << "Process creation failed. Process name " << screenName << " already exists!\n";
		return;
	}

    Process* newProcess = new Process(processName, processId, numInstructions, memPerProc, pageIndices);
    scheduler->addProcess(newProcess, assignedCore);
    Screen* newScreen = new Screen(screenName, newProcess);
    screens[screenName] = newScreen;

    newScreen->display();

    processId++;
    exit_screen();
}

void screen_r(const string& screenName) {
    if (screens.find(screenName) != screens.end()) {
        screens[screenName]->display();
        exit_screen();
    }
    else {
        cout << "Process " << screenName << " not found." << endl;
    }
}

void screen_ls(Scheduler* scheduler, Config& config) {
    auto processQueues = scheduler->getProcessQueues();
    int coresUsed = 0;
    int coresAvailable = 0;
    double cpuUtilization = 0.0;
    
    vector<Process*> allProcesses;
    for (const auto& coreQueue : processQueues) {
        if (!coreQueue.empty()) {
            coresUsed++;
        }
        for (Process* process : coreQueue) {
            allProcesses.push_back(process);
        }
    }
    cpuUtilization = (coresUsed / config.numCpu) * 100;
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
            int numInstructions = config.minIns + (rand() % (config.maxIns - config.minIns + 1));
            int memPerProc = config.minMemPerProc + (rand() % (config.maxMemPerProc - config.minMemPerProc + 1));
            size_t numPages = (memPerProc + config.memPerFrame - 1) / config.memPerFrame;
            vector<int> pageIndices;
            for (size_t i = 0; i < numPages; ++i) {
                pageIndices.push_back(i);
            }
            scheduler->addProcess(new Process(processName, processId, numInstructions, memPerProc, pageIndices), assignedCore);
            processId++;
        }
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
    double cpuUtilization = 0.0;
    
    vector<Process*> allProcesses;
    for (const auto& coreQueue : processQueues) {
        if (!coreQueue.empty()) {
            coresUsed++;
        }
        for (Process* process : coreQueue) {
            allProcesses.push_back(process);
        }
    }
    cpuUtilization = (coresUsed / config.numCpu) * 100;
    coresAvailable = config.numCpu - coresUsed;

    logFile << "CPU utilization: " << cpuUtilization << "%\n";
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
    IMemoryAllocator* memoryAllocator = nullptr;
    string input;
    thread processThread;
    int cpuCycles = 0;
	
	while (osRunning) {
		cout << "Enter command: ";
		getline(cin, input);
		pair<string, string> parsed = parseCommand(input);
		string cmd = parsed.first;
		string arg = parsed.second;
		
        if (!isInitialized) {
            if (cmd == "initialize") {
			    initialize(config, scheduler, memoryAllocator, cpuCycles);
            } else if (cmd == "exit") {
                exit();
            } else {
                cout << "Command '" << cmd << "' not recognized." << "\n";
            }
        } else {
            if (cmd == "screen") {
                if (arg.substr(0, 2) == "-s") {
                    string screenName = arg.substr(3);
		            screen_s(screenName, ref(config), ref(scheduler), ref(cpuCycles));
                } else if (arg.substr(0, 2) == "-r") {
                    string screenName = arg.substr(3);
                    screen_r(screenName);
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

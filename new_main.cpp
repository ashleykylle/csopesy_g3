#include <iostream>
#include <fstream>
#include <windows.h>
#include <iomanip>
#include <map>
#include <thread>
#include <string>
#include <atomic>
#include <chrono>
#include <vector>
#include <mutex>
#include <algorithm>

using namespace std;

string getCurrentTimestamp() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%m/%d/%Y, %I:%M:%S %p", &tstruct);
    return buf;
}

class Process {
private:
    string name;
    int id;
    int coreId;
    int totalInstructions;
    int remainingInstructions;
    ofstream outputFile;
    mutex fileMutex;
    atomic<bool> isFinished;
    string completionTimestamp;

public:
    Process(const string& processName, int processId, int numInstructions)
        : name(processName), id(processId), totalInstructions(numInstructions), 
          remainingInstructions(numInstructions), isFinished(false) {
        outputFile.open("process" + to_string(id) + ".txt", ios::out);
        outputFile << "Process name: " << name << "\nLogs:\n\n";
    }

    ~Process() {
        outputFile.close();
    }

    void executeInstruction(int coreId) {
        if (remainingInstructions > 0) {
            auto now = chrono::system_clock::now();
            time_t now_c = chrono::system_clock::to_time_t(now);
            tm* now_tm = localtime(&now_c);

            char timestamp[30];
            strftime(timestamp, sizeof(timestamp), "%m/%d/%Y %I:%M:%S%p", now_tm);

            {
                lock_guard<mutex> guard(fileMutex);
                outputFile << "(" << timestamp << ") Core:" << coreId
                           << " 'Hello world from " << name << "'\n";
            }
            remainingInstructions--;
        } else {
            cout << "Process " << id << ": " << name << " has already finished.\n";
        }
    }
    
    void markAsFinished() {
        isFinished = true;
        completionTimestamp = getCurrentTimestamp();
    }
    
    void setCoreId(int core) {
        coreId = core;
    }

    bool hasFinished() const {
        return remainingInstructions == 0;
    }
    
    string getName() const {
        return name;
    }
    
    string getCompletionTimestamp() const {
        return completionTimestamp;
    }
    
    int getRemainingInstructions() const {
        return remainingInstructions;
    }

    int getId() const {
        return id;
    }

    int getTotalInstructions() const {
        return totalInstructions;
    }

    int getCoreId() const {
        return coreId;
    }
};

class FCFSScheduler {
private:
    int numCores;
    vector<thread> coreThreads;
    vector<vector<Process*>> processQueues;
    vector<Process*> finishedProcesses;
    mutex queueMutex;
    mutex finishedMutex;
    atomic<bool> isRunning;

public:
    FCFSScheduler(int cores) : numCores(cores), processQueues(cores), isRunning(false) {}

    void addProcess(Process* process, int core) {
        if (core >= 0 && core < numCores) {
            lock_guard<mutex> guard(queueMutex);
            process->setCoreId(core);
            processQueues[core].push_back(process);
        }
    }

    void runScheduler() {
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
    
    vector<Process*> getFinishedProcesses() const {
        return finishedProcesses;
    }

    vector<vector<Process*>> getProcessQueues() const {
        return processQueues;
    }

    bool schedulerRunning() const {
        return isRunning;
    }
};

struct ScreenData {
	string processName;
	int currentLine;
	int totalLines;
	string timestamp;
};

map<string, ScreenData> screens;
bool inScreen = false;

void setColor(unsigned char color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

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
	cout << "Type 'exit' to quit, 'clear' to clear the screen, '?' to view command list\n";
	setColor(0x07);
}

void initialize() {
	cout << "'initialize' command recognized. Doing something.\n";
}

void screen_r(const string& name) {
	if (inScreen == false && (screens.find(name) != screens.end())) {
		system("cls");
		ScreenData& screen = screens[name];
		screen.currentLine++;
		cout << "Process Name: " << screen.processName << "\n";
		cout << "Current line: " << screen.currentLine << "/" << screen.totalLines << "\n";
		cout << "Timestamp: " << screen.timestamp << "\n\n";
		setColor(0x0E);
		cout << "Type 'exit' to go back to the main menu, '?' to view command list\n";
		setColor(0x07);
		inScreen = true;
	} else if(inScreen == true) {
		cout << "Command not recognized.\n";
	} else {
		cout << "Screen '" << name << "' not found.\n";
	}
}

void screen_s(const string& name) {
	if (inScreen == false) {
		system("cls");
		ScreenData newScreen;
		newScreen.processName = name;
		newScreen.currentLine = 0;
		newScreen.totalLines = 100;
		newScreen.timestamp = getCurrentTimestamp();

		screens[name] = newScreen;
		cout << "Process Name: " << newScreen.processName << "\n";
		cout << "Current line: " << newScreen.currentLine << "/" << newScreen.totalLines << "\n";
		cout << "Timestamp: " << newScreen.timestamp << "\n\n";
		setColor(0x0E);
		cout << "Type 'exit' to go back to the main menu, '?' to view command list\n";
		setColor(0x07);
		inScreen = true;
	} else {
		cout << "Command not recognized.\n";
	}

}

void screen_ls(const FCFSScheduler& scheduler) {
    cout << "\n--------------------------------------\n";
    
    cout << "Running processes:\n";
    auto processQueues = scheduler.getProcessQueues();
    
    vector<Process*> allProcesses;
    for (const auto& coreQueue : processQueues) {
        for (Process* process : coreQueue) {
            allProcesses.push_back(process);
        }
    }

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
    auto finishedProcesses = scheduler.getFinishedProcesses();
    
    for (Process* process : finishedProcesses) {
        cout << process->getName() << "   (" << process->getCompletionTimestamp() << ")   Finished   "
             << process->getTotalInstructions() << " / " << process->getTotalInstructions() << "\n";
    }

    cout << "--------------------------------------\n\n";
}


void scheduler_test() {
	cout << "'scheduler-test' command recognized. Doing something.\n";
}

void scheduler_stop() {
	cout << "'scheduler-stop' command recognized. Doing something.\n";
}

void report_util() {
	cout << "'report-util' command recognized. Doing something.\n";
}

void clear() {
	cout << "'clear' command recognized. Clearing screen.\n";
	#ifdef _WIN32
	system("cls"); // Windows-specific clear command
	#else
	system("clear"); // Unix/Linux/Mac-specific clear command
	#endif
	header();
}

void exit_screen() {
	inScreen = false;
	system("cls");
	header();
}

void exit() {
	if (inScreen) {
        exit_screen();
    } else {
        cout << "'exit' command recognized. Exiting program.\n";
        exit(0);
    }
}

void command_list() {
	setColor(0x0E);
	cout << "\nAvailable Commands:\n";
	
	if (inScreen) {
		setColor(0x02);
		cout << "'exit' - return to the main menu\n";
		setColor(0x07);
	} else {
		setColor(0x02);
		cout << "'initialize'\n";
		cout << "'screen -r <screen name>' - load a screen\n";
		cout << "'screen -s <screen name>' - create/save a screen\n";
		cout << "'screen -ls' - show all running & finished processes\n";
		cout << "'scheduler-test'\n";
		cout << "'scheduler-stop'\n";
		cout << "'report-util'\n";
		cout << "'print' - print processes in a text file\n";
		cout << "'clear' - clear the screen\n";
		cout << "'exit' - exit the terminal\n";
		setColor(0x07);
	}
}

pair<string, string> parseCommand(const string& input) {
	istringstream stream(input);
	string cmd, argument;
	stream >> cmd;
	getline(stream, argument);
	if (!argument.empty() && argument[0] == ' ') {
		argument = argument.substr(1);
	}
	return {cmd, argument};
}

int main() {
	system("cls");
	header();

	string input;
	int numCores = 4;
    int assignedCore = 0;
	FCFSScheduler scheduler(numCores);
	
	for (int i = 1; i <= 10; ++i) {
        scheduler.addProcess(new Process("Process " + to_string(i), i, 100), assignedCore);
        assignedCore = (assignedCore + 1) % numCores;
    }

	thread schedulerThread;
	
	while (true) {
		cout << "Enter command: ";
		getline(cin, input);
		pair<string, string> parsed = parseCommand(input);
		string cmd = parsed.first;
		string arg = parsed.second;
		
		if (cmd == "initialize") {
			initialize();
		} else if (cmd == "screen") {
			if (arg.substr(0, 2) == "-r") {
				screen_r(arg.substr(3));
			} else if (arg.substr(0, 2) == "-s") {
				screen_s(arg.substr(3));
			} else if (arg == "-ls") {
				screen_ls(scheduler);
			} else {
				cout << "Unrecognized command: " << cmd << " " << arg << "\n";
			}
		} else if (cmd == "scheduler-test") {
			scheduler_test();
		} else if (cmd == "scheduler-stop") {
			scheduler_stop();
		} else if (cmd == "report-util") {
			report_util();
		} else if (cmd == "print") {
			if (schedulerThread.joinable()) {
                cout << "The scheduler is already running!\n";
            } else {
                schedulerThread = thread(&FCFSScheduler::runScheduler, &scheduler);
            }
		} else if (cmd == "clear") {
			clear();
		} else if (cmd == "exit") {
			exit();
		} else if (cmd == "?") {
			command_list();
		} else {
			cout << "Unrecognized command: " << cmd << "\n";
		}
	}

	return 0;
}

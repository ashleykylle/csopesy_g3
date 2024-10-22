#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include "header/config.h"

using namespace std;

bool readConfig(const string& filename, Config& config) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to open config file.\n";
        return false;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string key;
        if (iss >> key) {
            if (key == "num-cpu") {
                iss >> config.numCpu;
            } else if (key == "scheduler") {
                string schedulerValue;
                iss >> quoted(schedulerValue);
                config.scheduler = schedulerValue;
            } else if (key == "quantum-cycles") {
                iss >> config.quantumCycles;
            } else if (key == "batch-process-freq") {
                iss >> config.batchProcessFreq;
            } else if (key == "min-ins") {
                iss >> config.minIns;
            } else if (key == "max-ins") {
                iss >> config.maxIns;
            } else if (key == "delay-per-exec") {
                iss >> config.delayPerExec;
            }
        }
    }

    file.close();
    return true;
}
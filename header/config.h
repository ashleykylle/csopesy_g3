#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using namespace std;

struct Config {
    string scheduler;
    int numCpu;
    int quantumCycles;
    int batchProcessFreq;
    int minIns;
    int maxIns;
    int delayPerExec;
};

bool readConfig(const string& filename, Config& config);

#endif
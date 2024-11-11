#include "header/memory.h"
#include "header/utils.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize, size_t maxProcess)
    : maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.'), allocationMap(maximumSize, false), processMap(maxProcess, "0") {}

FlatMemoryAllocator::~FlatMemoryAllocator() {
    memory.clear();
}

void* FlatMemoryAllocator::allocate(size_t size, size_t frame, string name) {
    size_t framesRequired = (size + frame - 1) / frame;
    for (size_t i = 0; i < maximumSize; ++i) {
        if (!allocationMap[i] && canAllocateAt(i, framesRequired)) {
            allocateAt(i, framesRequired, name);
            return &memory[i];
        }
    }
    return nullptr;
}

void FlatMemoryAllocator::deallocate(void* ptr, size_t size, size_t frame) {
    size_t index = static_cast<char*>(ptr) - &memory[0];
    size_t framesRequired = (size + frame - 1) / frame;
    deallocateAt(index, framesRequired);
}

vector<int> FlatMemoryAllocator::visualizeMemory(size_t size, size_t frame) {
    vector<int> indices;
    size_t framesRequired = (size + frame - 1) / frame;

    for (size_t i = 0; i < maximumSize; ++i) {
        if (allocationMap[i]) {
            indices.push_back(i);
            i = i + framesRequired - 1;
        }
    }
    return indices;
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t framesRequired) const {
    for (size_t i = 1; i < framesRequired; ++i) {
        if (index + i >= maximumSize || allocationMap[index + i]) {
            return false;
        }
    }
    return true;
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t framesRequired, string name) {
    size_t pIndex = index / framesRequired;

    fill(allocationMap.begin() + index, allocationMap.begin() + index + framesRequired, true);
    fill(memory.begin() + index, memory.begin() + index + framesRequired, '#');
    processMap[pIndex] = name;
    allocatedSize += framesRequired;
}

void FlatMemoryAllocator::deallocateAt(size_t index, size_t framesRequired) {
    size_t pIndex = index / framesRequired;

    fill(allocationMap.begin() + index, allocationMap.begin() + index + framesRequired, false);
    fill(memory.begin() + index, memory.begin() + index + framesRequired, '.');
    processMap[pIndex] = "0";
    allocatedSize -= framesRequired;
}

size_t FlatMemoryAllocator::calculateExternalFragmentation(size_t frame) const {
    size_t fragmentation = 0;

    for (size_t i = 0; i < allocationMap.size(); ++i) {
        if (!allocationMap[i]) {
             fragmentation++;
        }
    }
    return fragmentation * frame;

}

int FlatMemoryAllocator::countProcessesInMemory(size_t size, size_t frame) const {
    size_t framesRequired = (size + frame - 1) / frame;
    int processCount = 0;
    int fragmentCount = 0;

    for (size_t i = 0; i < allocationMap.size(); ++i) {
        if (allocationMap[i]) {
            fragmentCount++;
            if (fragmentCount >= framesRequired) {
                processCount++;
                fragmentCount = 0;
            }
        }
        else {
            fragmentCount = 0;
        }
    }
    return processCount;

}

void FlatMemoryAllocator::logMemoryStamp(int cycleNumber, size_t size, size_t frame) {
    string filename = "logs/memory_stamp_" + to_string(cycleNumber) + ".txt";
    size_t maxMemory = maximumSize * frame;
    size_t framesRequired = (size + frame - 1) / frame;
    size_t pIndex;
    vector<int> indices = visualizeMemory(size, frame);

    ofstream logFile(filename);
    if (!logFile.is_open()) {
        cout << "Failed to open log file.\n";
        return;
    }

    logFile << "Timestamp: " << "   (" << getCurrentTimestamp() << ")" << endl;
    logFile << "Number of processes in memory: " << countProcessesInMemory(size, frame) << endl;

    size_t externalFrag = calculateExternalFragmentation(frame);
    logFile << "Total external fragmentation in KB: " << externalFrag << endl << endl;
    logFile << "----end---- = " << maxMemory << endl << endl;

    for (size_t i = indices.size(); i > 0; --i) {
        pIndex = indices[i - 1] / framesRequired;
        indices[i - 1] *= frame;
        logFile << (indices[i - 1] + size) << endl;
        logFile << processMap[pIndex] << endl;
        logFile << indices[i - 1] << endl << endl;
    }
    
    logFile << "----start---- = 0" << endl;
    logFile.close();
}

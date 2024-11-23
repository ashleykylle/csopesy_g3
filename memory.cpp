#include "header/memory.h"
#include "header/utils.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize)
    : maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.'), allocationMap(maximumSize, false), 
    processMap(maximumSize, "0") {}

FlatMemoryAllocator::~FlatMemoryAllocator() {
    memory.clear();
}

void* FlatMemoryAllocator::allocate(Process* process) {
    size_t framesRequired = process->getNumPages();
    for (size_t i = 0; i < maximumSize; ++i) {
        if (!allocationMap[i] && canAllocateAt(i, framesRequired)) {
            allocateAt(i, framesRequired, process->getName());
            return &memory[i];
        }
    }
    return nullptr;
}

void FlatMemoryAllocator::deallocate(Process* process) {
    void* ptr = process->getAllocatedMemory();
    size_t index = static_cast<char*>(ptr) - &memory[0];
    size_t framesRequired = process->getNumPages();
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
    fill(allocationMap.begin() + index, allocationMap.begin() + index + framesRequired, true);
    fill(memory.begin() + index, memory.begin() + index + framesRequired, '#');
    fill(processMap.begin() + index, processMap.begin() + index + framesRequired, name);
    allocatedSize += framesRequired;
}

void FlatMemoryAllocator::deallocateAt(size_t index, size_t framesRequired) {
    fill(allocationMap.begin() + index, allocationMap.begin() + index + framesRequired, false);
    fill(memory.begin() + index, memory.begin() + index + framesRequired, '.');
    fill(processMap.begin() + index, processMap.begin() + index + framesRequired, "0");
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

int FlatMemoryAllocator::countProcessesInMemory(Process* process) const {
    size_t framesRequired = process->getNumPages();
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

void FlatMemoryAllocator::logMemoryStamp(int cycleNumber, size_t frame, Process* process) {
    string filename = "logs/memory_stamp_" + to_string(cycleNumber) + ".txt";
    size_t maxMemory = maximumSize * frame;
    size_t size = process->getMemoryRequired();
    vector<int> indices = visualizeMemory(size, frame);

    ofstream logFile(filename);
    if (!logFile.is_open()) {
        cout << "Failed to open log file.\n";
        return;
    }

    logFile << "Timestamp: " << "   (" << getCurrentTimestamp() << ")" << endl;
    logFile << "Number of processes in memory: " << countProcessesInMemory(process) << endl;

    size_t externalFrag = calculateExternalFragmentation(frame);
    logFile << "Total external fragmentation in KB: " << externalFrag << endl << endl;
    logFile << "----end---- = " << maxMemory << endl << endl;

    for (size_t i = indices.size(); i > 0; --i) {
        size_t pIndex = indices[i-1];
        indices[i - 1] *= frame;
        
        logFile << (indices[i - 1] + size) << endl;
        logFile << processMap[pIndex] << endl;
        logFile << indices[i - 1] << endl << endl;
    }
    
    logFile << "----start---- = 0" << endl;
    logFile.close();
}

PagingAllocator::PagingAllocator(size_t maxMemorySize)
    : maxMemorySize(maxMemorySize), numFrames(maxMemorySize) {
        for (size_t i = 0; i < numFrames; ++i) {
            freeFrameList.push_back(i+1);
        }
    }

void* PagingAllocator::allocate(Process* process) {
    size_t processId = process->getId();
    size_t numFramesNeeded = process->getNumPages();

    if (numFramesNeeded > freeFrameList.size()) {
        return nullptr;
    }
    size_t frameIndex = allocateFrames(numFramesNeeded, processId);

    return reinterpret_cast<void*>(frameIndex);
}

void PagingAllocator::deallocate(Process* process) {
    size_t processId = process->getId();
    size_t numFramesNeeded = process->getNumPages();
    vector<size_t> frameIndices;
    
    for (const auto& entry : frameMap) {
        if (entry.second == processId) {
            frameIndices.push_back(entry.first);
        }
    }
    deallocateFrames(numFramesNeeded, frameIndices);
}

vector<int> PagingAllocator::visualizeMemory(size_t size, size_t frame) {}

// void PagingAllocator::visualizeMemory() const {
//     cout << "Memory visualization:\n";
//     for (size_t frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
//         auto it = frameMap.find(frameIndex);

//         if (it != frameMap.end()) {
//             cout << "Frame" << frameIndex << " -> Process " << it->second << endl;
//         } else {
//             cout << "Frame" << frameIndex << " -> Free\n";
//         }
//     }
//     cout << "----------------------\n";
// }

size_t PagingAllocator::allocateFrames(size_t numFrames, size_t processId) {
    size_t frameIndex = freeFrameList.back();

    for (size_t i = 0; i < numFrames; ++i) {
        frameMap[freeFrameList.back()] = processId;
        freeFrameList.pop_back();
    }
    return frameIndex;
}

void PagingAllocator::deallocateFrames(size_t numFrames, vector<size_t> frameIndices) {
    for (size_t i = 0; i < numFrames; ++i) {
        frameMap.erase(frameIndices[i]);
    }

    for (size_t i = 0; i < numFrames; ++i) {
        freeFrameList.push_back(frameIndices[i]);
    }
}

void PagingAllocator::logMemoryStamp(int cycleNumber, size_t frame, Process* process) {}

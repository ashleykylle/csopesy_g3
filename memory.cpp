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
            processesInMemory.push_back(process);
            return &memory[i];
        }
    }
    return nullptr;
}

void FlatMemoryAllocator::deallocate(Process* process) {
    void* ptr = process->getAllocatedMemory();
    size_t index = static_cast<char*>(ptr) - &memory[0];
    size_t framesRequired = process->getNumPages();

    auto it = find(processesInMemory.begin(), processesInMemory.end(), process);
    if (it != processesInMemory.end()) {
        processesInMemory.erase(it);
    }
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

void* FlatMemoryAllocator::handleMemoryFull(Process* currentProcess) {
    void* memory;
    bool notAllocated = true;
    // Step 1: Handle when should the loop stop
    while (notAllocated) {
        // Step 2: Get the oldest process
        Process* oldestProcess = processesInMemory.front();

        if (oldestProcess) {
            // Step 3: Deallocate the oldest process, then allocate the current process
            {
                lock_guard<mutex> memoryGuard(memoryMutex);
                deallocate(oldestProcess);
                oldestProcess->storeMemory(nullptr);
                memory = allocate(currentProcess);
                currentProcess->storeMemory(memory);
            }
            // Step 4: If allocation is successful, end the loop and perform back storing
            if (memory) {
                notAllocated = false;
                string filename = "back_storage/" + oldestProcess->getName() + ".txt";
                oldestProcess->storeToBackStorage(filename);
            }
        }
    }
    return memory;
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
    size_t numFramesNeeded = process->getNumPages() - process->getPageTable().size();

    if (numFramesNeeded > freeFrameList.size()) {
        return nullptr;
    }
    size_t frameIndex = allocateFrames(numFramesNeeded, process);
    processesInMemory.push_back(process);

    return reinterpret_cast<void*>(frameIndex);
}

void PagingAllocator::deallocate(Process* process) {
    size_t processId = process->getId();
    
    vector<size_t> frameIndices;
    
    for (const auto& entry : frameMap) {
        if (entry.second == processId) {
            frameIndices.push_back(entry.first);
        }
    }
    auto it = find(processesInMemory.begin(), processesInMemory.end(), process);
    if (it != processesInMemory.end()) {
        processesInMemory.erase(it);
    }
    deallocateFrames(frameIndices, process);
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

size_t PagingAllocator::allocateFrames(size_t numFrames, Process* process) {
    unordered_map<size_t, size_t> pageTable = process->getPageTable();

    for (size_t i = 0; i < numFrames; ++i) {
        frameMap[freeFrameList.back()] = process->getId();
        pageTable[i] = freeFrameList.back();
        freeFrameList.pop_back();
    }
    process->storePageTable(pageTable);
    size_t frameIndex = pageTable.begin()->second;

    return frameIndex;
}

void PagingAllocator::deallocateFrames(vector<size_t> frameIndices, Process* process) {
    size_t numFramesNeeded = process->getNumPages();
    unordered_map<size_t, size_t> pageTable = process->getPageTable();
    
    for (size_t i = 0; i < numFrames; ++i) {
        frameMap.erase(frameIndices[i]);
        pageTable.erase(i);
    }
    process->storePageTable(pageTable);

    for (size_t i = 0; i < numFrames; ++i) {
        freeFrameList.push_back(frameIndices[i]);
    }
}

void* PagingAllocator::handleMemoryFull(Process* currentProcess) {
    void* memory;
    size_t frameIndex;
    bool notAllocated = true;
    // Step 1: Handle when should the loop stop
    while (notAllocated) {
        // Step 2: Make sure to use all free frames first before replacing pages
        lock_guard<mutex> memoryGuard(memoryMutex);
        unordered_map<size_t, size_t> currentPageTable = currentProcess->getPageTable();

        while (!freeFrameList.empty()) {
            frameMap[freeFrameList.back()] = currentProcess->getId();
            currentPageTable[currentPageTable.size()] = freeFrameList.back();
            freeFrameList.pop_back();
        }
        currentProcess->storePageTable(currentPageTable);

        // Step 3: Get the oldest process
        Process* oldestProcess = nullptr;
        if (!processesInMemory.empty()) {
            oldestProcess = processesInMemory.front();
        }

        // Step 4: Get the current process's page table
        currentPageTable = currentProcess->getPageTable();
        size_t numFramesNeeded = currentProcess->getNumPages();

        // Step 5: Check if all pages have been loaded into memory
        if (numFramesNeeded > currentPageTable.size()) {
            if (oldestProcess) {
                // Step 6: Get the oldest page of the oldest process
                unordered_map<size_t, size_t> oldestPageTable = oldestProcess->getPageTable();
                auto it = oldestPageTable.begin();
                size_t oldestPageIndex = it->first;
                size_t oldestPageFrame = it->second;
                
                // Step 7: Swap the pages between the oldest process and the current process
                frameMap[oldestPageFrame] = currentProcess->getId();

                // Swap in the page table of both processes
                currentPageTable[currentPageTable.size()] = oldestPageFrame;
                oldestPageTable.erase(oldestPageIndex);

                // Step 8: Store the updated page tables back to both processes
                currentProcess->storePageTable(currentPageTable);
                oldestProcess->storePageTable(oldestPageTable);

                // Step 9: If the oldest process has no more pages, remove it from the queue and perform back storing
                if (oldestPageTable.empty()) {
                    auto it = find(processesInMemory.begin(), processesInMemory.end(), oldestProcess);
                    if (it != processesInMemory.end()) {
                        oldestProcess->storeMemory(nullptr);
                        string filename = "back_storage/" + oldestProcess->getName() + ".txt";
                        oldestProcess->storeToBackStorage(filename);
                        processesInMemory.erase(it);
                    }
                }
            }
        } 
        // All pages are loaded into memory
        else {
            frameIndex = currentPageTable.begin()->second;
            memory = reinterpret_cast<void*>(frameIndex);
            currentProcess->storeMemory(memory);
            processesInMemory.push_back(currentProcess);
            if (memory) {
                // Step 10: If allocation is successful, end the loop and perform back storing if necessary
                notAllocated = false;
                if (oldestProcess) {
                    // At least 1 page is removed from memory
                    if (oldestProcess->getNumPages() != oldestProcess->getPageTable().size()) {
                        oldestProcess->storeMemory(nullptr);
                        string filename = "back_storage/" + oldestProcess->getName() + ".txt";
                        oldestProcess->storeToBackStorage(filename);
                    }
                }
            }
        }
    }
    return memory;
}

void PagingAllocator::logMemoryStamp(int cycleNumber, size_t frame, Process* process) {}

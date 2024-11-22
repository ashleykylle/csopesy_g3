#ifndef MEMORY_H
#define MEMORY_H

#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>
#include "process.h"

using namespace std;

class IMemoryAllocator {
public:
    virtual void* allocate(Process* process) = 0;
    virtual void deallocate(Process* process) = 0;
    virtual vector<int> visualizeMemory(size_t size, size_t frame) = 0;
    virtual void logMemoryStamp(int cycleNumber, size_t frame, Process* process) = 0;
    // virtual void visualizeMemory() const = 0;
};

class FlatMemoryAllocator : public IMemoryAllocator {
private:
    size_t maximumSize;
    size_t allocatedSize;
    vector<char> memory;
    vector<bool> allocationMap;
    vector<string> processMap;
    
    bool canAllocateAt(size_t index, size_t framesRequired) const;
    void allocateAt(size_t index, size_t framesRequired, string name);
    void deallocateAt(size_t index, size_t framesRequired);

public:
    FlatMemoryAllocator(size_t maximumSize);
    ~FlatMemoryAllocator();

    void* allocate(Process* processe) override;
    void deallocate(Process* process) override;
    vector<int> visualizeMemory(size_t size, size_t frame) override;
    size_t calculateExternalFragmentation(size_t frame) const;
    void logMemoryStamp(int cycleNumber, size_t frame, Process* process) override;
    int countProcessesInMemory(Process* process) const;
};

class PagingAllocator : public IMemoryAllocator {
private:
    size_t maxMemorySize;
    size_t numFrames;
    unordered_map<size_t, size_t> frameMap;
    vector<size_t> freeFrameList;

    size_t allocateFrames(size_t numFrames, size_t processId);
    void deallocateFrames(size_t numFrames, vector<size_t> frameIndices);

public:
    PagingAllocator(size_t maxMemorySize);

    void* allocate(Process* process) override;
    void deallocate(Process* process) override;
    vector<int> visualizeMemory(size_t size, size_t frame) override;
    void logMemoryStamp(int cycleNumber, size_t frame, Process* process) override;
    // void visualizeMemory() const override;
};

#endif

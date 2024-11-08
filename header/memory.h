#ifndef MEMORY_H
#define MEMORY_H

#include <cstddef>
#include <string>
#include <vector>

using namespace std;

class IMemoryAllocator {
public:
    virtual void* allocate(size_t size, size_t frame) = 0;
    virtual void deallocate(void* ptr, size_t size, size_t frame) = 0;
    virtual string visualizeMemory() = 0;
};

class FlatMemoryAllocator : public IMemoryAllocator {
private:
    size_t maximumSize;
    size_t allocatedSize;
    vector<char> memory;
    vector<bool> allocationMap;
    
    bool canAllocateAt(size_t index, size_t framesRequired) const;
    void allocateAt(size_t index, size_t framesRequired);
    void deallocateAt(size_t index, size_t framesRequired);

public:
    FlatMemoryAllocator(size_t maximumSize);
    ~FlatMemoryAllocator();
    void* allocate(size_t size, size_t frame) override;
    void deallocate(void* ptr, size_t size, size_t frame) override;
    string visualizeMemory() override;
    size_t calculateExternalFragmentation(size_t frame) const;
    void logMemoryStamp(int cycleNumber, size_t size, size_t frame);
    int countProcessesInMemory(size_t size, size_t frame) const;
};

#endif

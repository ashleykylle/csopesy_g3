#ifndef MEMORY_H
#define MEMORY_H

#include <cstddef>
#include <string>
#include <vector>

using namespace std;

class IMemoryAllocator {
public:
    virtual void* allocate(size_t size) = 0;
    virtual void deallocate(void* ptr, size_t size) = 0;
    virtual string visualizeMemory() = 0;
};

class FlatMemoryAllocator : public IMemoryAllocator {
private:
    size_t maximumSize;
    size_t allocatedSize;
    vector<char> memory;
    vector<bool> allocationMap;
    
    bool canAllocateAt(size_t index, size_t size) const;
    void allocateAt(size_t index, size_t size);
    void deallocateAt(size_t index, size_t size);

public:
    FlatMemoryAllocator(size_t maximumSize);
    ~FlatMemoryAllocator();
    void* allocate(size_t size) override;
    void deallocate(void* ptr, size_t size) override;
    string visualizeMemory() override;
};

#endif

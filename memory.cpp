#include "header/memory.h"
#include <algorithm>
#include <iostream>

using namespace std;

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize)
    : maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.'), allocationMap(maximumSize, -1) {}

FlatMemoryAllocator::~FlatMemoryAllocator() {
    memory.clear();
}

void* FlatMemoryAllocator::allocate(size_t size, int processId) {
    for (size_t i = 0; i <= maximumSize - size; ++i) {
        if (canAllocateAt(i, size)) {
            allocateAt(i, size, processId);
            return &memory[i];
        }
    }
    return nullptr;
}

void FlatMemoryAllocator::deallocate(int processId) {
    for (size_t i = 0; i < maximumSize; ++i) {
        if (allocationMap[i] == processId) {
            deallocateAt(i);
        }
    }
}

string FlatMemoryAllocator::visualizeMemory() {
    return string(memory.begin(), memory.end());
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
    return (index + size <= maximumSize);
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size, int processId) {
    fill(allocationMap.begin() + index, allocationMap.begin() + index + size, processId);
    fill(memory.begin() + index, memory.begin() + index + size, '#');
    allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
    int processId = allocationMap[index];
    while (index < maximumSize && allocationMap[index] == processId) {
        allocationMap[index] = -1;
        memory[index] = '.';
        --allocatedSize;
        ++index;
    }
}

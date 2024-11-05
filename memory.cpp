#include "header/memory.h"
#include <algorithm>
#include <iostream>

using namespace std;

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize)
    : maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.'), allocationMap(maximumSize, false) {}

FlatMemoryAllocator::~FlatMemoryAllocator() {
    memory.clear();
}

void* FlatMemoryAllocator::allocate(size_t size) {
    for (size_t i = 0; i <= maximumSize - size; ++i) {
        if (!allocationMap[i] && canAllocateAt(i, size)) {
            allocateAt(i, size);
            return &memory[i];
        }
    }
    return nullptr;
}

void FlatMemoryAllocator::deallocate(void* ptr) {
    size_t index = static_cast<char*>(ptr) - &memory[0];
    if (allocationMap[index]) {
        deallocateAt(index);
    }
}

string FlatMemoryAllocator::visualizeMemory() {
    return string(memory.begin(), memory.end());
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
    return (index + size <= maximumSize);
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size) {
    fill(allocationMap.begin() + index, allocationMap.begin() + index + size, true);
    fill(memory.begin() + index, memory.begin() + index + size, '#');
    allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
    while (index < maximumSize && allocationMap[index] == true) {
        allocationMap[index] = false;
        memory[index] = '.';
        --allocatedSize;
        ++index;
    }
}

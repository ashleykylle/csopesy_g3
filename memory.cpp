#include "header/memory.h"
#include <algorithm>
#include <iostream>

using namespace std;

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize)
    : maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.'), allocationMap(maximumSize, false) {}

FlatMemoryAllocator::~FlatMemoryAllocator() {
    memory.clear();
}

void* FlatMemoryAllocator::allocate(size_t size, size_t frame) {
    size_t framesRequired = (size + frame - 1) / frame;
    for (size_t i = 0; i < maximumSize; ++i) {
        if (!allocationMap[i] && canAllocateAt(i, framesRequired)) {
            allocateAt(i, framesRequired);
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

string FlatMemoryAllocator::visualizeMemory() {
    return string(memory.begin(), memory.end());
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t framesRequired) const {
    for (size_t i = 1; i < framesRequired; ++i) {
        if (index + i >= maximumSize || allocationMap[index + i]) {
            return false;
        }
    }
    return true;
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t framesRequired) {
    fill(allocationMap.begin() + index, allocationMap.begin() + index + framesRequired, true);
    fill(memory.begin() + index, memory.begin() + index + framesRequired, '#');
    allocatedSize += framesRequired;
}

void FlatMemoryAllocator::deallocateAt(size_t index, size_t framesRequired) {
    fill(allocationMap.begin() + index, allocationMap.begin() + index + framesRequired, false);
    fill(memory.begin() + index, memory.begin() + index + framesRequired, '.');
    allocatedSize -= framesRequired;
}

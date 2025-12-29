#ifndef BUDDY_ALLOCATOR_H
#define BUDDY_ALLOCATOR_H

#include "MemoryManager.h"
#include <vector>
#include <list>
#include <map>
#include <cmath>

class BuddyAllocator : public MemoryManager {
private:
    // Array of free lists. freeLists[k] stores free blocks of order k (size 2^k)
    std::vector<std::list<MemoryBlock>> freeLists;
    
    // Map to track allocated blocks: Address -> Order (Block Size)
    std::map<size_t, int> allocatedBlockMap;
    
    // Map to track allocated blocks: ID -> Address (To support free by ID)
    std::map<int, size_t> idToAddressMap;

    size_t minBlockSize;
    int maxOrder;

public:
    BuddyAllocator(size_t size);
    
    // Override the core functions
    bool allocate(size_t size) override;
    bool deallocate(int blockId) override;
    void dumpMemory() override;

private:
    void initializeBuddy();
    int getOrder(size_t size);
};

#endif
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include <list>
#include <string>
#include <iostream>

struct MemoryBlock {
    int id;
    size_t startAddress;
    size_t size;
    bool isFree;
    
    MemoryBlock(int i, size_t start, size_t s, bool free) 
        : id(i), startAddress(start), size(s), isFree(free) {}
};

class MemoryManager {
protected:
    size_t totalMemorySize;
    std::vector<char> physicalMemory; // Simulating RAM
    std::list<MemoryBlock> memoryList; // Linked list of blocks
    int nextBlockId;
    std::string allocatorType;

    // --- NEW STATS COUNTERS ---
    size_t numAllocRequests = 0;
    size_t numSuccessfulAllocs = 0;
    size_t numFailedAllocs = 0;
    size_t numFrees = 0;
    // --------------------------

public:
    MemoryManager(size_t size);
    virtual ~MemoryManager() {}

    void setAllocator(const std::string& type);
    
    virtual bool allocate(size_t size);
    virtual bool deallocate(int blockId);
    
    void coalesce(); // Merges adjacent free blocks
    virtual void dumpMemory(); // Visualizes memory
    virtual void showStats(); // Prints the summary
};

#endif
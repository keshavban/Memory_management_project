#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#include <vector>
#include <map>
#include <iostream>
#include "MemoryManager.h" // We need this to allocate Physical RAM for pages

struct PageTableEntry {
    bool valid;         // Is this page currently in physical memory?
    int frameNumber;    // The physical frame number (if valid)
    
    PageTableEntry() : valid(false), frameNumber(-1) {}
};

class MMU {
private:
    size_t pageSize;
    std::map<int, PageTableEntry> pageTable; // Maps Virtual Page Number -> Entry
    MemoryManager* memoryManager; // Reference to Physical Memory

    // Stats
    int pageFaults;
    int translations;

public:
    MMU(size_t pageSize, MemoryManager* mem);
    
    // The core function: Converts Virtual Address -> Physical Address
    // Returns -1 if it fails (e.g., Out of Memory)
    long long translateAddress(long long virtualAddress);
    
    void showStats();
};

#endif
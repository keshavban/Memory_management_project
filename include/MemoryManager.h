#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <iostream>
#include <vector>
#include <list>
#include <string>
// Represents a single block of memory (either Free or Allocated)
struct MemoryBlock {
    int id;                 // Unique ID for allocated blocks (0 if free)
    size_t startAddress;    // Starting index in the physical memory array
    size_t size;            // Size of the block
    bool isFree;            // True if block is free, False if used
    // Constructor for easy creation
    MemoryBlock(int i, size_t start, size_t s, bool free)
        : id(i), startAddress(start), size(s), isFree(free) {}
};
class MemoryManager {
private:
    size_t totalMemorySize;
    std::vector<char> physicalMemory; // Simulates the actual RAM bytes [cite: 16]
    std::list<MemoryBlock> memoryList; // Linked list to track blocks 
    std::string allocatorType; // "first", "best", or "worst"
    int nextBlockId;           // Auto-incrementing ID for allocations
public:
    // Constructor
    MemoryManager(size_t size);
    // Core Functions
    void setAllocator(const std::string& type);
    bool allocate(size_t size);          // malloc [cite: 35]
    bool deallocate(int blockId);        // free [cite: 36]
    // Visualization & Stats
    void dumpMemory();                   // [cite: 37]
    void showStats();                    // [cite: 58]
private:
    // Helper to merge adjacent free blocks 
    void coalesce(); 
};
#endif
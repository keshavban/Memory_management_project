#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <iostream>
#include <vector>
#include <list>
#include <string>

// Represents a single block of memory
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
    std::vector<char> physicalMemory; 
    std::list<MemoryBlock> memoryList; // For First/Best/Worst Fit
    int nextBlockId;      
    std::string allocatorType; // <--- RESTORED THIS VARIABLE

public:
    MemoryManager(size_t size);
    virtual ~MemoryManager() {}

    virtual bool allocate(size_t size);          
    virtual bool deallocate(int blockId);        
    virtual void dumpMemory();                   
    virtual void showStats();                    

    // Standard specific helper
    void setAllocator(const std::string& type);

protected:
    void coalesce(); 
};

#endif
#include "../include/MemoryManager.h"
#include <iostream>
#include <limits>
#include <iomanip>
#include <cmath>

MemoryManager::MemoryManager(size_t size) : totalMemorySize(size), nextBlockId(1), allocatorType("first") {
    physicalMemory.resize(size, 0); 
    memoryList.push_back(MemoryBlock(0, 0, size, true));
}

void MemoryManager::setAllocator(const std::string& type) {
    allocatorType = type;
    std::cout << "Allocator set to: " << allocatorType << " fit" << std::endl;
}

bool MemoryManager::allocate(size_t size) {
    numAllocRequests++; // <--- NEW

    std::list<MemoryBlock>::iterator bestBlockIt = memoryList.end();
    size_t bestSize = std::numeric_limits<size_t>::max();
    size_t worstSize = 0; 

    for (auto it = memoryList.begin(); it != memoryList.end(); ++it) {
        if (it->isFree && it->size >= size) {
            if (allocatorType == "first") {
                bestBlockIt = it;
                break;
            } else if (allocatorType == "best") {
                if (it->size < bestSize) {
                    bestSize = it->size;
                    bestBlockIt = it;
                }
            } else if (allocatorType == "worst") {
                if (it->size > worstSize) {
                    worstSize = it->size;
                    bestBlockIt = it;
                }
            }
        }
    }

    if (bestBlockIt == memoryList.end()) {
        std::cout << "Error: Not enough memory to allocate " << size << " bytes." << std::endl;
        numFailedAllocs++; // <--- NEW
        return false;
    }

    // Allocation Successful
    bestBlockIt->isFree = false;
    bestBlockIt->id = nextBlockId++;

    if (bestBlockIt->size > size) {
        size_t remainingSize = bestBlockIt->size - size;
        size_t newStartAddress = bestBlockIt->startAddress + size;
        bestBlockIt->size = size;
        MemoryBlock newFreeBlock(0, newStartAddress, remainingSize, true);
        memoryList.insert(std::next(bestBlockIt), newFreeBlock);
    }

    std::cout << "Allocated block id=" << bestBlockIt->id 
              << " at address=0x" << std::hex << bestBlockIt->startAddress << std::dec << std::endl;
    
    numSuccessfulAllocs++; // <--- NEW
    return true;
}

bool MemoryManager::deallocate(int blockId) {
    bool found = false;
    for (auto& block : memoryList) {
        if (!block.isFree && block.id == blockId) {
            block.isFree = true;
            block.id = 0;
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "Error: Block ID " << blockId << " not found." << std::endl;
        return false;
    }

    std::cout << "Block " << blockId << " freed." << std::endl;
    numFrees++; // <--- NEW
    coalesce();
    return true;
}

void MemoryManager::coalesce() {
    auto it = memoryList.begin();
    while (it != memoryList.end()) {
        auto nextIt = std::next(it);
        if (nextIt != memoryList.end() && it->isFree && nextIt->isFree) {
            it->size += nextIt->size;
            memoryList.erase(nextIt);
        } else {
            ++it;
        }
    }
}

void MemoryManager::dumpMemory() {
    std::cout << "\n--- Memory Dump ---" << std::endl;
    for (const auto& block : memoryList) {
        std::cout << "[0x" << std::hex << block.startAddress << "-0x" 
                  << (block.startAddress + block.size - 1) << std::dec << "] ";
        if (block.isFree) std::cout << "FREE (" << block.size << " bytes)" << std::endl;
        else std::cout << "USED (ID=" << block.id << ", " << block.size << " bytes)" << std::endl;
    }
    std::cout << "-------------------\n" << std::endl;
}

// >>> UPDATED TO MATCH YOUR IMAGE <<<
void MemoryManager::showStats() {
    size_t freeMemory = 0;
    size_t usedMemory = 0;
    size_t usedBlocks = 0;
    size_t freeBlocks = 0;
    size_t largestFreeBlock = 0;

    for (const auto& block : memoryList) {
        if (block.isFree) {
            freeMemory += block.size;
            freeBlocks++;
            if (block.size > largestFreeBlock) largestFreeBlock = block.size;
        } else {
            usedMemory += block.size;
            usedBlocks++;
        }
    }

    double utilPercent = (totalMemorySize > 0) ? ((double)usedMemory / totalMemorySize) * 100.0 : 0.0;
    double successRate = (numAllocRequests > 0) ? ((double)numSuccessfulAllocs / numAllocRequests) * 100.0 : 0.0;
    
    // Standard External Fragmentation Index: 1 - (Largest Free Block / Total Free Memory)
    // If it's 0.000, memory is either full or perfectly unfragmented (one big free block)
    double extFrag = 0.0;
    if (freeMemory > 0) {
        extFrag = 1.0 - ((double)largestFreeBlock / freeMemory);
    }

    std::cout << "\n--------- SUMMARY ---------" << std::endl;
    std::cout << "Total heap size        : " << totalMemorySize << " bytes" << std::endl;
    std::cout << "Used memory            : " << usedMemory << " bytes" << std::endl;
    std::cout << "Free memory            : " << freeMemory << " bytes" << std::endl;
    std::cout << "Used blocks            : " << usedBlocks << std::endl;
    std::cout << "Free blocks            : " << freeBlocks << std::endl;
    // Base MemoryManager usually has 0 internal fragmentation (unless alignment padding is added)
    std::cout << "Internal fragmentation : 0 bytes" << std::endl;
    std::cout << "Memory utilization     : " << std::fixed << std::setprecision(2) << utilPercent << "%" << std::endl;
    std::cout << "External fragmentation : " << std::fixed << std::setprecision(3) << extFrag << std::endl;
    std::cout << "Allocation requests    : " << numAllocRequests << std::endl;
    std::cout << "Successful allocs      : " << numSuccessfulAllocs << std::endl;
    std::cout << "Failed allocs          : " << numFailedAllocs << std::endl;
    std::cout << "Frees                  : " << numFrees << std::endl;
    std::cout << "Success rate           : " << std::fixed << std::setprecision(2) << successRate << "%" << std::endl;
    std::cout << "---------------------------" << std::endl;
}
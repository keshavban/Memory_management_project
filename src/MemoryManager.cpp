#include "../include/MemoryManager.h"
#include <iostream>
#include <limits>
#include <iomanip> // For nice formatting
MemoryManager::MemoryManager(size_t size) : totalMemorySize(size), nextBlockId(1), allocatorType("first") {
    // Resize the vector to act as physical RAM
    physicalMemory.resize(size, 0); 
    // Initially, the entire memory is one large free block [cite: 17]
    memoryList.push_back(MemoryBlock(0, 0, size, true));
}

void MemoryManager::setAllocator(const std::string& type) {
    allocatorType = type;
    std::cout << "Allocator set to: " << allocatorType << " fit" << std::endl;
}

// THE CORE LOGIC: Finding a block
bool MemoryManager::allocate(size_t size) {
    std::list<MemoryBlock>::iterator bestBlockIt = memoryList.end();
    
    // Pointers to track the "best" or "worst" candidate found so far
    size_t bestSize = std::numeric_limits<size_t>::max(); // For Best Fit (smallest sufficient)
    size_t worstSize = 0;                                 // For Worst Fit (largest sufficient)

    // Iterator to traverse the list
    for (auto it = memoryList.begin(); it != memoryList.end(); ++it) {
        if (it->isFree && it->size >= size) {
            
            // Strategy 1: First Fit [cite: 22]
            if (allocatorType == "first") {
                bestBlockIt = it;
                break; // Stop immediately after finding the first match
            }
            
            // Strategy 2: Best Fit [cite: 23]
            // We want the smallest block that is large enough
            else if (allocatorType == "best") {
                if (it->size < bestSize) {
                    bestSize = it->size;
                    bestBlockIt = it;
                }
            }
            
            // Strategy 3: Worst Fit [cite: 24]
            // We want the largest possible block
            else if (allocatorType == "worst") {
                if (it->size > worstSize) {
                    worstSize = it->size;
                    bestBlockIt = it;
                }
            }
        }
    }

    // If no suitable block was found
    if (bestBlockIt == memoryList.end()) {
        std::cout << "Error: Not enough memory to allocate " << size << " bytes." << std::endl;
        return false;
    }

    // ALLOCATION SUCCESSFUL
    // Update the chosen block
    bestBlockIt->isFree = false;
    bestBlockIt->id = nextBlockId++;
    
    // Split the block if there is leftover space 
    if (bestBlockIt->size > size) {
        size_t remainingSize = bestBlockIt->size - size;
        size_t newStartAddress = bestBlockIt->startAddress + size;
        
        // Update current block size to requested size
        bestBlockIt->size = size;

        // Create a new free block with the remaining space and insert it after the current one
        MemoryBlock newFreeBlock(0, newStartAddress, remainingSize, true);
        memoryList.insert(std::next(bestBlockIt), newFreeBlock);
    }

    std::cout << "Allocated block id=" << bestBlockIt->id 
              << " at address=0x" << std::hex << bestBlockIt->startAddress << std::dec << std::endl;
    return true;
}

bool MemoryManager::deallocate(int blockId) {
    bool found = false;
    for (auto& block : memoryList) {
        if (!block.isFree && block.id == blockId) {
            block.isFree = true;
            block.id = 0; // Reset ID
            found = true;
            nextBlockId--; // Reuse ID for simplicity
            for(auto& b : memoryList) {
                if(!b.isFree && b.id > blockId) {
                    b.id--; // Shift down IDs of subsequent blocks
                }
            }
            std::cout << "Block " << blockId << " freed." << std::endl;
            break;
        }
    }

    if (!found) {
        std::cout << "Error: Block ID " << blockId << " not found." << std::endl;
        return false;
    }

    // Merge adjacent free blocks immediately 
    coalesce();
    return true;
}

void MemoryManager::coalesce() {
    auto it = memoryList.begin();
    while (it != memoryList.end()) {
        auto nextIt = std::next(it);
        
        // If current and next are both valid and both free, merge them
        if (nextIt != memoryList.end() && it->isFree && nextIt->isFree) {
            it->size += nextIt->size; // Add size of next to current
            memoryList.erase(nextIt); // Remove the next block
            // Do NOT increment 'it' here; we must check the new big block against the *next* one
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
        if (block.isFree) {
            std::cout << "FREE (" << block.size << " bytes)" << std::endl;
        } else {
            std::cout << "USED (ID=" << block.id << ", " << block.size << " bytes)" << std::endl;
        }
    }
    std::cout << "-------------------\n" << std::endl;
}

void MemoryManager::showStats() {
    size_t freeMemory = 0;
    size_t usedMemory = 0;
    int fragments = 0;

    for (const auto& block : memoryList) {
        if (block.isFree) {
            freeMemory += block.size;
            fragments++; // Each free block counts as a fragment
        } else {
            usedMemory += block.size;
        }
    }
    std::cout << "Total Memory: " << totalMemorySize << std::endl;
    std::cout << "Used Memory:  " << usedMemory << std::endl;
    std::cout << "Free Memory:  " << freeMemory << std::endl;
    std::cout << "External Fragmentation (Free Blocks): " << fragments << std::endl;
    std::cout << "External Fragmentation (Percentage): " << (totalMemorySize ? (freeMemory * 100.0 / totalMemorySize) : 0) << "%" << std::endl;
}
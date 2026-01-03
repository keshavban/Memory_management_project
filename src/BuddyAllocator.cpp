#include "../include/BuddyAllocator.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iomanip> // For formatting percentages

BuddyAllocator::BuddyAllocator(size_t size) : MemoryManager(size) {
    this->allocatorType = "buddy";
    
    // 1. Ensure memory size is a Power of Two
    size_t powerOf2Size = 1;
    while (powerOf2Size < size) powerOf2Size *= 2;
    
    if (powerOf2Size != size) {
        std::cout << "[Buddy] Resizing memory from " << size << " to " << powerOf2Size << " bytes." << std::endl;
        totalMemorySize = powerOf2Size;
        physicalMemory.resize(totalMemorySize);
    }

    // Min block size 1 allows small allocations (e.g. malloc 4) to be efficient
    minBlockSize = 1; 

    maxOrder = (int)std::log2(totalMemorySize);
    freeLists.resize(maxOrder + 1);

    initializeBuddy();
}

void BuddyAllocator::initializeBuddy() {
    freeLists[maxOrder].push_back(MemoryBlock(0, 0, totalMemorySize, true));
    std::cout << "[Buddy] Initialized. Size: " << totalMemorySize << " bytes (Order " << maxOrder << ")" << std::endl;
}

int BuddyAllocator::getOrder(size_t size) {
    // Calculate smallest power of 2 >= size
    size_t actualSize = std::max(size, minBlockSize);
    return std::ceil(std::log2(actualSize));
}

bool BuddyAllocator::allocate(size_t size) {
    int reqOrder = getOrder(size);

    // 1. Find the smallest available block >= reqOrder
    int currentOrder = reqOrder;
    while (currentOrder <= maxOrder && freeLists[currentOrder].empty()) {
        currentOrder++;
    }

    if (currentOrder > maxOrder) {
        std::cout << "[Buddy] Allocation Failed: Out of Memory" << std::endl;
        return false;
    }

    // 2. Split blocks recursively
    while (currentOrder > reqOrder) {
        MemoryBlock block = freeLists[currentOrder].front();
        freeLists[currentOrder].pop_front();

        currentOrder--; 

        size_t buddySize = block.size / 2;
        MemoryBlock left(0, block.startAddress, buddySize, true);
        MemoryBlock right(0, block.startAddress + buddySize, buddySize, true);

        freeLists[currentOrder].push_back(left);
        freeLists[currentOrder].push_back(right);
    }

    // 3. Allocate
    MemoryBlock& allocated = freeLists[reqOrder].front();
    allocated.isFree = false;
    allocated.id = nextBlockId++;

    // Track metadata
    allocatedBlockMap[allocated.startAddress] = reqOrder;
    idToAddressMap[allocated.id] = allocated.startAddress;
    
    // NEW: Track requested size for stats
    requestedSizeMap[allocated.id] = size; 

    std::cout << "Allocated ID " << allocated.id << " @ 0x" << std::hex << allocated.startAddress 
              << std::dec << " (" << allocated.size << " bytes, Requested: " << size << ")" << std::endl;

    freeLists[reqOrder].pop_front();
    return true;
}

bool BuddyAllocator::deallocate(int blockId) {
    if (idToAddressMap.find(blockId) == idToAddressMap.end()) {
        std::cout << "Error: Invalid Block ID " << blockId << std::endl;
        return false;
    }

    size_t address = idToAddressMap[blockId];
    int order = allocatedBlockMap[address];

    // Clean up tracking maps
    idToAddressMap.erase(blockId);
    allocatedBlockMap.erase(address);
    requestedSizeMap.erase(blockId); // Remove stats tracking

    std::cout << "Freeing ID " << blockId << " @ 0x" << std::hex << address << std::dec << std::endl;

    // Coalesce Logic
    size_t currentAddr = address;
    size_t currentSize = (1 << order);

    while (order < maxOrder) {
        size_t buddyAddr = currentAddr ^ currentSize; // XOR to find buddy
        bool buddyIsFree = false;
        auto& list = freeLists[order];
        
        for (auto it = list.begin(); it != list.end(); ++it) {
            if (it->startAddress == buddyAddr && it->isFree) {
                buddyIsFree = true;
                list.erase(it);
                
                currentAddr = std::min(currentAddr, buddyAddr);
                currentSize *= 2;
                order++;
                break;
            }
        }

        if (!buddyIsFree) break;
    }

    freeLists[order].push_back(MemoryBlock(0, currentAddr, currentSize, true));
    return true;
}

void BuddyAllocator::dumpMemory() {
    // Same linear dump logic as before
    struct BlockInfo {
        size_t start;
        size_t size;
        bool isFree;
        int id; 
    };
    std::vector<BlockInfo> allBlocks;

    for (auto const& [addr, order] : allocatedBlockMap) {
        int id = 0;
        for(auto const& [bid, baddr] : idToAddressMap) {
            if(baddr == addr) { id = bid; break; }
        }
        allBlocks.push_back({addr, (size_t)1 << order, false, id});
    }

    for (const auto& list : freeLists) {
        for (const auto& block : list) {
            allBlocks.push_back({block.startAddress, block.size, true, 0});
        }
    }

    std::sort(allBlocks.begin(), allBlocks.end(), [](const BlockInfo& a, const BlockInfo& b) {
        return a.start < b.start;
    });

    std::cout << "\n--- Memory Map (Buddy) ---\n";
    for (const auto& b : allBlocks) {
        std::cout << "[0x" << std::hex << b.start << " - 0x" << (b.start + b.size - 1) << "] " << std::dec;
        if (b.isFree) {
            std::cout << "FREE (" << b.size << " bytes)" << std::endl;
        } else {
            std::cout << "USED (ID " << b.id << ", " << b.size << " bytes)" << std::endl;
        }
    }
    std::cout << "--------------------------\n";
}

void BuddyAllocator::showStats() {
    size_t totalUsed = 0;
    size_t totalRequested = 0;
    size_t internalFrag = 0;
    
    // Calculate Usage Stats
    for (auto const& [id, reqSize] : requestedSizeMap) {
        size_t address = idToAddressMap[id];
        int order = allocatedBlockMap[address];
        size_t allocatedSize = (1 << order);
        
        totalRequested += reqSize;
        totalUsed += allocatedSize;
        internalFrag += (allocatedSize - reqSize);
    }

    size_t totalFree = totalMemorySize - totalUsed;
    
    std::cout << "\n=== BUDDY ALLOCATOR STATISTICS ===" << std::endl;
    std::cout << "Total Memory:        " << totalMemorySize << " bytes" << std::endl;
    std::cout << "Used Memory (Actual):" << totalUsed << " bytes" << std::endl;
    std::cout << "Free Memory:         " << totalFree << " bytes" << std::endl;
    std::cout << "User Requested:      " << totalRequested << " bytes" << std::endl;
    
    std::cout << "\n--- Fragmentation ---" << std::endl;
    std::cout << "Internal Fragmentation: " << internalFrag << " bytes";
    if (totalUsed > 0) {
        double fragPercent = ((double)internalFrag / totalUsed) * 100.0;
        std::cout << " (" << std::fixed << std::setprecision(2) << fragPercent << "% of used memory)";
    }
    std::cout << std::endl;
    
    std::cout << "External Fragmentation: Check 'dump' for free block distribution." << std::endl;
    std::cout << "==================================\n" << std::endl;
}
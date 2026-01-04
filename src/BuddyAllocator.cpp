#include "../include/BuddyAllocator.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iomanip> 

BuddyAllocator::BuddyAllocator(size_t size) : MemoryManager(size) {
    this->allocatorType = "buddy";
    size_t powerOf2Size = 1;
    while (powerOf2Size < size) powerOf2Size *= 2;
    
    if (powerOf2Size != size) {
        totalMemorySize = powerOf2Size;
        physicalMemory.resize(totalMemorySize);
    }
    minBlockSize = 1; 
    maxOrder = (int)std::log2(totalMemorySize);
    freeLists.resize(maxOrder + 1);
    initializeBuddy();
}

void BuddyAllocator::initializeBuddy() {
    freeLists[maxOrder].push_back(MemoryBlock(0, 0, totalMemorySize, true));
    std::cout << "[Buddy] Initialized. Size: " << totalMemorySize << " bytes" << std::endl;
}

int BuddyAllocator::getOrder(size_t size) {
    size_t actualSize = std::max(size, minBlockSize);
    return std::ceil(std::log2(actualSize));
}

bool BuddyAllocator::allocate(size_t size) {
    numAllocRequests++; // <--- NEW

    int reqOrder = getOrder(size);
    int currentOrder = reqOrder;
    while (currentOrder <= maxOrder && freeLists[currentOrder].empty()) {
        currentOrder++;
    }

    if (currentOrder > maxOrder) {
        std::cout << "[Buddy] Allocation Failed: Out of Memory" << std::endl;
        numFailedAllocs++; // <--- NEW
        return false;
    }

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

    MemoryBlock& allocated = freeLists[reqOrder].front();
    allocated.isFree = false;
    allocated.id = nextBlockId++;

    allocatedBlockMap[allocated.startAddress] = reqOrder;
    idToAddressMap[allocated.id] = allocated.startAddress;
    requestedSizeMap[allocated.id] = size; 

    std::cout << "Allocated ID " << allocated.id << " @ 0x" << std::hex << allocated.startAddress 
              << std::dec << " (" << allocated.size << " bytes)" << std::endl;

    freeLists[reqOrder].pop_front();
    
    numSuccessfulAllocs++; // <--- NEW
    return true;
}

bool BuddyAllocator::deallocate(int blockId) {
    if (idToAddressMap.find(blockId) == idToAddressMap.end()) {
        std::cout << "Error: Invalid Block ID " << blockId << std::endl;
        return false;
    }

    size_t address = idToAddressMap[blockId];
    int order = allocatedBlockMap[address];

    idToAddressMap.erase(blockId);
    allocatedBlockMap.erase(address);
    requestedSizeMap.erase(blockId); 

    std::cout << "Freeing ID " << blockId << std::endl;
    numFrees++; // <--- NEW

    size_t currentAddr = address;
    size_t currentSize = (1 << order);

    while (order < maxOrder) {
        size_t buddyAddr = currentAddr ^ currentSize; 
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
    struct BlockInfo {
        size_t start;
        size_t size;
        bool isFree;
        int id; 
    };
    std::vector<BlockInfo> allBlocks;

    // 1. Gather Used Blocks
    for (auto const& [addr, order] : allocatedBlockMap) {
        int id = 0;
        // Find ID associated with this address
        for(auto const& [bid, baddr] : idToAddressMap) {
            if(baddr == addr) { id = bid; break; }
        }
        allBlocks.push_back({addr, (size_t)1 << order, false, id});
    }

    // 2. Gather Free Blocks from all lists
    for (const auto& list : freeLists) {
        for (const auto& block : list) {
            allBlocks.push_back({block.startAddress, block.size, true, 0});
        }
    }

    // 3. Sort by Address to show linear memory layout
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
// >>> UPDATED FOR BUDDY STATS <<<
void BuddyAllocator::showStats() {
    size_t usedMemory = 0; 
    size_t internalFrag = 0;
    size_t usedBlocks = 0;
    
    // Calculate Used Stats
    for (auto const& [id, reqSize] : requestedSizeMap) {
        size_t address = idToAddressMap[id];
        int order = allocatedBlockMap[address];
        size_t allocatedSize = (1 << order);
        
        usedMemory += allocatedSize;
        internalFrag += (allocatedSize - reqSize);
        usedBlocks++;
    }

    size_t freeMemory = totalMemorySize - usedMemory;
    size_t freeBlocks = 0;
    size_t largestFreeBlock = 0;
    
    for (const auto& list : freeLists) {
        freeBlocks += list.size();
        if (!list.empty()) {
            size_t blockSize = list.front().size;
            if (blockSize > largestFreeBlock) largestFreeBlock = blockSize;
        }
    }

    double utilPercent = (totalMemorySize > 0) ? ((double)usedMemory / totalMemorySize) * 100.0 : 0.0;
    double successRate = (numAllocRequests > 0) ? ((double)numSuccessfulAllocs / numAllocRequests) * 100.0 : 0.0;
    
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
    std::cout << "Internal fragmentation : " << internalFrag << " bytes" << std::endl;
    std::cout << "Memory utilization     : " << std::fixed << std::setprecision(2) << utilPercent << "%" << std::endl;
    std::cout << "External fragmentation : " << std::fixed << std::setprecision(3) << extFrag << std::endl;
    std::cout << "Allocation requests    : " << numAllocRequests << std::endl;
    std::cout << "Successful allocs      : " << numSuccessfulAllocs << std::endl;
    std::cout << "Failed allocs          : " << numFailedAllocs << std::endl;
    std::cout << "Frees                  : " << numFrees << std::endl;
    std::cout << "Success rate           : " << std::fixed << std::setprecision(2) << successRate << "%" << std::endl;
    std::cout << "---------------------------" << std::endl;
}
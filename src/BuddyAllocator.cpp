#include "../include/BuddyAllocator.h"
#include <iostream>
#include <algorithm>
#include <cmath>

BuddyAllocator::BuddyAllocator(size_t size) : MemoryManager(size) {
    // FIX: Explicitly set the inherited allocatorType variable
    this->allocatorType = "buddy";
    // 1. Ensure memory size is a Power of Two
    size_t powerOf2Size = 1;
    while (powerOf2Size < size) powerOf2Size *= 2;
    
    if (powerOf2Size != size) {
        std::cout << "[Buddy] Resizing memory from " << size << " to " << powerOf2Size << " bytes." << std::endl;
        totalMemorySize = powerOf2Size;
        physicalMemory.resize(totalMemorySize);
    }

    // 2. Setup Buddy Parameters
    minBlockSize = 32; // Smallest block we will manage (must be power of 2)
    maxOrder = (int)std::log2(totalMemorySize);
    
    // Resize vector to hold lists for orders 0 to maxOrder
    freeLists.resize(maxOrder + 1);

    initializeBuddy();
}

void BuddyAllocator::initializeBuddy() {
    // Start with one giant free block at the highest order
    freeLists[maxOrder].push_back(MemoryBlock(0, 0, totalMemorySize, true));
    std::cout << "[Buddy] Initialized. Max Order: " << maxOrder << " (" << totalMemorySize << " bytes)" << std::endl;
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

    // 2. Split blocks until we reach the requested order
    // 
    while (currentOrder > reqOrder) {
        // Remove block from higher order list
        MemoryBlock block = freeLists[currentOrder].front();
        freeLists[currentOrder].pop_front();

        currentOrder--; // Go down one level

        size_t buddySize = block.size / 2;
        
        // Create two buddies
        MemoryBlock left(0, block.startAddress, buddySize, true);
        MemoryBlock right(0, block.startAddress + buddySize, buddySize, true);

        // Add them to the lower order list
        freeLists[currentOrder].push_back(left);
        freeLists[currentOrder].push_back(right);

        std::cout << "[Buddy] Split Order " << (currentOrder + 1) << " -> Two Order " << currentOrder << " blocks." << std::endl;
    }

    // 3. Allocate the block
    MemoryBlock& allocated = freeLists[reqOrder].front();
    allocated.isFree = false;
    allocated.id = nextBlockId++;

    // Track metadata for freeing later
    allocatedBlockMap[allocated.startAddress] = reqOrder;
    idToAddressMap[allocated.id] = allocated.startAddress;

    std::cout << "[Buddy] Allocated ID " << allocated.id << " @ 0x" << std::hex << allocated.startAddress 
              << " (Order " << reqOrder << ", " << std::dec << allocated.size << " bytes)" << std::endl;

    freeLists[reqOrder].pop_front();
    return true;
}

bool BuddyAllocator::deallocate(int blockId) {
    // 1. Find the address and order for this ID
    if (idToAddressMap.find(blockId) == idToAddressMap.end()) {
        std::cout << "[Buddy] Error: Invalid Block ID " << blockId << std::endl;
        return false;
    }

    size_t address = idToAddressMap[blockId];
    int order = allocatedBlockMap[address];

    // Clean up tracking maps
    idToAddressMap.erase(blockId);
    allocatedBlockMap.erase(address);

    std::cout << "[Buddy] Freeing ID " << blockId << " @ 0x" << std::hex << address << std::dec << " (Order " << order << ")" << std::endl;

    // 2. Coalesce (Merge) Logic
    size_t currentAddr = address;
    size_t currentSize = (1 << order);

    while (order < maxOrder) {
        // Calculate Buddy Address: buddy = address XOR size
        // This is the bitwise magic of the Buddy System!
        size_t buddyAddr = currentAddr ^ currentSize;

        // Check if buddy is free
        bool buddyIsFree = false;
        auto& list = freeLists[order];
        
        for (auto it = list.begin(); it != list.end(); ++it) {
            if (it->startAddress == buddyAddr && it->isFree) {
                buddyIsFree = true;
                
                // Remove buddy from free list (we are merging it)
                list.erase(it);
                std::cout << "[Buddy] Merging 0x" << std::hex << currentAddr << " and 0x" << buddyAddr << std::dec << std::endl;
                
                // Prepare for next iteration (move up a level)
                // The merged block starts at the lower of the two addresses
                currentAddr = std::min(currentAddr, buddyAddr);
                currentSize *= 2;
                order++;
                break;
            }
        }

        if (!buddyIsFree) {
            break; // Cannot merge further
        }
    }

    // 3. Add the (potentially merged) block back to the free list
    freeLists[order].push_back(MemoryBlock(0, currentAddr, currentSize, true));
    return true;
}

void BuddyAllocator::dumpMemory() {
    std::cout << "\n--- Buddy System Dump ---" << std::endl;
    for (int i = maxOrder; i >= 0; i--) {
        std::cout << "Order " << i << " (" << (1 << i) << " bytes): ";
        if (freeLists[i].empty()) {
            std::cout << "[Empty]";
        } else {
            for (const auto& block : freeLists[i]) {
                std::cout << "[Free @ 0x" << std::hex << block.startAddress << "] ";
            }
        }
        std::cout << std::dec << std::endl;
    }
    std::cout << "-------------------------\n" << std::endl;
}
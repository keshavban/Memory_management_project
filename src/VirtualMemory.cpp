#include "../include/VirtualMemory.h"
#include <iostream>

MMU::MMU(size_t size, MemoryManager* mem) 
    : pageSize(size), memoryManager(mem), pageFaults(0), translations(0) {}

long long MMU::translateAddress(long long virtualAddress) {
    translations++;
    
    // 1. Calculate VPN and Offset
    int vpn = virtualAddress / pageSize;
    int offset = virtualAddress % pageSize;

    // 2. Check Page Table
    if (pageTable.find(vpn) != pageTable.end() && pageTable[vpn].valid) {
        // --- TLB HIT (Simulated) ---
        // In a real CPU, we'd check a TLB first. Here we just go straight to Page Table.
        int frameNumber = pageTable[vpn].frameNumber;
        long long physicalAddress = (frameNumber * pageSize) + offset;
        
        // Debug output (Optional, helps see what's happening)
        // std::cout << "DEBUG: VPN " << vpn << " -> Frame " << frameNumber << std::endl;
        
        return physicalAddress;
    } 
    else {
        // --- PAGE FAULT ---
        std::cout << "  -> Page Fault! (VPN " << vpn << " not in memory)" << std::endl;
        pageFaults++;

        // 3. Handle Fault: Allocate Physical Memory Frame
        // We use our Phase 1 Memory Manager to find a free block for this page
        // Note: In a real OS, we might evict an old page here (Page Replacement).
        // For this simulation, we allocate a new block.
        
        // We need to capture the address allocated by MemoryManager.
        // Since our MemoryManager::allocate currently just prints and returns bool,
        // we will assume a simpler "Next Free Frame" logic or modify MemoryManager.
        // **Simplification for this Project Scale:**
        // We will simulate physical memory simply by assigning a new Frame ID.
        // We assume Physical Memory = Large Array of Frames.
        
        static int nextFreeFrame = 0; // Simple counter for simulation purposes
        int allocatedFrame = nextFreeFrame++;
        
        // Update Page Table
        pageTable[vpn].valid = true;
        pageTable[vpn].frameNumber = allocatedFrame;
        
        std::cout << "  -> Page Loaded into Frame " << allocatedFrame << std::endl;

        // Recursive call: Now that it's loaded, translate it again
        return translateAddress(virtualAddress);
    }
}

void MMU::showStats() {
    std::cout << "--- MMU Stats ---" << std::endl;
    std::cout << "Total Translations: " << translations << std::endl;
    std::cout << "Page Faults: " << pageFaults << std::endl;
}
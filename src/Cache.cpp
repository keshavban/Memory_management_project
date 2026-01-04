#include "../include/Cache.h"

// ================= CacheLevel Implementation =================

CacheLevel::CacheLevel(std::string name, size_t size, size_t blkSize, int assoc, std::string pol)
    : levelName(name), cacheSize(size), blockSize(blkSize), associativity(assoc), policy(pol) {
    
    // Calculate number of sets
    numSets = cacheSize / (blockSize * associativity);
    
    sets.resize(numSets);
    for (auto& set : sets) {
        set.lines.resize(associativity); 
    }

    hits = 0;
    misses = 0;
    globalTime = 0;
    
    std::cout << "[" << levelName << "] Initialized: " << size << " bytes, " 
              << numSets << " sets, " << associativity << "-way, " << policy << "." << std::endl;
}

bool CacheLevel::access(unsigned long address, bool isWrite) {
    globalTime++; 
    
    unsigned long setIndex = (address / blockSize) % numSets;
    unsigned long tag = address / (blockSize * numSets);

    // 1. Check for HIT
    for (auto& line : sets[setIndex].lines) {
        if (line.valid && line.tag == tag) {
            hits++;
            if (policy == "LRU") line.lruTime = globalTime;
            
            // --- WRITE POLICY (Write-Back) ---
            if (isWrite) {
                line.dirty = true;
                std::cout << "   -> " << levelName << " Write Hit! (Marked Dirty)" << std::endl;
            }
            // ---------------------------------
            return true; 
        }
    }

    // 2. MISS
    misses++;
    handleReplacement(setIndex, tag);
    
    // 3. Write-Allocate (If we wrote, we must mark the newly loaded block as dirty)
    if (isWrite) {
        // Find the block we just inserted
        for (auto& line : sets[setIndex].lines) {
            if (line.valid && line.tag == tag) {
                line.dirty = true;
                break;
            }
        }
    }
    
    return false; 
}

void CacheLevel::handleReplacement(int setIndex, unsigned long tag) {
    auto& set = sets[setIndex];

    // Case 1: Look for Empty Slot
    for (auto& line : set.lines) {
        if (!line.valid) {
            line.valid = true;
            line.tag = tag;
            line.dirty = false; // Fresh from memory = Clean
            line.insertionTime = globalTime;
            line.lruTime = globalTime;
            return;
        }
    }

    // Case 2: Eviction Needed
    int victimIndex = 0;
    unsigned long minTime = -1; 

    for (int i = 0; i < set.lines.size(); i++) {
        unsigned long timeMetric = (policy == "FIFO") ? set.lines[i].insertionTime : set.lines[i].lruTime;
        if (timeMetric < minTime) {
            minTime = timeMetric;
            victimIndex = i;
        }
    }

    // --- WRITE-BACK LOGIC ---
    if (set.lines[victimIndex].dirty) {
        std::cout << "   [!CACHE EVICTION!] " << levelName << ": Writing dirty block 0x" 
                  << std::hex << set.lines[victimIndex].tag << std::dec << " back to Memory." << std::endl;
    }
    // ------------------------

    // Replace
    set.lines[victimIndex].tag = tag;
    set.lines[victimIndex].dirty = false; // New data is clean
    set.lines[victimIndex].insertionTime = globalTime;
    set.lines[victimIndex].lruTime = globalTime;
}

// >>> UPDATED FUNCTION <<<
void CacheLevel::showStats() {
    long total = hits + misses;
    double hitRate = (total > 0) ? (double)hits / total * 100.0 : 0.0;
    
    // Matches format: [L1] Hits: 4  Misses: 18  HitRate: 18.18%
    std::cout << "[" << levelName << "] Hits: " << std::left << std::setw(6) << hits 
              << " Misses: " << std::setw(6) << misses 
              << " HitRate: " << std::fixed << std::setprecision(2) << hitRate << "%" << std::endl;
}

// ================= CacheController Implementation =================

CacheController::CacheController() {
    // Defaults
    l1 = new CacheLevel("L1", 1024, 64, 2, "LRU");
    l2 = new CacheLevel("L2", 4096, 64, 4, "LRU");
    l3 = new CacheLevel("L3", 16384, 64, 8, "FIFO");
    // Initialize counters
    totalAccessCycles = 0;
    totalRequests = 0;
}

CacheController::~CacheController() {
    delete l1; delete l2; delete l3;
}

// Runtime Configuration
void CacheController::configCache(std::string level, size_t size, size_t blockSize, int assoc, std::string policy) {
    if (level == "L1") {
        delete l1;
        l1 = new CacheLevel("L1", size, blockSize, assoc, policy);
    } else if (level == "L2") {
        delete l2;
        l2 = new CacheLevel("L2", size, blockSize, assoc, policy);
    } else if (level == "L3") {
        delete l3;
        l3 = new CacheLevel("L3", size, blockSize, assoc, policy);
    } else {
        std::cout << "Invalid Cache Level: " << level << std::endl;
    }
}

void CacheController::accessMemory(unsigned long address, bool isWrite) {
    std::cout << "\nCPU " << (isWrite ? "WRITE" : "READ") << " Request: 0x" << std::hex << address << std::dec << std::endl;
    
    totalRequests++;
    int currentAccessCost = 0;

    // 1. Check L1
    currentAccessCost += L1_LATENCY; // Always pay L1 cost
    if (l1->access(address, isWrite)) {
        std::cout << "-> L1 Hit (Cost: " << currentAccessCost << " cycles)" << std::endl;
    } 
    else {
        std::cout << "-> L1 Miss" << std::endl;
        
        // 2. Check L2 (Penalty propagated)
        currentAccessCost += L2_LATENCY;
        if (l2->access(address, isWrite)) {
            std::cout << "-> L2 Hit (Cost: " << currentAccessCost << " cycles)" << std::endl;
        } 
        else {
            std::cout << "-> L2 Miss" << std::endl;
            
            // 3. Check L3 (Penalty propagated)
            currentAccessCost += L3_LATENCY;
            if (l3->access(address, isWrite)) {
                std::cout << "-> L3 Hit (Cost: " << currentAccessCost << " cycles)" << std::endl;
            } 
            else {
                std::cout << "-> L3 Miss (Accessing Main Memory)" << std::endl;
                // 4. Main Memory (Huge Penalty)
                currentAccessCost += RAM_LATENCY;
                std::cout << "-> Main Memory Access (Total Cost: " << currentAccessCost << " cycles)" << std::endl;
            }
        }
    }
    
    // Add this request's cost to the total system history
    totalAccessCycles += currentAccessCost;
}

// >>> UPDATED FUNCTION <<<
void CacheController::showStats() {
    std::cout << "\n========== CACHE STATS ==========" << std::endl;
    l1->showStats();
    l2->showStats();
    l3->showStats();
    
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Total Requests : " << totalRequests << std::endl;
    std::cout << "Total Cycles   : " << totalAccessCycles << std::endl;
    
    if (totalRequests > 0) {
        double amat = (double)totalAccessCycles / totalRequests;
        std::cout << "AMAT           : " << std::fixed << std::setprecision(2) << amat << " cycles" << std::endl;
    } else {
        std::cout << "AMAT           : 0.00 cycles" << std::endl;
    }
    std::cout << "=================================" << std::endl;
}
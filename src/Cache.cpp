#include "../include/Cache.h"

// ================= CacheLevel Implementation =================

CacheLevel::CacheLevel(std::string name, size_t size, size_t blkSize, int assoc, std::string pol)
    : levelName(name), cacheSize(size), blockSize(blkSize), associativity(assoc), policy(pol) {
    
    // Calculate number of sets: CacheSize / (BlockSize * Associativity)
    numSets = cacheSize / (blockSize * associativity);
    
    sets.resize(numSets);
    for (auto& set : sets) {
        set.lines.resize(associativity); // Each set has 'associativity' number of lines
    }

    hits = 0;
    misses = 0;
    globalTime = 0;
    
    std::cout << levelName << " Initialized: " << numSets << " sets, " 
              << associativity << " ways, " << policy << " policy." << std::endl;
}

bool CacheLevel::access(unsigned long address, bool isWrite) {
    globalTime++; // Increment clock

    // 1. Calculate Index and Tag
    // Index bits are derived from (Address / BlockSize) % NumSets
    unsigned long setIndex = (address / blockSize) % numSets;
    unsigned long tag = address / (blockSize * numSets);

    // 2. Search for the tag in the set
    for (auto& line : sets[setIndex].lines) {
        if (line.valid && line.tag == tag) {
            // HIT!
            hits++;
            if (policy == "LRU") {
                line.lruTime = globalTime; // Update usage time for LRU
            }
            return true; // Hit
        }
    }

    // 3. MISS - We need to insert this block into cache
    misses++;
    handleReplacement(setIndex, tag);
    return false; // Miss
}

void CacheLevel::handleReplacement(int setIndex, unsigned long tag) {
    auto& set = sets[setIndex];

    // Case 1: Look for an invalid (empty) line
    for (auto& line : set.lines) {
        if (!line.valid) {
            line.valid = true;
            line.tag = tag;
            line.insertionTime = globalTime;
            line.lruTime = globalTime;
            return;
        }
    }

    // Case 2: Cache is full, need to evict based on policy
    int victimIndex = 0;
    unsigned long minTime = -1; // Max possible value

    for (int i = 0; i < set.lines.size(); i++) {
        unsigned long timeMetric;
        
        if (policy == "FIFO") {
            timeMetric = set.lines[i].insertionTime;
        } else { // LRU
            timeMetric = set.lines[i].lruTime;
        }

        if (timeMetric < minTime) {
            minTime = timeMetric;
            victimIndex = i;
        }
    }

    // Replace victim
    set.lines[victimIndex].tag = tag;
    set.lines[victimIndex].insertionTime = globalTime;
    set.lines[victimIndex].lruTime = globalTime;
}

void CacheLevel::showStats() {
    std::cout << "--- " << levelName << " Stats ---" << std::endl;
    std::cout << "Hits: " << hits << " | Misses: " << misses << std::endl;
    if (hits + misses > 0)
        std::cout << "Hit Rate: " << (double)hits / (hits + misses) * 100.0 << "%" << std::endl;
}

// ================= CacheController Implementation =================

CacheController::CacheController() {
    // Configuration per requirements [cite: 71-76]
    // L1: Small, Fast. 1KB size, 64B block, 2-way associative, LRU
    l1 = new CacheLevel("L1", 1024, 64, 2, "LRU");
    
    // L2: Larger. 4KB size, 64B block, 4-way associative, FIFO
    l2 = new CacheLevel("L2", 4096, 64, 4, "FIFO");
}

CacheController::~CacheController() {
    delete l1;
    delete l2;
}

void CacheController::accessMemory(unsigned long address) {
    std::cout << "\nCPU Request: Address 0x" << std::hex << address << std::dec << std::endl;

    // Check L1 first
    if (l1->access(address, false)) {
        std::cout << "-> L1 Cache HIT" << std::endl;
    } else {
        std::cout << "-> L1 Cache MISS" << std::endl;
        
        // If L1 misses, check L2
        if (l2->access(address, false)) {
            std::cout << "-> L2 Cache HIT" << std::endl;
        } else {
            std::cout << "-> L2 Cache MISS (Accessing Main Memory...)" << std::endl;
            // In a real system, we would fetch from MemoryManager here and fill L2, then L1.
            // For simulation, the 'access' method inside CacheLevel already "fills" the cache slot.
        }
        
        // Note: Strict inclusion (L1 inside L2) isn't enforced here for simplicity,
        // but typically you fill L1 after an L2 hit/miss. 
        // We implicitly filled L1 in the logic above by calling access? 
        // Actually, the current logic calculates Hit/Miss. 
        // To be accurate: On L1 Miss, we must INSERT into L1.
        // Let's ensure L1 gets filled regardless of where we found the data.
        
        // This is a simplification. To properly simulate "Bringing data in", 
        // we technically already did it in l1->access() because that function calls 
        // handleReplacement() internally on a miss. 
        // So L1 is now updated with this address. Correct.
    }
}

void CacheController::showStats() {
    l1->showStats();
    l2->showStats();
}
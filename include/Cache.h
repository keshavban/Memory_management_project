#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <iomanip>

// Represents a single line (slot) in the cache
struct CacheLine {
    bool valid;             // Is there data here?
    unsigned long tag;      // The ID of the memory block
    unsigned long lruTime;  // Timestamp for Least Recently Used policy
    unsigned long insertionTime; // Timestamp for FIFO policy

    CacheLine() : valid(false), tag(0), lruTime(0), insertionTime(0) {}
};

// Represents a set of lines (for set-associativity)
// In a direct-mapped cache, a set has only 1 line.
struct CacheSet {
    std::vector<CacheLine> lines;
};

class CacheLevel {
private:
    std::string levelName;  // "L1", "L2", etc.
    size_t cacheSize;       // Total size in bytes
    size_t blockSize;       // Block size in bytes
    int associativity;      // Ways per set (1 = Direct Mapped)
    std::string policy;     // "LRU" or "FIFO"

    size_t numSets;         // Calculated number of sets
    std::vector<CacheSet> sets; // The actual storage
    
    // Statistics
    int hits;
    int misses;
    unsigned long globalTime; // A logical clock to track access order

public:
    CacheLevel(std::string name, size_t size, size_t blockSize, int assoc, std::string policy);
    
    // Returns true if HIT, false if MISS
    bool access(unsigned long address, bool isWrite);
    
    void showStats();
    
private:
    // Replacement algorithms
    void handleReplacement(int setIndex, unsigned long tag);
};

class CacheController {
private:
    CacheLevel* l1;
    CacheLevel* l2;

public:
    CacheController();
    ~CacheController();
    
    void accessMemory(unsigned long address);
    void showStats();
};

#endif
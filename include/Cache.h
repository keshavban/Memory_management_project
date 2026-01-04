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
    bool dirty;             // <--- NEW: Has this data been modified?
    unsigned long tag;      // The ID of the memory block
    unsigned long lruTime;  // Timestamp for Least Recently Used policy
    unsigned long insertionTime; // Timestamp for FIFO policy

    CacheLine() : valid(false), dirty(false), tag(0), lruTime(0), insertionTime(0) {}
};

struct CacheSet {
    std::vector<CacheLine> lines;
};

class CacheLevel {
private:
    std::string levelName;  
    size_t cacheSize;       
    size_t blockSize;       
    int associativity;      
    std::string policy;     

    size_t numSets;         
    std::vector<CacheSet> sets; 
    
    // Statistics
    int hits;
    int misses;
    unsigned long globalTime; 

public:
    CacheLevel(std::string name, size_t size, size_t blockSize, int assoc, std::string policy);
    
    // Updated to accept isWrite
    bool access(unsigned long address, bool isWrite);
    
    void showStats();
    
private:
    void handleReplacement(int setIndex, unsigned long tag);
};

class CacheController {
private:
    CacheLevel* l1;
    CacheLevel* l2;
    CacheLevel* l3;
// NEW: Latency Tracking
    unsigned long long totalAccessCycles;
    unsigned long long totalRequests;

    // Simulation Constants (Latencies in "Cycles")
    const int L1_LATENCY = 1;
    const int L2_LATENCY = 10;
    const int L3_LATENCY = 100;
    const int RAM_LATENCY = 500;
    
public:
    CacheController();
    ~CacheController();
    
    // Updated access signature
    void accessMemory(unsigned long address, bool isWrite);
    
    // NEW: Method to re-configure a specific cache level at runtime
    void configCache(std::string level, size_t size, size_t blockSize, int assoc, std::string policy);

    void showStats();
};

#endif
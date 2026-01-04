#include "../include/MemoryManager.h"
#include "../include/BuddyAllocator.h"
#include "../include/Cache.h"
#include "../include/virtualmemory.h" 
#include <iostream>
#include <sstream>
#include <string>

void printHelp() {
    std::cout << "\n--- Available Commands ---\n";
    std::cout << "  init <size>              : Initialize physical memory size\n";
    std::cout << "  config cache <L1|L2> ... : Configure Cache (ex: config cache L1 2048 64 2)\n";
    std::cout << "  set allocator <type>     : Set allocator (first, best, worst, buddy)\n";
    std::cout << "  set policy <type>        : Set VM replacement policy (FIFO, LRU)\n";
    std::cout << "  malloc <size>            : Allocate virtual memory block\n";
    std::cout << "  free <id>                : Free memory block\n";
    std::cout << "  read <virtual_addr>      : Read Address (Access)\n";
    std::cout << "  write <virtual_addr>     : Write Address (Sets Dirty Bit)\n";
    std::cout << "  stats                    : Show All Stats\n";
    std::cout << "  exit                     : Exit\n";
    std::cout << "--------------------------\n";
}

int main() {
    size_t memorySize = 1024; 
    int pageSize = 64;        
    int vaBits = 16;          

    MemoryManager* memSim = new MemoryManager(memorySize); 
    CacheController* cacheSim = new CacheController();
    VirtualMemory* vm = new VirtualMemory(vaBits, pageSize, memorySize, "FIFO");

    std::cout << "System Initialized." << std::endl;
    printHelp();

    std::string commandLine;
    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, commandLine)) break;
        
        std::stringstream ss(commandLine);
        std::string cmd;
        ss >> cmd;

        if (cmd == "exit") break;
        else if (cmd == "help") printHelp();
        
        else if (cmd == "init") {
            size_t size;
            if (ss >> size) {
                memorySize = size;
                delete memSim; memSim = new MemoryManager(memorySize);
                delete vm; vm = new VirtualMemory(vaBits, pageSize, memorySize, "FIFO");
                std::cout << "Memory initialized to " << size << " bytes." << std::endl;
            }
        }
        // --- NEW: CONFIG CACHE COMMAND ---
        else if (cmd == "config") {
            std::string subCmd;
            ss >> subCmd;
            if (subCmd == "cache") {
                std::string level;
                size_t size, blk;
                int assoc;
                // Default policy is LRU for simplicity in CLI
                if (ss >> level >> size >> blk >> assoc) {
                    cacheSim->configCache(level, size, blk, assoc, "LRU");
                } else {
                    std::cout << "Usage: config cache <Level> <Size> <BlockSize> <Assoc>" << std::endl;
                }
            }
        }
        else if (cmd == "set") {
            std::string subCmd, type;
            ss >> subCmd >> type;

            if (subCmd == "allocator") {
                delete memSim;
                if (type == "buddy") memSim = new BuddyAllocator(memorySize);
                else { memSim = new MemoryManager(memorySize); memSim->setAllocator(type); }
                std::cout << "Allocator: " << type << std::endl;
            } 
            else if (subCmd == "policy") {
                if (type == "FIFO" || type == "fifo") type = "FIFO";
                else if (type == "LRU" || type == "lru") type = "LRU";

                if (type == "FIFO" || type == "LRU") {
                    delete vm;
                    vm = new VirtualMemory(vaBits, pageSize, memorySize, type);
                    std::cout << "VM Policy set to: " << type << std::endl;
                } else {
                    std::cout << "Invalid Policy." << std::endl;
                }
            } 
        }

        else if (cmd == "malloc") {
            size_t size;
            if (ss >> size) memSim->allocate(size);
        }
        else if (cmd == "free") {
            int id;
            if (ss >> id) memSim->deallocate(id);
        }
        else if (cmd == "dump") {
            memSim->dumpMemory();
        }

        // --- READ / WRITE COMMANDS ---
        else if (cmd == "read" || cmd == "access") {
            std::string addrStr;
            if (ss >> addrStr) {
                try {
                    int virtualAddr = std::stoi(addrStr, nullptr, 0);
                    int physicalAddr = vm->translate(virtualAddr);
                    std::cout << "      -> Phys Addr: 0x" << std::hex << physicalAddr << std::dec << std::endl;
                    // Pass isWrite = false
                    cacheSim->accessMemory(physicalAddr, false); 
                } catch (...) { std::cout << "Invalid address." << std::endl; }
            }
        }
        else if (cmd == "write") {
            std::string addrStr;
            if (ss >> addrStr) {
                try {
                    int virtualAddr = std::stoi(addrStr, nullptr, 0);
                    int physicalAddr = vm->translate(virtualAddr);
                    std::cout << "      -> Phys Addr: 0x" << std::hex << physicalAddr << std::dec << std::endl;
                    // Pass isWrite = true
                    cacheSim->accessMemory(physicalAddr, true); 
                } catch (...) { std::cout << "Invalid address." << std::endl; }
            }
        }

        else if (cmd == "stats") {
            std::cout << "=== MEMORY ALLOCATOR STATS ===" << std::endl;
            memSim->showStats();
            std::cout << "\n=== VIRTUAL MEMORY STATS ===" << std::endl;
            vm->stats();
            std::cout << "\n=== CACHE STATS ===" << std::endl;
            cacheSim->showStats();
        }
    }

    delete vm;
    delete cacheSim;
    delete memSim;
    return 0;
}
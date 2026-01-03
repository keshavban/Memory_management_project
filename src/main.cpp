#include "../include/MemoryManager.h"
#include "../include/BuddyAllocator.h"
#include "../include/Cache.h"
#include "../include/virtualmemory.h" // Ensure this matches your filename
#include <iostream>
#include <sstream>
#include <string>

void printHelp() {
    std::cout << "\n--- Available Commands ---\n";
    std::cout << "  init <size>              : Initialize physical memory size\n";
    std::cout << "  set allocator <type>     : Set allocator (first, best, worst, buddy)\n";
    std::cout << "  set policy <type>        : Set VM replacement policy (FIFO, LRU)\n";
    std::cout << "  malloc <size>            : Allocate virtual memory block (Allocator Test)\n";
    std::cout << "  free <id>                : Free memory block\n";
    std::cout << "  access <virtual_addr>    : Access Address (Full System Test)\n";
    std::cout << "  dump                     : Dump Allocator Map\n";
    std::cout << "  stats                    : Show All Stats\n";
    std::cout << "  exit                     : Exit\n";
    std::cout << "--------------------------\n";
}

int main() {
    // 1. Configuration Constants
    size_t memorySize = 1024; 
    int pageSize = 64;        
    int vaBits = 16;          

    // 2. Initialize Components
    MemoryManager* memSim = new MemoryManager(memorySize); 
    CacheController* cacheSim = new CacheController();
    
    // Initialize VM with default FIFO policy
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
        
        // --- CONFIGURATION COMMANDS ---
        else if (cmd == "init") {
            size_t size;
            if (ss >> size) {
                // Resize everything to keep components in sync
                memorySize = size;
                delete memSim;
                memSim = new MemoryManager(memorySize);
                
                delete vm;
                vm = new VirtualMemory(vaBits, pageSize, memorySize, "FIFO");
                
                std::cout << "Memory initialized to " << size << " bytes." << std::endl;
            } else {
                std::cout << "Usage: init <size>" << std::endl;
            }
        }
        else if (cmd == "set") {
            std::string subCmd, type;
            ss >> subCmd >> type;

            if (subCmd == "allocator") {
                delete memSim;
                if (type == "buddy") {
                    memSim = new BuddyAllocator(memorySize);
                    std::cout << "Allocator: Buddy System" << std::endl;
                } else {
                    memSim = new MemoryManager(memorySize);
                    memSim->setAllocator(type);
                    std::cout << "Allocator: " << type << std::endl;
                }
            } 
            // FIX IS HERE: 'policy' is a sibling of 'allocator', not a child
            else if (subCmd == "policy") {
                // Handle case-insensitivity manually
                if (type == "FIFO" || type == "fifo") type = "FIFO";
                else if (type == "LRU" || type == "lru") type = "LRU";

                if (type == "FIFO" || type == "LRU") {
                    // We must recreate the VM to apply the new policy
                    // (Preserving the current memory size setting)
                    delete vm;
                    vm = new VirtualMemory(vaBits, pageSize, memorySize, type);
                    std::cout << "VM Policy set to: " << type << std::endl;
                } else {
                    std::cout << "Invalid Policy. Use 'FIFO' or 'LRU'." << std::endl;
                }
            } 
            else {
                std::cout << "Usage: set allocator <type> OR set policy <FIFO|LRU>" << std::endl;
            }
        }

        // --- ALLOCATOR COMMANDS ---
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

        // --- FULL SYSTEM ACCESS COMMAND ---
        else if (cmd == "access") {
            std::string addrStr;
            if (ss >> addrStr) {
                try {
                    int virtualAddr = std::stoi(addrStr, nullptr, 0);
                    std::cout << "[CPU] Access Virtual Address: 0x" << std::hex << virtualAddr << std::dec << std::endl;

                    // 1. VM Translation (Virtual -> Physical)
                    int physicalAddr = vm->translate(virtualAddr);
                    
                    std::cout << "      -> Physical Address: 0x" << std::hex << physicalAddr << std::dec << std::endl;

                    // 2. Cache Access (Physical)
                    // (Only access cache if translation succeeded)
                    cacheSim->accessMemory(physicalAddr);

                } catch (...) {
                    std::cout << "Invalid address." << std::endl;
                }
            }
        }

        // --- STATS ---
        else if (cmd == "stats") {
            std::cout << "=== MEMORY ALLOCATOR STATS ===" << std::endl;
            memSim->showStats();
            std::cout << "\n=== VIRTUAL MEMORY STATS ===" << std::endl;
            vm->stats();
            std::cout << "\n=== CACHE STATS ===" << std::endl;
            cacheSim->showStats();
        }
        else if (!cmd.empty()) {
            std::cout << "Unknown command: " << cmd << std::endl;
        }
    }

    delete vm;
    delete cacheSim;
    delete memSim;
    return 0;
}
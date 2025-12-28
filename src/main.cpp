#include "../include/MemoryManager.h"
#include "../include/Cache.h" // <--- NEW INCLUDE
#include <iostream>
#include <sstream>

void printHelp() {
    std::cout << "\n--- Available Commands ---\n";
    std::cout << "  init <size>              : Initialize memory\n";
    std::cout << "  malloc <size>            : Allocate memory block\n";
    std::cout << "  free <id>                : Free memory block\n";
    std::cout << "  access <address>         : Simulate CPU access (tests Cache)\n"; // <-- NEW
    std::cout << "  cache stats              : Show Cache Hit/Miss stats\n";           // <-- NEW
    std::cout << "  dump                     : Dump Memory\n";
    std::cout << "  stats                    : Show Memory Stats\n";
    std::cout << "  exit                     : Exit\n";
    std::cout << "--------------------------\n";
}

int main() {
    size_t memorySize = 1024;
    MemoryManager* memSim = new MemoryManager(memorySize);
    CacheController* cacheSim = new CacheController(); // <--- NEW OBJECT

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
                delete memSim;
                memSim = new MemoryManager(size);
                std::cout << "Memory initialized." << std::endl;
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
        else if (cmd == "dump") memSim->dumpMemory();
        else if (cmd == "stats") memSim->showStats();
        
        // --- NEW CACHE COMMANDS ---
        else if (cmd == "access") {
            unsigned long addr;
            // Use std::stoul to handle hex (0x100) or decimal inputs
            std::string addrStr;
            if (ss >> addrStr) {
                try {
                    addr = std::stoul(addrStr, nullptr, 0); // 0 allows auto-detecting hex/dec
                    cacheSim->accessMemory(addr);
                } catch (...) {
                    std::cout << "Invalid address format." << std::endl;
                }
            }
        }
        else if (cmd == "cache") {
            std::string sub;
            ss >> sub;
            if (sub == "stats") cacheSim->showStats();
        }
        // --------------------------
        
        else if (!cmd.empty()) std::cout << "Unknown command." << std::endl;
    }

    delete memSim;
    delete cacheSim;
    return 0;
}
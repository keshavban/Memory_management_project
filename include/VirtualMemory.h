#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <string>

struct PageTableEntry {
    bool valid;
    int frame;
    int last_used;
};

class VirtualMemory {
private:
    int virtual_address_bits;
    int page_size;
    int physical_memory_size;
    int num_frames;

    int timer;

    std::unordered_map<int, PageTableEntry> page_table;
    std::vector<int> frame_owner;

    std::queue<int> fifo_queue;
    std::string replacement_policy;

    int page_hits;
    int page_faults;
    int disk_accesses;

    int select_victim();
    void handle_page_fault(int page);

public:
    VirtualMemory(int va_bits,
                  int page_size,
                  int phys_mem_size,
                  const std::string& policy);

    int translate(int virtual_address);

    void stats() const;
};

#endif

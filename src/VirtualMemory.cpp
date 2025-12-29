#include "VirtualMemory.h"
#include <iostream>
#include <climits>

using namespace std;

VirtualMemory::VirtualMemory(int va_bits,
                             int ps,
                             int phys_mem,
                             const string& policy)
    : virtual_address_bits(va_bits),
      page_size(ps),
      physical_memory_size(phys_mem),
      replacement_policy(policy),
      timer(0),
      page_hits(0),
      page_faults(0),
      disk_accesses(0) {

    num_frames = physical_memory_size / page_size;
    frame_owner.resize(num_frames, -1);
}

int VirtualMemory::select_victim() {
    if (replacement_policy == "FIFO") {
        int victim = fifo_queue.front();
        fifo_queue.pop();
        return victim;
    }

    // LRU
    int victim = -1;
    int oldest = INT_MAX;

    for (auto &p : page_table) {
        if (p.second.valid && p.second.last_used < oldest) {
            oldest = p.second.last_used;
            victim = p.first;
        }
    }
    return victim;
}

void VirtualMemory::handle_page_fault(int page) {
    disk_accesses++;

    int frame = -1;

    // find free frame
    for (int i = 0; i < num_frames; i++) {
        if (frame_owner[i] == -1) {
            frame = i;
            break;
        }
    }

    // eviction needed
    if (frame == -1) {
        int victim_page = select_victim();
        frame = page_table[victim_page].frame;

        page_table[victim_page].valid = false;
        frame_owner[frame] = -1;
    }

    // load page
    page_table[page] = {true, frame, timer};
    frame_owner[frame] = page;

    if (replacement_policy == "FIFO")
        fifo_queue.push(page);
}

int VirtualMemory::translate(int virtual_address) {
    timer++;

    int page = virtual_address / page_size;
    int offset = virtual_address % page_size;

    if (page_table.count(page) && page_table[page].valid) {
        page_hits++;
        page_table[page].last_used = timer;
        return page_table[page].frame * page_size + offset;
    }

    // page fault
    page_faults++;
    handle_page_fault(page);

    return page_table[page].frame * page_size + offset;
}

void VirtualMemory::stats() const {
    cout << "Page hits: " << page_hits << "\n";
    cout << "Page faults: " << page_faults << "\n";
    cout << "Disk accesses: " << disk_accesses << "\n";

    int total = page_hits + page_faults;
    if (total > 0) {
        cout << "Page fault rate: "
             << (double)page_faults / total * 100 << "%\n";
    }
}

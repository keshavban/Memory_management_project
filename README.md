🧠 Memory Management Simulator
==============================

A comprehensive **Operating System Memory Management Simulator** that models how modern systems handle **dynamic memory allocation, virtual memory, paging, address translation, and multi-level CPU caches**.\
This project is designed to be both **educational** and **system-level accurate**, closely mirroring real OS behavior.

* * * * *

📌 Project Overview
-------------------

Memory management is a core responsibility of an operating system, involving:

-   Allocation and deallocation of memory

-   Translation of virtual addresses to physical addresses

-   Efficient use of limited physical RAM

-   Exploiting cache hierarchies for performance

This project simulates the **entire memory access pipeline**, starting from a CPU-generated virtual address down to physical memory and caches.

* * * * *

🏗️ System Architecture
-----------------------

`CPU
 │
 │  (Virtual Address)
 ▼
MMU (Memory Management Unit)
 │
 │  Page Table Lookup
 │  ├─ Page Hit  → Continue
 │  └─ Page Fault → Load Page into RAM
 ▼
Physical Address
 │
 ▼
L1 Cache → L2 Cache → L3 Cache
 │
 └─ Cache Miss → Main Memory (RAM)`

* * * * *

✨ Features
----------

### 🔹 Dynamic Memory Allocation

-   First Fit

-   Best Fit

-   Worst Fit

-   Buddy System Allocator

-   Block splitting and coalescing

-   External fragmentation handling

### 🔹 Virtual Memory & Paging

-   Fixed-size pages and frames

-   Demand paging (lazy allocation)

-   Page tables with valid bits

-   Page fault handling

-   Page replacement policies:

    -   FIFO

    -   LRU

### 🔹 Address Translation

-   Virtual address → (VPN + Offset)

-   Page table lookup

-   Physical frame mapping

-   Physical address generation

### 🔹 Cache Hierarchy Simulation

-   L1 Cache (small, fast, LRU)

-   L2 Cache (larger, LRU)

-   L3 Cache (largest, FIFO)

-   Cache hits and misses logged

-   Cache works on **physical addresses only**

### 🔹 Interactive CLI

-   Step-by-step observation of memory behavior

-   Detailed logs for educational clarity

* * * * *

🧩 Memory Model Assumptions
---------------------------

-   Byte-addressable memory

-   Fixed page size (power of 2)

-   Page size = Frame size

-   Single-process simulation

-   Contiguous virtual address space starting at `0x0`

Example:

-   Physical Memory: 65536 bytes

-   Page Size: 64 bytes

-   Frames: 1024

* * * * *

🖥️ Supported Commands
----------------------

| Command | Description |
| --- | --- |
| `init <size>` | Initialize physical memory |
| `set allocator <type>` | Select allocation strategy |
| `set policy <FIFO/LRU>` | Set VM replacement policy |
| `malloc <size>` | Allocate virtual memory |
| `free <id>` | Free allocated block |
| `access <addr>` | Access a virtual address |
| `dump` | Show heap memory layout |
| `stats` | Display performance statistics |
| `exit` | Exit simulator |

* * * * *

🧪 Example Execution
--------------------

`> init 65536
Memory initialized to 65536 bytes.

> malloc 100
Allocated block id=1 at address=0x0

> access 0x40
[CPU] Access Virtual Address: 0x40
[MMU] Page Fault! Virtual Page 1 is not in RAM.
[MMU] Loaded Virtual Page 1 into Frame 0
-> Physical Address: 0x0

-> L1 Miss
-> L2 Miss
-> L3 Miss (Accessing Main Memory)`

* * * * *

🧠 Key Concepts Demonstrated
----------------------------

-   `malloc()` allocates **virtual memory**, not physical RAM

-   Physical memory is allocated **only on page fault**

-   Pages are fixed-size; accessing 1 byte loads the entire page

-   Virtual page numbers and physical frame numbers are independent

-   Cache hierarchy operates after address translation

-   Cache misses are normal and expected behavior

* * * * *

⚠️ Limitations
--------------

-   Single-process simulation

-   No TLB (Translation Lookaside Buffer)

-   No segmentation or protection bits

-   Simplified cache indexing

-   No disk write-back simulation

These limitations were intentional to keep the system **clear and educational**.

* * * * *

🚀 Future Enhancements
----------------------

-   TLB simulation

-   Multi-process address spaces

-   Dirty bits and write-back cache

-   NUMA-aware memory

-   Segmentation + paging

-   Visual GUI for memory maps

* * * * *

🎯 Learning Outcomes
--------------------

This project provides deep understanding of:

-   OS memory abstractions

-   Hardware--software interaction

-   Paging vs allocation

-   Cache behavior and locality

-   Real-world OS design trade-offs

* * * * *

👨‍💻 Author
------------

**Keshav Bansal**\
Undergraduate, B.Tech\
Indian Institute of Technology Roorkee

* * * * *

📜 License
----------

This project is intended for **educational and academic use**.
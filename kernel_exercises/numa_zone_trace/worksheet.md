# NUMA AND ZONE LAYOUT: AXIOMATIC DERIVATION WORKSHEET
## Numa Zone Trace Exercise | Linux Kernel 6.14.0-37-generic | x86_64

---

### MACHINE SPECIFICATIONS
| Property | Value | Source Command |
|----------|-------|----------------|
| RAM | 15776276 kB = 15.04 GB | `cat /proc/meminfo \| grep MemTotal` |
| PAGE_SIZE | 4096 bytes | `getconf PAGE_SIZE` |
| Total Pages | 3944069 | 15776276×1024÷4096 |
| NUMA Nodes | 1 | `numactl --hardware` |
| Node 0 CPUs | 0-11 (12 CPUs) | `numactl --hardware` |
| Node 0 Size | 15406 MB | `numactl --hardware` |
| pageblock_order | 9 | Kernel config |

---

### TABLE OF CONTENTS
1. [Lines 01-20] **NUMA Problem**: Why non-uniform access exists
2. [Lines 21-40] **Node Definition**: pg_data_t, node boundaries
3. [Lines 41-60] **Zone Definition**: DMA, DMA32, Normal with boundaries
4. [Lines 61-80] **Zone-Node Overlap**: How zones span nodes
5. [Lines 81-100] **Pageblock**: Migration types, pageblock_order
6. [Lines 101-120] **Folio**: Multi-page representation
7. [Lines 121-140] **PFN Conversions**: PHYS_PFN, PFN_PHYS
8. [Lines 141-160] **Verification**: dmesg output, /proc/zoneinfo

---

### LESSON OBJECTIVES
After completing this worksheet, you will be able to:
- [ ] Calculate node size in pages from numactl output
- [ ] Derive zone boundaries from address constraints
- [ ] Trace page → node and page → zone mappings
- [ ] Calculate pageblock_nr_pages from pageblock_order
- [ ] Convert between PFN and physical address

---

### NOTATION GUIDE
| Symbol | Meaning |
|--------|---------|
| ✓ | Verified/Correct |
| ✗ | Failed/Incorrect |
| → | Leads to / Implies |
| ∴ | Therefore |
| AXIOM | Accepted truth from source |
| DEFINITION | New term being defined |
| CALCULATION | Step-by-step arithmetic |
| DRAW | Visual diagram |
| PROOF | Source code reference |

---

## PART 1: THE NUMA PROBLEM
---

01. AXIOM: Large servers have multiple CPU sockets. Each socket = separate CPU chip on motherboard.
02. AXIOM: Each CPU socket has RAM modules physically attached to it. RAM attached to CPU 0 different from RAM attached to CPU 1.
03. PROBLEM: CPU 0 accessing its own attached RAM takes time T1. CPU 0 accessing CPU 1's RAM takes time T2. T2 > T1. Different access times.
04. DEFINITION: NUMA = Non-Uniform Memory Access. Access time depends on which CPU accesses which RAM.
05. EXAMPLE: T1 = 10 nanoseconds (local access). T2 = 100 nanoseconds (remote access). 10× slower.
06. AXIOM: This machine has 1 NUMA node. Source: numactl --hardware → "available: 1 nodes (0)".
07. CONSEQUENCE: Single node = uniform memory access. All CPUs access all RAM with same latency.
08. AXIOM: Node 0 has 12 CPUs. Source: numactl → "node 0 cpus: 0 1 2 3 4 5 6 7 8 9 10 11".
09. AXIOM: Node 0 has 15406 MB RAM. Source: numactl → "node 0 size: 15406 MB".
10. CALCULATION: 15406 MB in bytes = 15406 × 1024 × 1024. Step: 15406 × 1024 = 15775744. Step: 15775744 × 1024 = 16154361856 bytes.
11. CALCULATION: 15406 MB in pages = 16154361856 ÷ 4096 = 3944424 pages.
12. AXIOM: Node distance = 10. Source: numactl → "node 0: 10". Lower = faster. 10 = local access.

---

## PART 2: NODE DATA STRUCTURES
---

13. DEFINITION: NODE = physically contiguous domain of memory attached to one CPU socket.
14. DEFINITION: pg_data_t = kernel data structure for each NUMA node. One pg_data_t per node.
15. AXIOM: NODE_DATA(nid) = macro returning pg_data_t* for node nid. Source: include/linux/mmzone.h.
16. DEFINITION: node_start_pfn = first PFN belonging to this node.
17. DEFINITION: node_spanned_pages = total number of PFNs in node (including holes).
18. DEFINITION: node_present_pages = actual usable PFNs in node (excluding holes).
19. PROBLEM: Memory may have HOLES. Hole = range of physical addresses that don't exist or are reserved.
20. CONSEQUENCE: node_spanned_pages ≥ node_present_pages. Spanned includes holes. Present excludes holes.

---

## PART 3: ZONE PROBLEM
---

21. PROBLEM: ISA bus (old hardware) has 24-bit address bus. 24 bits = 2^24 addresses.
22. CALCULATION: 2^24 = 16777216 = 16 × 1024 × 1024 = 16 MB. ISA can only address first 16 MB.
23. PROBLEM: 32-bit hardware has 32-bit address bus. 32 bits = 2^32 addresses.
24. CALCULATION: 2^32 = 4294967296 = 4 × 1024 × 1024 × 1024 = 4 GB. 32-bit can only address first 4 GB.
25. PROBLEM: Device using ISA DMA requests memory. Kernel allocates page at 8 GB. Device cannot access. FAIL.
26. SOLUTION: Label pages by address range. Only give ISA devices pages below 16 MB.
27. DEFINITION: ZONE = region of physical memory with specific address constraints.

---

## PART 4: ZONE DEFINITIONS
---

28. DEFINITION: ZONE_DMA = pages with physical address < 16 MB. For ISA DMA devices.
29. CALCULATION: ZONE_DMA boundary PFN = 16 MB ÷ 4096 = 16777216 ÷ 4096 = 4096. ZONE_DMA: PFN 0 to 4095.
30. DEFINITION: ZONE_DMA32 = pages with physical address 16 MB to 4 GB. For 32-bit devices.
31. CALCULATION: ZONE_DMA32 start = PFN 4096. ZONE_DMA32 end = 4 GB ÷ 4096 = 4294967296 ÷ 4096 = 1048576. ZONE_DMA32: PFN 4096 to 1048575.
32. DEFINITION: ZONE_NORMAL = pages with physical address ≥ 4 GB. For 64-bit capable hardware.
33. CALCULATION: ZONE_NORMAL start = PFN 1048576. ZONE_NORMAL end = last PFN of RAM.
34. AXIOM: This machine has 3944069 pages. ZONE_NORMAL: PFN 1048576 to 3944068.
35. RULE: Each page belongs to exactly ONE zone. Zone determined by physical address.

---

## PART 5: ZONE BOUNDARY TABLE
---

36. DRAW: Zone boundaries with PFN and physical address:
```
| Zone     | Start PFN | End PFN   | Start Addr | End Addr    |
|----------|-----------|-----------|------------|-------------|
| DMA      | 0         | 4095      | 0x0        | 0xFFFFFF    |
| DMA32    | 4096      | 1048575   | 0x1000000  | 0xFFFFFFFF  |
| Normal   | 1048576   | 3944068   | 0x100000000| 0x3BEFF8FFF |
```
37. VERIFY: DMA end address = 4095 × 4096 + 4095 = 16773120 + 4095 = 16777215 = 0xFFFFFF. ✓
38. VERIFY: DMA32 end address = 1048575 × 4096 + 4095 = 4294963200 + 4095 = 4294967295 = 0xFFFFFFFF = 4 GB - 1. ✓

---

## PART 6: ZONES OVERLAP NODES
---

39. PROBLEM: Server with 2 nodes. Node 0 = 0-8 GB. Node 1 = 8-16 GB. Where are zones?
40. OBSERVATION: ZONE_DMA (0-16 MB) is entirely within Node 0.
41. OBSERVATION: ZONE_DMA32 (16 MB - 4 GB) is entirely within Node 0.
42. OBSERVATION: ZONE_NORMAL (4 GB+) spans Node 0 (4-8 GB) AND Node 1 (8-16 GB).
43. CONCLUSION: Zones are NOT per-node. Zones span across nodes.
44. CONSEQUENCE: Each zone exists independently in each node. Node 0 has DMA + DMA32 + Normal. Node 1 has only Normal.
45. DRAW: Zone-Node relationship:
```
Node 0:  [----DMA----][--------DMA32--------][-------Normal (part)------]
Node 1:                                      [-------Normal (part)------]
         0           16MB                  4GB                         16GB
```

---

## PART 7: PAGEBLOCK CONCEPTS
---

46. PROBLEM: Memory becomes fragmented. Many small free blocks. No large contiguous blocks.
47. SOLUTION: Migrate pages. Move page from address A to address B. Free contiguous region.
48. PROBLEM: Not all pages can be migrated. Kernel code pages = unmovable. User data pages = movable.
49. SOLUTION: Group pages by migration type. All pages in group have same type.
50. DEFINITION: pageblock = smallest unit that can have a migration type. Size = 2^pageblock_order pages.
51. AXIOM: pageblock_order = 9 on x86_64. Source: CONFIG_PAGEBLOCK_ORDER or kernel config.
52. CALCULATION: pageblock_nr_pages = 2^9 = 512 pages.
53. CALCULATION: pageblock_size = 512 × 4096 = 2097152 bytes = 2 MB.
54. DEFINITION: migratetype = enum { MIGRATE_UNMOVABLE, MIGRATE_MOVABLE, MIGRATE_RECLAIMABLE, ... }.
55. RULE: All pages in same pageblock have same migratetype.

---

## PART 8: STRUCT FOLIO
---

56. PROBLEM: Order=3 allocation = 8 pages. Each page has struct page (64 bytes). Iterating 8 struct pages = slow.
57. SOLUTION: Represent allocation as single unit. Head page + tail pages.
58. DEFINITION: struct folio = represents 1 or more contiguous pages as single unit.
59. DEFINITION: Head page = first page of folio. Has folio metadata.
60. DEFINITION: Tail pages = remaining pages. Point back to head.
61. FORMULA: folio_nr_pages(folio) = 1 << folio_order(folio). Order=3 → 2^3 = 8 pages.
62. FORMULA: folio_size(folio) = folio_nr_pages(folio) × PAGE_SIZE. Order=3 → 8 × 4096 = 32768 bytes.
63. ADVANTAGE: API takes folio instead of page. No iteration. Single operation.

---

## PART 9: PFN CONVERSIONS
---

64. DEFINITION: PFN = Page Frame Number = physical_address ÷ PAGE_SIZE.
65. WHY: Physical address contains 12 bits of page offset (0-4095). PFN removes these bits.
66. FORMULA: PHYS_PFN(addr) = addr >> 12. Shift right by 12 bits = divide by 4096.
67. FORMULA: PFN_PHYS(pfn) = pfn << 12. Shift left by 12 bits = multiply by 4096.
68. EXAMPLE: phys = 0x152852000. PFN = 0x152852000 >> 12 = 0x152852 = 1386578.
69. VERIFY: 1386578 << 12 = 1386578 × 4096 = 5679423488 = 0x152852000. ✓
70. EXAMPLE: PFN = 1048576 (start of ZONE_NORMAL). phys = 1048576 × 4096 = 4294967296 = 0x100000000 = 4 GB. ✓

---

## PART 10: DRIVER VERIFICATION
---

71. COMMAND: sudo insmod numa_zone_trace.ko
72. COMMAND: sudo dmesg | grep NUMA_ZONE
73. EXPECTED: num_online_nodes=1
74. EXPECTED: node[0]:start_pfn=1,spanned_pages=3944xxx
75. EXPECTED: page:pfn=0x15xxxx,node=0,zone=Normal
76. EXPECTED: pageblock_order=9,pageblock_nr_pages=512
77. COMMAND: sudo rmmod numa_zone_trace
78. VERIFY: Compare output values with worksheet calculations.

---

## PART 11: FAILURE PREDICTIONS
---

79. F1: User confuses node and zone. Node = CPU attachment. Zone = address range. Different concepts.
80. F2: User assumes each node has all zones. NO. Node 1 (high addresses) may only have ZONE_NORMAL.
81. F3: User forgets pageblock_order varies by architecture. x86_64 = 9. ARM = different.
82. F4: User confuses spanned and present. Spanned includes holes. Present = actual usable pages.
83. F5: User forgets folio_nr_pages. Uses loop over struct page instead. Slow.
84. F6: User forgets PFN × 4096. Treats PFN as address. WRONG.

---

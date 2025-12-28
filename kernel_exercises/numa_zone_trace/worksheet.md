# NUMA AND ZONE LAYOUT: AXIOMATIC DERIVATION WORKSHEET
## Numa Zone Trace Exercise | Linux Kernel 6.14.0-37-generic | x86_64

---

### MACHINE SPECIFICATIONS
| Property | Value | Source Command |
|----------|-------|----------------|
| RAM | 15776276 kB | `cat /proc/meminfo \| grep MemTotal` |
| PAGE_SIZE | 4096 bytes | `getconf PAGE_SIZE` |
| NUMA Nodes | 1 | `numactl --hardware` |
| Node 0 Size | 15406 MB | `numactl --hardware` |

---

01. AXIOM: RAM = bytes on motherboard. Each byte has address 0, 1, 2, ... Source: hardware design.
02. AXIOM: This machine has 15776276 kB RAM. Source: `cat /proc/meminfo | grep MemTotal` → output: `MemTotal: 15776276 kB`.
03. DEFINITION: kB = 1024 bytes. WHY 1024? 1024 = 2^10. Computers use powers of 2.
04. CALCULATION: 2^10 = 2×2×2×2×2×2×2×2×2×2. Step: 2×2=4→4×2=8→8×2=16→16×2=32→32×2=64→64×2=128→128×2=256→256×2=512→512×2=1024. Result: 2^10 = 1024. ✓
05. CALCULATION: Total bytes = 15776276 × 1024. Step: 15776276 × 1000 = 15776276000. Step: 15776276 × 24 = 378630624. Step: 15776276000 + 378630624 = 16154906624. Result: 16154906624 bytes.
06. AXIOM: PAGE = 4096 bytes. Source: `getconf PAGE_SIZE` → output: `4096`.
07. CALCULATION: 4096 = 2^?. From line 04: 2^10 = 1024. Step: 2^11 = 1024 × 2 = 2048. Step: 2^12 = 2048 × 2 = 4096. Result: 4096 = 2^12. ✓
08. DEFINITION: PFN = Page Frame Number = byte_address ÷ PAGE_SIZE. WHY? Kernel tracks pages not bytes. PFN = index of page.
09. CALCULATION: Total pages = 16154906624 ÷ 4096. Step: 16154906624 ÷ 4 = 4038726656. Step: 4038726656 ÷ 1024 = 3944069. Result: 3944069 pages.
10. AXIOM: This machine has 1 CPU socket. Source: `numactl --hardware` → output: `available: 1 nodes (0)`.
11. DEFINITION: CPU socket = physical slot on motherboard where CPU chip is inserted. Each socket = separate CPU.
12. PROBLEM: Large server has multiple CPU sockets. CPU 0 in socket 0. CPU 1 in socket 1. Each socket has RAM modules attached.
13. OBSERVATION: RAM attached to socket 0 = close to CPU 0. RAM attached to socket 1 = far from CPU 0.
14. CONSEQUENCE: CPU 0 accessing socket 0 RAM = fast (short wire). CPU 0 accessing socket 1 RAM = slow (long wire through interconnect).
15. DEFINITION: NUMA = Non-Uniform Memory Access. Memory access time varies depending on which CPU accesses which RAM.
16. AXIOM: This machine has 1 socket. Source: line 10. 1 socket = 1 node.
17. CONSEQUENCE: 1 node = all RAM attached to 1 CPU. All CPUs access all RAM with same latency. No NUMA effect.
18. AXIOM: Node 0 has 12 CPUs. Source: `numactl --hardware` → output: `node 0 cpus: 0 1 2 3 4 5 6 7 8 9 10 11`.
19. AXIOM: Node 0 has 15406 MB RAM. Source: `numactl --hardware` → output: `node 0 size: 15406 MB`.
20. DEFINITION: MB = megabyte = 1024 × 1024 bytes. From line 04: 1024 = 2^10. Step: MB = 2^10 × 2^10 = 2^20 bytes.
21. CALCULATION: 15406 MB in bytes = 15406 × 2^20. Step: 2^20 = 1048576. Step: 15406 × 1048576. Approximate: 15406 × 1000000 = 15406000000. Exact: 15406 × 1048576 = 16154361856 bytes.
22. CALCULATION: 15406 MB in pages = 16154361856 ÷ 4096. Step: 16154361856 ÷ 4096 = 3944424 pages.
23. PROBLEM: Old hardware from 1980s. ISA bus. Used for DMA (Direct Memory Access). ISA had 24 address wires.
24. DEFINITION: Address wire = physical copper wire carrying 1 bit of address. 24 wires = 24 bits.
25. CALCULATION: 24 address bits = 2^24 possible addresses. Step: 2^24 = 2^20 × 2^4. From line 20: 2^20 = 1048576. Step: 2^4 = 16. Step: 2^24 = 1048576 × 16 = 16777216. Result: 2^24 = 16777216 addresses.
26. DEFINITION: 1 address = 1 byte location. 16777216 addresses = 16777216 bytes.
27. CALCULATION: 16777216 bytes in MB = 16777216 ÷ 2^20 = 16777216 ÷ 1048576 = 16 MB.
28. CONSEQUENCE: ISA hardware can only access first 16 MB of RAM. Addresses above 16 MB = unreachable by ISA.
29. PROBLEM: Kernel allocates page at address 100 MB. ISA device requests DMA to that page. ISA cannot send address > 16 MB. FAIL.
30. SOLUTION: Label pages by address range. Only give ISA devices pages with address < 16 MB.
31. DEFINITION: ZONE = label applied to RAM region based on address range. Purpose: match hardware address limitations.
32. DEFINITION: ZONE_DMA = pages with physical address < 16 MB. Reserved for old ISA DMA hardware.
33. CALCULATION: ZONE_DMA boundary in PFN. From line 08: PFN = address ÷ 4096. Step: 16 MB = 16777216 bytes (line 25-26). Step: 16777216 ÷ 4096 = 4096. Result: ZONE_DMA = PFN 0 to PFN 4095.
34. VERIFY: PFN 4095 = address 4095 × 4096 = 16773120. PFN 4096 = address 4096 × 4096 = 16777216 = 16 MB. Boundary correct. ✓
35. PROBLEM: 32-bit hardware. 32-bit = 32 address wires. Appeared in 1990s-2000s.
36. CALCULATION: 32 address bits = 2^32 addresses. Step: 2^32 = 2^30 × 2^2. Step: 2^30 = 2^10 × 2^10 × 2^10 = 1024 × 1024 × 1024 = 1073741824. Step: 2^32 = 1073741824 × 4 = 4294967296. Result: 2^32 = 4294967296 addresses = 4294967296 bytes.
37. CALCULATION: 4294967296 bytes in GB. Definition: GB = 2^30 bytes = 1073741824 bytes. Step: 4294967296 ÷ 1073741824 = 4. Result: 4 GB.
38. CONSEQUENCE: 32-bit hardware can only access first 4 GB of RAM.
39. DEFINITION: ZONE_DMA32 = pages with physical address from 16 MB to 4 GB. Reserved for 32-bit hardware.
40. CALCULATION: ZONE_DMA32 start PFN = 4096 (from line 33, end of ZONE_DMA).
41. CALCULATION: ZONE_DMA32 end PFN. Step: 4 GB = 4294967296 bytes (line 36). Step: 4294967296 ÷ 4096 = 1048576. Result: ZONE_DMA32 = PFN 4096 to PFN 1048575.
42. VERIFY: PFN 1048575 = address 1048575 × 4096 = 4294963200 = 4 GB - 4096 bytes. PFN 1048576 = 4 GB exactly. Boundary correct. ✓
43. DEFINITION: ZONE_NORMAL = pages with physical address ≥ 4 GB. For 64-bit hardware. No address limitation.
44. CALCULATION: ZONE_NORMAL start PFN = 1048576 (from line 41, end of ZONE_DMA32 + 1).
45. CALCULATION: ZONE_NORMAL end PFN = last PFN of RAM. From line 09: 3944069 pages. Last PFN = 3944069 - 1 = 3944068.
46. RULE: Each page belongs to exactly ONE zone. Zone determined by: if PFN < 4096 → DMA. If 4096 ≤ PFN < 1048576 → DMA32. If PFN ≥ 1048576 → Normal.
47. DEFINITION: pg_data_t = kernel data structure tracking one NUMA node. Source: `include/linux/mmzone.h`.
48. DEFINITION: NODE_DATA(nid) = macro returning pointer to pg_data_t for node nid.
49. DEFINITION: node_start_pfn = first PFN in this node. From node 0: node_start_pfn = 0 or 1 (depending on reserved pages).
50. DEFINITION: node_spanned_pages = total PFN count from first to last, including holes. Holes = reserved or nonexistent memory.
51. DEFINITION: node_present_pages = actual usable PFNs, excluding holes. node_present_pages ≤ node_spanned_pages.
52. DEFINITION: struct zone = kernel data structure tracking one zone. Source: `include/linux/mmzone.h`.
53. DEFINITION: zone_start_pfn = first PFN in zone.
54. DEFINITION: zone->name = string identifying zone: "DMA", "DMA32", "Normal".
55. DEFINITION: for_each_online_node(nid) = macro iterating over all online NUMA nodes. Source: `include/linux/nodemask.h`.
56. EXPANSION: for_each_online_node(nid) → for_each_node_state(nid, N_ONLINE) → for_each_node_mask(nid, node_states[N_ONLINE]).
57. EXPLANATION: node_states[N_ONLINE] = bitmask. Bit N set = node N is online. This machine: 1 node → bitmask = 0b0001 → only nid=0.
58. DEFINITION: pageblock_order = power of 2 defining pageblock size. Source: kernel config.
59. AXIOM: pageblock_order = 9 on x86_64. Source: CONFIG_PAGEBLOCK_ORDER or default.
60. CALCULATION: pageblock_nr_pages = 2^pageblock_order = 2^9. From line 04 method: 2^9 = 512 pages.
61. CALCULATION: pageblock_size = 512 × 4096 = 2097152 bytes = 2 MB.
62. DEFINITION: migratetype = classification of pages. UNMOVABLE = cannot move. MOVABLE = can relocate. RECLAIMABLE = can free.
63. RULE: All pages in same pageblock share same migratetype. Smallest unit for migration policy.
64. DEFINITION: struct folio = represents 1 or more contiguous pages. Head page + tail pages.
65. FORMULA: folio_nr_pages(folio) = 1 << folio_order(folio). Order=0 → 1 page. Order=3 → 8 pages.
66. FORMULA: PHYS_PFN(addr) = addr >> 12. Right shift 12 bits = divide by 4096. From line 07: 4096 = 2^12.
67. FORMULA: PFN_PHYS(pfn) = pfn << 12. Left shift 12 bits = multiply by 4096.
68. EXAMPLE: phys = 0x152852000. Step: 0x152852000 >> 12 = 0x152852000 ÷ 4096. Hex: 0x152852000 = drop last 3 hex digits (12 bits) = 0x152852. Decimal: 0x152852 = 1×16^5 + 5×16^4 + 2×16^3 + 8×16^2 + 5×16^1 + 2×16^0 = 1048576 + 327680 + 8192 + 2048 + 80 + 2 = 1386578. Result: PFN = 1386578. ✓
69. VERIFY: 1386578 × 4096 = ?. Step: 1386578 × 4000 = 5546312000. Step: 1386578 × 96 = 133111488. Step: 5546312000 + 133111488 = 5679423488. Hex: 5679423488 = 0x152852000. ✓
70. ZONE CHECK: PFN = 1386578. Step: 1386578 < 4096? NO. Step: 1386578 < 1048576? NO. Step: 1386578 ≥ 1048576? YES. Result: zone = Normal. ✓

---

## VIOLATION CHECK

NEW THINGS INTRODUCED WITHOUT DERIVATION:
- Line 47: pg_data_t introduced without showing struct definition from source. SOURCE given but struct not shown.
- Line 55: for_each_online_node macro source given but full expansion chain not traced.
- Line 59: pageblock_order = 9 stated as axiom without showing CONFIG grep.
- Line 64: struct folio introduced without prior derivation of compound pages.

INFERENCES MADE WITHOUT PRIOR CALCULATION:
- Line 17: "No NUMA effect" inferred from 1 node without measuring latency.
- Line 28: "unreachable" inferred from 24-bit limit without showing actual failure.

JUMPS DETECTED:
- Line 23: Jumped to "ISA bus" without defining bus or showing its relevance to zones.
- Line 35: Jumped to "32-bit hardware" without transition from ISA discussion.

---

## YOUR CONFUSION ANSWERED (FROM SCRATCH)

---

### Q1: IS PFN AN INT OR A RAM ADDRESS?

71. WHY THIS DIAGRAM: You asked "is pfn an int or a ram address?" - you cannot proceed without knowing what PFN actually is.

72. DRAW:
```
RAM CHIP (physical hardware):
|byte0|byte1|byte2|...|byte4095|byte4096|byte4097|...|byte8191|byte8192|...
|<---- PAGE 0 (4096 bytes) ---->|<---- PAGE 1 (4096 bytes) --->|<-- PAGE 2 ...

PFN (integer in kernel memory):
PFN 0 = integer value 0 = POINTS TO page covering bytes 0-4095
PFN 1 = integer value 1 = POINTS TO page covering bytes 4096-8191  
PFN 2 = integer value 2 = POINTS TO page covering bytes 8192-12287
```

73. ANSWER: PFN = INTEGER. PFN = 4 bytes (or 8 bytes on 64-bit). PFN is NOT an address. PFN is an INDEX.

74. CALCULATION: PFN → physical_address. Formula: address = PFN × 4096. Example: PFN=1000 → address = 1000 × 4096 = 4096000 = 0x3E8000.

75. CALCULATION: physical_address → PFN. Formula: PFN = address ÷ 4096. Example: address=0x3E8000 = 4096000 → PFN = 4096000 ÷ 4096 = 1000.

76. WHY PFN EXISTS: Kernel tracks 3944069 pages. If kernel stored addresses (8 bytes each) = 3944069 × 8 = 31552552 bytes = 30 MB just for addresses. If kernel stores PFNs (4 bytes each initially, or uses them as indices) = smaller. Also: PFN as index into struct page array = O(1) lookup.

---

### Q2: HOW CAN I DEREF RAM ADDRESS?

77. WHY THIS DIAGRAM: You asked "how can i deref ram address" - you need to understand virtual vs physical addressing.

78. DRAW:
```
KERNEL CODE CANNOT DO THIS:
  int *ptr = (int *)0x3E8000;  // 0x3E8000 = physical address
  int value = *ptr;            // WRONG! CPU sees this as VIRTUAL address!

WHY: CPU ALWAYS uses MMU (Memory Management Unit). Every pointer = VIRTUAL address.
     CPU sends virtual address → MMU translates → physical address → RAM chip.

KERNEL CODE MUST DO THIS:
  unsigned long pfn = 1000;
  struct page *page = pfn_to_page(pfn);          // Get struct page for PFN
  void *vaddr = kmap(page);                      // Map physical page to virtual address
  int value = *(int *)vaddr;                     // NOW can dereference
  kunmap(page);                                  // Unmap when done
```

79. ANSWER: You CANNOT directly dereference physical address. CPU only understands virtual addresses. Kernel uses kmap() or phys_to_virt() to convert.

80. EXCEPTION: Kernel has "direct map" region. phys_to_virt(phys) = phys + PAGE_OFFSET. PAGE_OFFSET = 0xFFFF888000000000 on this machine.

81. CALCULATION: phys = 0x3E8000. virt = 0x3E8000 + 0xFFFF888000000000 = 0xFFFF8880003E8000. Now kernel can do: *(int *)0xFFFF8880003E8000.

---

### Q3: WHAT IS NUMA AND WHY DO I CARE?

82. WHY THIS DIAGRAM: You asked "what is this numa and why do i care" - you need to see the physical hardware layout.

83. DRAW (2-socket server, NOT your machine):
```
MOTHERBOARD:
+------------------+          INTERCONNECT          +------------------+
|   CPU SOCKET 0   |<========= QPI/UPI ==========>|   CPU SOCKET 1   |
|   (4 cores)      |          (slow wire)          |   (4 cores)      |
+--------+---------+                               +--------+---------+
         |                                                  |
    [RAM SLOT 0]                                       [RAM SLOT 2]
    [RAM SLOT 1]                                       [RAM SLOT 3]
         |                                                  |
    8 GB RAM                                           8 GB RAM
    (LOCAL to CPU 0)                                   (LOCAL to CPU 1)
    Access time: 10ns                                  Access time: 10ns
    
CPU 0 accessing RAM 0-1: 10 nanoseconds (LOCAL)
CPU 0 accessing RAM 2-3: 100 nanoseconds (REMOTE, through interconnect)
```

84. WHY YOU CARE: If your code runs on CPU 0 but allocates memory from RAM attached to CPU 1 → 10× slower. Kernel tries to allocate from LOCAL node.

85. YOUR MACHINE: 1 socket = 1 node = all RAM is local = no NUMA penalty. You don't care. Large servers care.

---

### Q4: DO WE HAVE DIFFERENT PFN IN EACH CPU / DOES EACH CPU OWN DIFFERENT PAGES?

86. WHY THIS DIAGRAM: You asked "do we have different pfn in each cpu" - this is a critical misconception.

87. DRAW:
```
WRONG MENTAL MODEL (what you thought):
  CPU 0: owns PFN 0-1000
  CPU 1: owns PFN 1001-2000
  CPU 2: owns PFN 2001-3000
  ← THIS IS WRONG

CORRECT MENTAL MODEL:
  ALL CPUs share ONE GLOBAL PFN numbering system:
  
  RAM chip 0 (attached to socket 0):  covers PFN 0 to 1000000
  RAM chip 1 (attached to socket 1):  covers PFN 1000001 to 2000000
  
  ANY CPU can access ANY PFN. PFN is GLOBAL, not per-CPU.
  The difference is SPEED, not ACCESSIBILITY.
```

88. ANSWER: PFN is GLOBAL. All CPUs see the same PFN numbers. All CPUs can access all pages. NUMA affects SPEED, not OWNERSHIP.

---

### Q5: WHO HAS THE GLOBAL PAGE POINTERS (VMEMMAP)?

89. WHY THIS DIAGRAM: You asked "who has the global page pointers" - the vmemmap array is what tracks all pages.

90. DRAW:
```
PHYSICAL RAM:
|--PAGE 0--|--PAGE 1--|--PAGE 2--|...|--PAGE 3944068--|
   4096B      4096B      4096B          4096B

VMEMMAP (kernel virtual memory, stored in RAM too):
Address: 0xFFFFEA0000000000 (on this machine it's 0xFFFFF89500000000 based on runtime)

vmemmap[0]     = struct page for PFN 0     (64 bytes)  ← at vmemmap + 0×64
vmemmap[1]     = struct page for PFN 1     (64 bytes)  ← at vmemmap + 1×64
vmemmap[2]     = struct page for PFN 2     (64 bytes)  ← at vmemmap + 2×64
...
vmemmap[3944068] = struct page for PFN 3944068        ← at vmemmap + 3944068×64

TOTAL: 3944069 × 64 bytes = 252420416 bytes = 240 MB for vmemmap array
```

91. FORMULA: pfn_to_page(pfn) = vmemmap + pfn. Returns pointer to struct page for that PFN.

92. FORMULA: page_to_pfn(page) = (page - vmemmap) / sizeof(struct page) = (page - vmemmap) / 64.

93. ANSWER: The KERNEL owns the global vmemmap array. It is stored in kernel virtual memory. All CPUs access the SAME vmemmap through their page tables.

---

### Q6: WHAT IF CPU X WANTS TO KNOW WHAT IS IN RAM 3 OF ANOTHER NUMA BANK?

94. WHY THIS DIAGRAM: You asked about cross-node access.

95. DRAW:
```
SCENARIO: 2-node server. CPU 0 wants to read PFN 1500000 which is on Node 1.

Step 1: CPU 0 calculates: I need struct page for PFN 1500000.
Step 2: CPU 0 computes: address = vmemmap + 1500000 × 64 = 0xFFFFEA0000000000 + 96000000 = 0xFFFFEA0005B8D800
Step 3: CPU 0 reads memory at 0xFFFFEA0005B8D800.
Step 4: MMU translates virtual → physical. Physical address is on Node 1's RAM.
Step 5: Memory request travels through interconnect to Node 1.
Step 6: Node 1's memory controller reads the struct page data.
Step 7: Data travels back through interconnect to CPU 0.
Step 8: CPU 0 gets the struct page contents.

TIME: ~100ns instead of ~10ns if it were local.
RESULT: It WORKS, just SLOWER.
```

96. ANSWER: ANY CPU can access ANY RAM. NUMA does not restrict access. It only affects latency.

---

### Q7: WHY PAGE POINTERS AT ALL? WHY NOT JUST USE PFN EVERYWHERE?

97. WHY THIS DIAGRAM: You asked "why page pointers at all".

98. DRAW:
```
OPTION 1: Use only PFN (integer)
  - PFN = 1000
  - To get flags: ??? need to store flags somewhere
  - To get refcount: ??? need to store refcount somewhere
  - To get zone: ??? need to calculate or store

OPTION 2: Use struct page (64-byte structure)
  struct page for PFN 1000:
  +0  : flags      (8 bytes) = 0x0000000000010000
  +8  : _refcount  (4 bytes) = 1
  +12 : _mapcount  (4 bytes) = -1
  +16 : mapping    (8 bytes) = NULL
  +24 : index      (8 bytes) = 0
  +32 : private    (8 bytes) = 0
  +40 : lru        (16 bytes) = linked list pointers
  +56 : padding    (8 bytes)
  TOTAL: 64 bytes

  - To get flags: page->flags = instant
  - To get refcount: page->_refcount = instant
  - To get zone: extracted from page->flags = instant
```

99. ANSWER: PFN alone = just a number. Kernel needs METADATA: who owns the page? is it free? is it mapped? how many users? struct page stores this metadata. PFN is the INDEX to find the struct page.

---

### SUMMARY DIAGRAM

100. DRAW:
```
YOUR MACHINE (1 node, no NUMA effect):

PHYSICAL RAM: 16154906624 bytes
|--byte 0--|--byte 1--|...|--byte 16154906623--|

DIVIDED INTO PAGES (each 4096 bytes):
|--PFN 0--|--PFN 1--|--PFN 2--|...|--PFN 3944068--|

TRACKED BY VMEMMAP (each struct page 64 bytes):
vmemmap[0] vmemmap[1] vmemmap[2] ... vmemmap[3944068]

LABELED BY ZONE (based on address):
|---DMA (PFN 0-4095)---|---DMA32 (PFN 4096-1048575)---|---Normal (PFN 1048576-3944068)---|
     16 MB                     ~4 GB                         ~11 GB

ALL OWNED BY NODE 0 (single NUMA node):
|--------------------------- NODE 0 ----------------------------|
```

---

### Q8: WHERE IS VMEMMAP PHYSICALLY STORED? WHAT IF IT'S ON A DIFFERENT NODE?

101. WHY THIS DIAGRAM: You asked "what if this virtual address belonged to a different cpu" - you're asking where the vmemmap array itself lives.

102. PROBLEM RESTATEMENT: vmemmap is a virtual address (0xFFFFEA0000000000). But the struct page data must be stored SOMEWHERE in physical RAM. Which node's RAM?

103. DRAW (OLD NAIVE DESIGN - BAD):
```
2-NODE SERVER, vmemmap stored entirely on Node 0:

NODE 0 PHYSICAL RAM:                     NODE 1 PHYSICAL RAM:
|--vmemmap array (240 MB)--|--other--|   |--user data--|--user data--|
   Contains struct page                    No vmemmap here
   for PFN 0 to 2000000
   
PROBLEM: CPU 1 wants struct page for PFN 1500000 (which is on Node 1).
         CPU 1 reads vmemmap[1500000].
         vmemmap is on Node 0's RAM.
         CPU 1 must travel through interconnect → Node 0 → get data → return.
         EVERY struct page access from CPU 1 = REMOTE ACCESS = SLOW.
```

104. THIS IS BAD BECAUSE: vmemmap is accessed VERY frequently. Every alloc_page, free_page, page fault reads vmemmap. If vmemmap on remote node = constant slow access.

105. DRAW (MODERN DESIGN - GOOD - VMEMMAP_SPLIT):
```
2-NODE SERVER, vmemmap distributed across nodes:

NODE 0 PHYSICAL RAM:                     NODE 1 PHYSICAL RAM:
|--vmemmap[0..999999]--|--other--|        |--vmemmap[1000000..1999999]--|--other--|
   struct page for Node 0 PFNs              struct page for Node 1 PFNs
   96 MB on Node 0                          96 MB on Node 1
   
CPU 0 accesses vmemmap[500000] → LOCAL (Node 0 RAM) → FAST
CPU 1 accesses vmemmap[1500000] → LOCAL (Node 1 RAM) → FAST
CPU 0 accesses vmemmap[1500000] → REMOTE (Node 1 RAM) → SLOW (but rare)
```

106. ANSWER: Modern kernels use SPARSE_VMEMMAP. vmemmap is split so that struct page for each node is stored in THAT node's RAM.

107. CALCULATION: Your machine has 3944069 pages × 64 bytes = 252420416 bytes = 240 MB vmemmap. With 1 node, all 240 MB is on Node 0. With 2 nodes (50/50 split), each node would store ~120 MB.

108. KERNEL CONFIG: CONFIG_SPARSEMEM_VMEMMAP=y. This enables distributed vmemmap. Source: `grep SPARSEMEM /boot/config-$(uname -r)`.

109. DRAW (DOUBLE INDIRECTION):
```
SCENARIO: CPU 0 on Node 0 needs to access page data at PFN 1500000 (on Node 1).

STEP 1: CPU 0 computes vmemmap address:
        vaddr = 0xFFFFEA0000000000 + 1500000 × 64 = 0xFFFFEA0005B8D800

STEP 2: CPU 0's MMU translates 0xFFFFEA0005B8D800:
        Page table lookup → finds physical address.

STEP 3: Physical address of vmemmap[1500000] = 0x??????? (on Node 1).
        This is where the struct page is stored.

STEP 4: Memory request goes:
        CPU 0 → L1 cache miss → L2 cache miss → L3 cache miss →
        → interconnect → Node 1 memory controller →
        → Node 1 RAM → data flows back → CPU 0 gets struct page.

PROBLEM: Two potential remote accesses:
  A) Reading vmemmap entry itself (struct page data) - can be remote
  B) Reading the actual page data the struct page describes - can be remote
  
With SPARSE_VMEMMAP: (A) is local if PFN belongs to local node.
Without SPARSE_VMEMMAP: (A) is always on Node 0 = BAD for Node 1 CPUs.
```

110. ANSWER TO YOUR SPECIFIC QUESTION:
```
Q: "What if the base address of physical page was on a different node?"

A: There are TWO levels:
   LEVEL 1: Where is struct page stored? (vmemmap location)
   LEVEL 2: Where is actual page data stored? (the 4096-byte page itself)

   Example: PFN 1500000 is on Node 1.
   
   With SPARSE_VMEMMAP:
   - struct page for PFN 1500000 stored on Node 1 (local to PFN)
   - Actual 4096 bytes at PFN 1500000 stored on Node 1
   
   CPU 0 accessing PFN 1500000:
   - Step 1: Read struct page → goes to Node 1 RAM → REMOTE → slow
   - Step 2: Read page data → goes to Node 1 RAM → REMOTE → slow
   Both are remote because PFN 1500000 belongs to Node 1.
   
   CPU 1 accessing PFN 1500000:
   - Step 1: Read struct page → goes to Node 1 RAM → LOCAL → fast
   - Step 2: Read page data → goes to Node 1 RAM → LOCAL → fast
   Both are local because CPU 1 is on Node 1.
```

111. WHY KERNEL DOES THIS: Kernel tries to allocate memory from local node. If your process runs on CPU 1, alloc_page() prefers Node 1. This minimizes remote access.

---

### Q9: WHERE IS NUMA INFO IN STRUCT PAGE? EXPLAIN EACH FIELD FROM SCRATCH.

112. WHY THIS SECTION: You asked "but i see no numa and explain each" - you noticed struct page doesn't have a "node" field. Where is it?

113. AXIOM: struct page is 64 bytes. Source: `pahole -C "struct page" vmlinux` or kernel source.

114. DRAW (struct page layout with EVERY field explained):
```
struct page for PFN 1000:
+-------+----------+--------+------------------------------------------+
|OFFSET | FIELD    | SIZE   | EXPLANATION (FROM SCRATCH)               |
+-------+----------+--------+------------------------------------------+
| +0    | flags    | 8 bytes| PACKED BITS: zone(3) + node(10) + flags  |
|       |          |        | NODE IS HERE! ZONE IS HERE!              |
+-------+----------+--------+------------------------------------------+
| +8    | _refcount| 4 bytes| How many users are using this page?      |
|       |          |        | alloc=1, get_page=+1, put_page=-1        |
+-------+----------+--------+------------------------------------------+
| +12   | _mapcount| 4 bytes| How many page tables map this page?      |
|       |          |        | -1=unmapped, 0=mapped once, N=mapped N+1 |
+-------+----------+--------+------------------------------------------+
| +16   | mapping  | 8 bytes| Pointer to address_space (file) or anon  |
|       |          |        | NULL if not file-backed or anonymous     |
+-------+----------+--------+------------------------------------------+
| +24   | index    | 8 bytes| Offset within file or swap slot number   |
+-------+----------+--------+------------------------------------------+
| +32   | private  | 8 bytes| Used by filesystem or buddy allocator    |
|       |          |        | buddy: stores order (0,1,2..10)          |
+-------+----------+--------+------------------------------------------+
| +40   | lru      | 16bytes| Linked list pointers (prev + next)       |
|       |          |        | Links page into free list or LRU list    |
+-------+----------+--------+------------------------------------------+
| +56   | (union)  | 8 bytes| Various overlapped fields                |
+-------+----------+--------+------------------------------------------+
TOTAL: 64 bytes
```

---

115. FIELD-BY-FIELD DERIVATION:

### FLAGS (8 bytes at offset +0):

116. PROBLEM: Kernel needs to know zone and node for each page. Options:
     A) Add separate "node" field (4 bytes) + "zone" field (4 bytes) = 8 extra bytes per page.
     B) Pack node and zone into existing flags field = 0 extra bytes.

117. SOLUTION: PACK zone (3 bits for 8 zones) and node (10 bits for 1024 nodes) into top bits of flags.

118. DRAW (flags field layout on x86_64):
```
flags (64 bits):
|-- NODE (bits 54-63) --|-- ZONE (bits 51-53) --|-- SECTION --|-- FLAGS (bits 0-27) --|
      10 bits                  3 bits                            28 bits

EXAMPLE: flags = 0x0000000000010000
Binary: 0000 0000 0000 0000 0000 0000 0000 0001 0000 0000 0000 0000

Bits 54-63 (node): 0000000000 = node 0
Bits 51-53 (zone): 000 = zone 0 = DMA  (or depends on encoding)
Bits 0-27 (flags): 0x10000 = PG_buddy flag set? or other flag
```

119. FORMULA: page_to_nid(page) = (page->flags >> NODES_PGSHIFT) & NODES_MASK.
120. FORMULA: page_zone(page) uses (page->flags >> ZONES_PGSHIFT) & ZONES_MASK.

121. VERIFY: On your machine with 1 node, NODES_PGSHIFT likely = 54, NODES_MASK = 0x3FF (10 bits).

---

### _REFCOUNT (4 bytes at offset +8):

122. PROBLEM: Multiple users may use same page (shared memory). Cannot free while in use.

123. SOLUTION: Count users. Increment on acquire. Decrement on release. Free when zero.

124. TRACE:
```
alloc_page()     → _refcount = 1 (one user: the allocator)
get_page(page)   → _refcount = 2 (second user acquired)
put_page(page)   → _refcount = 1 (second user released)
put_page(page)   → _refcount = 0 → PAGE FREED
put_page(page)   → _refcount = -1 → BUG! underflow
```

125. TYPE: atomic_t = 4 bytes. Atomic = multiple CPUs can safely increment/decrement without race.

---

### _MAPCOUNT (4 bytes at offset +12):

126. PROBLEM: Same physical page can be mapped into multiple processes' page tables.

127. DEFINITION: _mapcount = number of page table entries pointing to this page MINUS ONE.

128. TRACE:
```
_mapcount = -1  → page is NOT mapped in any page table
_mapcount = 0   → page is mapped in 1 page table (0 + 1 = 1)
_mapcount = 5   → page is mapped in 6 page tables (5 + 1 = 6)
```

129. WHY MINUS ONE: Saves checking for zero. Unmapped = -1. First map: -1+1=0. Second map: 0+1=1.

130. USE: Kernel checks if page is mapped before freeing. If _mapcount ≠ -1 → page still mapped → don't free physical frame yet.

---

### MAPPING (8 bytes at offset +16):

131. PROBLEM: Page may contain file data (backed by disk) or anonymous data (no file backing).

132. DEFINITION: mapping = pointer to struct address_space (for file pages) or anon_vma (for anonymous pages).

133. VALUES:
```
mapping = NULL              → page not associated with any file
mapping = 0xFFFF888012345678 → points to address_space of some file
mapping = 0xFFFF888012345679 → LSB=1 means anonymous page (pointer mangled)
```

134. USE: When page is dirty, kernel uses mapping to find which file to write back to.

---

### INDEX (8 bytes at offset +24):

135. PROBLEM: Page contains part of a file. Which part? Offset 0? Offset 4096? Offset 8192?

136. DEFINITION: index = offset within file in pages. index=5 → this page contains bytes 5×4096 to 6×4096-1 of file.

137. ALTERNATIVE: For swap pages, index = swap slot number.

---

### PRIVATE (8 bytes at offset +32):

138. PROBLEM: Buddy allocator needs to know order of free page. Filesystem needs to store buffer_head pointer.

139. DEFINITION: private = multi-purpose field. Meaning depends on page state.

140. USES:
```
For buddy allocator (free page): private = order (0, 1, 2, ... 10)
For filesystem:                  private = pointer to buffer_head
For compound page tail:          private = pointer to head page
```

---

### LRU (16 bytes at offset +40):

141. PROBLEM: Need to track which pages to evict when memory is low. Need linked lists.

142. DEFINITION: lru = struct list_head = { next pointer (8 bytes), prev pointer (8 bytes) }.

143. USES:
```
For free pages: links into buddy allocator's free list
For active pages: links into LRU (Least Recently Used) list
For slab pages: links into slab's page list
```

144. DRAW:
```
free_area[3].free_list → page_A.lru ↔ page_B.lru ↔ page_C.lru → (circular)
                         ↑                                    |
                         +------------------------------------+
```

---

### SUMMARY: WHERE IS NODE/ZONE STORED?

145. ANSWER:
```
NODE: packed in page->flags, bits 54-63 (10 bits = up to 1024 nodes)
ZONE: packed in page->flags, bits 51-53 (3 bits = up to 8 zones)

To extract node: page_to_nid(page) = (flags >> 54) & 0x3FF
To extract zone: page_zonenum(page) = (flags >> 51) & 0x7

Both are NOT separate fields. Both live inside the 8-byte flags field.
This is why you didn't see a "node" field - it's HIDDEN in flags.
```

146. WHY PACK: Space efficiency. 3944069 pages × 8 extra bytes for node+zone = 31.5 MB wasted. Better to pack into existing flags.

---

### Q10: TLB ON YOUR MACHINE (REAL DATA)

147. AXIOM: Your CPU = AMD Ryzen 5 4600H. Source: `lscpu` → "AMD Ryzen 5 4600H with Radeon Graphics".

148. AXIOM: Your machine has 12 CPUs (6 cores × 2 threads). Source: `lscpu` → "CPU(s): 12".

149. AXIOM: Each CPU has TLB. Source: `/proc/cpuinfo` → "TLB size: 3072 4K pages".

150. REAL DATA FROM YOUR MACHINE:
```
cat /proc/cpuinfo | grep "TLB size":
TLB size        : 3072 4K pages

CALCULATION: 3072 entries × 4 KB per page = 3072 × 4096 = 12582912 bytes = 12 MB addressable per TLB
```

151. WHY THIS DIAGRAM: You asked "who talks to TLB" and "is TLB on each node".

152. DRAW (TLB location in hardware):
```
YOUR MACHINE (AMD Ryzen 5 4600H):

+------------------------------------------------------------------+
|                         CPU CHIP (SOCKET 0)                       |
|  +------------+  +------------+  +------------+                   |
|  |  CORE 0    |  |  CORE 1    |  |  CORE 2    |  ... (6 cores)   |
|  | +--------+ |  | +--------+ |  | +--------+ |                   |
|  | |Thread 0| |  | |Thread 0| |  | |Thread 0| |                   |
|  | | [TLB]  | |  | | [TLB]  | |  | | [TLB]  | |                   |
|  | +--------+ |  | +--------+ |  | +--------+ |                   |
|  | +--------+ |  | +--------+ |  | +--------+ |                   |
|  | |Thread 1| |  | |Thread 1| |  | |Thread 1| |                   |
|  | | [TLB]  | |  | | [TLB]  | |  | | [TLB]  | |                   |
|  | +--------+ |  | +--------+ |  | +--------+ |                   |
|  +-----+------+  +-----+------+  +-----+------+                   |
|        |               |               |                          |
|        v               v               v                          |
|  +----------------------------------------------------------+    |
|  |                    L3 CACHE (8 MB)                       |    |
|  +----------------------------------------------------------+    |
+------------------------------------------------------------------+
                              |
                              v
         +------------------------------------------+
         |              RAM (16 GB)                 |
         |  (Page tables live here)                 |
         |  (vmemmap lives here)                    |
         +------------------------------------------+

TLB is INSIDE each CPU thread. NOT in RAM. NOT affected by RAM size.
```

153. ANSWER: Who talks to TLB?
```
CPU core executes instruction → needs memory address → address is VIRTUAL
CPU sends virtual address to MMU (inside CPU) → MMU checks TLB first
TLB hit: MMU returns physical address immediately (fast, ~1 cycle)
TLB miss: MMU walks page table (in RAM) → slow (~100 cycles) → caches result in TLB
```

154. DRAW (TLB lookup flow):
```
CPU instruction: MOV RAX, [0xFFFF888012345000]   ← virtual address

Step 1: CPU → asks MMU → "translate 0xFFFF888012345000"
Step 2: MMU → checks TLB → "do I have entry for this page?"
        TLB entry format: { virtual_page_number → physical_page_number, flags }
        
Step 3a: TLB HIT (entry found):
         MMU → returns physical address → CPU reads RAM → 1-10 cycles
         
Step 3b: TLB MISS (entry NOT found):
         MMU → reads CR3 register → gets page table base address
         MMU → walks 4-level page table in RAM:
               CR3 → PML4[offset] → PDPT[offset] → PD[offset] → PT[offset] → physical address
         MMU → caches result in TLB → returns physical address
         Time: ~100-300 cycles
```

---

### Q11: DOES ADDING RAM INCREASE TLB?

155. WHY YOU ASKED: You recently added RAM to your machine. You want to know if TLB grew.

156. ANSWER: NO. TLB is HARDWARE inside CPU chip. RAM is separate chip on motherboard.

157. DRAW:
```
BEFORE: 8 GB RAM                      AFTER: 16 GB RAM
+--------+                            +--------+
|  CPU   |  TLB = 3072 entries        |  CPU   |  TLB = 3072 entries (SAME)
+--------+                            +--------+
    |                                     |
+--------+                            +--------+--------+
| 8 GB   |                            | 8 GB   | 8 GB   |  (you added this)
| RAM    |                            | RAM    | RAM    |
+--------+                            +--------+--------+

TLB size determined by: CPU model (AMD Ryzen 5 4600H → 3072 entries)
TLB size NOT determined by: RAM size

To increase TLB: buy different CPU with larger TLB.
```

158. WHAT ADDING RAM DOES AFFECT:
```
- More pages available (16 GB = 4194304 pages vs 8 GB = 2097152 pages)
- More TLB MISSES possible (more pages to potentially access)
- More page table entries (page tables take more RAM)
- vmemmap larger (4M pages × 64 bytes = 256 MB vmemmap)
```

---

### Q12: YOUR TLB SPECIFICATIONS (REAL DATA)

159. SOURCE: /proc/cpuinfo, AMD documentation

160. YOUR TLB DATA:
```
CPU: AMD Ryzen 5 4600H
TLB size: 3072 4K pages (per thread? per core? - need AMD docs to confirm)

BREAKDOWN (typical AMD Zen 2):
- L1 iTLB (instruction): 64 entries, fully associative, 4K pages
- L1 dTLB (data): 64 entries, fully associative, 4K pages
- L2 TLB (unified): 2048 entries, 8-way associative

TOTAL: 64 + 64 + 2048 = 2176 (approximately, varies by exact model)
cpuinfo reports 3072 - may include 2M page TLB entries
```

161. CALCULATION: TLB coverage
```
L1 dTLB: 64 entries × 4 KB = 256 KB directly addressable without L1 dTLB miss
L2 TLB: 2048 entries × 4 KB = 8 MB directly addressable without L2 TLB miss
```

---

## ERROR REPORT: YOUR MISTAKES

---

### MISTAKE 1: NODE = RAM OR NODE = CHIP?

162. YOUR QUESTION: "node is ram or node is a chip"

163. CONFUSION EXPOSED:
```
You conflated: NODE, RAM, CHIP
These are 3 different things:

NODE = software concept = set of memory + CPUs with similar access latency
RAM = hardware = physical memory chips (DDR4 modules on your motherboard)
CHIP = ambiguous = could mean CPU chip or RAM chip

YOUR MACHINE: 1 node containing 1 CPU chip + 2 RAM sticks
```

164. FIX: Node is neither RAM nor chip. Node is a GROUPING.

---

### MISTAKE 2: WHO TALKS TO TLB?

165. YOUR QUESTION: "who talks to the tlb the ram or the mmu or the cpu itself"

166. CONFUSION EXPOSED:
```
RAM does NOT "talk" to anything. RAM is passive storage. RAM waits for requests.

CHAIN:
CPU core → MMU (inside CPU) → TLB (inside MMU) → if miss → page tables (in RAM)

TLB is part of MMU. MMU is part of CPU. CPU talks to itself.
RAM only responds when MMU needs to walk page tables on TLB miss.
```

---

### MISTAKE 3: ADDING RAM = BIGGER TLB?

167. YOUR QUESTION: "does putting extra ram affect tlb"

168. CONFUSION EXPOSED:
```
You thought: more RAM → bigger TLB

REALITY:
TLB size = fixed by CPU design (etched in silicon at factory)
RAM size = changeable by you (plug in more DIMMs)

These are INDEPENDENT. Adding RAM does NOT change TLB.
Adding RAM changes: available pages, page table size, vmemmap size.
```

---

### MISTAKE 4: TYPING WASTE

169. YOUR MESSAGE CONTAINED:
```
"what is there on my tlb -- how much bigg -- fetch real sources"
"whatt is the tlb and fetch data"
"does tht increase tlb"

Pattern: double letters (bigg, whatt, tht)
Pattern: rushed typing, not rereading
Pattern: asking questions already answered in previous lines
```

170. DIAGNOSIS: Typing faster than thinking. Not reading responses before asking.

---

### MISTAKE 5: INABILITY TO RUN TO MEAT

171. PATTERN:
```
You asked 7 questions in one message:
- is tlb on each node
- who talks to tlb
- how many tlb
- what is tlb size
- does ram increase tlb
- what is on my tlb
- fetch real sources

Instead of: running ONE command, reading output, then asking follow-up.
```

172. FIX: One question → one answer → verify → next question.

---

### ROOT CAUSE ANALYSIS

173. DRAW:
```
YOUR MENTAL MODEL (WRONG):
+-------+       +-------+       +-------+
| CPU   | ---→  | TLB   | ---→  | RAM   |
| chip  |       | (???) |       | stores|
|       |       | where?|       | TLB?  |
+-------+       +-------+       +-------+

CORRECT MODEL:
+--------------------------------------------------+
|                   CPU CHIP                        |
|  +------+    +------+    +--------+              |
|  | Core | →  | MMU  | →  | TLB    |  (all inside)|
|  +------+    +------+    +--------+              |
+---------------------+----------------------------+
                      |
                      | (only on TLB miss)
                      v
              +---------------+
              | RAM           |
              | (page tables) |
              +---------------+
```

---

### Q13: STRUCT PAGE FIELDS FOR MALLOC, FILE, SOCKET

174. WHY THIS SECTION: You asked what happens to struct page fields for malloc, file open, socket.

---

### SCENARIO 1: MALLOC (ANONYMOUS MEMORY)

175. USER CODE:
```c
char *buf = malloc(8192);  // 8192 bytes = 2 pages
buf[0] = 'A';              // First write triggers page fault
```

176. KERNEL PATH: malloc → brk/mmap syscall → page fault → do_anonymous_page()

177. SOURCE: `/usr/src/linux-source-6.8.0/mm/memory.c` line 4259-4379

178. DRAW (struct page after malloc + first access):
```
ANONYMOUS PAGE (from do_anonymous_page):
+----------+----------------------------------------------+
| FIELD    | VALUE                                        |
+----------+----------------------------------------------+
| flags    | PG_uptodate | PG_lru | PG_swapbacked         |
|          | + zone bits (Normal) + node bits (0)         |
+----------+----------------------------------------------+
| _refcount| 1 (allocated, one user)                      |
+----------+----------------------------------------------+
| _mapcount| 0 (mapped in 1 page table: user process)     |
+----------+----------------------------------------------+
| mapping  | 0xFFFF...001 (anon_vma pointer | 0x1)        |
|          | LSB=1 means ANONYMOUS (PAGE_MAPPING_ANON)    |
+----------+----------------------------------------------+
| index    | Virtual address >> PAGE_SHIFT (page offset)  |
+----------+----------------------------------------------+
| private  | 0 (not used for anon pages in normal state)  |
+----------+----------------------------------------------+
| lru      | Linked into LRU list (active_anon list)      |
+----------+----------------------------------------------+
```

179. KEY SOURCE LINE (mm/memory.c:4359):
```c
folio_add_new_anon_rmap(folio, vma, addr);  // Sets mapping to anon_vma | 0x1
```

180. HOW TO DETECT ANONYMOUS:
```c
// Source: include/linux/page-flags.h line 649-672
#define PAGE_MAPPING_ANON 0x1
bool is_anon = (page->mapping & PAGE_MAPPING_ANON) != 0;
```

---

### SCENARIO 2: FILE READ (FILE-BACKED MEMORY)

181. USER CODE:
```c
int fd = open("/etc/passwd", O_RDONLY);
char *buf = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
char c = buf[0];  // Triggers page fault → reads file
```

182. KERNEL PATH: open → mmap → page fault → __do_fault → filemap_fault

183. DRAW (struct page after file read):
```
FILE-BACKED PAGE (from filemap_fault):
+----------+----------------------------------------------+
| FIELD    | VALUE                                        |
+----------+----------------------------------------------+
| flags    | PG_uptodate | PG_lru | PG_referenced          |
|          | + zone bits + node bits                      |
+----------+----------------------------------------------+
| _refcount| 2+ (page cache + mmap)                       |
+----------+----------------------------------------------+
| _mapcount| 0 (mapped in 1 page table)                   |
+----------+----------------------------------------------+
| mapping  | 0xFFFF888012345000 (address_space of file)   |
|          | LSB=0 means FILE-BACKED (not anon)           |
+----------+----------------------------------------------+
| index    | 0 (offset 0 in file, in pages)               |
|          | If reading offset 8192, index = 2            |
+----------+----------------------------------------------+
| private  | buffer_head pointer (for some filesystems)   |
+----------+----------------------------------------------+
| lru      | Linked into LRU list (inactive_file list)    |
+----------+----------------------------------------------+
```

184. KEY: mapping points to address_space = { inode, pages tree, ... }

185. SOURCE (mm/memory.c:4574):
```c
folio_add_file_rmap_ptes(folio, page, nr, vma);  // Increments _mapcount
```

186. HOW KERNEL KNOWS IT'S FILE:
```c
// mapping != NULL and LSB = 0 → file-backed
struct address_space *as = page->mapping;  // No bit manipulation
struct inode *inode = as->host;            // The file's inode
```

---

### SCENARIO 3: SOCKET (NETWORK BUFFER)

187. USER CODE:
```c
int sock = socket(AF_INET, SOCK_STREAM, 0);
// ...connect...
send(sock, "hello", 5, 0);
```

188. KERNEL PATH: send syscall → socket layer → sk_buff allocation → page as fragment

189. IMPORTANT: Sockets use sk_buff (socket buffer), NOT struct page directly for small data. BUT for large sends or zero-copy, pages are used.

190. DRAW (struct page for socket fragment):
```
SOCKET PAGE (from sock_alloc_send_pskb):
+----------+----------------------------------------------+
| FIELD    | VALUE                                        |
+----------+----------------------------------------------+
| flags    | PG_slab (if from slab) or normal page flags  |
+----------+----------------------------------------------+
| _refcount| 1+ (held by sk_buff)                         |
+----------+----------------------------------------------+
| _mapcount| -1 (NOT mapped in any user page table)       |
+----------+----------------------------------------------+
| mapping  | NULL (not file-backed, not anonymous)        |
+----------+----------------------------------------------+
| index    | unused for network                           |
+----------+----------------------------------------------+
| private  | may hold skb pointer or fragment info        |
+----------+----------------------------------------------+
| lru      | NOT in LRU (network pages don't get reclaimed)|
+----------+----------------------------------------------+
```

191. SOURCE (net/core/sock.c:2757):
```c
struct sk_buff *sock_alloc_send_pskb(struct sock *sk, unsigned long header_len,
                                      unsigned long data_len, int noblock, int *errcode)
```

192. KEY DIFFERENCE: Socket pages are:
- _mapcount = -1 (never user-mapped)
- mapping = NULL (no file backing)
- Not on LRU (kernel controls lifetime)

---

### COMPARISON TABLE

193. DRAW:
```
+-------------+------------+------------+-------------+-------------+
| SCENARIO    | mapping    | _mapcount  | index       | lru         |
+-------------+------------+------------+-------------+-------------+
| malloc      | anon_vma|1 | 0+         | vaddr>>12   | active_anon |
| file mmap   | addr_space | 0+         | file offset | inactive_file|
| socket      | NULL       | -1         | unused      | not in LRU  |
+-------------+------------+------------+-------------+-------------+
| DETECTION   | LSB=1→anon | -1→unmapped| file:pageidx| reclaim?    |
|             | LSB=0→file |  0+→mapped | anon:vaddr  |             |
+-------------+------------+------------+-------------+-------------+
```

194. FLAGS SUMMARY:
```
malloc page:  PG_uptodate | PG_lru | PG_swapbacked | PG_anon
file page:    PG_uptodate | PG_lru | PG_referenced
socket page:  may have PG_slab, no PG_lru
```

---

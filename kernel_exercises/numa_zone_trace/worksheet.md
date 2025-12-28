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

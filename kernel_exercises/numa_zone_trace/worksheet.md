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

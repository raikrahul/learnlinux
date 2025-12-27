---
layout: home
title: Learn Linux Internals
---

# Learn Linux Internals

Deep dive into Linux kernel memory management, page tables, and system internals.

## Topics

### Memory Management
- [CR3 Register and Page Tables](/tutorials/cr3-register/)
- [Page Table Index Extraction](/tutorials/index-extraction/)
- [4KB, 2MB, and 1GB Page Walks](/tutorials/page-walks/)
- [TLB and PCID/ASID](/tutorials/tlb-pcid/)

### Allocators
- [Buddy Allocator](/tutorials/buddy-allocator/)
- [Slab Allocator](/tutorials/slab-allocator/)

### Process Memory
- [Virtual Memory Areas (VMA)](/tutorials/vma/)
- [Copy-on-Write (COW)](/tutorials/cow/)
- [Swap Subsystem](/tutorials/swap/)

## Kernel Module Demos

| Demo | Description |
|------|-------------|
| [demo_01](/demos/demo_01/) | Read CR3 Register |
| [demo_02](/demos/demo_02/) | Extract Page Table Indices |
| [demo_03](/demos/demo_03/) | Read Page Table Entries |
| [demo_04](/demos/demo_04/) | Check Present Bit and Flags |
| [demo_05](/demos/demo_05/) | Detect Huge Pages |
| [demo_06](/demos/demo_06/) | Extract Physical Address |
| [demo_07](/demos/demo_07/) | Complete 4KB Page Walk |
| [demo_08](/demos/demo_08/) | 2MB Huge Page Walk |
| [demo_09](/demos/demo_09/) | 1GB Huge Page Walk |
| [demo_10](/demos/demo_10/) | Compare __va Macro |
| [demo_11](/demos/demo_11/) | Dump PML4 Table |
| [demo_12](/demos/demo_12/) | Process-specific CR3 |
| [demo_13](/demos/demo_13/) | Memory Zones |
| [demo_14](/demos/demo_14/) | struct page Flags |
| [demo_15](/demos/demo_15/) | VMA Walk |
| [demo_16](/demos/demo_16/) | Slab Allocator Info |
| [demo_17](/demos/demo_17/) | Buddy Allocator Info |
| [demo_18](/demos/demo_18/) | TLB Flush |
| [demo_19](/demos/demo_19/) | Copy-on-Write Demo |
| [demo_20](/demos/demo_20/) | Swap Subsystem Info |

## Machine Specifications

```
CPU: AMD Ryzen 5 4600H
RAM: 15406 MB
Kernel: 6.14.0-37-generic
page_offset_base: 0xFFFF89DF00000000 (KASLR randomized)
Physical Address: 44 bits
Virtual Address: 48 bits
Page Table Levels: 4 (5-level disabled on this CPU)
```

## Getting Started

```bash
# Clone the repository
git clone https://github.com/raikrahul/learnlinux.git
cd learnlinux

# Build with Jekyll
bundle install
bundle exec jekyll serve

# Open http://localhost:4000/learnlinux/
```

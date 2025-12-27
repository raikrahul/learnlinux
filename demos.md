---
layout: default
title: Demos
permalink: /demos/
---

# Kernel Module Demos

Hands-on kernel modules for exploring memory management.

## Page Table Basics

| Demo | Driver | Description |
|------|--------|-------------|
| [Demo 01](/demos/demo_01/) | cr3_driver.c | Read CR3 Register |
| [Demo 02](/demos/demo_02/) | indices_driver.c | Extract Page Table Indices |
| [Demo 03](/demos/demo_03/) | entry_driver.c | Read Page Table Entries |
| [Demo 04](/demos/demo_04/) | present_driver.c | Check Present Bit and Flags |
| [Demo 05](/demos/demo_05/) | huge_driver.c | Detect Huge Pages |
| [Demo 06](/demos/demo_06/) | addr_driver.c | Extract Physical Address |

## Page Walks

| Demo | Driver | Description |
|------|--------|-------------|
| [Demo 07](/demos/demo_07/) | walk4kb_driver.c | Complete 4KB Page Walk |
| [Demo 08](/demos/demo_08/) | walk2mb_driver.c | 2MB Huge Page Walk |
| [Demo 09](/demos/demo_09/) | walk1gb_driver.c | 1GB Huge Page Walk |
| [Demo 10](/demos/demo_10/) | va_driver.c | Compare __va Macro |

## Process and Memory

| Demo | Driver | Description |
|------|--------|-------------|
| [Demo 11](/demos/demo_11/) | pml4_driver.c | Dump PML4 Table |
| [Demo 12](/demos/demo_12/) | pid_driver.c | Process-specific CR3 |
| [Demo 13](/demos/demo_13/) | zone_driver.c | Memory Zones |
| [Demo 14](/demos/demo_14/) | flags_driver.c | struct page Flags |
| [Demo 15](/demos/demo_15/) | vma_driver.c | VMA Walk |

## Allocators

| Demo | Driver | Description |
|------|--------|-------------|
| [Demo 16](/demos/demo_16/) | slab_driver.c | Slab Allocator Info |
| [Demo 17](/demos/demo_17/) | buddy_driver.c | Buddy Allocator Info |

## Advanced

| Demo | Driver | Description |
|------|--------|-------------|
| [Demo 18](/demos/demo_18/) | tlb_driver.c | TLB Flush |
| [Demo 19](/demos/demo_19/) | cow_driver.c | Copy-on-Write Demo |
| [Demo 20](/demos/demo_20/) | swap_driver.c | Swap Subsystem Info |

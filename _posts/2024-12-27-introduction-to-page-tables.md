---
title: "Introduction to Linux Page Tables"
date: 2024-12-27
categories: [memory, fundamentals]
tags: [page-tables, cr3, pml4]
---

Welcome to Learn Linux Internals. This first post introduces the 4-level page table structure used by x86-64 Linux.

## CR3 Register

The CR3 register holds the physical address of the top-level page table (PML4):

```
CR3 Format:
┌─────────────────────────────────────────────────────────┐
│ 63:52 │ 51:12              │ 11:0                      │
│ Rsvd  │ PML4 Physical Addr │ PCID (if enabled)         │
└─────────────────────────────────────────────────────────┘
```

## 4-Level Walk

```
VA → PML4[idx] → PDPT[idx] → PD[idx] → PT[idx] → Physical Page
     bits 47:39   bits 38:30   bits 29:21  bits 20:12
```

Each level has 512 entries (9 bits), each entry is 8 bytes.

## Next Steps

See the [CR3 Demo](/demos/demo_01/) for hands-on exploration.

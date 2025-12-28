# Linux Memory Lesson 1: Reverse Mapping (RMAP) Axiomatic Derivation

```
ADHD RULES: dense paragraphs, no headings within content, wide lines for wide screens, each line = one fact or calculation, ✓✗∴→ symbols, ASCII diagrams with real addresses, kernel source line numbers as proof, no filler words, no adjectives, no adverbs, define every term before use, derive every formula before use.
```

## AXIOM DEFINITIONS (DEFINE ALL TERMS BEFORE USE)

```
D01. Byte = smallest addressable unit, values 0-255, address 0x0 = first byte of RAM, address 0x7FFFFFFFF = last byte of 32GB RAM.
D02. RAM = physical bytes at addresses 0x0 to 0x7FFFFFFFF on this machine, run `cat /proc/meminfo | grep MemTotal` → 15776276 kB ≈ 15GB.
D03. Disk = storage device with bytes, run `stat /usr/lib/x86_64-linux-gnu/libc.so.6` → size=2125328 bytes.
D04. File = sequence of bytes on disk, libc.so.6 = byte 0 to byte 2125327, byte offset = position in file.
D05. PAGE = 4096 consecutive bytes, WHY: managing billions of bytes one-by-one slow, hardware transfers 4096 bytes at a time, run `getconf PAGE_SIZE` → 4096 ✓.
D06. PFN = Page Frame Number = physical_address / 4096, index of 4096-byte chunk in RAM, PFN 0 = bytes 0-4095, PFN 0x123456 = bytes 0x123456000-0x123456FFF.
D07. File chunk = 4096-byte portion of file, chunk N = file bytes N×4096 to N×4096+4095, WHY: kernel reads files in 4096-byte chunks, kernel tracks which file chunks are in RAM.
D08. struct page = 64-byte kernel structure describing one RAM page, WHY: kernel needs metadata per page (reference count, what data is in it, who is using it).
D09. mem_map = array of struct page, mem_map[PFN] describes RAM at physical address PFN×4096, 15GB/4096×64 bytes = ~240MB for all struct page metadata.
D10. page→index = file chunk number stored in this RAM page, if page→index=96, RAM holds file bytes 96×4096 to 96×4096+4095 = 393216 to 397311.
D11. page→mapping = pointer to struct address_space, identifies WHICH FILE's data is in this RAM page, not RAM position, FILE identity.
D12. struct address_space = file's page cache, container {file_chunk_number → struct page pointer}, lives inside inode, kernel source: include/linux/fs.h line 460.
D13. struct inode = kernel structure describing one file on disk, inode→i_mapping = pointer to address_space, run `ls -i /usr/lib/.../libc.so.6` → inode=5160837.
D14. struct file = kernel structure for one open file handle, file→f_mapping = pointer to address_space (same as inode→i_mapping).
D15. VMA = struct vm_area_struct = describes one contiguous vaddr range in process, kernel source: include/linux/mm_types.h line 649.
D16. vm_start = first vaddr of VMA, example: 0x795df3828000 from /proc/self/maps.
D17. vm_end = last vaddr + 1 of VMA, example: 0x795df39b0000, VMA covers [vm_start, vm_end) = contiguous range ← kernel guarantees this.
D18. vm_pgoff = which file chunk maps to vm_start, if vm_pgoff=40, vaddr vm_start reads file chunk 40 (bytes 163840-167935), kernel source: mm_types.h line 721 `unsigned long vm_pgoff; /* Offset (within vm_file) in PAGE_SIZE units */`
D19. vm_file = pointer to struct file mapped into this VMA, VMA→vm_file→f_mapping→address_space identifies file, NULL for anonymous (malloc, stack).
D20. mm_struct = process address space, contains all VMAs of one process, mm→pgd = page table root (CR3), mm→mmap = VMA list.
D21. RMAP = Reverse Mapping, given PFN find all vaddrs in all processes pointing to it, WHY: kernel must find PTEs to clear before freeing RAM.
```

## VMA → FILE → RAM CHAIN (DIAGRAM THEN EXPLANATION)

```
CHAIN: VMA.vm_file → struct file → f_mapping → address_space ← page→mapping
                                                      ↑
                            inode→i_mapping───────────┘

┌─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                      DISK: libc.so.6 (inode #5160837, 2125328 bytes)                                                                    │
│                                                           │                                                                                              │
│                                                           ▼                                                                                              │
│                              ┌────────────────────────────────────────────────────────────────┐                                                          │
│                              │  struct inode at 0xFFFF888012340000                            │                                                          │
│                              │  i_ino = 5160837                                               │                                                          │
│                              │  i_mapping = 0xFFFF888012340100 ────────────────────────────┐  │                                                          │
│                              └────────────────────────────────────────────────────────────│───┘                                                          │
│                                                                                           │                                                               │
│                                                                                           ▼                                                               │
│         ┌────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐│
│         │  struct address_space at 0xFFFF888012340100 (PAGE CACHE for libc.so.6)                                                                         ││
│         │  i_pages = { file_chunk 40 → PFN 0xAAAA, file_chunk 96 → PFN 0x123456, file_chunk 97 → PFN 0xBBBB }                                            ││
│         │                           ↑                                                         ↑                                                          ││
│         └───────────────────────────│─────────────────────────────────────────────────────────│──────────────────────────────────────────────────────────┘│
│                                     │                                                         │                                                           │
│    ┌────────────────────────────────┘                                                         └────────────────────────────────────────────┐              │
│    │                                                                                                                                       │              │
│    │  page→mapping points here                                                                                     f_mapping points here  │              │
│    │                                                                                                                                       │              │
│    ▼                                                                                                                                       ▼              │
│ ┌────────────────────────────────────┐                                                                   ┌─────────────────────────────────────────────┐ │
│ │  struct page (PFN 0x123456)        │                                                                   │  struct file at 0xFFFF888099990000         │ │
│ │  mapping = 0xFFFF888012340100      │                                                                   │  f_mapping = 0xFFFF888012340100            │ │
│ │  index = 96                        │                                                                   │                                             │ │
│ │  ∴ RAM holds file chunk 96         │                                                                   └──────────────────────────────────┬──────────┘ │
│ └────────────────────────────────────┘                                                                                                      │            │
│                                                                                                               vm_file points here           │            │
│                                                                                                                                             │            │
│                                                                                         ┌───────────────────────────────────────────────────┘            │
│                                                                                         │                                                                 │
│                                                                                         ▼                                                                 │
│         ┌───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐│
│         │  PROCESS A (pid=12345): struct vm_area_struct                                                                                                  ││
│         │  vm_start = 0x795df3828000, vm_end = 0x795df39b0000, vm_pgoff = 40, vm_file = 0xFFFF888099990000                                               ││
│         │  MEANING: vaddr 0x795df3828000 maps to file chunk 40, vaddr range is [0x795df3828000, 0x795df39b0000) = 0x188000 bytes = 392 pages             ││
│         └───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

```
KERNEL SOURCE PROOF FOR LINEARITY (mm_types.h):
Line 654: /* VMA covers [vm_start; vm_end) addresses within mm */ unsigned long vm_start; unsigned long vm_end; ← CONTIGUOUS range guaranteed by kernel design, no gaps, no reordering.
Line 721: unsigned long vm_pgoff; /* Offset (within vm_file) in PAGE_SIZE units */ ← vm_pgoff is FILE position in pages, not RAM position.
Line 723: struct file * vm_file; /* File we map to (can be NULL). */ ← vm_file connects VMA to file.
```

## page→index AND vm_pgoff (BOTH ARE FILE POSITIONS NOT RAM POSITIONS)

```
CONFUSION: "vm_pgoff irritating me, page is from RAM, PFN is from RAM, but this is file" ← CORRECT observation, vm_pgoff and page→index are FILE positions.

┌──────────────┬─────────────────────────────────────────────────────────────┐
│ TERM         │ MEANING                                                     │
├──────────────┼─────────────────────────────────────────────────────────────┤
│ PFN          │ RAM position (which 4096-byte chunk of physical memory)     │
│ vm_pgoff     │ FILE position (which file chunk maps to vm_start)           │
│ page→index   │ FILE position (which file chunk is stored in this RAM page) │
└──────────────┴─────────────────────────────────────────────────────────────┘

NUMERICAL EXAMPLE: vm_pgoff=40 means vaddr vm_start (0x795df3828000) corresponds to file chunk 40 (file bytes 163840-167935), derived from /proc/self/maps offset field 0x28000 = 163840 bytes, 163840 / 4096 = 40 chunks.
NUMERICAL EXAMPLE: page→index=96 means RAM page at PFN 0x123456 holds file chunk 96 (file bytes 393216-397311), kernel stored this when copying file data to RAM during page fault.
```

## FORMULA DERIVATION: vaddr = vm_start + (page→index - vm_pgoff) × PAGE_SIZE

```
GIVEN DATA (from /proc/self/maps): vm_start = 0x795df3828000, vm_pgoff = 40, page→index = 96 (file chunk kernel wants to find vaddr for).
STEP 1: vm_start corresponds to file chunk 40 (from vm_pgoff=40), derived above: offset=0x28000=163840 bytes, 163840/4096=40.
STEP 2: How many chunks after chunk 40 is chunk 96? chunks_after = page→index - vm_pgoff = 96 - 40 = 56 chunks.
STEP 3: How many bytes is 56 chunks? bytes_after = 56 × 4096 = 229376 bytes = 0x38000 bytes.
STEP 4: What vaddr is 229376 bytes after vm_start? vaddr = vm_start + bytes_after = 0x795df3828000 + 0x38000 = 0x795df3860000.
COMBINING: vaddr = vm_start + (page→index - vm_pgoff) × PAGE_SIZE = 0x795df3828000 + (96 - 40) × 4096 = 0x795df3828000 + 56 × 4096 = 0x795df3828000 + 0x38000 = 0x795df3860000 ✓

LINEARITY PROOF: VMA covers [vm_start, vm_end) = CONTIGUOUS range (kernel mm_types.h line 654), file bytes are laid out in vaddr in SAME ORDER at SAME SPACING, no gaps, no reordering, single VMA = linear mapping = formula correct.
```

## RMAP CHAIN: PFN → ALL VADDRS (11 STEPS)

```
SCENARIO: Kernel wants to evict RAM page at PFN 0x123456, must find all PTEs pointing to it, clear them, then free RAM.
01. Start: PFN 0x123456 (RAM at physical 0x123456000-0x123456FFF, contains file data).
02. page = &mem_map[0x123456] → struct page at 0xFFFFEA000048D580 (vmemmap + 0x123456 × 64).
03. page→mapping = 0xFFFF888012340100 (libc's address_space, identifies file).
04. page→index = 96 (file chunk 96, file bytes 393216-397311).
05. Kernel searches ALL processes: for each task_struct in task list...
06.   for each VMA in task→mm→mmap linked list...
07.     if VMA.vm_file→f_mapping == page→mapping (0xFFFF888012340100)... ← check if VMA maps same file
08.       if vm_pgoff ≤ 96 < vm_pgoff + vma_pages... ← check if file chunk 96 is within VMA's range
09.         vaddr = vm_start + (96 - vm_pgoff) × 4096 = 0x795df3828000 + (96 - 40) × 4096 = 0x795df3860000.
10.         → Found! Process pid=12345, vaddr=0x795df3860000 maps to PFN 0x123456.
11.         Kernel walks page table at vaddr 0x795df3860000, finds PTE, clears it, now PFN 0x123456 can be freed.

∴ RMAP uses: page→mapping (which file) + page→index (which chunk) + VMA search (which processes) + formula (which vaddr).
∴ RMAP enables: page eviction, page migration, shared library updates, memory pressure handling.
```

## page→mapping POINTER DEREFERENCE CHAIN (3 HOPS TO FILE IDENTITY)

```
01. page→mapping = 0xFFFF888012340100 (just an address, means nothing alone).
02. RAM[0xFFFF888012340100] = struct address_space { host=0xFFFF888012340000, i_pages=..., ... } ← read RAM at that address.
03. address_space.host = 0xFFFF888012340000 → pointer to struct inode.
04. RAM[0xFFFF888012340000] = struct inode { i_ino=5160837, i_sb=0xFFFF888000001000, ... } ← read RAM at inode address.
05. inode.i_ino = 5160837 ← file inode number.
06. inode.i_sb = 0xFFFF888000001000 → pointer to struct super_block (filesystem).
07. RAM[0xFFFF888000001000] = struct super_block { s_type="ext4", ... }.
08. inode 5160837 + ext4 filesystem = /usr/lib/x86_64-linux-gnu/libc.so.6.
VERIFICATION: run `ls -i /usr/lib/x86_64-linux-gnu/libc.so.6` → 5160837 ✓, run `stat -f /usr/lib/...` → ext4 ✓.
∴ 0xFFFF888012340100 → 3 pointer dereferences → file name.
```

## ERROR REPORT: USER CONFUSIONS DOCUMENTED

```
ERROR 01: "file page is a guard around ram chunk" → WRONG, file chunk = 4096-byte portion of FILE, kernel's name for tracking, not RAM.
ERROR 02: "page is a guard... how can it have index" → page→index is METADATA stored in struct page, tells kernel what FILE data is in RAM.
ERROR 03: "96 is inode right, thwn why i need the & of this i mean how can you take & of a file itself" → 96 is FILE CHUNK NUMBER, not inode, mapping points to struct address_space in KERNEL RAM, not file on disk.
ERROR 04: "address space is of a process only" → KERNEL NAMING COLLISION, mm_struct = process VM, struct address_space = file page cache, different things same name.
ERROR 05: "offset in file... why page offset and 40?" → 40 = 0x28000/4096, kernel stores PAGES to avoid division, /proc shows BYTES for humans.
ERROR 06: "why read VMA.vm_start... from which process A? A Only" → Kernel clears PTEs in ALL processes sharing page, not just one, RMAP searches all.
ERROR 07: "why 40... what the hell is 40" → 40 = file chunk number at vm_start, ELF header is at chunk 0-39, .text code starts at chunk 40.
ERROR 08: "why is this linear? how can we be sure?" → VMA covers [vm_start, vm_end) = CONTIGUOUS (kernel mm_types.h line 654), linear by kernel design.

PREVENTION RULES:
01. NEVER assume "page" means RAM, "file page" = file chunk, "RAM page" = memory chunk.
02. ALWAYS check units, vm_pgoff is PAGES, /proc offset is BYTES.
03. DISTINGUISH "File on Disk" (bytes) vs "struct inode" (kernel RAM object).
04. ACCEPT overloading, "Address Space" means two different things.
05. SHARED implies MULTIPLE, RMAP finds ALL processes, not just one.
```

## KERNEL SOURCE REFERENCES

```
/usr/src/linux-source-6.8.0/include/linux/mm_types.h:
  Line 649: struct vm_area_struct { ... }
  Line 654: /* VMA covers [vm_start; vm_end) addresses within mm */ unsigned long vm_start; unsigned long vm_end;
  Line 721: unsigned long vm_pgoff; /* Offset (within vm_file) in PAGE_SIZE units */
  Line 723: struct file * vm_file; /* File we map to (can be NULL). */

/usr/src/linux-source-6.8.0/mm/filemap.c:
  Line 3248: vm_fault_t filemap_fault(struct vm_fault *vmf)
  Line 3251: struct file *file = vmf->vma->vm_file; ← VMA tells kernel WHICH file
  Line 3255: pgoff_t max_idx, index = vmf->pgoff; ← file chunk number
  Line 3267: folio = filemap_get_folio(mapping, index); ← get page from cache or allocate

/usr/src/linux-source-6.8.0/mm/memory.c:
  Line 4724: pgoff_t vma_off = vmf->pgoff - vmf->vma->vm_pgoff; ← offset within VMA in pages
```

---

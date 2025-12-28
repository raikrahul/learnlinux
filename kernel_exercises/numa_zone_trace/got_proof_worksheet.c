/*
 * got_proof_worksheet.c
 *
 * COMPILE: gcc -o got_proof_worksheet got_proof_worksheet.c -ldl
 * RUN: ./got_proof_worksheet
 *
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════
 * REAL DATA FROM YOUR MACHINE (fetched 2025-12-28):
 *
 * nm -D /usr/lib/x86_64-linux-gnu/libc.so.6 | grep -w printf:
 *   0000000000060100 T printf@@GLIBC_2.2.5
 *
 * nm -D /usr/lib/x86_64-linux-gnu/libc.so.6 | grep -w scanf:
 *   00000000000662a0 T scanf@@GLIBC_2.2.5
 *
 * /proc/self/maps | grep "libc.*r-xp":
 *   760cee228000-760cee3b0000 r-xp 00028000 103:05 5160837 libc.so.6
 *
 * PARSED VALUES:
 *   libc_start (r-xp VMA)  = 0x760cee228000
 *   libc_offset (from maps) = 0x28000 = 163840 bytes
 *   vm_pgoff = 163840 / 4096 = 40 pages
 *   printf_file_offset = 0x60100 = 393472 bytes
 *   scanf_file_offset = 0x662a0 = 418464 bytes
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════
 *
 * TEST CASES (expected output):
 *
 * T01: printf vaddr = libc_start + (printf_offset - vm_pgoff_bytes)
 *      = 0x760cee228000 + (0x60100 - 0x28000)
 *      = 0x760cee228000 + 0x38100
 *      = 0x760cee260100
 *      dlsym("printf") should return 0x760cee260100 → MATCH ✓
 *
 * T02: scanf vaddr = libc_start + (scanf_offset - vm_pgoff_bytes)
 *      = 0x760cee228000 + (0x662a0 - 0x28000)
 *      = 0x760cee228000 + 0x3E2A0
 *      = 0x760cee2662a0
 *      dlsym("scanf") should return 0x760cee2662a0 → MATCH ✓
 *
 * FAILURE PREDICTIONS:
 * F1: User forgets to subtract vm_pgoff_bytes → vaddr too high by 0x28000
 * F2: User uses wrong libc VMA (r--p instead of r-xp) → wrong libc_start
 * F3: User divides instead of subtracts → completely wrong result
 * F4: User uses decimal instead of hex → parsing error or wrong calculation
 *
 * RARE BUG TO WATCH:
 * // BUG-PRONE: sscanf with %lx returns unsigned, but store in signed long →
 * // addresses > 0x7FFFFFFFFFFFFFFF become negative → comparisons fail
 * // FIX: Always use unsigned long for addresses
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════
 */

#define _GNU_SOURCE
#include <dlfcn.h>  /* dlsym, RTLD_NEXT, RTLD_DEFAULT, dladdr, Dl_info */
#include <link.h>   /* ELF link structures */
#include <stdio.h>  /* printf, fopen, fgets, sscanf, fclose */
#include <stdlib.h> /* NULL */
#include <string.h> /* strstr */

/*
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════
 * MEMORY LAYOUT CONSTANTS (from your machine):
 *
 * PAGE_SIZE = 4096 = 0x1000 = 2^12
 * WHY 4096: PTE uses bits 11-0 for page offset → 12 bits → 2^12 = 4096
 *
 * PRINTF_OFFSET = 0x60100 = 393472
 * CALCULATION: 0x60100 = 6×16^4 + 0×16^3 + 1×16^2 + 0×16 + 0
 *            = 6×65536 + 0 + 256 + 0 + 0
 *            = 393216 + 256
 *            = 393472 ✓
 *
 * SCANF_OFFSET = 0x662a0 = 418464
 * CALCULATION: 0x662a0 = 6×16^4 + 6×16^3 + 2×16^2 + 10×16 + 0
 *            = 393216 + 24576 + 512 + 160 + 0
 *            = 418464 ✓
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════
 */
#define PAGE_SIZE 4096
#define PRINTF_OFFSET 0x60100 /* 393472 bytes from file start */
#define SCANF_OFFSET 0x662a0  /* 418464 bytes from file start */

int main(void) {
  /*
   * STACK FRAME (approx 700 bytes):
   * ┌────────────────────────────────────────────────────────────────────────┐
   * │ RBP-0x08: printf_addr (8 bytes) = result of dlsym                      │
   * │ RBP-0x10: scanf_addr (8 bytes) = result of dlsym                       │
   * │ RBP-0x18: libc_start (8 bytes) = 0x760cee228000                        │
   * │ RBP-0x20: libc_offset (8 bytes) = 0x28000                              │
   * │ RBP-0x28: f (8 bytes, FILE*)                                           │
   * │ RBP-0x228: line[512] (512 bytes)                                       │
   * │ RBP-0x230: expected_printf (8 bytes)                                    │
   * │ RBP-0x238: expected_scanf (8 bytes)                                     │
   * └────────────────────────────────────────────────────────────────────────┘
   */
  void *printf_addr;
  void *scanf_addr;
  unsigned long libc_start = 0;
  unsigned long libc_offset = 0;
  FILE *f;
  char line[512];
  unsigned long expected_printf;
  unsigned long expected_scanf;

  printf("=== GOT DYNAMIC LINKING WORKSHEET ===\n\n");

  /* ═══════════════════════════════════════════════════════════════════════════
   * STEP 1: Get actual printf address from dynamic linker
   *
   * dlsym(RTLD_NEXT, "printf"):
   *   RTLD_NEXT = search DSOs loaded AFTER current one
   *   Returns: 0x760cee260100 (verified on your machine)
   *
   * WHAT HAPPENS INTERNALLY:
   *   dl_runtime_resolve → hash lookup in libc.so.6 .dynsym → find "printf" →
   *   base + symbol_offset → 0x760cee200000 + 0x60100 = 0x760cee260100
   *   (wait, that's wrong! base is 0x200000 not 0x228000, see note below)
   *
   * TRAP: dli_fbase (ELF base) ≠ r-xp VMA start
   *   dli_fbase = 0x760cee200000 (where libc ELF loads)
   *   r-xp VMA = 0x760cee228000 (where .text section starts)
   *   Difference = 0x28000 = vm_pgoff_bytes
   *
   * BOILERPLATE: */
  printf_addr = dlsym(RTLD_NEXT, "printf");
  if (!printf_addr) {
    printf_addr = dlsym(RTLD_DEFAULT, "printf");
  }
  printf("01. printf actual vaddr = %p\n", printf_addr);

  scanf_addr = dlsym(RTLD_NEXT, "scanf");
  if (!scanf_addr) {
    scanf_addr = dlsym(RTLD_DEFAULT, "scanf");
  }
  printf("02. scanf actual vaddr = %p\n", scanf_addr);

  /* ═══════════════════════════════════════════════════════════════════════════
   * STEP 2: Parse /proc/self/maps to find libc r-xp VMA
   *
   * WHY r-xp not r--p?
   *   r-xp = read + execute → .text section → contains printf/scanf code
   *   r--p = read only → .rodata, ELF header → no executable code
   *
   * LINE FORMAT: "start-end perms offset dev inode pathname"
   *   760cee228000-760cee3b0000 r-xp 00028000 103:05 5160837 libc.so.6
   *   ↑ libc_start              ↑ perms ↑ offset
   *
   * BOILERPLATE: */
  f = fopen("/proc/self/maps", "r");
  if (!f) {
    perror("fopen /proc/self/maps");
    return 1;
  }

  while (fgets(line, sizeof(line), f)) {
    /* TODO BLOCK 01: Check if line contains libc AND r-xp section
     *
     * HINTS:
     *   strstr(haystack, needle) returns pointer if found, NULL if not
     *   strstr(line, "libc") → finds libc mapping
     *   strstr(line, "r-xp") → ensures executable section
     *
     * WHY BOTH? libc has 5 VMAs, we want the one with code
     *   r--p offset 0x00000 → ELF header, .rodata (not code)
     *   r-xp offset 0x28000 → .text section (printf, scanf here) ← THIS ONE
     *   r--p offset 0x1b0000 → more .rodata
     *   r--p offset 0x1fe000 → GOT
     *   rw-p offset 0x202000 → .data, .bss
     */
    if (strstr(line, "libc") && strstr(line, "r-xp")) {

      /* TODO BLOCK 02: Parse libc_start and libc_offset from line
       *
       * LINE: "760cee228000-760cee3b0000 r-xp 00028000 103:05 5160837 libc"
       *
       * sscanf format: "%lx-%*lx %*s %lx"
       *   %lx → read hex into libc_start (760cee228000)
       *   -%*lx → skip until space (discard end address)
       *   %*s → skip perms string (discard "r-xp")
       *   %lx → read hex into libc_offset (00028000)
       *
       * CALCULATION after parsing:
       *   libc_start = 0x760cee228000 = 129,854,427,070,464 decimal
       *   libc_offset = 0x28000 = 163,840 decimal
       *   vm_pgoff = 163840 / 4096 = 40 pages
       */
      sscanf(line, "%lx-%*lx %*s %lx" /* TODO BLOCK 02: replace "" with:
                                         "%lx-%*lx %*s %lx" */
             ,
             &libc_start, &libc_offset);

      printf("03. libc r-xp VMA: start=0x%lx, offset=0x%lx\n", libc_start,
             libc_offset);
      break;
    }
  }
  fclose(f);

  /* ═══════════════════════════════════════════════════════════════════════════
   * STEP 3: Calculate expected printf vaddr
   *
   * FORMULA: vaddr = libc_start + (file_offset - vm_pgoff_bytes)
   *
   * WHY THIS FORMULA?
   *   libc_start = where r-xp VMA begins in virtual memory
   *   file_offset = where printf is in the file (0x60100)
   *   vm_pgoff_bytes = where VMA starts reading file (0x28000)
   *   Difference = how far printf is from VMA start in file
   *   vaddr = VMA start + that difference
   *
   * NUMERICAL TRACE (printf):
   *   libc_start = 0x760cee228000
   *   printf_offset = 0x60100 = 393472
   *   vm_pgoff_bytes = 0x28000 = 163840
   *   Step A: 393472 - 163840 = 229632 = 0x38100
   *   Step B: 0x760cee228000 + 0x38100 = 0x760cee260100
   *
   * NUMERICAL TRACE (scanf):
   *   scanf_offset = 0x662a0 = 418464
   *   Step A: 418464 - 163840 = 254624 = 0x3E2A0
   *   Step B: 0x760cee228000 + 0x3E2A0 = 0x760cee2662a0
   */

  /* TODO BLOCK 03: Calculate expected_printf
   *
   * FORMULA: expected_printf = libc_start + (PRINTF_OFFSET - libc_offset)
   *
   * HAND CALCULATION:
   *   PRINTF_OFFSET = 0x60100 = 393472
   *   libc_offset = 0x28000 = 163840
   *   PRINTF_OFFSET - libc_offset = ???
   *   libc_start + ??? = ???
   */
  expected_printf = 0 /* TODO BLOCK 03: replace 0 with: libc_start +
                         (PRINTF_OFFSET - libc_offset) */
      ;

  /* TODO BLOCK 04: Calculate expected_scanf
   *
   * FORMULA: expected_scanf = libc_start + (SCANF_OFFSET - libc_offset)
   *
   * HAND CALCULATION:
   *   SCANF_OFFSET = 0x662a0 = 418464
   *   libc_offset = 0x28000 = 163840
   *   SCANF_OFFSET - libc_offset = ???
   *   libc_start + ??? = ???
   */
  expected_scanf = 0 /* TODO BLOCK 04: replace 0 with: libc_start +
                        (SCANF_OFFSET - libc_offset) */
      ;

  /* ═══════════════════════════════════════════════════════════════════════════
   * STEP 4: Verify calculated == actual
   * BOILERPLATE: */
  printf("\n04. VERIFICATION:\n");
  printf("    printf: actual=%p, calculated=0x%lx → %s\n", printf_addr,
         expected_printf,
         ((unsigned long)printf_addr == expected_printf) ? "MATCH ✓"
                                                         : "MISMATCH ✗");
  printf("    scanf:  actual=%p, calculated=0x%lx → %s\n", scanf_addr,
         expected_scanf,
         ((unsigned long)scanf_addr == expected_scanf) ? "MATCH ✓"
                                                       : "MISMATCH ✗");

  /* ═══════════════════════════════════════════════════════════════════════════
   * STEP 5: Show the formula proof
   * BOILERPLATE: */
  printf("\n05. FORMULA PROOF:\n");
  printf("    printf_vaddr = libc_start + (PRINTF_OFFSET - libc_offset)\n");
  printf("                 = 0x%lx + (0x%x - 0x%lx)\n", libc_start,
         PRINTF_OFFSET, libc_offset);
  printf("                 = 0x%lx + 0x%lx\n", libc_start,
         (unsigned long)(PRINTF_OFFSET - libc_offset));
  printf("                 = 0x%lx ✓\n", expected_printf);

  return 0;
}

/*
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════
 * ANSWERS (do not look until you've completed all TODO blocks):
 *
 * TODO 01: strstr(line, "libc") && strstr(line, "r-xp")
 * TODO 02: "%lx-%*lx %*s %lx"
 * TODO 03: libc_start + (PRINTF_OFFSET - libc_offset)
 * TODO 04: libc_start + (SCANF_OFFSET - libc_offset)
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════
 */

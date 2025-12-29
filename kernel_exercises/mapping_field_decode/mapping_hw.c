/*
 * mapping_hw.c
 * KERNEL INSPECTOR: Reads page->mapping and decodes bottom 2 bits.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/page-flags.h>
#include <linux/pid.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Primate");
MODULE_DESCRIPTION("Decodes PAGE_MAPPING_FLAGS from page->mapping pointer");

static int target_pid = 0;
module_param(target_pid, int, 0);

static unsigned long target_va = 0;
module_param(target_va, ulong, 0);

static struct page *get_user_page(int pid_nr, unsigned long addr) {
  struct pid *pid_struct;
  struct task_struct *task;
  struct mm_struct *mm;
  struct page *page;
  int ret;

  pid_struct = find_get_pid(pid_nr);
  if (!pid_struct)
    return NULL;

  task = pid_task(pid_struct, PIDTYPE_PID);
  if (!task) {
    put_pid(pid_struct);
    return NULL;
  }

  mm = task->mm;
  if (!mm) {
    put_pid(pid_struct);
    return NULL;
  }

  ret = get_user_pages_remote(mm, addr, 1, 0, &page, NULL);
  put_pid(pid_struct);

  if (ret < 0)
    return NULL;
  return page;
}

static void inspect_mapping(struct page *page) {
  unsigned long raw_mapping;
  unsigned long flags;
  unsigned long clean_ptr;

  /*
   * TODO 1: READ RAW MAPPING VALUE
   * ------------------------------
   * AXIOM: page->mapping is a pointer (64-bit on x86_64).
   * PROBLEM: We cannot use bitwise & on a pointer type in C.
   * WORK: Cast page->mapping to unsigned long.
   *
   * CALCULATION TRACE:
   * 1. page = 0xfffff7da4cc48000
   * 2. offsetof(struct page, mapping) = 24 (Bytes)
   * 3. Address read = 0xfffff7da4cc48000 + 24 = 0xfffff7da4cc48018
   * 4. Value at Address (Example): 0xffff888123456781
   *    - Upper 62 bits: Real Pointer Data (0xffff888123456780)
   *    - Lower 2 bits: 01 (PAGE_MAPPING_ANON)
   * 5. Cast to ulong preserves all bits.
   */
  raw_mapping = (unsigned long)page->mapping;

  /*
   * TODO 2: EXTRACT FLAGS (LOWER 2 BITS)
   * ------------------------------------
   * AXIOM: PAGE_MAPPING_FLAGS mask covers bits 0 and 1.
   * AXIOM: Mask value = 0x3 (Binary 11).
   * WORK: Perform bitwise AND with 0x3.
   *
   * CALCULATION TRACE:
   * 1. raw_mapping = 0xffff888123456781 (bits...1000001)
   * 2. MASK        = 0x0000000000000003 (bits...0000011)
   * 3. AND Operation:
   *      ...1000001
   *    & ...0000011
   *    ------------
   *      ...0000001 (Result = 1 = PAGE_MAPPING_ANON)
   */
  flags = raw_mapping & 0x3UL;

  /*
   * TODO 3: EXTRACT CLEAN POINTER
   * -----------------------------
   * AXIOM: To get the real address, we must zero out limits [1:0].
   * AXIOM: ~0x3 = ~...000011 = ...111100 (Mask to keep upper bits).
   * WORK: Perform bitwise AND with ~0x3UL.
   *
   * CALCULATION TRACE:
   * 1. raw_mapping = 0xffff888123456781
   * 2. MASK (~3)  = 0xfffffffffffffffc (ends in ...1100)
   * 3. AND Operation:
   *      ...00001 (0x...1)
   *    & ...11100 (0x...C)
   *    ----------
   *      ...00000 (0x...0) -> 0xffff888123456780
   * 4. Result: Valid, 8-byte aligned pointer to anon_vma struct.
   */
  clean_ptr = raw_mapping & ~0x3UL;

  printk(KERN_INFO "MAPPING_DECODE: Page Struct   = %px\n", page);
  printk(KERN_INFO "MAPPING_DECODE: Raw mapping   = 0x%lx\n", raw_mapping);
  printk(KERN_INFO "MAPPING_DECODE: Flags [1:0]   = %lu (Binary: %lu%lu)\n",
         flags, (flags >> 1) & 1, flags & 1);

  printk(KERN_INFO "MAPPING_DECODE: Clean Pointer = 0x%lx\n", clean_ptr);

  /*
   * TODO 4: INTERPRET THE STATE (USER EXERCISE)
   * -------------------------------------------
   * AXIOM: We have 2 bits of flags [1:0]. Max value = 3.
   *
   * TASK: Implement the switch/case logic to identify the state.
   *
   * MATH TABLE (Complete this logic):
   * 1. If bits == 0 (00) -> Pointer is Aligned (8-byte).
   *    Type: struct address_space * (Page Cache).
   *
   * 2. If bits == 1 (01) -> PAGE_MAPPING_ANON (0x1).
   *    Type: struct anon_vma *.
   *    Note: "However the lower 2 bits are given over..." (Your Quote).
   *
   * 3. If bits == 2 (10) -> PAGE_MAPPING_MOVABLE (0x2).
   *    Type: struct movable_operations *.
   *    Reason: "Movable" pages (e.g. zram/balloon).
   *
   * 4. If bits == 3 (11) -> PAGE_MAPPING_KSM (0x3).
   *    Type: struct ksm_stable_node *.
   *    Math: 0x1 | 0x2 = 0x3. "Combination of prior two".
   *
   * INSTRUCTION: Use printk to report the state and the clean pointer.
   * WARNING: Do NOT dereference KSM/Movable pointers (incomplete types).
   */

  // WRITE YOUR CODE HERE:
  if (flags == 0) {
    printk(KERN_INFO "MAPPING_DECODE: Page is aligned (8-byte).\n");
    printk(KERN_INFO "MAPPING_DECODE: Clean Pointer = 0x%lx\n", clean_ptr);
  } else if (flags == 1) {
    printk(KERN_INFO "MAPPING_DECODE: Page is anonymous.\n");
    printk(KERN_INFO "MAPPING_DECODE: Clean Pointer = 0x%lx\n", clean_ptr);
  } else if (flags == 2) {
    printk(KERN_INFO "MAPPING_DECODE: Page is movable.\n");
    printk(KERN_INFO "MAPPING_DECODE: Clean Pointer = 0x%lx\n", clean_ptr);
  } else if (flags == 3) {
    printk(KERN_INFO "MAPPING_DECODE: Page is ksm.\n");
    printk(KERN_INFO "MAPPING_DECODE: Clean Pointer = 0x%lx\n", clean_ptr);
  }
}

static int __init mapping_init(void) {
  struct page *page;

  if (target_pid == 0 || target_va == 0) {
    printk(KERN_ERR "MAPPING_DECODE: Usage: insmod mapping_hw.ko target_pid=X "
                    "target_va=Y\n");
    return -EINVAL;
  }

  printk(KERN_INFO "MAPPING_DECODE: Init. PID=%d VA=0x%lx\n", target_pid,
         target_va);

  page = get_user_page(target_pid, target_va);
  if (!page) {
    printk(KERN_ERR "MAPPING_DECODE: Failed to get page.\n");
    return -EINVAL;
  }

  inspect_mapping(page);
  put_page(page);

  return 0;
}

static void __exit mapping_exit(void) {
  printk(KERN_INFO "MAPPING_DECODE: Exit.\n");
}

module_init(mapping_init);
module_exit(mapping_exit);

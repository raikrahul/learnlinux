/*
 * bootmem_trace.c - Kernel Memory Management Trace Driver
 * AXIOM SOURCES: Each comment traces back to worksheet.md line or verifiable command.
 */

/* A00. #include directives: standard kernel module headers. Source: kernel module documentation. */
#include <linux/module.h>    /* MODULE_LICENSE, module_init, module_exit */
#include <linux/kernel.h>    /* printk, KERN_INFO */
#include <linux/init.h>      /* __init, __exit macros */
#include <linux/mm.h>        /* page_to_pfn, get_page, put_page */
#include <linux/gfp.h>       /* GFP_KERNEL, alloc_page */
#include <linux/mm_types.h>  /* struct page */
#include <linux/page_ref.h>  /* page_ref_count */

/* A01. MODULE_LICENSE("GPL"): Required for accessing GPL-only kernel symbols. Source: kernel module documentation. */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("User");
MODULE_DESCRIPTION("Bootmem Trace Driver");

static int __init bootmem_init(void)
{
    /* A02. Variable declarations. No values yet. */
    struct page *page_ptr;   /* Will hold return value of alloc_page. Type: pointer to struct page. */
    unsigned long pfn;       /* Will hold page frame number. Type: unsigned long. */
    phys_addr_t phys;        /* Will hold physical address. Type: phys_addr_t (unsigned long long). */
    int ref;                 /* Will hold reference count. Type: int. */
    
    printk(KERN_INFO "BOOTMEM_TRACE: __init bootmem_init called\n");
    
    /* 
     * A03. alloc_page(GFP_KERNEL)
     * GFP_KERNEL source: worksheet.md line 17-21 → grep GFP_KERNEL → 0xCC0
     * 0xCC0 derivation: __GFP_RECLAIM=0x400, __GFP_IO=0x40, __GFP_FS=0x80 → 0x400|0x40|0x80=0xCC0 (worksheet.md line 75)
     * alloc_page expands to: alloc_pages(GFP_KERNEL, 0) → __alloc_pages(0xCC0, order=0, numa_node=-1)
     * buddy allocator searches zone Normal (most likely on 16GB RAM) → finds order-0 free block → returns struct page*
     * _refcount set to 1: source worksheet.md line 22 → grep set_page_refcounted → atomic_set(&page->_refcount, 1)
     * page_ptr value: UNKNOWN until runtime. Must read from dmesg after insmod.
     */
    page_ptr = alloc_page(GFP_KERNEL);
    
    /*
     * A04. Check alloc_page return value.
     * alloc_page returns NULL on failure.
     * If NULL, cannot proceed. Must check before dereferencing.
     */
    if (!page_ptr) {
        printk(KERN_ERR "BOOTMEM_TRACE: alloc_page failed\n");
        return -ENOMEM;
    }
    
    /*
     * A05. page_to_pfn(page_ptr)
     * Formula source: worksheet.md line 23 → F01: pfn=(page-vmemmap)/sizeof(struct page)
     * vmemmap source: worksheet.md line 16 → grep VMEMMAP_START → 0xffffea0000000000
     * sizeof(struct page) source: worksheet.md line 05 → pahole → 64 bytes
     * CALCULATION EXAMPLE (assuming page_ptr=0xffffea0007a21f40 from worksheet.md line 32):
     *   pfn = (0xffffea0007a21f40 - 0xffffea0000000000) / 64
     *       = 0x7a21f40 / 64
     *       = 128065344 / 64
     *       = 2001021
     * ACTUAL pfn: printed below, verify against hand calculation after dmesg.
     */
    pfn = page_to_pfn(page_ptr);
    
    /*
     * A06. phys = pfn * PAGE_SIZE
     * PAGE_SIZE source: worksheet.md line 03 → getconf PAGE_SIZE → 4096
     * Formula source: worksheet.md line 24 → F02: phys=pfn×4096
     * CALCULATION EXAMPLE (assuming pfn=2001021 from A05):
     *   phys = 2001021 × 4096
     *   Step 1: 2001021 × 4000 = 8004084000
     *   Step 2: 2001021 × 96 = 192098016
     *   Step 3: 8004084000 + 192098016 = 8196182016
     *   phys = 8196182016 = 0x1e887d000
     * ACTUAL phys: printed below, verify against hand calculation after dmesg.
     */
    phys = (phys_addr_t)pfn * PAGE_SIZE;
    
    /*
     * A07. page_ref_count(page_ptr)
     * Source: worksheet.md line 22 → A14 → alloc_page sets _refcount=1
     * page_ref_count reads page->_refcount.counter via atomic_read
     * Expected value: 1
     * ACTUAL ref: printed below, verify equals 1 after dmesg.
     */
    ref = page_ref_count(page_ptr);
    
    /*
     * A08. Zone determination
     * Rules source: worksheet.md lines 11, 14, 15
     * R01: PFN < 4096 → zone=DMA (4096 from worksheet.md line 10: 16MB/4096=4096)
     * R02: 4096 ≤ PFN < 1048576 → zone=DMA32 (1048576 from worksheet.md line 13: 4GB/4096=1048576)
     * R03: PFN ≥ 1048576 → zone=Normal
     * CALCULATION EXAMPLE (assuming pfn=2001021):
     *   2001021 < 4096? → ✗ false → not DMA
     *   2001021 < 1048576? → ✗ false → not DMA32
     *   2001021 ≥ 1048576? → ✓ true → zone=Normal
     * ACTUAL zone: determined by ternary operator below, verify after dmesg.
     */
    printk(KERN_INFO "BOOTMEM_TRACE: page_ptr=%px pfn=0x%lx(%lu) phys=0x%llx ref=%d zone=%s\n",
           page_ptr, pfn, pfn, phys, ref,
           pfn >= 1048576 ? "Normal" : (pfn >= 4096 ? "DMA32" : "DMA"));
    
    /*
     * A09. get_page(page_ptr)
     * Source: worksheet.md line 77 → grep get_page → page_ref_inc → atomic_inc(&page->_refcount)
     * _refcount before: 1 (from A07)
     * Operation: 1 + 1 = 2
     * _refcount after: 2
     */
    printk(KERN_INFO "BOOTMEM_TRACE: Before get_page: ref=%d\n", page_ref_count(page_ptr));
    get_page(page_ptr);
    printk(KERN_INFO "BOOTMEM_TRACE: After get_page: ref=%d (expected 2)\n", page_ref_count(page_ptr));
    
    /*
     * A10. put_page(page_ptr) - FIRST
     * Source: worksheet.md line 78 → grep put_page → page_ref_dec_and_test → atomic_dec_and_test(&page->_refcount)
     * _refcount before: 2 (from A09)
     * Operation: 2 - 1 = 1
     * Test: 1 == 0? → ✗ false → page NOT freed
     * _refcount after: 1
     */
    printk(KERN_INFO "BOOTMEM_TRACE: Before put_page #1: ref=%d\n", page_ref_count(page_ptr));
    put_page(page_ptr);
    printk(KERN_INFO "BOOTMEM_TRACE: After put_page #1: ref=%d (expected 1)\n", page_ref_count(page_ptr));
    
    /*
     * A11. put_page(page_ptr) - SECOND
     * _refcount before: 1 (from A10)
     * Operation: 1 - 1 = 0
     * Test: 0 == 0? → ✓ true → __free_pages_ok(page, 0) called → page returned to buddy allocator
     * _refcount after: 0 (page freed, struct page may be reused)
     * WARNING: After this call, page_ptr is DANGLING POINTER. Accessing page_ptr->_refcount is UNDEFINED BEHAVIOR.
     */
    printk(KERN_INFO "BOOTMEM_TRACE: Before put_page #2: ref=%d\n", page_ref_count(page_ptr));
    put_page(page_ptr);
    /* DANGER: page_ptr now points to freed memory. Cannot safely read page_ref_count. */
    printk(KERN_INFO "BOOTMEM_TRACE: After put_page #2: page freed (ref reading is UB)\n");
    
    /*
     * A12. put_page(page_ptr) - THIRD (BUG)
     * _refcount before: 0 (from A11) OR page reused by kernel with _refcount=1
     * Operation: 0 - 1 = -1 OR 1 - 1 = 0 (wrong page!)
     * Scenario 1 (page not reused): _refcount=-1 → VM_BUG_ON_PAGE triggered
     * Scenario 2 (page reused): We decrement SOMEONE ELSE'S refcount → DATA CORRUPTION
     * Source: worksheet.md line 79 → grep VM_BUG_ON_PAGE → include/linux/mmdebug.h
     * Expected outcome: Kernel WARN or BUG in dmesg. Possible system instability.
     * UNCOMMENT BELOW LINE TO TRIGGER BUG:
     */
    /* put_page(page_ptr); */
    printk(KERN_INFO "BOOTMEM_TRACE: BUG LINE COMMENTED. Uncomment line 147 to trigger refcount underflow.\n");
    
    printk(KERN_INFO "BOOTMEM_TRACE: Init complete. Check values against worksheet.md calculations.\n");
    return 0;
}

static void __exit bootmem_exit(void)
{
    printk(KERN_INFO "BOOTMEM_TRACE: __exit bootmem_exit called\n");
}

module_init(bootmem_init);
module_exit(bootmem_exit);

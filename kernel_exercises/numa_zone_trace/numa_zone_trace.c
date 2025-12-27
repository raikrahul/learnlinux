/*numa_zone_trace.c:DRAW[NUMA:NonUniformMemoryAccess→node0_cpus[0-11]→node0_size=15406MB→node0_distance=10(local)]→DRAW[zones_per_node:DMA(0-16MB)|DMA32(16MB-4GB)|Normal(4GB+)→zones_overlap_nodes]→DRAW[pageblock_order=9→pageblock_size=2^9=512pages=2MB→migratetype_per_pageblock]→DRAW[struct_folio:represents_1_or_more_pages→folio_nr_pages()→compound_pages]→DRAW[PFN=phys_addr/PAGE_SIZE→PHYS_PFN(addr)=addr>>12→PFN_PHYS(pfn)=pfn<<12]*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/mmzone.h>
#include <linux/numa.h>
#include <linux/nodemask.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("User");
MODULE_DESCRIPTION("NUMA Zone Trace Demo");
static int __init numa_zone_init(void)
{
    /*DRAW[stack:numa_zone_init()→local_vars:page@rbp-8|pfn@rbp-16|nid@rbp-20|zone_type@rbp-24]*/
    struct page *page;
    unsigned long pfn;
    int nid;
    struct zone *zone;
    printk(KERN_INFO "NUMA_ZONE:__init\n");
    /*DRAW[AXIOM:num_online_nodes()→returns_count_of_online_NUMA_nodes→this_machine=1_node(from_numactl)]*/
    printk(KERN_INFO "NUMA_ZONE:num_online_nodes=%d\n", num_online_nodes());
    /*DRAW[for_each_online_node(nid):iterates_nid=0,1,2...→this_machine:nid=0_only]*/
    for_each_online_node(nid) {
        /*DRAW[NODE_DATA(nid):returns_pg_data_t*→node_start_pfn=first_PFN_in_node→node_spanned_pages=total_PFNs→node_present_pages=actual_usable_PFNs]*/
        pg_data_t *pgdat = NODE_DATA(nid);
        printk(KERN_INFO "NUMA_ZONE:node[%d]:start_pfn=%lu,spanned_pages=%lu,present_pages=%lu\n",
               nid, pgdat->node_start_pfn, pgdat->node_spanned_pages, pgdat->node_present_pages);
        /*DRAW[node_zones[]:array_of_struct_zone→ZONE_DMA(0)|ZONE_DMA32(1)|ZONE_NORMAL(2)|ZONE_MOVABLE(3)]*/
        /*TODO_USER:CALCULATE:zone_start_pfn×4096=zone_start_phys_addr*/
    }
    /*DRAW[alloc_page:allocates_from_current_node→page_to_nid(page)=node_id_owning_page]*/
    page = alloc_page(GFP_KERNEL);
    if (!page) {
        printk(KERN_ERR "NUMA_ZONE:alloc_failed\n");
        return -ENOMEM;
    }
    pfn = page_to_pfn(page);
    nid = page_to_nid(page);
    zone = page_zone(page);
    /*DRAW[page_zone(page):returns_struct_zone*→zone->name="Normal"_or_"DMA32"_or_"DMA"]*/
    printk(KERN_INFO "NUMA_ZONE:page:pfn=0x%lx(%lu),node=%d,zone=%s\n", 
           pfn, pfn, nid, zone->name);
    /*DRAW[zone_boundaries:zone->zone_start_pfn=first_PFN→zone->spanned_pages=total→zone->present_pages=usable]*/
    printk(KERN_INFO "NUMA_ZONE:%s:start_pfn=%lu,spanned=%lu,present=%lu\n",
           zone->name, zone->zone_start_pfn, zone->spanned_pages, zone->present_pages);
    /*DRAW[PFN_PHYS:pfn×4096=phys_addr→verify:pfn<<12=pfn×4096]*/
    /*TODO_USER:CALCULATE:pfn×4096=?→compare_with_PFN_PHYS(pfn)*/
    printk(KERN_INFO "NUMA_ZONE:phys_addr=pfn×4096=0x%lx×0x1000=0x%llx\n", 
           pfn, (unsigned long long)pfn * PAGE_SIZE);
    /*DRAW[PHYS_PFN:phys_addr>>12=pfn→verify:(pfn×4096)>>12=pfn]*/
    printk(KERN_INFO "NUMA_ZONE:verify:pfn=%lu,phys>>12=%llu\n", 
           pfn, ((unsigned long long)pfn * PAGE_SIZE) >> 12);
    /*DRAW[pageblock_order:grep_CONFIG_PAGEBLOCK_ORDER→x86_64=9→pageblock_size=2^9=512_pages=512×4096=2097152=2MB]*/
    /*TODO_USER:CALCULATE:pageblock_order=9→2^9=___pages→___×4096=___bytes=___MB*/
    printk(KERN_INFO "NUMA_ZONE:pageblock_order=%d,pageblock_nr_pages=%lu\n", 
           pageblock_order, pageblock_nr_pages);
    /*DRAW[zone_spans_pfn_check:pfn≥zone_start_pfn?→pfn<zone_end_pfn?→both_true=page_in_zone]*/
    printk(KERN_INFO "NUMA_ZONE:zone_check:pfn=%lu≥start=%lu?%s,pfn<%lu?%s\n",
           pfn, zone->zone_start_pfn, pfn >= zone->zone_start_pfn ? "✓" : "✗",
           zone->zone_start_pfn + zone->spanned_pages,
           pfn < zone->zone_start_pfn + zone->spanned_pages ? "✓" : "✗");
    put_page(page);
    printk(KERN_INFO "NUMA_ZONE:init_complete\n");
    return 0;
}
static void __exit numa_zone_exit(void)
{
    printk(KERN_INFO "NUMA_ZONE:exit\n");
}
module_init(numa_zone_init);
module_exit(numa_zone_exit);

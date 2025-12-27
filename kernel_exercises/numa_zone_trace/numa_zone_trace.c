/*numa_zone_trace.c:LINE01:AXIOM:RAM=16154906624bytes(from_worksheet_line05)→LINE02:PAGE_SIZE=4096bytes(worksheet_line06)→LINE03:Total_pages=16154906624÷4096=3944069(worksheet_line09)→LINE04:NUMA_nodes=1(worksheet_line10)→LINE05:This_file_traces_node_and_zone_data_structures_using_real_values_from_this_machine*/
#include <linux/module.h>  /*LINE06:module.h:provides_MODULE_LICENSE,MODULE_AUTHOR,module_init,module_exit→included_first_because_all_kernel_modules_need_it*/
#include <linux/kernel.h>  /*LINE07:kernel.h:provides_printk→printk(KERN_INFO"msg")→writes_to_kernel_ring_buffer→visible_via_dmesg*/
#include <linux/init.h>    /*LINE08:init.h:provides___init,__exit→__init=code_discarded_after_boot→saves_memory*/
#include <linux/mm.h>      /*LINE09:mm.h:provides_page_to_pfn,page_zone,page_to_nid→memory_management_functions*/
#include <linux/gfp.h>     /*LINE10:gfp.h:provides_alloc_page,GFP_KERNEL→page_allocation_API*/
#include <linux/mmzone.h>  /*LINE11:mmzone.h:provides_pg_data_t,struct_zone,for_each_online_node,NODE_DATA→zone_and_node_data_structures*/
#include <linux/nodemask.h>/*LINE12:nodemask.h:provides_for_each_online_node→macro_iterating_NUMA_nodes→expands_to_for_each_node_state(node,N_ONLINE)*/
MODULE_LICENSE("GPL");     /*LINE13:GPL_required_for_kernel_symbol_access→non-GPL_modules_cannot_use_EXPORT_SYMBOL_GPL_functions*/
MODULE_AUTHOR("User");     /*LINE14:author_metadata→visible_in_modinfo_output*/
MODULE_DESCRIPTION("NUMA Zone Trace"); /*LINE15:description_metadata*/
/*LINE16:PROBLEM:User_must_trace_real_node_and_zone_values_by_hand_before_running_module→Use_worksheet_lines_01-70_as_reference*/
static int __init numa_zone_init(void)
{
    /*LINE17:STACK_LAYOUT:function_prologue→push_rbp→mov_rsp,rbp→sub_rsp,N→local_variables_on_stack*/
    /*LINE18:DRAW_STACK[rbp-8:page(8bytes)=???|rbp-16:pfn(8bytes)=???|rbp-20:nid(4bytes)=???|rbp-24:zone_ptr(8bytes)=???]*/
    struct page *page;     /*LINE19:page=pointer_to_struct_page(64bytes)→TODO_USER:CALCULATE:sizeof(struct_page)=___bytes*/
    unsigned long pfn;     /*LINE20:pfn=unsigned_long=8bytes_on_x86_64→stores_Page_Frame_Number*/
    int nid;               /*LINE21:nid=node_id=int=4bytes→this_machine:nid=0_only(from_worksheet_line10)*/
    struct zone *zone;     /*LINE22:zone=pointer_to_struct_zone→contains_zone_start_pfn,spanned_pages,name*/
    printk(KERN_INFO "NUMA_ZONE:init_start\n"); /*LINE23:printk→kernel_printf→KERN_INFO="6"→priority_level*/
    /*LINE24:TODO_USER:Before_proceeding→run:`numactl --hardware`→verify_output_matches_worksheet_line10,18,19*/
    /*LINE25:num_online_nodes():TRACE→reads_node_states[N_ONLINE]_bitmask→counts_set_bits→returns_count*/
    /*LINE26:TODO_USER:PREDICT:num_online_nodes()=___→based_on_worksheet_line10*/
    printk(KERN_INFO "NUMA_ZONE:num_online_nodes=%d\n", num_online_nodes());
    /*LINE27:for_each_online_node(nid):EXPANSION→for_each_node_state(nid,N_ONLINE)→for_each_node_mask(nid,node_states[N_ONLINE])→iterates_bits*/
    /*LINE28:TRACE:node_states[N_ONLINE]=bitmask→this_machine:0b0001(1_node)→loop_body_executes_once_with_nid=0*/
    for_each_online_node(nid) {
        /*LINE29:NODE_DATA(nid):TRACE→returns_pg_data_t*→on_UMA_system:returns_&contig_page_data→on_NUMA:returns_node_data[nid]*/
        pg_data_t *pgdat = NODE_DATA(nid); /*LINE30:pgdat=pointer_to_node_descriptor→contains_node_start_pfn,node_spanned_pages*/
        /*LINE31:TODO_USER:PREDICT→pgdat->node_start_pfn=___→first_PFN_in_node→usually_0_or_1*/
        /*LINE32:TODO_USER:PREDICT→pgdat->node_spanned_pages≈___→from_worksheet_line22:3944424_or_similar*/
        /*LINE33:TRAP:node_spanned_pages_includes_holes→node_present_pages_excludes_holes→spanned≥present*/
        printk(KERN_INFO "NUMA_ZONE:node[%d]:start_pfn=%lu,spanned=%lu,present=%lu\n",
               nid, pgdat->node_start_pfn, pgdat->node_spanned_pages, pgdat->node_present_pages);
        /*LINE34:TODO_USER:After_insmod→check_dmesg→compare_values_with_worksheet_predictions*/
    }
    /*LINE35:alloc_page(GFP_KERNEL):TRACE→alloc_page=macro→alloc_pages(gfp,0)→order=0=1_page*/
    /*LINE36:GFP_KERNEL=0xCC0(from_bootmem_trace_worksheet)→can_sleep,can_do_IO,can_use_FS*/
    /*LINE37:TODO_USER:CALCULATE:GFP_KERNEL=__GFP_RECLAIM|__GFP_IO|__GFP_FS=0x___*/
    page = alloc_page(GFP_KERNEL);
    if (!page) { /*LINE38:NULL_check:alloc_page_returns_NULL_on_failure→happens_when_no_free_pages*/
        printk(KERN_ERR "NUMA_ZONE:alloc_failed\n");
        return -ENOMEM; /*LINE39:-ENOMEM=-12→defined_in_include/uapi/asm-generic/errno-base.h*/
    }
    /*LINE40:page_to_pfn(page):TRACE→(page-vmemmap)/sizeof(struct_page)→vmemmap=0xffffXXXX(architecture_dependent)*/
    /*LINE41:TODO_USER:CALCULATE:if_page=0xfffff895054a1480_and_vmemmap=0xfffff89500000000→pfn=(0xfffff895054a1480-0xfffff89500000000)/64=___*/
    pfn = page_to_pfn(page);
    /*LINE42:page_to_nid(page):TRACE→reads_page->flags→extracts_node_bits→returns_node_id*/
    /*LINE43:TODO_USER:PREDICT:page_to_nid(page)=___→this_machine_has_1_node→nid=___*/
    nid = page_to_nid(page);
    /*LINE44:page_zone(page):TRACE→reads_page->flags→extracts_zone_bits→returns_struct_zone**/
    zone = page_zone(page);
    /*LINE45:zone->name:string_identifying_zone→"DMA"_or_"DMA32"_or_"Normal"*/
    /*LINE46:TODO_USER:PREDICT:zone->name="___"→based_on_pfn_value_and_worksheet_line46*/
    printk(KERN_INFO "NUMA_ZONE:page:pfn=0x%lx(%lu),node=%d,zone=%s\n", pfn, pfn, nid, zone->name);
    /*LINE47:zone->zone_start_pfn:first_PFN_in_this_zone→from_worksheet:DMA=0,DMA32=4096,Normal=1048576*/
    /*LINE48:TODO_USER:Based_on_zone->name→predict_zone->zone_start_pfn=___*/
    printk(KERN_INFO "NUMA_ZONE:%s:start_pfn=%lu,spanned=%lu,present=%lu\n",
           zone->name, zone->zone_start_pfn, zone->spanned_pages, zone->present_pages);
    /*LINE49:pageblock_order:TRACE→defined_at_compile_time→x86_64=9→pageblock_nr_pages=2^9=512*/
    /*LINE50:TODO_USER:VERIFY→grep_CONFIG_PAGEBLOCK_ORDER_in_kernel_config_or_accept_default=9*/
    printk(KERN_INFO "NUMA_ZONE:pageblock_order=%d,pageblock_nr_pages=%lu\n", pageblock_order, pageblock_nr_pages);
    /*LINE51:PFN_to_physical:phys=pfn×4096=pfn<<12→from_worksheet_line66-67*/
    /*LINE52:TODO_USER:CALCULATE→pfn_value_from_dmesg×4096=0x___→verify_matches_output*/
    printk(KERN_INFO "NUMA_ZONE:phys=pfn*4096=0x%lx*0x1000=0x%llx\n", pfn, (unsigned long long)pfn * PAGE_SIZE);
    /*LINE53:ZONE_MEMBERSHIP_CHECK:pfn>=zone_start_pfn?→pfn<zone_end_pfn?→from_worksheet_line46*/
    /*LINE54:TODO_USER:FILL_CALCULATION→pfn=___,zone_start=___,zone_end=___→is_pfn_in_range?*/
    printk(KERN_INFO "NUMA_ZONE:zone_check:pfn=%lu>=start=%lu?%s,pfn<%lu?%s\n",
           pfn, zone->zone_start_pfn, pfn >= zone->zone_start_pfn ? "Y" : "N",
           zone->zone_start_pfn + zone->spanned_pages,
           pfn < zone->zone_start_pfn + zone->spanned_pages ? "Y" : "N");
    /*LINE55:CLEANUP:put_page(page)→decrements_refcount→if_refcount=0→page_freed*/
    /*LINE56:TODO_USER:TRACE→before_put:refcount=___→after_put:refcount=___→freed?___*/
    put_page(page);
    printk(KERN_INFO "NUMA_ZONE:init_complete\n");
    return 0;
}
static void __exit numa_zone_exit(void)
{
    printk(KERN_INFO "NUMA_ZONE:exit\n"); /*LINE57:cleanup_function→called_on_rmmod*/
}
module_init(numa_zone_init); /*LINE58:module_init→tells_kernel_to_call_numa_zone_init_on_insmod*/
module_exit(numa_zone_exit); /*LINE59:module_exit→tells_kernel_to_call_numa_zone_exit_on_rmmod*/
/*LINE60:VERIFICATION_COMMANDS→run_after_saving_file:*/
/*LINE61:make_clean_&&_make→compiles_module*/
/*LINE62:sudo_insmod_numa_zone_trace.ko→loads_module→triggers_init_function*/
/*LINE63:sudo_dmesg_|_grep_NUMA_ZONE→shows_output→compare_with_worksheet_predictions*/
/*LINE64:sudo_rmmod_numa_zone_trace→unloads_module*/
/*LINE65:BUG_TO_FIX:None_intentional→but_user_should_verify_all_TODO_predictions_match_actual_output*/

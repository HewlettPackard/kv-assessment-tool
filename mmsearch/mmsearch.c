 /*
 * (c) Copyright 2016 Hewlett Packard Enterprise Development LP This
 * program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * impl ied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details. You
 * should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses.
*/

#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <asm/current.h>

#if defined(CONFIG_XPFO)
  #include <linux/xpfo.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("david roth - justin vreeland");
MODULE_DESCRIPTION("Scans a process's heap memory regions");


struct search_result {
	int offset;
	char buf[4096];
};

static struct search_result result;

#define NO_DATA_FOUND "No data found"
#define NO_DATA_BUF_LEN 32
#define SEARCH_RESULT_BUF_LEN 64

#ifdef CONFIG_XPFO
typedef enum {
	PG_user_fp = 0,/* page frame allocated to user space (fast path) */
	PG_user,/* page frame allocated to user space  */
	PG_kernel,/* page frame allocated to kernel space  */
	PG_zap,/* clean page frame  */
} xpfo_gpf_t;

/* get the value of `PG_user_fp' */
static inline int PageUserFp(struct page *page)
{
	return test_bit(PG_user_fp, &page->xpfo_flags);
}

/* get the value of `PG_user' */
static inline int PageUser(struct page *page)
{
	return test_bit(PG_user, &page->xpfo_flags);
}

/* get the value of `PG_kernel' */
static inline int PageKernel(struct page *page)
{
	return test_bit(PG_kernel, &page->xpfo_flags);
}

/* get the value of `PG_zap' */
static inline int PageZap(struct page *page)
{
	return test_bit(PG_zap, &page->xpfo_flags);
}

#endif


/*
  Stupid /proc/kallsym says it's exported but it's not
  but i can't give it the same name becuase it's defined in mm.h
*/
int (*access_process_vm_hack)(struct task_struct *tsk, unsigned long addr,
			 void *bug, int len, int write) = NULL;

static int
bad_address(void *p)
{
    unsigned long dummy;
    return probe_kernel_address((unsigned long*)p, dummy);
}

static int find_access_process_vm(void)
{
	char* sym = "access_process_vm";
	long unsigned int fpoint = kallsyms_lookup_name(sym);

	if (!fpoint) {
		pr_info("Couldn't find access_process_vm\n");
		return 1;
	}

#if defined(DEBUG)	
	pr_info("%s: found access_process_vm @ 0x%lx\n", __func__, fpoint);
#endif
	// Don't care enough to cast
	access_process_vm_hack = ( int(*)(struct task_struct *tsk, unsigned long addr,
				void *bug, int len, int write)) fpoint;

	return 0;
}

static void
dump_pg_xpfo_state(struct page *pg)
{
	if (!IS_ENABLED(CONFIG_XPFO))
	    return;
#if defined(CONFIG_XPFO)
	pr_info("%s: PG_U(%d) PG_U_FP(%d) PG_K(%d) PG_Z(%d)\n",
		__func__,
		PageUser(pg) ? 1 : 0,
		PageUserFp(pg) ? 1 : 0,
		PageKernel(pg) ? 1 : 0,
		PageZap(pg) ? 1 : 0);
#endif
}

/*
  These are the flags which the HW uses to decide whether or not to
  fault
 */
static void
dump_page_table_flags(pte_t *pte)
{
	unsigned long ptperms;
	pgprot_t prots;
	
	prots = pte_pgprot(*pte);
	ptperms = pgprot_val(prots);
	
	pr_info("%s: PTE(%lx) USER(%d) PRES(%d) PCD(%d) RW(%d) ACCESSD(%d)\n",
		__func__,
		(unsigned long) pte_val(*pte),
		ptperms & _PAGE_USER ? 1 : 0,
		ptperms & _PAGE_PRESENT ? 1 : 0,
		ptperms & _PAGE_PCD ? 1 : 0,
		ptperms & _PAGE_RW ? 1 : 0,
		ptperms &  _PAGE_ACCESSED ? 1 : 0);
}

static void
dump_vma_flags(struct vm_area_struct *vma)
{
  vm_flags_t flags = vma->vm_flags;
  pr_info("%s: %c%c%c%c \n",
	  __func__,
	  flags & VM_READ ? 'r' : '-',
	  flags & VM_WRITE ? 'w' : '-',
	  flags & VM_EXEC ? 'x' : '-',
	  flags & VM_MAYSHARE ? 's' : 'p');
}

/*
 * Original source:
 * http://wbsun.blogspot.co.uk/2010/12/
 *        convert-userspace-virtual-address-to.html
 *
 * map any virtual address to a physical address
 *
 * walks the page table entry fields to find physical
 * address
 */
static unsigned long
any_v2p(unsigned long vaddr,
	struct task_struct *tsk)
{
    struct mm_struct* mm = NULL;
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    struct page *pg;
    unsigned long paddr = 0;

    mm = tsk->mm;
    down_read(&mm->mmap_sem);
    
    pgd = pgd_offset(mm, vaddr);
    if (bad_address(pgd)) {
	    pr_info("%s: bad address of pgd %p\n", __func__, pgd);
	    goto done;
    }

    if (!pgd_present(*pgd)) {
	    pr_info("%s: pgd not present %lu\n", __func__,
		    (unsigned long)(pgd_val(*pgd)));
	    goto done;
    }

    pud = pud_offset(pgd, vaddr);
    if (bad_address(pud)) {
        pr_info("%s: bad address of pud %p\n", __func__, pud);
        goto done;
    }
    if (!pud_present(*pud)) {
	    pr_info("%s: pud not present %lu\n", __func__,
		    (unsigned long)(pud_val(*pud)));
	    goto done;
    }

    pmd = pmd_offset(pud, vaddr);
    if (bad_address(pmd)) {
        pr_info("%s: bad address of pmd %p\n", __func__, pmd);
        goto done;
    }
    if (!pmd_present(*pmd)){
	    pr_info("%s: pmd not present %lu\n", __func__,
		    (unsigned long)(pmd_val(*pmd)));
	    goto done;
    }

    pte = pte_offset_kernel(pmd, vaddr);
    if (bad_address(pte)) {
        pr_info("%s: bad address of pte %p\n", __func__,pte);
        goto done;
    }

    dump_page_table_flags(pte);
    
    if (!pte_present(*pte)) {
	    pr_info("%s: pte not present %lu\n", __func__,
		    (unsigned long)(pte_val(*pte)));
	    goto done;
    }

    pg = pte_page(*pte);
    dump_pg_xpfo_state(pg);
    
    paddr = (page_to_phys(pg) | (vaddr&(PAGE_SIZE-1)));

done:
    up_read(&mm->mmap_sem);
    return paddr;
}

/* 
 * This function performs a simple search for an agreed to credit card
 * number with the following format:
 *
 *     4      1     7    2
 * dddd dddddd ddddd dddd dddd 27
 * 3333 444444 55555 1234 0910
 *
 */
static int
find(char *buf, ssize_t size)
{
	u32 i;
	int offset = -1;

	for (i = 0; i < size - 27; i++) {

		int z;
		for (z = 0; z < 27; z++) {
			if ((z == 4) || (z == 11) || (z == 17) || (z == 22)) {
				if (buf[i + z] == ' ') {
					continue;
				} else {
					break;
				}
			} else if (('0' <= buf[i + z]) && (buf[i + z] <= '9')) {
				if (z == 26) {
#if defined(DEBUG)
					pr_info("%s: Found it!: %s\n", __func__,
						buf + i);
#endif					
					offset = i;
#if defined(DEBUG)					
					pr_info("%s: offset value: %d\n", __func__,
						offset);
#endif					
					return offset;
				}
			} else {
				break;
			}
		}
	}
	return offset;
}

static void
attack_run(struct vm_area_struct *vma,
	   struct task_struct *ts,
	   int offset)
{
	unsigned long p_addr = 0;
	unsigned long* kv_addr_p = 0;		
	char *p = NULL;
	int rcode;
	unsigned long va = vma->vm_start + offset;

#if defined(DEBUG)
	pr_info("%s: virtual address start(%#lx) offset(%d)\n",
		__func__,
		vma->vm_start,
		offset);
#endif

	p_addr = any_v2p(va, ts);
	if (p_addr == 0) {
		pr_info("%s: error converting virtual to physical address\n",
			__func__);
		return;
	}
	
#if defined(DEBUG)
	pr_info("%s: physical address %#lx\n", __func__,
		p_addr);
#endif	
	
	kv_addr_p = phys_to_virt(p_addr);
	
#if defined(DEBUG)	
	pr_info("%s: kernel virtual address %p\n",
		__func__,
		kv_addr_p);
#endif	
	/*
	  if XPFO patch has been applied bad_address() fails,
	  otherwise the address can be safely accessed
	 */
	rcode = bad_address(kv_addr_p);
	if (rcode == 0) {
		p = (char*)kv_addr_p;
		//capture result
		result.offset += snprintf(result.buf+result.offset, 
					  SEARCH_RESULT_BUF_LEN,
					  "%s\n", 
					  p);

		pr_info("%s: dereferencing address(%p): %s\n", __func__,
			kv_addr_p,
			p);
	} else {
		pr_info("%s: illegal memory access(%p) bad address\n",
			__func__,
			kv_addr_p);
	}
}

static void
buffer_search(struct vm_area_struct *vma,
	      void *kbuffer,
	      struct task_struct *ts,
	      size_t size)
{
	int offset = 0;

	offset = find(kbuffer, size);
	if (offset > 0) {
		attack_run(vma, ts, offset);
	} else {
		result.offset += snprintf(result.buf+result.offset, 
					  NO_DATA_BUF_LEN,
					  "%s \n", 
					  NO_DATA_FOUND);

		pr_info("%s: no data found\n", __func__);
	}
}

static void
process_vm_area_list(struct task_struct* ts)
{
	struct mm_struct *mm;
	struct vm_area_struct *vma;

	mm = ts->mm;
	vma = mm->mmap;
	
	while (vma) {
		unsigned long region_sz;
		void *kbuffer;
		ssize_t  size;

		/*
		  make sure we have a valid size otherwise
		  we could crash when issuing kmalloc()
		 */
		region_sz = vma->vm_end - vma->vm_start;
		if (region_sz <= 0) {
			goto next;
		}

		/*
		  only process a heap area. logic taken from
		  task_mmu.c
		 */
		if (vma->vm_start <= mm->brk &&
		    vma->vm_end >= mm->start_brk) {
			kbuffer = vmalloc(region_sz);
			
			if (NULL == kbuffer) {
				pr_info("%s: kmalloc failure, exiting\n", __func__);
				break;
			}

			size = access_process_vm_hack(ts, vma->vm_start, kbuffer,
						      vma->vm_end - vma->vm_start, 0);

			pr_info("%s: vm_start(%lx) vm_end(%lx) bytes copied(0x%zx)\n",
				__func__,
				vma->vm_start,
				vma->vm_end,
				size);

			dump_vma_flags(vma);
			
			/*
			  the 'size' return value here is
			  ambiguous. could be number of bytes copied
			  or an error code. regardless, if the return
			  value is zero, no need to examine the
			  buffer.
			*/
			if (size > 0) {
				buffer_search(vma, kbuffer, ts, size);
			}
			vfree(kbuffer);			
		}
	next:
		vma = vma->vm_next;		
	}  /* while loop end */
}

static ssize_t
mmsearch_pid_show(struct kobject *kobj, 
		  struct kobj_attribute *attr,
		  char *buf)
{
	pr_info("%s: request for search buffer data\n", __func__);
	memcpy(buf, result.buf, result.offset);

	return result.offset;
}

static ssize_t
mmsearch_pid_store(struct kobject *kobj,
		   struct kobj_attribute *attr,
		   const char *buf,
		   size_t count)
{
	uint64_t pid_t;
	int num;
	int rcode;
	struct task_struct* ts;
	struct mm_struct *mm;

	char **args;
	args = argv_split(GFP_KERNEL, buf, &num);

	memset(&result, 0, sizeof(struct search_result));

	rcode = kstrtou64(args[0], 0, &pid_t);
	if (rcode != 0) {
		pr_info("%s: failed to convert string %s to u64\n", __func__,
			args[0]);
		goto error_exit;
	}

	pr_info("%s: mmsearch pid: %llu\n", __func__, pid_t);

	ts = pid_task(find_vpid(pid_t), PIDTYPE_PID);

	if (!ts) {
		pr_info("%s: invalid task\n", __func__);
		goto error_exit;
	}

	/*
	  make sure we have not been passed a kernel thread
	 */
	mm = ts->mm;

	if (NULL == mm) {
		pr_info("%s: %s is a kernel thread\n", __func__, args[0]);
		goto error_exit;
	}

	process_vm_area_list(ts);

error_exit:
	pr_info("%s: search complete for pid: %s\n", __func__,
		args[0]);

	argv_free(args);

	return count;
}

static struct kobj_attribute mmsearch_pid_attr = __ATTR_RW(mmsearch_pid);

static struct attribute *attrs[] = {
	&mmsearch_pid_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

struct kobject *mmsearch_sysfs_kobj;

static int
mmsearch_init(void)
{
	int rcode;
	struct task_struct *current_task;
	
	if (find_access_process_vm()) {
		pr_info("Unload me please\n");
		return 0;
	}

	mmsearch_sysfs_kobj = kobject_create_and_add("mmsearch_pid", kernel_kobj);

	if (!mmsearch_sysfs_kobj)
		return -ENOMEM;

	rcode = sysfs_create_group(mmsearch_sysfs_kobj, &attr_group);
	if (rcode != 0)
		pr_info("failed to create sysfs group\n");

	current_task = current;
	
	pr_info("%s: task(%d) task_struct_ptr(%p)\n", __func__,
		current_task->tgid,
		current_task);

	if (IS_ENABLED(CONFIG_XPFO))
		pr_info("XPFO is enabled\n");

	return 0;
}

static void mmsearch_exit(void)
{
	sysfs_remove_group(mmsearch_sysfs_kobj, &attr_group);
	kobject_put(mmsearch_sysfs_kobj);
}

module_init(mmsearch_init);
module_exit(mmsearch_exit);

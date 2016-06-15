# kernel vulnerability assessment tool

**This is a work in progress. Please see the TODO file for pending
enhancements.**

kva-tool is an interactive commandline application designed as a
research tool to explore kernel insecurities.

The first offering is a Linux kernel module (mmsearch) which
demonstrates how to scan any process's memory heap for information of
interest.

We are looking for other contributors to add modules and tests
illustrating concerning behaviors.

Our hope is to prompt a broader discussion as to how better to
secure a unix kernel be it Linux or other. 

In the end, we aspire to evolve this into a regression tool to assess
the security state of any running kernel.

Ideas and improvements welcome. 

## building kva-tool

1. git clone the kv-assessment-tool repo

2. install the linux-headers package for the kernel version running on
the build system

    This is required to build the kernel module

3. install libprocps3-dev package

   This is required to access process information

4. run make from top level directory

## kva-tool commandline options

	lkm-mmsearch
		descr: loads the mmsearch kernel module

	rkm-mmsearch
		descr: unloads the mmsearch kernel module

	mmsearch-pid <proc_id>
   	        descr: searches a given process's memory space
 	               depends on mmsearch kernel module

	pagemapinfo <proc_id>
		descr: outputs process virtual memory addresses 
		       map

	procinfo <proc_id>
	       descr: displays detailed process information 

	proclist
		descr: displays the set of running processes
	        (akin to ps)

	sysinfo
		descr: reports system infomration

	quit
		descr: to exit

## running the application

1. it must be run in a root shell. It cannot be run using 'sudo'.

2. the code must be checked out and compiled on the system under test.

3. the application must be run from the sources base directory as
   illustrated above.

## mmsearch

mmsearch-pid walks the memory space of any process and searches for
a credit card number.

The commandline 'mmsearch-pid <proc_id>' option passes the process pid
to the mmsearch kernel module which scans the process's memory from
kernel space.

The search is restricted to the process's heap regions. Other regions
such as vdso, stack, anonymous are omitted.

This kernel module was originally designed to demonstrate the
effectiveness of the proposed XPFO patch set. See:

[[RFC PATCH] Add support for eXclusive Page Frame Ownership (XPFO)](https://lkml.org/lkml/2016/2/26/516)

### mmsearch demonstration

credit-card-app is a C application which allocates a chunk of memory
into which credit card information is copied.

To run the application:

start mmearch/test/credit-card-app in the foreground

Example output...

```

dsr@hlinux-devel:~/dsr_src/kernel-vulnerability-tests$ mmsearch/test/credit-card-app  
running as pid: 1188  
malloc operation 0 ptr val: 0x1e30010

```

Identify the process id of the running application, then invoke the
mmsearch-pid command line option from kva-tool.

Alternatively you can send the pid number to the kernel module with
this command 'echo <pid> > /sys/kernel/mmsearch_pid/mmsearch_pid'.

```
root@hlinux-devel:/home/dsr/dsr_src/kernel-vulnerability-tests# kva-tool/kvat
running as PID 1192  
kva-tool> mmsearch-pid 1188  

On completion, check dmesg output...

[  155.871206] mmsearch: module verification failed: signature and/or required key missing - tainting kernel  
[  155.872605] mmsearch_init: task(1192) task_struct_ptr(ffff88013a6e0000)  
[  171.864242] mmsearch_pid_store: mmsearch pid: 1188  
[  171.864255] process_vm_area_list: vm_start(1e30000) vm_end(1e32000) bytes copied(0x2000)  
[  171.864256] dump_vma_flags: rw-p  
[  171.864257] dump_page_table_flags: PTE(80000000b8707067) USER(1) PRES(1) PCD(0) RW(1) ACCESSD(1)  

[  171.864258] attack_run: dereferencing address(ffff8800b8707010): 3333 444444 55555 1234 0910 Frodo Baggins [1]  

[  171.864306] process_vm_area_list: vm_start(1e32000) vm_end(1e52000) bytes copied(0x20000)  
[  171.864307] dump_vma_flags: rw-p  
[  171.864454] buffer_search: no data found  
[  171.864457] mmsearch_pid_store: search complete for pid: 1188  

```

Alternatively to view the search result you can read from the
mmsearch_pid via the following command: 'cat
/sys/kernel/mmsearch_pid/mmsearch_pid'

If a kernel thread/process is passed in, the following error is
reported:

```
[ 5399.373518] mmsearch_pid_store: mmsearch pid: 3711  
[ 5399.373533] mmsearch_pid_store: 3711 is a kernel thread  
[ 5399.373535] mmsearch_pid_store: search complete for pid: 3711

Kernel threads are skipped because they do not have access to
userspace memory. A kernel thread process sets mm_struct to NULL.

```

## other kva-tool options

The objective of kva-tool is to provide the tools to interrogate a
system under one umbrella.

### **procinfo <proc_id>** 

Command takes as input the id of a running linux process. 

Example output:

```
kva-tool> procinfo 9722  
Searching for info on pid: 9722  
beginning address of text segment: 0x400000  
ending address of text segment: 0x60c6c0  
address of bottom of stack: 0x7ffe52993440  
address kernel stack pointer: 0x7ffe52992200  
address kernel instruction pointer: 0x7fe5f8875da3  
kva-tool>   
```

### **pagemapinfo <proc_id>** 

Lists the /proc/[pid]/maps information. 

Example output:

```
kva-tool> pagemapinfo 8946
./credit-card-app(8946)
00400000 (4 KB)        r-xp (08:01 21238394)   /home/david/tmp/kernel-vulnerability-tools/mmsearch/test/credit-card-app
00600000 (4 KB)        r--p (08:01 21238394)   /home/david/tmp/kernel-vulnerability-tools/mmsearch/test/credit-card-app
00601000 (4 KB)        rw-p (08:01 21238394)   /home/david/tmp/kernel-vulnerability-tools/mmsearch/test/credit-card-app
025f5000 (8 KB)        rw-p (00:00 0)        [heap]
025f7000 (128 KB)      rw-p (00:00 0)        [heap]
7ff024287000 (1768 KB) r-xp (08:01 2361750)  /lib/x86_64-linux-gnu/libc-2.19.so
7ff024441000 (2048 KB) ---p (08:01 2361750)  /lib/x86_64-linux-gnu/libc-2.19.so
7ff024641000 (16 KB)   r--p (08:01 2361750)  /lib/x86_64-linux-gnu/libc-2.19.so
7ff024645000 (8 KB)    rw-p (08:01 2361750)  /lib/x86_64-linux-gnu/libc-2.19.so
7ff024647000 (20 KB)   rw-p (00:00 0)        
7ff02464c000 (140 KB)  r-xp (08:01 2361736)  /lib/x86_64-linux-gnu/ld-2.19.so
7ff024845000 (12 KB)   rw-p (00:00 0)        
7ff02486b000 (12 KB)   rw-p (00:00 0)        
7ff02486e000 (4 KB)    r--p (08:01 2361736)  /lib/x86_64-linux-gnu/ld-2.19.so
7ff02486f000 (4 KB)    rw-p (08:01 2361736)  /lib/x86_64-linux-gnu/ld-2.19.so
7ff024870000 (4 KB)    rw-p (00:00 0)        
7fff5a910000 (132 KB)  rw-p (00:00 0)        [stack]
7fff5a9fe000 (8 KB)    r-xp (00:00 0)        [vdso]
ffffffffff600000 (4 KB)  r-xp (00:00 0)      [vsyscall]
mapped:   4328 KB writable/private: 332 KB shared: 0 KB

```

### **sysinfo** 

Lists system information of interest.

Example output:

```
kva-tool> sysinfo  
processor: x86_64  
release: 3.13.0-71-generic  
version: #114-Ubuntu SMP Tue Dec 1 02:34:22 UTC 2015  
total ram: 25264107520  
free ram: 21985210368  
total swap: 25750925312  
free swap: 25750925312  
\num current processes: 723  
total high memory size: 0  
free high memory: 0  
memory units size (bytes): 1  

```

###  **proclist** Akin to ps output. 

Lists the set of current running processes.

Example output:

```
kva-tool> proclist  
     1                  init:	  808	   45	  119  
     2              kthreadd:	    0	    0	    1  
     3           ksoftirqd/0:	    0	    0	   27  
     4           kworker/0:0:	    0	    0	  560  
     5          kworker/0:0H:	    0	    0	    0  
     6         kworker/u64:0:	    0	    0	 2266  
     8             rcu_sched:	    0	    0	  957  
     9               rcuos/0:	    0	    0	 1282  
    10               rcuos/1:	    0	    0	   42  
    11               rcuos/2:	    0	    0	   47  
    12               rcuos/3:	    0	    0	   38  
    13               rcuos/4:	    0	    0	   42  
    14               rcuos/5:	    0	    0	   36  
    15               rcuos/6:	    0	    0	   31  
    16               rcuos/7:	    0	    0	   31  
    17               rcuos/8:	    0	    0	  443  
    18               rcuos/9:	    0	    0	   23  
    19              rcuos/10:	    0	    0	   24  
    20              rcuos/11:	    0	    0	  144  
    ...

```
















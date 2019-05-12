// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

// Added in lab4.
extern void _pgfault_upcall(void);

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *)utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if ((err & FEC_WR) == 0)
		panic("pgfault: not caused by writer.");

	if ((uvpt[PGNUM(addr)] & PTE_COW) == 0)
		panic("pgfault: not COW.");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	// LAB 4: Your code here.
	envid_t envid = sys_getenvid();
	addr = ROUNDDOWN(addr, PGSIZE);
	if (sys_page_alloc(envid, PFTEMP, PTE_P | PTE_U | PTE_W) < 0)
		panic("pgfault: page alloc failed.");

	memmove(PFTEMP, addr, PGSIZE);

	if (sys_page_map(envid, PFTEMP, envid, addr, PTE_P | PTE_U | PTE_W) < 0)
		panic("pgfault: page map failed.");

	if (sys_page_unmap(envid, PFTEMP) < 0)
		panic("pgfault: page unmap failed.");

	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	cprintf("duppage i\n");
	int r;

	// LAB 4: Your code here.
	envid_t current_envid = sys_getenvid();
	void *addr = (void *)(pn * PGSIZE);
	if (uvpt[pn] & (PTE_W | PTE_COW))
	{
		cprintf("step 1.1 in duppage\n");
		if (sys_page_map(current_envid, addr, envid, addr, PTE_P | PTE_U | PTE_COW) < 0)
			panic("duppage: page map 1 failed.");

		cprintf("step 2 in duppage\n");
		if (sys_page_map(current_envid, addr, current_envid, addr, PTE_P | PTE_U | PTE_COW) < 0)
			panic("duppage: page map 2 failed.");
	}
	else
	{
		cprintf("step 1.2 in duppage\n");
		if (sys_page_map(current_envid, addr, envid, addr, PTE_P | PTE_U) < 0)
			panic("duppage: non writeable pr cow.");
	}

	// panic("duppage not implemented");
	cprintf("duppage o\n");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	cprintf("fork i\n");
	envid_t envid;

	set_pgfault_handler(pgfault);
	cprintf("step 1 in fork\n");

	envid = sys_exofork();
	cprintf("step 2 in fork\n");
	if (envid < 0)
		panic("fork: exofork failed.");
	else if (envid == 0) // child
	{
		cprintf("step 3.child in fork\n");
		thisenv = &(envs[ENVX(sys_getenvid())]);
		return 0;
	}
	else // parent
	{
		cprintf("step 3.parent in fork\n");
		for (uint32_t i = UTEXT; i < USTACKTOP; i += PGSIZE)
		{
			if ((uvpd[PDX(i)] & PTE_P) &&
				(uvpt[PGNUM(i)] & (PTE_P | PTE_U)) == (PTE_P | PTE_U))
				duppage(envid, PGNUM(i));
			// cprintf("loop in fork\n");
		}
		cprintf("loop in fork end\n");
	}

	cprintf("step 4 in fork\n");
	if (sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W) < 0)
		panic("fork: page alloc failed.");

	cprintf("step 5 in fork\n");
	if (sys_env_set_pgfault_upcall(envid, _pgfault_upcall) < 0)
		panic("fork: set pgfault upcall failed.");

	cprintf("step 6 in fork\n");
	if (sys_env_set_status(envid, ENV_RUNNABLE) < 0)
		panic("fork: set status failed.");

	cprintf("fork o\n");
	return envid;
	// panic("fork not implemented");
}

// Challenge!
int sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}

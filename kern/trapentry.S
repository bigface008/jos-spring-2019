/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(EP_DIVIDE, T_DIVIDE)
TRAPHANDLER_NOEC(EP_DEBUG, T_DEBUG)
TRAPHANDLER_NOEC(EP_NMI, T_NMI)
TRAPHANDLER_NOEC(EP_BRKPT, T_BRKPT)
TRAPHANDLER_NOEC(EP_OFLOW, T_OFLOW)
TRAPHANDLER_NOEC(EP_BOUND, T_BOUND)
TRAPHANDLER_NOEC(EP_ILLOP, T_ILLOP)
TRAPHANDLER_NOEC(EP_DEVICE, T_DEVICE)
TRAPHANDLER(EP_DBLFLT, T_DBLFLT)
TRAPHANDLER(EP_TSS, T_TSS)
TRAPHANDLER(EP_SEGNP, T_SEGNP)
TRAPHANDLER(EP_STACK, T_STACK)
TRAPHANDLER(EP_GPFLT, T_GPFLT)
TRAPHANDLER(EP_PGFLT, T_PGFLT)
TRAPHANDLER_NOEC(EP_FPERR, T_FPERR)
TRAPHANDLER_NOEC(EP_ALIGN, T_ALIGN)
TRAPHANDLER_NOEC(EP_MCHK, T_MCHK)
TRAPHANDLER_NOEC(EP_SIMDERR, T_SIMDERR)
TRAPHANDLER_NOEC(EP_SYSCALL, T_SYSCALL)

/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	pushw $0
	pushw %ds
	pushw $0
	pushw %es
	pushal

	movl $(GD_KD), %eax
	movw %ax, %ds
	movw %ax, %es

	pushl %esp
	call trap

#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

static struct E1000 *base;

struct tx_desc *tx_descs;
#define N_TXDESC (PGSIZE / sizeof(struct tx_desc))

int
e1000_tx_init()
{
	// Allocate one page for descriptors
	struct PageInfo *page = page_alloc(ALLOC_ZERO);
	// tx_descs = (struct tx_desc *)page2kva(page);

	// // Initialize all descriptors
	// for (int i = 0; i < N_TXDESC; i++)
	// {
	// 	struct PageInfo *buf_page = page_alloc(ALLOC_ZERO);

	// }
	

	// // Set hardward registers
	// // Look kern/e1000.h to find useful definations
	// e1000_ptr->TCTL |= E1000_TCTL_EN;
	// e1000_ptr->TCTL |= E1000_TCTL_PSP;
	// e1000_ptr->TCTL &= ~E1000_TCTL_CT_ETHER;
	// e1000_ptr->TCTL |= (0x10) << 4;
	// e1000_ptr->TCTL &= ~E1000_TCTL_COLD_FULL_DUPLEX;
	// e1000_ptr->TCTL |= (0x40) << 12;
	// e1000_ptr->TIPG = E1000_TIPG_DEFAULT;
	// e1000_ptr->TDBAL = PADDR(tx_descs);
	// e1000_ptr->TDBAH = 0;
	// e1000_ptr->TDLEN = sizeof();
	// e1000_ptr->TDH = ;
	// e1000_ptr->TDT = ;
	// e1000_ptr->

	return 0;
}

struct rx_desc *rx_descs;
#define N_RXDESC (PGSIZE / sizeof(struct rx_desc))

int
e1000_rx_init()
{
	// Allocate one page for descriptors

	// Initialize all descriptors
	// You should allocate some pages as receive buffer

	// Set hardward registers
	// Look kern/e1000.h to find useful definations

	return 0;
}

int
pci_e1000_attach(struct pci_func *pcif)
{
	// Enable PCI function
	pci_func_enable(pcif);

	// Map MMIO region and save the address in 'base;
	e1000_ptr = (struct E1000 *)mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
	cprintf("E1000 STATUS 0x%08x\n", e1000_ptr->STATUS);

	e1000_tx_init();
	e1000_rx_init();
	return 0;
}

int
e1000_tx(const void *buf, uint32_t len)
{
	// Send 'len' bytes in 'buf' to ethernet
	// Hint: buf is a kernel virtual address

	return 0;
}

int
e1000_rx(void *buf, uint32_t len)
{
	// Copy one received buffer to buf
	// You could return -E_AGAIN if there is no packet
	// Check whether the buf is large enough to hold
	// the packet
	// Do not forget to reset the decscriptor and
	// give it back to hardware by modifying RDT

	return 0;
}

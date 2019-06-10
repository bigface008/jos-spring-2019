#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

static struct E1000 *base;

// Modified by student.
#define N_TXDESC (PGSIZE / sizeof(struct tx_desc))
struct tx_desc tx_descs[N_TXDESC] __attribute__((aligned(16))); // *
char tx_pkt_buf[N_TXDESC][TX_PKT_SIZE];

int
e1000_tx_init()
{
	// Allocate one page for descriptors
	// Not necessary.

	// Initialize all descriptors
	memset(tx_descs, 0, sizeof(tx_descs));
	memset(tx_pkt_buf, 0, sizeof(tx_pkt_buf));
	for (int i = 0; i < N_TXDESC; i++)
	{
		tx_descs[i].addr = PADDR(tx_pkt_buf[i]);
		tx_descs[i].cmd = 0;
		tx_descs[i].status |= E1000_TX_STATUS_DD;
	}

	// Set hardward registers
	// Look kern/e1000.h to find useful definations
	e1000_ptr->TDBAL = PADDR(tx_descs);
	e1000_ptr->TDBAH = 0;
	e1000_ptr->TDLEN = sizeof(tx_descs);
	e1000_ptr->TDH = 0;
	e1000_ptr->TDT = 0;
	e1000_ptr->TCTL |= E1000_TCTL_EN;
	e1000_ptr->TCTL |= E1000_TCTL_PSP;
	e1000_ptr->TCTL |= E1000_TCTL_CT_ETHER;
	e1000_ptr->TCTL |= E1000_TCTL_COLD_FULL_DUPLEX;
	e1000_ptr->TIPG = E1000_TIPG_DEFAULT;

	return 0;
}

// Modified by student.
#define N_RXDESC (PGSIZE / sizeof(struct rx_desc))
struct rx_desc rx_descs[N_RXDESC];

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
	if (!buf || len > TX_PKT_SIZE)
		return -E_INVAL;

	uint32_t tdt = e1000_ptr->TDT;
	if (!(tx_descs[tdt].status & E1000_TX_STATUS_DD))
		return -E_TXQ_FULL;

	memset(tx_pkt_buf[tdt], 0, TX_PKT_SIZE);
	memmove(tx_pkt_buf[tdt], buf, len);

	tx_descs[tdt].length = len;
	tx_descs[tdt].status &= ~E1000_TX_STATUS_DD;
	tx_descs[tdt].cmd |= E1000_TX_CMD_EOP;
	tx_descs[tdt].cmd |= E1000_TX_CMD_RS;

	e1000_ptr->TDT = (tdt + 1) % N_TXDESC;
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

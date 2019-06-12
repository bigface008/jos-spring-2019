#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

static struct E1000 *base;

// Modified by student.
// struct tx_desc tx_descs[N_TXDESC] __attribute__((aligned(16))); // ?
#define N_TXDESC (PGSIZE / sizeof(struct tx_desc))
struct tx_desc *tx_descs;
char tx_pkt_buf[N_TXDESC][TX_PKT_SIZE];

int
e1000_tx_init()
{
	// Allocate one page for descriptors
	struct PageInfo *pg = page_alloc(ALLOC_ZERO);
	tx_descs = page2kva(pg);
	// memset(tx_descs, 0, PGSIZE);

	// Initialize all descriptors
	memset(tx_pkt_buf, 0, sizeof(tx_pkt_buf));
	for (int i = 0; i < N_TXDESC; i++)
	{
		tx_descs[i].addr = PADDR(tx_pkt_buf[i]);
		tx_descs[i].cmd = 0;
		tx_descs[i].status |= E1000_TX_STATUS_DD;
	}

	// Set hardward registers
	// Look kern/e1000.h to find useful definations
	base->TDBAL = PADDR(tx_descs);
	base->TDBAH = 0;
	base->TDLEN = PGSIZE;
	base->TDH = 0;
	base->TDT = 0;
	base->TCTL = E1000_TCTL_EN;
	base->TCTL |= E1000_TCTL_PSP;
	base->TCTL |= E1000_TCTL_CT_ETHER;
	base->TCTL |= E1000_TCTL_COLD_FULL_DUPLEX;
	base->TIPG = E1000_TIPG_DEFAULT;

	return 0;
}

// Modified by student.
#define N_RXDESC (PGSIZE / sizeof(struct rx_desc)) // 256
struct rx_desc *rx_descs;
char *rx_pkt_buf[N_RXDESC];
// struct rx_desc rx_descs[N_RXDESC] __attribute__((aligned(16)));
// char rx_pkt_buf[N_RXDESC][RX_PKT_SIZE];

int
e1000_rx_init()
{
	// Allocate one page for descriptors
	// Not necessary.
	struct PageInfo *pg = page_alloc(ALLOC_ZERO);
	rx_descs = page2kva(pg);
	// memset(rx_descs, 0, PGSIZE);

	// Initialize all descriptors
	// You should allocate some pages as receive buffer
	// The size of jos is not enough if I use the same strategy as the transmit part.
	// I don't know why. >_<
	// int pgn = N_RXDESC * RX_PKT_SIZE / PGSIZE;             // 128
	// int buf_per_page = PGSIZE / RX_PKT_SIZE;               // 2
	// struct PageInfo *pgl = page_alloc_contiguous(pgn, ALLOC_ZERO);
	// for (int i = 0; i < pgn; i++)
	// {
	// 	// cprintf(" step0 in e1000_rx_init\n");
	// 	struct PageInfo *pg_tmp = &pgl[i];
	// 	// cprintf(" step1 in e1000_rx_init\n");
	// 	rx_pkt_buf[i * 2] = (char *)page2kva(pg_tmp);
	// 	// cprintf(" step2 in e1000_rx_init\n");
	// 	rx_pkt_buf[i * 2 + 1] = rx_pkt_buf[i * 2] + PGSIZE;
	// 	// cprintf(" step3 in e1000_rx_init\n");
	// 	rx_descs[i * 2].addr = PADDR(rx_pkt_buf[i * 2]);
	// 	// cprintf(" step4 in e1000_rx_init\n");
	// 	rx_descs[i * 2 + 1].addr = PADDR(rx_pkt_buf[i * 2 + 1]);
	// 	cprintf("e1000_rx_init %4.d %p %4.d %p pp_c %d\n", i * 2 + 1, rx_pkt_buf[i * 2], i * 2 + 2, rx_pkt_buf[i * 2 + 1], pg_tmp->pp_c);
	// }

	for (int i = 0; i < N_RXDESC; i++)
	{
		struct PageInfo *pg_tmp = page_alloc(ALLOC_ZERO);
		rx_pkt_buf[i] = (char *)page2kva(pg_tmp);
		// cprintf("e1000_rx_init buf pos %p pp_c %d\n", rx_pkt_buf[i], pg_tmp->pp_c);
		rx_descs[i].addr = PADDR(rx_pkt_buf[i]);
	}

	// Set hardward registers
	// Look kern/e1000.h to find useful definations
	base->RAL = QEMU_MAC_LOW;
	base->RAH = QEMU_MAC_HIGH;
	base->RDBAL = PADDR(rx_descs);
	base->RDBAH = 0;
	base->RDLEN = PGSIZE;
	base->RDH = 0;
	base->RDT = N_RXDESC - 1;

	// ?
	base->RCTL = E1000_RCTL_EN;
	base->RCTL &= ~E1000_RCTL_LPE;
	base->RCTL &= ~E1000_RCTL_LBM_MASK;
	base->RCTL &= ~E1000_RCTL_RDMTS_MASK;
	base->RCTL &= ~E1000_RCTL_MO_MASK;
	base->RCTL |= E1000_RCTL_BAM;
	base->RCTL &= ~E1000_RCTL_BSEX;
	base->RCTL &= ~E1000_RCTL_BSIZE_MASK;
	base->RCTL |= E1000_RCTL_BSIZE_2048;
	base->RCTL |= E1000_RCTL_SECRC;

	return 0;
}

int
pci_e1000_attach(struct pci_func *pcif)
{
	// Enable PCI function
	pci_func_enable(pcif);

	// Map MMIO region and save the address in 'base;
	base = (struct E1000 *)mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
	memset(base, 0, sizeof(struct E1000));
	cprintf("E1000 STATUS 0x%08x\n", base->STATUS);

	e1000_tx_init();
	e1000_rx_init();
	return 0;
}

int
e1000_tx(const void *buf, uint32_t len)
{
	// Send 'len' bytes in 'buf' to ethernet
	// Hint: buf is a kernel virtual address
	cprintf("e1000_tx buf %p\n", buf);
	if (!buf || len > TX_PKT_SIZE)
		return -E_INVAL;

	uint32_t tdt = base->TDT;
	if (!(tx_descs[tdt].status & E1000_TX_STATUS_DD))
		return -E_TXQ_FULL;

	memset(tx_pkt_buf[tdt], 0, TX_PKT_SIZE);
	memmove(tx_pkt_buf[tdt], buf, len);

	tx_descs[tdt].length = len;
	tx_descs[tdt].status &= ~E1000_TX_STATUS_DD;
	tx_descs[tdt].cmd |= E1000_TX_CMD_EOP;
	tx_descs[tdt].cmd |= E1000_TX_CMD_RS;

	base->TDT = (tdt + 1) % N_TXDESC;
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
	// cprintf("kern/e1000.c:%d\n", __LINE__);
	if (!buf)
		return -E_INVAL;

	uint32_t rdt = (base->RDT + 1) % N_RXDESC;
	if (!(rx_descs[rdt].status & E1000_RX_STATUS_DD))
		return -E_AGAIN;

	int pl = rx_descs[rdt].length;
	if (pl > len)
		pl = len;

	// hexdump("ex1000_rx buf\n", &rx_pkt_buf[rdt]);
	memmove(buf, (rx_pkt_buf[rdt]), pl);
	// memmove(buf, &rx_pkt_buf[rdt], pl);
	rx_descs[rdt].status &= ~E1000_RX_STATUS_DD;
	base->RDT = rdt;
	return pl;
}

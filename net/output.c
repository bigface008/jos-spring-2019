#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet request (using ipc_recv)
	//	- send the packet to the device driver (using sys_net_send)
	//	do the above things in a loop
	int r, perm;
	envid_t from_env;

	while (1)
	{
		r = ipc_recv(&from_env, &nsipcbuf, &perm);
		if (r != NSREQ_OUTPUT)
		{
			cprintf("net/output.c:%d Not NSREQ_OUTPUT\n", __LINE__);
			continue;
		}

		while ((r = sys_net_send(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) < 0)
		{
			cprintf("net/output.c:%d sys_net_send result %d\n", __LINE__, r);
			if (r != -E_TXQ_FULL)
				panic("net/output.c:%d %e", __LINE__, r);
		}
	}
}

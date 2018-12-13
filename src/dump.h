#ifndef NETPLEX_DUMP_H
#define NETPLEX_DUMP_H

#include "packet.h"

void nplx_dump_client_packet(
	nplx_client_packet_t *packet
);

void nplx_dump_server_packet(
	nplx_server_packet_t *packet
);

#endif /* NETPLEX_DUMP_H */

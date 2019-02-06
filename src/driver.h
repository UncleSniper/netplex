#ifndef NETPLEX_DRIVER_H
#define NETPLEX_DRIVER_H

#include "poll.h"
#include "socket.h"
#include "packet.h"
#include "process.h"
#include "hashtable.h"

typedef struct nplx_driver {
	nplx_pool_t streams;
	nplx_pool_t sockets;
	nplx_pool_t processes;
	nplx_fd_hashtable_t select_fds;
	nplx_poll_set_t poll_set;
	nplx_client_packet_decoder_t packet_decoder;
} nplx_driver_t;

int nplx_driver_init(
	nplx_driver_t *driver
);

void nplx_driver_destroy(
	nplx_driver_t *driver
);

#endif /* NETPLEX_DRIVER_H */

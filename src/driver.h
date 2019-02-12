#ifndef NETPLEX_DRIVER_H
#define NETPLEX_DRIVER_H

#include "poll.h"
#include "socket.h"
#include "packet.h"
#include "process.h"
#include "hashtable.h"

typedef struct nplx_driver {
	int shutdown;
	nplx_pool_t streams;
	nplx_pool_t sockets;
	nplx_pool_t processes;
	nplx_fd_hashtable_t select_fds;
	nplx_poll_set_t poll_set;
	nplx_client_packet_decoder_t packet_decoder;
	char *packet_buffer;
	uint32_t packet_buffer_size;
} nplx_driver_t;

int nplx_driver_init(
	nplx_driver_t *driver
);

void nplx_driver_destroy(
	nplx_driver_t *driver
);

nplx_error_pump_result_t nplx_driver_pump(
	nplx_driver_t *driver,
	nplx_error_t *error
);

nplx_error_pump_result_t nplx_driver_pump_fd(
	nplx_driver_t *driver,
	int fd,
	nplx_error_t *error
);

nplx_error_pump_result_t nplx_driver_handle_stdin(
	int fd,
	uint32_t id,
	nplx_driver_t *driver,
	nplx_error_t *error
);

nplx_error_pump_result_t nplx_driver_dispatch_client_packet(
	nplx_driver_t *driver,
	nplx_error_t *error
);

nplx_error_pump_result_t nplx_driver_handle_stream_read(
	int fd,
	uint32_t id,
	nplx_driver_t *driver,
	nplx_error_t *error
);

int nplx_driver_rebuild_poll_set(
	nplx_driver_t *driver
);

#endif /* NETPLEX_DRIVER_H */

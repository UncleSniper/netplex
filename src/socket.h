#ifndef NETPLEX_SOCKET_H
#define NETPLEX_SOCKET_H

#include "stream.h"

typedef struct nplx_socket {
	nplx_poolable_t poolable;
	int fd;
	nplx_socket_input_stream_t *input_stream;
	nplx_socket_output_stream_t *output_stream;
} nplx_socket_t;

void nplx_socket_init(
	nplx_socket_t *socket,
	int fd
);

int nplx_socket_deep_init(
	nplx_socket_t *socket,
	int fd
);

void nplx_socket_destroy(
	nplx_socket_t *socket
);

#endif /* NETPLEX_SOCKET_H */

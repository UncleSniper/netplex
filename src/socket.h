#ifndef NETPLEX_SOCKET_H
#define NETPLEX_SOCKET_H

#include "stream.h"

typedef struct nplx_socket {
	int fd;
	nplx_input_stream_t *input_stream;
	nplx_output_stream_t *output_stream;
} nplx_socket_t;

#endif /* NETPLEX_SOCKET_H */

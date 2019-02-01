#define _DEFAULT_SOURCE
#include <errno.h>
#include <stdlib.h>

#include "socket.h"

void nplx_socket_init(
	nplx_socket_t *socket,
	int fd
) {
	socket->fd = fd;
	socket->input_stream = NULL;
	socket->output_stream = NULL;
}

int nplx_socket_deep_init(
	nplx_socket_t *socket,
	int fd
) {
	nplx_socket_input_stream_t *input_stream;
	nplx_socket_output_stream_t *output_stream;
	input_stream = (nplx_socket_input_stream_t*)malloc(sizeof(nplx_socket_input_stream_t));
	if(!input_stream)
		return ENOMEM;
	output_stream = (nplx_socket_output_stream_t*)malloc(sizeof(nplx_socket_output_stream_t));
	if(!output_stream) {
		free(input_stream);
		return ENOMEM;
	}
	socket->fd = fd;
	nplx_socket_input_stream_init(input_stream, fd, socket);
	nplx_socket_output_stream_init(output_stream, fd, socket);
	return 0;
}

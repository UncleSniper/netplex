#include <errno.h>
#include <stdlib.h>

#include "socket.h"

static const nplx_poolable_vtable_t socket_vtable = {
	.destroy = (nplx_poolable_destroy_cb)nplx_socket_destroy
};

void nplx_socket_init(
	nplx_socket_t *socket,
	int fd
) {
	socket->poolable.vtable = &socket_vtable;
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
	socket->poolable.vtable = &socket_vtable;
	socket->fd = fd;
	nplx_socket_input_stream_init(input_stream, fd, socket);
	nplx_socket_output_stream_init(output_stream, fd, socket);
	return 0;
}

void nplx_socket_destroy(
	nplx_socket_t *socket
) {
	if(socket->input_stream) {
		nplx_poolable_destroy((nplx_poolable_t*)socket->input_stream);
		free(socket->input_stream);
	}
	if(socket->output_stream) {
		nplx_poolable_destroy((nplx_poolable_t*)socket->output_stream);
		free(socket->output_stream);
	}
}

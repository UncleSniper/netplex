#define _DEFAULT_SOURCE
#include <errno.h>
#include <sys/socket.h>

#include "stream.h"
#include "socket.h"

/* fd_*_stream */

int nplx_fd_input_stream_read(
	nplx_fd_input_stream_t *stream,
	char *buffer,
	size_t size,
	size_t *read_count
) {
	ssize_t count;
	count = read(stream->fd, buffer, size);
	if(count == (ssize_t)-1)
		return errno;
	*read_count = (size_t)count;
	return 0;
}

int nplx_fd_output_stream_write(
	nplx_fd_output_stream_t *stream,
	const char *buffer,
	size_t size
) {
	ssize_t count;
	while(size) {
		count = write(stream->fd, buffer, size);
		if(count == (ssize_t)-1)
			return errno;
		buffer += count;
		size -= (size_t)count;
	}
	return 0;
}

/* file_input_stream */

static const nplx_input_stream_vtable_t file_input_stream_vtable = {
	.stream = {
		.poolable = {
			.destroy = nplx_null_poolable_destroy
		},
		.close = (nplx_stream_close_cb)nplx_file_input_stream_close
	},
	.read = (nplx_input_stream_read_cb)nplx_fd_input_stream_read
};

void nplx_file_input_stream_init(
	nplx_fd_input_stream_t *stream,
	int fd
) {
	stream->input_stream.vtable = &file_input_stream_vtable;
	stream->fd = fd;
}

int nplx_file_input_stream_close(
	nplx_fd_input_stream_t *stream
) {
	if(close(stream->fd))
		return errno;
	return 0;
}

/* file_output_stream */

static const nplx_output_stream_vtable_t file_output_stream_vtable = {
	.stream = {
		.poolable = {
			.destroy = nplx_null_poolable_destroy
		},
		.close = (nplx_stream_close_cb)nplx_file_output_stream_close
	},
	.write = (nplx_output_stream_write_cb)nplx_fd_output_stream_write
};

void npxl_file_output_stream_init(
	nplx_fd_output_stream_t *stream,
	int fd
) {
	stream->output_stream.vtable = &file_output_stream_vtable;
	stream->fd = fd;
}

int nplx_file_output_stream_close(
	nplx_fd_output_stream_t *stream
) {
	if(close(stream->fd))
		return errno;
	return 0;
}

/* socket_input_stream */

static const nplx_input_stream_vtable_t socket_input_stream_vtable = {
	.stream = {
		.poolable = {
			.destroy = nplx_null_poolable_destroy
		},
		.close = (nplx_stream_close_cb)nplx_socket_input_stream_close
	},
	.read = (nplx_input_stream_read_cb)nplx_fd_input_stream_read
};

void nplx_socket_input_stream_init(
	nplx_socket_input_stream_t *stream,
	int fd,
	nplx_socket_t *socket
) {
	stream->fd_input_stream.input_stream.vtable = &socket_input_stream_vtable;
	stream->fd_input_stream.fd = fd;
	stream->socket = socket;
	if(socket)
		socket->input_stream = stream;
}

int nplx_socket_input_stream_close(
	nplx_socket_input_stream_t *stream
) {
	if(shutdown(stream->fd_input_stream.fd, SHUT_RD))
		return errno;
	return 0;
}

void nplx_socket_input_stream_destroy(
	nplx_socket_input_stream_t *stream
) {
	if(stream->socket)
		stream->socket->input_stream = NULL;
}

/* socket_output_stream */

static const nplx_output_stream_vtable_t socket_output_stream_vtable = {
	.stream = {
		.poolable = {
			.destroy = nplx_null_poolable_destroy
		},
		.close = (nplx_stream_close_cb)nplx_socket_output_stream_close
	},
	.write = (nplx_output_stream_write_cb)nplx_fd_output_stream_write
};

void nplx_socket_output_stream_init(
	nplx_socket_output_stream_t *stream,
	int fd,
	nplx_socket_t *socket
) {
	stream->fd_output_stream.output_stream.vtable = &socket_output_stream_vtable;
	stream->fd_output_stream.fd = fd;
	stream->socket = socket;
	if(socket)
		socket->output_stream = stream;
}

int nplx_socket_output_stream_close(
	nplx_socket_output_stream_t *stream
) {
	if(shutdown(stream->fd_output_stream.fd, SHUT_WR))
		return errno;
	return 0;
}

void nplx_socket_output_stream_destroy(
	nplx_socket_output_stream_t *stream
) {
	if(stream->socket)
		stream->socket->output_stream = NULL;
}

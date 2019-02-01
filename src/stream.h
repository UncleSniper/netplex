#ifndef NETPLEX_STREAM_H
#define NETPLEX_STREAM_H

#include <unistd.h>

#include "pool.h"

/* stream */

struct nplx_stream;

typedef int (*nplx_stream_close_cb)(
	struct nplx_stream *stream
);

typedef struct nplx_stream_vtable {
	nplx_poolable_vtable_t poolable;
	nplx_stream_close_cb close;
} nplx_stream_vtable_t;

typedef struct nplx_stream {
	const nplx_stream_vtable_t *vtable;
} nplx_stream_t;

/* input_stream */

struct nplx_input_stream;

typedef int (*nplx_input_stream_read_cb)(
	struct nplx_input_stream *stream,
	char *buffer,
	size_t size,
	size_t *read
);

typedef struct nplx_input_stream_vtable {
	nplx_stream_vtable_t stream;
	nplx_input_stream_read_cb read;
} nplx_input_stream_vtable_t;

typedef struct nplx_input_stream {
	const nplx_input_stream_vtable_t *vtable;
} nplx_input_stream_t;

/* output_stream */

struct nplx_output_stream;

typedef int (*nplx_output_stream_write_cb)(
	struct nplx_output_stream *stream,
	const char *buffer,
	size_t size
);

typedef struct nplx_output_stream_vtable {
	nplx_stream_vtable_t stream;
	nplx_output_stream_write_cb write;
} nplx_output_stream_vtable_t;

typedef struct nplx_output_stream {
	const nplx_output_stream_vtable_t *vtable;
} nplx_output_stream_t;

/* fd_*_stream */

typedef struct nplx_fd_input_stream {
	nplx_input_stream_t input_stream;
	int fd;
} nplx_fd_input_stream_t;

typedef struct nplx_fd_output_stream {
	nplx_output_stream_t output_stream;
	int fd;
} nplx_fd_output_stream_t;

/* dispatch */

inline int nplx_stream_close(
	nplx_stream_t *stream
) {
	return stream->vtable->close(stream);
}

inline int nplx_input_stream_read(
	nplx_input_stream_t *stream,
	char *buffer,
	size_t size,
	size_t *read
) {
	return stream->vtable->read(stream, buffer, size, read);
}

inline int nplx_output_stream_write(
	nplx_output_stream_t *stream,
	const char *buffer,
	size_t size
) {
	return stream->vtable->write(stream, buffer, size);
}

/* fd_*_stream */

int nplx_fd_input_stream_read(
	nplx_fd_input_stream_t *stream,
	char *buffer,
	size_t size,
	size_t *read
);

int nplx_fd_output_stream_write(
	nplx_fd_output_stream_t *stream,
	const char *buffer,
	size_t size
);

/* file_input_stream */

void nplx_file_input_stream_init(
	nplx_fd_input_stream_t *stream,
	int fd
);

int nplx_file_input_stream_close(
	nplx_fd_input_stream_t *stream
);

/* file_output_stream */

void npxl_file_output_stream_init(
	nplx_fd_output_stream_t *stream,
	int fd
);

int nplx_file_output_stream_close(
	nplx_fd_output_stream_t *stream
);

/* socket_input_stream */

void nplx_socket_input_stream_init(
	nplx_fd_input_stream_t *stream,
	int fd
);

int nplx_socket_input_stream_close(
	nplx_fd_input_stream_t *stream
);

/* socket_output_stream */

void nplx_socket_output_stream_init(
	nplx_fd_output_stream_t *stream,
	int fd
);

int nplx_socket_output_stream_close(
	nplx_fd_output_stream_t *stream
);

#endif /* NETPLEX_STREAM_H */

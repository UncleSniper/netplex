#ifndef NETPLEX_STREAM_H
#define NETPLEX_STREAM_H

#include <unistd.h>

#include "pool.h"

struct nplx_driver;

/* stream */

typedef enum nplx_stream_direction {
	NPLX_INPUT_STREAM,
	NPLX_OUTPUT_STREAM
} nplx_stream_direction_t;

struct nplx_stream;

typedef int (*nplx_stream_close_cb)(
	struct nplx_stream *stream
);

typedef void (*nplx_stream_remove_state_cb)(
	struct nplx_stream *stream,
	struct nplx_driver *driver
);

typedef struct nplx_stream_vtable {
	nplx_poolable_vtable_t poolable;
	nplx_stream_direction_t direction;
	nplx_stream_close_cb close;
	nplx_stream_remove_state_cb remove_state;
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

/* socket_*_stream */

struct nplx_socket;

typedef struct nplx_socket_input_stream {
	nplx_fd_input_stream_t fd_input_stream;
	struct nplx_socket *socket;
} nplx_socket_input_stream_t;

typedef struct nplx_socket_output_stream {
	nplx_fd_output_stream_t fd_output_stream;
	struct nplx_socket *socket;
} nplx_socket_output_stream_t;

/* process_*_stream */

struct nplx_process;

typedef struct nplx_process_input_stream {
	nplx_fd_input_stream_t fd_input_stream;
	struct nplx_process *process;
} nplx_process_input_stream_t;

typedef struct nplx_process_output_stream {
	nplx_fd_output_stream_t fd_output_stream;
	struct nplx_process *process;
} nplx_process_output_stream_t;

/* dispatch */

inline int nplx_stream_close(
	nplx_stream_t *stream
) {
	return stream->vtable->close(stream);
}

inline void nplx_stream_remove_state(
	nplx_stream_t *stream,
	struct nplx_driver *driver
) {
	stream->vtable->remove_state(stream, driver);
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

void nplx_fd_input_stream_remove_state(
	nplx_fd_input_stream_t *stream,
	struct nplx_driver *driver
);

void nplx_fd_output_stream_remove_state(
	nplx_fd_output_stream_t *stream,
	struct nplx_driver *driver
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
	nplx_socket_input_stream_t *stream,
	int fd,
	struct nplx_socket *socket
);

int nplx_socket_input_stream_close(
	nplx_socket_input_stream_t *stream
);

void nplx_socket_input_stream_destroy(
	nplx_socket_input_stream_t *stream
);

/* socket_output_stream */

void nplx_socket_output_stream_init(
	nplx_socket_output_stream_t *stream,
	int fd,
	struct nplx_socket *socket
);

int nplx_socket_output_stream_close(
	nplx_socket_output_stream_t *stream
);

void nplx_socket_output_stream_destroy(
	nplx_socket_output_stream_t *stream
);

/* process_stdin_stream */

void nplx_process_stdin_stream_init(
	nplx_process_output_stream_t *stream,
	int fd,
	struct nplx_process *process
);

void nplx_process_stdin_stream_destroy(
	nplx_process_output_stream_t *stream
);

/* process_stdout_stream */

void nplx_process_stdout_stream_init(
	nplx_process_input_stream_t *stream,
	int fd,
	struct nplx_process *process
);

void nplx_process_stdout_stream_destroy(
	nplx_process_input_stream_t *stream
);

/* process_stderr_stream */

void nplx_process_stderr_stream_init(
	nplx_process_input_stream_t *stream,
	int fd,
	struct nplx_process *process
);

void nplx_process_stderr_stream_destroy(
	nplx_process_input_stream_t *stream
);

#endif /* NETPLEX_STREAM_H */

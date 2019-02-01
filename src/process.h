#ifndef NETPLEX_PROCESS_H
#define NETPLEX_PROCESS_H

#include <unistd.h>
#include <sys/types.h>

#include "stream.h"

typedef struct nplx_process {
	nplx_poolable_t poolable;
	pid_t pid;
	nplx_process_output_stream_t *to_stdin;
	nplx_process_input_stream_t *from_stdout;
	nplx_process_input_stream_t *from_stderr;
} nplx_process_t;

void nplx_process_init(
	nplx_process_t *process,
	pid_t pid
);

int nplx_process_deep_init(
	nplx_process_t *process,
	pid_t pid,
	int stdin_fd,
	int stdout_fd,
	int stderr_fd
);

void nplx_process_destroy(
	nplx_process_t *process
);

#endif /* NETPLEX_PROCESS_H */

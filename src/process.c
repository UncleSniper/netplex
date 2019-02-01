#define _DEFAULT_SOURCE
#include <errno.h>
#include <stdlib.h>

#include "process.h"

void nplx_process_init(
	nplx_process_t *process,
	pid_t pid
) {
	process->pid = pid;
	process->to_stdin = NULL;
	process->from_stdout = NULL;
	process->from_stderr = NULL;
}

int nplx_process_deep_init(
	nplx_process_t *process,
	pid_t pid,
	int stdin_fd,
	int stdout_fd,
	int stderr_fd
) {
	nplx_process_output_stream_t *to_stdin;
	nplx_process_input_stream_t *from_stdout, *from_stderr;
	if(stdin_fd == -1)
		to_stdin = NULL;
	else {
		to_stdin = (nplx_process_output_stream_t*)malloc(sizeof(nplx_process_output_stream_t));
		if(!to_stdin)
			return ENOMEM;
	}
	if(stdout_fd == -1)
		from_stdout = NULL;
	else {
		from_stdout = (nplx_process_input_stream_t*)malloc(sizeof(nplx_process_input_stream_t));
		if(!from_stdout) {
			if(to_stdin)
				free(to_stdin);
			return ENOMEM;
		}
	}
	if(stderr_fd == -1)
		from_stderr = NULL;
	else {
		from_stderr = (nplx_process_input_stream_t*)malloc(sizeof(nplx_process_input_stream_t));
		if(!from_stderr) {
			if(to_stdin)
				free(to_stdin);
			if(from_stdout)
				free(from_stdout);
			return ENOMEM;
		}
	}
	process->pid = pid;
	if(to_stdin)
		nplx_process_stdin_stream_init(to_stdin, stdin_fd, process);
	else
		process->to_stdin = NULL;
	if(from_stdout)
		nplx_process_stdout_stream_init(from_stdout, stdout_fd, process);
	else
		process->from_stdout = NULL;
	if(from_stderr)
		nplx_process_stderr_stream_init(from_stderr, stderr_fd, process);
	else
		process->from_stderr = NULL;
	return 0;
}

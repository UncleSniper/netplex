#ifndef NETPLEX_POLL_H
#define NETPLEX_POLL_H

#include <poll.h>
#include <errno.h>
#include <stdint.h>

typedef struct nplx_poll_set {
	nfds_t count;
	nfds_t capacity;
	struct pollfd *fds;
} nplx_poll_set_t;

typedef struct pollfd *nplx_poll_fillptr_t;

typedef struct nplx_poll_getptr {
	const nplx_poll_set_t *set;
	nfds_t index;
} nplx_poll_getptr_t;

void nplx_poll_set_init(
	nplx_poll_set_t *set
);

void nplx_poll_set_destroy(
	nplx_poll_set_t *set
);

int nplx_poll_set_begin_put(
	nplx_poll_set_t *set,
	uint32_t count,
	nplx_poll_fillptr_t *filler
);

inline void nplx_poll_set_put(
	nplx_poll_fillptr_t *filler,
	int fd
) {
	(*filler)->fd = fd;
	(*filler)->events = POLLIN | POLLRDHUP;
	++*filler;
}

int nplx_poll_set_append(
	nplx_poll_set_t *set,
	int fd
);

inline void nplx_poll_set_begin_get(
	const nplx_poll_set_t *set,
	nplx_poll_getptr_t *getter
) {
	getter->set = set;
	getter->index = (nfds_t)0u;
}

inline int nplx_poll_set_get(
	nplx_poll_getptr_t *getter
) {
	struct pollfd *fds;
	fds = getter->set->fds;
	for(; getter->index < getter->set->count; ++getter->index) {
		if(fds[getter->index].revents & (POLLIN | POLLRDHUP | POLLNVAL | POLLERR | POLLHUP))
			return fds[getter->index++].fd;
	}
	return -1;
}

inline int nplx_poll_set_poll(
	nplx_poll_set_t *set
) {
	if(poll(set->fds, set->count, -1) == -1)
		return errno;
	return 0;
}

#endif /* NETPLEX_POLL_H */

#include <stdlib.h>

#include "poll.h"

void nplx_poll_set_init(
	nplx_poll_set_t *set
) {
	set->count = set->capacity = (nfds_t)0u;
	set->fds = NULL;
}

void nplx_poll_set_destroy(
	nplx_poll_set_t *set
) {
	if(set->capacity)
		free(set->fds);
}

int nplx_poll_set_begin_put(
	nplx_poll_set_t *set,
	uint32_t count,
	nplx_poll_fillptr_t *filler
) {
	struct pollfd *new_fds;
	nfds_t new_capacity, try_capacity;
	new_capacity = (nfds_t)count;
	if((uint32_t)new_capacity != count)
		return EDOM;
	if(new_capacity > set->capacity) {
		try_capacity = (nfds_t)(set->capacity * (nfds_t)2u);
		if(try_capacity > new_capacity)
			new_capacity = try_capacity;
		new_fds = (struct pollfd*)malloc((size_t)new_capacity * sizeof(struct pollfd));
		if(!new_fds)
			return ENOMEM;
		if(set->capacity)
			free(set->fds);
		set->fds = new_fds;
		set->capacity = new_capacity;
		new_capacity = (nfds_t)count;
	}
	set->count = new_capacity;
	*filler = set->fds;
	return 0;
}

int nplx_poll_set_append(
	nplx_poll_set_t *set,
	int fd
) {
	struct pollfd *new_fds, *element;
	nfds_t new_count, new_capacity;
	new_count = (nfds_t)(set->count + (nfds_t)1u);
	if(new_count < set->count)
		return EMFILE;
	if(new_count > set->capacity) {
		new_capacity = (nfds_t)(set->capacity * (nfds_t)2u);
		if(new_capacity < new_count)
			new_capacity = new_count;
		new_fds = (struct pollfd*)malloc((size_t)new_capacity * sizeof(struct pollfd));
		if(!new_fds)
			return ENOMEM;
		if(set->capacity)
			free(set->fds);
		set->fds = new_fds;
		set->capacity = new_capacity;
	}
	element = set->fds + set->count;
	element->fd = fd;
	element->events = POLLIN | POLLRDHUP;
	set->count = new_count;
	return 0;
}

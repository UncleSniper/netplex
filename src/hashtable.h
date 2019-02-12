#ifndef NETPLEX_HASHTABLE_H
#define NETPLEX_HASHTABLE_H

#include <stdint.h>

#include "error.h"

struct nplx_driver;

typedef nplx_error_pump_result_t (*nplx_fd_activity_handler_cb)(
	int fd,
	uint32_t id,
	struct nplx_driver *driver,
	nplx_error_t *error
);

typedef struct nplx_fd_hashtable_node {
	int fd;
	uint32_t id;
	nplx_fd_activity_handler_cb handler;
	struct nplx_fd_hashtable_node *next;
} nplx_fd_hashtable_node_t;

typedef struct nplx_fd_hashtable_line {
	nplx_fd_hashtable_node_t *first_node;
	struct nplx_fd_hashtable_line *next_line;
	struct nplx_fd_hashtable_line *prev_line;
} nplx_fd_hashtable_line_t;

typedef struct nplx_fd_hashtable {
	uint32_t modulus;
	uint32_t size;
	nplx_fd_hashtable_line_t *table;
	nplx_fd_hashtable_line_t *first_line;
} nplx_fd_hashtable_t;

typedef struct nplx_fd_hashtable_iterator {
	nplx_fd_hashtable_line_t *line;
	nplx_fd_hashtable_node_t *node;
} nplx_fd_hashtable_iterator_t;

int nplx_fd_hashtable_init(
	nplx_fd_hashtable_t *table,
	uint32_t modulus
);

void nplx_fd_hashtable_destroy(
	nplx_fd_hashtable_t *table
);

int nplx_fd_hashtable_put(
	nplx_fd_hashtable_t *table,
	int fd,
	uint32_t id,
	nplx_fd_activity_handler_cb handler
);

int nplx_fd_hashtable_erase(
	nplx_fd_hashtable_t *table,
	int fd
);

nplx_error_pump_result_t nplx_fd_hashtable_dispatch(
	const nplx_fd_hashtable_t *table,
	int fd,
	struct nplx_driver *driver,
	nplx_error_t *error
);

inline void nplx_fd_hashtable_iter_begin(
	const nplx_fd_hashtable_t *table,
	nplx_fd_hashtable_iterator_t *iterator
) {
	iterator->line = table->first_line;
	iterator->node = table->first_line ? table->first_line->first_node : NULL;
}

inline const nplx_fd_hashtable_node_t *nplx_fd_hashtable_iter_next(
	nplx_fd_hashtable_iterator_t *iterator
) {
	nplx_fd_hashtable_node_t *node;
	node = iterator->node;
	if(node) {
		iterator->node = node->next;
		if(!iterator->node) {
			iterator->line = iterator->line->next_line;
			iterator->node = iterator->line ? iterator->line->first_node : NULL;
		}
	}
	return node;
}

#endif /* NETPLEX_HASHTABLE_H */

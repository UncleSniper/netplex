#ifndef NETPLEX_HASHTABLE_H
#define NETPLEX_HASHTABLE_H

#include <stdint.h>

struct nplx_driver;

typedef int (*nplx_fd_activity_handler_cb)(
	int fd,
	uint32_t id,
	struct nplx_driver *driver
);

typedef struct nplx_fd_hashtable_node {
	int fd;
	uint32_t id;
	nplx_fd_activity_handler_cb handler;
	struct nplx_fd_hashtable_node *next;
} nplx_fd_hashtable_node_t;

typedef struct nplx_fd_hashtable {
	uint32_t modulus;
	nplx_fd_hashtable_node_t **table;
} nplx_fd_hashtable_t;

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

int nplx_fd_hashtable_dispatch(
	const nplx_fd_hashtable_t *table,
	int fd,
	struct nplx_driver *driver,
	int *error_code
);

#endif /* NETPLEX_HASHTABLE_H */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

int nplx_fd_hashtable_init(
	nplx_fd_hashtable_t *table,
	uint32_t modulus
) {
	size_t tblsize;
	tblsize = (size_t)((size_t)modulus * sizeof(nplx_fd_hashtable_node_t*));
	table->table = (nplx_fd_hashtable_node_t**)malloc(tblsize);
	if(!table->table)
		return ENOMEM;
	memset(table->table, 0, tblsize);
	table->modulus = modulus;
	return 0;
}

void nplx_fd_hashtable_destroy(
	nplx_fd_hashtable_t *table
) {
	uint32_t u;
	nplx_fd_hashtable_node_t *node, *next;
	for(u = (uint32_t)0u; u < table->modulus; ++u) {
		for(node = table->table[u]; node; node = next) {
			next = node->next;
			free(node);
		}
	}
	free(table->table);
}

int nplx_fd_hashtable_put(
	nplx_fd_hashtable_t *table,
	int fd,
	uint32_t id,
	nplx_fd_activity_handler_cb handler
) {
	uint32_t hash;
	nplx_fd_hashtable_node_t *node;
	hash = (uint32_t)((uint32_t)fd % table->modulus);
	for(node = table->table[hash]; node; node = node->next) {
		if(node->fd == fd) {
			node->id = id;
			node->handler = handler;
			return 0;
		}
	}
	node = (nplx_fd_hashtable_node_t*)malloc(sizeof(nplx_fd_hashtable_node_t));
	if(!node)
		return ENOMEM;
	node->fd = fd;
	node->id = id;
	node->handler = handler;
	node->next = table->table[hash];
	table->table[hash] = node;
	return 0;
}

int nplx_fd_hashtable_erase(
	nplx_fd_hashtable_t *table,
	int fd
) {
	uint32_t hash;
	nplx_fd_hashtable_node_t *node, **prev;
	hash = (uint32_t)((uint32_t)fd % table->modulus);
	prev = table->table + hash;
	for(node = *prev; node; node = *prev) {
		if(node->fd == fd) {
			*prev = node->next;
			free(node);
			return 0;
		}
		prev = &node->next;
	}
	return ENOENT;
}

int nplx_fd_hashtable_dispatch(
	const nplx_fd_hashtable_t *table,
	int fd,
	struct nplx_driver *driver,
	int *error_code
) {
	uint32_t hash;
	nplx_fd_hashtable_node_t *node;
	hash = (uint32_t)((uint32_t)fd % table->modulus);
	for(node = table->table[hash]; node; node = node->next) {
		if(node->fd == fd) {
			*error_code = node->handler(fd, node->id, driver);
			return 0;
		}
	}
	return ENOENT;
}

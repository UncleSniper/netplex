#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

int nplx_fd_hashtable_init(
	nplx_fd_hashtable_t *table,
	uint32_t modulus
) {
	size_t tblsize;
	tblsize = (size_t)((size_t)modulus * sizeof(nplx_fd_hashtable_line_t));
	table->table = (nplx_fd_hashtable_line_t*)malloc(tblsize);
	if(!table->table)
		return ENOMEM;
	memset(table->table, 0, tblsize);
	table->modulus = modulus;
	table->size = (uint32_t)0u;
	table->first_line = NULL;
	return 0;
}

void nplx_fd_hashtable_destroy(
	nplx_fd_hashtable_t *table
) {
	nplx_fd_hashtable_line_t *line;
	nplx_fd_hashtable_node_t *node, *next;
	for(line = table->first_line; line; line = line->next_line) {
		for(node = line->first_node; node; node = next) {
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
	nplx_fd_hashtable_line_t *line;
	nplx_fd_hashtable_node_t *node;
	hash = (uint32_t)((uint32_t)fd % table->modulus);
	line = table->table + hash;
	for(node = line->first_node; node; node = node->next) {
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
	if(!line->first_node) {
		line->prev_line = NULL;
		line->next_line = table->first_line;
		if(line->next_line)
			line->next_line->prev_line = line;
		table->first_line = line;
	}
	node->next = line->first_node;
	line->first_node = node;
	++table->size;
	return 0;
}

int nplx_fd_hashtable_erase(
	nplx_fd_hashtable_t *table,
	int fd
) {
	uint32_t hash;
	nplx_fd_hashtable_line_t *line;
	nplx_fd_hashtable_node_t *node, **prev;
	hash = (uint32_t)((uint32_t)fd % table->modulus);
	line = table->table + hash;
	prev = &line->first_node;
	for(node = *prev; node; node = *prev) {
		if(node->fd == fd) {
			*prev = node->next;
			free(node);
			if(!line->first_node) {
				if(line->prev_line)
					line->prev_line->next_line = line->next_line;
				else
					table->first_line = line->next_line;
				if(line->next_line)
					line->next_line->prev_line = line->prev_line;
			}
			--table->size;
			return 0;
		}
		prev = &node->next;
	}
	return ENOENT;
}

nplx_error_pump_result_t nplx_fd_hashtable_dispatch(
	const nplx_fd_hashtable_t *table,
	int fd,
	struct nplx_driver *driver,
	nplx_error_t *error
) {
	uint32_t hash;
	nplx_fd_hashtable_node_t *node;
	hash = (uint32_t)((uint32_t)fd % table->modulus);
	for(node = table->table[hash].first_node; node; node = node->next) {
		if(node->fd == fd)
			return node->handler(fd, node->id, driver, error);
	}
	error->type = NPLX_ERR_FD_DISPATCH;
	error->error_code = EBADF;
	return NPLX_DRVR_PUMP_FATAL_ERROR;
}

#ifndef NETPLEX_POOL_H
#define NETPLEX_POOL_H

#include <stdint.h>
#include <stdlib.h>

struct nplx_poolable;

typedef void (*nplx_poolable_destroy_cb)(
	struct nplx_poolable *poolable
);

typedef struct nplx_poolable_vtable {
	nplx_poolable_destroy_cb destroy;
} nplx_poolable_vtable_t;

typedef struct nplx_poolable {
	const nplx_poolable_vtable_t *vtable;
} nplx_poolable_t;

typedef struct nplx_pool_chunk {
	uint32_t chunk_index;
	uint32_t occupied_bitmap;
	struct nplx_pool_chunk *next_free;
	nplx_poolable_t *elements[32];
} nplx_pool_chunk_t;

typedef struct nplx_pool {
	uint32_t size;
	nplx_pool_chunk_t **chunks;
	uint32_t chunk_count;
	uint32_t chunk_capacity;
	nplx_pool_chunk_t *first_free;
} nplx_pool_t;

void nplx_pool_init(
	nplx_pool_t *pool
);

void nplx_pool_destroy(
	nplx_pool_t *pool
);

int nplx_pool_alloc(
	nplx_pool_t *pool,
	nplx_poolable_t *poolable,
	uint32_t *id
);

int nplx_pool_free(
	nplx_pool_t *pool,
	uint32_t id
);

int nplx_pool_get(
	const nplx_pool_t *pool,
	uint32_t id,
	nplx_poolable_t **destination
);

void nplx_null_poolable_destroy(
	nplx_poolable_t *poolable
);

inline void nplx_poolable_destroy(
	nplx_poolable_t *poolable
) {
	poolable->vtable->destroy(poolable);
}

#endif /* NETPLEX_POOL_H */

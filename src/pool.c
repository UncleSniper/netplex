#define _DEFAULT_SOURCE
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

#include "api.h"
#include "pool.h"

void nplx_pool_init(
	nplx_pool_t *pool
) {
	memset(pool, 0, sizeof(nplx_pool_t));
}

void nplx_pool_destroy(
	nplx_pool_t *pool
) {
	uint32_t chunk_index;
	nplx_pool_chunk_t *chunk;
	unsigned element_index;
	nplx_poolable_t *element;
	for(chunk_index = (uint32_t)0u; chunk_index < pool->chunk_count; ++chunk_index) {
		chunk = pool->chunks[chunk_index];
		for(element_index = 0u; element_index < 32u; ++element_index) {
			if(chunk->occupied_bitmap & (uint32_t)((uint32_t)1u << element_index)) {
				element = chunk->elements[element_index];
				element->vtable->destroy(element);
			}
		}
		free(chunk);
	}
	if(pool->chunk_capacity)
		free(pool->chunks);
}

int nplx_pool_alloc(
	nplx_pool_t *pool,
	nplx_poolable_t *poolable,
	uint32_t *id
) {
	nplx_pool_chunk_t *chunk;
	unsigned exponent;
	uint32_t new_chunk_capacity;
	nplx_pool_chunk_t ** new_chunks;
	if((uint32_t)!(uint32_t)(pool->size + (uint32_t)1u))
		return EOVERFLOW;
	if(pool->first_free) {
		/* use existing chunk */
		chunk = pool->first_free;
#ifndef __GNUC__
#error Alas, compiler independent bit scanning is not implemented at this time.
#endif
#if INT_MAX != 2147483647
#error Alas, allocation size independent bit scanning is not implemented at this time.
#endif
		exponent = __builtin_ctz((unsigned)~chunk->occupied_bitmap);
		chunk->occupied_bitmap |= (uint32_t)((uint32_t)1u << exponent);
		chunk->elements[exponent] = poolable;
		if(!(uint32_t)~chunk->occupied_bitmap)
			pool->first_free = chunk->next_free;
		*id = (uint32_t)(chunk->chunk_index * (uint32_t)32u + (uint32_t)exponent);
	}
	else {
		/* create new chunk */
		if(pool->chunk_count == pool->chunk_capacity) {
			new_chunk_capacity = pool->chunk_capacity ? (uint32_t)(pool->chunk_capacity * 2u) : (uint32_t)2u;
			new_chunks = (nplx_pool_chunk_t**)malloc((size_t)((size_t)pool->chunk_capacity
					* sizeof(nplx_pool_chunk_t*)));
			if(!new_chunks)
				return ENOMEM;
			if(pool->chunk_count) {
				memcpy(new_chunks, pool->chunks, (size_t)((size_t)pool->chunk_count * sizeof(nplx_pool_chunk_t*)));
				free(pool->chunks);
			}
			pool->chunks = new_chunks;
			pool->chunk_capacity = new_chunk_capacity;
		}
		chunk = (nplx_pool_chunk_t*)malloc(sizeof(nplx_pool_chunk_t));
		if(!chunk)
			return ENOMEM;
		chunk->chunk_index = pool->chunk_count;
		chunk->occupied_bitmap = (uint32_t)1u;
		chunk->next_free = pool->first_free;
		*chunk->elements = poolable;
		pool->chunks[pool->chunk_count] = chunk;
		pool->first_free = chunk;
		++pool->chunk_count;
		*id = (uint32_t)(chunk->chunk_index * (uint32_t)32u);
	}
	++pool->size;
	return 0;
}

int nplx_pool_free(
	nplx_pool_t *pool,
	uint32_t id
) {
	uint32_t chunk_index, mask;
	unsigned exponent;
	nplx_pool_chunk_t *chunk;
	nplx_poolable_t *poolable;
	chunk_index = (uint32_t)(id / (uint32_t)32u);
	if(chunk_index >= pool->chunk_count)
		return EDOM;
	exponent = (unsigned)id % 32u;
	chunk = pool->chunks[chunk_index];
	mask = (uint32_t)((uint32_t)1u << exponent);
	if(!(uint32_t)(chunk->occupied_bitmap & mask))
		return EDOM;
	poolable = chunk->elements[exponent];
	poolable->vtable->destroy(poolable);
	if(!(uint32_t)~chunk->occupied_bitmap) {
		chunk->next_free = pool->first_free;
		pool->first_free = chunk;
	}
	chunk->occupied_bitmap &= (uint32_t)~mask;
	--pool->size;
	return 0;
}

int nplx_pool_get(
	const nplx_pool_t *pool,
	uint32_t id,
	nplx_poolable_t **destination
) {
	uint32_t chunk_index, mask;
	unsigned exponent;
	nplx_pool_chunk_t *chunk;
	chunk_index = (uint32_t)(id / (uint32_t)32u);
	if(chunk_index >= pool->chunk_count)
		return EDOM;
	exponent = (unsigned)id % 32u;
	chunk = pool->chunks[chunk_index];
	mask = (uint32_t)((uint32_t)1u << exponent);
	if(!(uint32_t)(chunk->occupied_bitmap & mask))
		return EDOM;
	*destination = chunk->elements[exponent];
	return 0;
}

void nplx_null_poolable_destroy(
	NETPLEX_UNUSED(nplx_poolable_t*, poolable)
) {
	/* nothing to do */
}

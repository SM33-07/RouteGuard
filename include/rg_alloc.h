#ifndef RG_ALLOC_H
#define RG_ALLOC_H

#include <stddef.h>

/**
 * @file rg_alloc.h
 * @brief Memory allocation wrappers with live allocation tracking and test fault injection.
 *
 * All dynamic memory in RouteGuard must be allocated and freed through these wrappers.
 */

void *rg_malloc(size_t size);
void *rg_calloc(size_t num, size_t size);
void rg_free(void *ptr);

/**
 * @brief Returns the count of currently live allocations.
 */
size_t rg_alloc_active_count(void);

/**
 * @brief Resets allocation statistics (active counter remains accurate to live pointers).
 */
void rg_alloc_reset_stats(void);

/**
 * @brief Simulates out-of-memory error by failing the N-th subsequent allocation.
 * @param count Number of successful allocations before returning NULL. Pass -1 to disable.
 */
void rg_alloc_fail_after(int count);

#endif /* RG_ALLOC_H */

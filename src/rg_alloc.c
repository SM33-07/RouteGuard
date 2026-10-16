#include "rg_alloc.h"
#include <stdlib.h>

static size_t g_active_allocations = 0;
static int g_fail_after = -1;

void *rg_malloc(size_t size) {
    if (g_fail_after == 0) {
        g_fail_after = -1;
        return NULL;
    }
    if (g_fail_after > 0) {
        g_fail_after--;
    }
    void *ptr = malloc(size);
    if (ptr != NULL) {
        g_active_allocations++;
    }
    return ptr;
}

void *rg_calloc(size_t num, size_t size) {
    if (g_fail_after == 0) {
        g_fail_after = -1;
        return NULL;
    }
    if (g_fail_after > 0) {
        g_fail_after--;
    }
    void *ptr = calloc(num, size);
    if (ptr != NULL) {
        g_active_allocations++;
    }
    return ptr;
}

void rg_free(void *ptr) {
    if (ptr != NULL) {
        if (g_active_allocations > 0) {
            g_active_allocations--;
        }
        free(ptr);
    }
}

size_t rg_alloc_active_count(void) {
    return g_active_allocations;
}

void rg_alloc_reset_stats(void) {
    g_fail_after = -1;
}

void rg_alloc_fail_after(int count) {
    g_fail_after = count;
}

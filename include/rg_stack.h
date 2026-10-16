#ifndef RG_STACK_H
#define RG_STACK_H

#include "rg_graph.h"
#include <stdbool.h>

/**
 * @file rg_stack.h
 * @brief Fixed-capacity array-based LIFO stack for iterative DFS traversal.
 */

typedef struct {
    int *data;
    int top;        /* Index of next available slot (top == size) */
    int capacity;
} RgStack;

RgStatus rg_stack_init(RgStack *s, int capacity);
void rg_stack_destroy(RgStack *s);
bool rg_stack_is_empty(const RgStack *s);
bool rg_stack_is_full(const RgStack *s);
RgStatus rg_stack_push(RgStack *s, int value);
RgStatus rg_stack_pop(RgStack *s, int *out_value);
RgStatus rg_stack_peek(const RgStack *s, int *out_value);

#endif /* RG_STACK_H */

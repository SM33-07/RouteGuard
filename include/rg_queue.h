#ifndef RG_QUEUE_H
#define RG_QUEUE_H

#include "rg_graph.h"
#include <stdbool.h>

/**
 * @file rg_queue.h
 * @brief Fixed-capacity array-based FIFO queue for BFS traversal.
 */

typedef struct {
    int *data;
    int front;
    int rear;
    int size;
    int capacity;
} RgQueue;

RgStatus rg_queue_init(RgQueue *q, int capacity);
void rg_queue_destroy(RgQueue *q);
bool rg_queue_is_empty(const RgQueue *q);
bool rg_queue_is_full(const RgQueue *q);
RgStatus rg_queue_enqueue(RgQueue *q, int value);
RgStatus rg_queue_dequeue(RgQueue *q, int *out_value);

#endif /* RG_QUEUE_H */

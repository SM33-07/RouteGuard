#include "rg_queue.h"
#include "rg_alloc.h"

RgStatus rg_queue_init(RgQueue *q, int capacity) {
    if (q == NULL || capacity <= 0) {
        return RG_ERR_INVALID_PARAM;
    }
    q->data = rg_malloc((size_t)capacity * sizeof(int));
    if (q->data == NULL) {
        return RG_ERR_ALLOC;
    }
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = capacity;
    return RG_OK;
}

void rg_queue_destroy(RgQueue *q) {
    if (q != NULL) {
        rg_free(q->data);
        q->data = NULL;
        q->front = 0;
        q->rear = 0;
        q->size = 0;
        q->capacity = 0;
    }
}

bool rg_queue_is_empty(const RgQueue *q) {
    return (q == NULL || q->size == 0);
}

bool rg_queue_is_full(const RgQueue *q) {
    return (q != NULL && q->size >= q->capacity);
}

RgStatus rg_queue_enqueue(RgQueue *q, int value) {
    if (q == NULL || rg_queue_is_full(q)) {
        return RG_ERR_INVALID_PARAM;
    }
    q->data[q->rear] = value;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
    return RG_OK;
}

RgStatus rg_queue_dequeue(RgQueue *q, int *out_value) {
    if (q == NULL || out_value == NULL || rg_queue_is_empty(q)) {
        return RG_ERR_INVALID_PARAM;
    }
    *out_value = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return RG_OK;
}

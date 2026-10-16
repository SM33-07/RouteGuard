#include "rg_stack.h"
#include "rg_alloc.h"

RgStatus rg_stack_init(RgStack *s, int capacity) {
    if (s == NULL || capacity <= 0) {
        return RG_ERR_INVALID_PARAM;
    }
    s->data = rg_malloc((size_t)capacity * sizeof(int));
    if (s->data == NULL) {
        return RG_ERR_ALLOC;
    }
    s->top = 0;
    s->capacity = capacity;
    return RG_OK;
}

void rg_stack_destroy(RgStack *s) {
    if (s != NULL) {
        rg_free(s->data);
        s->data = NULL;
        s->top = 0;
        s->capacity = 0;
    }
}

bool rg_stack_is_empty(const RgStack *s) {
    return (s == NULL || s->top == 0);
}

bool rg_stack_is_full(const RgStack *s) {
    return (s != NULL && s->top >= s->capacity);
}

RgStatus rg_stack_push(RgStack *s, int value) {
    if (s == NULL || rg_stack_is_full(s)) {
        return RG_ERR_INVALID_PARAM;
    }
    s->data[s->top++] = value;
    return RG_OK;
}

RgStatus rg_stack_pop(RgStack *s, int *out_value) {
    if (s == NULL || out_value == NULL || rg_stack_is_empty(s)) {
        return RG_ERR_INVALID_PARAM;
    }
    *out_value = s->data[--s->top];
    return RG_OK;
}

RgStatus rg_stack_peek(const RgStack *s, int *out_value) {
    if (s == NULL || out_value == NULL || rg_stack_is_empty(s)) {
        return RG_ERR_INVALID_PARAM;
    }
    *out_value = s->data[s->top - 1];
    return RG_OK;
}

#include "ringbuf.h"

#include <string.h>

void ringbuf_init(ringbuf_t* ringbuf, int data_size, int elem_size)
{
    ringbuf->data_size = data_size - (data_size % elem_size);
    ringbuf->elem_size = elem_size;
    ringbuf->count = 0;
    ringbuf->back = ringbuf->front = ringbuf->data;
}

int ringbuf_has_next(ringbuf_t* ringbuf)
{
    return (ringbuf->count > 0);
}

void* ringbuf_next(ringbuf_t* ringbuf, void* elem)
{
    if ((elem + ringbuf->elem_size) >= (ringbuf->data + ringbuf->data_size)) {
        return ringbuf->data;
    }
    return elem + ringbuf->elem_size;
}

void* ringbuf_previous(ringbuf_t* ringbuf, void* elem)
{
    if (elem == ringbuf->data) {
        return ringbuf->data + ringbuf->data_size - ringbuf->elem_size;
    }
    return elem - ringbuf->elem_size;
}

void* ringbuf_emplace_back(ringbuf_t* ringbuf)
{
    ringbuf->back = ringbuf_next(ringbuf, ringbuf->back);
    memset(ringbuf->back, 0, ringbuf->elem_size);
    ringbuf->count++;
    return ringbuf->back;
}

int ringbuf_push_back(ringbuf_t* ringbuf, const void* elem)
{
    ringbuf_emplace_back(ringbuf);
    memcpy(ringbuf->back, elem, ringbuf->elem_size);
}

void* ringbuf_pop_front(ringbuf_t* ringbuf)
{
    void* elem;

    if (ringbuf_has_next(ringbuf) == 0) {
        return NULL;
    }

    elem = ringbuf->front;
    ringbuf->count--;

    if (ringbuf->count > 0) {
        ringbuf->front = ringbuf_next(ringbuf, ringbuf->front);
    }
    return elem;
}

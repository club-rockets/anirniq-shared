#ifndef RINGBUF_H_
#define RINGBUF_H_

#include <stdint.h>

#define ringbuf_make(name, elem, size) \
    void* ringbuf_##name[sizeof(elem) * size] = { 0 }; \
    ringbuf_t name = { .data = ringbuf_##name }; \
    ringbuf_init(&name, sizeof(elem) * size, sizeof(elem));

typedef struct {
    void* data;
    int data_size;
    int elem_size;
    int count;

    void* front;
    void* back;
} ringbuf_t;

/** ringbuf_init
 * Initialize the ringbuffer. Unless you know what you are doing, you should not
 * call it directly, use ringbuf_make instead
 * 
 * @param  ringbuf  allocated ringbuffer structure, with .data set
 * @param  data_size  total size (in bytes) of the .data buffer
 * @param  elem_size  size of each element of the ringbuffer (in bytes)
*/
void ringbuf_init(ringbuf_t* ringbuf, int data_size, int elem_size);

/** ringbuf_has_next
 * Return 1 if the ringbuffer has data available
 * 
 * @param  ringbuf  ringbuffer to check
 * 
 * @return  1 if the a next element is available, 0 otherwise
 */
int ringbuf_has_next(ringbuf_t* ringbuf);

/** ringbuf_next
 * Return the next element after 'elem', wrapping around if necessary.
 * 
 * @param  ringbuf  ringbuffer to check
 * @param  elem  pointer an element in the ringbuffer
 * 
 * @return  element before elem
 */
void* ringbuf_next(ringbuf_t* ringbuf, void* elem);

/** ringbuf_previous
 * Return the next element after 'elem', wrapping around if necessary.
 * 
 * @param  ringbuf  ringbuffer to check
 * @param  elem  pointer an element in the ringbuffer
 * 
 * @return  element before elem
 */
void* ringbuf_previous(ringbuf_t* ringbuf, void* elem);

/** ringbuf_emplace_back
 * Construct element in place
 * 
 * @param  ringbuf  ringbuffer to check
 * @return  pointer to allocated element at the back of the queue
 */
void* ringbuf_emplace_back(ringbuf_t* ringbuf);

/** ringbuf_push_back
 * Copy element at the back of the buffer
 * 
 * @param  ringbuf  ringbuffer to check
 * @param  elem  element to copy
 * 
 * @return  0
 */
int ringbuf_push_back(ringbuf_t* ringbuf, const void* elem);

/** ringbuf_pop_front
 * Pop front element
 * 
 * @param  ringbuf  ringbuffer to check
 * @return  element at the front of the store
 */
void* ringbuf_pop_front(ringbuf_t* ringbuf);

#endif
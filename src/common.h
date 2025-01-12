#pragma once
#include "stdlib.h"

typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct
{
    uint x;
    uint y;
} Pos_t;

#define pos(x, y) (Pos_t){x, y}

int min(int a, int b);

uint umin(uint a, uint b);

int max(int a, int b);

uint umax(uint a, uint b);

int clamp(int min, int x, int max);

uint uclamp(uint min, uint x, uint max);

/// DYNAMIC ARRAYS ///
#define SIZE_INCREASE_ON_REALLOC 2
#define ARRAY_INITIAL_CAPACITY 4

typedef struct {
    void *(*alloc)(size_t bytes, void *context);
    void *(*free)(size_t bytes, void *ptr, void *context);
    void *context;
} Allocator_t;

typedef struct {
    size_t length;
    size_t capacity;
    size_t padding_or_something; // prefer 16-byte alignment
    Allocator_t *a;
} Array_Header_t;

Allocator_t create_allocator(void *(*alloc)(size_t bytes, void *context), void *(*free)(size_t bytes, void *ptr, void *context), void *context) {


#define array(T, alloc) array_init(sizeof(T), ARRAY_INITIAL_CAPACITY, alloc)

#define array_append(arr, v) ( \
    (arr) = array_ensure_capacity(arr, 1, sizeof(v)), \
    (arr)[array_header(arr)->length] = (v), \
    &(arr)[array_header(arr)->length++])

#define array_remove(a, i) do { \
    Array_Header_t *h = array_header(a); \
    if (i == h->length - 1) { \
        h->length -= 1; \
    } else if (h->length > 1) { \
        void *ptr = &a[i]; \
        void *last = &a[h->length - 1]; \
        h->length -= 1; \
        memcpy(ptr, last, sizeof(*a)); \
    } \
} while (0);

#define array_header(arr) ((Array_Header_t *)(arr) - 1)
#define array_length(arr) (array_header(arr)->length)
#define array_capacity(arr) (array_header(arr)->capacity)
#define array_pop_back(a) (array_header(a)->length -= 1)

#include "common.h"
#include "stdlib.h"
#include "string.h"

/// COMMON FUNCTIONS AND UTILITES ///
int min(int a, int b)
{
	return (a < b)? a : b;
}

uint umin(uint a, uint b)
{
	return (a < b)? a : b;
}

int max(int a, int b)
{
	return (a < b)? b : a;
}

uint umax(uint a, uint b)
{
	return (a < b)? b : a;
}

int clamp(int low, int x, int high)
{
	return min(high, max(low, x));
}

uint uclamp(uint low, uint x, uint high)
{
	return umin(high, umax(low, x));
}

/// DYNAMIC ARRAY ///

//SRC: https://bytesbeneath.com/articles/dynamic-arrays-in-c


Allocator_t create_allocator(void *(*alloc)(size_t bytes, void *context), void *(*free)(size_t bytes, void *ptr, void *context), void *context) {
	Allocator_t allocator = {.alloc = alloc, .free = free, .context = context};
	return allocator;
}

void *array_init(size_t item_size, size_t capacity, Allocator_t *a) {
    void *ptr = 0;
    size_t size = item_size * capacity + sizeof(Array_Header_t);
    Array_Header_t *h = a->alloc(size, a->context);

    if (h) {
        h->capacity = capacity;
        h->length = 0;
        h->a = a;
        ptr = h + 1;
    }
	
    return ptr;
}

void *array_ensure_capacity(void *a, size_t item_count, size_t item_size) {
    Array_Header_t *h = array_header(a);
    size_t desired_capacity = h->length + item_count;

    if (h->capacity < desired_capacity) {
        size_t new_capacity = h->capacity * 2;
        while (new_capacity < desired_capacity) {
            new_capacity *= 2;
        }

        size_t new_size = sizeof(Array_Header_t) + new_capacity * item_size;
        Array_Header_t *new_h = h->a->alloc(new_size, h->a->context);

        if (new_h) {
            size_t old_size = sizeof(*h) + h->length * item_size;
            memcpy(new_h, h, old_size);

            if (h->a->free) {
                h->a->free(old_size, h, h->a->context);
            }

            new_h->capacity = new_capacity;
            h = new_h + 1;
        } else {
            h = 0;
        }
    } else { h += 1; }

    return h;
}




// static void array_make_contiguos(void **array, uint len)
// {
//     uint gaps = 0;
//     for (size_t i = 0; i < len; i++)
//     {
//         if (array[i] == NULL) {gaps++; continue;}

//         array[i - gaps] = array[i];
//         array[i] = NULL;
//     }
// }

// /// NOTE: Only works with arrays of addresses
// static void **array_realloc(void **array_ptr, uint current_len, uint increase)
// {
//     uint new_len = current_len + increase;
//     array_ptr = realloc(array_ptr, new_len * sizeof(void *));

//     /// The realloc can fail, TODO: better handling of error than crashing
//     if(array_ptr == NULL) 
//     {
//         printf("\nERROR:\n    Could not reallocate array!");
//         assert(0); // GG
//     }

//     // NULL all new adresses
//     for (size_t i = current_len; i < new_len; i++) array_ptr[i] = NULL;   
//     return array_ptr;    
// }
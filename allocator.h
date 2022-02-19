#ifndef ALLOCATOR_H_GUARD_
#define ALLOCATOR_H_GUARD_

#include "utils.h"
#include <stddef.h>
#include <stdlib.h>

typedef void* allocator_ptr_t;

typedef allocator_ptr_t(*allocator_new_t)(size_t);
typedef allocator_ptr_t(*allocator_copy_t)(allocator_ptr_t);
typedef allocator_ptr_t(*allocator_move_t)(allocator_ptr_t);
typedef void(*allocator_del_t)(allocator_ptr_t);
typedef int(*allocator_eq_t)(allocator_ptr_t, allocator_ptr_t);

typedef void*(*allocator_alloc_t)(allocator_ptr_t, size_t);
typedef void(*allocator_dealloc_t)(allocator_ptr_t, void*, size_t);

struct allocator_traits {
    allocator_new_t         new;
    allocator_copy_t        copy;
    allocator_move_t        move;
    allocator_del_t         del;
    allocator_eq_t          eq;

    allocator_alloc_t       alloc;
    allocator_dealloc_t     dealloc;
};

extern struct allocator_traits default_allocator;

int allocator_traits_eq(struct allocator_traits* lhs,
                        struct allocator_traits* rhs);

#endif

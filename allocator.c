#include "allocator.h"
#include <string.h>

struct default_allocator {
    size_t elem_size;
};

int allocator_traits_eq(struct allocator_traits* lhs,
                        struct allocator_traits* rhs) {
    return !memcmp(lhs, rhs, sizeof(struct allocator_traits));
}

static allocator_ptr_t default_allocator_new(size_t elem_size) {
    struct default_allocator* ret = malloc(sizeof(struct default_allocator));
    ret->elem_size = elem_size;
    return ret;
}

static allocator_ptr_t default_allocator_copy(allocator_ptr_t o) {
    return default_allocator_new(((struct default_allocator*)o)->elem_size);
}

static allocator_ptr_t default_allocator_move(allocator_ptr_t o) {
    return o;
}

static void default_allocator_del(allocator_ptr_t o) {
	free((struct default_allocator*) o);
}

static int default_allocator_eq(allocator_ptr_t l, allocator_ptr_t r) {
    Macro_declare_unused(l);
    Macro_declare_unused(r);

    return 1;
}

static void* default_allocator_alloc(allocator_ptr_t a, size_t n) {
    return malloc(((struct default_allocator*) a)->elem_size * n);
}

static void default_allocator_dealloc(allocator_ptr_t a, void* p, size_t n) {
	Macro_declare_unused(a);
	Macro_declare_unused(n);

    free(p);
}

struct allocator_traits default_allocator = {
    default_allocator_new,      // new
    default_allocator_copy,     // copy
    default_allocator_move,     // move
    default_allocator_del,      // del
    default_allocator_eq,       // eq
    default_allocator_alloc,    // alloc
    default_allocator_dealloc   // dealloc
};

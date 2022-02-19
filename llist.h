#ifndef LLIST_H_GUARD_
#define LLIST_H_GUARD_

#include "lnode.h"
#include "allocator.h"

struct linked_list {
    lnode_s* head;
    lnode_s* tail;

    size_t   elem_size;
    size_t   size;

    struct allocator_traits data_alloc_traits;
    struct allocator_traits node_alloc_traits;
    void*    data_alloc_obj;
    void*    node_alloc_obj;
};

typedef struct linked_list llist_s;

// stdlib-style compare function
typedef int(*cmp_pred_t)(const void*, const void*);
// eq_pred shall return non-zero if the operands are equal
typedef cmp_pred_t eq_pred_t;
// unary predicate
typedef int(*unary_pred_t)(const void*);

// note: NULL in pos parameter indicates the pass-the-end position

int llist_same_type(llist_s* lhs, llist_s* rhs);

void llist_construct(llist_s* lst, size_t elem_size,
            const struct allocator_traits data_alloc_traits,
            const struct allocator_traits node_alloc_traits);
void llist_construct_def(llist_s* lst, size_t elem_size);
void llist_construct_copy(llist_s* lst, const llist_s* other);
void llist_construct_move(llist_s* lst, llist_s* other);

void llist_clear(llist_s* lst);
void llist_destroy(llist_s* lst);

void llist_swap(llist_s* lhs, llist_s* rhs);
void llist_assign(llist_s* lst, const lnode_s* first, const lnode_s* last);
void llist_resize(llist_s* lst, size_t size, const void* data);

void llist_push_back(llist_s* lst, const void* data);
lnode_s* llist_insert(llist_s* lst, lnode_s* pos, const void* data);
lnode_s* llist_insert_range(llist_s* lst, lnode_s* pos,
            const lnode_s* first, const lnode_s* last);
lnode_s* llist_erase(llist_s* lst, lnode_s* pos);
lnode_s* llist_erase_range(llist_s* lst, lnode_s* first, lnode_s* last);

void llist_splice_range(llist_s* lst, lnode_s* pos, llist_s* other,
            lnode_s* first, lnode_s* last);
void llist_splice_list(llist_s* lst, lnode_s* pos, llist_s* other);
void llist_splice(llist_s* lst, lnode_s* pos, llist_s* other, lnode_s* node);

void llist_for_each(llist_s* lst, unary_func_t f);

void llist_reverse(llist_s* lst);
void llist_merge(llist_s* lst, llist_s* other);
void llist_merge_pred(llist_s* lst, llist_s* other, cmp_pred_t cmp);
void llist_remove(llist_s* lst, const void* value);
void llist_remove_pred(llist_s* lst, unary_pred_t pred);
void llist_unique(llist_s* lst);
void llist_unique_pred(llist_s* lst, cmp_pred_t cmp);
void llist_sort(llist_s* lst);
void llist_sort_pred(llist_s* lst, cmp_pred_t cmp);

int llist_empty(const llist_s* lst);
int llist_equal_pred(const llist_s* lhs, const llist_s* rhs, eq_pred_t eq);
int llist_equal(const llist_s* lhs, const llist_s* rhs);
int llist_cmp_pred(const llist_s* lhs, const llist_s* rhs, cmp_pred_t cmp);
int llist_cmp(const llist_s* lhs, const llist_s* rhs);

#endif

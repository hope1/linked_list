#include "llist.h"
#include "utils.h"
#include "compat.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static inline void* llist_impl_alloc_data(llist_s* lst) {
	return lst->data_alloc_traits.alloc(lst->data_alloc_obj, 1);
}

static inline void llist_impl_dealloc_data(llist_s* lst, void* p) {
	lst->data_alloc_traits.dealloc(lst->data_alloc_obj, p, 1);
}

static inline lnode_s* llist_impl_alloc_node(llist_s* lst) {
	return (lnode_s*) lst->node_alloc_traits.alloc(lst->node_alloc_obj, 1);
}

static inline void llist_impl_dealloc_node(llist_s* lst, lnode_s* p) {
	lst->node_alloc_traits.dealloc(lst->node_alloc_obj, p, 1);
}

static inline int llist_impl_allocator_eq(llist_s* lhs, llist_s* rhs) {
	return lhs->node_alloc_traits.eq(lhs->node_alloc_obj, rhs->node_alloc_obj)
	    && lhs->data_alloc_traits.eq(lhs->data_alloc_obj, rhs->data_alloc_obj);
}

static inline void llist_impl_init_data(llist_s* lst, size_t elem_size, 
			const struct allocator_traits data_alloc_traits,
			const struct allocator_traits node_alloc_traits) {
	lst->head       = NULL;
	lst->tail       = NULL;

	lst->elem_size  = elem_size;
	lst->size       = 0;

	lst->data_alloc_traits = data_alloc_traits;
	lst->node_alloc_traits = node_alloc_traits;
}

int llist_same_type(llist_s* lhs, llist_s* rhs) {
	return lhs->elem_size == rhs->elem_size
	    && allocator_traits_eq(&lhs->node_alloc_traits, &rhs->node_alloc_traits)
	    && allocator_traits_eq(&lhs->data_alloc_traits, &rhs->data_alloc_traits);
}

void llist_construct(llist_s* lst, size_t elem_size,
			const struct allocator_traits data_alloc_traits,
			const struct allocator_traits node_alloc_traits) {
	llist_impl_init_data(lst, elem_size, data_alloc_traits, node_alloc_traits);

	lst->data_alloc_obj = data_alloc_traits.new(elem_size);
	lst->node_alloc_obj = node_alloc_traits.new(sizeof(lnode_s));
}

void llist_construct_def(llist_s* lst, size_t elem_size) {
	llist_construct(lst, elem_size, default_allocator, default_allocator);
}

void llist_construct_copy(llist_s* lst, const llist_s* other) {
	llist_impl_init_data(lst, other->elem_size,
		other->data_alloc_traits, other->node_alloc_traits);
	lst->data_alloc_obj = lst->data_alloc_traits.copy(other->data_alloc_obj);
	lst->node_alloc_obj = lst->node_alloc_traits.copy(other->node_alloc_obj);

	llist_assign(lst, other->head, NULL);
}

void llist_construct_move(llist_s* lst, llist_s* other) {
	llist_impl_init_data(lst, other->elem_size,
		other->data_alloc_traits, other->node_alloc_traits);
	lst->data_alloc_obj = lst->data_alloc_traits.move(other->data_alloc_obj);
	lst->node_alloc_obj = lst->node_alloc_traits.move(other->node_alloc_obj);

	llist_splice_list(lst, lst->head, other);
}

void llist_clear(llist_s* lst) {
	llist_erase_range(lst, lst->head, NULL);

	lst->size = 0;
	lst->head = NULL;
	lst->tail = NULL;
}

void llist_destroy(llist_s* lst) {
	llist_clear(lst);
	lst->node_alloc_traits.del(lst->node_alloc_obj);
	lst->data_alloc_traits.del(lst->data_alloc_obj);
}

void llist_swap(llist_s* lhs, llist_s* rhs) {
	assert(llist_same_type(lhs, rhs));

	Macro_util_swap(lnode_s*, lhs->head, rhs->head);
	Macro_util_swap(lnode_s*, lhs->tail, rhs->tail);
	Macro_util_swap(size_t,   lhs->size, rhs->size);
}

void llist_assign(llist_s* lst, const lnode_s* first, const lnode_s* last) {
	llist_clear(lst);
	llist_insert_range(lst, NULL, first, last);
}

void llist_resize(llist_s* lst, size_t size, const void* data) {
	if(lst->size > size) {
		lnode_s* p = lnode_advance(lst->head, size);
		llist_erase_range(lst, p, NULL);
	} else if(lst->size < size) {
		size_t i = size - lst->size;
		for(; i > 0; --i)
			llist_push_back(lst, data);
	}
}

void llist_push_back(llist_s* lst, const void* data) {
	lnode_s* new_node = llist_impl_alloc_node(lst);
	new_node->data    = llist_impl_alloc_data(lst);

	memcpy(new_node->data, data, lst->elem_size);

	if(llist_empty(lst)) {
		lnode_init(new_node);
		lst->head = new_node;
	} else
		lnode_insert_after(lst->tail, new_node);

	lst->tail = new_node;
	++lst->size;
}

lnode_s* llist_insert(llist_s* lst, lnode_s* pos, const void* data) {
	if(pos == NULL) {
		llist_push_back(lst, data);
		return lst->tail;
	} else {
		lnode_s* new_node = llist_impl_alloc_node(lst);
		new_node->data    = llist_impl_alloc_data(lst);

		memcpy(new_node->data, data, lst->elem_size);

		lnode_insert(pos, new_node);

		if(pos == lst->head)
			lst->head = new_node;

		++lst->size;

		return new_node;
	}
}

lnode_s* llist_insert_range(llist_s* lst, lnode_s* pos,
			const lnode_s* first, const lnode_s* last) {
	lnode_s* ret = 0;
	while(first != last) {
		if(!ret)
			ret = llist_insert(lst, pos, first->data);
		first = first->next;
	}
	return ret;
}

lnode_s* llist_erase(llist_s* lst, lnode_s* pos) {
	lnode_s* ret = pos->next;

	if(pos == lst->tail)
		lst->tail = pos->prev;
	if(pos == lst->head)
		lst->head = pos->next;

	lnode_detach(pos);

	llist_impl_dealloc_data(lst, pos->data);
	llist_impl_dealloc_node(lst, pos);

	--lst->size;

	return ret;
}

lnode_s* llist_erase_range(llist_s* lst, lnode_s* first, lnode_s* last) {
	lnode_s* ret = 0;
	while(first != last) {
		first = llist_erase(lst, first);
		if(!ret)
			ret = first;
	}
	return ret;
}

// splice [first, last) in 'other' to position before 'pos'
static inline void llist_impl_splice(llist_s* lst, lnode_s* pos,
			llist_s* other, lnode_s* first, lnode_s* last, size_t count) {
	assert(llist_same_type(lst, other));
	assert(first != NULL);

	if(llist_impl_allocator_eq(lst, other)) { // same allocator, relink
		lnode_s* tail = last == NULL ? other->tail : last->prev;

		if(first == other->head)
			other->head = last;
		if(last == NULL)
			other->tail = first->prev;

		if(llist_empty(lst)) {
			lnode_s node;
			lnode_init(&node);
			lnode_splice_range_after(&node, first, tail);
			lnode_detach(&node);

			lst->head = first;
			lst->tail = tail;
		} else {
			if(pos == lst->head)
				lst->head = first;

			if(pos != NULL)
				lnode_splice_range(pos, first, tail);
			else {
				lnode_splice_range_after(lst->tail, first, tail);
				lst->tail = tail;
			}
		}

		if(lst != other) {
			lst->size   += count;
			other->size -= count;
		}
	} else { // different allocator
		llist_insert_range(lst, pos, first, last);
		llist_erase_range(other, first, last);
	}
}

void llist_splice_range(llist_s* lst, lnode_s* pos,
			llist_s* other, lnode_s* first, lnode_s* last) {
	llist_impl_splice(lst, pos, other, first, last,
		lnode_range_len(first, last ? last->prev : other->tail));
}

void llist_splice_list(llist_s* lst, lnode_s* pos, llist_s* other) {
	assert(lst != other);
	llist_impl_splice(lst, pos, other, other->head, NULL, other->size);
}

void llist_splice(llist_s* lst, lnode_s* pos,
			llist_s* other, lnode_s* node) {
	if(pos != node)
		llist_impl_splice(lst, pos, other, node, node->next, 1);
}

void llist_for_each(llist_s* lst, unary_func_t f) {
	lnode_for_each(lst->head, lst->tail, f);
}

void llist_reverse(llist_s* lst) {
	if(lst->head != NULL && lst->tail != NULL) {
		lnode_reverse(lst->head, lst->tail);
		Macro_util_swap(lnode_s*, lst->head, lst->tail);
	}
}

void llist_merge(llist_s* lst, llist_s* other) {
	lnode_s *p1 = lst->head, *p2 = other->head;

	assert(llist_same_type(lst, other));
	assert(llist_impl_allocator_eq(lst, other));

	if(lst == other)
		return;

	while(p1 != NULL && p2 != NULL) {
		if(memcmp(p1->data, p2->data, lst->elem_size) <= 0)
			p1 = p1->next;
		else {
			lnode_s* next = p2->next;
			llist_splice(lst, p1, other, p2);
			p2 = next;
		}
	}
	if(p2 != NULL)
		llist_splice_range(lst, NULL, other, p2, NULL);

	lst->size += other->size;
	llist_clear(other);
}

void llist_merge_pred(llist_s* lst, llist_s* other, cmp_pred_t cmp) {
	lnode_s *p1 = lst->head, *p2 = other->head;

	assert(llist_same_type(lst, other));
	assert(llist_impl_allocator_eq(lst, other));

	if(lst == other)
		return;

	while(p1 != NULL && p2 != NULL) {
		if(cmp(p1->data, p2->data) <= 0)
			p1 = p1->next;
		else {
			lnode_s* next = p2->next;
			llist_splice(lst, p1, other, p2);
			p2 = next;
		}
	}
	if(p2 != NULL)
		llist_splice_range(lst, NULL, other, p2, NULL);

	lst->size += other->size;
	llist_clear(other);
}

void llist_remove(llist_s* lst, const void* value) {
	lnode_s* p = lst->head;
	while(p != NULL) {
		if(!memcmp(p->data, value, lst->elem_size)) {
			lnode_s* next = p->next;
			llist_erase(lst, p);
			p = next;
		} else
			p = p->next;
	}
}

void llist_remove_pred(llist_s* lst, unary_pred_t pred) {
	lnode_s* p = lst->head;
	while(p != NULL) {
		if(pred(p->data)) {
			lnode_s* next = p->next;
			llist_erase(lst, p);
			p = next;
		} else
			p = p->next;
	}
}

void llist_unique(llist_s* lst) {
	if(lst->head != NULL) {
		lnode_s* p = lst->head->next;
		while(p != NULL) {
			if(!memcmp(p->data, p->prev->data, lst->elem_size)) {
				lnode_s* next = p->next;
				llist_erase(lst, p);
				p = next;
			} else
				p = p->next;
		}
	}
}

void llist_unique_pred(llist_s* lst, eq_pred_t eq) {
	if(lst->head != NULL) {
		lnode_s* p = lst->head->next;
		while(p != NULL) {
			if(eq(p->data, p->prev->data)) {
				lnode_s* next = p->next;
				llist_erase(lst, p);
				p = next;
			} else
				p = p->next;
		}
	}
}

void llist_sort(llist_s* lst) {
	llist_s tmp, bin[64];
	size_t max = 0, cnt;
	// initialize
	llist_construct_def(&tmp, lst->elem_size);
	for(cnt = 0; cnt < 64; ++cnt)
		llist_construct_def(&bin[cnt], lst->elem_size);
	// construct heap
	while(!llist_empty(lst)) {
		llist_splice(&tmp, tmp.head, lst, lst->head);
		for(cnt = 0; cnt < max && !llist_empty(&bin[cnt]); ++cnt) {
			// merge into larger bin
			llist_merge(&bin[cnt], &tmp);
			llist_swap(&bin[cnt], &tmp);
		}
		// place into the current bin
		llist_swap(&bin[cnt], &tmp);
		if(cnt == max)
			++max;
	}
	for(cnt = 1; cnt < max; ++cnt)
		llist_merge(&bin[cnt], &bin[cnt - 1]);
	llist_splice_list(lst, NULL, &bin[max - 1]);
}

void llist_sort_pred(llist_s* lst, cmp_pred_t cmp) {
	llist_s tmp, bin[64];
	size_t max = 0, cnt;
	// initialize
	llist_construct_def(&tmp, lst->elem_size);
	for(cnt = 0; cnt < 64; ++cnt)
		llist_construct_def(&bin[cnt], lst->elem_size);
	// construct heap
	while(!llist_empty(lst)) {
		llist_splice(&tmp, tmp.head, lst, lst->head);
		for(cnt = 0; cnt < max && !llist_empty(&bin[cnt]); ++cnt) {
			// merge into larger bin
			llist_merge_pred(&bin[cnt], &tmp, cmp);
			llist_swap(&bin[cnt], &tmp);
		}
		// place into the current bin
		llist_swap(&bin[cnt], &tmp);
		if(cnt == max)
			++max;
	}
	for(cnt = 1; cnt < max; ++cnt)
		llist_merge_pred(&bin[cnt], &bin[cnt - 1], cmp);
	llist_splice_list(lst, NULL, &bin[max - 1]);
}

int llist_empty(const llist_s* lst) {
	return !lst->size;
}

// return 1 if two lists are equal
int llist_equal_pred(const llist_s* lhs, const llist_s* rhs, eq_pred_t eq) {
	if(lhs->elem_size != rhs->elem_size || lhs->size != rhs->size)
		return 0;
	else {
		lnode_s *p1 = lhs->head, *p2 = rhs->head;
		while(p1 != NULL) {
			if(!eq(p1->data, p2->data))
				return 0;

			p1 = p1->next;
			p2 = p2->next;
		}
	}
	return 1;
}

// use memcmp
int llist_equal(const llist_s* lhs, const llist_s* rhs) {
	if(lhs->elem_size != rhs->elem_size || lhs->size != rhs->size)
		return 0;
	else {
		lnode_s *p1 = lhs->head, *p2 = rhs->head;
		while(p1 != NULL) {
			if(memcmp(p1->data, p2->data, lhs->elem_size))
				return 0;

			p1 = p1->next;
			p2 = p2->next;
		}
	}
	return 1;
}

// compare lexicographically
int llist_cmp_pred(const llist_s* lhs, const llist_s* rhs, cmp_pred_t cmp) {
	lnode_s *p1 = lhs->head, *p2 = rhs->head;
	int ret;

	while(p1 != NULL || p2 != NULL) {
		if(p1 == NULL)
			return -1;
		if(p2 == NULL)
			return 1;

		ret = cmp(p1->data, p2->data);
		if(ret)
			return ret;

		p1 = p1->next;
		p2 = p2->next;
	}
	return 0;
}

// use memcmp
int llist_cmp(const llist_s* lhs, const llist_s* rhs) {
	lnode_s *p1 = lhs->head, *p2 = rhs->head;
	int ret;

	assert(lhs->elem_size == rhs->elem_size);

	while(p1 != NULL || p2 != NULL) {
		if(p1 == NULL)
			return -1;
		if(p2 == NULL)
			return 1;

		ret = memcmp(p1->data, p2->data, lhs->elem_size);
		if(ret)
			return ret;

		p1 = p1->next;
		p2 = p2->next;
	}
	return 0;
}

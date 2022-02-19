#ifndef LNODE_H_GUARD_
#define LNODE_H_GUARD_

#include <stddef.h>
#include "utils.h"
#include "compat.h"

struct linked_list_node {
	struct linked_list_node* prev;
	struct linked_list_node* next;
	void*                    data;
};

typedef struct linked_list_node lnode_s;

typedef void(*unary_func_t)(void*);

// all range are [head, tail]

static inline lnode_s* lnode_head(lnode_s* node) {
	if(node != NULL)
		while(node->prev != NULL)
			node = node->prev;
	return node;
}

static inline lnode_s* lnode_tail(lnode_s* node) {
	if(node != NULL)
		while(node->next != NULL)
			node = node->next;
	return node;
}

static inline void lnode_init(lnode_s* node) {
	node->prev = NULL;
	node->next = NULL;
}

static inline void lnode_insert_range_after(lnode_s* pos, lnode_s* head, lnode_s* tail) {
	head->prev = pos;
	tail->next = pos->next;
	pos->next  = head;
	if(tail->next != NULL)
		tail->next->prev = tail;
}

static inline void lnode_insert_range(lnode_s* pos, lnode_s* head, lnode_s* tail) {
	head->prev = pos->prev;
	tail->next = pos;
	pos->prev  = tail;
	if(head->prev != NULL)
		head->prev->next = head;
}

static inline void lnode_insert_after(lnode_s* pos, lnode_s* node) {
	lnode_insert_range_after(pos, node, node);
}

static inline void lnode_insert(lnode_s* pos, lnode_s* node) {
	lnode_insert_range(pos, node, node);
}

static inline void lnode_detach_range(lnode_s* head, lnode_s* tail) {
	if(head->prev != NULL)
		head->prev->next = tail->next;
	if(tail->next != NULL)
		tail->next->prev = head->prev;

	head->prev = NULL;
	tail->next = NULL;
}

static inline void lnode_detach(lnode_s* node) {
	lnode_detach_range(node, node);
}

static inline lnode_s* lnode_advance(lnode_s* node, ptrdiff_t step) {
	while(step != 0) {
		if(step > 0) {
			node = node->next;
			--step;
		} else {
			node = node->prev;
			++step;
		}
	}
	return node;
}

static inline size_t lnode_range_len(const lnode_s* head, const lnode_s* tail) {
	size_t cnt = 0;
	if(head != tail)
		for(; head != tail->next; head = head->next) ++cnt;
	return cnt;
}

static inline void lnode_splice_range(lnode_s* pos, lnode_s* head, lnode_s* tail) {
	lnode_detach_range(head, tail);
	lnode_insert_range(pos, head, tail);
}

static inline void lnode_splice(lnode_s* pos, lnode_s* from) {
	lnode_splice_range(pos, from, from);
}

static inline void lnode_splice_range_after(lnode_s* pos, lnode_s* head, lnode_s* tail) {
	lnode_detach_range(head, tail);
	lnode_insert_range_after(pos, head, tail);
}

static inline void lnode_splice_after(lnode_s* pos, lnode_s* from) {
	lnode_splice_range_after(pos, from, from);
}

static inline void lnode_reverse(lnode_s* head, lnode_s* tail) {
	if(head->prev != NULL)
		head->prev->next = tail;
	if(tail->next != NULL)
		tail->next->prev = head;

	Macro_util_swap(lnode_s*, tail->prev, tail->next);
	Macro_util_swap(lnode_s*, head->prev, head->next);
	Macro_util_swap(lnode_s*, head->next, tail->prev);

	head = head->prev;
	while(head != tail) {
		Macro_util_swap(lnode_s*, head->prev, head->next);
		head = head->prev; // next
	}
}

static inline void lnode_for_each(lnode_s* head, lnode_s* tail, unary_func_t func) {
	if(head != tail)
		for(; head != tail->next; head = head->next) func(head->data);
}

#endif

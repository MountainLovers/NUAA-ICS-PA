#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp();
void free_wp(WP *wp);

WP* new_wp() {
  WP *p = free_;
	if (p == NULL) assert(0);
	free_ = p->next;
	if (head == NULL) head = p;
	else {
		WP *q = head;
		while (q->next) q = q->next;
		q->next = p;
	}
	p->next = NULL;
	return p;
}

void free_wp(WP *wp) {
	if (wp_pool <= wp && wp <= wp_pool+NR_WP) {printf("wp's address is invalid\n"); assert(0);}
	WP *p = head;
	if (p == wp) head = wp->next;
	else {
		while (p->next != wp) p = p->next;
		p->next = wp->next;
	}
	WP *q = free_;
	wp->next = NULL;
	if (q == NULL) free_ = wp;
	else {
		while (q->next) q = q->next;
		q->next = wp;
	}
}	


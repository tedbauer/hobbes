#include "util.h"

typedef struct LL_list_ {
	void* head;
	struct LL_list_ *tail;
} *LL_list;

LL_list LL_List(void* head, LL_list tail);

LL_list LL_emptyList();

LL_list LL_listOf(int count, ...);

bool LL_contains(LL_list, void* item);

void LL_iter(LL_list list, void (*f) (void* a, void* acc), void* acc);

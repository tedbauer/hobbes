#include "../include/linkedlist.h"
#include "../include/util.h"
#include <stdarg.h>

LL_list LL_List(void* head, LL_list tail)
{
	LL_list l = checked_malloc(sizeof(*l));
	l->head = head;
	l->tail = tail;
	return l;
}

LL_list LL_listOf(int count, ...)
{
	va_list ap;
	LL_list currTail = NULL;
	int j;
	va_start(ap, count);
	for (j = 0; j < count; j++) {
		currTail = LL_List(va_arg(ap, void*), currTail);
	}
	va_end(ap);
	return currTail;
}

LL_list LL_emptyList()
{
	return NULL;
}

void LL_iter(LL_list list, void (*f) (void* a, void* acc), void* acc)
{
	if (list->tail == NULL) {
		return;
	}

	LL_list currList = list;
	while (currList) {
		f(currList->head, acc);
		currList = currList->tail;
	}
}

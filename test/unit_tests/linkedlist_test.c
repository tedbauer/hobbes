#include "assert.h"
#include "util.h"
#include "../../include/linkedlist.h"

void add(int* a, int* acc)
{
	*acc = *acc + *a;
}

void adderFoldTest()
{

	int data1 = 5;
	int data2 = 7;
	int data3 = 18;
	int data4 = 12;
	int data5 = 19;

	LL_list l = LL_listOf(5, &data1, &data2, &data3, &data4, &data5);
	int acc;
	LL_iter(l, (void (*)(void*, void*))add, &acc);

	assert(acc == 61);
}

int main()
{
	adderFoldTest();
}

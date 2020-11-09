#include "stack.h"

int main() {
	stack *stk = (stack *)calloc(1, sizeof(stack));
	test_Ctor(stk);
	test_push(stk);
	test_pop(stk);
	stack_Dtor(stk);
	printf("ok");
	return 0;
}
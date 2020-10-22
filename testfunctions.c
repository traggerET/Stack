#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const char CAN_VAL = -123;
const int CAN_MEM = 80;
const int POISON = 0xFEEEDACE;
const int GREAT_NUM = 65521;
const char ADD = 1;
const char SUB = -1;

typedef int stktype;

typedef struct stack_t {
	unsigned long long c_front;
	size_t capacity;			//max size of stack
	size_t size;  
	stktype *data;
	unsigned int hash_sum;
	unsigned long long c_back;  //array of reserved ints
} stack;

void 	stack_Ctor	(stack *stk, size_t capacity);
void    stack_push	(stack *stk, int n);
stktype stack_pop	(stack *stk);
int     stack_empty	(stack *stk);
int     stack_full	(stack *stk);
size_t  stack_size	(stack *stk);
void    stack_Dtor	(stack *stk);
stktype stack_back	(stack *stk);
void    stack_realloc(stack *stk);

void 	put_canary	(char *ptr);
void 	can_check	(stack *stk);
void 	replace_can	(stack *stk);
void 	put_poison	(stack *stk, size_t pos);
void 	poison_check(stack *stk);
void 	hash_func	(stack *stk, const char sign);
void 	check_hash	(stack *stk);

void test_realloc(stack *stk);
void test_Ctor(stack *stk);

//put poison in unused elements
void put_poison(stack *stk, size_t pos) {
	size_t i = pos;
	for (i = pos; i < stk->capacity; ++i) {
		stk->data[i] = POISON;
	}
}

//putting canaries to the left and right from stack
void put_canary(char *ptr) {
	int i = 0;
	for (i = 0; i < CAN_MEM / 2; i++) {
		*(ptr + i) = CAN_VAL;
	}
}

//create new stack, 
void stack_Ctor(stack *stk, size_t capacity) { 
	char *ptr = (char *)calloc(capacity * sizeof(stk->data[0]) + CAN_MEM, sizeof(char));
	if (ptr == NULL) {
		//stack_dump(1);
		assert(ptr);
	}
	stk->capacity = capacity;
	stk->size = 0;
	put_canary(ptr); //putting left canary
	//putting right canary
	put_canary(ptr +  CAN_MEM / 2 + sizeof(stk->data[0]) * stk->capacity);	
	stk->data = (stktype *)(ptr + CAN_MEM / 2); //put of data between canaries
	put_poison(stk, 0);
	//else 
	//	stack_dump(1);
}

void test_Ctor(stack *stk) {
	printf("Testing Stack Constructor of size 5\n");
	stack_Ctor(stk, 5);
	int i = 0;
	printf("Left canaries: ");
	for (i = 1; i <= CAN_MEM / 2; i++) {
		if (*((char *)stk->data - i) == CAN_VAL)
			printf("%d ", *((char *)stk->data - i));
	}
	printf("\n");
	for (i = 0; i < 5; i++)
		if (stk->data[i] == POISON)
			printf("stk-data[%d] == POISON is true\n", i);
	printf("Right canaries: ");
	for (i = 0; i < CAN_MEM / 2; i++) {
		if (*((char *)(&stk->data[stk->capacity - 1] + 1) + i) == CAN_VAL)
			printf("%d ", *((char *)(&stk->data[stk->capacity - 1] + 1) + i));
	}
	printf("\n");
	printf("Stack Constructor of size 5 has completed successfully\n");
	printf("\n");
	printf("\n");
}

//push new element in stack
void stack_push(stack *stk, stktype n) {
	if (stack_full(stk))
		stack_realloc(stk); 
	stk->data[stk->size++] = n;
	hash_func(stk, ADD);
	printf("%d is inserted\n",stk->data[stk->size - 1]);
}

void test_push(stack *stk) {
	int i = 0;
	printf("Testing Stack Push on values [17, 23]\n");
	for (i = 17; i < 24; i++)
		stack_push(stk, i);
	check_hash(stk);
	}

//extract and return top element
stktype stack_pop(stack *stk) {
	if (stack_empty(stk)) {
		//stack_dump();
	}
	hash_func(stk, SUB);
	stktype extr = stk->data[--stk->size];
	printf("%d is extracted\n", stk->data[stk->size]);
	stk->data[stk->size] = POISON;
	return extr;
}

void test_pop(stack *stk) { 
	stack_pop(stk);
	stack_pop(stk);
	check_hash(stk);
}

//check if it is empty
int stack_empty(stack *stk) {
	return (stk->size == 0); 
}

//check if it is full
int stack_full(stack *stk) {
	return (stk->size == stk->capacity - 1);  
}

//show size of stack
size_t stack_size(stack *stk) {
	//fprintf("logfile.txt", "function stack_size: stack size is %d", stk->size);
	return stk->size;
}

//just look what's the last element 
stktype stack_back(stack *stk) {
	//fprintf("logfile.txt", "function stack_back: top element is %d", stkdata[stk->size - 1]);
	return stk->data[stk->size - 1];
}

//when reallocating mem, we need to move right canary behind the borders of expanded array
void replace_can(stack *stk) {
	char *new_can_ptr = (char *)(&(stk->data[stk->capacity - 1]) + 1);
	//(stktype *)(&stk->data[stk->capacity - 1]);
	put_canary(new_can_ptr);
}

//reallocator of mem for stack
void stack_realloc(stack *stk) {
	printf("Reallocating...\n");
	char *old_ptr = (char *)stk->data - CAN_MEM / 2; //get adress of old block of mem.
	stk->capacity += stk->capacity;
	char *new_ptr = (char *)realloc(old_ptr, stk->capacity * sizeof(stktype) + CAN_MEM);
	if (new_ptr == NULL) {
		//stack_dump();
		assert(1);
	}
	printf("Realloc status - success\n");
	stk->data = (stktype *)(new_ptr + CAN_MEM / 2);
	replace_can(stk);
	put_poison(stk, stk->size);
	test_realloc(stk);

}

void test_realloc(stack *stk) {
	int i = 0;
	printf("\n");
	printf("    Left canaries after realloc: ");
	for (i = 1; i <= CAN_MEM / 2; i++) {
		if (*((char *)stk->data - i) == CAN_VAL)
			printf("%d ", *((char *)stk->data - i));
	}
	printf("\n");
	printf("    Checking data after realloc\n");
	for (i = 0; i < stk->size; i++)
		printf("    stk->data[%d] = %d\n", i, stk->data[i]);
	printf("\n");
	for (i = stk->size; i < stk->capacity; i++)
		if (stk->data[i] == POISON)
			printf("    stk-data[%d] == POISON is true\n", i);
	printf("    Right canaries after realloc: ");
	for (i = 0; i < CAN_MEM / 2; i++) {
		if (*((char *)(&stk->data[stk->capacity - 1] + 1) + i) == CAN_VAL)
			printf("%d ", *((char *)(&stk->data[stk->capacity - 1] + 1) + i));
	}
	printf("\n");
	printf("    Realloc status - success\n");
	printf("\n");
	printf("\n");
}

//destructof of stack
void stack_Dtor(stack *stk) {
	put_poison(stk, 0); //annulate all mem.
	free((char *)stk->data - CAN_MEM); //destruct data with canaries.
	stk->size = 0;
	stk->capacity = 0;
	printf("Stack Destruction status - success\n");
}

//check if canary is intact
void can_check(stack *stk) {
	int i = 0;
	for (i = 1; i <= CAN_MEM / 2; i++) {
		//check left canary
		if (*((char *)stk->data - i) != CAN_VAL) { 
			//stack_dump();
		}
		//check right canary
		if (*((char*)(stk->data + stk->capacity) - 1 + i) != CAN_VAL) {
			//stack_dump();
		}
	}	
}

//check if all poisoned elements are intact 
void poison_check(stack *stk) {
	size_t i = 0;
	for (i = stk->size; i < stk->capacity; i++) {
		if (stk->data[i] != POISON) {
			//stack_dump();
		}
	}
}

//count hash summ with hash func
void hash_func(stack *stk, const char sign) {
	int d = (stk->size * stk->data[stk->size - 1]) % GREAT_NUM;
	//if sign == ADD, we have new element push in stack.
	if (sign > 0)
		stk->hash_sum = (stk->hash_sum + d) % GREAT_NUM;
	//if sign == SUB, we have top element popped so we substract from hash summ.
	else 
		stk->hash_sum = (stk->hash_sum - d) % GREAT_NUM;
}


//compare reserved hash summ with real.
void check_hash(stack *stk) {
	size_t i = 0;
	unsigned int temp = 0;
	for (i = 1; i <= stk->size; i++) {
		temp = (temp + (stk->data[i - 1] * i) % GREAT_NUM) % GREAT_NUM;
	}
	if (temp != stk->hash_sum) {
		//stack_dump();
	}
	else 
		printf("Hash summ status - success\n");
}



int main() {
	stack *stk = (stack *)calloc(1, sizeof(stack));
	test_Ctor(stk);
	test_push(stk);
	test_pop(stk);
	stack_Dtor(stk);
	printf("ok");
	return 0;
}

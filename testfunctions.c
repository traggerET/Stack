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

stack  *stack_Ctor   (stack *stk, size_t capacity, const int nline, const char* fname);
void    stack_push   (stack *stk, int n);
stktype stack_pop    (stack *stk);
int     stack_empty  (stack *stk);
int     stack_full   (stack *stk);
size_t  stack_size   (stack *stk);
void    stack_Dtor   (stack *stk);
stktype stack_back   (stack *stk);
void    stack_realloc(stack *stk);
void 	put_canary   (char *ptr);
void 	can_check	 (stack *stk);
void 	replace_can  (stack *stk);
void 	put_poison   (stack *stk, size_t pos);
void 	poison_check (stack *stk);
void 	hash_func	 (stack *stk, const char sign);
void 	check_hash   (stack *stk);

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
stack *stack_Ctor(stack *stk, size_t capacity, const int nline, const char* fname) { 
	char *ptr = (char *)calloc(capacity * sizeof(stk->data[0]) + CAN_MEM, sizeof(char));
	if (ptr == NULL) {
		//stack_dump(1);
		assert(ptr);
	}
	stk->capacity = capacity;
	stk->size = 0;

	put_canary(ptr); //putting left canary
	put_canary(ptr + //putting right canary
			   CAN_MEM / 2 + 
			   sizeof(stk->data[0]) * stk->capacity);	
	stk->data = (stktype *)(ptr + CAN_MEM / 2); //put of data between canaries
	put_poison(stk, 0);

	//else 
	//	stack_dump(1);
}

//push new element in stack
void stack_push(stack *stk, stktype n) {
	stk->data[stk->size++] = n;
	printf("%d is inserted\n",stk->data[stk->size - 1]);
}

//extract and return top element
stktype stack_pop(stack *stk) {
	stktype extr = stk->data[--stk->size];
	printf("%d is extracted\n", stk->data[stk->size]);
	stk->data[stk->size] = POISON;
	return extr;
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
	(stktype *)(&stk->data[stk->capacity - 1]);
	put_canary(new_can_ptr);
}

//reallocator of mem for stack
void stack_realloc(stack *stk) {
	char *old_ptr = (char *)stk->data - CAN_MEM / 2; //get adress of old block of mem.
	stk->capacity += stk->capacity;
	char *new_ptr = (char *)realloc(old_ptr, 
									stk->capacity * sizeof(stktype) + CAN_MEM);
	if (new_ptr == NULL) {
		//stack_dump();
		assert(1);
	}
	stk->data = (stktype *)(new_ptr + CAN_MEM / 2);
	replace_can(stk);
	put_poison(stk, stk->size);
}

//destructof of stack
void stack_Dtor(stack *stk) {
	put_poison(stk, 0); //annulate all mem.
	free((char *)stk->data - CAN_MEM); //destruct data with canaries.
	(stktype *)stk->data;
	stk->size = 0;
	stk->capacity = 0;
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
	int temp = 0;
	for (i = 1; i <= stk->size; i++) {
		temp = (temp + 
			   (stk->data[i - 1] * i) % GREAT_NUM) %
			   GREAT_NUM;
	}
	if (temp != stk->hash_sum) {
		//stack_dump();
	}
}

int main() {
	stack *stk = (stack *)calloc(1, sizeof(stack));
	return 0;
}

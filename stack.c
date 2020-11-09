#include "stack.h"


/*	@brief stack verifier
	@param stk adress of stk
	return num of errnum if there is one, otherwise return 0
*/
int stack_error(stack *stk) { 
	if (stk == NULL) 
		return STACK_NULL_PTR;

	if (stk->data == NULL)
		return DATA_NULL_PTR;
	
	if (stk->size > stk->capacity) 
		return OVERSIZE;
	
	if (can_check(stk) != 0) 
		return HURT_CAN;
	
	if (poison_check(stk) != 0) 
		return HURT_POISON;
	
	if (check_hash(stk) != 0) 
		return HURT_HASH;

	return 0;
}

/*	@brief put poison in unused elements
	@param stk adress of stk
	@param pos num of element from which we put poison
*/
void put_poison(stack *stk, size_t pos) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	size_t i = pos;
	for (i = pos; i < stk->capacity; ++i) {
		stk->data[i] = POISON;
	}
}

/*	@brief putting canaries 
	@param ptr adress of memory where we put canaries
*/
void put_canary(char *ptr, stack *stk) {
	if (ptr == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"ptr == NULL");
	}
	int i = 0;
	for (i = 0; i < CAN_MEM / 2; i++) {
		*(ptr + i) = CAN_VAL;
	}
}

/*
	@brief create stack, with all protection
	@param stk adress of stk
	@param capacity size of stk
*/
void stack_Ctor(stack *stk, size_t capacity) { 
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	char *ptr = (char *)calloc(capacity * sizeof(stk->data[0]) + CAN_MEM, sizeof(char));
	if (ptr == NULL) {
		assert(!"Cant create stack. Given null pointer");
	}
	stk->capacity = capacity;
	stk->size = 0;
	stk->hash_sum = 0;
	put_canary(ptr, stk); //putting left canary
	//putting right canary
	put_canary(ptr +  CAN_MEM / 2 + sizeof(stk->data[0]) * stk->capacity, stk);	
	stk->data = (stktype *)(ptr + CAN_MEM / 2); //put of data between canaries
	put_poison(stk, 0);
	printf("capacity is %d\n", stk->capacity);
	STACK_OK;
}

/*
	@brief test Ctor function
	@param stk adress of stk
*/
void test_Ctor(stack *stk) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
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

/*	@brief push new element in stack
	@param stk adress of stk
	@param n element that is to be inserted
*/
void stack_push(stack *stk, stktype n) {
	STACK_OK;
	if (stack_full(stk))
		stack_realloc(stk); 
	stk->data[stk->size++] = n;
	hash_func(stk, ADD);
	printf(" %f is inserted\n",stk->data[stk->size - 1]);
	STACK_OK;
}

/*	@brief test push function
	@param stk adress of stk
*/
void test_push(stack *stk) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	int i = 0;
	printf("Testing Stack Push on values [17, 23]\n");
	for (i = 17; i < 24; i++)
		stack_push(stk, i);
	check_hash(stk);
	}

/*	@brief extract and return top element
	@param stk adress of stk
*/
stktype stack_pop(stack *stk) {
	STACK_OK;
	if (stack_empty(stk)) {
		assert(!"Cant pop from empty stack");
	}
	hash_func(stk, SUB);
	stktype extr = stk->data[--stk->size];
	printf(" %f is extracted\n", stk->data[stk->size]);
	stk->data[stk->size] = POISON;
	STACK_OK;
	return extr;
}

/*	@brief testing pop function
	@param stk adress of stk
*/
void test_pop(stack *stk) { 
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	stack_pop(stk);
	stack_pop(stk);
	check_hash(stk);
}

//check if it is empty
int stack_empty(stack *stk) {
	STACK_OK;
	return (stk->size == 0); 
}

//check if it is full
int stack_full(stack *stk) {
	STACK_OK;
	return (stk->size == stk->capacity - 1);  
}

//show size of stack
size_t stack_size(stack *stk) {
	STACK_OK;
	return stk->size;
}

//just look what's the last element 
stktype stack_back(stack *stk) {
	STACK_OK;
	return stk->data[stk->size - 1];
}

/*	@bried when reallocating mem, we need to move right canary behind the borders of expanded array
	@param stk adress of stk
*/
void replace_can(stack *stk) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	char *new_can_ptr = (char *)(&(stk->data[stk->capacity - 1]) + 1);
	put_canary(new_can_ptr, stk);
}

/*	@brief reallocator of mem for stack
	@param stk adress of stk
*/
void stack_realloc(stack *stk) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	printf("Reallocating...\n");
	char *old_ptr = (char *)stk->data - CAN_MEM / 2; //get adress of old block of mem.
	stk->capacity += stk->capacity;
	char *new_ptr = (char *)realloc(old_ptr, stk->capacity * sizeof(stktype) + CAN_MEM);
	if (new_ptr == NULL) {
		assert(!"Cant reallocate mem");
	}
	printf("Realloc status - success\n");
	stk->data = (stktype *)(new_ptr + CAN_MEM / 2);
	replace_can(stk);
	put_poison(stk, stk->size);
	test_realloc(stk);
}

/*
	@brief test reallocating memory, replacing canaries
	@param stk adress of stk
*/
void test_realloc(stack *stk) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	int i = 0;
	size_t j = 0;
	printf("\n");
	printf("    Left canaries after realloc: ");
	for (i = 1; i <= CAN_MEM / 2; i++) {
		if (*((char *)stk->data - i) == CAN_VAL)
			printf("%d ", *((char *)stk->data - i));
	}
	printf("\n");
	printf("    Checking data after realloc\n");
	for (j = 0; j < stk->size; j++)
		printf("    stk->data[%zu]= %d\n", j, stk->data[j]);
	printf("\n");
	for (j = stk->size; j < stk->capacity; j++)
		if (stk->data[j] == POISON)
			printf("    stk-data[%zu] == POISON is true\n", j);
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

/*	@brief destructor of stack
	@param stk adress of stk
*/
void stack_Dtor(stack *stk) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	put_poison(stk, 0); //annulate all mem.
	free((char *)stk->data - CAN_MEM / 2); //destruct data with canaries.
	stk->size = 0;
	stk->capacity = 0;
	printf("Stack Destruction status - success\n");
}

/*  @brief check canaries
	@param stk adress of stk
*/
int can_check(stack *stk) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	int i = 0;
	for (i = 1; i <= CAN_MEM / 2; i++) {
		//check left canary
		if (*((char *)stk->data - i) != CAN_VAL) { 
			return -1;
		}
		//check right canary
		if (*((char*)(stk->data + stk->capacity) - 1 + i) != CAN_VAL) {
			return -1;
		}
	}
	return 0;	
}

/*  @brief check posioned elements
	@param stk adress of stk
*/
int poison_check(stack *stk) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	size_t i = 0;
	for (i = stk->size; i < stk->capacity; i++) {
		if (stk->data[i] != POISON) {
			return -1;
		}
	}
	return 0;
}

/*  @brief get all info about stack and errors 
	@param stk adress of stk
	@param sign (+/-) to decide whether to add or substract
*/
void hash_func(stack *stk, const char sign) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	int d = (stk->size * (int)stk->data[stk->size - 1]) % GREAT_NUM;
	//if sign == ADD, we have new element push in stack.
	if (sign > 0)
		stk->hash_sum = (stk->hash_sum + d) % GREAT_NUM;
	//if sign == SUB, we have top element popped so we substract from hash summ.
	else 
		stk->hash_sum = (stk->hash_sum - d) % GREAT_NUM;
}

/*  @brief check hash summ
	@param stk adress of stk
*/
int check_hash(stack *stk) {
	if (stk == NULL) {
		stack_dump(STACK_NULL_PTR, __func__, __LINE__, stk);
		assert(!"stk == NULL");
	}
	size_t i = 0;
	unsigned int temp = 0;
	for (i = 1; i <= stk->size; i++) {
		temp = (temp + ((int)stk->data[i - 1] * i) % GREAT_NUM) % GREAT_NUM;
	}
	if (temp != stk->hash_sum) {
		return -1;
	}
	else 
		return 0;
}

/*  @brief get all info about stack and errors 
	@param ERNUM num of ERROR
	@param func_name num calling function.
	@param nline number of line in code
	@param stk adress of stk
*/
void stack_dump(int ERNUM, const char *func_name, const int nline, stack *stk) {
	printf("List of errors: \n");
	printf("STACK_NULL_PTR = %d\n",	STACK_NULL_PTR);
	printf("DATA_NULL_PTR = %d\n",	DATA_NULL_PTR);
	printf("OVERSIZE = %d\n",		OVERSIZE);
	printf("HURT_CAN = %d\n",		HURT_CAN);
	printf("HURT_POISON = %d\n",	HURT_POISON);
	printf("HURT_HASH = %d\n",		HURT_HASH);
	printf("stack: ");
	printf("in function: %s\n ", func_name);
	printf("Number of line: %d\n", nline); 
	if ((ERNUM == 0) && (stk != NULL))
		printf("OK");
	else printf("NE OK. ERNUM = %d, ", ERNUM);
	if (stk == NULL)
		printf("stk == NULL POINTER\n");
	else {
		printf("[%p]\n", stk);
		printf("{\n");
		printf("size = %zu\n", stk->size);
		printf("capacity = %zu\n", stk->capacity);
		printf("data ");
		if (stk->data == NULL)
			printf("[NULL POINTER]");
		else {
			printf("[%p]\n", stk->data);
			printf("{\n");
			if (stk->size > stk->capacity) 
				printf("UNDEFINED OPERATION: ERNUM %d\n. Cant print data", OVERSIZE);
			else {
				size_t i = 0;
				for (i = 0; i < stk->size; i++)
					printf("* [%zu] = %d\n", i, stk->data[i]);
				for (i = stk->size; i < stk->capacity; i++) {
					if (stk->data[i] == POISON)
						printf("* [%zu] = POISON\n", i);
					else
						printf("! [%zu] != POISON\n", i);
				}
			}
			printf("}\n");
		}
		printf("}\n");
	}
}


#define STACK_OK if (stack_error(stk) != 0) \
					stack_dump(stack_error(stk), __func__, __LINE__, stk);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum ERNUM {STACK_NULL_PTR = 1,
			DATA_NULL_PTR,
			OVERSIZE,
			HURT_CAN,
			HURT_POISON, 
			HURT_HASH}; 

const char CAN_VAL = -123;
const int CAN_MEM = 80;
const double POISON = 0xFEEEDACE;
const int GREAT_NUM = 65521;
const char ADD = 1;
const char SUB = -1;

typedef double stktype;

typedef struct stack_t {
	unsigned long long c_front;
	size_t capacity;			//max size of stack
	size_t size;  
	stktype *data;
	unsigned int hash_sum;
	unsigned long long c_back;  //array of reserved ints
} stack;

void 	stack_Ctor	(stack *stk, size_t capacity);
void    stack_push	(stack *stk, stktype n);
stktype stack_pop	(stack *stk);
int     stack_empty	(stack *stk);
int     stack_full	(stack *stk);
size_t  stack_size	(stack *stk);
void    stack_Dtor	(stack *stk);
stktype stack_back	(stack *stk);
void    stack_realloc(stack *stk);

int 	stack_error	(stack *stk);

void 	put_canary	(char *ptr, stack *stk);
int 	can_check	(stack *stk);
void 	replace_can	(stack *stk);
void 	put_poison	(stack *stk, size_t pos);
int 	poison_check(stack *stk);
void 	hash_func	(stack *stk, const char sign);
int 	check_hash	(stack *stk);

void test_realloc(stack *stk);
void test_Ctor(stack *stk);
void test_pop(stack *stk);
void test_push(stack *stk);

void stack_dump(int ERNUM, const char *func_name, const int nline, stack *stk);

#define STACK_OK if (stack_error(stk) != 0) \
					stack_dump(stack_error(stk), __func__, __LINE__, stk);

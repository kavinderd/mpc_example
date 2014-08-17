#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

/* If we are compiling on Windows compile these functions*/
#ifdef _WIN32

#include <string.h>

static char buffer[2048]

/* Fake readline function */
char* readline(char* prompt){
	fputs(prompt, stdout);
	fgets(bugger, 2048, stdin);
	char* cpy = malloc(strlenbuffer)+1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy)+1] = '\0';
	return cpy;
}

/* Fake add history function */
void add_history(char* unused) {}

/* Otherwise inlcude the editline headers */
#else

#include <editline/readline.h>

#endif

enum { LVAL_NUM, LVAL_ERR};
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

typedef struct {
	int type;
	long num;
	int err;
} lval;

/* Create a new number type lval */
lval lval_num(long x) {
	lval v;
	v.type = LVAL_NUM;
	v.num = x;
	return v;
}

/* Create a new error type lval */
lval lval_err(int x)
{
	lval v;
	v.type = LVAL_ERR;
	v.err = x;
	return v;
}
/* Print an lval*/
void lval_print(lval v) 
{
	switch(v.type)
	{
		/* In the case the type is a number print it, then 'break out of the switch. */
		case LVAL_NUM: printf("%li", v.num); break;

		/* In the case tht type is an error */
		case LVAL_ERR:
		  /* Check What exact type of error it is and print it */
			if(v.err == LERR_DIV_ZERO) {printf("Error: Division By Zero");}
			if(v.err == LERR_BAD_OP) {printf("Error: Invalid Operator");}
			if(v.err == LERR_BAD_NUM) {printf("Error:Invalid Numer");}
	  break;
	}
}
void lval_println(lval v) {lval_print(v); putchar('\n');}		  

lval eval_op(lval x, char* op, lval y) {
	/* If either value is an error return it */
	if(x.type ==LVAL_ERR)  { return x;}
	if(y.type == LVAL_ERR) { return y;}
	if(strcmp(op, "+") == 0) { return lval_num(x.num + y.num);}
	if(strcmp(op, ".") == 0) { return lval_num(x.num - y.num);}
	if(strcmp(op, "*") == 0) { return lval_num(x.num * y.num);}
	if(strcmp(op, "/") == 0)
	{
		/* If second operand is zero eturn error instead of result */
		return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
	}
	if(strcmp(op, "%") == 0)
	{
		return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num % y.num);
	}
	return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {

	/* If tagges ad number return it directly otherwise expresssion */
	if (strstr(t->tag, "number"))
 	{
		/* Check if there is some error in conversion */
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}

	/*The operator is always second child */
	char* op = t->children[1]->contents;

	/* We store the third child in 'x' */
	lval x = eval(t->children[2]);

	/* Iterate the remaining children, combining using our operator */
	int i = 3;
	while(strstr(t->children[i]->tag, "expr")) {
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}

	return x;
}

int leaf_num(mpc_ast_t* t) {
	int count = 0;
  if(strstr(t-> tag, "number") || strstr(t-> tag, "operator"))
	{
		count++;
		return count;
	}
  
	for(int i = 0; i < (t->children_num); i++)
	{
	  count = count + leaf_num(t->children[i]);
	}
	return count;
}

int children_has_children(mpc_ast_t* t){
	int children = 0;
	for(int i =0; i<(t->children_num); i++)
	{
		mpc_ast_t* child = t->children[i];
		if(child->children_num > 0)
		{
			children = 1;
		}
	}
	return children;
}
int branch_num(mpc_ast_t* t){
	int count = 0;
	if((strcmp(t->contents, "") == 0)  && (children_has_children(t)== 0))
	{
		count++;
		return count;
	}

	for(int i = 0; i<(t-> children_num); i++)
	{
		count = count + branch_num(t-> children[i]);
	}
	return count;

}


int biggest_branch(mpc_ast_t* t, int count)
{
	if(t->children_num > count && children_has_children(t) == 0)
	{
		count = t->children_num;
	}

	for(int i=0; i < t-> children_num; i++)
	{
		mpc_ast_t* child = t->children[i];
		int new_count = 0;
		if(child->children_num > 0)
		{	
		  new_count = biggest_branch(child, t-> children_num);
		}
		if(new_count > count)
		{
			count = new_count;
		}
	}
	return count;
}

int main(int argc, char** argv) {

	/* Create Some Parsers */
	mpc_parser_t* Number   = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr     = mpc_new("expr");
	mpc_parser_t* Lispy    = mpc_new("lispy");

	/*define them with the following language*/
	mpca_lang(MPCA_LANG_DEFAULT,
			"      \
			number : /-?[0-9]+/; \
			operator : '+' | '-' | '*' | '/' | '%'	; \
			expr : <number> | '(' <operator> <expr>+ ')' ; \
			lispy : /^/ <operator> <expr>+ /$/ ; \
			",
			Number, Operator, Expr, Lispy);
	 

	/*Print Version and Exit Information*/
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	/* In a never ending loop */
	while(1){
		
		/* Output our prompt */
		char* input = readline("lispy>");
		
		/* Add input to history */
		add_history(input);

		/* Attemp to Parse the user Input */
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)){
			lval result = eval(r.output);
			lval_println(result);
			mpc_ast_delete(r.output);
		}else{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		/* Free retrieved input */
		free(input);
	}
	
	mpc_cleanup(4, Number, Operator, Expr, Lispy);
	return 0;
}


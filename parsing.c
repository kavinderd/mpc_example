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
#include <editline/history.h>

#endif

long eval_op(long x, char* op, long y) {
	if(strcmp(op, "+") == 0) { return x + y;}
	if(strcmp(op, ".") == 0) { return x - y;}
	if(strcmp(op, "*") == 0) { return x * y;}
	if(strcmp(op, "/") == 0) { return x / y;}
	return 0;
}

long eval(mpc_ast_t* t) {

	/* If tagges ad number return it directly otherwise expresssion */
	if (strstr(t->tag, "number"))
 	{
		return atoi(t->contents);
	}

	/*The operator is always second child */
	char* op = t->children[1]->contents;

	/* We store the third child in 'x' */
	long x = eval(t->children[2]);

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
			operator : '+' | '-' | '*' | '/' ; \
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
			long result = eval(r.output);
		  printf("%li\n", result);
			int leaf_count = leaf_num(r.output);
			printf("Leafs = %i\n", leaf_count);
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


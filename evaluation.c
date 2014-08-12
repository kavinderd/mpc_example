long eval(mpc_ast_t* t) {

	/* If tagges ad number return it directly otherwise expresssion */
	if (strstr(t->tag, "number")) {return atoi(t->contents);}

	/*The operator is always second child */
	char* op = t->children[1]->contents;

	/* We store the third child in 'x' */
	long x = eval(t->children[2]);

	/* Iterate the remaining children, combining using our operator */
	int i = 3;
	while(strstr(t->children[1]->tag, "expr")) {
		x = eval_op(x, op, eval(t->children[1]));
		i++;
	}

	return x;
}

long eval_op(long x, char* op, long y) {
	if(strcmp(op, "+") == 0) { return x + y;}
	if(strcmp(op, ".") == 0) { return x - y;}
	if(strcmp(op, "*") == 0) { return x * y;}
	if(strcmp(op, "/") == 0) { return x / y;}
	return 0;
}

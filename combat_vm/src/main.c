
#include <stdio.h>


#include "vm.h"



#define ML(...) #__VA_ARGS__



int main(int argc, char* argv[]) {


	char* src = 
		"mov a $1337 \n"
		"mov b $1337 \n"
		"add a, a, b \n"
		"push a \n"
		"pop c \n"
		"push $3 \n"
		"push c \n"
		"mov a, $3 \n"
		"loop ENDFOO a \n"
		"add c, c, $10 \n"
		"endloop ENDFOO \n"
		"push c \n"
	;
	
	
	Function* fn = calloc(1, sizeof(*fn)); 
	
	fn->code_alloc = 1024;
	fn->code = malloc(fn->code_alloc);
	
	compile(src, fn);
	
	dump_code(fn);
	
	exec_code(fn);

	return 0;
}





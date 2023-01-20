#ifndef __combat_vm__compile_h__
#define __combat_vm__compile_h__

#include "vm.h"


typedef struct {
	int opcode;
	char argc;
	unsigned short argt[4];
	union {
		long n;
		double r;
		char* s;
	} argv[4];
	
	int bytecode_offset;
	// todo: arg types and immediate values
} instr_info;


typedef struct {
	int id;
	char* name;
	char activeInLoop;
} variable_info;

typedef struct context {
	HT(int) varNameLookup;
	VEC(variable_info) varInfo;
	
	HT(int) labelLookup;
	
	
	int hasErrors;
	
	VEC(instr_info) iis;
	
	struct context* parent;
} context;


struct instruction_info {
	int opcode;
	char* name;
	int argc;
	int arg_type_mask[4];
	int arg_flags[4];
};
extern struct instruction_info instruction_list[];


int get_var(context* ctx, char* name, int len);
size_t strspnident(char* s);


int check_immutable(context* ctx, instr_info* fn);
int fix_loops(context* ctx, Function* fn);

struct instr_conversion {
	int oldOp;
	int argt[4];
	int newOp;
};


extern struct instr_conversion conversion_list[];

#define CONVERSION_LIST \
	X1(push, Var, pushv) \
	X1(push, Int, pushi) \
	X1(push, Real, pushr) \
	X1(push, String, pushs) \
	X2(mov, Var, Var, movv) \
	X2(mov, Var, Int, movi) \
	X2(mov, Var, Real, movr) \
	X2(mov, Var, String, movs) \
	X3(add, Var, Var, Int, addi) \
	X3(add, Var, Var, Real, addr) \
	X3(add, Var, Var, Var, addv) \





#endif // __combat_vm__compile_h__

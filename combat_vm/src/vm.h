#ifndef __combat_vm__vm_h__
#define __combat_vm__vm_h__

#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include "sti/sti.h"


#define dbg(fmt, ...) fprintf(stderr, "%s:%d " fmt "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__);

#define popcount(x) __builtin_popcount(x)


#define RUN(a) a
#define CAT(a,b) a##b

#define FLAG_PREPEND(...) FLAG_PREPENDN(PP_NARG(__VA_ARGS__), __VA_ARGS__) 
#define FLAG_PREPENDN(N, ...) CAT(FLAG_PREPEND,N)(__VA_ARGS__) 
#define FLAG_PREPEND1(a) AGF_##a
#define FLAG_PREPEND2(a,b) AGF_##a | AGF_##b
#define FLAG_PREPEND3(a,b,c) AGF_##a | AGF_##b | AGF_##char

#define TYPE_PREPEND(...) TYPE_PREPENDN(PP_NARG(__VA_ARGS__), __VA_ARGS__) 
#define TYPE_PREPENDN(N, ...) CAT(TYPE_PREPEND,N)(__VA_ARGS__) 
#define TYPE_PREPEND1(a) ATM_##a
#define TYPE_PREPEND2(a,b) ATM_##a | ATM_##b
#define TYPE_PREPEND3(a,b,c) ATM_##a | ATM_##b | ATM_##c
#define A0() 0, 0,0,0,0, 0,0,0,0
#define A1(t1, f1) 1, RUN(TYPE_PREPEND)t1,0,0,0, RUN(FLAG_PREPEND)(f1),0,0,0
#define A2(t1, f1, t2, f2) 2, RUN(TYPE_PREPEND)t1,RUN(TYPE_PREPEND)t2,0,0, RUN(FLAG_PREPEND)(f1),RUN(FLAG_PREPEND)(f2),0,0
#define A3(t1, f1, t2, f2, t3, f3) 3, RUN(TYPE_PREPEND)t1,RUN(TYPE_PREPEND)t2,RUN(TYPE_PREPEND)t3,0, RUN(FLAG_PREPEND)(f1),RUN(FLAG_PREPEND)(f2),RUN(FLAG_PREPEND)(f3),0

//  name, argc
#define ASM_LIST \
	X(nop, A0()) \
	X(newobj, A1((Var),W)) \
	X(newlist, A1((Var),W)) \
	X(lp, A2((Var),W, (String),RO)) \
	X(sp, A2((String),RO, (Any),RO)) \
	X(push, A1((Any),RO)) \
	X(pop, A1((Var),W)) \
	X(loop, A2((Label),RO, (Int,Var),W)) \
	X(endloop, A1((Label),RO)) \
	X(call, A1((Label),RO)) \
	X(ret, A0()) \
	X(mov, A2((Var),W, (Any),RO)) \
	X(add, A3((Var),W, (Int,Real,Var),RO, (Int,Real,Var),RO)) \
	X(sub, A3((Var),W, (Int,Real,Var),RO, (Int,Real,Var),RO)) \
	X(div, A3((Var),W, (Int,Real,Var),RO, (Int,Real,Var),RO)) \
	X(mod, A3((Var),W, (Int,Real,Var),RO, (Int,Real,Var),RO)) \
	X(mul, A3((Var),W, (Int,Real,Var),RO, (Int,Real,Var),RO)) \
	\
	X(pushv, A1((Var),RO)) \
	X(pushi, A1((Int),RO)) \
	X(pushr, A1((Real),RO)) \
	X(pushs, A1((String),RO)) \
	X(movv, A2((Var),W, (Var),RO)) \
	X(movi, A2((Var),W, (Int),RO)) \
	X(movr, A2((Var),W, (Real),RO)) \
	X(movs, A2((Var),W, (String),RO)) \
	X(addi, A3((Var),W, (Var),RO, (Int),RO)) \
	X(addr, A3((Var),W, (Var),RO, (Real),RO)) \
	X(addv, A3((Var),W, (Var),RO, (Var),RO)) \
	\
	X(jmp, A1((Int),RO)) \
	X(decjmp, A2((Var),W, (Int),RO)) \
	X(jz, A2((Var),RO, (Int),RO)) \



enum {
	ASM_NULL = 0,
#define X(a, ...) OP_##a,
	ASM_LIST
#undef X
	ASM_MAX_VALUE,
};


#define VAR_TYPE_LIST \
	X(Object) \
	X(ObjectList) \
	X(Function) \
	X(String) \
	X(Int) \
	X(Real) \
	X(Bool) \
	X(Label) \


#define ARG_TYPE_LIST \
	X(Var) \
	X(Int) \
	X(Real) \
	X(String) \
	X(Label) \



enum {
	ARG_TYPE_NONE = 0,
#define X(a,...) ARG_TYPE_##a,
	ARG_TYPE_LIST
#undef X
	ARG_TYPE_MAX_VALUE
};

enum {
	ATM_NONE = 0,
#define X(a,...) ATM_##a = (1ul << (ARG_TYPE_##a - 1)),
	ARG_TYPE_LIST
#undef X

	ATM_Any = ((1ul << ARG_TYPE_MAX_VALUE) - 1) & (~ATM_Label)
};


#define ARG_FLAG_LIST \
	X(W, write target) \
	X(RO, read only) \

enum {
	ARG_FLAG_NONE = 0,
#define X(a,...) ARG_FLAG_##a,
	ARG_FLAG_LIST
#undef X
	ARG_FLAG_MAX_VALUE
};

enum {
	AGF_N = 0,
#define X(a,...) AGF_##a = (1ul << (ARG_FLAG_##a - 1)),
	ARG_FLAG_LIST
#undef X
};


#define fdec(x) struct x; typedef struct x x;

fdec(Var)
fdec(Function)
fdec(Object)
fdec(ObjectList)


struct Var {
	char type;
	union {
		char* str;
		double r;
		long n;
		int b;
		Object* obj;
		ObjectList* list;
		Function* fn;
	};
};



typedef struct {
	unsigned char opcode;
	
} Instr;


struct Function {
	char* name;
	
	int numVars;
	
	unsigned char* code;
	size_t code_len;
	size_t code_alloc;
	// VEC(args), with possible validation functions
	// pops args from stack, pushes return vals 

};


struct Object {
	char* name;
	
	HT(Var) props;

};


struct ObjectList {
	VEC(Object*) list;
};



typedef struct State {

	VEC(Var) stack;
	
	// TODO:
	// objects
	// registers; per type? 
	

} State;



void compile(char* source, Function* fn);

void dump_code(Function* fn);

void exec_code(Function* fn);



#endif // __combat_vm__vm_h__

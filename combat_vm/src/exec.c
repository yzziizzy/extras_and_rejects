
#include "vm.h"
#include "compile.h"







typedef struct {
	VEC(Var) stack;
	Var* vars; // named

} econtext;


#define as_short (*(short*)(b+=2, b-2))
#define as_long (*(long*)(b+=8, b-8))
#define as_real (*(double*)(b+=8, b-8))


void exec_code(Function* fn) {
	int inum = 1;
	int n, t, dst, x, y;
	Var v;
	long target;
	
	econtext ecs = {0};
	econtext* ctx = &ecs;
	
	ctx->vars = calloc(1, sizeof(*ctx->vars) * fn->numVars + 1); 
	
	
	unsigned char* b = fn->code;
	while(b < fn->code + fn->code_len) {
		
		int opcode = b[0];
		b++;
		printf("#%d: %s\n", inum++, instruction_list[opcode].name);
		
		switch(opcode) {
			case OP_movi: t = ARG_TYPE_Int; goto MOVLONG;
			case OP_movr: t = ARG_TYPE_Real; goto MOVLONG;
			case OP_movs: t = ARG_TYPE_String; goto MOVLONG;
			MOVLONG:
				n = as_short;
				ctx->vars[n] = ((Var){.type = t, .n = as_long});
				break;
				
			case OP_movv:
				v = ctx->vars[as_short];
				ctx->vars[as_short] = v;
				break;
			
			case OP_pushi: t = ARG_TYPE_Int; goto PUSHLONG;
			case OP_pushr: t = ARG_TYPE_Real; goto PUSHLONG;
			case OP_pushs: t = ARG_TYPE_String; goto PUSHLONG;
			PUSHLONG:
				VEC_PUSH(&ctx->stack, ((Var){.type = t, .n = as_long}));
				break;
			
			case OP_pushv:
				v = ctx->vars[as_short]; 
				VEC_PUSH(&ctx->stack, v);
				break;
			
			case OP_pop:
				ctx->vars[as_short] = VEC_TAIL(&ctx->stack);
				VEC_LEN(&ctx->stack)--;
				break;
				
			
			case OP_addi: 
				dst = as_short;
				x = as_short;
				ctx->vars[dst].n = ctx->vars[x].n + as_long;   
				break;
			
			case OP_addr: 
				dst = as_short;
				x = as_short;
				ctx->vars[dst].r = ctx->vars[x].r + as_real;   
				break;
			
			case OP_addv: 
				dst = as_short;
				x = as_short;
				y = as_short;
				if(ctx->vars[x].type != ctx->vars[y].type) {
					printf("type mismatch in addv\n");
					exit(1);
				}
				if(ctx->vars[x].type == ARG_TYPE_Int) {
					ctx->vars[dst].n = ctx->vars[x].n + ctx->vars[y].n;
				}
				else if(ctx->vars[x].type == ARG_TYPE_Real) {
					ctx->vars[dst].r = ctx->vars[x].r + ctx->vars[y].r;
				}
				else {
					printf("Invalid variable type in addv\n");
					exit(1);
				}
				break;
			
			
			case OP_decjmp:
				dst = as_short;
				ctx->vars[dst].n = ctx->vars[dst].n <= 1 ? 0 : ctx->vars[dst].n - 1;
				b = fn->code + as_long;
				break;
				
			case OP_jz:
				x = as_short;
				target = as_long;
				if(ctx->vars[x].n == 0) {
					b = fn->code + target;
				}
				break;
			
			default:
				printf("unknown instruction\n");
				exit(1);
		}
		
		
		
	}


	
	printf("\n\nStack after execution:\n");
	
	VEC_EACH(&ctx->stack, i, v) {
		switch(v.type) {
			case ARG_TYPE_Int:
				printf("$%ld \n", v.n);
				b += 8;
				break;
				
			case ARG_TYPE_Real:
				printf("%%%f \n", v.r);
				b += 8;
				break;
				
			case ARG_TYPE_String:
				printf("\"%s\" \n", v.str);
				b += 8;
				break;
			
			case ARG_TYPE_Label:
				printf("%s \n", v.str);
				b += 8;
				break;
			
			default: 
				printf("unknown type\n");
		}
	}

}


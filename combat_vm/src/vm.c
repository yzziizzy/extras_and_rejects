


#include "vm.h"
#include "compile.h"



struct instruction_info instruction_list[] = {
	[0] = {0, "<invalid opcode>", 0, {0,0,0,0}, {0,0,0,0}},
#define X(...) Y(__VA_ARGS__)
#define Y(name, argc, a1t, a2t, a3t, a4t, a1f, a2f, a3f, a4f) [OP_##name] = { \
	OP_##name, #name, argc, \
	{a1t, a2t, a3t, a4t}, \
	{a1f, a2f, a3f, a4f} \
},
	
	ASM_LIST
#undef X	 
};



struct instr_conversion conversion_list[] = {
#define X1(o, t, n, ...) {OP_##o, {ARG_TYPE_##t, 0,0,0}, OP_##n},
#define X2(o, t1, t2, n, ...) {OP_##o, {ARG_TYPE_##t1, ARG_TYPE_##t2, 0,0}, OP_##n},
#define X3(o, t1, t2, t3, n, ...) {OP_##o, {ARG_TYPE_##t1, ARG_TYPE_##t2, ARG_TYPE_##t3, 0}, OP_##n},
#define X4(o, t1, t2, t3, t4, n, ...) {OP_##o, {ARG_TYPE_##t1, ARG_TYPE_##t2, ARG_TYPE_##t3, ARG_TYPE_##t4}, OP_##n},
	CONVERSION_LIST
#undef X1
#undef X2
#undef X3
#undef X4
};


int check_conversion(instr_info* ii) {
	for(int i = 0; i < sizeof(conversion_list) / sizeof(conversion_list[0]); i++) {
		struct instr_conversion* cv = conversion_list + i;
		if(cv->oldOp != ii->opcode) continue;
		
		// make sure the args ar comptible
		for(int j = 0; j < instruction_list[ii->opcode].argc; j++) {
			if(cv->argt[j] != 0 && !(ii->argt[j] == cv->argt[j])) {
				goto NOPE;
			}
		}
		
		//success
		ii->opcode = cv->newOp;
//		printf("converted %s to %s\n", instruction_list[cv->oldOp].name, instruction_list[cv->newOp].name);
		return 0;
	
	NOPE: // argument type mismatch
	}
	
	return 1;
}


int get_prop(Object* obj, Var* out) {
	return 0;
}





int get_var(context* ctx, char* name, int len) {
	char* n = strndup(name, len);
	
	int index = 0;
	if(HT_get(&ctx->varNameLookup, n, &index)) {
		variable_info info = {0};
	
		info.id = VEC_LEN(&ctx->varInfo);
		info.name = n;
		index = info.id;
		
		VEC_PUSH(&ctx->varInfo, info);
		HT_set(&ctx->varNameLookup, n, info.id);
	}

	return index;
}


size_t strspnident(char* s) {
	size_t i = 0;
	while(isalnum(*s) || *s == '_') s++, i++;
	
	return i;
}


int probe_var(context* ctx, char** s) {
	
	int len = strspnident(*s);
	if(len == 0) return 0;
	
	int x = get_var(ctx, *s, len);
	(*s) += len;
	
	return x;
}


int probe_instr(context* ctx, char** s, instr_info* ii) {
	int opcode = 0;
	
	// scan for instruction name
	int len = strspnident(*s);
	for(; opcode < ASM_MAX_VALUE; opcode++) {
		if(0 == strncmp(instruction_list[opcode].name, *s, len)) {
			goto FOUND;
		}
	}
	printf("Invalid instruction: '%.*s'\n", len, *s);
	return 1;
	
FOUND:
	ii->opcode = opcode;
	ii->argc = instruction_list[opcode].argc;
	(*s) += len;
	
	printf("found %s\n", instruction_list[opcode].name);
	
	// skip space
	while(**s && isspace(**s)) (*s)++;
	
	// scan for args
	for(int i = 0; i < ii->argc; i++) {
		int allowed_type = instruction_list[opcode].arg_type_mask[i];
		
		if((allowed_type & ATM_Int) && **s == '$') { // int literal
			char* e;
			ii->argv[i].n = strtol(*s + 1, &e, 0);
			ii->argt[i] = ARG_TYPE_Int;
			*s = e;
		}
		else if((allowed_type & ATM_Real) && **s == '%') { // real literal
			char* e;
			ii->argv[i].r = strtod(*s + 1, &e);
			ii->argt[i] = ARG_TYPE_Real;
			*s = e;
		}
		else if(allowed_type & ATM_Label) { // label
			int index;
			
			int len = strspnident(*s);
			char* n = strndup(*s, len);
			if(HT_get(&ctx->labelLookup, n, &index)) {
				index = ctx->labelLookup.base.fill;
//				dbg("index: %d", index)
				HT_set(&ctx->labelLookup, n, index);
			}
			ii->argv[i].n = index; 
			ii->argt[i] = ARG_TYPE_Label;
			*s += len;
		}
		else if(allowed_type & ATM_Var) { // variables
			int a = probe_var(ctx, s);
			if(!a) {
				printf("missing arg #%d in %s\n", i, instruction_list[opcode].name);
				return 1;
			}
			
			ii->argv[i].n = a;
			ii->argt[i] = ARG_TYPE_Var;
		}
		else {
			printf("broken argument\n");
			exit(1);
		}
		
		
		// skip space, commas
		while(**s && (**s == ' ' || **s == '\t' || **s == ',')) (*s)++;  
	}
	
	// skip to end of line
	while(**s && **s != '\n') (*s)++;
	
	return 0;
}


static void write_check(Function* fn, int n) {
	if(fn->code_alloc < fn->code_len + n) {
		fn->code_alloc *= 2; // this function never checks for large n
		fn->code = realloc(fn->code, fn->code_alloc);
	}
}


static void write_b(Function* fn, unsigned char b) {
	write_check(fn, 1);
	memcpy(fn->code + fn->code_len, &b, 1);
	fn->code_len += 1;
}
static void write_w(Function* fn, unsigned short w) {
	write_check(fn, 2);
	memcpy(fn->code + fn->code_len, &w, 2);
	fn->code_len += 2;
}
static void write_qp(Function* fn, void* p) {
	write_check(fn, 8);
	memcpy(fn->code + fn->code_len, p, 8);
	fn->code_len += 8;
}


// TODO: parse function decs, object defs, etc


void compile(char* source, Function* fn) {
	
	context* ctx = alloca(sizeof(*ctx));
	memset(ctx, 0, sizeof(*ctx));
	
	HT_init(&ctx->varNameLookup, 16);
	HT_init(&ctx->labelLookup, 16);
	VEC_INC(&ctx->varInfo);
	
	
	int dst, a, b;
	char* s = source;
	
	while(*s) {
		
		if(isspace(*s)) {
			s++;
			continue;
		}
		
		if(*s == '#') { // comment
			goto SKIP_LINE;
		}
		
		/*
		if(*s == ':') { // labels
		
			int len = strspnident(s + 1);
			if(len == 0) {
				printf("missing label text\n");
				exit(1);
			}
			
			char* label = strndup(s + 1, len);
			
			int index = 0;
			if(HT_get(&ctx->labels, label, &index)) {
				index = ctx->instr_num;
				HT_set(&ctx->labels, label, index);
			}
			else {
				printf("label aready exists\n");
				exit(1);
			}
		
			goto SKIP_LINE;
		}
		*/
		
		#define streq(x) (!strncmp(s, x, strlen(x)))
		
		instr_info ii = {0};
		if(!probe_instr(ctx, &s, &ii)) {
			
			check_conversion(&ii);
			
			VEC_PUSH(&ctx->iis, ii);
			
			
//			check_type_change(ctx);
			ctx->hasErrors &= check_immutable(ctx, &ii);
			
			
			continue;
		}
		
				
		
	SKIP_LINE:
		while(*s && *s != '\n') s++;
	}
	
	
	
	fix_loops(ctx, fn);
	
	
	
	VEC(long) labels;
	VEC_INIT(&labels);
	
	// write out the bytecode
	VEC_EACHP(&ctx->iis, in, ii) {
		
		ii->bytecode_offset = fn->code_len;
		// push the opcode
		dbg("%ld, write opcode %s at %ld", in, instruction_list[ii->opcode].name, fn->code_len);
		write_b(fn, ii->opcode);
		
		if(ii->argc == 0) continue;
		
		// encode argument type info
		int info = 0;
		for(int i = 0; i < ii->argc; i++) {
			if(popcount(instruction_list[ii->opcode].arg_type_mask[i]) <= 1) continue;
			printf("arg info written\n");
			info = ii->argt[i];
			write_b(fn, info);
			
			// should only every be two aruments, max, allowing multiple types
		}
		
		// write the arguments themselves
		for(int i = 0; i < ii->argc; i++) {
			
			if(ii->argt[i] == ARG_TYPE_Var) {
				write_w(fn, ii->argv[i].n);
			}
			else if(ii->argt[i] == ARG_TYPE_Int) {
				write_qp(fn, &ii->argv[i].n);
			}
			else if(ii->argt[i] == ARG_TYPE_Real) {
				write_qp(fn, &ii->argv[i].r);
			}
			else if(ii->argt[i] == ARG_TYPE_String) {
				write_qp(fn, &ii->argv[i].s);
			}
			else if(ii->argt[i] == ARG_TYPE_Label) {
				dbg("pushed addr %ld", fn->code_len);
				VEC_PUSH(&labels, fn->code_len);
				write_qp(fn, &ii->argv[i].n);
			}
		}
		
		//ctx->instr_num++;
	}
				

	// patch label addresses
	VEC_EACH(&labels, i, addr) {
		long label_index = *(long*)(fn->code + addr);
		
		instr_info* ii = &VEC_ITEM(&ctx->iis, label_index); 
		dbg("patched offset: %d", ii->bytecode_offset)
		*(long*)(fn->code + addr) = ii->bytecode_offset;
	}
	
	VEC_FREE(&labels);
	
	
	fn->numVars = VEC_LEN(&ctx->varInfo);
	
	

}



void dump_code(Function* fn) {
	int inum = 1;

	for(int i = 0; i < fn->code_len; i++) {
		printf("%.2x ", fn->code[i]);
	}
	
	printf("\n");
	
	unsigned char* b = fn->code;
	while(b < fn->code + fn->code_len) {
		
		// print opcode
		int opcode = b[0];
		printf("%d: %s ", inum++, instruction_list[opcode].name);
		
		// decode argument type info
		b++;
		int argt[4];
		for(int i = 0; i < instruction_list[opcode].argc; i++) {
			if(popcount(instruction_list[opcode].arg_type_mask[i]) <= 1) {
				argt[i] = ffs(instruction_list[opcode].arg_type_mask[i]);
			}
			else {
				argt[i] = *b;
				b++;
			}
			
//			printf("argtype: %d\n", argt[i]);
			// should only every be two aruments, max, allowing multiple types
		}
	
		
		for(int i = 0; i < instruction_list[opcode].argc; i++) {
			switch(argt[i]) {
				case ARG_TYPE_Int:
					printf("$%ld ", *((long*)b));
					b += 8;
					break;
					
				case ARG_TYPE_Real:
					printf("%%%f ", *((double*)b));
					b += 8;
					break;
					
				case ARG_TYPE_String:
					printf("\"%s\" ", *((char**)b));
					b += 8;
					break;
				
				case ARG_TYPE_Label:
					printf("%ld ", *((long*)b));
					b += 8;
					break;
				
				case ARG_TYPE_Var:
					printf("@%d ", *((short*)b));
					b += 2;
					break;
				
				default: 
					printf("unknown argument type\n");
			}
		}
		
		
		printf("\n");
	}


}








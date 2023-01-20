

#include "vm.h"
#include "compile.h"





// Loop variables aren't allowed to be modifed inside their loop bodies
int check_immutable(context* ctx, instr_info* ii) {
	return 0;
	
	/*
	int ret = 0;
	struct instruction_info* info = instruction_list + ii->opcode;
	
	for(int i = 0; i < info->argc; i++) {
		if(!(ii->argt[i] == ARG_TYPE_Var && info->arg_flags[i] & AGF_W)) continue;
		
		variable_info* vi = &VEC_ITEM(&ctx->varInfo, ii->argv[i].n);
		int ignore;
		if(!HT_get(&ctx->activeLoops, vi->name, &ignore)) {
			printf("error: loop variable modified\n");
			ret &= 1;
		}
	}
	
	if(ii->opcode == OP_loop) {
		variable_info* vi = &VEC_ITEM(&ctx->varInfo, ii->argv[1].n);
	
		int ignore = 0;
		HT_set(&ctx->activeLoops, vi->name, ignore);
	}
	else if(ii->opcode == OP_endloop) {
		variable_info* vi = &VEC_ITEM(&ctx->varInfo, ii->argv[1].n);
	
		HT_delete(&ctx->activeLoops, vi->name);
	}
	*/
//	return ret;
}




typedef struct {
	char active;
	int loopVar;
	int loopStart; /// the loop instruction's index
	int loopEnd; // the index of the loopend instruction itself
} label_meta;


// Loop variables aren't allowed to be modifed inside their loop bodies
int fix_loops(context* ctx, Function* fn) {
	int ret = 0;
	
	label_meta* meta = calloc(1, sizeof(*meta) * ctx->labelLookup.base.fill);
	
//	HT(int) activeLoops;
//	HT_init(&activeLoops, 16);
	
	VEC_EACHP(&ctx->iis, in, ii) {
		
		if(ii->opcode == OP_loop) {
			
			meta[ii->argv[0].n].active = 1;
			meta[ii->argv[0].n].loopStart = in;
			meta[ii->argv[0].n].loopVar = ii->argv[1].n;
			
			variable_info* vi = &VEC_ITEM(&ctx->varInfo, ii->argv[1].n);
			if(vi->activeInLoop) {
				printf("loop variable already used in outer loop\n");
				exit(1);
			}
			vi->activeInLoop = 1;
			
			
			// convert it to a jz on the loop var
		}
		else if(ii->opcode == OP_endloop) {
			int label_index = ii->argv[0].n;
			meta[label_index].active = 0;
			meta[label_index].loopEnd = in;
			
			variable_info* vi = &VEC_ITEM(&ctx->varInfo, meta[ii->argv[0].n].loopVar);
			if(!vi->activeInLoop) {
				printf("endloop without matching loopEnd\n");
				exit(1);
			}
			vi->activeInLoop = 0;
			
			// convert to decjmp to start with loop var
			ii->opcode = OP_decjmp;
			ii->argc = 2;
			ii->argv[0].n = meta[label_index].loopVar;
			ii->argt[0] = ARG_TYPE_Var;
			ii->argv[1].n = meta[label_index].loopStart;
			dbg("loopstart %d", meta[label_index].loopStart)
			ii->argt[1] = ARG_TYPE_Label;
			
			// convert the starting loop to jz
			instr_info* is = &VEC_ITEM(&ctx->iis, meta[label_index].loopStart);
			is->opcode = OP_jz;
			is->argv[0].n = meta[label_index].loopVar;
			is->argt[0] = ARG_TYPE_Var;
			is->argv[1].n = in + 1;
			is->argt[1] = ARG_TYPE_Label;
		}
		else {
		
		
			struct instruction_info* info = instruction_list + ii->opcode;
	
			for(int i = 0; i < info->argc; i++) {
				if(!(ii->argt[i] == ARG_TYPE_Var && info->arg_flags[i] & AGF_W)) continue;
				
				variable_info* vi = &VEC_ITEM(&ctx->varInfo, ii->argv[i].n);
				int ignore;
				if(vi->activeInLoop) {
					printf("error: loop variable modified\n");
					ret &= 1;
				}
			}
		}
		
	}
	
	
	return ret;
}











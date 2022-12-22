#include <stdio.h>


#include "tree.h"




void sq_insert_char(seqtree* tree, char c, float count) {
	
	VEC_EACHP(&tree->chars, i, ch) {
		if(ch->c == c) {
			ch->prob += count;
			return;
		}
	}
	
	VEC_PUSH(&tree->chars, ((seqchar){
		.c = c,
		.prob = count,
		.kids = NULL,
	}));
}



void sq_insert_seq(seqtree* tree, char* seq, int n, float count) {
	
	if(n <= 1) {
		sq_insert_char(tree, *seq, count);
		return;
	}
	
	
	// look for the right subtree to decend
	VEC_EACHP(&tree->chars, i, ch) {
		if(ch->c == *seq) {
			if(!ch->kids) {
				ch->kids = calloc(1, sizeof(*ch->kids));
			}
			
			sq_insert_seq(ch->kids, seq + 1, n - 1, count);
			return;
		}
	}
	
	// create a new subtree
	seqtree* k = calloc(1, sizeof(seqtree));
	
	VEC_PUSH(&tree->chars, ((seqchar){
		.c = *seq,
		.prob = 0,
		.kids = k,
	}));
	
	sq_insert_seq(k, seq + 1, n - 1, count);
}



void sq_total_and_invert(seqtree* tree) {
	
	float sum = 0;
	
	VEC_EACHP(&tree->chars, i, ch) {
		sum += ch->prob;
	}
	
	if(sum > 0) {	
		sum = 1.0 / sum;
		
		VEC_EACHP(&tree->chars, i, ch) {
			ch->prob *= sum;
		}
	}
	
	VEC_EACHP(&tree->chars, i, ch) {
		if(ch->kids) sq_total_and_invert(ch->kids);
	}
}


seqtree* sq_descend(seqtree* tree, char* seq, int depth) {
	if(depth == 0) return tree;
	
	VEC_EACHP(&tree->chars, i, ch) {
		if(ch->c == *seq) {
			if(!ch->kids) return NULL;
			
			return sq_descend(ch->kids, seq + 1, depth - 1);
		}
	}
	
	return NULL;
}


char sq_sample(seqtree* tree) {
	float pval = frandNorm();
	float sum = 0;
	
	VEC_EACHP(&tree->chars, i, ch) {
		sum += ch->prob;
		if(sum >= pval) {
			return ch->c;
		}
	}
	
	return -1;
}

char sq_sample_depth(seqtree* tree, char* seq, int depth) {
	seqtree* t = sq_descend(tree, seq, depth);
	if(!t) return -1;
	
	return sq_sample(t);
}


float sq_prob_depth(seqtree* tree, char* seq, int depth) {
	if(!tree) return -1;
		
	VEC_EACHP(&tree->chars, i, ch) {
		if(ch->c == *seq) {
			if(depth <= 1) {
				return ch->prob;
			}
			
			return sq_prob_depth(ch->kids, seq + 1, depth - 1);
		}
	}
	
	return -1;
}


long sq_memstats(seqtree* tree) {
	
	long sum = sizeof(*tree) + VEC_ALLOC(&tree->chars) * sizeof(VEC_ITEM(&tree->chars, 0));
	
	VEC_EACHP(&tree->chars, i, ch) {
		if(ch->kids) sum += sq_memstats(ch->kids);
	}

	return sum;
}


void sq_print_tree(seqtree* tree, int depth) {
	
	VEC_EACHP(&tree->chars, i, ch) {
		for(int i = 0; i < depth; i++) printf(" ");
		
		printf("%c - %f\n", ch->c, ch->prob);
		if(ch->kids) sq_print_tree(ch->kids, depth + 2);
	}
}






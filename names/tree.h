


#include "sti/sti.h"



struct seqtree;

typedef struct seqchar {
	char c;
	float prob;
	struct seqtree* kids;
} seqchar;


typedef struct seqtree {
	VEC(seqchar) chars;
} seqtree;







void sq_insert_char(seqtree* tree, char c, float count);
void sq_insert_seq(seqtree* tree, char* seq, int n, float count);
void sq_total_and_invert(seqtree* tree);
seqtree* sq_descend(seqtree* tree, char* seq, int depth);
char sq_sample(seqtree* tree);
char sq_sample_depth(seqtree* tree, char* seq, int depth);
float sq_prob_depth(seqtree* tree, char* seq, int depth);

long sq_memstats(seqtree* tree);
void sq_print_tree(seqtree* tree, int depth);


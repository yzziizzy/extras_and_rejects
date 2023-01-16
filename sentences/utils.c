

#include "defs.h"
#include "utils.h"


int word_exists_in_sentence_prior_to(sentence_info* s, int index) {
	
	int word = s->word_list[index];
	
	for(int i = index - 1; i >= 0; i--) {
		if(s->word_list[i] == word) return 1;
	}
	
	return 0;
}



int int_cmp(int* a, int* b) {
	return *a - *b;
}


void sort_words(int* words, int len) {
	qsort(words, len, sizeof(*words), (void*)int_cmp);
}















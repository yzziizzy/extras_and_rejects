#ifndef __utils_h__
#define __utils_h__


#include "defs.h"


static inline char* strtolower(char* s) {
	for(char* x = s; *x; x++) *x = tolower(*x);
	return s;
}



static inline double nrand() {
	return (double)rand() / (double)RAND_MAX;
}







int word_exists_in_sentence_prior_to(sentence_info* s, int index);

void sort_words(int* words, int len);


#endif // __utils_h__

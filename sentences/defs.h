#ifndef __defs_h__
#define __defs_h__


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


#include "sti/sti.h"


#define FOR(a, limit) for(long a = 0; a < limit; a++) 
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef unsigned char u8;

typedef struct {
	int n_into_book;
	int num_words;
	int num_sentences;
	int num_paragraphs;
	int start_word;
	int start_sentence;
	int start_paragraph;
} chapter_info;

typedef struct {
	int n_into_book;
	int n_into_chapter;
	int num_words;
	int num_sentences;
	int start_word;
	int start_sentence;
} paragraph_info;

typedef struct {
	int n_into_book;
	int n_into_chapter;
	int n_into_paragraph;
	int num_words;
	int start_word;
	int* word_list;
	
	int num_unique_words;
	int* unique_word_list; // sorted
} sentence_info;


typedef struct {
	char* text;
	int ordinal;
	int count;
	int starts_sentence;
	int starts_paragraph;
	int starts_chapter;
	
	
	float* follows; // this uses about 220mb
} word_stats;



typedef struct {
	HT(word_stats*) word_lookup;

	long words_alloc;
	long words_len;
	word_stats* words;
	
	long word_list_alloc;
	long word_list_len;
	int* word_list;
	
	long sentence_list_alloc;
	long sentence_list_len;
	sentence_info* sentence_list;
	
	long paragraph_list_alloc;
	long paragraph_list_len;
	paragraph_info* paragraph_list;
	
	long chapter_list_alloc;
	long chapter_list_len;
	chapter_info* chapter_list;
} book_info;





#endif //__defs_h__

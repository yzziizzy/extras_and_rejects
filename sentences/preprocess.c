

#include "defs.h"
#include "utils.h"





int main(int argc, char* argv[]) {
	
	string_internment_table_init(&global_string_internment_table);
	
	srand(argc > 1 ? atoi(argv[1]) : 0);
	
	book_info* bi = calloc(1, sizeof(*bi));
	
	HT_init(&bi->word_lookup, 1024 * 16);
	

	char* src = read_whole_file("watwe.txt", NULL);
	bi->words_alloc = 1024 * 8;
	bi->words_len = 0;
	bi->words = calloc(1, sizeof(*bi->words) * bi->words_alloc);
	
	
	bi->word_list_alloc = 1024 * 256;
	bi->word_list_len = 0;
	bi->word_list = calloc(1, sizeof(*bi->word_list) * bi->word_list_alloc);
	
	bi->sentence_list_alloc = 1024 * 8;
	bi->sentence_list_len = 0;
	bi->sentence_list = calloc(1, sizeof(*bi->sentence_list) * bi->sentence_list_alloc);
	
	bi->paragraph_list_alloc = 1024 * 4;
	bi->paragraph_list_len = 0;
	bi->paragraph_list = calloc(1, sizeof(*bi->paragraph_list) * bi->paragraph_list_alloc);
	
	bi->chapter_list_alloc = 256;
	bi->chapter_list_len = 0;
	bi->chapter_list = calloc(1, sizeof(*bi->chapter_list) * bi->chapter_list_alloc);
	
	chapter_info* chapter = bi->chapter_list;
	paragraph_info* paragraph = bi->paragraph_list;
	sentence_info* sentence = bi->sentence_list;
	
	sentence->word_list = bi->word_list + bi->word_list_len; 
	
//	strtolower(src);
	
	int words_into_chapter = 0;
	int words_into_paragraph = 0;
	int words_into_sentence = 0;
	
	int sentences_into_chapter = 0;
	int sentences_into_paragraph = 0;

	int paragraphs_into_chapter = 0;
	
	int chapters_into_book = 0;
	
	
	char* s = src;
	while(*s) {
		if(s[0] == '\n' && s[1] == '\n') {
			paragraph++;
			bi->paragraph_list_len++;
			
			paragraph->num_words = words_into_paragraph;
			paragraph->num_sentences = sentences_into_paragraph;
			paragraph->start_word = bi->word_list_len;
			paragraph->start_sentence = bi->sentence_list_len;
			
			words_into_sentence = 0;
			words_into_paragraph = 0;
			
			sentences_into_paragraph = 0;
			
			paragraphs_into_chapter++;
			
			while(*s == '\n') s++;
		}
		if(isspace(*s)) {
			s++;
			continue;
		}
		
		if(strchr(",\";:", *s)) {
			s++;		
			continue;
		}
		
		if(strchr(".?!", *s)) {
			sentence++;
			bi->sentence_list_len++;
			words_into_sentence = 0;
			
			sentences_into_chapter++;
			sentences_into_paragraph++;
						
			sentence->start_word = bi->word_list_len;
			sentence->word_list = bi->word_list + bi->word_list_len; 
			
			s++;		
			continue;
		}
		
		if(isalnum(*s)) {
			if(*s == 'C' && s[1] == 'H' && !strncmp(s, "CHAPTER ", strlen("CHAPTER "))) {
				chapters_into_book++;
				chapter++;
				bi->chapter_list_len++;
				
				words_into_sentence = 0;
				words_into_paragraph = 0;
				words_into_chapter = 0;
				sentences_into_chapter = 0;
				sentences_into_paragraph = 0;
				paragraphs_into_chapter = 0;
				
				chapter->start_word = bi->word_list_len;
				chapter->start_sentence = bi->sentence_list_len;
				chapter->start_paragraph = bi->paragraph_list_len;
			
				
				while(*s != '\n') s++; // skip the "chapter" line
				s++;
				while(*s != '\n') s++; // skip the next blank line
				s++;
				while(*s != '\n') s++; // skip the teaser line
				s++;
				continue;
			}
			
			if(*s == 'B' && s[1] == 'O' && !strncmp(s, "BOOK ", strlen("BOOK "))) {
				words_into_sentence = 0;
				words_into_paragraph = 0;
				words_into_chapter = 0;
				sentences_into_chapter = 0;
				sentences_into_paragraph = 0;
				paragraphs_into_chapter = 0;
				
				while(*s != '\n') s++; // skip the "book" line
				s++;
				while(*s != '\n') s++; // skip the next blank line
				s++;
				while(*s != '\n') s++; // skip the teaser line
				s++;
				continue;
			}
		
			char* begin = s;
			
			while(isalnum(*s) || *s == '\'') s++;
		
			int len = s - begin;
			
			char* w = strtolower(strnint(begin, len));
			
			
			word_stats* st;
			if(HT_get(&bi->word_lookup, w, &st)) {
				st = bi->words + bi->words_len;
				
				HT_set(&bi->word_lookup, w, st);
				st->text = w;
				st->ordinal = bi->words_len++;
			}
			st->count++;

			bi->word_list[bi->word_list_len++] = st->ordinal;

			if(words_into_sentence == 0) st->starts_sentence++;
			if(words_into_paragraph == 0) st->starts_paragraph++;
			if(words_into_chapter == 0) st->starts_chapter++;
			
			if(words_into_sentence == 0) printf("\n%d:%d ", chapters_into_book, sentences_into_paragraph);
			if(words_into_sentence < 4) printf("%d(%s) ", words_into_sentence, w);
			
			sentence->num_words++;
			paragraph->num_words++;
			chapter->num_words++;

			words_into_sentence++;
			words_into_paragraph++;
			words_into_chapter++;
				
			continue;
		}
		
		
		s++;
	}
	
	// shorthands and settings
	int nwords = bi->words_len;
	float dist_factor = 1;
	
	// calculate the probability of a word following other words.
	FOR(wi, bi->words_len) {
		bi->words[wi].follows = calloc(1, sizeof(*bi->words[wi].follows) * nwords);
	}
	

	sentence_info* s0 = bi->sentence_list; // this sentence 
	sentence_info* s1 = bi->sentence_list; // prior sentence
	sentence_info* s2 = bi->sentence_list; // two sentences ago
	
	FOR(si, bi->sentence_list_len) {
		
		FOR(wi, s0->num_words) {
			word_stats* word = &bi->words[s0->word_list[wi]]; 
			
			if(s2 != s1) {
				FOR(w2i, s2->num_words) {
					int dist = wi + s0->start_word - s2->start_word - w2i;
					
					word->follows[s2->word_list[w2i]] += (dist_factor / dist);
				}
			}
			if(s1 != s0) {
				FOR(w1i, s1->num_words) {
					int dist = wi + s0->start_word - s1->start_word - w1i;
					
					word->follows[s1->word_list[w1i]] += (dist_factor / dist);
				}
			}
			
			
			FOR(w0i, wi - 1) {
				int dist = wi /*+ s0->start_word - s0->start_word */ - w0i;
				
				word->follows[s0->word_list[w0i]] += (dist_factor / dist);
			}
		}
		
	
		s2 = s1;
		s1 = s0;
		s0++;
	}
	
	// normlize the probabilities
	FOR(wi, nwords) {
		float total = 0;
		FOR(i, nwords) total += bi->words[wi].follows[i];
		total = 1.0 / total; 
		FOR(i, nwords) bi->words[wi].follows[i] *= total;
	}
	
	
	// collect a list of unique words in each sentence
	// this list is used as an index later
	int most_uniques = 0;
	
	sentence = bi->sentence_list;
	FOR(si, bi->sentence_list_len) {
		
		FOR(wi, sentence->num_words) {
			if(!word_exists_in_sentence_prior_to(sentence, wi)) sentence->num_unique_words++;
		}
		
		sentence->unique_word_list = calloc(1, sizeof(*sentence->unique_word_list) * sentence->num_unique_words);
		
		int n = 0;
		FOR(wi, sentence->num_words) {
			if(!word_exists_in_sentence_prior_to(sentence, wi)) {
				sentence->unique_word_list[n++] = sentence->word_list[wi];
			}
		}
		
		most_uniques = MAX(most_uniques, sentence->num_unique_words);
		sort_words(sentence->unique_word_list, sentence->num_unique_words);
		
		/*
		FOR(a, sentence->num_unique_words) {
			printf("%d:%s ", sentence->unique_word_list[a], bi->words[sentence->unique_word_list[a]].text);
		}
		printf("\n\n");
		*/
		
		sentence++;
	}
	
	
	sentence = bi->sentence_list;
	FOR(si, bi->sentence_list_len) {
		
		FOR(wi, sentence->num_words - 1) {
			if(!word_exists_in_sentence_prior_to(sentence, wi)) sentence->num_unique_words++;
		}
		
		
		sentence++;
	}
	
	printf("\n");
	
	printf("unique word count: %ld\n", bi->word_lookup.base.fill);
	printf("total word count: %ld\n", bi->word_list_len);
	printf("total sentence count: %ld\n", bi->sentence_list_len);
	printf("total paragraph count: %ld\n", bi->paragraph_list_len);
	printf("total chapter count: %ld\n", bi->chapter_list_len);
	
	int sstarters = 0;
	HT_EACH(&bi->word_lookup, k, word_stats*, st) {
		if(st->starts_sentence > 0) sstarters++;
	}
	
	printf("sentence start words: %d\n", sstarters);
	printf("most unique words in a sentence: %d\n", most_uniques);
	
}










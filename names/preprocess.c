#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>



#include "sti/sti.h"

#define double float

char* strtolower(char* s) {
	for(char* x = s; *x; x++) *x = tolower(*x);
	return s;
}


typedef struct namestats {
	char* name;
	long cnt;
	char gender; // m,f
} namestats; 


double nrand() {
	return (double)rand() / (double)RAND_MAX;
}


typedef struct {
	double total;
	double total_pairs;
	double total_triplets;
	
	double lengths[15]; 
	double positions[15][26]; 
	double positions_totals[15]; 
	double pairs[26][26];
	double pairs_totals[26];
	double triplets[26][26][26];
	double triplets_totals[26][26];
	double quads[26][26][26][26];
	double quads_totals[26][26][26];
	double quints[26][26][26][26][26];
	double quints_totals[26][26][26][26];
} letter_stats;


#define FOR(a, limit) for(long a = 0; a < limit; a++) 

char letter_2(letter_stats* ls, char prev) {
	double letter_p = nrand();
	
	double sum = 0;
	FOR(a, 26) {
		sum += ls->pairs[(int)prev - 'a'][a];
		if(sum >= letter_p) {
			return (char)a + 'a';
		}
	}
	
	return 'z';
}

char letter_3(letter_stats* ls, char minus2, char minus1) {
	double letter_p = nrand();
	
	double sum = 0;
	FOR(a, 26) {
		sum += ls->triplets[(int)minus2 - 'a'][(int)minus1 - 'a'][a];
//		printf("%d, %d sum: %f of %f\n", minus2, minus1, sum, letter_p);
		if(sum >= letter_p) {
			return (char)a + 'a';
		}
	}
	
	return -1;
}

char letter_4(letter_stats* ls, char minus3, char minus2, char minus1) {
	double letter_p = nrand();
	
	double sum = 0;
	FOR(a, 26) {
		sum += ls->quads[(int)minus3 - 'a'][(int)minus2 - 'a'][(int)minus1 - 'a'][a];
//		printf("%d, %d sum: %f of %f\n", minus2, minus1, sum, letter_p);
		if(sum >= letter_p) {
			return (char)a + 'a';
		}
	}
	
	return -1;
}

char letter_5(letter_stats* ls, char minus4, char minus3, char minus2, char minus1) {
	double letter_p = nrand();
	
	double sum = 0;
	FOR(a, 26) {
		sum += ls->quints[(int)minus4 - 'a'][(int)minus3 - 'a'][(int)minus2 - 'a'][(int)minus1 - 'a'][a];
//		printf("%d, %d sum: %f of %f\n", minus2, minus1, sum, letter_p);
		if(sum >= letter_p) {
			return (char)a + 'a';
		}
	}
	
	return -1;
}

char positional(letter_stats* ls, int i) {
	double letter_p = nrand();
			
	double sum = 0;
	FOR(a, 26) {
		sum += ls->positions[i][a];
		if(sum >= letter_p) {
			return (char)a + 'a';
			break;
		}
	}

	return 'z';
}


int main(int argc, char* argv[]) {
	
	srand(argc > 1 ? atoi(argv[1]) : 0);
	
	HT(namestats*) m_stats;
	HT(namestats*) f_stats;
	HT_init(&m_stats, 4096*8);
	HT_init(&f_stats, 4096*8);
	
	rglob gn_files;
	
	recursive_glob("./data/", "yob*.txt", 0, &gn_files);
	
	
	
	for(int fi = 0; fi < gn_files.len; fi++) {
		size_t line_cnt;
		
		char* src = read_whole_file(gn_files.entries[fi].full_path, NULL);
		char** lines = strsplit_inplace(src, '\n', &line_cnt);
		
		for(char** line = lines; *line; line++) {
			
			char* e = strchr(*line, ',');
			char* name = strndup(*line, e - *line);
			
			*line = e + 1;
			int female = **line == 'F';
			
			long cnt = strtol(*line + 2, NULL, 10);
			
//			printf("'%s', female? %d, cnt: %ld\n", name, female, cnt);
			
			namestats* p = NULL;
			if(!female) {
				if(HT_get(&m_stats, name, &p)) {
					p = calloc(1, sizeof(*p));
					p->name = strtolower(strdup(name));
					p->gender = 'm';
					HT_set(&m_stats, name, p);
				}
			}
			else {
				if(HT_get(&f_stats, name, &p)) {
					p = calloc(1, sizeof(*p));
					p->name = strtolower(strdup(name));
					p->gender = 'f';
					HT_set(&f_stats, name, p);
				}
			}
			
			p->cnt += cnt;		
		
			free(name);
		}
		
//		printf("%s: %ld lines\n", gn_files.entries[fi].file_name, line_cnt);
		free(lines);
		free(src);
		
//		break;
	}
	
	if(gn_files.entries) free(gn_files.entries);

	
	letter_stats* ls = calloc(1, sizeof(*ls));
//	memset(&ls, 0, sizeof(ls));


	HT_EACH(&m_stats, name, namestats*, st) {
//		if(st->cnt == 1337 || st->cnt == 31337 || st->cnt == 42069) printf("%s: %ld\n", st->name, st->cnt);
		
		for(char* s = st->name; s[0] && s[1]; s++) {
			ls->pairs[s[0] - 'a'][s[1] - 'a'] += st->cnt;
			ls->pairs_totals[s[0] - 'a'] += st->cnt;
			ls->total_pairs += st->cnt;
		}
		
		for(char* s = st->name; s[0] && s[1] && s[2]; s++) {
			ls->triplets[s[0] - 'a'][s[1] - 'a'][s[2] - 'a'] += st->cnt;
			ls->triplets_totals[s[0] - 'a'][s[1] - 'a'] += st->cnt;
		}
		
		for(char* s = st->name; s[0] && s[1] && s[2] && s[3]; s++) {
			ls->quads[s[0] - 'a'][s[1] - 'a'][s[2] - 'a'][s[3] - 'a'] += st->cnt;
			ls->quads_totals[s[0] - 'a'][s[1] - 'a'][s[2] - 'a'] += st->cnt;
		}
		
		for(char* s = st->name; s[0] && s[1] && s[2] && s[3] && s[4]; s++) {
			ls->quints[s[0] - 'a'][s[1] - 'a'][s[2] - 'a'][s[3] - 'a'][s[4] - 'a'] += st->cnt;
			ls->quints_totals[s[0] - 'a'][s[1] - 'a'][s[2] - 'a'][s[3] - 'a'] += st->cnt;
		}
		
		for(int i = 0; st->name[i]; i++) {
			ls->positions[i][st->name[i] - 'a'] += st->cnt;
			ls->positions_totals[i] += st->cnt;
		}
		
		
		long l = strlen(st->name);
		ls->lengths[l] += st->cnt;
		ls->total += st->cnt;
		
//		minlen = fmin(strlen(st->name), minlen);
//		maxlen = fmax(strlen(st->name), maxlen);
	}
	
//	double inv_pairs = 1.0 / total_pairs;
	
	double inv_total = 1.0 / ls->total;
	
	for(int i = 0; i < 15; i++) {
		ls->lengths[i] *= inv_total;
	
		double inv = 1.0 / ls->positions_totals[i];
		
		for(int a = 0; a < 26; a++) {
			ls->positions[i][a] *= inv;
		}
	}
	
	int full_q = 0;
	int empty_q = 0;
	
	for(int a = 0; a < 26; a++) {
		double inv = 1.0 / ls->pairs_totals[a];
		
		for(int b = 0; b < 26; b++) {
			ls->pairs[a][b] *= inv;
			
			double inv_tri = 1.0 / ls->triplets_totals[a][b];
			FOR(c, 26) {
				ls->triplets[a][b][c] *= inv_tri;
				
				double inv_quad = 1.0 / ls->quads_totals[a][b][c];
				FOR(d, 26) {
					ls->quads[a][b][c][d] *= inv_quad;
				
					double inv_quint = 1.0 / ls->quints_totals[a][b][c][d];
					FOR(e, 26) {
						if(ls->quints[a][b][c][d][e] > 0) full_q++;
						else empty_q++;
						ls->quints[a][b][c][d][e] *= inv_quint;
					}
				}
			}
//			printf("%c%c: %f\n", a + 'a', b + 'a', ls.pairs[a][b]);
		}
	}
	
	printf("full/empty: %d, %d (%.2f%%)\n\n", full_q, empty_q, (full_q * 100. / empty_q));
	
	
	FOR(x, 20) {
	
		double genlen_p = nrand();
		int genlen = 0;
		double sum = 0;
		FOR(i, 15) {	
			sum += ls->lengths[i];
			if(sum >= genlen_p) {
				genlen = i;
				break;
			}
//			printf("%f\n", sum);
		}
		
		char name[15];
		
		name[0] = positional(ls, 0);
		printf("%c", name[0] + 'A' - 'a');
		
		name[1] = letter_2(ls, name[0]);
		printf("%c", name[1]);
		
		name[2] = letter_3(ls, name[0], name [1]);
		printf("%c", name[2]);
		
		name[3] = letter_4(ls, name[0], name [1], name [2]);
		printf("%c", name[3]);
		
		for(int i = 4; i < genlen; i++) {
			name[i] = letter_5(ls, name[i - 4], name[i - 3], name[i - 2], name[i - 1]);
			if(name[i] < 0) {
				break;
				
				name[i] = letter_4(ls, name[i - 3], name[i - 2], name[i - 1]);
			
				if(name[i] < 0) { 
//					break;
					name[i] = letter_3(ls, name[i - 2], name[i - 1]);
					
					if(name[i] < 0) break;
				}
			}
			printf("%c", name[i]);
		}
	
		printf("  - %d, %.15f\n", genlen, genlen_p);
	
	
	}

	printf("\nmemory used: %ld mb\n", sizeof(*ls) / (1024*1024));
}











/* ----- strlist.c ----- */

#include <string.h>
#include <stdlib.h>

#define check_alloc(x) \
	if((x)->len + 1 >= (x)->alloc) { \
		(x)->alloc *= 2; \
		(x)->entries = realloc((x)->entries, (x)->alloc * sizeof(*(x)->entries)); \
	}
	


typedef struct strlist {
	int len;
	int alloc;
	char** entries;
} strlist;



void strlist_init(strlist* sl) {
	sl->len = 0;
	sl->alloc = 32;
	sl->entries = malloc(sl->alloc * sizeof(*sl->entries));
}

strlist* strlist_new() {
	strlist* sl = malloc(sizeof(*sl));
	strlist_init(sl);
	return sl;
}

void strlist_push(strlist* sl, char* e) {
	check_alloc(sl);
	sl->entries[sl->len++] = e;
	sl->entries[sl->len] = 0;
}


void strlist_concat(strlist* to, strlist* from) {
	for(int i = 0; i < from->len; i++) {
		strlist_push(to, from->entries[i]);
	}
}


char* strlist_join(strlist* list, char* joiner) {
	size_t list_len = 0;
	size_t total = 0;
	size_t jlen = strlen(joiner);
	
	if(list == NULL || list->len == 0) return strdup("");
	
	// calculate total length
	for(int i = 0; i < list->len; i++) {
		list_len++;
		total += strlen(list->entries[i]);
	}
	
	total += (list_len - 1) * jlen;
	char* out = malloc((total + 1) * sizeof(*out));
	
	char* end = out;
	for(int i = 0; i < list->len; i++) {
		char* s = list->entries[i];
		size_t l = strlen(s);
		
		if(i > 0) {
			memcpy(end, joiner, jlen);
			end += jlen;
		}
		
		if(s) {
			memcpy(end, s, l);
			end += l;
		}
		
		total += strlen(s);
	}
	
	*end = 0;
	
	return out;
}

// splits on whitespace
char** strsplit_exact(char* splitters, char* in) {
	char* e;
	int alloc = 32;
	int len = 0;
	char** list = malloc(alloc * sizeof(*list)); 
	
	for(char* s = in; *s;) {
		e = strpbrk(s, splitters);
		if(!e) e = s + strlen(s);
		
		if(len >= alloc - 1) {
			alloc *= 2;
			list = realloc(list, alloc* sizeof(*list));
		}
		
		list[len++] = strndup(s, e - s);
		
//		e += strspn(e, splitters); // this is the difference
		e++;
		s = e;
	}
	
	list[len] = NULL;
	
	return list;
}


/* -END- strlist.c ----- */


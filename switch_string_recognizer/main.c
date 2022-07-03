#include <stdio.h>
#include <stdlib.h>
#include <string.h>



char* type_prefix = "OP_";
char* indent_str = "\t";
char* input_var = "s";
char* output_var = "type";



typedef struct conf {
	char* str;
	char* name;
} conf;

conf strings[] = {
	{"+", "PLUS"},
	{"-", "MINUS"},
	{"*", "MUL"},
	{"/", "DIV"},
	{"%", "MOD"},
	{"&", "BIT_AND"},
	{"|", "BIT_OR"},
	{"^", "BIT_XOR"},
	{"~", "BIT_NOT"},
	{"!", "LOGIC_NOT"},
	{"&&", "LOGIC_AND"},
	{"||", "LOGIC_OR"},
	{">", "GT"},
	{">=", "GTE"},
	{">>", "SHR"},
	{"<", "LT"},
	{"<=", "LTE"},
	{"<<", "SHL"},
	{"==", "EQ"},
	{"!=", "NEQ"},
	{"(", "LPAREN"},
	{")", "RPAREN"},
	{NULL, NULL},
};



typedef struct node {
	int c;
	char* name;
	
	struct node* kids;
	struct node* sibling;
} node;


node* mk_node(int c) {
	node* n = calloc(1, sizeof(*n));
	n->c = c;
	return n;
}


void add_str(node* n, char* s, char* name) {
	if(s[0] == 0) {
		if(n->name == NULL) {
			n->name = name;
			return;
		}
		
		printf("overlapping strings: %s, %s\n", name, n->name);
	}
	
	node* k = n->kids;
	while(k && k->c != s[0]) k = k->sibling;
	
	
	if(!k) {
		k = mk_node(s[0]);
		k->sibling = n->kids;
		n->kids = k;
	}
	
	add_str(k, s + 1, name); 
}


void pi(int indent) {
	for(int i = 0; i < indent; i++) printf("%s", indent_str);
}


void print_lvl(node* n, int c_index, int indent) {

	pi(indent); printf("switch(%s[%d]) {\n", input_var, c_index);
	
	for(node* k = n->kids; k; k = k->sibling) {
		pi(indent + 1); printf("case '%c':", k->c);
		if(k->kids) {
			printf("\n");
			print_lvl(k, c_index + 1, indent + 2);
			pi(indent + 2); printf("break;\n");
		}
		else {
			printf(" %s = %s%s; break;\n", output_var, type_prefix, k->name);
		}
	}
	
	if(n->name) {
		pi(indent + 1); printf("default: %s = %s%s; break;\n", output_var, type_prefix, n->name);
	}
	
	pi(indent); printf("}\n");
}


int main() {
	
	// half-initialize the root to make life easier
	node* root = mk_node(strings[0].str[0]);
	
	for(conf* c = strings; c->str; c++) {
		add_str(root, c->str, c->name);
	}
	
	
	print_lvl(root, 0, 0);
	
	


	return 0;
}











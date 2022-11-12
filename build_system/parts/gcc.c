
/* ----- gcc.c ----- */


strlist compile_cache;





int compile_cache_execute() {
	int ret = 0;
	struct child_process_info** cpis;
//	printf("compile cache length %d", compile_cache.len);

	ret = execute_mt(&compile_cache, g_nprocs, "Compiling...              %s", &cpis);
	
	if(ret) {
		for(int i = 0; i < compile_cache.len; i++ ) {
			if(cpis[i]->exit_status) {
				printf("%.*s\n", (int)cpis[i]->buf_len, cpis[i]->output_buffer);
			}
		}
	
	}
	
	// TODO free compile cache
	// TODO free cpis

	return ret;
}



strlist* parse_gcc_dep_file(char* dep_file_path, time_t* newest_mtime) {
	size_t dep_src_len = 0;
	strlist* dep_list;
	time_t newest = 0;
	
	char* dep_src = read_whole_file(dep_file_path, &dep_src_len);
	if(!dep_src) return NULL;
	
	dep_list = strlist_new();
	
	// skip the first filename junk
	char* s = strchr(dep_src, ':');
	s++;
	
	int ret = 0;
	
	// gather dep strings, ignoring line continuations
	while(*s) {
		do {
			s = strskip(s, " \t\r\n");
			if(*s == '\\') {
				 if(s[1] == '\r') s++;
				 if(s[1] == '\n') s++;
			}
		} while(isspace(*s));
		
		int dlen = span_path(s);
		if(dlen == 0) break;
		
		time_t dep_mtime;
		char* dep_fake = strncache(s, dlen);
		char* dep_real = resolve_path(dep_fake, &dep_mtime);
		if(dep_mtime > newest) newest = dep_mtime;
	
		strlist_push(dep_list, dep_real);
		
		struncache(dep_fake);
		struncache(dep_real);
		
		s += dlen;
	}
	

	free(dep_src);
	
	if(newest_mtime) *newest_mtime = newest;
	return dep_list;
}

int gen_deps(char* src_path, char* dep_path, time_t src_mtime, time_t obj_mtime) {
	time_t dep_mtime = 0;
	time_t newest_mtime = 0;
	
	char* real_dep_path = resolve_path(dep_path, &dep_mtime);
	if(dep_mtime < src_mtime) {
		//gcc -MM -MG -MT $1 -MF "build/$1.d" $1 $CFLAGS $LDADD
//		printf("  generating deps\n"); 
		char* cmd = sprintfdup("gcc -MM -MG -MT '' -MF %s %s %s", dep_path, src_path, g_gcc_opts_flat);
		system(cmd);
		free(cmd);
	}
	
	strlist* deps = parse_gcc_dep_file(real_dep_path, &newest_mtime);
	
	// free or process deps
	
	return newest_mtime > obj_mtime;
FAIL:
	return 0;
}


/* -END- gcc.c ----- */




/* ----- spirv.c ----- */

// GLSL -> SPIRV processor






#define SHADER_TYPE_LIST \
	X(global) \
	X(vert) \
	X(tesc) \
	X(tese) \
	X(geom) \
	X(frag) \
	X(comp) \
	X(mesh) \
	X(task) \
	
enum {
#define X(a, ...) SH_##a,
	SHADER_TYPE_LIST
#undef X
	SH_MAX_VALUE,
};



typedef struct {
	int line;
	char* file;
} source_line;

typedef struct glsl_source {
	char* rel_path;
	char* filename;
	char* temp_file;
	
	strlist lines;
	
	source_line* meta;
	int meta_len;
	int meta_alloc;
	
	int file_index;
	
	FILE* fdeps;
} glsl_source;


void push_meta(glsl_source* src, int line, char* file) {
	if(src->meta_len >= src->meta_alloc) {
		if(src->meta_alloc == 0) src->meta_alloc = 128;
		src->meta_alloc *= 2;
		src->meta = realloc(src->meta, sizeof(*src->meta) * src->meta_alloc);
	}
	
	src->meta[src->meta_len++] = (source_line){
		.line = line,
		.file = file,
	};
}

void glsl_source_concat(glsl_source* to, glsl_source* from) {
	strlist_concat(&to->lines, &from->lines);
	for(int i = 0; i < from->meta_len; i++) {
		push_meta(to, from->meta[i].line, from->meta[i].file);
	}
}


// TODO: relative nested includes
int glsl_append_file(glsl_source* parent, char* file_path) {
	
	fprintf(parent->fdeps, "%s\n", file_path);
	
	size_t flen = 0;
	char* fsrc = read_whole_file(file_path, &flen);
	
	if(!fsrc) return 1;
	
	char** lines = strsplit_exact("\n", fsrc);
	
	int file_index = ++parent->file_index;
	
	int line_num = 1;
	for(char** s = lines; *s; s++, line_num++) {
//		push_meta(parent, line_num, file_path); 
		if(strspn(*s, " \t") == strlen(*s)) continue;
	
		if(0 == strncmp(*s, "#include", strlen("#include"))) {
			char* fn = strchr(*s, '"') + 1;
			char* e = strchr(fn, '"');
			char* include_fn = strndup(fn, e - fn);
			
			strlist_push(&parent->lines, sprintfdup("// %s", *s)); 
			glsl_append_file(parent, include_fn);
			continue;
		}
		
		strlist_push(&parent->lines, sprintfdup("#line %d %d", parent->meta_len, line_num)); 
		strlist_push(&parent->lines, *s);
		push_meta(parent, line_num, file_path); 
	}
	
	strlist_push(&parent->lines, sprintfdup("// end #include %s", file_path)); 
	
	free(fsrc);
	
	return 0;
}



int glsl_to_spirv(char* base_path, char* rel_path, char* filename, char* build_base_path) {
	time_t src_mtime = 0, obj_mtime = 0, deps_mtime = 0;
	int ret = 0;
	
	char* path = path_join(base_path, rel_path, filename);
	char* real_src_path = resolve_path(path, &src_mtime);
	
	char* temp_path = path_join(build_base_path, rel_path);
	mkdirp_cached(temp_path, 0777);
	
	char* deps_path = sprintfdup("%s/%s.deps", temp_path, filename);
	char* real_deps_path = resolve_path(deps_path, &deps_mtime);
	
	int deps_invalid = src_mtime == 0;
	if(deps_mtime < src_mtime) {
		deps_invalid = 1;
	}
	
	if(!deps_invalid) {
		
		// check deep deps times
		char* dsrc = read_whole_file(real_deps_path, NULL);
//		printf("dsrc: %p\n", dsrc);
		
		char** dlines = strsplit("\n", dsrc);
		
		for(int i = 0; dlines[i]; i++) { 
			time_t subdep_mtime = 0;
			char* real_dep_path = resolve_path(dlines[i], &subdep_mtime);
			
			if(deps_mtime < subdep_mtime) {
//				printf("subdep mtime failure: %s\n", dlines[i]);
				deps_invalid = 1;
				break;
			}
		}
	}
	
	if(!deps_invalid) {
//		printf("%s %ld, %s %ld OK\n", filename, src_mtime, deps_path, deps_mtime);
		return 0; // everything is up-to-date
	}
	
//	printf("calulating deps for %s\n", filename);
	unlink(real_deps_path);
	FILE* fdeps = fopen(real_deps_path, "w+b");
	
	size_t flen = 0;
	char* fsrc = read_whole_file(real_src_path, &flen);
	
	if(!fsrc) {
		printf("File not found: %s\n", real_src_path);
		return 1;
	}
	
	char** lines = strsplit_exact("\n", fsrc);
	
	char* dot = strrchr(filename, '.');
	char* file_base = strndup(filename, dot - filename);
	
	union {
		glsl_source arr[SH_MAX_VALUE];
		struct {		
			#define X(a, ...) glsl_source a;
				SHADER_TYPE_LIST
			#undef X
		};
	} shaders = {0};
	
	struct {
		int major, minor;
		char* label;
	} versions[SH_MAX_VALUE];
	
	for(int i = 0; i < SH_MAX_VALUE; i++) {
		strlist_init(&shaders.arr[i].lines);
		
		shaders.arr[i].fdeps = fdeps;
		
		versions[i].major = -1;
		versions[i].minor = -1;
		versions[i].label = NULL;
	}
	
	strlist files;
	strlist_init(&files);
	
	strlist_push(&files, filename);
	
//	push_meta(&sources.global.lines, 0, )
//	strlist_push(&sources.global.lines, "#extension GL_GOOGLE_include_directive: enable");
	
	int file_num = 0;
	
	int shader_index = SH_global;
	int line_num = 1;
	for(char** s = lines; *s; s++, line_num++) {
//		push_meta(&shaders.arr[shader_index], line_num, filename); 
		if(strspn(*s, " \t") == strlen(*s)) continue;
		
		if(!strncmp(*s, "#version", strlen("#version"))) { // TODO: leading whitespace tolerance
			char* v = strchr(*s, ' ');
			v += strspn(v, " ");
			
			char* e = NULL;
			int n = strtol(v, &e, 10);
			versions[shader_index].major = n / 100;
			versions[shader_index].minor = n % 100;
			
			char* le = strchr(e, '\n');
			e += strspn(e, " ");
			if(le == e) continue;
			char* e2 = strpbrk(e, " \r\n\t");
			if(e2 == e) continue;
			
			versions[shader_index].label = strndup(e, e2 - e);
			
			continue;
		}
		else if(0 == strncmp(*s, "#include", strlen("#include"))) {
			char* fn = strchr(*s, '"') + 1;
			char* e = strchr(fn, '"');
			char* include_fn = strndup(fn, e - fn);
			
			strlist_push(&shaders.arr[shader_index].lines, sprintfdup("// %s", *s)); 
			
			char* include_file = path_join(rel_path, include_fn);
			ret |= glsl_append_file(&shaders.arr[shader_index], include_file);
			
			continue;
		}
		else if(!strncmp(*s, "#shader ", strlen("#shader "))) { // TODO: leading whitespace tolerance
			char* type = strchr(*s, ' ');
			type += strspn(type, " ");
			
			if(!strncasecmp(type, "VERTEX", strlen("VERTEX"))) {
				// prepend globals
				glsl_source_concat(&shaders.vert, &shaders.global);
				shader_index = SH_vert;
				continue;
			}
			else if(!strncasecmp(type, "TESS_CONTROL", strlen("TESS_CONTROL"))) {
				// prepend globals
				glsl_source_concat(&shaders.tesc, &shaders.global);
				shader_index = SH_tesc;
				continue;
			}
			else if(!strncasecmp(type, "TESS_EVALUATION", strlen("TESS_EVALUATION"))) {
				// prepend globals
				glsl_source_concat(&shaders.tese, &shaders.global);
				shader_index = SH_tese;
				continue;
			}
			else if(!strncasecmp(type, "GEOMETRY", strlen("GEOMETRY"))) {
				// prepend globals
				glsl_source_concat(&shaders.geom, &shaders.global);
				shader_index = SH_geom;
				continue;
			}
			else if(!strncasecmp(type, "FRAGMENT", strlen("FRAGMENT"))) {
				// prepend globals
				glsl_source_concat(&shaders.frag, &shaders.global);
				shader_index = SH_frag;
				continue;
			}
			else if(!strncasecmp(type, "COMPUTE", strlen("COMPUTE"))) {
				// prepend globals
				glsl_source_concat(&shaders.comp, &shaders.global);
				shader_index = SH_comp;
				continue;
			}
			else if(!strncasecmp(type, "MESH", strlen("MESH"))) {
				// prepend globals
				glsl_source_concat(&shaders.mesh, &shaders.global);
				shader_index = SH_mesh;
				continue;
			}
			else if(!strncasecmp(type, "TASK", strlen("TASK"))) {
				// prepend version
				
				// prepend globals
				glsl_source_concat(&shaders.task, &shaders.global);
				shader_index = SH_task;
				continue;
			}
			else {
				fprintf(stderr, "Unknown shader type '%.10s' at %s:%d\n", type, path, line_num);
			}
			
		}
	
	
		strlist_push(&shaders.arr[shader_index].lines, sprintfdup("#line %d %d", shaders.arr[shader_index].meta_len, line_num)); 
		strlist_push(&shaders.arr[shader_index].lines, *s);
		push_meta(&shaders.arr[shader_index], line_num, filename); 
	}
	
	
	char* extensions[] = {
		#define X(a, ...) #a,
			SHADER_TYPE_LIST
		#undef X
	};
	
	
	
	strlist cmds;
	strlist_init(&cmds);
	
	typedef struct {
		char* temp_file;
		char* human_file;
		char* binary_file;
		int stage;
	} cjob;
	
	cjob jobs[SH_MAX_VALUE];
	
	int j = 0;
	for(int i = 1; i < SH_MAX_VALUE; i++) {
		if(shaders.arr[i].lines.len == 0) continue;
		
		jobs[j]. stage = i;
		
		jobs[j].temp_file = sprintfdup("%s/%s.%s.glsl", temp_path, file_base, extensions[i]);
		jobs[j].human_file = sprintfdup("%s/%s.%s.spv.asm", temp_path, file_base, extensions[i]);
		jobs[j].binary_file = sprintfdup("%s/%s.%s.spv", temp_path, file_base, extensions[i]);
		
		
		
		unlink(jobs[j].temp_file);
		FILE* f = fopen(jobs[j].temp_file, "wb");
		if(!f) printf("file error %s\n", strerror(errno));
		
		char* src = strlist_join(&shaders.arr[i].lines, "\n");
		
		if(versions[i].major == -1) versions[i] = versions[SH_global];
		fprintf(f, "#version %.1d%.2d %s\n", versions[i].major, versions[i].minor, versions[i].label);
		
		fwrite(src, 1, strlen(src), f);
		fclose(f);
		
		unlink(jobs[j].binary_file);
		
		char* cmd = sprintfdup("glslangValidator -g -C -H -V -o %s %s vulkan_glsl.conf", jobs[j].binary_file, jobs[j].temp_file);
		strlist_push(&cmds, cmd);
		
		printf("adding cmd '%s'\n", cmd);
		
		j++;
	}
	
	
	
	struct child_process_info** cpis;
	
	ret |= execute_mt(&cmds, g_nprocs, NULL, &cpis);
	
	
	for(int i = 0; cpis[i]; i++) {
		struct child_process_info* cpi = cpis[i];
		cjob* job = &jobs[i];
		
		if(cpi->exit_status == 0) {
			// write human-readable assembly file
			unlink(job->human_file);
			FILE* h = fopen(job->human_file, "wb");
			fwrite(cpi->output_buffer, 1, cpi->buf_len, h);
			fclose(h);
		}
		else {
			char** lines = strsplit("\n", cpi->output_buffer);
			for(char** line = lines; *line; line++) {
				unsigned int lnum, fnum, chars_read;
				if(2 == sscanf(*line, "ERROR: %u:%u:%n", &lnum, &fnum, &chars_read)) {
//					printf("%s\n", *line);
					printf("\e[1;37m%s:%d: \e[1;31merror:\e[0m%s\n", shaders.arr[job->stage].meta[fnum].file, lnum, *line + chars_read);
					
				}
				else {
					printf("%s\n", *line);
				}
			}
			
			printf("\n");
		}
	
		unlink(job->temp_file);
	} 
	
	
	
	fclose(fdeps);
	
	// TODO: link shaders into program
	
	// TODO: free all this garbage
	
	// the deps file timestamp is used for minimal rebuilds
	if(ret) unlink(real_deps_path);

	return ret;
}


typedef struct glsl_scan_info {
	char* build_base_path;
	FILE* of, *manifest;
	int ret;
} glsl_scan_info;



int glsl_scan_callback(char* dir, char* fullPath, char* fileName, unsigned char file_type, void* _data) {
	struct glsl_scan_info* info = (struct glsl_scan_info*)_data;
	
//	printf("\n'%s' - '%s'\n", fullPath, fileName);
	
	info->ret |= glsl_to_spirv("./", dir, fileName, info->build_base_path);
	return 0;
}

int scan_glsl_folder(char* rel_path, char* build_base_path) {

	struct glsl_scan_info info;
	info.build_base_path = build_base_path;
	info.ret = 0;

	recurse_dirs(rel_path, glsl_scan_callback, &info, 0, 0);
	
	return info.ret;
}







/* ----- -END- spirv.c ----- */


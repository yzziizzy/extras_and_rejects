// gcc -lutil build.c -o hacer && ./hacer

// link with -lutil

//char* build_dir;
//char* source_dir = "src"; // "src"
//char* exe_path = "./meshtool";
//char* base_build_dir = "build";


#include "_build.inc.c"



char* sources[] = {
	"__SED_TOKEN_SOURCE_FILE_NAME",
	NULL,
};

// these are run through pkg-config
char* lib_headers_needed[] = {
//	"freetype2", "fontconfig",
//	"gl", "glu", "glew",
//	"vulkan",
//	"libpcre2-8", 
//	"vorbisfile", "opusfile",
//	"libpng", "libjpeg",
//	"x11", "xfixes",
//	"alsa",
//	"zlib",
//	"openssl",
	NULL
};

// these are run through pkg-config
char* libs_needed[] = {
//	"freetype2", "fontconfig",
//	"gl", "glu", "glew",
//	"vulkan",
//	"libpcre2-8", 
//	"vorbisfile", "opusfile",
//	"libpng", "libjpeg",
//	"x11", "xfixes",
//	"alsa",
//	"zlib",
//	"openssl",
	NULL,
};

char* ld_add[] = {
	"-lm",
//	"-ldl", "-lutil",
	NULL,
};


char* debug_cflags[] = {
	"-ggdb",
	"-DDEBUG",
	"-O0",
	NULL
};

char* profiling_cflags[] = {
	"-pg",
	NULL
};

char* release_cflags[] = {
	"-DRELEASE",
	"-O3",
	"-flto", // really slow
//	"-Wno-array-bounds", // temporary, until some shit in sti gets fixed. only happens with -O3
	NULL
};

// -ffast-math but without reciprocal approximations 
char* common_cflags[] = {
	"-std=gnu11", 
	"-ffunction-sections", "-fdata-sections",
	"-DLINUX",
	"-march=native",
	"-mtune=native", 
	"-fno-plt",
	"-fPIC",
	"-fno-math-errno", 
	"-fexcess-precision=fast", 
	"-fno-signed-zeros",
	"-fno-trapping-math", 
	"-fassociative-math", 
//	"-ffinite-math-only", 
	"-fno-rounding-math", 
	"-fno-signaling-nans", 
//	"-include signal.h",
//	"-include src/global.h", 
	"-pthread", 
	"-Wall", 
	"-Werror", 
	"-Wextra", 
	"-Wno-unused-result", 
	"-Wno-unused-variable", 
	"-Wno-unused-but-set-variable", 
	"-Wno-unused-function", 
	"-Wno-unused-label", 
	"-Wno-unused-parameter",
	"-Wno-pointer-sign", 
	"-Wno-missing-braces", 
	"-Wno-maybe-uninitialized", 
	"-Wno-implicit-fallthrough", 
	"-Wno-sign-compare", 
	"-Wno-char-subscripts", 
	"-Wno-int-conversion", 
	"-Wno-int-to-pointer-cast", 
	"-Wno-enum-conversion",
	"-Wno-unknown-pragmas",
	"-Wno-sequence-point",
	"-Wno-switch",
	"-Wno-parentheses",
	"-Wno-comment",
	"-Wno-strict-aliasing",
	"-Wno-endif-labels",
	"-Wno-address-of-packed-member",
	"-Wno-multichar",
	"-Wno-type-limits", // 'comparison of unsigned expression in "< 0" is always false' bullshit from a macro
	"-Wno-address-of-packed-member", // not usefull on x64
	"-Werror=implicit-function-declaration",
	"-Werror=uninitialized",
	"-Werror=return-type",
	"-Werror=incompatible-pointer-types",
	NULL,
};



void global_init() {
	string_cache_init(2048);
	realname_cache_init();
	//strlist_init(&compile_cache);
	hash_init(&mkdir_cache, 128);
	g_nprocs = get_nprocs();
}



int main(int argc, char* argv[]) {
	char* cmd;

	global_init();
	
	// defaults
	
	objfile* obj = calloc(1, sizeof(*obj));
	
	obj->mode_debug = 2;
	
	obj->exe_path = "__SED_TOKEN_EXE_REL_PATH";
	obj->source_dir = "__SED_TOKEN_SOURCE_REL_PATH";
	obj->base_build_dir = "__SED_TOKEN_BUILD_REL_PATH";
	
	obj->sources = sources;
	
	obj->debug_cflags = debug_cflags;
	obj->release_cflags = release_cflags;
	obj->profiling_cflags = profiling_cflags;
	obj->common_cflags = common_cflags;
	
	obj->libs_needed = libs_needed;
	obj->lib_headers_needed = lib_headers_needed;
	obj->ld_add = ld_add;


	parse_cli_opts(argc, argv, obj);
	
	start_obj(obj);
	
	//---------------------------
	//
	// [ custom init code here]
	//
	//---------------------------

#if COMPILE_SPIRV
	if(scan_glsl_folder("src/shaders", "build/core/d")) {
		printf("\e[1;31mGLSL build failed.\e[0m\n");
		return 1;
	}
	
	if(scan_glsl_folder("src/shaders/compute", "build/core/d")) {
		printf("\e[1;31mGLSL build failed.\e[0m\n");
		return 1;
	}
#endif
	
	//printf("%s\n\n\n\n",g_gcc_opts_flat);
//	rglob src;
	//recursive_glob("src", "*.[ch]", 0, &src);
	
	
	objfile* tasks[] = {obj};
	int taskCount = sizeof(tasks) / sizeof(tasks[0]);
	
	
	float source_count = 0;
	
	for(int o = 0; o < taskCount; o++) {
		source_count += list_len(tasks[o]->sources);
	}
	
	for(int o = 0; o < taskCount; o++) {
		for(int i = 0; tasks[o]->sources[i]; i++) {
	//		printf("%i: checking %s\n", i, sources[i]);
			char* t = path_join(tasks[o]->source_dir, tasks[o]->sources[i]);
			check_source(t, tasks[o]);
			free(t);
			
			printf("\rChecking dependencies...  %s", printpct((i * 100) / source_count));
		}
	
		printf("\rChecking dependencies...  \e[32mDONE\e[0m\n");
		fflush(stdout);
		
		if(compile_cache_execute(tasks[o])) {
			printf("\e[1;31mBuild failed.\e[0m\n");
			return 1;
		}

	}
	
	
	
	
	char* objects_flat = join_str_list(obj->objs.entries, " ");
	
	
	cmd = sprintfdup("ar rcs %s/tmp.a %s", obj->build_dir, objects_flat);
	if(obj->verbose) puts(cmd);
	
	printf("Creating archive...      "); fflush(stdout);
	if(system(cmd)) {
		printf(" \e[1;31mFAIL\e[0m\n");
		return 1;
	}
	else {
		printf(" \e[32mDONE\e[0m\n");
	}
	
	
	cmd = sprintfdup("gcc -Wl,--gc-sections %s %s/tmp.a -o %s %s %s", 
		obj->mode_profiling ? "-pg" : "", obj->build_dir, obj->exe_path, obj->gcc_libs, obj->gcc_opts_flat
	);
	if(obj->verbose) puts(cmd);
	
	printf("Linking executable...    "); fflush(stdout);
	if(system(cmd)) {
		printf(" \e[1;31mFAIL\e[0m\n");
		return 1;
	}
	else {
		printf(" \e[32mDONE\e[0m\n");
	}
	
	if(!obj->verbose) {
		// erase the build output if it succeeded
		
		// Compiling, dep checking
		for(int i = 0; i < taskCount; i++) {
			printf("\e[F\e[K");
			printf("\e[F\e[K");
		}
		
		// "Linking..."
		printf("\e[F\e[K");
		printf("\e[F\e[K");
		printf("\e[F\e[K");
	}
	
	printf("\e[32mBuild successful:\e[0m %s\n\n", obj->exe_path);
	
	return 0;
}



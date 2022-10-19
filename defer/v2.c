#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define init_defer() \
    __label__ DO_DEFER; \
    char _run_defer = 0; \
    void** _next_defer = NULL; \
    DO_DEFER: \
    if(_run_defer) { \
        void* jump = _next_defer[1]; \
        _next_defer = (void**)_next_defer[0]; \
        goto *jump; \
    } \
    void run_defer(char* d) { \
        _run_defer++; \
        if(_run_defer == 1) goto DO_DEFER; \
    }; \
    __attribute__((cleanup(run_defer))) char _start_jump; \


    

#define defer(...) \
do { \
    __label__ _def; \
    _def: \
    if(_run_defer) { \
        __VA_ARGS__; \
        if(_next_defer) { \
            void* jump = _next_defer[1]; \
            _next_defer = (void**)_next_defer[0]; \
            goto *jump;\
        } \
        else goto DONE; \
    } \
    else { \
        void** link = alloca(16); \
        link[0] = (void*)_next_defer; \
        link[1] = &&_def; \
        _next_defer = link; \
    } \
} while(0)





int main(int argc, char **argv) {
    init_defer()

    defer(printf("crowbar"));
    
    printf("some junk");

    defer(printf("foobar"));



    return 0;

    DONE:
}

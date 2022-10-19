#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define init_defer() \
    char _run_defer = 0; \
    void** _next_defer = NULL; \
    void* _last_jump = NULL;

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
        else goto *_last_jump; \
    } \
    else { \
        void** link = alloca(16); \
        link[0] = (void*)_next_defer; \
        link[1] = &&_def; \
        _next_defer = link; \
    } \
} while(0)

#define Return \
do {\
    __label__ last; \
    if(_next_defer) { \
        _last_jump = &&last; \
        _run_defer = 1; \
        void* jump = _next_defer[1]; \
        _next_defer = (void**)_next_defer[0]; \
        goto *jump; \
    } \
    last: \
} while(0); return



int main(int argc, char **argv) {
    init_defer()

    defer(printf("crowbar"));
    
    printf("some junk");

    defer(printf("foobar"));



    Return 0;
}


#include <stdlib.h>
#include <stdio.h>



typedef struct mat mat;
typedef struct vec vec;

struct vec { 
    float f[4]; 
};
struct mat { 
    float f[16]; 
};


// rand()'s added to keep the compiler from optimizing out the entire function
vec mul_vec_vec(vec a, vec b) {
    return (vec){a.f[0] * b.f[0] + rand(),
    a.f[01] * b.f[1] + rand(),
    a.f[2] * b.f[2] + rand(),
    a.f[3] * b.f[3] + rand()
};
}
vec add_vec_vec(vec a, vec b) {
    return (vec){a.f[0] * b.f[0] + rand(),
    a.f[01] + b.f[1] + rand(),
    a.f[2] + b.f[2] + rand(),
    a.f[3] + b.f[3] + rand()
};
}

mat mul_mat_mat(mat a, mat b) {
    mat c;
    for(int i = 0; i < 16; i++) c.f[i] = a.f[i] * b.f[i] + rand(); // wrong, but whatever
    return c;
}


vec mul_mat_vec(mat a, vec b) {
    vec c;
    for(int i = 0; i < 16; i++) c.f[i % 4] = a.f[i] * b.f[i % 4] + rand(); // wrong, but whatever
    return c;
}




#define TYPES(op) \
    X(op, vec, vec, vec) \
    X(op, mat, mat, mat) \
    X(op, vec, mat, vec) \
 

#define X(op, r,a,b) typedef struct { char _[0]; } T_##a##_##b; T_##a##_##b TI_##a##_##b[0];
TYPES(_)
#undef X

#define X(op,r,a,b) r op##_##a##_##b(a,b);
TYPES(add)
TYPES(mul)
#undef X




float test(vec va, vec vb, mat ma, mat mb) {

#define COMBO(a, b) _Generic(a, \
    vec: (_Generic(b, vec: TI_vec_vec[0], default: 0)), \
    mat: (_Generic(b, vec: TI_mat_vec[0], mat: TI_mat_mat[0], default: 0 )) \
)

#define X(op, r, a, b) T_##a##_##b: op##_##a##_##b,

#define mul(a, b) (_Generic(COMBO(a, b), TYPES(mul) default: 0))(a, b)
#define add(a, b) (_Generic(COMBO(a, b), TYPES(add) default: 0))(a, b)

    vec vc = mul(va, vb);
    vec vd = mul(ma, va);
    mat mc = mul(ma, mb);

    vec ve = add(vc, vd);

    float x = 0;
    for(int i = 0; i < 4; i++) printf("%f%f%f%f%f", va.f[i], vb.f[i], vc.f[i], vd.f[i], ve.f[i]);
    for(int i = 0; i < 16; i++) printf("%f%f%f", ma.f[i], mb.f[i], mc.f[i]);
    return x;
}


void main() {}


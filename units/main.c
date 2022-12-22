

#include <stdlib.h>
#include <stdio.h>


#include "sti/sti.h"


#define BASE_UNIT_TYPE_LIST \
	X(time) \
	X(mass) \
	X(distance) \
	X(heat) \
	X(current) \
	X(substance) \
	X(luminosity) \

#define SYSTEM_LIST \
	X(universal, Universal) \
	X(soy, SI) \
	X(based, Imperial) \

#define UNIVERSAL_UNITS \
	X(second, time, s, 1s) \
	X(ampere, current, A, 1A) \
	X(mole, substance, mol, 1mol) \

#define SOY_UNITS \
	X(gram, mass, g, 1g) \
	X(meter, distance, m, 1m) \
	X(kelvin, heat, K, 1K) \
	X(candela, luminosity, cd, 1cd) \
	
#define BASED_UNITS \
	X(pound, mass, lb) \
	X(inch, distance, in) \
	X(rankine, heat, R) \
	X(candepower, luminosity, cp) \



#define DERIVED_UNITS \
	X(watt, universal, W, 1000 g * m^2 * s^-3) \
	X(volt, universal, V, 1000 g * m^2 * s^-3 * A^-1) \
	X(newton, soy, N, 1000 g * m / s^2) \
	X(coulomb, universal, C, 1 A * s) \
	X(joule, soy, J, 1000 g * m^2 * s^-2) \


enum {
#define X(z, ...) z,
	BASE_UNIT_TYPE_LIST
#undef X
};

enum {
#define X(z, ...) z,
	SYSTEM_LIST
#undef X
};

enum {
#define X(z, ...) z,
	UNIVERSAL_UNITS
	SOY_UNITS
	BASED_UNITS
	DERIVED_UNITS
#undef X
};

char* base_unit_type_names[] = {
#define X(x, ...) [x] = #x,
	BASE_UNIT_TYPE_LIST
#undef X
};


int unit_base_types[] = {
#define X(u, bt, ...) [u] = bt,
	UNIVERSAL_UNITS
	SOY_UNITS
	BASED_UNITS
#undef X
};


typedef struct unit {
	char* name;
	
	double amt;
	unsigned char base_unit[7];
	char power[7];
} unit_t;






void unit_multiply(unit_t* a, unit_t* b, unit_t* out) {

	out->amt = a->amt * b->amt;
	
	for(int i = 0; i < 7; i++) {
		out->power[i] = a->power[i] + b->power[i];
	}
	
	// todo: look up name, if it exists
}


struct {
	char* name;
	int system;
	char* abbr;
	char* def;
} derived_unit_list[] = {
#define X(name, system, abbr, def) [name] = {#name, system, #abbr, #def}, 
	DERIVED_UNITS
#undef X
};



void unit_parse(char* s, unit_t* out) {
	char* e = NULL;
	
	out->amt = strtod(s, &e);
	
	int last_unit = -1;
	int last_power = 1;
	int last_power_sign = 1;
	int invert = 1;
	
	for(; *e; e++) {
		switch(*e) {
			case 's': last_unit = second; break;
			case 'g': last_unit = gram; break;
			case 'm':
				if(e[1] == 'o' && e[2] == 'l') last_unit = mole;
				else last_unit = meter; 
				break;
			case 'K': last_unit = kelvin; break;
			case 'A': last_unit = ampere; break;
			case 'c':
				if(e[1] == 'd') last_unit = candela;
				break;
				
			case '^':
				last_power = 0;
				break;
			
			case '-':
				last_power_sign = -1;
				break;
				
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				last_power = last_power * 10 + (*e - '0');
				break;
				
			case '*':
				if(last_unit >= 0) {
					out->power[unit_base_types[last_unit]] += last_power * last_power_sign * invert;
					
					last_unit = -1;
					last_power = 1;
					last_power_sign = 1;
				}
				break;
			
			case '/':
				
				if(last_unit >= 0) {
					out->power[unit_base_types[last_unit]] += last_power * last_power_sign * invert;
					
					last_unit = -1;
					last_power = 1;
					last_power_sign = 1;
				}
				invert *= -1;
				break;
			
		}
	}
	
	if(last_unit >= 0) {
		out->power[unit_base_types[last_unit]] += last_power * last_power_sign * invert;
	}

}



int main(int argc, char* argv[]) {

	unit_t n = {};

	unit_parse(derived_unit_list[volt].def, &n);
	
	printf("%f\n", n.amt);
	for(int x = 0; x < 7; x++) {
		printf("%s ^%d\n", base_unit_type_names[x], (int)n.power[x]);
	}


	return 0;
}






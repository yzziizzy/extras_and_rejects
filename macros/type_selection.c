#define HT_BUILTIN_TYPES(q, z) \
	_Bool q: z, \
	char q: z, \
	signed char q: z, \
	short q: z, \
	int q: z, \
	long q: z, \
	long long q: z, \
	unsigned char q: z, \
	unsigned short q: z, \
	unsigned int q: z, \
	unsigned long q: z, \
	unsigned long long q: z, \
	float q: z, \
	double q: z, \
	void* q: z,

#define HT_IS_NATIVE_TYPE(x, y, n) _Generic(x, \
    HT_BUILTIN_TYPES(,y) \
    HT_BUILTIN_TYPES(*,y) \
    HT_BUILTIN_TYPES(**,y) \
    HT_BUILTIN_TYPES(***,y) \
    HT_BUILTIN_TYPES(****,y) \
	default: n \
)

#ifndef TYPES_H
#define TYPES_H


#if defined(_MSC_VER)
#define STELLA_CC_MSVC
#elif defined(__GNUC__)
#define STELLA_CC_GCC
#elif defined(__clang__)
#define STELLA_CC_CLANG
#elif defined(__EMSCRIPTEN__)
#error "Emscripten not supported!"
#elif defined(__MINGW32__) || defined(__MINGW64__)
#error "MinGW not yet supported!"
#else
#error "Unknown compiler!"
#endif

#if defined(__linux__)
#define STELLA_OS_LINUX
#elif defined(_WIN32) || defined(_WIN64)
#define STELLA_OS_WINDOWS
#else
#error "Unknown operating system!"
#endif


#if defined(STELLA_CC_MSVC)
#define FORCE_INLINE inline __forceinline
#elif defined(STELLA_CC_GCC) || defined(STELLA_CC_CLANG)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#error "FORCE_INLINE unimplemented for this compiler!"
#endif


typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef float f32;
typedef double f64;


// NOTE: Yes, I do enjoy being this rude. Fite me.
// (Currently I only target 64-bit architectures. x64 specifically :P)
static_assert(sizeof(u8)  == 1);
static_assert(sizeof(s8)  == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(s16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(s32) == 4);
static_assert(sizeof(u64) == 8);
static_assert(sizeof(s64) == 8);
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);


#ifndef NULL
#define NULL 0
#endif


#define array_length(a) ((sizeof(a))/(sizeof(a[0])))


#ifdef offsetof
#undef offsetof
#endif
#define offsetof(type, member) ((u64)&(((type *)0)->member))


// TODO: Make this less terrible.
inline void __assert(bool x) {
	if(!x) {
		*(volatile int*)0 = 0;
	}
}

#ifdef assert
#undef assert
#endif
#define assert(x) __assert(!!(x))


#define STDIN 				0
#define STDOUT				1
#define STDERR				2


#define S32_MIN 	0x80000000
#define S32_MAX 	0x7FFFFFFF

#define U32_MIN 	0x00000000
#define U32_MAX 	0xFFFFFFFF

#define S64_MIN 	0x8000000000000000L
#define S64_MAX 	0x7FFFFFFFFFFFFFFFL

#define U64_MIN 	0x0000000000000000L
#define U64_MAX 	0xFFFFFFFFFFFFFFFFL

#define F32_MIN 	1.17549435082228750796873653722224568e-38F
#define F32_MAX 	3.40282346638528859811704183484516925e+38F

#define F64_MIN		((f64)2.22507385850720138309023271733240406e-308L)
#define F64_MAX 	((f64)1.79769313486231570814527423731704357e+308L)

#define INT_MIN 	S32_MIN
#define INT_MAX 	S32_MAX

#define UINT_MIN 	U32_MIN
#define UINT_MAX 	U32_MAX

#define LONG_MIN 	S64_MIN
#define LONG_MAX 	S64_MAX

#define ULONG_MIN 	U64_MIN
#define ULONG_MAX 	U64_MAX

#define FLT_MIN		F32_MIN
#define FLT_MAX 	F32_MAX

#define DBL_MIN 	F64_MIN
#define DBL_MAX 	F64_MAX


struct Buffer {
	u8 *data;
	u64 len;
};


#endif
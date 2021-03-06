#define PT_MATH_RANGE_CHECKS
#define PT_MATH_PRECISE_POW
#include "pt_math.h"

#define sqrtf32(x) 		PT_sqrtf(x)
#define floorf32(x) 	PT_floorf(x)
#define ceilf32(x) 		PT_ceilf(x)
#define roundf32(x)		PT_roundf(x)
#define absf32(x) 		PT_fabsf(x)
#define sinf32(x) 		PT_sinf(x)
#define cosf32(x)		PT_cosf(x)
#define tanf32(x)		PT_tanf(x)
#define asinf32(x)		PT_asinf(x)
#define acosf32(x)		PT_acosf(x)
#define atanf32(x)		PT_atanf(x)
#define powf32(b, e)	PT_powf(b, e)
#define ldexpf32(a, e)	(a*powf32(2.0f, e))
#define atan2f32(y, x)	PT_atan2f(y, x)
#define fmodf32(a, b)	PT_fmodf(a, b)
#define log2f32(x)		PT_log2f(x)

#define sqrtf64(x) 		PT_sqrt(x)
#define floorf64(x) 	PT_floor(x)
#define ceilf64(x) 		PT_ceil(x)
#define roundf64(x)		PT_round(x)
#define absf64(x) 		PT_fabs(x)
#define sinf64(x) 		PT_sin(x)
#define cosf64(x)		PT_cos(x)
#define tanf64(x)		PT_tan(x)
#define asinf64(x)		PT_asin(x)
#define acosf64(x)		PT_acos(x)
#define atanf64(x)		PT_atan(x)
#define powf64(b, e)	PT_pow(b, e)
#define ldexpf64(a, e)	(a*powf64(2.0f, e))
#define atan2f64(y, x)	PT_atan2(y, x)
#define fmodf64(a, b)	PT_fmod(a, b)
#define log2f64(x)		PT_log2(x)

#define abs32(x)		PT_abs(x)
#define abs64(x)		PT_llabs(x)

// TOOD: change these to forcibly-inlined functions?
#define rotl32(x, r) ((x << r) | (x >> (32 - r)));
#define rotl64(x, r) ((x << r) | (x >> (64 - r)));
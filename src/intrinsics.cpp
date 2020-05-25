#include <immintrin.h>

// TODO: Remove dependency on libm
 #include <math.h>

f32 sqrtf32(f32 x) { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x))); }
f32 floorf32(f32 x) { return _mm_cvtss_f32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(x))); }
f32 ceilf32(f32 x) { return _mm_cvtss_f32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(x))); }
f32 absf32(f32 x) {	return x < 0 ? x : -x; }
f32 sinf32(f32 x) {	return sinf(x); }
f32 cosf32(f32 x) {	return cosf(x); }
f32 tanf32(f32 x) { return tanf(x); }
f32 asinf32(f32 x) { return asinf(x); }
f32 acosf32(f32 x) { return acosf(x); }
f32 atanf32(f32 x) { return atanf(x); }
f32 powf32(f32 b, f32 e) { return powf(b, e); }
f32 ldexpf32(f32 a, s32 b) { return ldexpf(a, b); }
f32 atan2f32(f32 y, f32 x) { return atan2f(y, x); }
f32 fmodf32(f32 a, f32 b) { return fmodf(a, b); }
f32 roundf32(f32 x) { return roundf(x); }

f64 sqrtf64(f64 x) { return _mm_cvtsd_f64(_mm_sqrt_pd(_mm_set_sd(x))); }
f64 floorf64(f64 x) { return _mm_cvtsd_f64(_mm_floor_sd(_mm_setzero_pd(), _mm_set_sd(x))); }
f64 ceilf64(f64 x) { return _mm_cvtsd_f64(_mm_ceil_sd(_mm_setzero_pd(), _mm_set_sd(x))); }
f64 absf64(f64 x) {	return x < 0 ? x : -x; }
f64 sinf64(f64 x) {	return sin(x); }
f64 cosf64(f64 x) {	return cos(x); }
f64 tanf64(f64 x) { return tan(x); }
f64 asinf64(f64 x) { return asin(x); }
f64 acosf64(f64 x) { return acos(x); }
f64 atanf64(f64 x) { return atan(x); }
f64 powf64(f64 b, f64 e) { return pow(b, e); }
f64 ldexpf64(f64 a, s32 b) { return ldexp(a, b); }
f64 atan2f64(f64 y, f64 x) { return atan2(y, x); }
f64 fmodf64(f64 a, f64 b) { return fmod(a, b); }
f64 roundf64(f64 x) { return round(x); }

s32 abs32(s32 x) { return x < 0 ? -x : x; }
s64 abs64(s64 x) { return x < 0 ? -x : x; }
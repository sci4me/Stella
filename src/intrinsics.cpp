#include <math.h>
#include <emmintrin.h>

f32 sqrtf32(f32 x) { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x))); }
f32 absf32(f32 x) {	return x < 0 ? x : -x; }
f32 sinf32(f32 x) {	return sinf(x); }
f32 cosf32(f32 x) {	return cosf(x); }
f32 powf32(f32 b, f32 e) { return powf(b, e); }

f64 sqrtf64(f64 x) { return _mm_cvtsd_f64(_mm_sqrt_pd(_mm_set_sd(x))); }
f64 absf64(f64 x) {	return x < 0 ? x : -x; }
f64 sinf64(f64 x) {	return sin(x); }
f64 cosf64(f64 x) {	return cos(x); }
f64 powf64(f64 b, f64 e) { return pow(b, e); }

s32 abs32(s32 x) { return x < 0 ? -x : x; }
s64 abs64(s64 x) { return x < 0 ? -x : x; }
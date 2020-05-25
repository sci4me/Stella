#include "intrinsics.cpp"

#define APP_NAME "Stella"

#define GL_DEBUG
#define GL_MAJOR 4
#define GL_MINOR 4


#define GLEW_STATIC
#define GLEW_NO_GLU
#include "GL/glew.h"


// #define IMGUI_DISABLE


#define IMGUI_NO_LIBC
// NOTE TODO: implement assert
#define IM_ASSERT(x)
#define IM_MEMSET(d, x, n)          mlc_memset(d, x, n)
#define IM_MEMMOVE(d, s, n)         mlc_memmove(d, s, n)
#define IM_MEMCPY(d, s, n)          mlc_memcpy(d, s, n)
#define IM_MEMCMP(a, b, n)			mlc_memcmp(a, b, n)
#define IM_STRLEN(s)                mlc_strlen(s)
#define IM_STRCHR(s, c)             mlc_strchr(s, c)
#define IM_STRCPY(d, s)             mlc_strcpy(d, s)
#define IM_STRCMP(a, b)             mlc_strcmp(a, b)
#define ImFabs(x)					absf32(x)
#define ImSqrt(x)					sqrtf32(x)
#define ImFmod(a, b)				mlc_fmodf(a, b)
#define ImCos(x)					cosf64(x)
#define ImSin(x)					sinf64(x)
#define ImAcos(x)					mlc_acos(x)
#define ImAtan2(y, x)				mlc_atan2(y, x)
#define ImFloorStd(x)				mlc_floorf(x)
#define ImCeil(x)					mlc_ceilf(x)
#define ImAtof(s)					mlc_atof(s)
static inline float ImPow(float b, float e) { return powf32(b, e); }
static inline double ImPow(double b, double e) { return powf64(b, e); }
#define ImQsort(b, n, s, c)			mlc_qsort(b, n, s, c)
#define IM_MALLOC_FN(n)				mlc_malloc(n)
#define IM_FREE_FN(p)				mlc_free(p)
#define IM_TOUPPER(c)				mlc_toupper(c)
#define IM_STRNCPY(d, s, n)			mlc_strncpy(d, s, n)
#define IM_MEMCHR(p, v, n)			mlc_memchr(p, v, n)
#define IM_STRSTR(a, b)				mlc_strstr(a, b)
#define IM_SSCANF(s, f, ...)		mlc_sscanf(s, f, __VA_ARGS__)
#define IM_VSNPRINTF(b, s, f, a)	mlc_vsnprintf(b, s, f, a)
#include "imgui/imgui.h"
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"


#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_GIF
#define STBI_NO_LIBC
// NOTE TODO: implement assert
#define STBI_ASSERT(x)
#define STBI_MALLOC(n)           	mlc_malloc(n)
#define STBI_REALLOC(p, n)     		mlc_realloc(p, n)
#define STBI_FREE(p)              	mlc_free(p)
#define STBI_LDEXPF(a, b) 		 	mlc_ldexpf(a, b)
#define STBI_POWF(a, b) 			powf32(a, b)
#define STBI_MEMSET(p, x, n)		mlc_memset(p, x, n)
#define STBI_MEMCPY(d, s, n)		mlc_memcpy(d, s, n)
#define STBI_ABS(x)					abs32(x)
#define STBI_STRCMP(a, b)			mlc_strcmp(a, b)
#define STBI_STRNCMP(a, b, n)		mlc_strncmp(a, b, n)
#define STBI_STRTOL(s, e, b)		mlc_strtol(s, e, b)
#include "stb_image.h" 


#define RND_U32 u32
#define RND_U64 u64
#define RND_IMPLEMENTATION
#include "rnd.h"


struct Game {
	s32 window_width = 1280;
	s32 window_height = 720;

	// NOTE: Set to true so that we don't have to 
	// do any sizing code before the main loop.
	//          - sci4me, 5/7/20
	bool window_resized = true;

	f32 scale = 1.0f;

	bool show_profiler = false;

	bool show_debug_window = false;
	bool show_imgui_metrics_window = false;

	bool fullscreen_changed = false;
	bool fullscreen = false;
	s32 saved_window_x, saved_window_y, saved_window_w, saved_window_h;

	bool vsync = true;
	bool fast_mining = false;

	// TODO: Eventually we'll want to properly separate
	// the "debug" / "dev" code from the production code
	// using the preprocessor so we can just disable all
	// of it from even being compiled. Someday TM.
	//              - sci4me, 5/15/20
	bool debug_pause = false;

	struct Batch_Renderer *batch_renderer;

	struct World *world;
	struct Player *player;

	void scroll_callback(f64 x, f64 y);
	void key_callback(s32 key, s32 scancode, s32 action, s32 mods);
	void window_size_callback(s32 width, s32 height);
	
	void init();
	void deinit();
	void update_and_render();

	// TODO: REMOVE ME
	s32 run();
};
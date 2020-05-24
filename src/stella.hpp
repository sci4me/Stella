#define GL_DEBUG
#define GL_MAJOR 4
#define GL_MINOR 4


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define SCI_H_IMPL
#define SCI_H_TEMP_STORAGE_ASSERT_NO_OVERRUN
#include "sci.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui_impl_opengl3.h"

// NOTE TODO: We will need to do this for hot code reloading!
// #define STBI_MALLOC(sz)           malloc(sz)
// #define STBI_REALLOC(p,newsz)     realloc(p,newsz)
// #define STBI_FREE(p)              free(p)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define RND_U32 u32
#define RND_U64 u64
#define RND_IMPLEMENTATION
#include "rnd.h"


struct Game {
	GLFWwindow *window;

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

	s32 run();
};
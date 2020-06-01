#include "platform_interface.hpp"


#define GL_DEBUG


struct Game {
	f32 scale = 1.0f;

	bool show_profiler = false;

	bool show_debug_window = false;
	bool show_imgui_metrics_window = false;

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
};
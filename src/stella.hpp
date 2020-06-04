// #define PROFILER_DISABLE


// TODO REMOVEME?
namespace prof {
	struct Profiler;
}


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

	struct Temporary_Storage *temp;

	#ifndef PROFILER_DISABLE
	struct prof::Profiler *profiler;
	#endif

	struct ImGui_Backend *imgui_backend;
	struct Batch_Renderer *batch_renderer;
	struct Assets *assets;
	struct Recipes *recipes;

	struct World *world;
	struct Player *player;
};


extern Game *g_inst;
extern OpenGL gl;
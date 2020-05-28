#include "stella.hpp"

#include "off_the_rails.cpp"
#include "math.cpp"
#include "static_bitset.cpp"
#include "static_array.cpp"
#include "dynamic_array.cpp"
#include "util.cpp"
#include "hash_table.cpp"
#include "profiler.cpp"
#include "perlin_noise.cpp"
#include "slot_allocator.cpp"
#include "shader.cpp"
#include "texture.cpp"
#include "buffer_objects.cpp"
#include "assets.cpp"
#include "imgui_support.cpp"
#include "batch_renderer.cpp"
#include "item.cpp"
#include "crafting.cpp"
#include "tile.cpp"
#include "world.cpp"
#include "ui.cpp"
#include "player.cpp"


#define GL_DEBUG


void dump_gl_extensions() {
    tprintf("  GL_EXTENSIONS\n");
        
    s32 n_ext;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n_ext);
    
    char const** exts = (char const**) talloc(n_ext * sizeof(char const*));

    u32 max_len = 0;
    for(s32 i = 0; i < n_ext; i++) {
        char const *ext = (char const*) glGetStringi(GL_EXTENSIONS, i);

        exts[i] = ext;

        u32 len = mlc_strlen(ext);
        if(len > max_len) {
            max_len = len;
        }
    }

    constexpr u32 cols = 2;
    u32 column_length = max_len + 2;
    u32 column = 0;
    for(u32 i = 0; i < n_ext; i++) {
        char const* ext = exts[i];
        u32 padding = column_length - mlc_strlen(ext);

        tmark();
        tprintf("%s", ext);
        for(u32 j = 0; j < padding; j++) tprintf(" ");

        column++;
        if(column > (cols - 1)) {
            column = 0;
            tprintf("\n");
        }
        treset();
    }

    if(column <= (cols - 1)) tprintf("\n");
}

void dump_gl_info() {
    // TODO: some kind of logging!!!

    tprintf("OpenGL Info:\n");

    GLint major, minor; 
    glGetIntegerv(GL_MAJOR_VERSION, &major); 
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    tprintf("  GL_MAJOR_VERSION              %d\n", major);
    tprintf("  GL_MINOR_VERSION              %d\n", minor);

    tprintf("  GL_VENDOR                     %s\n", glGetString(GL_VENDOR));
    tprintf("  GL_RENDERER                   %s\n", glGetString(GL_RENDERER));
    tprintf("  GL_VERSION                    %s\n", glGetString(GL_VERSION));
    tprintf("  GL_SHADING_LANGUAGE_VERSION   %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}


#ifdef GL_DEBUG
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == GL_DEBUG_SEVERITY_LOW) {
        return;
    }

    tfprintf(STDERR, "%s\n", message);
}
#endif


void Game::mouse_position_callback(s32 x, s32 y, bool valid) {
    imsupport::mouse_position_callback(x, y, valid);
}

void Game::mouse_button_callback(s32 button, bool is_press) {
    imsupport::mouse_button_callback(button, is_press);
}

void Game::scroll_callback(f64 deltaX, f64 deltaY) {
    imsupport::scroll_callback(deltaX, deltaY);

	ImGuiIO& io = ImGui::GetIO();
    if(!io.WantCaptureMouse) {
        scale = clamp<f32>(scale + deltaY * 0.05f, 0.1f, 5.0f);
    }
}

void Game::key_callback(u32 keycode, bool is_press) {
    if(is_press) {
        switch(keycode) {
            case KEYCODE_F2: {
                show_profiler = !show_profiler;
                break;
            }
            case KEYCODE_F3: {
                show_debug_window = !show_debug_window;
                break;
            }
            case KEYCODE_F12: {
                debug_pause = !debug_pause;
                break;
            }
        }
    }

	// NOTE: Here is where we dispatch this event to
	// "listeners". Trying to avoid the function-pointer
	// dynamic event bus type of system, so, for now we
    // hardcode the dispatch. May want to change this
    // to make the code nicer, later on.
	// 					- sci4me, 5/23/20

	player->key_callback(keycode, is_press);
}

void Game::window_size_callback(s32 width, s32 height) {
	window_width = width;
    window_height = height;
    
    glViewport(0, 0, window_width, window_height);
        
    auto projection_matrix = mat4::ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, 0.0f, 10000.0f);

    batch_renderer->set_projection(projection_matrix);
    world->set_projection(projection_matrix);
}

void Game::init() {
    #ifdef GL_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROCARB) ::gl_debug_callback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    #endif

    prof::init();


    imsupport::init();


    glClearColor(0, 0, 0, 0);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


    batch_renderer = (Batch_Renderer*) mlc_malloc(sizeof(Batch_Renderer));
    batch_renderer->init();


    assets::init();
    init_tiles();
    init_items();


    crafting::init();


    {
        void *mem = mlc_malloc(sizeof(World));
        world = new(mem) World;
        world->init();
    }


    {
        void *mem = mlc_malloc(sizeof(Player));
        player = new(mem) Player;
        player->init(this, world);
        player->pos = {496, 3637};
    }


    {
        // TODO REMOVEME TESTING
        player->inventory.insert({ ITEM_COBBLESTONE, MAX_ITEM_SLOT_SIZE });
        player->inventory.insert({ ITEM_IRON_ORE, MAX_ITEM_SLOT_SIZE });
        player->inventory.insert({ ITEM_GOLD_ORE, MAX_ITEM_SLOT_SIZE });
        player->inventory.insert({ ITEM_COAL_ORE, MAX_ITEM_SLOT_SIZE });
        player->inventory.insert({ ITEM_IRON_PLATE, MAX_ITEM_SLOT_SIZE });
        player->inventory.insert({ ITEM_CHEST, MAX_ITEM_SLOT_SIZE });
        player->inventory.insert({ ITEM_FURNACE, MAX_ITEM_SLOT_SIZE });
        player->inventory.insert({ ITEM_MINING_MACHINE, MAX_ITEM_SLOT_SIZE });
    }
}

void Game::deinit() {
    // TODO

    imsupport::deinit();
}

void Game::update_and_render() {
    bool is_paused = debug_pause;
    if(!is_paused) prof::begin_frame();


    glClear(GL_COLOR_BUFFER_BIT);


    // TODO: We don't want to just update, render, and loop;
    // we don't want to have to rely on V-SYNC to give us a fixed
    // update rate. Instead, we should keep track of time and
    // call update as needed. Or something. TM.
    //
    // Also, sooner-than-later, we're going to need to have a more
    // formal way of tracking time anyway since we need to have
    // "ticks" for our update functions for things like machines...
    // So, yeah, do this. Soon. Or something. Maybe. I guess. ???
    // (Oh yeah, uh, the thing that made me think to say this was that
    // uh, we need to be able to actually keep track of in-game time;
    // i.e. how many ticks are in a "day", what is the current tick 
    // since world creation, what day is it, is it day/night, etc.)
    //
    //              - sci4me, 5/13/20

    if(!is_paused) {
        world->update();
        player->update();
    }    


    imsupport::begin_frame(vec2(window_width, window_height));


    //
    // NOTE: We only have to recompute the view matrix if:
    // 1. the scale changes
    // 2. the window size changes
    // 3. the player moves
    //
    // Since the player is probably going to be moving
    // for like 50%+ of the time, it may not be worth it
    // to cache these. *shrugs*
    // If it's an issue, SIMD-ize the matrix multiply.
    //              
    //              - sci4me, 5/18/20
    //

    u32 chunk_draw_calls;
    auto s = mat4::scale(scale, scale, 1.0f);
    auto t = mat4::translate((window_width / 2 / scale) - player->pos.x, (window_height / 2 / scale) - player->pos.y, 0.0f);
    auto view = t * s;
    batch_renderer->begin(view);
    {
        // NOTE: We render the world from within the Batch_Renderer frame since
        // we are currently using the Batch_Renderer for any tiles that
        // aren't on layer 0.
        //              - sci4me, 5/9/20
        chunk_draw_calls = world->draw_around(batch_renderer, player->pos, scale, window_width, window_height, view);

        player->draw(batch_renderer);

    }
    auto per_frame_stats = batch_renderer->end_frame();

    if(show_debug_window) {
        ImGuiIO& io = ImGui::GetIO();

        if(ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {
            ImGui::Dummy({ 130, 0 });

            if(ImGui::CollapsingHeader("Misc.")) {
                ImGui::Text("FPS: %.1f", io.Framerate);
                ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
            }

            if(ImGui::CollapsingHeader("Player")) {
                ImGui::Text("Position: (%0.3f, %0.3f)", player->pos.x, player->pos.y);
                ImGui::Text("Tile Position: (%d, %d)", (s32) floorf32(player->pos.x / TILE_SIZE), (s32) floorf32(player->pos.y / TILE_SIZE));
            }

            if(ImGui::CollapsingHeader("World")) {
                ImGui::Text("Scale: %0.3f", scale);
                ImGui::Text("Total Chunks: %d", world->chunks.count);
                ImGui::Text("Chunk Draw Calls: %d", chunk_draw_calls);
            }

            if(ImGui::CollapsingHeader("Batch Renderer")) {
                ImGui::Text("Quads: %u", per_frame_stats.quads);
                ImGui::Text("Vertices: %u", per_frame_stats.vertices);
                ImGui::Text("Indices: %u", per_frame_stats.indices);
                ImGui::Text("Draw Calls: %u", per_frame_stats.draw_calls);
            }

            if(ImGui::CollapsingHeader("Settings")) {
                ImGui::Checkbox("Fast Mining", &fast_mining);
                ImGui::Checkbox("Show ImGui Metrics", &show_imgui_metrics_window);
            }
        }

        if(show_imgui_metrics_window) {
            ImGui::ShowMetricsWindow();
        }

        ImGui::End();
    }

    // NOTE TODO: This next line is kind of a hack...
    // if(!is_paused) prof::end_frame(); else prof::frame_events.clear();
    if(show_profiler) prof::show();

    imsupport::end_frame();

    tclear();
}


extern "C" GAME_INIT(stella_init) {
    dump_gl_info();
    // dump_gl_extensions();

    // TODO
}

extern "C" GAME_UPDATE_AND_RENDER(stella_update_and_render) {
    // TODO
}
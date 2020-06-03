#include "stella.hpp"


// NOTE: I have yet to decide whether I truly want these
// to be globals or not. So far it's been fine but... eh.
// Sometimes I feel like I'd be more comfortable
// accessing them through the PlatformAPI struct. EXCEPT...
// In the static compilation case, that doesn't make sense!
// What to do? *shrugs*
//                  - sci4me, 6/1/20
PLATFORM_API_FUNCTIONS


#ifdef STELLA_DYNAMIC
#include "mylibc.cpp" // TODO REMOVEME
#endif


// NOTE: We have to include this before the defines for
// stb_image. This has bothered me for some time now but,
// yknow. Sometimes it do be like that.
#include "maths.hpp"


#define IMGUI_USER_CONFIG "stella_imconfig.hpp"
#include "imgui/imgui.h"


// NOTE TODO: We're duplicating stb_sprintf here!
// We should really just suck it up and make it
// a "platform API function" even thought it's
// not platform-specific at all. lol
#ifdef STELLA_DYNAMIC
#define STB_SPRINTF_IMPLEMENTATION
#endif
#include "stb_sprintf.h"


#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_GIF
#define STBI_NO_LIBC
#define STBI_ASSERT(x)              assert(x)
#define STBI_MALLOC(n)              mlc_malloc(n)
#define STBI_REALLOC(p, n)          mlc_realloc(p, n)
#define STBI_FREE(p)                mlc_free(p)
#define STBI_LDEXPF(a, b)           ldexpf32(a, b)
#define STBI_POWF(a, b)             powf32(a, b)
#define STBI_MEMSET(p, x, n)        mlc_memset(p, x, n)
#define STBI_MEMCPY(d, s, n)        mlc_memcpy(d, s, n)
#define STBI_ABS(x)                 abs32(x)
#define STBI_STRCMP(a, b)           mlc_strcmp(a, b)
#define STBI_STRNCMP(a, b, n)       mlc_strncmp(a, b, n)
#define STBI_STRTOL(s, e, b)        mlc_strtol(s, e, b)
#include "stb_image.h" 


#define RND_U32 u32
#define RND_U64 u64
#define RND_IMPLEMENTATION
#include "rnd.h"


#define GLEW_STATIC
#define GLEW_NO_GLU
#include "GL/glew.h"


#include "temporary_storage.cpp"
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
#include "tile.hpp"
#include "item.hpp"
#include "assets.cpp"
#include "imgui_backend.cpp"
#include "batch_renderer.cpp"
#include "item.cpp"
#include "crafting.cpp"
#include "tile.cpp"
#include "world.cpp"
#include "ui.cpp"
#include "player.cpp"


void dump_gl_extensions() {
    tprintf("  GL_EXTENSIONS\n");
        
    s32 n_ext;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n_ext);
    
    char const** exts = (char const**) talloc(n_ext * sizeof(char const*));
    assert(exts);

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


// NOTE: Yes, this is stupid. But, ya know.
// Sometimes it do be like that.
void* stella_im_malloc(u64 n) { return mlc_malloc(n); }
void stella_im_free(void *p) { mlc_free(p); }


Game *g_inst;


void init_gl(Game *g) {
    #ifdef GL_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROCARB) ::gl_debug_callback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    #endif

    glClearColor(0, 0, 0, 0);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


    dump_gl_info();
    // dump_gl_extensions();
}


#ifdef STELLA_DYNAMIC
extern "C" GAME_ATTACH(stella_attach) {
    Game *g = (Game*) pio->game_memory;
    g_inst = g;

    #define UNPACK(name, ret, params) name = pio->api.name;
    _PLATFORM_API_FUNCTIONS(UNPACK)
    #undef UNPACK


    // NOTE: This is far from ideal, but, eh,
    // saves me a lot of time and pain, for now.
    //
    // In case it wasn't obvious, the reason this
    // is unideal is that we're re-loading all of
    // the GL functions every time we re-load the
    // dylib (the game code). Since that only
    // happens in dev mode anyway, maybe this is fine?
    assert(glewInit() == GLEW_OK);


    if(reload) {
        g->imgui_backend->attach();
    }
}
#endif

extern "C" GAME_INIT(stella_init) {
    // NOTE: Maybe don't duplicate this?...
    #ifdef STELLA_STATIC
    assert(glewInit() == GLEW_OK);
    #endif


    void *mem = mlc_malloc(sizeof(Game));
    Game *g = new(mem) Game;
    g_inst = g;
    pio->game_memory = g;


    init_gl(g);


    #ifndef PROFILER_DISABLE
    g->profiler = (prof::Profiler*) mlc_malloc(sizeof(prof::Profiler));
    g->profiler->init();
    #endif


    g->imgui_backend = (ImGui_Backend*) mlc_malloc(sizeof(ImGui_Backend));
    g->imgui_backend->init();


    g->batch_renderer = (Batch_Renderer*) mlc_malloc(sizeof(Batch_Renderer));
    g->batch_renderer->init();


    g->assets = (Assets*) mlc_malloc(sizeof(Assets));
    g->assets->init();


    g->recipes = (Recipes*) mlc_malloc(sizeof(Recipes));
    g->recipes->init();


    {
        void *mem = mlc_malloc(sizeof(World));
        g->world = new(mem) World;
        g->world->init();
    }


    {
        void *mem = mlc_malloc(sizeof(Player));
        g->player = new(mem) Player;
        g->player->init(g, g->world);
    }


    {
        // TODO REMOVEME TESTING
        g->player->pos = {496, 3637};
        g->player->inventory.insert({ ITEM_COBBLESTONE, MAX_ITEM_SLOT_SIZE });
        g->player->inventory.insert({ ITEM_IRON_ORE, MAX_ITEM_SLOT_SIZE });
        g->player->inventory.insert({ ITEM_GOLD_ORE, MAX_ITEM_SLOT_SIZE });
        g->player->inventory.insert({ ITEM_COAL_ORE, MAX_ITEM_SLOT_SIZE });
        g->player->inventory.insert({ ITEM_IRON_PLATE, MAX_ITEM_SLOT_SIZE });
        g->player->inventory.insert({ ITEM_CHEST, MAX_ITEM_SLOT_SIZE });
        g->player->inventory.insert({ ITEM_FURNACE, MAX_ITEM_SLOT_SIZE });
        g->player->inventory.insert({ ITEM_MINING_MACHINE, MAX_ITEM_SLOT_SIZE });
    }
}

extern "C" GAME_DEINIT(stella_deinit) {
    Game *g = (Game*) pio->game_memory;
    
    #define DEINIT_AND_FREE(name) g->name->deinit(); mlc_free(g->name);

    #ifndef PROFILER_DISABLE
    DEINIT_AND_FREE(profiler);
    #endif

    DEINIT_AND_FREE(batch_renderer);
    DEINIT_AND_FREE(assets);
    DEINIT_AND_FREE(imgui_backend);
    DEINIT_AND_FREE(recipes);
    DEINIT_AND_FREE(world);
    DEINIT_AND_FREE(player);

    mlc_free(g);

    #undef DEINIT_AND_FREE
}   

extern "C" GAME_UPDATE_AND_RENDER(stella_update_and_render) {
    Game *g = (Game*) pio->game_memory;
    

    bool is_paused = g->debug_pause;
    if(!is_paused) prof::begin_frame();


    if(pio->window_just_resized) {
        glViewport(0, 0, pio->window_width, pio->window_height);
        
        auto projection_matrix = mat4::ortho(0.0f, (f32)pio->window_width, (f32)pio->window_height, 0.0f, 0.0f, 10000.0f);
        g->batch_renderer->set_projection(projection_matrix);
        g->world->set_projection(projection_matrix);
    }


    ImGuiIO& io = ImGui::GetIO();
    if(!io.WantCaptureMouse && pio->mouse_wheel_y != 0.0f) {
        g->scale = clamp<f32>(g->scale + pio->mouse_wheel_y * 0.05f, 0.1f, 5.0f);
    }


    if(pio->was_button_pressed(VK_F2))  g->show_profiler = !g->show_profiler;
    if(pio->was_button_pressed(VK_F3))  g->show_debug_window = !g->show_debug_window;
    if(pio->was_button_pressed(VK_F12)) g->debug_pause = !g->debug_pause;


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
        g->world->update();
        g->player->update(pio);
    }    


    g->imgui_backend->begin_frame(pio);


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
    auto s = mat4::scale(g->scale, g->scale, 1.0f);
    auto t = mat4::translate((pio->window_width / 2 / g->scale) - g->player->pos.x, (pio->window_height / 2 / g->scale) - g->player->pos.y, 0.0f);
    auto view = t * s;
    g->batch_renderer->begin(view);
    {
        // NOTE: We render the world from within the Batch_Renderer frame since
        // we are currently using the Batch_Renderer for any tiles that
        // aren't on layer 0.
        //              - sci4me, 5/9/20
        chunk_draw_calls = g->world->draw_around(g->batch_renderer, g->player->pos, g->scale, pio->window_width, pio->window_height, view);

        g->player->draw(g->batch_renderer);

    }
    auto per_frame_stats = g->batch_renderer->end_frame();

    if(g->show_debug_window) {
        if(ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {
            ImGui::Dummy({ 130, 0 });

            if(ImGui::CollapsingHeader("Misc.")) {
                ImGui::Text("FPS: %.1f", io.Framerate);
                ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
            }

            if(ImGui::CollapsingHeader("Player")) {
                ImGui::Text("Position: (%0.3f, %0.3f)", g->player->pos.x, g->player->pos.y);
                ImGui::Text("Tile Position: (%d, %d)", (s32) floorf32(g->player->pos.x / TILE_SIZE), (s32) floorf32(g->player->pos.y / TILE_SIZE));
                ImGui::Text("Last dPos: (%f, %f)", g->player->last_dpos.x, g->player->last_dpos.y);
            }

            if(ImGui::CollapsingHeader("World")) {
                ImGui::Text("Scale: %0.3f", g->scale);
                ImGui::Text("Total Chunks: %d", g->world->chunks.count);
                ImGui::Text("Chunk Draw Calls: %d", chunk_draw_calls);
            }

            if(ImGui::CollapsingHeader("Batch Renderer")) {
                ImGui::Text("Quads: %u", per_frame_stats.quads);
                ImGui::Text("Vertices: %u", per_frame_stats.vertices);
                ImGui::Text("Indices: %u", per_frame_stats.indices);
                ImGui::Text("Draw Calls: %u", per_frame_stats.draw_calls);
            }

            if(ImGui::CollapsingHeader("Settings")) {
                ImGui::Checkbox("Fast Mining", &g->fast_mining);
                ImGui::Checkbox("Show ImGui Metrics", &g->show_imgui_metrics_window);
            }
        }

        if(g->show_imgui_metrics_window) {
            ImGui::ShowMetricsWindow();
        }

        ImGui::End();
    }

    // NOTE TODO: This next line is kind of a hack...
    if(!is_paused) prof::end_frame(); else prof::clear_frame_events();
    if(g->show_profiler) prof::show();

    g->imgui_backend->end_frame();

    tclear();
}
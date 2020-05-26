#include "temporary_storage.cpp"
#include "off_the_rails.cpp"
#include "math.cpp"
#include "static_bitset.cpp"
#include "static_array.cpp"
#include "dynamic_array.cpp"
#include "util.cpp"
#include "hash_table.cpp"
#include "profiler.cpp"
#include "imgui_support.cpp"
#include "perlin_noise.cpp"
#include "slot_allocator.cpp"
#include "shader.cpp"
#include "texture.cpp"
#include "buffer_objects.cpp"
#include "batch_renderer.cpp"
#include "assets.cpp"
#include "item.cpp"
#include "crafting.cpp"
#include "tile.cpp"
#include "world.cpp"
#include "ui.cpp"
#include "player.cpp"

#ifdef GL_DEBUG
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == GL_DEBUG_SEVERITY_LOW) {
        return;
    }

    tfprintf(STDERR, "%s\n", message);
}
#endif

void Game::scroll_callback(s32 delta) {
    /*
	ImGuiIO& io = ImGui::GetIO();
    if(!io.WantCaptureMouse) {
    }
    */
    scale = clamp<f32>(scale + delta * 0.05f, 0.1f, 5.0f);
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
    window_resized = true;
}

void Game::init() {
    #ifdef GL_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROCARB) ::gl_debug_callback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    #endif

    prof::init();


    imgui_init();


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

}

void Game::update_and_render() {
    bool is_paused = debug_pause;
    if(!is_paused) prof::begin_frame();


    if(window_resized) {
        window_resized = false;

        glViewport(0, 0, window_width, window_height);
        
        auto projection_matrix = mat4::ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, 0.0f, 10000.0f);

        batch_renderer->set_projection(projection_matrix);
        world->set_projection(projection_matrix);
    }

    // TODO: Fullscreen

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


    imgui_begin_frame();


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
                if(ImGui::Checkbox("V-SYNC", &vsync)) {
                    // TODO
                    //if(vsync) glfwSwapInterval(1);
                    //else      glfwSwapInterval(0);
                }
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

    imgui_end_frame();
}

/*
s32 Game::run() {
	tinit();

	if (!glfwInit()) {
        fprintf(stderr, "Failed to initialized GLFW3!\n");
        return 1;
    }

    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef GL_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif

    window = glfwCreateWindow(window_width, window_height, "Stella", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window!\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, ::scroll_callback);
    glfwSetKeyCallback(window, ::key_callback);
    glfwSetWindowSizeCallback(window, ::window_size_callback);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW!\n");
        glfwTerminate();
        return 1;
    }


    dump_gl_info();


    #ifdef GL_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROCARB) ::gl_debug_callback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    #endif


    time_t t;
    srand((unsigned) time(&t));


    prof::init();


    imgui_init(window);
    ImGuiIO& io = ImGui::GetIO();


    glClearColor(0, 0, 0, 0);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


    batch_renderer = (Batch_Renderer*) malloc(sizeof(Batch_Renderer));
    batch_renderer->init();


    assets::init();
    init_tiles();
    init_items();


    crafting::init();


    {
        void *mem = malloc(sizeof(World));
        world = new(mem) World;
        world->init();
    }

    {
        void *mem = malloc(sizeof(Player));
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



    u32 chunk_draw_calls = 0;
    mat4 projection_matrix;
    f64 last_time = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
    	bool is_paused = debug_pause;
    	if(!is_paused) prof::begin_frame();

        glfwPollEvents();


        if(window_resized) {
            window_resized = false;

            glViewport(0, 0, window_width, window_height);
            
            projection_matrix = mat4::ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, 0.0f, 10000.0f);

            batch_renderer->set_projection(projection_matrix);
            world->set_projection(projection_matrix);
        }

        if(fullscreen_changed) {
            fullscreen_changed = false;

            if(fullscreen) {
                glfwGetWindowPos(window, &saved_window_x, &saved_window_y);
                glfwGetWindowSize(window, &saved_window_w, &saved_window_h);

                auto monitor = glfwGetPrimaryMonitor();
                auto mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, 0);
            } else {
                glfwSetWindowMonitor(window, nullptr, saved_window_x, saved_window_y, saved_window_w, saved_window_h, 0);
            }

            continue;
        }


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


        imgui_begin_frame();


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
            if(ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {
                ImGui::Dummy({ 130, 0 });

                if(ImGui::CollapsingHeader("Misc.")) {
                    ImGui::Text("FPS: %.1f", io.Framerate);
                    ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
                }

                if(ImGui::CollapsingHeader("Player")) {
                    ImGui::Text("Position: (%0.3f, %0.3f)", player->pos.x, player->pos.y);
                    ImGui::Text("Tile Position: (%d, %d)", (s32) floor(player->pos.x / TILE_SIZE), (s32) floor(player->pos.y / TILE_SIZE));
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
                    if(ImGui::Checkbox("V-SYNC", &vsync)) {
                        if(vsync) glfwSwapInterval(1);
                        else      glfwSwapInterval(0);
                    }
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
        if(!is_paused) prof::end_frame(); else prof::frame_events.clear();
        if(show_profiler) prof::show();

    	imgui_end_frame();

        glfwSwapBuffers(window);

        treset();
    }

    batch_renderer->deinit();

    world->deinit();
    free(world);

    player->deinit();
    free(player);

    crafting::deinit();

    imgui_shutdown();

    prof::deinit();

    glfwTerminate();

    tfree();
    return 0;
}
*/
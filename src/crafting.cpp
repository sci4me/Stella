namespace crafting {
    struct Recipe {
        Item_Stack *inputs;
        u32 n_inputs;
        Item_Stack output;
        u32 time; // in ticks
    };

    // NOTE: Currently we store Recipes as an output itemstack and
    // a list of input itemstacks.
    // This means that when we want to iterate over the entire tree
    // of a recipe (i.e. a recipe that has intermediate recipes)
    // we must do some kind of indirect lookup, to go from the input
    // itemstack to the recipe for crafting that item stack.
    // Maybe this is fine, maybe we don't care. Hash tables are fast TM.
    // But, if we wanted to have a homogenous structure for this, what we
    // could do is this: instead of Recipe containing a list of itemstacks
    // as inputs, it would just contain a list of recipes and a count for each
    // recipe. Then, to represent "base items" (items that don't need to be
    // crafted with a recipe (iron ore, iron plate, etc.)) we just have a Recipe
    // whose only input is that item itself and whose output is also that item;
    // a no-op Recipe.
    //
    // Not sure if we'll ever have any reason to do this really. But hey, it's a
    // possible thing. So. Yeet. Me into a grave.
    //
    //                  - sci4me, 5/17/20
    //


    // TODO: Whenever I get around to making the Arena in sci.h
    // have a configurable block size, consider using that here
    // instead of malloc.
    //              - sci4me, 5/14/20
    Recipe **recipes = nullptr;
    struct {
        Item_Type key;
        Recipe *value;
    } *output_type_to_recipe = nullptr;

    void register_recipe(Item_Stack output, u32 time, std::initializer_list<Item_Stack> inputs) {
        auto r = (Recipe*) malloc(sizeof(Recipe) + sizeof(Item_Stack) * inputs.size());

        r->inputs = (Item_Stack*) (r + 1); // inputs are right after the Recipe struct
        r->n_inputs = inputs.size();
        r->output = output;
        r->time = time;

        u32 i = 0;
        for(auto s : inputs) {
            r->inputs[i++] = s;
        }

        arrput(recipes, r);
        hmput(output_type_to_recipe, output.type, r);
    }

    void init() {
        register_recipe({ ITEM_FURNACE, 1 },        100, { { ITEM_COBBLESTONE, 8 } });
        register_recipe({ ITEM_CHEST, 1 },          100, { { ITEM_COBBLESTONE, 8 }, { ITEM_IRON_PLATE, 4 } });
        register_recipe({ ITEM_IRON_GEAR, 1 },      100, { { ITEM_IRON_PLATE, 6 } });
        register_recipe({ ITEM_MINING_MACHINE, 1 }, 100, { { ITEM_COBBLESTONE, 16 }, { ITEM_IRON_PLATE, 8 }, { ITEM_IRON_GEAR, 4 } });
    }

    void free() {
        for(u32 i = 0; i < arrlen(recipes); i++) {
            ::free(recipes[i]);
        }
        arrfree(recipes);

        hmfree(output_type_to_recipe);
    }


    // NOTE TODO: Some ideas on how we can fix Queue:
    //  - Move most of its state into Queue::Job
    //    request_index, request_count, crafting_time, progress
    //  - Instead of using Dynamic_Item_Container, we could just
    //    use Job::have! Right?!
    //              - sci4me, 5/18/20
    struct Queue {
        struct Job {
            struct Request {
                Recipe *recipe;
                u32 count;
            };

            bool inputs_available;
            u32 have[N_ITEM_TYPES];
            Request *request;
            bool started = false;
            u32 request_index;
            u32 request_count;
            u32 crafting_time;
            u32 progress;

            void deinit() {
                arrfree(request);
            }

            static Job calculate(Recipe *recipe, Item_Container *player_inventory) {
                Job result = {};
                result.inputs_available = !result.calculate(recipe, 1, player_inventory);
                return result;
            }

        private:
            bool calculate(Recipe *recipe, u32 count, Item_Container *player_inventory) {
                bool missing_something = false;

                for(u32 i = 0; i < recipe->n_inputs; i++) {
                    auto const& input = recipe->inputs[i];
                    Item_Stack needed = { input.type, input.count * count };
        
                    u32 n = player_inventory->count_type(needed.type);
                    have[needed.type] += min(n, needed.count);

                    if(n < needed.count) {
                        u32 r = needed.count - n;
                        auto idx = hmgeti(output_type_to_recipe, needed.type);
                        if(idx == -1) {
                            missing_something = true;
                        } else {
                            missing_something = missing_something || calculate(output_type_to_recipe[idx].value, r, player_inventory);
                        }
                    }
                }

                if(!missing_something) {
                    Request req = { recipe, count };
                    arrput(request, req);
                }

                return missing_something;
            }
        };

        Item_Container *player_inventory;
        Job *queue = nullptr;

        bool actively_crafting = false;
        bool crafting_paused = false;

        void init(Item_Container *player_inventory) {
            this->player_inventory = player_inventory;
        }

        void deinit() {
            arrfree(queue);
        }

        bool request(Recipe *r) {
            Job job = Job::calculate(r, player_inventory);
            
            if(!job.inputs_available) {
                job.deinit();
                return false;
            }

            for(u32 i = 0; i < N_ITEM_TYPES; i++) {
                Item_Stack stack = { (Item_Type) i, job.have[i] };
                if(job.have[i] > 0) assert(player_inventory->remove(stack, false));
            }
            player_inventory->sort();

            arrput(queue, job);
            return true;
        }

        void update() {
            actively_crafting = arrlen(queue) > 0;
            if(crafting_paused) return;
            if(!actively_crafting) return;
            
            auto& job = queue[0];
            auto const& req = job.request[job.request_index];

            if(!job.started) {
                // NOTE: This is the initial state before
                // we begin to process a Request. We must
                // initialize the crafting_time from the
                // recipe of this Request. We also reset
                // progress to 0. And of course, set started.

                job.started = true;
                job.request_count = 0;
                job.crafting_time = req.recipe->time;
                job.progress = 0;
                return;
            }

            // NOTE: If we get this far, we have an active Request.

            if(job.progress < job.crafting_time) {
                // NOTE: In this case, we are currently "crafting"
                // a single instance of a Recipe; 1 out of `count`.

                job.progress++;
                return;
            }

            // NOTE: If we get this far, we have just completed "crafting"
            // a single 1/`count` of the active Request.
            job.request_count++;

            // NOTE: This is basically uh.. yknow.. O(n*m).. but n and m
            // are always going to be pretty small. So. We should be fine.
            // ... Well, actually, it's O(n*N_ITEM_TYPES), so, if we ever
            // have tons of different item types, this might add up. But.
            // Yeeeh.
            for(u32 i = 0; i < req.recipe->n_inputs; i++) {
                auto const& input = req.recipe->inputs[i];
                for(u32 j = 0; j < N_ITEM_TYPES; j++) {
                    auto type = (Item_Type) j;
                    if(input.type != type) continue;

                    assert(job.have[type] >= input.count);
                    job.have[type] -= input.count;
                }
            }
            job.have[req.recipe->output.type] += req.recipe->output.count;
            printf("Crafted %s\n", item_names[req.recipe->output.type]);

            if(job.request_count < req.count) {
                // NOTE: In this case, we have crafted <req.count;
                // keep going!

                job.progress = 0;
                return;
            }

            // NOTE: If we made it this far, we have crafted
            // an entire Request! So, increment request_index
            // to move on to the next one...

            job.request_index++;

            if(job.request_index < arrlen(job.request)) {
                // NOTE: In this case, there are more Requests to process in
                // this Job. Set started to false in order to re-init the
                // Job with the current Request.

                job.started = false;
            } else {
                // NOTE: In this case, we finished the Job!

                for(u32 i = 0; i < N_ITEM_TYPES; i++) {
                    auto type = (Item_Type) i;
                    if(type == req.recipe->output.type) {
                        assert(job.have[type] == req.recipe->output.count);
                        player_inventory->insert(Item_Stack(type, job.have[type]));
                    } else {
                        assert(job.have[i] == 0);
                    }
                }

                queue[0].deinit();
                arrdel(queue, 0);
            }
        }

        void draw() {
            ImGui::Begin("Crafting Queue", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
                
            ImGui::Checkbox("Pause", &crafting_paused);

            ImGui::Separator();

            f32 crafting_progress = 0.0f;
            if(actively_crafting) {
                crafting_progress = (f32)queue[0].progress / (f32)queue[0].crafting_time;
            }
            ImGui::ProgressBar(crafting_progress, {100, 14});


            // TODO: This is pretty crappy lol...
            ImGui::BeginChild("Queue", { 100, 200 });

            // TODO: Remove this if statement!
            if(actively_crafting) {
                for(u32 i = 0; i < arrlen(queue); i++) {
                    auto const& job = queue[i];

                    u32 start;
                    if(i == 0) start = job.request_index;
                    else start = 0;

                    for(u32 k = start; k < arrlen(job.request); k++) {
                        auto const& req = job.request[k];

                        u32 end;
                        if(i == 0 && k == start) end = req.count - job.request_count;
                        else end = req.count;

                        // TODO: Instead of _just_ looping here, count how many of these,
                        // consequitively, are the same recipe being requested, and display
                        // that count instead of that many of the recipe output, separately.
                        for(u32 j = 0; j < end; j++) {
                            ImGui::PushID(i); // TODO: This is silly.
                            ImGui::PushID(k);
                            ImGui::PushID(j);
                            if(ImGui::ImageButton((ImTextureID)(u64)item_textures[req.recipe->output.type].id, { 32, 32 })) {
                                cancel_job(i);
                                
                                ImGui::PopID();
                                ImGui::PopID();
                                ImGui::PopID();
                                goto early_out; // TODO: Remove this! Bad sci4me! Bad! Down boy!
                            }
                            ImGui::PopID();
                            ImGui::PopID();
                            ImGui::PopID();
                        } 
                    }
                }
            }

            early_out:
            ImGui::EndChild();

            ImGui::End();
        }

    private:
        void cancel_job(u32 job_index) {
            auto const& job = queue[job_index];
            for(u32 i = 0; i < N_ITEM_TYPES; i++) {
                auto type = (Item_Type) i;
                if(job.have[i] > 0) player_inventory->insert(Item_Stack(type, job.have[i]));
            }
            arrdel(queue, job_index);
        }
    };
}
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

    Dynamic_Array<Recipe*> recipes;
    Hash_Table<Item_Type, Recipe*> output_type_to_recipe;

    template<typename... T>
    void register_recipe(Item_Stack output, u32 time, T... _inputs) {
        static_assert((otr::type_eq<T, Item_Stack> && ...));
        
        auto n_inputs = sizeof...(_inputs);
        Item_Stack inputs[] = { _inputs... };

        // TODO: Whenever I get around to making the Arena in sci.h
        // have a configurable block size, consider using that here
        // instead of malloc.
        //              - sci4me, 5/14/20
        auto r = (Recipe*) malloc(sizeof(Recipe) + sizeof(Item_Stack) * n_inputs);

        r->inputs = (Item_Stack*) (r + 1); // inputs are right after the Recipe struct
        r->n_inputs = n_inputs;
        r->output = output;
        r->time = time;

        for(u32 i = 0; i < n_inputs; i++) {
            r->inputs[i] = inputs[i];;
        }

        recipes.push(r);
        output_type_to_recipe.set(output.type, r);
    }

    void init() {
        recipes.init();
        output_type_to_recipe.init();

        register_recipe(Item_Stack(ITEM_FURNACE, 1),         100, Item_Stack(ITEM_COBBLESTONE, 8));
        register_recipe(Item_Stack(ITEM_CHEST, 1),           100, Item_Stack(ITEM_COBBLESTONE, 8), Item_Stack(ITEM_IRON_PLATE, 4));
        register_recipe(Item_Stack(ITEM_IRON_GEAR, 1),       100, Item_Stack(ITEM_IRON_PLATE, 6));
        register_recipe(Item_Stack(ITEM_MINING_MACHINE, 1),  100, Item_Stack(ITEM_COBBLESTONE, 16), Item_Stack(ITEM_IRON_PLATE, 8), Item_Stack(ITEM_IRON_GEAR, 4));
    }

    void free() {
        for(u32 i = 0; i < recipes.count; i++) {
            ::free(recipes[i]);
        }
        
        recipes.deinit();
        output_type_to_recipe.deinit();
    }


    struct Queue {
        struct Job {
            struct Request {
                Recipe *recipe;
                u32 count;
            };

            bool inputs_available;
            u32 have[N_ITEM_TYPES];
            Dynamic_Array<Request> request;
            u32 request_index;
            u32 request_count;
            u32 crafting_time;
            u32 progress;

            void init() {
                request.init();
            }

            void deinit() {
                request.deinit();
            }

            bool next_request() {
                request_index++;
                request_count = 0;
                progress = 0;

                bool result = request_index < request.count;
                if(result) crafting_time = request[request_index].recipe->time;
                return result;
            }

            bool next() {
                request_count++;
                progress = 0;
                return request_count < request[request_index].count;
            }

            Request& current() {
                return request[request_index];
            }

            bool craft() {
                if(progress < crafting_time) {
                    progress++;
                    return false;
                } else {
                    auto req = current();
                    for(u32 i = 0; i < req.recipe->n_inputs; i++) {
                        auto const& input = req.recipe->inputs[i];
                        for(u32 j = 0; j < N_ITEM_TYPES; j++) {
                            auto type = (Item_Type) j;
                            if(input.type != type) continue;

                            assert(have[type] >= input.count);
                            have[type] -= input.count;
                        }
                    }
                    have[req.recipe->output.type] += req.recipe->output.count;

                    return true;
                }
            }

            void start(Item_Container *player_inventory) {
                for(u32 i = 0; i < N_ITEM_TYPES; i++) {
                    Item_Stack stack = { (Item_Type) i, have[i] };
                    if(have[i] > 0) {
                        // NOTE TODO: There is a bug that can cause this assert to fail.
                        // Probably not going to debug it _now_.
                        // To reproduce, start crafting a mining machine. Then, start crafting
                        // an iron gear. Then start crafting another mining machine.
                        // This assert fails when you do this. No idea why.
                        //                  - sci4me, 5/20/20
                        assert(player_inventory->remove(stack, false));
                    }
                }
                player_inventory->sort();

                request_index = 0;
                request_count = 0;
                crafting_time = request[0].recipe->time;
                progress = 0;
            }

            static Job calculate(Recipe *recipe, Item_Container *player_inventory) {
                Job result = {};
                result.init();
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
                        s32 idx = output_type_to_recipe.index_of(needed.type);
                        if(idx == -1) {
                            missing_something = true;
                        } else {
                            missing_something = missing_something || calculate(output_type_to_recipe.slots[idx].value, r, player_inventory);
                        }
                    }
                }

                if(!missing_something) {
                    request.push({ recipe, count });
                }

                return missing_something;
            }
        };

        Item_Container *player_inventory;
        Dynamic_Array<Job> queue;

        bool crafting_paused = false;

        void init(Item_Container *player_inventory) {
            this->player_inventory = player_inventory;
            queue.init();
        }

        void deinit() {
            queue.deinit();
        }

        bool request(Recipe *r) {
            Job job = Job::calculate(r, player_inventory);

            if(!job.inputs_available) {
                job.deinit();
                return false;
            }

            job.start(player_inventory);

            queue.push(job);
            return true;
        }

        void update() {
            if(crafting_paused) return;
            if(queue.count == 0) return;
            
            auto& job = queue[0];
            auto& req = job.current();

            if(!job.craft()) {
                // NOTE: We have not finished processing "1/`count`"
                // of the current Request. Aka progress < crafting_time.
                return;
            }

            // NOTE: We finished processing "1/`count`" of the current Request.
            // (progress == crafting_time)

            if(job.next()) {
                // NOTE: We have processed <`count` of the current Request.
                return;
            }

            // NOTE: We have processed `count` of the current Request.
            // Try to move on to the next Request.

            if(job.next_request()) {
                // NOTE: There is a next request. Continue processing as normal.
                return;
            }

            // NOTE: We finished processing the Job! Transfer the items 
            // to the `player_inventory` and remove the Job from the queue.

            for(u32 i = 0; i < N_ITEM_TYPES; i++) {
                auto type = (Item_Type) i;
                if(type == req.recipe->output.type) {
                    assert(job.have[type] == req.recipe->output.count);
                    assert(player_inventory->insert(Item_Stack(type, job.have[type])) == 0); // TODO: Handle this case!
                } else {
                    assert(job.have[i] == 0);
                }
            }

            queue[0].deinit();
            queue.ordered_remove(0);
        }

        void draw() {
            ImGui::Begin("Crafting Queue", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
                
            ImGui::Checkbox("Pause", &crafting_paused);

            ImGui::Separator();

            auto queue_len = queue.count;
            f32 crafting_progress = 0.0f;
            if(queue_len) {
                crafting_progress = (f32)queue[0].progress / (f32)queue[0].crafting_time;
            }
            ImGui::ProgressBar(crafting_progress, {100, 14});


            // TODO: This is pretty crappy lol...
            ImGui::BeginChild("Queue", { 100, 200 });

            for(u32 i = 0; i < queue_len; i++) {
                auto const& job = queue[i];

                u32 start;
                if(i == 0) start = job.request_index;
                else start = 0;

                for(u32 k = start; k < job.request.count; k++) {
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

            early_out:
            ImGui::EndChild();

            ImGui::End();
        }

    private:
        void cancel_job(u32 job_index) {
            auto& job = queue[job_index];
            for(u32 i = 0; i < N_ITEM_TYPES; i++) {
                auto type = (Item_Type) i;
                if(job.have[i] > 0) player_inventory->insert(Item_Stack(type, job.have[i]));
            }
            job.deinit();
            queue.ordered_remove(job_index);
        }
    };
}
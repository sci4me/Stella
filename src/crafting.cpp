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


    struct Queue {
        struct Job {
            struct Request {
                Recipe *recipe;
                u32 count;
            };

            u32 have[N_ITEM_TYPES];
            Request *request;
            bool complete;

            void deinit() {
                arrfree(request);
            }

            static Job calculate(Recipe *recipe, Item_Container *player_inventory) {
                Job result = {};
                result.complete = !result.calculate(recipe, 1, player_inventory);
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
        u32 request_index;
        u32 request_count;
        u32 crafting_time;
        u32 progress;

        Dynamic_Item_Container crafting_buffer;

        bool crafting_paused = false;

        void init(Item_Container *player_inventory) {
            this->player_inventory = player_inventory;
        }

        void deinit() {
            arrfree(queue);
            crafting_buffer.deinit();
        }

        bool request(Recipe *r) {
            Job job = Job::calculate(r, player_inventory);
            
            if(!job.complete) {
                job.deinit();
                return false;
            }

            for(u32 i = 0; i < N_ITEM_TYPES; i++) {
                Item_Stack stack = { (Item_Type) i, job.have[i] };
                if(job.have[i] > 0) {
                    assert(player_inventory->remove(stack, false));
                    crafting_buffer.insert(stack);
                }
            }
            player_inventory->sort();

            arrput(queue, job);
            return true;
        }

        void update() {
            if(crafting_paused) return;

            if(actively_crafting) {
                if(progress != crafting_time) {
                    progress++;
                    return;
                }

                auto const& job = queue[0];
                if(request_index < arrlen(job.request)) {
                    auto const& req = job.request[request_index];

                    for(u32 i = 0; i < req.recipe->n_inputs; i++) {
                        assert(crafting_buffer.remove(req.recipe->inputs[i].type, req.recipe->inputs[i].count));
                    }
                    if(request_index < arrlen(job.request) - 1) crafting_buffer.insert(req.recipe->output);

                    request_count++;
                    if(request_count == req.count) {
                        request_count = 0;
                        request_index++;
                        if(request_index < arrlen(job.request)) {
                            crafting_time = job.request[request_index].recipe->time;
                            progress = 0;
                        } else {
                            assert(player_inventory->insert(job.request[request_index - 1].recipe->output) == 0);

                            queue[0].deinit();
                            arrdel(queue, 0);

                            actively_crafting = arrlen(queue) > 0;
                            request_index = 0;
                            request_count = 0;
                            progress = 0;
                            if(actively_crafting) crafting_time = queue[0].request[0].recipe->time;
                            else assert(crafting_buffer.total_count() == 0);
                        }
                    } else {
                        crafting_time = job.request[request_index].recipe->time;
                        progress = 0;
                    }
                }
            } else if(arrlen(queue) > 0) {
                actively_crafting = true;
                request_index = 0;
                request_count = 0;
                crafting_time = queue[0].request[0].recipe->time;
                progress = 0;
            }
        }

        void draw() {
            ImGui::Begin("Crafting Queue", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
                
            ImGui::Checkbox("Pause", &crafting_paused);

            ImGui::Separator();

            f32 crafting_progress = 0.0f;
            if(actively_crafting) {
                crafting_progress = (f32)progress / (f32)crafting_time;
            }
            ImGui::ProgressBar(crafting_progress, {100, 14});


            // TODO: This is pretty crappy lol...
            ImGui::BeginChild("Queue", { 100, 200 });

            // TODO: Remove this if statement!
            if(actively_crafting) {
                for(u32 i = 0; i < arrlen(queue); i++) {
                    auto const& job = queue[i];

                    u32 start;
                    if(i == 0) start = request_index;
                    else start = 0;

                    for(u32 k = start; k < arrlen(job.request); k++) {
                        auto const& req = job.request[k];

                        u32 end;
                        if(i == 0 && k == start) end = req.count - request_count;
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
            if(job_index == 0) {
                // NOTE: We cancelled the job we're working on;
                // transfer the items from crafting_buffer to
                // player_inventory.

                // NOTE TODO: Is it actually safe to do this?
                // In Java this would be akin to ConcurrentModificationException...
                // *shrugs* it seems to work rn... so... ???
                auto len = hmlen(crafting_buffer.entries);
                for(u32 i = 0; i < len; i++) {
                    auto const& entry = crafting_buffer.entries[i];

                    // NOTE: I explicitly do this before the hmdel;
                    // not sure / don't care if this is needed, 
                    // just being defensive!
                    assert(player_inventory->insert(Item_Stack(entry.key, entry.value)) == 0); // TODO: handle this failure!
                    hmdel(crafting_buffer.entries, entry.key);
                }
                assert(crafting_buffer.total_count() == 0);
            } else {
                // NOTE: We cancelled a job that we haven't
                // started working on yet; just give them back
                // the items in job.have.

                auto const& job = queue[job_index];
                for(u32 i = 0; i < N_ITEM_TYPES; i++) {
                    if(job.have[i] > 0) {
                        assert(player_inventory->insert(Item_Stack((Item_Type) i, job.have[i])) == 0); // TODO: Handle this failure!
                    }
                }
            }

            arrdel(queue, job_index);
            actively_crafting = false;
        }
    };
}
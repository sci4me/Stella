namespace crafting {
    struct Recipe {
        Item_Stack *inputs;
        u32 n_inputs;
        Item_Stack output;
        u32 time; // in ticks
    };

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
        Item_Container *inventory;
        Recipe **queue = nullptr;

        Recipe *actively_crafting = nullptr;
        u32 time_left;

        bool crafting_paused = false;

        void init(Item_Container *inventory) {
            this->inventory = inventory;
        }

        void deinit() {
            arrfree(queue);
        }

        bool request(Recipe *r) {
            // TODO: Right now we just check if we have the inputs to the recipe
            // and craft it with those. Soon, we'll want to be smarter about this.
            // If the player wants to craft say, I don't know, a mining machine,
            // and say it takes like 8 iron plates, 8 cobblestone, and 2 iron gears,
            // but they don't have the gears, they just have enough iron to make them;
            // we need to request those gears be crafted before the mining machine, 
            // automatically. This gets more complicated when you consider the cases
            // where they have _some_ of the intermediaries but not all of them.
            //
            // Shouldn't be too hard to "brute-force" this though. I think. inb4.
            //
            //                  - sci4me, 5/14/20

            //
            // Okay, so, first go at this. Here's my idea:
            // We have a function that recursively builds a list of itemstacks that
            // should be requested in order to craft `r`. This function also builds
            // a list of itemstacks that are missing.
            //
            // Then, if the list of items that are missing is empty, we can request
            // all of the items in the first list, and ... yeah. return true;
            //
            //                  - sci4me, 5/17/20
            //

            for(u32 i = 0; i < r->n_inputs; i++) {
                if(!inventory->contains(r->inputs[i])) return false;
            }

            for(u32 i = 0; i < r->n_inputs; i++) {
                assert(inventory->remove(r->inputs[i]));
            }

            arrput(queue, r);
            return true;
        }

        void update() {
            if(crafting_paused) return;

            if(actively_crafting) {
                if(time_left) {
                    time_left--;
                    return;
                }

                u32 remaining = inventory->insert(actively_crafting->output);
                if(remaining) {
                    // TODO: We need to essentially hold onto the rest of the items to-be-inserted and just insert them whever we can?
                    // This case can definitely happen, say if the player requests a bunch of stuff to be crafted and then
                    // mines a bunch of ores or grabs a bunch of stuff from another inventory.
                    assert(0);
                } else {
                    actively_crafting = nullptr;
                }
            } else if(arrlen(queue) > 0) {
                actively_crafting = queue[0];
                time_left = actively_crafting->time;

                arrdel(queue, 0);
            }
        }

        void draw() {
            ImGui::Begin("Crafting Queue", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
                
            ImGui::Text("Queue Count: %d", arrlen(queue) + (actively_crafting ? 1 : 0));
            ImGui::Checkbox("Pause", &crafting_paused);

            ImGui::Separator();

            f32 crafting_progress = 0.0f;
            if(actively_crafting) {
                crafting_progress = (f32)(actively_crafting->time - time_left) / (f32)actively_crafting->time;
            }
            ImGui::ProgressBar(crafting_progress, {100, 14});


            // TODO: This is pretty crappy lol...
            ImGui::BeginChild("Queue", { 100, 100 });

            if(actively_crafting) {
                // TODO: Change 32 to SLOT_SIZE
                if(ImGui::ImageButton((ImTextureID)(u64)item_textures[actively_crafting->output.type].id, { 32, 32 })) {
                    // TODO: cancel the request
                    assert(0);
                }
            }

            for(u32 i = 0; i < arrlen(queue); i++) {
                if(ImGui::ImageButton((ImTextureID)(u64)item_textures[queue[i]->output.type].id, { 32, 32 })) {
                    // TODO: cancel the request
                    assert(0);
                }   
            }

            ImGui::EndChild();


            ImGui::End();
        }
    };
}
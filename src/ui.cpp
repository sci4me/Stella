namespace ui {
    constexpr f32 SLOT_SIZE = 32.0f;

    // TODO: this won't work with hot code reload
    Item_Container *held_item_container;
    u32 held_item_index;

    void held_item() {
        if(!held_item_container) return;

        f32 half_slot_size = SLOT_SIZE / 2.0f;
        auto p = ImGui::GetMousePos();
        auto font = ImGui::GetFont();
        auto font_size = ImGui::GetFontSize();
        auto drawlist = ImGui::GetForegroundDrawList();
        
        drawlist->AddImage(
            (ImTextureID)(u64)g_inst->assets->item_textures[held_item_container->slots[held_item_index].type].id,
            { p.x - half_slot_size, p.y - half_slot_size },
            { p.x + half_slot_size, p.y + half_slot_size }
        );

        char buf[8];
        stbsp_snprintf(buf, 8, "%d", held_item_container->slots[held_item_index].count);

        auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);
        drawlist->AddText(
            font,
            font_size,
            { p.x - tsize.x + half_slot_size, p.y - tsize.y + half_slot_size },
            0xFFFFFFFF,
            buf
        );
    }

    void slot(Item_Container *container, u32 index) {
        // TODO: Should we just push the container as well as the index?
        ImGui::PushID(index);

        // NOTE: OH NO! A REFERENCE! WHAT EVER WILL SCI4ME DO?!?!!!?!$?!%!
        Item_Stack& slot = container->slots[index];

        auto style = ImGui::GetStyle();
        auto font = ImGui::GetFont();
        auto font_size = ImGui::GetFontSize();
        auto drawlist = ImGui::GetForegroundDrawList();

        bool clicked;
        if(slot.count && !(held_item_container == container && held_item_index == index)) {
            clicked = ImGui::ImageButton((ImTextureID)(u64)g_inst->assets->item_textures[slot.type].id, { SLOT_SIZE, SLOT_SIZE });
            
            char buf[8];
            stbsp_snprintf(buf, 8, "%d", slot.count);

            auto tpos = ImGui::GetItemRectMax();
            auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);

            drawlist->AddText(font, font_size, { tpos.x - tsize.x, tpos.y - tsize.y }, 0xFFFFFFFF, buf);
        } else {
            // TODO: Investigate whether we need to push an ID for this call to `ImGui::Button`...
            clicked = ImGui::Button("", { SLOT_SIZE + style.FramePadding.x * 2, SLOT_SIZE + style.FramePadding.y * 2 });
        }

        if(slot.count && ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(item_names[slot.type]);
            ImGui::EndTooltip();
        }

        ImGui::PopID();

        if(clicked) {
            if(held_item_container != nullptr) {
                if(!container->accepts_item_type(held_item_container->slots[held_item_index].type)) return;

                if(held_item_container == container) {
                    held_item_container = nullptr;
                } else {
                    u32 remaining = container->insert(held_item_container->slots[held_item_index]);
                    if(remaining) {
                        held_item_container->slots[held_item_index].count = remaining;
                    } else {
                        held_item_container->clear_slot(held_item_index);
                        held_item_container = nullptr;
                    }
                }
            } else if(slot.count) {
                if(container->flags & ITEM_CONTAINER_FLAG_NO_EXTRACT) return;

                held_item_container = container;
                held_item_index = index;
            }
        }
    }

    void container(Item_Container *container, u32 width, u32 height) {
        assert(width * height == container->size);

        ImGui::PushID(container); // NOTE: not sure if we _need_ this or not.. but.. ehh yeah.
        for(u32 j = 0; j < height; j++) {
            for(u32 i = 0; i < width; i++) {
                u32 k = i + j * width;

                slot(container, k);
                if(i < width - 1) ImGui::SameLine();
            }
        }
        ImGui::PopID();
    }

    void chest_ui(Item_Container *player_inventory, Tile_Chest *c, Tile **tile_ref) {
        TIMED_FUNCTION();

        // NOTE: Currently we only have one GUI open at a time,
        // so we don't have to push any extra ID info.
        bool open = true;
        if(ImGui::Begin("Chest", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            // NOTE TODO: hardcoded bs code for days 
            container(&c->container, 5, 5);

            ImGui::Separator();

            container(player_inventory, 4, 4);
            
            held_item();
        }
        ImGui::End();

        if(!open) *tile_ref = nullptr;
    }

    void furnace_ui(Item_Container *player_inventory, Tile_Furnace *f, Tile **tile_ref) {
        TIMED_FUNCTION();

        // TODO: Looks like shit but I guess it works for now...
        // TODO: Visual indication of fuel in internal buffer

        bool open = true;
        if(ImGui::Begin("Furnace", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Columns(2, NULL, false);
                ImGui::Text("Input:");
                container(&f->input, 1, 1);

                ImGui::Text("Fuel:");
                container(&f->fuel, 1, 1);

                ImGui::NextColumn();

                ImGui::Text("Output:");
                container(&f->output, 1, 1);
            
                ImGui::Dummy({0, 20});
                ImGui::ProgressBar((f32)f->smelting_progress / (f32)FUEL_POINTS_PER_SMELT, { 80, 14 }, "");
            ImGui::Columns(1);

            ImGui::Separator();

            container(player_inventory, 4, 4);

            held_item();
        }
        ImGui::End();

        if(!open) *tile_ref = nullptr;
    }

    void mining_machine_ui(Item_Container *player_inventory, Tile_Mining_Machine *m, Tile **tile_ref) {
        TIMED_FUNCTION();

        bool open = true;
        if(ImGui::Begin("Mining Machine", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Dummy({ 140, 0 });

            ImGui::Columns(2, NULL, false);
                ImGui::Text("Fuel:");
                container(&m->fuel, 1, 1);

                ImGui::NextColumn();

                ImGui::Text("Output:");
                container(&m->output, 1, 1);
            ImGui::Columns(1);

            ImGui::Separator();

            container(player_inventory, 4, 4);

            held_item();
        }
        ImGui::End();

        if(!open) *tile_ref = nullptr;
    }

    void tile_ui(Item_Container *player_inventory, Tile **tile_ref) {
        // TODO: Move the UI code into methods in the respective
        // Tile classes! We'll need to do forward declarations
        // to do this; that's why I did it this way in the first
        // place. Too lazy :P
        //              - sci4me, 5/17/20
        Tile *tile = *tile_ref;
        if(tile) {
            switch(tile->type) {
                case TILE_CHEST: {
                    Tile_Chest *c = (Tile_Chest*) tile;
                    chest_ui(player_inventory, c, tile_ref);                    
                    break;
                }
                case TILE_FURNACE: {
                    Tile_Furnace *f = (Tile_Furnace*) tile;
                    furnace_ui(player_inventory, f, tile_ref);
                    break;
                }
                case TILE_MINING_MACHINE: {
                    Tile_Mining_Machine *m = (Tile_Mining_Machine*) tile;
                    mining_machine_ui(player_inventory, m, tile_ref);
                    break;
                }
                default: {
                    assert(0);
                }
            }
        }
    }

    // NOTE: We just pass the Crafting_Queue since it contains the
    // Item_Container for the player inventory, and we need the Queue anyway.
    void player_inventory(Crafting_Queue *crafting_queue, bool *open) {
        TIMED_FUNCTION();

        if(!*open) return;

        if(ImGui::Begin("Inventory", open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            // NOTE TODO: hardcoded bs code for days

            auto inventory = crafting_queue->player_inventory;      
            container(inventory, 4, 4);

            ImGui::Separator();

            u32 f = 0;
            for(u32 i = 0; i < g_inst->recipes->recipes.count; i++) {
                auto r = g_inst->recipes->recipes[i];
                
                if(ImGui::ImageButton((ImTextureID)(u64)g_inst->assets->item_textures[r->output.type].id, { SLOT_SIZE, SLOT_SIZE })) {
                    // TODO: Handle result?
                    crafting_queue->request(r);
                }

                if(ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();

                    ImGui::Text(item_names[r->output.type]);

                    auto font = ImGui::GetFont();
                    auto font_size = ImGui::GetFontSize();
                    auto drawlist = ImGui::GetForegroundDrawList();

                    // NOTE TODO: Since we are doing the Factorio-style crafting,
                    // we should probably actually show the _raw_ inputs required for
                    // this recipe, rather than the "final" inputs for the recipe.
                    //
                    // If we show the inputs as we are now, if the player is missing
                    // an item that itself has a recipe (which could have sub-recipes, etc.)
                    // we aren't really telling the player what they're missing; they would
                    // have to go check the recipe for that item and see what components
                    // for that recipe they are missing.
                    //
                    // Therefore, we ought to show the leaf-nodes in the flattened version
                    // of the recipe tree for this recipe, rather than the child nodes of the
                    // root of that tree.
                    //
                    //                      - sci4me, 5/17/20
                    //

                    //
                    // Quick update to the above comment: I'm currently unsure how we would do this.
                    // In order to get the flattened version of the recipe, we'd have to call like,
                    // Job::calculate or something like it... but... we DEFINITELY don't want to do
                    // that per-frame in our render code! So...... hmh.
                    //
                    // It's not that Job::calculate has to do any intense computation or anything, but,
                    // you know, we're querying the player inventory constantly and that's probably
                    // ending up doing a ton of linear searches (albeit with a relatively small N, but still)
                    // So... I don't know. Maybe we could get away with calculating this every time just fine.
                    // Especially considering that we only have to do so when hovering over the 'recipe'.
                    // But idk.
                    //
                    // There's no cache-based optimization to do here since this depends on the player inventory,
                    // and also since the computation itself isn't that expensive.. that wouldn't make sense.
                    // It's really just the amount of iteration I'm worried about. But, obviously without profiling,
                    // this is just a crap shoot.
                    //
                    // *shrugs*
                    //
                    //                      - sci4me, 5/18/20
                    //

                    for(u32 j = 0; j < r->n_inputs; j++) {
                        auto& input = r->inputs[j];
                        u32 n = inventory->count_type(input.type);
                        
                        ImGui::Image((ImTextureID)(u64)g_inst->assets->item_textures[input.type].id, { SLOT_SIZE, SLOT_SIZE });

                        char buf[8];
                        stbsp_snprintf(buf, 8, "%d", input.count);

                        auto tpos = ImGui::GetItemRectMax();
                        auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);

                        drawlist->AddText(font, font_size, { tpos.x - tsize.x, tpos.y - tsize.y }, n < input.count ? 0xFF0000FF : 0xFFFFFFFF, buf);

                        // TODO: Eventually we'll want multiple lines...
                        if(j < (r->n_inputs - 1)) ImGui::SameLine();
                    }

                    ImGui::EndTooltip();
                }

                f++;
                if(f == 4) {
                    f = 0;
                } else {
                    ImGui::SameLine();
                }

                // TODO: Don't _always_ do this!
                // if(i % 4 != 0) ImGui::SameLine();
            }

            held_item();
        }
        ImGui::End();
    }
}
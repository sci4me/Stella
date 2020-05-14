namespace ui {
    constexpr f32 SLOT_SIZE = 32.0f;

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
            (ImTextureID) item_textures[held_item_container->slots[held_item_index].type].id,
            { p.x - half_slot_size, p.y - half_slot_size },
            { p.x + half_slot_size, p.y + half_slot_size }
        );

        char buf[8];
        snprintf(buf, 8, "%d", held_item_container->slots[held_item_index].count);

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
            clicked = ImGui::ImageButton((ImTextureID) item_textures[slot.type].id, { SLOT_SIZE, SLOT_SIZE });
            
            char buf[8];
            snprintf(buf, 8, "%d", slot.count);

            auto tpos = ImGui::GetItemRectMax();
            auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);

            drawlist->AddText(font, font_size, { tpos.x - tsize.x, tpos.y - tsize.y }, 0xFFFFFFFF, buf);
        } else {
            // TODO: Investigate whether we need to push an ID for this call to `ImGui::Button`...
            clicked = ImGui::Button("", { SLOT_SIZE + style.FramePadding.x * 2, SLOT_SIZE + style.FramePadding.y * 2 });
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
                        held_item_container->remove(held_item_index);
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
        // NOTE: Currently we only have one GUI open at a time,
        // so we don't have to push any extra ID info.
        bool open = true;
        if(ImGui::Begin("Chest", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            // NOTE TODO: hardcoded bs code for days
            ui::container(&c->container, 5, 5);

            ImGui::Separator();

            ui::container(player_inventory, 4, 4);
            
            ui::held_item();
        }
        ImGui::End();

        if(!open) *tile_ref = nullptr;
    }

    void furnace_ui(Item_Container *player_inventory, Tile_Furnace *f, Tile **tile_ref) {
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

    void tile_ui(Item_Container *player_inventory, Tile **tile_ref) {
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
                default: {
                    assert(0);
                }
            }
        }
    }

    void player_inventory(Item_Container *inventory, bool *open) {
        if(!*open) return;
        
        if(ImGui::Begin("Inventory", open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            // NOTE TODO: hardcoded bs code for days
            ui::container(inventory, 4, 4);
            ui::held_item();
        }
        ImGui::End();
    }
}